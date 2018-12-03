#include "playback.h"
#include "reader.h"

#include <ao/ao.h>

#include <inttypes.h>
#include <pthread.h>
#include <assert.h>


#define MAX_QUEUED 1024  // the number of sounds to queue before we start overwriting
#define BUFFER_SIZE 256  // number of audio samples we play at a time

enum PlaybackState {
    PLAYBACK_WAITING,
    PLAYBACK_ATTACK,
    PLAYBACK_SUSTAIN,
    PLAYBACK_RELEASE
};

// locals
ao_sample_format format;
ao_device* device;

pthread_t playback_thread;
uint8_t quit = 0;

struct ASRSamples* queue[MAX_QUEUED];
u_int insertion_index = 0;
u_int playback_index = 0;

enum PlaybackState playback_state = PLAYBACK_WAITING;
int samples_released = 0;


// functions
static void increment_index(u_int *value) {
    ++*value;
    if (*value >= MAX_QUEUED) {
        *value = 0;
    }
}

static void play(struct Sample* sample) {
    uint32_t num_chunks = sample->num_bytes / BUFFER_SIZE;
    uint32_t chunk_size = BUFFER_SIZE;
    for (uint i = 0; i < num_chunks; i++) {
        int is_final_chunk = i == num_chunks - 1;
        if (is_final_chunk) {
            chunk_size = sample->num_bytes % BUFFER_SIZE;
        }
        ao_play(device, sample->sample_data + i * BUFFER_SIZE, chunk_size);
    }
}

static void set_format() {
    format.bits = 16;  // entire program is built on this assumption, reading and playback will fail if changed
    format.channels = 1;
    format.rate = 44100;
    format.byte_format = AO_FMT_BIG;  // load_sample in reader.c assumes big endian order
}

static void initialise_ao() {
    set_format();
    ao_initialize();
    device = ao_open_live(ao_default_driver_id(), &format, NULL);
    if (device == NULL) {
        fprintf(stderr, "Error opening audio device, exiting.\n");
        exit(EXIT_FAILURE);
    }
}

static void playback_thread_loop(void) {
    while (!quit) {
        struct ASRSamples* samples = queue[playback_index];

        if (samples != NULL) {
            assert(playback_state == PLAYBACK_ATTACK);  // Must have already been set or we've missed a safeguard.
            play(samples->attack);

            playback_state = PLAYBACK_SUSTAIN;
            play(samples->sustain);  // Play sustain sample at least once.
            while (samples_released == 0) {
                play(samples->sustain);
            }

            playback_state = PLAYBACK_RELEASE;
            play(samples->release);

            playback_state = PLAYBACK_WAITING;
            --samples_released;
            queue[playback_index] = NULL;
            increment_index(&playback_index);
        } else {
            // TODO: sleep so we don't thrash the CPU
        }
    }
}

static void start_playback_thread(void) {
    pthread_create(&playback_thread, NULL, (void*) &playback_thread_loop, NULL);
}

static void tear_down_ao(void) {
    if (ao_close(device) != 1) {
        fprintf(stderr, "Error closing audio output device.\n");
    }
    ao_shutdown();
}

void initialise_playback(void) {
    initialise_ao();
    start_playback_thread();
}

void release_sample(void) {
    ++samples_released;
}

void play_if_silent(struct ASRSamples *asr_samples) {
    if (asr_samples == NULL || asr_samples->attack == NULL ||
            asr_samples->sustain == NULL || asr_samples->release == NULL) {
        fprintf(stderr, "NULL samples queued for playback.\n");
        return;
    }

    if (playback_state != PLAYBACK_WAITING) {
        return;
    }

    playback_state = PLAYBACK_ATTACK;
    queue[insertion_index] = asr_samples;
    increment_index(&insertion_index);
}

void tear_down_playback(void) {
    quit = 1;
    pthread_join(playback_thread, NULL);  // wait for currently playing sound to finish
    tear_down_ao();
}

#undef MAX_QUEUED
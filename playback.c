#include "playback.h"

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
u_int pitch_index = SEMITONES_RANGE;  // the centre pitch (unshifted)

enum PlaybackState playback_state = PLAYBACK_WAITING;
int samples_released = 0;


// functions
void increase_pitch(void) {
    if (pitch_index < PITCHES_AVAILABLE - 1) {
        ++pitch_index;
    }
}

void decrease_pitch(void) {
    if (pitch_index > 0) {
        --pitch_index;
    }
}

int asr_samples_lengths_consistent(struct ASRSamples *asr_samples) {
    const uint32_t attack_length = asr_samples->attack[0]->num_bytes;
    const uint32_t sustain_length = asr_samples->sustain[0]->num_bytes;
    const uint32_t release_length = asr_samples->release[0]->num_bytes;

    for (uint i = 1; i < PITCHES_AVAILABLE; i++) {
        if (asr_samples->attack[i]->num_bytes != attack_length
            ||asr_samples->sustain[i]->num_bytes != sustain_length
            ||asr_samples->release[i]->num_bytes != release_length) {
            return 0;
        }
    }

    return 1;
}

int asr_samples_contains_null(struct ASRSamples* asr_samples) {
    if (asr_samples == NULL) {
        return 1;
    }

    for (uint i = 0; i < PITCHES_AVAILABLE; i++) {
        if (asr_samples->attack[i] == NULL || asr_samples->sustain[i] == NULL || asr_samples->release[i] == NULL) {
            return 1;
        }
    }

    return 0;
}

static void increment_index(u_int *value) {
    ++*value;
    if (*value >= MAX_QUEUED) {
        *value = 0;
    }
}

static void play(struct Sample* sample[]) {
    // Assumes that every struct Sample has the same num_bytes value.
    // This is checked when samples are loaded in reader.c so should be fine here.
    uint32_t num_chunks = sample[0]->num_bytes / BUFFER_SIZE;
    uint32_t chunk_size = BUFFER_SIZE;
    for (uint i = 0; i < num_chunks; i++) {
        int is_final_chunk = i == num_chunks - 1;
        if (is_final_chunk) {
            chunk_size = sample[0]->num_bytes % BUFFER_SIZE;
        }
        ao_play(device, sample[pitch_index]->sample_data + i * BUFFER_SIZE, chunk_size);
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

        if (!asr_samples_contains_null(samples)) {
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

int is_silent(void) {
    return playback_state == PLAYBACK_WAITING;
}

void play_if_silent(struct ASRSamples *asr_samples) {
    if (asr_samples_contains_null(asr_samples)) {
        fprintf(stderr, "NULL samples queued for playback.\n");
        return;
    }

    if (!is_silent()) {
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
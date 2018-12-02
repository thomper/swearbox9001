#include "playback.h"
#include "reader.h"

#include <ao/ao.h>

#include <inttypes.h>
#include <pthread.h>


#define MAX_QUEUED 1024  // the number of sounds to queue before we start overwriting

enum PlaybackState {
    PLAYBACK_WAITING,
    PLAYBACK_ATTACK,
    PLAYBACK_SUSTAIN,
    PLAYBACK_RELEASE
};

// locals
pthread_t playback_thread;
ao_sample_format format;
ao_device* device;
uint8_t quit = 0;
struct Sample* queue[MAX_QUEUED];
u_int insertion_index = 0;
u_int playback_index = 0;
enum PlaybackState playback_state = PLAYBACK_WAITING;

// functions
static void increment_index(u_int *value) {
    ++*value;
    if (*value >= MAX_QUEUED) {
        *value = 0;
    }
}

static void play(struct Sample* sample) {
    ao_play(device, sample->sample_data, sample->num_bytes);
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
        if (queue[playback_index] != NULL) {
            playback_state = PLAYBACK_ATTACK;
            play(queue[playback_index]);
            playback_state = PLAYBACK_WAITING;
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

void play_if_silent(struct Sample *sample) {
    if (sample == NULL) {
        fprintf(stderr, "NULL sample queued for playback.\n");
        return;
    }

    if (playback_state != PLAYBACK_WAITING) {
        return;
    }

    queue[insertion_index] = sample;
    increment_index(&insertion_index);
}

void tear_down_playback(void) {
    quit = 1;
    pthread_join(playback_thread, NULL);  // wait for currently playing sound to finish
    tear_down_ao();
}

#undef MAX_QUEUED
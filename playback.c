#include "playback.h"
#include <pthread.h>
#include <ao/ao.h>

/* local variables */
pthread_t playback_thread;
u_int quit = 0;
char* queue_buffers[MAX_QUEUED];
uint_32 queue_lengths[MAX_QUEUED];
u_int insertion_index = 0;
u_int playback_index = 0;

/* global variables */
ao_device* device;
ao_sample_format format;

/* local functions */
void increment_index(u_int *value) {
    ++*value;
    if (*value >= MAX_QUEUED) {
        *value = 0;
    }
}

void set_format() {
    format.bits = 16;
    format.channels = 1;
    format.rate = 44100;
    format.byte_format = AO_FMT_LITTLE;
}

void initialise_ao() {
    set_format();
    ao_initialize();
    device = ao_open_live(ao_default_driver_id(), &format, NULL /* no options */);
    if (device == NULL) {
        fprintf(stderr, "Error opening device, exiting.\n");
        exit(EXIT_FAILURE);
    }
}

void play(char* samples, uint_32 num_bytes) {
    ao_play(device, samples, num_bytes);
}

void playback_thread_loop(void* unused) {
    while (!quit) {
        if (queue_lengths[playback_index] > 0) {
            play(queue_buffers[playback_index], queue_lengths[playback_index]);
            queue_buffers[playback_index] = NULL;
            queue_lengths[playback_index] = 0;
            increment_index(&playback_index);
        } else {
            // TODO: sleep so we don't thrash the CPU
        }
    }
}

void start_playback_thread(void) {
    pthread_create(&playback_thread, NULL, (void*) &playback_thread_loop, NULL);
}

void tear_down_ao(void) {
    ao_close(device);
    ao_shutdown();
}

/* global functions */
void initialise(void) {
    initialise_ao();
    start_playback_thread();
}

void queue_playback(char *samples, uint_32 num_bytes) {
    queue_buffers[insertion_index] = samples;
    queue_lengths[insertion_index] = num_bytes;
    increment_index(&insertion_index);
}

void tear_down(void) {
    quit = 1;
    pthread_join(playback_thread, NULL);  // wait for currently playing sound to finish
    tear_down_ao();
}
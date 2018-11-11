#include "playback.h"
#include <ao/ao.h>

ao_device* device;
ao_sample_format format;

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

void initialise(void) {
    initialise_ao();
}

void play(char *samples, uint_32 num_bytes) {
    ao_play(device, samples, num_bytes);
}

void tear_down_ao(void) {
    ao_close(device);
    ao_shutdown();
}

void tear_down(void) {
    tear_down_ao();
}
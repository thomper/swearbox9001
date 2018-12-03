#ifndef SWEARBOX9001_PLAYBACK_H
#define SWEARBOX9001_PLAYBACK_H

#include <ao/ao.h>

#include <inttypes.h>

#define BANK_SIZE 24  // the number of sounds available per bank


struct Sample {
    uint32_t num_bytes;
    char* sample_data;
};

struct ASRSamples {
    struct Sample* attack;
    struct Sample* sustain;
    struct Sample* release;
};

struct SampleBank {
    struct ASRSamples* array[BANK_SIZE];
    char* filename_stubs[BANK_SIZE];
};

void initialise_playback(void);

int is_silent(void);

void play_if_silent(struct ASRSamples *asr_samples);

void release_sample(void);

void tear_down_playback(void);


#endif //SWEARBOX9001_PLAYBACK_H

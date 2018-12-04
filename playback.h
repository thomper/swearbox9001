#ifndef SWEARBOX9001_PLAYBACK_H
#define SWEARBOX9001_PLAYBACK_H

#include <ao/ao.h>

#include <inttypes.h>

#define BANK_SIZE 24  // the number of sounds available per bank
#define SEMITONES_RANGE 12  // the number of semitones available up and down
#define PITCHES_AVAILABLE (SEMITONES_RANGE * 2 + 1)


struct Sample {
    uint32_t num_bytes;
    char* sample_data;
};

struct ASRSamples {
    struct Sample* attack[PITCHES_AVAILABLE];
    struct Sample* sustain[PITCHES_AVAILABLE];
    struct Sample* release[PITCHES_AVAILABLE];
};

struct SampleBank {
    struct ASRSamples* array[BANK_SIZE];
    char* filename_stubs[BANK_SIZE];
};

void initialise_playback(void);

int is_silent(void);

void play_if_silent(struct ASRSamples *asr_samples);

void release_sample(void);

void increase_pitch(void);

void decrease_pitch(void);

int asr_samples_lengths_consistent(struct ASRSamples *asr_samples);

void tear_down_playback(void);


#endif //SWEARBOX9001_PLAYBACK_H

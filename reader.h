#ifndef SWEARBOX9001_READER_H
#define SWEARBOX9001_READER_H

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

/**
 * Load samples from file.
 *
 * @param bank Must be already allocated with filename_stubs set.  array must not be allocated.
 * @return 0 on success, non-zero otherwise.
 */
int load_sample_bank(struct SampleBank* bank);
void free_sample_bank_array(struct SampleBank* bank);

#endif //SWEARBOX9001_READER_H

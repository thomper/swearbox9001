#ifndef SWEARBOX9001_READER_H
#define SWEARBOX9001_READER_H

#include <inttypes.h>


struct Sample {
    uint32_t num_bytes;
    char* sample_data;
};

void load_samples(void);

#endif //SWEARBOX9001_READER_H

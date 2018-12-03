#ifndef SWEARBOX9001_READER_H
#define SWEARBOX9001_READER_H

#include "playback.h"  // TODO: this is just for the sample structs, they should be split out to their own module

#include <inttypes.h>


/**
 * Load samples from file.
 *
 * @param bank Must be already allocated with filename_stubs set.  array must not be allocated.
 * @return 0 on success, non-zero otherwise.
 */
int load_sample_bank(struct SampleBank* bank);
void free_sample_bank_array(struct SampleBank* bank);


#endif //SWEARBOX9001_READER_H

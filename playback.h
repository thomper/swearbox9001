#ifndef SWEARBOX9001_PLAYBACK_H
#define SWEARBOX9001_PLAYBACK_H

#include <ao/ao.h>

extern ao_device* device;
extern ao_sample_format format;

void initialise(void);

void play(char *samples, uint_32 num_bytes);

void tear_down(void);

#endif //SWEARBOX9001_PLAYBACK_H

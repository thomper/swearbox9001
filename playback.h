#ifndef SWEARBOX9001_PLAYBACK_H
#define SWEARBOX9001_PLAYBACK_H

#include <ao/ao.h>

#define MAX_QUEUED 1024  // the number of sounds to queue before we start overwriting

extern ao_sample_format format;

void initialise(void);

void queue_playback(char *samples, uint_32 num_bytes);

void tear_down(void);

#endif //SWEARBOX9001_PLAYBACK_H

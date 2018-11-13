#ifndef SWEARBOX9001_PLAYBACK_H
#define SWEARBOX9001_PLAYBACK_H

#include "reader.h"

#include <ao/ao.h>

#include <inttypes.h>


void initialise_playback(void);

void queue_playback(struct Sample* sample);

void tear_down_playback(void);

#endif //SWEARBOX9001_PLAYBACK_H

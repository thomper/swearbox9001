#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ao/ao.h>
#include "playback.h"

int main() {
	char *buffer;
	size_t buf_size;
	int sample;
	float freq = 440.0;
	int i;

    initialise();

    /* create some sample data for testing */
	buf_size = format.bits / 8 * format.channels * format.rate;
	buffer = calloc(buf_size, sizeof(char));

	for (i = 0; i < format.rate; i++) {
		sample = (int)(0.75 * 32768.0 *
			sin(2 * M_PI * freq * ((float) i/format.rate)));

		buffer[4*i] = sample & 0xff;
		buffer[4*i+1] = (sample >> 8) & 0xff;
	}

	play(buffer, buf_size);
	tear_down();

    return EXIT_SUCCESS;
}


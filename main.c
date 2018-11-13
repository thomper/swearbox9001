#include "playback.h"
#include "reader.h"

#include <ao/ao.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>


int main() {
	load_samples();

    initialise_playback();

    sleep(1);  // for testing, so we don't quit before we play the first sound

    tear_down_playback();

    return EXIT_SUCCESS;
}


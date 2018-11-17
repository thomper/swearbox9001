#include "playback.h"
#include "reader.h"

#include <ao/ao.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>


int main() {
    initialise_playback();

    struct SampleBank bank;
    for (uint16_t i = 0; i < BANK_SIZE; i++) {
        bank.filename_stubs[i] = NULL;
    }
    bank.filename_stubs[0] = "test_f";
    bank.filename_stubs[1] = "song_f";

    load_sample_bank(&bank);
    queue_playback(bank.array[0]->attack);
    queue_playback(bank.array[0]->release);
    queue_playback(bank.array[1]->attack);
    queue_playback(bank.array[1]->sustain);
    queue_playback(bank.array[1]->release);

    sleep(9);  // for testing, so we don't quit before we're done playing sounds

    tear_down_playback();

    return EXIT_SUCCESS;
}


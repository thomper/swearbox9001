#include "input.h"
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
    bank.filename_stubs[0] = "song_f";
    bank.filename_stubs[1] = "test_f";

    load_sample_bank(&bank);

    input_thread_loop(&bank);

    free_sample_bank_array(&bank);
    tear_down_playback();

    return EXIT_SUCCESS;
}


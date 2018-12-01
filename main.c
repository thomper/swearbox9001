#include "input.h"
#include "playback.h"
#include "reader.h"

#include <ao/ao.h>
#include <SDL2/SDL_events.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>


struct SampleBank bank;

void initialise_samples(void) {
    for (uint16_t i = 0; i < BANK_SIZE; i++) {
        bank.filename_stubs[i] = NULL;
    }
    bank.filename_stubs[0] = "song_f";

    load_sample_bank(&bank);
}

int main_loop() {
    SDL_Event event;
    uint16_t current_buttons = 0;
    while (1) {
        if (SDL_PollEvent(&event) == 0) {
            continue;
        }

        current_buttons = key_enum_from_sdl_event(&event, current_buttons);

        printf("New current: ");
        {
            unsigned char *b = (unsigned char *) &current_buttons;
            unsigned char byte;
            int i, j;

            for (i = sizeof (uint16_t) - 1; i >= 0; i--) {
                for (j = 7; j >= 0; j--) {
                    byte = (b[i] >> j) & 1;
                    printf("%u", byte);
                }
            }
            puts("");
        }

        if (event.type == SDL_JOYBUTTONDOWN) {
            if (event.jbutton.button == 8) {
                //TODO: Testing only, should never quit
                break;
            }
        }
    }
}

int main() {
    initialise_joystick();
    initialise_playback();
    initialise_samples();

    main_loop();

    free_sample_bank_array(&bank);
    tear_down_playback();

    return EXIT_SUCCESS;
}

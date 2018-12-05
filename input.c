#include "input.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

#include <assert.h>
#include <stdlib.h>


SDL_Joystick* joystick = NULL;
const struct SB9001Event EMPTY_SB_EVENT = {
    SB9001_HORI_MOTION_NONE,  // hori_motion
    SB9001_VERT_MOTION_NONE,  // vert_motion

    SB9001_BANK_KEY_NONE,     // bank_key
    0,                        // bank_key_depressed

    SB9001_ACTION_KEY_NONE,   // action_key
    0                         // action_key_depressed
};

struct SB9001Event sb_event_from_sdl_event(SDL_Event* event) {
    struct SB9001Event current = EMPTY_SB_EVENT;

    if (event->type == SDL_JOYBUTTONDOWN) {
        switch (event->jbutton.button) {
            case 0: current.bank_key_depressed = 1; current.bank_key = SB9001_BANK_KEY_B; break;
            case 4: current.bank_key_depressed = 1; current.bank_key = SB9001_BANK_KEY_C; break;
            case 1: current.action_key_depressed = 1; current.action_key = SB9001_ACTION_KEY_A; break;
            case 2: current.action_key_depressed = 1; current.action_key = SB9001_ACTION_KEY_B; break;
            case 3: current.action_key_depressed = 1; current.action_key = SB9001_ACTION_KEY_C; break;
            case 5: current.action_key_depressed = 1; current.action_key = SB9001_ACTION_KEY_D; break;
            case 6: current.action_key_depressed = 1; current.action_key = SB9001_ACTION_KEY_E; break;
            case 7: current.action_key_depressed = 1; current.action_key = SB9001_ACTION_KEY_F; break;
            default: ;
        }
    } else if (event->type == SDL_JOYBUTTONUP) {
        switch (event->jbutton.button) {
            case 0: current.bank_key_depressed = 0; current.bank_key = SB9001_BANK_KEY_B; break;
            case 4: current.bank_key_depressed = 0; current.bank_key = SB9001_BANK_KEY_C; break;
            case 1: current.action_key_depressed = 0; current.action_key = SB9001_ACTION_KEY_A; break;
            case 2: current.action_key_depressed = 0; current.action_key = SB9001_ACTION_KEY_B; break;
            case 3: current.action_key_depressed = 0; current.action_key = SB9001_ACTION_KEY_C; break;
            case 5: current.action_key_depressed = 0; current.action_key = SB9001_ACTION_KEY_D; break;
            case 6: current.action_key_depressed = 0; current.action_key = SB9001_ACTION_KEY_E; break;
            case 7: current.action_key_depressed = 0; current.action_key = SB9001_ACTION_KEY_F; break;
            default: ;
        }
    } else if (event->type == SDL_JOYAXISMOTION) {
        if (event->jaxis.axis == 0) {
            switch (event->jaxis.value) {
                case -32768: current.hori_motion = SB9001_HORI_MOTION_LEFT; break;
                case 0: current.hori_motion = SB9001_HORI_MOTION_CENTRE; break;
                case 32767: current.hori_motion = SB9001_HORI_MOTION_RIGHT; break;
                default: break;
            }
        } else if (event->jaxis.axis == 1) {
            switch (event->jaxis.value) {
                case -32768: current.vert_motion = SB9001_VERT_MOTION_UP; break;
                case 0: current.vert_motion = SB9001_VERT_MOTION_CENTRE; break;
                case 32767: current.vert_motion = SB9001_VERT_MOTION_DOWN; break;
                default: break;
            }
        }
    }

    return current;
}

void initialise_joystick(void) {
    int num_joysticks = SDL_NumJoysticks();
    if (num_joysticks < 1) {
        fprintf(stderr, "No joysticks found, exiting.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_joysticks; i++) {
        joystick = SDL_JoystickOpen(i);
        if (joystick != NULL) {
            printf("Successfully opened joystick %d.\n", i);
            SDL_JoystickEventState(SDL_ENABLE);
            return;
        }
    }

    fprintf(stderr, "Could not open a joystick (found %d).  Exiting.\n", num_joysticks);
    exit(EXIT_FAILURE);
}

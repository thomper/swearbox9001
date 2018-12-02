#include "input.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

#include <assert.h>
#include <stdlib.h>


uint16_t SB9001_KEY_LEFT     = 0b00000000000001;
uint16_t SB9001_KEY_RIGHT    = 0b00000000000010;
uint16_t SB9001_KEY_UP       = 0b00000000000100;
uint16_t SB9001_KEY_DOWN     = 0b00000000001000;
uint16_t SB9001_KEY_BANK_B   = 0b00000000010000;
uint16_t SB9001_KEY_BANK_C   = 0b00000000100000;
uint16_t SB9001_KEY_A        = 0b00000001000000;
uint16_t SB9001_KEY_B        = 0b00000010000000;
uint16_t SB9001_KEY_C        = 0b00000100000000;
uint16_t SB9001_KEY_D        = 0b00001000000000;
uint16_t SB9001_KEY_E        = 0b00010000000000;
uint16_t SB9001_KEY_F        = 0b00100000000000;
uint16_t SB9001_KEY_OPTION_A = 0b01000000000000;
uint16_t SB9001_KEY_OPTION_B = 0b10000000000000;

SDL_Joystick* joystick = NULL;

uint16_t key_enum_from_sdl_event(SDL_Event* event, uint16_t current) {
    if (event->type == SDL_JOYBUTTONDOWN) {
        switch (event->jbutton.button) {
            case 0: current |= SB9001_KEY_BANK_B; break;
            case 1: current |= SB9001_KEY_A; break;
            case 2: current |= SB9001_KEY_B; break;
            case 3: current |= SB9001_KEY_C; break;
            case 4: current |= SB9001_KEY_BANK_C; break;
            case 5: current |= SB9001_KEY_D; break;
            case 6: current |= SB9001_KEY_E; break;
            case 7: current |= SB9001_KEY_F; break;
            default: break;
        }
    } else if (event->type == SDL_JOYBUTTONUP) {
        switch (event->jbutton.button) {
            case 0: current &= ~SB9001_KEY_BANK_B; break;
            case 1: current &= ~SB9001_KEY_A; break;
            case 2: current &= ~SB9001_KEY_B; break;
            case 3: current &= ~SB9001_KEY_C; break;
            case 4: current &= ~SB9001_KEY_BANK_C; break;
            case 5: current &= ~SB9001_KEY_D; break;
            case 6: current &= ~SB9001_KEY_E; break;
            case 7: current &= ~SB9001_KEY_F; break;
            default: break;
        }
    } else if (event->type == SDL_JOYAXISMOTION) {
        if (event->jaxis.axis == 0) {
            switch (event->jaxis.value) {
                case -32768: current |= SB9001_KEY_LEFT; break;
                case 0: current &= (~SB9001_KEY_LEFT & ~SB9001_KEY_RIGHT); break;
                case 32767: current |= SB9001_KEY_RIGHT; break;
                default: break;
            }
        } else if (event->jaxis.axis == 1) {
            switch (event->jaxis.value) {
                case -32768: current |= SB9001_KEY_UP; break;
                case 0: current &= (~SB9001_KEY_UP & ~SB9001_KEY_DOWN); break;
                case 32767: current |= SB9001_KEY_DOWN; break;
                default: break;
            }
        }
    }

    return current;
}

void initialise_joystick(void) {
    SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_EVENTS);

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

    fprintf(stderr, "Could not open a joystick (%d found).  Exiting.\n", num_joysticks);
    exit(EXIT_FAILURE);
}

uint16_t action_key_in_keycodes(uint16_t keys) {
    return keys & (SB9001_KEY_A |
                   SB9001_KEY_B |
                   SB9001_KEY_C |
                   SB9001_KEY_D |
                   SB9001_KEY_E |
                   SB9001_KEY_F);
}

uint16_t bank_addend_from_keycodes(uint16_t keys) {
    if (keys & SB9001_KEY_BANK_B) {
        return 6;
    }

    if (keys & SB9001_KEY_BANK_C) {
        return 12;
    }

    return 0;
}

uint16_t action_addend_from_keycodes(uint16_t keys) {
    assert(action_key_in_keycodes(keys));

    if (keys & SB9001_KEY_A) {
        return 0;
    }

    if (keys & SB9001_KEY_B) {
        return 1;
    }

    if (keys & SB9001_KEY_C) {
        return 2;
    }

    if (keys & SB9001_KEY_D) {
        return 3;
    }

    if (keys & SB9001_KEY_E) {
        return 4;
    }

    if (keys & SB9001_KEY_F) {
        return 5;
    }

    assert(0);  // should never be reached
}

uint16_t bank_array_index_from_keycodes(uint16_t keys) {
    assert(action_key_in_keycodes(keys));

    return bank_addend_from_keycodes(keys) + action_addend_from_keycodes(keys);
}

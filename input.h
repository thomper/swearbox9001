#ifndef SWEARBOX9001_INPUT_H
#define SWEARBOX9001_INPUT_H

#include "playback.h"

#include <SDL2/SDL_events.h>

#include <stdint.h>


extern uint16_t SB9001_KEY_LEFT;
extern uint16_t SB9001_KEY_RIGHT;
extern uint16_t SB9001_KEY_UP;
extern uint16_t SB9001_KEY_DOWN;
extern uint16_t SB9001_KEY_BANK_B;
extern uint16_t SB9001_KEY_BANK_C;
extern uint16_t SB9001_KEY_A;
extern uint16_t SB9001_KEY_B;
extern uint16_t SB9001_KEY_C;
extern uint16_t SB9001_KEY_D;
extern uint16_t SB9001_KEY_E;
extern uint16_t SB9001_KEY_F;
extern uint16_t SB9001_KEY_OPTION_A;
extern uint16_t SB9001_KEY_OPTION_B;

void initialise_joystick(void);

uint16_t key_enum_from_sdl_event(SDL_Event* event, uint16_t current);

uint16_t action_key_in_keycodes(uint16_t keys);

uint16_t bank_array_index_from_keycodes(uint16_t keys);


#endif //SWEARBOX9001_INPUT_H

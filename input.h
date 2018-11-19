#ifndef SWEARBOX9001_INPUT_H
#define SWEARBOX9001_INPUT_H

#include "playback.h"

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

struct KeyEnum {
    uint16_t KEY_LEFT;
    uint16_t KEY_RIGHT;
    uint16_t KEY_UP;
    uint16_t KEY_DOWN;
    uint16_t KEY_BANK_A;
    uint16_t KEY_BANK_B;
    uint16_t KEY_A;
    uint16_t KEY_B;
    uint16_t KEY_C;
    uint16_t KEY_D;
    uint16_t KEY_E;
    uint16_t KEY_F;
    uint16_t KEY_OPTION_A;
    uint16_t KEY_OPTION_B;
};

extern struct KeyEnum SB9001_KEYS;

uint16_t action_key_in_keycodes(uint16_t keys);

uint16_t bank_addend_from_keycodes(uint16_t keys);

uint16_t action_addend_from_keycodes(uint16_t keys);

uint16_t bank_array_index_from_keycodes(uint16_t keys);

void input_thread_loop(struct SampleBank* bank);


#endif //SWEARBOX9001_INPUT_H

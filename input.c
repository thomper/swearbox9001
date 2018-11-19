#include "input.h"
#include "playback.h"

#include <stdio.h>
#include <stdlib.h>

struct KeyEnum SB9001_KEYS;

uint16_t action_key_in_keycodes(uint16_t keys) {
    return keys & (SB9001_KEYS.KEY_A |
                   SB9001_KEYS.KEY_B |
                   SB9001_KEYS.KEY_C |
                   SB9001_KEYS.KEY_D |
                   SB9001_KEYS.KEY_E |
                   SB9001_KEYS.KEY_F);
}

uint16_t bank_addend_from_keycodes(uint16_t keys) {
    if (keys & SB9001_KEYS.KEY_BANK_A) {
        return 6;
    }

    if (keys & SB9001_KEYS.KEY_BANK_B) {
        return 12;
    }

    return 0;
}

uint16_t action_addend_from_keycodes(uint16_t keys) {
    assert(action_key_in_keycodes(keys));

    if (keys & SB9001_KEYS.KEY_A) {
        return 0;
    }

    if (keys & SB9001_KEYS.KEY_B) {
        return 1;
    }

    if (keys & SB9001_KEYS.KEY_C) {
        return 2;
    }

    if (keys & SB9001_KEYS.KEY_D) {
        return 3;
    }

    if (keys & SB9001_KEYS.KEY_E) {
        return 4;
    }

    if (keys & SB9001_KEYS.KEY_F) {
        return 5;
    }

    assert(0);  // should never be reached
}

uint16_t bank_array_index_from_keycodes(uint16_t keys) {
    assert(action_key_in_keycodes(keys));

    return bank_addend_from_keycodes(keys) + action_addend_from_keycodes(keys);
}

static uint16_t keycodes_from_keyboard_key(int keychar) {
    switch (keychar) {
        case 'z':
            return SB9001_KEYS.KEY_A;
        case 'x':
            return SB9001_KEYS.KEY_B;
        case 'c':
            return SB9001_KEYS.KEY_C;
        case 'v':
            return SB9001_KEYS.KEY_D;
        case 'b':
            return SB9001_KEYS.KEY_E;
        case 'n':
            return SB9001_KEYS.KEY_F;

        case 'a':
            return SB9001_KEYS.KEY_A | SB9001_KEYS.KEY_BANK_A;
        case 's':
            return SB9001_KEYS.KEY_B | SB9001_KEYS.KEY_BANK_A;
        case 'd':
            return SB9001_KEYS.KEY_C | SB9001_KEYS.KEY_BANK_A;
        case 'f':
            return SB9001_KEYS.KEY_D | SB9001_KEYS.KEY_BANK_A;
        case 'g':
            return SB9001_KEYS.KEY_E | SB9001_KEYS.KEY_BANK_A;
        case 'h':
            return SB9001_KEYS.KEY_F | SB9001_KEYS.KEY_BANK_A;

        case 'q':
            return SB9001_KEYS.KEY_A | SB9001_KEYS.KEY_BANK_B;
        case 'w':
            return SB9001_KEYS.KEY_B | SB9001_KEYS.KEY_BANK_B;
        case 'e':
            return SB9001_KEYS.KEY_C | SB9001_KEYS.KEY_BANK_B;
        case 'r':
            return SB9001_KEYS.KEY_D | SB9001_KEYS.KEY_BANK_B;
        case 't':
            return SB9001_KEYS.KEY_E | SB9001_KEYS.KEY_BANK_B;
        case 'y':
            return SB9001_KEYS.KEY_F | SB9001_KEYS.KEY_BANK_B;
    }

    return 0;
}

static void process_key(int keychar, struct SampleBank* bank) {
    // TESTING ONLY: final version will take input from USB
    uint16_t keys = keycodes_from_keyboard_key(keychar);

    uint16_t index = bank_array_index_from_keycodes(keys);

    if (bank->array[index] != NULL) {
        queue_playback(bank->array[index]->attack);
    }

}

void input_thread_loop(struct SampleBank* bank) {
    SB9001_KEYS = (struct KeyEnum){
            0b00000000000001,
            0b00000000000010,
            0b00000000000100,
            0b00000000001000,
            0b00000000010000,
            0b00000000100000,
            0b00000001000000,
            0b00000010000000,
            0b00000100000000,
            0b00001000000000,
            0b00010000000000,
            0b00100000000000,
            0b01000000000000,
            0b10000000000000
    };
    int c;

    system("/bin/stty raw");

    while ((c = getchar()) != EOF) {
        process_key(c, bank);
    }

    system("/bin/stty cooked");
}

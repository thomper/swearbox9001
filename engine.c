#include "engine.h"
#include "input.h"
#include "playback.h"
#include "reader.h"

#include <assert.h>


static struct SampleBank bank;

struct HeldKeyState {
    int bank_b_held;
    int bank_c_held;
    enum SB9001ActionKey action_held;
};

static void update_held_keys(struct HeldKeyState* held_keys, struct SB9001Event* sb_event) {
    int bank_key_pressed_or_released = sb_event->bank_key != SB9001_BANK_KEY_NONE;
    if (bank_key_pressed_or_released) {
        if (sb_event->bank_key_depressed) {
            if (sb_event->bank_key == SB9001_BANK_KEY_B) {
                held_keys->bank_b_held = 1;
            } else {
                held_keys->bank_c_held = 1;
            }
        } else {  // bank key released
            if (sb_event->bank_key == SB9001_BANK_KEY_B) {
                held_keys->bank_b_held = 0;
            } else {
                held_keys->bank_c_held = 0;
            }
        }
    }

    int action_key_pressed_or_released = sb_event->action_key != SB9001_ACTION_KEY_NONE;
    if (!action_key_pressed_or_released) {
        return;
    }

    int key_just_pressed_and_no_key_was_held = sb_event->action_key_depressed
            && held_keys->action_held == SB9001_ACTION_KEY_NONE;
    if (key_just_pressed_and_no_key_was_held) {
        held_keys->action_held = sb_event->action_key;
    } else if (!sb_event->action_key_depressed
            && sb_event->action_key == held_keys->action_held) {
        held_keys->action_held = SB9001_ACTION_KEY_NONE;
    }
}

static uint16_t bank_addend_from_key_state(struct HeldKeyState* held_keys, struct SB9001Event* sb_event) {
    uint16_t addend = 0;

    if (held_keys->bank_b_held) {
        addend += 6;
    }

    if (held_keys->bank_c_held) {
        addend += 12;
    }

    if (sb_event->bank_key != SB9001_BANK_KEY_NONE && sb_event->bank_key_depressed) {
        if (sb_event->bank_key == SB9001_BANK_KEY_B) {
            addend += 6;
        } else {  // bank key c
            addend += 12;
        }
    }

    return addend;
}

static uint16_t action_addend_from_key_state(struct HeldKeyState* held_keys, struct SB9001Event* sb_event) {
    if (held_keys->action_held != SB9001_ACTION_KEY_NONE) {
        switch (held_keys->action_held) {
            case SB9001_ACTION_KEY_A: return 0;
            case SB9001_ACTION_KEY_B: return 1;
            case SB9001_ACTION_KEY_C: return 2;
            case SB9001_ACTION_KEY_D: return 3;
            case SB9001_ACTION_KEY_E: return 4;
            case SB9001_ACTION_KEY_F: return 5;
            default: ;
        }
    }

    if (sb_event->action_key != SB9001_ACTION_KEY_NONE && sb_event->action_key_depressed) {
        switch (sb_event->action_key) {
            case SB9001_ACTION_KEY_A: return 0;
            case SB9001_ACTION_KEY_B: return 1;
            case SB9001_ACTION_KEY_C: return 2;
            case SB9001_ACTION_KEY_D: return 3;
            case SB9001_ACTION_KEY_E: return 4;
            case SB9001_ACTION_KEY_F: return 5;
            default: ;
        }
    }

    assert(0);  // Should not be reachable or this function should not have been called.
}

static uint16_t bank_index_from_key_state(struct HeldKeyState* held_keys, struct SB9001Event* sb_event) {
    assert(sb_event->action_key != SB9001_ACTION_KEY_NONE);

    return action_addend_from_key_state(held_keys, sb_event)
        + bank_addend_from_key_state(held_keys, sb_event);
}


static void initialise_samples(void) {
    for (uint16_t i = 0; i < BANK_SIZE; i++) {
        bank.filename_stubs[i] = NULL;
    }
    bank.filename_stubs[0] = "song_f";
    bank.filename_stubs[1] = "test_f";

    load_sample_bank(&bank);
}

void initialise(void) {
    initialise_joystick();
    initialise_playback();
    initialise_samples();
}

void tear_down(void) {
    free_sample_bank_array(&bank);
    tear_down_playback();
}

int main_loop() {
    SDL_Event event;
    struct SB9001Event sb_event;
    const uint16_t invalid_bank_index = BANK_SIZE;
    uint16_t current_playing_bank_index = invalid_bank_index;

    struct HeldKeyState held_keys = {
            0,                      // bank_b_held
            0,                      // bank_c_held
            SB9001_ACTION_KEY_NONE  // action_held
    };

    while (1) {
        if (SDL_PollEvent(&event) == 0) {
            continue;
        }

        sb_event = sb_event_from_sdl_event(&event);

        if (is_silent()) {
            int action_key_just_pressed = sb_event.action_key != SB9001_ACTION_KEY_NONE && sb_event.action_key_depressed;
            if (action_key_just_pressed) {
                current_playing_bank_index = bank_index_from_key_state(&held_keys, &sb_event);
                play_if_silent(bank.array[current_playing_bank_index]);
            }
        } else {  // a sound is currently playing
            int held_action_key_just_released = held_keys.action_held == sb_event.action_key
                && held_keys.action_held != SB9001_ACTION_KEY_NONE
                && !sb_event.action_key_depressed;
            if (held_action_key_just_released) {
                int released_key_matches_playing_sample = bank_index_from_key_state(&held_keys, &sb_event) == current_playing_bank_index;
                if (released_key_matches_playing_sample) {
                    release_sample();
                    current_playing_bank_index = invalid_bank_index;
                }
            }
        }

        update_held_keys(&held_keys, &sb_event);

        //TODO: Testing only, should never quit
        if (event.type == SDL_JOYBUTTONDOWN) {
            if (event.jbutton.button == 8) {
                break;
            }
        }
    }
}

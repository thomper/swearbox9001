#include "engine.h"
#include "input.h"
#include "playback.h"
#include "reader.h"

#include <SDL2/SDL.h>

#include <assert.h>

#define PITCH_REPEAT_MS 50


static struct SampleBank bank;

struct HeldKeyState {
    int bank_b_held;
    int bank_c_held;
    enum SB9001ActionKey action_held;

    // These hold the point in time the motion was input, need to find a better name.
    uint64_t point_up_engaged;
    uint64_t point_down_engaged;
    uint64_t point_left_engaged;
    uint64_t point_right_engaged;
};

void handle_pitch_change_repeat(struct HeldKeyState* state) {
    if (state->point_up_engaged == 0 && state->point_down_engaged == 0) {
        return;
    }

    uint64_t now = SDL_GetPerformanceCounter();
    uint64_t before = state->point_up_engaged != 0 ? state->point_up_engaged : state->point_down_engaged;

    double milliseconds_since_change = (now - before) * 1000 / (double)SDL_GetPerformanceFrequency();
    if (milliseconds_since_change >= PITCH_REPEAT_MS) {
        if (state->point_up_engaged != 0) {
            increase_pitch();
            state->point_up_engaged = now;
        } else {
            decrease_pitch();
            state->point_down_engaged = now;
        }
    }
}

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

    if (sb_event->vert_motion == SB9001_VERT_MOTION_CENTRE) {
        held_keys->point_up_engaged = held_keys->point_down_engaged = 0;
    } else if (sb_event->vert_motion == SB9001_VERT_MOTION_UP) {
        held_keys->point_up_engaged = SDL_GetPerformanceCounter();
        held_keys->point_down_engaged = 0;
    } else if (sb_event->vert_motion == SB9001_VERT_MOTION_DOWN) {
        held_keys->point_down_engaged = SDL_GetPerformanceCounter();
        held_keys->point_up_engaged = 0;
    }

    if (sb_event->hori_motion == SB9001_HORI_MOTION_CENTRE) {
        held_keys->point_left_engaged = held_keys->point_right_engaged = 0;
    } else if (sb_event->hori_motion == SB9001_HORI_MOTION_LEFT) {
        held_keys->point_left_engaged = SDL_GetPerformanceCounter();
        held_keys->point_right_engaged = 0;
    } else if (sb_event->hori_motion == SB9001_HORI_MOTION_RIGHT) {
        held_keys->point_right_engaged = SDL_GetPerformanceCounter();
        held_keys->point_left_engaged = 0;
    }

    int action_key_pressed_or_released = sb_event->action_key != SB9001_ACTION_KEY_NONE;
    if (!action_key_pressed_or_released) {
        return;
    }

    int action_key_just_pressed_and_no_key_was_held = sb_event->action_key_depressed
            && held_keys->action_held == SB9001_ACTION_KEY_NONE;
    if (action_key_just_pressed_and_no_key_was_held) {
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

    if (load_sample_bank(&bank) != 0) {
        fprintf(stderr, "Error loading samples, exiting.\n");
        exit(EXIT_FAILURE);
    }
}

void initialise(void) {
    if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "Error initialising SDL, exiting.\n");
    };

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
            0,                       // bank_b_held
            0,                       // bank_c_held
            SB9001_ACTION_KEY_NONE,  // action_held

            0,                       // point_up_engaged;
            0,                       // point_down_engaged;
            0,                       // point_left_engaged;
            0                        // point_right_engaged;
    };

    while (1) {
        if (SDL_PollEvent(&event) == 0) {
            sb_event = EMPTY_SB_EVENT;
        } else {
            sb_event = sb_event_from_sdl_event(&event);
        }

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

        handle_pitch_change_repeat(&held_keys);
        if (sb_event.vert_motion == SB9001_VERT_MOTION_UP) {
            increase_pitch();
        } else if (sb_event.vert_motion == SB9001_VERT_MOTION_DOWN) {
            decrease_pitch();
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

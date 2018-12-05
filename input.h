#ifndef SWEARBOX9001_INPUT_H
#define SWEARBOX9001_INPUT_H

#include <SDL2/SDL_events.h>


enum SB9001BankKey {
    SB9001_BANK_KEY_NONE,
    SB9001_BANK_KEY_B,
    SB9001_BANK_KEY_C,
};


enum SB9001ActionKey {
    SB9001_ACTION_KEY_NONE,
    SB9001_ACTION_KEY_A,
    SB9001_ACTION_KEY_B,
    SB9001_ACTION_KEY_C,
    SB9001_ACTION_KEY_D,
    SB9001_ACTION_KEY_E,
    SB9001_ACTION_KEY_F
};

enum SB9001HoriMotion {
    SB9001_HORI_MOTION_NONE,
    SB9001_HORI_MOTION_LEFT,
    SB9001_HORI_MOTION_CENTRE,
    SB9001_HORI_MOTION_RIGHT
};

enum SB9001VertMotion {
    SB9001_VERT_MOTION_NONE,
    SB9001_VERT_MOTION_UP,
    SB9001_VERT_MOTION_CENTRE,
    SB9001_VERT_MOTION_DOWN
};

struct SB9001Event {
    enum SB9001HoriMotion hori_motion;
    enum SB9001VertMotion vert_motion;

    enum SB9001BankKey bank_key;
    int bank_key_depressed;

    enum SB9001ActionKey action_key;
    int action_key_depressed;
};

extern const struct SB9001Event EMPTY_SB_EVENT;

struct SB9001Event sb_event_from_sdl_event(SDL_Event* event);

void initialise_joystick(void);


#endif //SWEARBOX9001_INPUT_H

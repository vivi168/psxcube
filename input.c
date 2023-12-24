#include "stdafx.h"

typedef struct input_manager_t {
    struct pad_type_t {
        unsigned char stat;
        unsigned char len : 4;
        unsigned char type : 4;
        unsigned short btn;
        unsigned char rs_x, rs_y;
        unsigned char ls_x, ls_y;
    }* pad;
    char padbuff[2][34];

    unsigned int new_keystate, old_keystate;
    unsigned int keys_pressed, keys_held, keys_released;
} InputManager;

static InputManager input_manager;

void pad_update()
{
    // Parse controller input
    input_manager.pad = (struct pad_type_t*)input_manager.padbuff[0];

    // Only parse inputs when a controller is connected
    if (input_manager.pad->stat == 0) {
        // Only parse when a digital pad,
        // dual-analog and dual-shock is connected
        if ((input_manager.pad->type == 0x4) || (input_manager.pad->type == 0x5) || (input_manager.pad->type == 0x7)) {
            input_manager.old_keystate = input_manager.new_keystate;
            input_manager.new_keystate = input_manager.pad->btn ^ 0xffff;

            input_manager.keys_pressed = (input_manager.old_keystate ^ input_manager.new_keystate) & input_manager.new_keystate;
            input_manager.keys_held = input_manager.old_keystate & input_manager.new_keystate;
            // TODO: bug
            input_manager.keys_released = input_manager.old_keystate ^ input_manager.new_keystate;
        }
    }
}

void pad_init()
{
    printf("[INFO]: pad init\n");
    InitPAD(input_manager.padbuff[0], 34, input_manager.padbuff[1], 34);

    input_manager.padbuff[0][0] = input_manager.padbuff[0][1] = 0xff;
    input_manager.padbuff[1][0] = input_manager.padbuff[1][1] = 0xff;

    StartPAD();
    ChangeClearPAD(1);

    input_manager.new_keystate = 0;
    input_manager.old_keystate = 0;

    input_manager.keys_held = 0;
    input_manager.keys_pressed = 0;
    input_manager.keys_released = 0;
}

void pad_pollEvents()
{
    pad_update();
}

int pad_isHeld(int k)
{
    return (1 << k) & input_manager.keys_held;
}

int pad_isPressed(int k)
{
    return (1 << k) & input_manager.keys_pressed;
}

int pad_isReleased(int k)
{
    return (1 << k) & input_manager.keys_released;
}

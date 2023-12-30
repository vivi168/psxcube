#ifndef INPUT_H
#define INPUT_H

#define KEY_UP       4
#define KEY_RIGHT    5
#define KEY_DOWN     6
#define KEY_LEFT     7
#define KEY_L2       8
#define KEY_R2       9
#define KEY_L1       10
#define KEY_R1       11
#define KEY_TRIANGLE 12
#define KEY_CIRCLE   13
#define KEY_CROSS    14
#define KEY_SQUARE   15

void pad_init();
void pad_pollEvents();
int  pad_isHeld(int);
int  pad_isPressed(int);
int  pad_isReleased(int); // TODO bug

#endif

#ifndef CAMERA_H
#define CAMERA_H

#define CAM_ROT_SPEED 24
#define CAM_MOV_SCALE 6

typedef struct camera_t {
    VECTOR translate;
    SVECTOR rotation;
    MATRIX matrix;
} Camera;

void cam_processInput(Camera* cam);
void cam_setTranslation(Camera* cam, int, int, int);

#endif

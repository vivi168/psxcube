#ifndef CAMERA_H
#define CAMERA_H

typedef struct camera_t {
    VECTOR pos;
    VECTOR rot;

    VECTOR tpos;
    SVECTOR trot;
    MATRIX matrix;
} Camera;

void Cam_Update(Camera* cam);
void Cam_SetPos(Camera* cam, int, int, int);

#endif

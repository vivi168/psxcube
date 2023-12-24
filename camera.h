#ifndef CAMERA_H
#define CAMERA_H

#define CAM_ROT_SPEED 24
#define CAM_MOV_SCALE 6

typedef struct camera_t {
    VECTOR translate;
    SVECTOR rotation;
    MATRIX matrix;
} Camera;

void Cam_Update(Camera* cam);
void Cam_setTranslation(Camera* cam, int, int, int);

#endif

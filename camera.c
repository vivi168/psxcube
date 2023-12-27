#include "stdafx.h"

// TODO: tracking / fps mode ?
static void cam_update(Camera* cam)
{
    VECTOR translate;
    RotMatrix_gte(&cam->rotation, &cam->matrix);

    translate.vx = -cam->translate.vx;
    translate.vy = -cam->translate.vy;
    translate.vz = -cam->translate.vz;

    ApplyMatrixLV(&cam->matrix, &translate, &translate);
    TransMatrix(&cam->matrix, &translate);
}

void cam_setTranslation(Camera* cam, int x, int y, int z)
{
    setVector(&cam->translate, x, y, z);
}

void cam_processInput(Camera* cam)
{
    int dx = 0;
    int dy = 0;
    int dz = 0;

    if (pad_isHeld(KEY_UP)) {
        cam->rotation.vx -= CAM_ROT_SPEED;
    }
    if (pad_isHeld(KEY_DOWN)) {
        cam->rotation.vx += CAM_ROT_SPEED;
    }
    if (pad_isHeld(KEY_LEFT)) {
        cam->rotation.vy += CAM_ROT_SPEED;
    }
    if (pad_isHeld(KEY_RIGHT)) {
        cam->rotation.vy -= CAM_ROT_SPEED;
    }

    // Don't look too far up or down
    if (cam->rotation.vx > 768)
        cam->rotation.vx = 768;
    else if (cam->rotation.vx < -768)
        cam->rotation.vx = -768;

    if (pad_isHeld(KEY_TRIANGLE)) {
        dx -= FixedMulFixed(iSin(cam->rotation.vy), iCos(cam->rotation.vx)) << CAM_MOV_SCALE;
        dz += FixedMulFixed(iCos(cam->rotation.vy), iCos(cam->rotation.vx)) << CAM_MOV_SCALE;
    }
    if (pad_isHeld(KEY_CROSS)) {
        dx += FixedMulFixed(iSin(cam->rotation.vy), iCos(cam->rotation.vx)) << CAM_MOV_SCALE;
        dz -= FixedMulFixed(iCos(cam->rotation.vy), iCos(cam->rotation.vx)) << CAM_MOV_SCALE;
    }
    if (pad_isHeld(KEY_SQUARE)) {
        dx -= iCos(cam->rotation.vy) << CAM_MOV_SCALE;
        dz -= iSin(cam->rotation.vy) << CAM_MOV_SCALE;
    }
    if (pad_isHeld(KEY_CIRCLE)) {
        dx += iCos(cam->rotation.vy) << CAM_MOV_SCALE;
        dz += iSin(cam->rotation.vy) << CAM_MOV_SCALE;
    }
    if (pad_isHeld(KEY_R1)) {
        dy -= iCos(cam->rotation.vx) << CAM_MOV_SCALE;
    }
    if (pad_isHeld(KEY_R2)) {
        dy += iCos(cam->rotation.vx) << CAM_MOV_SCALE;
    }

    cam->translate.vx += FixedToInt(dx);
    cam->translate.vy += FixedToInt(dy);
    cam->translate.vz += FixedToInt(dz);

    cam_update(cam);
}

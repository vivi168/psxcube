#include "stdafx.h"

// TODO: tracking / fps mode ?
void cam_update(Camera* cam)
{
    VECTOR translate;
    RotMatrix_gte(&cam->rotation, &cam->matrix);

    // Divide out the fractions of camera coordinates and invert
    // the sign, so camera coordinates will line up to world
    // (or geometry) coordinates
    translate.vx = FixedToInt(-cam->translate.vx);
    translate.vy = FixedToInt(-cam->translate.vy);
    translate.vz = FixedToInt(-cam->translate.vz);

    // Apply rotation of matrix to translation value to achieve a
    // first person perspective
    // m_in * v_in = v_out
    ApplyMatrixLV(&cam->matrix, &translate, &translate);
    TransMatrix(&cam->matrix, &translate);
}

void cam_setTranslation(Camera* cam, int x, int y, int z)
{
    setVector(&cam->translate, x * ONE, y * ONE, z * ONE);
}

void cam_processInput(Camera* cam)
{
    if (iptm_is_held(KEY_UP)) {
        cam->rotation.vx -= CAM_ROT_SPEED;
    }
    if (iptm_is_held(KEY_DOWN)) {
        cam->rotation.vx += CAM_ROT_SPEED;
    }
    if (iptm_is_held(KEY_LEFT)) {
        cam->rotation.vy += CAM_ROT_SPEED;
    }
    if (iptm_is_held(KEY_RIGHT)) {
        cam->rotation.vy -= CAM_ROT_SPEED;
    }
    if (iptm_is_held(KEY_TRIANGLE)) {
        cam->translate.vx -= FixedMulFixed(iSin(cam->rotation.vy), iCos(cam->rotation.vx)) << CAM_MOV_SCALE;
        cam->translate.vy += iSin(cam->rotation.vx) << CAM_MOV_SCALE;
        cam->translate.vz += FixedMulFixed(iCos(cam->rotation.vy), iCos(cam->rotation.vx)) << CAM_MOV_SCALE;
    }
    if (iptm_is_held(KEY_CROSS)) {
        cam->translate.vx += FixedMulFixed(iSin(cam->rotation.vy), iCos(cam->rotation.vx)) << CAM_MOV_SCALE;
        cam->translate.vy -= iSin(cam->rotation.vx) << CAM_MOV_SCALE;
        cam->translate.vz -= FixedMulFixed(iCos(cam->rotation.vy), iCos(cam->rotation.vx)) << CAM_MOV_SCALE;
    }
    if (iptm_is_held(KEY_SQUARE)) {
        cam->translate.vx -= iCos(cam->rotation.vy) << CAM_MOV_SCALE;
        cam->translate.vz -= iSin(cam->rotation.vy) << CAM_MOV_SCALE;
    }
    if (iptm_is_held(KEY_CIRCLE)) {
        cam->translate.vx += iCos(cam->rotation.vy) << CAM_MOV_SCALE;
        cam->translate.vz += iSin(cam->rotation.vy) << CAM_MOV_SCALE;
    }

    cam_update(cam);
}

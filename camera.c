#include "stdafx.h"

static void cam_update(Camera* cam);
static void cam_update2(Camera* cam);
static void cam_lookAt(const VECTOR* eye, const VECTOR* center,
                       const SVECTOR* up, MATRIX* mat);

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
        dx -= FixedMulFixed(iSin(cam->rotation.vy), iCos(cam->rotation.vx))
              << CAM_MOV_SCALE;
        dz += FixedMulFixed(iCos(cam->rotation.vy), iCos(cam->rotation.vx))
              << CAM_MOV_SCALE;
    }
    if (pad_isHeld(KEY_CROSS)) {
        dx += FixedMulFixed(iSin(cam->rotation.vy), iCos(cam->rotation.vx))
              << CAM_MOV_SCALE;
        dz -= FixedMulFixed(iCos(cam->rotation.vy), iCos(cam->rotation.vx))
              << CAM_MOV_SCALE;
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

void cam_init(Camera* cam)
{
    cam->yaw = M_PI / 2;
    cam->pitch = 0;
    cam->speed = CAM_MOV_SCALE;
    cam->sensitivity = CAM_ROT_SPEED;
    cam->zoom = M_PI;

    setVector(&cam->world_up, 0, -ONE, 0);
}

void cam_processInput2(Camera* cam)
{
    if (pad_isHeld(KEY_UP)) {
        cam->pitch -= cam->sensitivity;
    }
    if (pad_isHeld(KEY_DOWN)) {
        cam->pitch += cam->sensitivity;
    }
    if (pad_isHeld(KEY_LEFT)) {
        cam->yaw += cam->sensitivity;
    }
    if (pad_isHeld(KEY_RIGHT)) {
        cam->yaw -= cam->sensitivity;
    }

    if (cam->pitch > 768)
        cam->pitch = 768;
    else if (cam->pitch < -768)
        cam->pitch = -768;

    if (pad_isHeld(KEY_TRIANGLE)) {
        cam->translate.vx -= cam->forward.vx >> cam->speed;
        cam->translate.vz -= cam->forward.vz >> cam->speed;
    }

    if (pad_isHeld(KEY_CROSS)) {
        cam->translate.vx += cam->forward.vx >> cam->speed;
        cam->translate.vz += cam->forward.vz >> cam->speed;
    }

    if (pad_isHeld(KEY_SQUARE)) {
        cam->translate.vx -= cam->right.vx >> cam->speed;
        cam->translate.vz -= cam->right.vz >> cam->speed;
    }

    if (pad_isHeld(KEY_CIRCLE)) {
        cam->translate.vx += cam->right.vx >> cam->speed;
        cam->translate.vz += cam->right.vz >> cam->speed;
    }

    if (pad_isHeld(KEY_R1)) {
        cam->translate.vy -= ONE >> cam->speed;
    }
    if (pad_isHeld(KEY_R2)) {
        cam->translate.vy += ONE >> cam->speed;
    }

    cam_update2(cam);
    VECTOR pf;
    setVector(&pf,
              cam->translate.vx + cam->front.vx,
              cam->translate.vy + cam->front.vy,
              cam->translate.vz + cam->front.vz);
    cam_lookAt(&cam->translate, &pf, &cam->up, &cam->matrix);
}

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

static void cam_update2(Camera* cam)
{
    VECTOR f;

    f.vx = FixedMulFixed(iCos(cam->yaw), iCos(cam->pitch));
    f.vy = iSin(cam->pitch);
    f.vz = FixedMulFixed(iSin(cam->yaw), iCos(cam->pitch));

    // front = glm::normalize(f);
    VectorNormalS(&f, &cam->front);

    // right = glm::normalize(glm::cross(front, world_up));
    crossProduct(&cam->front, &cam->world_up, &f);
    VectorNormalS(&f, &cam->right);

    // forward = glm::normalize(glm::cross(right, world_up));
    crossProduct(&cam->right, &cam->world_up, &f);
    VectorNormalS(&f, &cam->forward);

    // up = glm::normalize(glm::cross(right, front));
    crossProduct(&cam->right, &cam->front, &f);
    VectorNormalS(&f, &cam->up);
}

static void cam_lookAt(const VECTOR* eye, const VECTOR* center,
                       const SVECTOR* up, MATRIX* mat)
{
    VECTOR  tmp, pos;
    SVECTOR zaxis, xaxis, yaxis;

    // tmp = center - eye
    setVector(&tmp,
              center->vx - eye->vx,
              center->vy - eye->vy,
              center->vz - eye->vz);

    // zaxis = normal(center - eye)
    VectorNormalS(&tmp, &zaxis);

    crossProduct(&zaxis, up, &tmp);
    // xasis = normal(cross(up, zaxis))
    VectorNormalS(&tmp, &xaxis);

    crossProduct(&zaxis, &xaxis, &tmp);
    // yaxis = normal(cross(zaxis, xasis))
    VectorNormalS(&tmp, &yaxis);

    mat->m[0][0] = xaxis.vx;
    mat->m[1][0] = yaxis.vx;
    mat->m[2][0] = zaxis.vx;
    mat->m[0][1] = xaxis.vy;
    mat->m[1][1] = yaxis.vy;
    mat->m[2][1] = zaxis.vy;
    mat->m[0][2] = xaxis.vz;
    mat->m[1][2] = yaxis.vz;
    mat->m[2][2] = zaxis.vz;

    pos.vx = -eye->vx;
    pos.vy = -eye->vy;
    pos.vz = -eye->vz;

    ApplyMatrixLV(mat, &pos, &tmp);
    TransMatrix(mat, &tmp);
}

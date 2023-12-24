#include "stdafx.h"

// TODO: tracking / fps mode ?
void Cam_Update(Camera* cam)
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

void Cam_setTranslation(Camera* cam, int x, int y, int z)
{
    setVector(&cam->translate, x * ONE, y * ONE, z * ONE);
}

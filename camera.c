#include "stdafx.h"

// TODO: tracking / fps mode ?
void Cam_Update(Camera* cam)
{
        // Set rotation to the matrix
        RotMatrix_gte(&cam->trot, &cam->matrix);

        // Divide out the fractions of camera coordinates and invert
        // the sign, so camera coordinates will line up to world
        // (or geometry) coordinates
        cam->tpos.vx = FixedToInt(-cam->pos.vx);
        cam->tpos.vy = FixedToInt(-cam->pos.vy);
        cam->tpos.vz = FixedToInt(-cam->pos.vz);

        // Apply rotation of matrix to translation value to achieve a
        // first person perspective
        // m_in * v_in = v_out
        ApplyMatrixLV(&cam->matrix, &cam->tpos, &cam->tpos);

        // Set translation matrix
        TransMatrix(&cam->matrix, &cam->tpos);
}

void Cam_SetPos(Camera* cam, int x, int y, int z)
{
    setVector(&cam->pos, x * ONE, y * ONE, z * ONE);
}

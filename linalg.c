#include "stdafx.h"

void quat_normalize(quat q)
{
    FLOAT mag = SquareRoot12((q[X] * q[X]) + (q[Y] * q[Y]) + (q[Z] * q[Z]) + (q[W] * q[W]));

    if (mag > 0)
    {
        FLOAT one_over_mag = ONE / mag;

        q[X] *= one_over_mag;
        q[Y] *= one_over_mag;
        q[Z] *= one_over_mag;
        q[W] *= one_over_mag;
    }
}

void quat_mulvec(const quat q, const vec3 v, quat out)
{
    out[W] = -(q[X] * v[X]) - (q[Y] * v[Y]) - (q[Z] * v[Z]);
    out[X] =  (q[W] * v[X]) + (q[Y] * v[Z]) - (q[Z] * v[Y]);
    out[Y] =  (q[W] * v[Y]) + (q[Z] * v[X]) - (q[X] * v[Z]);
    out[Z] =  (q[W] * v[Z]) + (q[X] * v[Y]) - (q[Y] * v[X]);
}

void quat_mulquat(const quat qa, const quat qb, quat out)
{
    out[W] = (qa[W] * qb[W]) - (qa[X] * qb[X]) - (qa[Y] * qb[Y]) - (qa[Z] * qb[Z]);
    out[X] = (qa[X] * qb[W]) + (qa[W] * qb[X]) + (qa[Y] * qb[Z]) - (qa[Z] * qb[Y]);
    out[Y] = (qa[Y] * qb[W]) + (qa[W] * qb[Y]) + (qa[Z] * qb[X]) - (qa[X] * qb[Z]);
    out[Z] = (qa[Z] * qb[W]) + (qa[W] * qb[Z]) + (qa[X] * qb[Y]) - (qa[Y] * qb[X]);
}

void quat_rotate_point(const quat q, const vec3 in, vec3 out)
{
    quat tmp, inv, qout;

    inv[X] = -q[X]; inv[Y] = -q[Y];
    inv[Z] = -q[Z]; inv[W] = q[W];

    quat_normalize(inv);

    quat_mulvec(q, in, tmp);
    quat_mulquat(tmp, inv, qout);

    out[X] = qout[X];
    out[Y] = qout[Y];
    out[Z] = qout[Z];
}

#include "stdafx.h"

void print_vec3(vec3 v)
{
    printf("vec3: %d %d %d\n", v[X], v[Y], v[Z]);
}

void print_quat(quat q)
{
    printf("quat: %d %d %d %d\n", q[X], q[Y], q[Z], q[W]);
}

void quat_normalize(quat q)
{
    int x = ((q[X] * q[X]) + (q[Y] * q[Y]) + (q[Z] * q[Z]) + (q[W] * q[W])) >> SCALE;
    FLOAT mag = SquareRoot12(x);

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
    out[W] = -FixedMulFixed(q[X], v[X]) - FixedMulFixed(q[Y], v[Y]) - FixedMulFixed(q[Z], v[Z]);
    out[X] =  FixedMulFixed(q[W], v[X]) + FixedMulFixed(q[Y], v[Z]) - FixedMulFixed(q[Z], v[Y]);
    out[Y] =  FixedMulFixed(q[W], v[Y]) + FixedMulFixed(q[Z], v[X]) - FixedMulFixed(q[X], v[Z]);
    out[Z] =  FixedMulFixed(q[W], v[Z]) + FixedMulFixed(q[X], v[Y]) - FixedMulFixed(q[Y], v[X]);
}

void quat_mulquat(const quat qa, const quat qb, quat out)
{
    out[W] = FixedMulFixed(qa[W], qb[W]) - FixedMulFixed(qa[X], qb[X]) - FixedMulFixed(qa[Y], qb[Y]) - FixedMulFixed(qa[Z], qb[Z]);
    out[X] = FixedMulFixed(qa[X], qb[W]) + FixedMulFixed(qa[W], qb[X]) + FixedMulFixed(qa[Y], qb[Z]) - FixedMulFixed(qa[Z], qb[Y]);
    out[Y] = FixedMulFixed(qa[Y], qb[W]) + FixedMulFixed(qa[W], qb[Y]) + FixedMulFixed(qa[Z], qb[X]) - FixedMulFixed(qa[X], qb[Z]);
    out[Z] = FixedMulFixed(qa[Z], qb[W]) + FixedMulFixed(qa[W], qb[Z]) + FixedMulFixed(qa[X], qb[Y]) - FixedMulFixed(qa[Y], qb[X]);
}

void quat_rotate_point(const quat q, const vec3 in, vec3 out)
{
    quat tmp, inv, qout;

    inv[X] = -q[X]; inv[Y] = -q[Y];
    inv[Z] = -q[Z]; inv[W] = q[W];

    quat_normalize(inv);

    quat_mulvec(q, in, tmp);
    quat_mulquat(tmp, inv, qout);
    // print_quat(qout);

    out[X] = qout[X];
    out[Y] = qout[Y];
    out[Z] = qout[Z];
}

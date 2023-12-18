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
    int mag = SquareRoot12(x);

    if (mag > 0)
    {
        int one_over_mag = ONE / mag;

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

// trigonometry

#define qN	10
#define qA	12
#define B	19900
#define	C	3516

int iSin(int x)
{
    int c, x2, y;

    c= x<<(30-qN);              // Semi-circle info into carry.
    x -= 1<<qN;                 // sine -> cosine calc

    x= x<<(31-qN);              // Mask with PI
    x= x>>(31-qN);              // Note: SIGNED shift! (to qN)

    x= x*x>>(2*qN-14);          // x=x^2 To Q14

    y= B - (x*C>>14);           // B - x^2*C
    y= (1<<qA)-(x*y>>16);       // A - x^2*(B-x^2*C)

    return c>=0 ? y : -y;
}


// clip

#define CLIP_LEFT   1
#define CLIP_RIGHT  2
#define CLIP_TOP    4
#define CLIP_BOTTOM 8

int test_clip(RECT *clip, short x, short y)
{
    // Tests which corners of the screen a point lies outside of

    int result = 0;

    if (x < clip->x)
        result |= CLIP_LEFT;

    if (x >= (clip->x + (clip->w - 1)))
        result |= CLIP_RIGHT;

    if (y < clip->y)
        result |= CLIP_TOP;

    if (y >= (clip->y + (clip->h - 1)))
        result |= CLIP_BOTTOM;

    return result;
}

int tri_clip(RECT *clip, DVECTOR *v0, DVECTOR *v1, DVECTOR *v2)
{
    // Returns non-zero if a triangle is outside the screen boundaries

    short c[3];

    c[0] = test_clip(clip, v0->vx, v0->vy);
    c[1] = test_clip(clip, v1->vx, v1->vy);
    c[2] = test_clip(clip, v2->vx, v2->vy);

    if ((c[0] & c[1]) == 0)
        return 0;
    if ((c[1] & c[2]) == 0)
        return 0;
    if ((c[2] & c[0]) == 0)
        return 0;

    return 1;
}

int quad_clip(RECT *clip, DVECTOR *v0, DVECTOR *v1, DVECTOR *v2, DVECTOR *v3)
{
    // Returns non-zero if a quad is outside the screen boundaries

    short c[4];

    c[0] = test_clip(clip, v0->vx, v0->vy);
    c[1] = test_clip(clip, v1->vx, v1->vy);
    c[2] = test_clip(clip, v2->vx, v2->vy);
    c[3] = test_clip(clip, v3->vx, v3->vy);

    if ((c[0] & c[1]) == 0)
        return 0;
    if ((c[1] & c[2]) == 0)
        return 0;
    if ((c[2] & c[3]) == 0)
        return 0;
    if ((c[3] & c[0]) == 0)
        return 0;
    if ((c[0] & c[2]) == 0)
        return 0;
    if ((c[1] & c[3]) == 0)
        return 0;

    return 1;
}

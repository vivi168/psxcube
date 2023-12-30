#ifndef LINALG_H
#define LINALG_H

#define SCALE 12
#ifndef ONE
#define ONE (1 << SCALE)
#endif
#define HALF (ONE >> 1)

#define IntToFixed(x) ((x) << SCALE)
#define FixedToInt(x) ((x) >> SCALE)

#define FixedMulInt(x, y)   ((x) * (y))
#define FixedMulFixed(x, y) (((x) * (y)) >> SCALE)

#define FixedDivInt(x, y)   ((x) / (y))
#define FixedDivFixed(x, y) (((x) << SCALE) / (y))

enum { X = 0, Y, Z, W };

// TODO: define type to notify that a variable is actually fixed point?

typedef int vec2[2];
typedef int vec3[3];
typedef int quat[4];

void quat_rotate_point(const quat q, const vec3 in, vec3 out);

// trigonometry
int iSin(int x);
#define iCos(x) (iSin(x + 1024))

// clip

int tri_clip(RECT* clip, DVECTOR* v0, DVECTOR* v1, DVECTOR* v2);
int quad_clip(RECT* clip, DVECTOR* v0, DVECTOR* v1, DVECTOR* v2, DVECTOR* v3);

// normals

void surfaceNormal(SVECTOR* v1, SVECTOR* v2, SVECTOR* v3, SVECTOR* out);
void centroid(SVECTOR* v1, SVECTOR* v2, SVECTOR* v3, SVECTOR* out);

#endif

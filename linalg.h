#ifndef LINALG_H
#define LINALG_H

#define SCALE 12
#define ONE (1 << SCALE)

#define FloatToFixed(x) ((int16_t)((x) * (float)(1 << SCALE)))
#define FixedToFloat(x) ((float)(x) / (float)(1 << SCALE))

#define IntToFixed(x) ((x) << SCALE)
#define FixedToInt(x) ((x) >> SCALE)

#define FixedMulInt(x,y) ((x) * (y))
#define FixedMulFixed(x,y) (((x) * (y)) >> SCALE)

#define FixedDivInt(x,y) ((x) / (y))
#define FixedDivFixed(x,y) (((x) << SCALE) / (y))

enum {
    X = 0, Y, Z, W
};

typedef int FLOAT;
typedef int DOUBLE;

typedef FLOAT vec2[2];
typedef FLOAT vec3[3];
typedef FLOAT quat[4];

#endif

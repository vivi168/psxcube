#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ps1/gpucmd.h"
#include "ps1/gte.h"

typedef uint8_t u_char;
typedef uint16_t u_short;
typedef uint32_t u_long;

typedef struct {
    int16_t vx, vy;
} DVECTOR;

typedef struct {
    int16_t vx, vy, vz, pad;
} SVECTOR;

typedef struct {
    int32_t vx, vy, vz, pad;
} VECTOR;

typedef struct {
    uint8_t r, g, b, cd;
} CVECTOR;

typedef struct {
    int16_t x, y, w, h;
} RECT;

typedef struct {
    int16_t m[3][3];
    int16_t pad;
    VECTOR t;
} MATRIX;

typedef struct {
    int16_t x, y, w, h;
} DISPENV;

typedef struct {
    int16_t x, y, w, h;
    uint8_t isbg, r0, g0, b0;
} DRAWENV;

typedef struct {
    RECT *prect, *crect;
    uint32_t *paddr, *caddr;
    uint32_t mode;
} TIM_IMAGE;

typedef struct {
    uint32_t tag;
    uint8_t r0, g0, b0, code;
    int16_t x0, y0;
    int16_t x1, y1;
    int16_t x2, y2;
} POLY_F3;

typedef struct {
    uint32_t tag;
    uint8_t r0, g0, b0, code;
    int16_t x0, y0;
    uint8_t u0, v0;
    uint16_t clut;
    int16_t x1, y1;
    uint8_t u1, v1;
    uint16_t tpage;
    int16_t x2, y2;
    uint8_t u2, v2;
    uint16_t pad;
} POLY_FT3;

typedef struct {
    uint32_t tag;
    uint8_t r0, g0, b0, code;
    int16_t x0, y0, x1, y1;
} LINE_F2;

_Static_assert(sizeof(POLY_F3) == 20, "POLY_F3 must contain four GP0 words");
_Static_assert(sizeof(POLY_FT3) == 32, "POLY_FT3 must contain seven GP0 words");
_Static_assert(sizeof(LINE_F2) == 16, "LINE_F2 must contain three GP0 words");

#define PLATFORM_ORDERING_TABLE_SIZE 4096

#define setVector(v, x, y, z) ((v)->vx = (x), (v)->vy = (y), (v)->vz = (z))
#define setRECT(r, _x, _y, _w, _h) ((r)->x = (_x), (r)->y = (_y), (r)->w = (_w), (r)->h = (_h))
#define setRGB0(p, r, g, b) ((p)->r0 = (r), (p)->g0 = (g), (p)->b0 = (b))
#define copyVector(dst, src) (*(dst) = *(src))
#define setlen(p, n) ((p)->tag = ((p)->tag & 0x00ffffffu) | ((uint32_t)(n) << 24))
#define setPolyF3(p) (setlen((p), 4), (p)->code = 0x20)
#define setPolyFT3(p) (setlen((p), 7), (p)->code = 0x24)
#define setLineF2(p) (setlen((p), 3), (p)->code = 0x40)
#define setUV3(p, a, b, c, d, e, f) ((p)->u0 = (a), (p)->v0 = (b), (p)->u1 = (c), (p)->v1 = (d), (p)->u2 = (e), (p)->v2 = (f))
#define getTPage(mode, blend, x, y) gp0_page((x) / 64, (y) / 256, (blend), (mode))
#define getClut(x, y) gp0_clut((x) / 16, (y))

void ResetGraph(void);
void InitGeom(void);
void SetDefDispEnv(DISPENV*, int, int, int, int);
void SetDefDrawEnv(DRAWENV*, int, int, int, int);
void PutDispEnv(DISPENV*);
void PutDrawEnv(DRAWENV*);
void SetDispMask(int);
void DrawSync(void);
int VSync(int);
void DrawOTag(uint32_t*);
void ClearOTagR(uint32_t*, int);
void addPrim(uint32_t*, void*);
void LoadImage(const RECT*, const void*);
void OpenTIM(const void*, size_t);
int ReadTIM(TIM_IMAGE*);
void FntLoad(int, int);
int FntOpen(int, int, int, int, int, int);
int FntPrint(const char*, ...);
void FntFlushToOT(uint32_t*, uint8_t**, uint8_t*);
void pad_init(void);
void pad_pollEvents(void);
void InitHeap(void);
uint16_t platform_hblank_counter(void);
uint16_t platform_hblanks_per_vsync(void);
void RotMatrix_gte(const SVECTOR*, MATRIX*);
void TransMatrix(MATRIX*, const VECTOR*);
void ScaleMatrix(MATRIX*, const VECTOR*);
void MulMatrix0(const MATRIX*, const MATRIX*, MATRIX*);
void CompMatrixLV(const MATRIX*, const MATRIX*, MATRIX*);
void ApplyMatrixLV(const MATRIX*, const VECTOR*, VECTOR*);
void VectorNormalS(const VECTOR*, SVECTOR*);
int SquareRoot12(int);
void gte_SetGeomOffset(int, int);
void gte_SetGeomScreen(int);
void gte_SetBackColor(int, int, int);
void gte_SetColorMatrix(const MATRIX*);
void gte_SetLightMatrix(const MATRIX*);
void gte_SetRotMatrix(const MATRIX*);
void gte_SetTransMatrix(const MATRIX*);
void gte_ldv0(const SVECTOR*);
void gte_ldv3(const SVECTOR*, const SVECTOR*, const SVECTOR*);
void gte_rtps(void);
void gte_rtpt(void);
void gte_stflg(int32_t*);
void gte_nclip(void);
void gte_stopz(int32_t*);
void gte_avsz3(void);
void gte_stotz(int32_t*);
void gte_stsxy(int16_t*);
void gte_stsxy0(int16_t*);
void gte_stsxy1(int16_t*);
void gte_stsxy2(int16_t*);
void gte_ldrgb(const void*);
void gte_nct(void);
void gte_strgb(void*);

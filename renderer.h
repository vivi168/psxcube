#ifndef RENDERER_H
#define RENDERER_H

#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>

#include "mesh.h"

void rdr_init();
void rdr_render(Mesh*, SVECTOR*);
void rdr_cleanup();

unsigned int rdr_getticks();
void rdr_delay();

#undef gte_rtps
#define gte_rtps() __asm__ volatile (\
    "nop;"                  \
    "nop;"                  \
    "cop2 0x0180001;")

#undef gte_rtpt
#define gte_rtpt() __asm__ volatile (\
    "nop;"                  \
    "nop;"                  \
    "cop2 0x0280030;")

#undef gte_nclip
#define gte_nclip() __asm__ volatile (\
    "nop;"                  \
    "nop;"                  \
    "cop2 0x1400006;")

#undef gte_avsz4
#define gte_avsz4() __asm__ volatile (\
    "nop;"                  \
    "nop;"                  \
    "cop2 0x168002E;")

#endif

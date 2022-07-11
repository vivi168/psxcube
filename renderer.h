#ifndef RENDERER_H
#define RENDERER_H

#include "types.h"
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>

typedef struct mesh_t {
    SVECTOR* vertices;
    CVECTOR* colors;
    unsigned int* indices;
    unsigned int num_faces;
    unsigned int num_vertices;
} Mesh;

void rdr_init();
void rdr_render(Mesh*);
void rdr_cleanup();

unsigned int rdr_getticks();
void rdr_delay();

#endif

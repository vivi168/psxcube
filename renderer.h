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

#endif

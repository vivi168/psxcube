#ifndef RENDERER_H
#define RENDERER_H

void rdr_init();
void rdr_render(ObjMesh*, SVECTOR*);
void rdr_cleanup();

unsigned int rdr_getticks();
void rdr_delay();

#endif

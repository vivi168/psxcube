#ifndef RENDERER_H
#define RENDERER_H

void rdr_init();
void rdr_init_textures(const Mesh3D*);
void rdr_render(Mesh3D*, SVECTOR*);
void rdr_cleanup();

unsigned int rdr_getticks();
void rdr_delay();

#endif

#ifndef RENDERER_H
#define RENDERER_H

#define SCREEN_W 320
#define SCREEN_H 240
#define SCREEN_Z 512

void rdr_init();
void rdr_init_textures(const Mesh3D*);
void rdr_render(Model3D*, SVECTOR*);
void rdr_cleanup();

void rdr_delay();

#endif

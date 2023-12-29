#ifndef RENDERER_H
#define RENDERER_H

#define SCREEN_W 320
#define SCREEN_H 240
#define SCREEN_Z (SCREEN_W >> 1)

void rdr_prependToScene(Model3D*);
void rdr_appendToScene(Model3D*);
void rdr_setSceneCamera(Camera*);
void rdr_setSceneTerrain(Terrain*);

void rdr_init();
void rdr_init_textures(const Mesh3D*);
void rdr_processScene();

void rdr_draw();

#endif

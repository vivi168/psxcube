#ifndef RENDERER_H
#define RENDERER_H

#define SCREEN_W 320
#define SCREEN_H 240
#define SCREEN_Z (SCREEN_W >> 1)

void rdr_init();
void rdr_initMeshTextures(Mesh3D*);
void rdr_initTerrainTextures(Terrain* terrain);
void rdr_draw();

// Scene

void rdr_prependToScene(Model3D*);
void rdr_appendToScene(Model3D*);
void rdr_setSceneCamera(Camera*);
void rdr_setSceneTerrain(Terrain*);
void rdr_setSceneWeapon(Model3D*);
void rdr_processScene();

#endif

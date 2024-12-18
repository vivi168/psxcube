#pragma once

typedef struct game_ctx_t {
    Camera camera;
    Mesh3D   meshes[5];
    Model3D  models[5];
    MD5Model md5_models[5];
    MD5Anim  md5_anims[10];

    Terrain terrain;

    unsigned long long frameCounter;
    unsigned int frame_start;
    // int curr_frame = 0;
    int q, pq;
    int cx, cy;
} GameContext;

void OnInit(GameContext*);
void OnUpdate(GameContext*);

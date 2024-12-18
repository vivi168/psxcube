#include "psxcube.h"

// models/meshes/md5_models
#define TREE_MESH    0
#define BOB_MESH     1
#define CUBEGUY_MESH 2
#define HOUSE_MESH   3
#define SWORD_MESH   4
// anims
#define BOB_ANIM        0
#define CUBEGUY_RUNNING 1

static void init_assets(GameContext* ctx)
{
    // tree
    {
        obj_readMesh("\\TREE1.M3D;1", &ctx->meshes[TREE_MESH]);
        // print_mesh3d(&ctx->meshes[TREE_MESH]);
        // TODO: if multiple models share same texture
        // no need to reload texture
        rdr_initMeshTextures(&ctx->meshes[TREE_MESH]);

        model_initStaticModel(&ctx->models[TREE_MESH], &ctx->meshes[TREE_MESH]);

        model_setScale(&ctx->models[TREE_MESH], ONE);
        model_setRotation(&ctx->models[TREE_MESH], 0, 0, 0);
        model_setTranslation(&ctx->models[TREE_MESH], 500, 0, 500);

        rdr_appendToScene(&ctx->models[TREE_MESH]);
    }

    // sword
    {
        obj_readMesh("\\SWORD1.M3D;1", &ctx->meshes[SWORD_MESH]);
        // print_mesh3d(&ctx->meshes[SWORD_MESH]);
        // TODO: if multiple models share same texture
        // no need to reload texture
        rdr_initMeshTextures(&ctx->meshes[SWORD_MESH]);

        model_initStaticModel(&ctx->models[SWORD_MESH], &ctx->meshes[SWORD_MESH]);

        model_setScale(&ctx->models[SWORD_MESH], ONE);
        model_setRotation(&ctx->models[SWORD_MESH], -M_PI / 6, M_PI / 3, -M_PI / 2);
        model_setTranslation(&ctx->models[SWORD_MESH], 150, 200, (SCREEN_Z * 3) / 2);

        // rdr_appendToScene(&ctx->models[SWORD_MESH]);
        rdr_setSceneWeapon(&ctx->models[SWORD_MESH]);
    }

    // House
    {
        obj_readMesh("\\HOUSE.M3D;1", &ctx->meshes[HOUSE_MESH]);
        // print_mesh3d(&ctx->meshes[HOUSE_MESH]);
        // TODO: if multiple models share same texture
        // no need to reload texture
        rdr_initMeshTextures(&ctx->meshes[HOUSE_MESH]);

        model_initStaticModel(&ctx->models[HOUSE_MESH], &ctx->meshes[HOUSE_MESH]);

        model_setScale(&ctx->models[HOUSE_MESH], ONE);
        model_setRotation(&ctx->models[HOUSE_MESH], 0, 0, 0);
        model_setTranslation(&ctx->models[HOUSE_MESH], 2000, 0, 2000);

        rdr_appendToScene(&ctx->models[HOUSE_MESH]);
    }

    // CubeGuy
    {
        md5_readModel("\\CUBEGUY.MD5M;1", &ctx->md5_models[CUBEGUY_MESH]);
        md5_readAnim("\\RUNNING.MD5A;1", &ctx->md5_anims[CUBEGUY_RUNNING]);

        model_initAnimatedModel(&ctx->models[CUBEGUY_MESH],
                                &ctx->md5_models[CUBEGUY_MESH],
                                &ctx->md5_anims[CUBEGUY_RUNNING]);
        // TODO: do not load same texture file twice
        rdr_initMeshTextures(ctx->models[CUBEGUY_MESH].mesh);
        // print_mesh3d(ctx->models[CUBEGUY_MESH].mesh);

        model_setScale(&ctx->models[CUBEGUY_MESH], ONE);
        model_setRotation(&ctx->models[CUBEGUY_MESH], 0, 0, 0);
        model_setTranslation(&ctx->models[CUBEGUY_MESH], -500, 0, 500);

        rdr_appendToScene(&ctx->models[CUBEGUY_MESH]);
    }

    // Bob
#ifdef LOADBOB
    {
        md5_readModel("\\BOB.MD5M;1", &ctx->md5_models[BOB_MESH]);
        md5_readAnim("\\BOB.MD5A;1", &ctx->md5_anims[BOB_ANIM]);

        // TODO: what if multiple animations
        // animated model has mesh on heap ? can't share mesh because it's
        // animated and thus modified.
        model_initAnimatedModel(&ctx->models[BOB_MESH],
                                &ctx->md5_models[BOB_MESH],
                                &ctx->md5_anims[BOB_ANIM]);
        rdr_initMeshTextures(
            ctx->models[BOB_MESH].mesh); // TODO: be careful of doing this after
                                    // initing the mesh.
        // print_mesh3d(ctx->models[BOB_MESH].mesh);

        model_setScale(&ctx->models[BOB_MESH], ONE);
        model_setRotation(&ctx->models[BOB_MESH], 0, 0, 0);
        model_setTranslation(&ctx->models[BOB_MESH], 0, 0, 500);

        rdr_appendToScene(&ctx->models[BOB_MESH]);
    }
#endif

    rdr_initTerrainTextures(&ctx->terrain);

    printf("[INFO]: assets init done !\n");
}

void OnInit(GameContext* ctx)
{
    rdr_init();

    init_assets(ctx);

    // camera init
    {
        rdr_setSceneCamera(&ctx->camera);
        cam_setTranslation(&ctx->camera, 200, -2500, 200);
        setVector(&ctx->camera.rotation, 0, -512, 0);
        cam_init(&ctx->camera);
    }

    noise_init();
    rdr_setSceneTerrain(&ctx->terrain);

    ctx->frameCounter = 0;
    ctx->q = terrain_chunkQuadrant(ctx->camera.translate.vx,
                                   ctx->camera.translate.vz,
                                   &ctx->cx,
                                   &ctx->cy);
    terrain_init(&ctx->terrain, ctx->cx, ctx->cy, ctx->q, terrain_fbm3);

    printf("[INFO]: OnInit done !\n");
}

void OnUpdate(GameContext* ctx)
{
    ctx->frame_start = VSync(-1);
    ctx->pq = ctx->q;

    pad_pollEvents();

    int h = terrain_currentHeight(ctx->terrain.current_chunk,
                                  ctx->camera.translate.vx,
                                  ctx->camera.translate.vz);
    ctx->camera.translate.vy = h - 1536;
    cam_processInput2(&ctx->camera);

    ctx->q = terrain_chunkQuadrant(ctx->camera.translate.vx,
                                   ctx->camera.translate.vz,
                                   &ctx->cx,
                                   &ctx->cy);
    if (ctx->q != ctx->pq) {
        terrain_update(&ctx->terrain, ctx->cx, ctx->cy, ctx->q, terrain_fbm3);
    }

    // TODO: function to loop through scene linked list and update animated
    // models.
    // TODO 2: also loop through scene to update if model is visible or not
    // ?
    model_updateAnim(&ctx->models[CUBEGUY_MESH], ctx->frameCounter);
#ifdef LOADBOB
    model_updateAnim(&ctx->models[BOB_MESH], ctx->frameCounter);
#endif
    rdr_processScene();
    ctx->frameCounter++;
    // now we can compute how many frame per seconds
    rdr_draw();
}

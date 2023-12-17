#include "stdafx.h"
#include "header.h"

#define HEAP_SIZE (1024 * 1024)
static char heap[HEAP_SIZE];

// TODO: struct to hold this ?
unsigned long long vsyncCounter;
unsigned long long frameCounter;
unsigned long long timeCounter;

Camera camera;

// models/meshes/md5_models
#define CUBE_MESH 0
#define BOB_MESH 1
#define CUBEGUY_MESH 2
// anims
#define BOB_ANIM 0
#define CUBEGUY_RUNNING 1

Mesh3D meshes[5];
Model3D models[5];
MD5Model md5_models[5];
MD5Anim md5_anims[10];

SVECTOR rotvec;

// TODO: pass camera instead of global var?
// TODO: Cam_ProcessInput
// TODO: different camera types ?
void process_input()
{
    // rotvec.vx = 0;
    // rotvec.vy = 0;
    // rotvec.vz = 0;

    camera.trot.vx = FixedToInt(camera.rot.vx);
    camera.trot.vy = FixedToInt(camera.rot.vy);
    camera.trot.vz = FixedToInt(camera.rot.vz);

#define CAM_ROT_SPEED (ONE * 12)

    if (iptm_is_held(KEY_UP)) {
        camera.rot.vx -= CAM_ROT_SPEED;
    }
    if (iptm_is_held(KEY_DOWN)) {
        camera.rot.vx += CAM_ROT_SPEED;
    }
    if (iptm_is_held(KEY_LEFT)) {
        camera.rot.vy += CAM_ROT_SPEED;
    }
    if (iptm_is_held(KEY_RIGHT)) {
        camera.rot.vy -= CAM_ROT_SPEED;
    }
    if (iptm_is_held(KEY_TRIANGLE)) {
        camera.pos.vx -= FixedMulFixed(iSin(camera.trot.vy), iCos(camera.trot.vx)) << 2;
        // camera.pos.vy += iSin(camera.trot.vx) << 2;
        camera.pos.vz += FixedMulFixed(iCos(camera.trot.vy), iCos(camera.trot.vx)) << 2;
    }
    if (iptm_is_held(KEY_CROSS)) {
        camera.pos.vx += FixedMulFixed(iSin(camera.trot.vy), iCos(camera.trot.vx)) << 2;
        // camera.pos.vy -= iSin(camera.trot.vx) << 2;
        camera.pos.vz -= FixedMulFixed(iCos(camera.trot.vy), iCos(camera.trot.vx)) << 2;
    }
    if (iptm_is_held(KEY_SQUARE)) {
        camera.pos.vx -= iCos(camera.trot.vy) << 2;
        camera.pos.vz -= iSin(camera.trot.vy) << 2;
    }
    if (iptm_is_held(KEY_CIRCLE)) {
        camera.pos.vx += iCos(camera.trot.vy) << 2;
        camera.pos.vz += iSin(camera.trot.vy) << 2;
    }

    if (iptm_is_pressed(KEY_TRIANGLE)) printf("TRIANGLE\n");
    if (iptm_is_pressed(KEY_CIRCLE)) printf("CIRCLE\n");
    if (iptm_is_pressed(KEY_CROSS)) printf("CROSS\n");
    if (iptm_is_pressed(KEY_SQUARE)) printf("SQUARE\n");

}

// TODO better way
void init_assets()
{
    // Cube
    read_obj("\\CUBE.M3D;1", &meshes[CUBE_MESH]);
    print_mesh3d(&meshes[CUBE_MESH]);
    // TODO: if multiple models share same texture
    rdr_init_textures(&meshes[CUBE_MESH]);

    models[CUBE_MESH].mesh = &meshes[CUBE_MESH];
    model_setScale(&models[CUBE_MESH], 100);
    model_setRotation(&models[CUBE_MESH], 0, 0, 0);
    model_setTranslation(&models[CUBE_MESH], 500, 0, 500);

    rdr_appendToScene(&models[CUBE_MESH]);

    // Bob
    read_md5model("\\BOB.MD5M;1", &md5_models[BOB_MESH]);
    read_md5anim("\\BOB.MD5A;1", &md5_anims[BOB_ANIM]);

    init_mesh3d(&md5_models[BOB_MESH], &meshes[BOB_MESH]);
    update_mesh3d(&md5_models[BOB_MESH], md5_anims[BOB_ANIM].frameJoints[0], &meshes[BOB_MESH]);
    rdr_init_textures(&meshes[BOB_MESH]);

    // print_mesh3d(&meshes[BOB_MESH]);

    models[BOB_MESH].mesh = &meshes[BOB_MESH];
    models[BOB_MESH].md5_model = &md5_models[BOB_MESH];
    /* models[BOB_MESH].md5_anims = malloc3(sizeof(MD5Anim) * 1); */
    models[BOB_MESH].md5_anim = &md5_anims[BOB_ANIM];

    model_setScale(&models[BOB_MESH], 100);
    model_setRotation(&models[BOB_MESH], 0, 0, 0);
    model_setTranslation(&models[BOB_MESH], 0, 0, 500);

    rdr_appendToScene(&models[BOB_MESH]);

    printf("[INFO]: assets init done !\n");
}

void mainloop()
{
    unsigned int frame_start;
    int quit = 0;
    int curr_frame = 0;

    {
        rdr_setSceneCamera(&camera);
        Cam_SetPos(&camera, 0, -300, 0);
        setVector(&camera.rot, 0, 0, 0);
    }

    while (!quit) {
        frame_start = VSync(-1);

        iptm_poll_events();
        process_input();
        Cam_Update(&camera);

        // TODO: extract animation to function
        {
            int frameDuration = 60 / models[BOB_MESH].md5_anim->header.frameRate;
            if (frameCounter % frameDuration == 0) {
                curr_frame ++;
                if (curr_frame > models[BOB_MESH].md5_anim->header.numFrames - 1)
                    curr_frame = 0;

                update_mesh3d(models[BOB_MESH].md5_model,
                              models[BOB_MESH].md5_anim->frameJoints[curr_frame],
                              models[BOB_MESH].mesh);
                // printf("Animate !! %d %d\n", curr_frame, running.header.frameRate);
            }
        }

        rdr_processScene();
        frameCounter ++;
        // now we can compute how many frame per seconds
        rdr_draw();
    }
}

void vsync_callback()
{
    // VSync(-1);
    vsyncCounter ++;

    if (vsyncCounter % 60 == 59) {
        timeCounter ++;
        // printf("Time: %d\n" , timeCounter);
    }
}

int main(void)
{
    InitHeap3((void*)&heap, HEAP_SIZE);
    CdInit();

    vsyncCounter = 0;
    frameCounter = 0;
    timeCounter = 0;

    VSyncCallback(vsync_callback);

    rdr_init();

    init_assets();

    iptm_init();
    // setVector(&rotvec, 0, 0, 0);

    printf("[INFO]: init done !\n");

    mainloop();

    return 0;
}

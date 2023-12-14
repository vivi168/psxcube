#include "stdafx.h"
#include "header.h"

#define HEAP_SIZE (1024 * 1024)
static char heap[HEAP_SIZE];

int quit;

// TODO declare these 3 as extern ?
unsigned long long vsyncCounter;
unsigned long long frameCounter;
unsigned long long timeCounter;


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

void process_input()
{
    rotvec.vx = 0;
    rotvec.vy = 0;
    rotvec.vz = 0;

    if (iptm_is_held(KEY_UP)) {
        rotvec.vx = -16;
    }
    if (iptm_is_held(KEY_DOWN)) {
        rotvec.vx = 16;
    }
    if (iptm_is_held(KEY_LEFT)) {
        rotvec.vy = -16;
    }
    if (iptm_is_held(KEY_RIGHT)) {
        rotvec.vy = 16;
    }

}

// TODO better way
void init_assets()
{
    // Bob
    read_md5model("\\BOB.MD5M;1", &md5_models[BOB_MESH]);
    read_md5anim("\\BOB.MD5A;1", &md5_anims[BOB_ANIM]);

    init_mesh3d(&md5_models[BOB_MESH], &meshes[BOB_MESH]);
    update_mesh3d(&md5_models[BOB_MESH], md5_anims[BOB_ANIM].frameJoints[0], &meshes[BOB_MESH]);
    rdr_init_textures(&meshes[BOB_MESH]);

    models[BOB_MESH].mesh = &meshes[BOB_MESH];
    models[BOB_MESH].md5_model = &md5_models[BOB_MESH];
    /* models[BOB_MESH].md5_anims = malloc3(sizeof(MD5Anim) * 1); */
    models[BOB_MESH].md5_anim = &md5_anims[BOB_ANIM];

    int sc = 35;
    setVector(&models[BOB_MESH].scale, sc, sc, sc);
    setVector(&models[BOB_MESH].rotate, 0, 0, 0);
    setVector(&models[BOB_MESH].translate, 0, 100, (SCREEN_Z * 3) / 2);

    printf("[INFO]: assets init done !\n");
}

void mainloop()
{
    unsigned int frame_start;
    quit = 0;

    int curr_frame = 0;

    while (!quit) {
        frame_start = VSync(-1);

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

        iptm_poll_events();
        process_input();

        rdr_render(&models[BOB_MESH], &rotvec);
        frameCounter ++;
        // now we can compute of many frame per seconds
        rdr_delay();
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
    setVector(&rotvec, 0, 0, 0);

    printf("[INFO]: init done !\n");

    mainloop();

    return 0;
}

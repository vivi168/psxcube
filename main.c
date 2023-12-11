#include "stdafx.h"
#include "header.h"

#define HEAP_SIZE (1024 * 1024)
char heap[HEAP_SIZE];

int quit;
unsigned long long frameCounter;
unsigned long long timeCounter;

Mesh3D cube;
Mesh3D cubeguy_mesh;
MD5Model cubeguy;
MD5Anim running;

Model3D bob;

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

void init_cube()
{
    read_obj("\\CUBE.M3D;1", &cube);
    // print_mesh3d(&cube);

    // read_md5model("\\CUBEGUY.MD5M;1", &cubeguy);
    // read_md5anim("\\RUNNING.MD5A;1", &running);
    read_md5model("\\BOB.MD5M;1", &cubeguy);
    read_md5anim("\\BOB.MD5A;1", &running);

    init_mesh3d(&cubeguy, &cubeguy_mesh);
    update_mesh3d(&cubeguy, running.frameJoints[0], &cubeguy_mesh);
    // print_mesh3d(&cubeguy_mesh);

    // print_md5model(&cubeguy);
    // print_md5anim(&running);

    printf("[INFO]: cube init done !\n");
}

void mainloop()
{
    unsigned int frame_start;
    quit = 0;

    int curr_frame = 0;

    while (!quit) {
        frame_start = rdr_getticks();

        int frameDuration = 60 / running.header.frameRate;
        if (frameCounter % frameDuration == 0) {
            curr_frame ++;
            if (curr_frame > running.header.numFrames - 1)
                curr_frame = 0;

            update_mesh3d(&cubeguy, running.frameJoints[curr_frame], &cubeguy_mesh);
            // printf("Animate !! %d %d\n", curr_frame, running.header.frameRate);
        }

        iptm_poll_events();
        process_input();

        rdr_render(&cubeguy_mesh, &rotvec);
        rdr_delay(frame_start);
    }
}

void vsync_callback()
{
    // VSync(-1);
    frameCounter ++;

    if (frameCounter % 60 == 59) {
        timeCounter ++;
        // printf("Time: %d\n" , timeCounter);
    }
}

int main(int argc, char** argv)
{
    InitHeap3((void*)&heap, HEAP_SIZE);
    CdInit();

    frameCounter = 0;
    timeCounter = 0;

    VSyncCallback(vsync_callback);

    rdr_init();

    init_cube();
    rdr_init_textures(&cubeguy_mesh);

    iptm_init();
    setVector(&rotvec, 0, 0, 0);

    printf("[INFO]: init done !\n");

    mainloop();

    return 0;
}

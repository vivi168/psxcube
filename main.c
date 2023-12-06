#include "stdafx.h"

#ifdef modern_toolchain
#include "header.h"
#endif

#define HEAP_SIZE (1024 * 1024)
char heap[HEAP_SIZE];

int quit;

ObjMesh cube;
MD5Model cubeguy;

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
    read_objmesh("\\CUBE.M3D;1", &cube);
    mesh_print_mesh(&cube);

    read_md5model("\\CUBEGUY.MD5M;1", &cubeguy);


    printf("[INFO]: cube init done !\n");
}

void mainloop()
{
    unsigned int frame_start;
    quit = 0;

    init_cube();

    while (!quit) {
        frame_start = rdr_getticks();

        iptm_poll_events();
        process_input();

        rdr_render(&cube, &rotvec);
        rdr_delay(frame_start);
    }
}

int main(int argc, char** argv)
{
    InitHeap3((void*)&heap, HEAP_SIZE);
    CdInit();

    rdr_init();
    iptm_init();
    setVector(&rotvec, 0, 0, 0);

    printf("[INFO]: init done !\n");

    mainloop();

    return 0;
}

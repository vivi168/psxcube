#include <stdlib.h>
#include <stdio.h>
#include <libcd.h>

#include "mesh.h"
#include "renderer.h"

#include "input.h"

#define HEAP_SIZE (1024 * 1024)
char heap[HEAP_SIZE];

int quit;

Mesh cube;

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
    cube = mesh_load_from_file();
    mesh_print_mesh(&cube);

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
    rotvec.vx = 0;
    rotvec.vy = 0;
    rotvec.vz = 0;
    rotvec.pad = 0;


    printf("[INFO]: init done !\n");

    mainloop();

    rdr_cleanup();

    return 0;
}

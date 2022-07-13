#include <stdlib.h>
#include <stdio.h>
#include <libcd.h>

#include "mesh.h"
#include "renderer.h"

#define HEAP_SIZE (1024 * 1024)
char heap[HEAP_SIZE];

int quit;

Mesh cube;

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

        rdr_render(&cube);
        rdr_delay(frame_start);
    }
}

int main(int argc, char** argv)
{
    InitHeap3((void*)&heap, HEAP_SIZE);
    CdInit();

    rdr_init();

    printf("[INFO]: init done !\n");

    mainloop();

    rdr_cleanup();

    return 0;
}

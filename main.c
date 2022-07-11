#include <stdlib.h>
#include <stdio.h>
#include <libcd.h>

#include "renderer.h"

#define HEAP_SIZE (1024 * 1024)
char heap[HEAP_SIZE];

int quit;

Mesh cube;

#define CUBESIZE 150

static SVECTOR cube_vertices[] = {
    { -CUBESIZE / 2, -CUBESIZE / 2, -CUBESIZE / 2, 0},
    {  CUBESIZE / 2, -CUBESIZE / 2, -CUBESIZE / 2, 0},
    {  CUBESIZE / 2,  CUBESIZE / 2, -CUBESIZE / 2, 0},
    { -CUBESIZE / 2,  CUBESIZE / 2, -CUBESIZE / 2, 0},
    { -CUBESIZE / 2, -CUBESIZE / 2,  CUBESIZE / 2, 0},
    {  CUBESIZE / 2, -CUBESIZE / 2,  CUBESIZE / 2, 0},
    {  CUBESIZE / 2,  CUBESIZE / 2,  CUBESIZE / 2, 0},
    { -CUBESIZE / 2,  CUBESIZE / 2,  CUBESIZE / 2, 0},
};

static int cube_indices[] = {
    0, 1, 2, 3, // face 1
    1, 5, 6, 2, // face 2
    5, 4, 7, 6, // face 3
    4, 0, 3, 7, // face 4
    4, 5, 1, 0, // face 5
    6, 7, 3, 2, // face 6
};

// replace with load obj file
void init_cube()
{
    int i;

    cube.num_faces = 6;
    cube.num_vertices = 8;

    cube.vertices = (SVECTOR*)malloc3(8 * sizeof(SVECTOR));
    cube.indices = (unsigned int*)malloc3(6 * 4 * sizeof(unsigned int));
    cube.colors = (CVECTOR*)malloc3(6 * sizeof(CVECTOR));

    memcpy(cube.vertices, cube_vertices, (8 * sizeof(SVECTOR)));
    memcpy(cube.indices, cube_indices, (6 * 4 * sizeof(unsigned int)));

    for (i = 0; i < 6; ++i) {
        cube.colors[i].r = 255;
        cube.colors[i].g = 0;
        cube.colors[i].b = 255;
    }

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

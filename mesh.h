#ifndef MESH_H
#define MESH_H

#include "types.h"
#include <libgte.h>

typedef struct vertex_t {
    SVECTOR position;
    SVECTOR normal;
    DVECTOR uv;
    CVECTOR color;
} Vertex;

typedef struct face_t {
    unsigned int num_vertices;
    unsigned int *vertex_idx;
} Face;

typedef struct mesh_t {
    SVECTOR* vertices_old;
    CVECTOR* colors_old;
    unsigned int* indices;
    unsigned int num_faces;
    unsigned int num_vertices;

    Face *faces;
    Vertex *vertices;
    /* Texture *texture; */
} Mesh;

Mesh mesh_load_from_file();
void mesh_print_mesh(Mesh*);

#endif
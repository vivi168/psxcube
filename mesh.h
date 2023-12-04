#ifndef MESH_H
#define MESH_H

#include "types.h"
#include <libgte.h>

#define MAX_TEX_SIZ 20

typedef struct obj_header_t {
    int numVerts;
    int numTris;
    int numSubsets;
} ObjHeader;

typedef struct vertex_t {
    SVECTOR position;
    SVECTOR normal;
    DVECTOR uv;
} Vertex;

typedef struct subset_t {
    unsigned int start, count;
    char name[MAX_TEX_SIZ];
    // TODO: add color
} Subset;

typedef struct obj_mesh_t {
    ObjHeader header;
    Vertex* vertices;
    unsigned int* indices;
    Subset* subsets;
} ObjMesh;

void read_objmesh(const char* filename, ObjMesh*);
void mesh_print_mesh(ObjMesh*);

#endif

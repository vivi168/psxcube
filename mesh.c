#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "mesh.h"
#include "io.h"

ObjMesh mesh_load_from_file()
{
    ObjMesh mesh;
    unsigned long file_size;
    unsigned char *buff;
    int offset = 0, s;

    printf("[INFO]: loading mesh\n");

    buff = load_file("\\CUBE.M3D;1", &file_size);
    if (buff == NULL) {
        printf("[ERROR]: error while loading model file\n");
        while(1);
    }

    // header
    s = sizeof(ObjHeader);
    mesh.header = malloc3(s);
    IO_memcpy(mesh.header, buff, s);
    offset += s;

    // vertices
    s = sizeof(Vertex) * mesh.header->numVerts;
    mesh.vertices = malloc(s);
    IO_memcpy(mesh.vertices, buff + offset, s);
    offset += s;

    // triangles (indices * 3)
    s = sizeof(unsigned int) * mesh.header->numTris * 3;
    mesh.indices = malloc(s);
    IO_memcpy(mesh.indices, buff + offset, s);
    offset += s;

    s = sizeof(Subset) * mesh.header->numSubsets;
    mesh.subsets = malloc(s);
    IO_memcpy(mesh.subsets, buff + offset, s);

    printf("[INFO]: Done reading mesh\n");

    free3(buff);

    return mesh;
}

void mesh_print_mesh(ObjMesh* mesh)
{
	printf("*** Obj Mesh ***\n*** Header ***\n");
    printf("%d %d %d\n", mesh->header->numVerts, mesh->header->numTris, mesh->header->numSubsets);

    printf("*** Vertices ***\n");
    for (int i = 0; i < mesh->header->numVerts; i++) {
        printf("%hd %hd %hd (%hd %hd %hd) [%d %d]\n",
                        mesh->vertices[i].position.vx,
                        mesh->vertices[i].position.vy,
                        mesh->vertices[i].position.vz,
                        // normals
                        mesh->vertices[i].normal.vx,
                        mesh->vertices[i].normal.vy,
                        mesh->vertices[i].normal.vz,
                        // uvs
                        mesh->vertices[i].uv.vx,
                        mesh->vertices[i].uv.vy
                        );
    }

    printf("*** Triangles ***\n");
    for (int i = 0; i < mesh->header->numTris * 3; i += 3) {
        printf("%hd %hd %hd\n",
               mesh->indices[i],
               mesh->indices[i+1],
               mesh->indices[i+2]
               );
    }

    printf("*** Subsets ***\n");
    for (int i = 0; i < mesh->header->numSubsets; i++) {
        printf("%hd %hd %s\n",
               mesh->subsets[i].start,
               mesh->subsets[i].count,
               mesh->subsets[i].name
               );
    }
}

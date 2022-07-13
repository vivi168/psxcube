#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mesh.h"
#include "io.h"

Mesh parsemesh(char* buffer, uint32_t size);

Mesh mesh_load_from_file()
{
    Mesh m;
    uint32_t file_size;
    int8_t *buff;

    printf("[INFO]: loading mesh\n");

    buff = load_file("\\CUBE.M3D;1", &file_size);
    if (buff == NULL) {
        printf("[ERROR]: error while loading model file\n");
        while(1);
    }

    m = parsemesh(buff, file_size);

    printf("[INFO]: Done reading mesh\n");

    free3(buff);

    return m;
}

Mesh parsemesh(char* buffer, uint32_t size) {
    Mesh m;

    char *token;
    char line[100] = {0};
    int line_no = 0;
    int l = 0;
    int has_texture;

    int vi = 0;
    Face f;
    int vc;
    int fi = 0;

    uint32_t i;

    for (i = 0; i < size; i++) {

        if (buffer[i] != '\n')
            line[l++] = buffer[i];
        else {
            if (line_no == 0) {
                // vertices count
                token = strtok(line, " ");
                token = strtok(NULL, " ");
                m.num_vertices = atoi(token);
                m.vertices = malloc3(sizeof(Vertex) * m.num_vertices);
            } else if (line_no == 1) {
                // faces count
                token = strtok(line, " ");
                token = strtok(NULL, " ");
                m.num_faces = atoi(token);
                m.faces = malloc3(sizeof(Vertex) * m.num_faces);
            } else if (line_no == 2) {
                // texture name
                token = strtok(line, " ");
                token = strtok(NULL, " ");
                has_texture = strcmp(token, "none");
                printf("texture: (%s) [%d]\n", token, has_texture);
            } else if (line_no <= 2 + m.num_vertices) {
                // vertices
                token = strtok(line, " ");
                m.vertices[vi].position.vx = atoi(token);
                token = strtok(NULL, " ");
                m.vertices[vi].position.vy = atoi(token);
                token = strtok(NULL, " ");
                m.vertices[vi].position.vz = atoi(token);
                m.vertices[vi].position.pad = 0;
                token = strtok(NULL, " ");
                m.vertices[vi].uv.vx = atoi(token);
                token = strtok(NULL, " ");
                m.vertices[vi].uv.vy = atoi(token);

                vi ++;
            } else {
                // faces
                token = strtok(line, " ");
                f.num_vertices = atoi(token);
                f.vertex_idx = malloc(sizeof(unsigned int) * f.num_vertices);

                f.color.r = 255;
                f.color.g = 255;
                f.color.b = 255;

                for (vc = 0; vc < f.num_vertices; vc++) {
                    token = strtok(NULL, " ");
                    f.vertex_idx[vc] = atoi(token);
                }
                memcpy(&m.faces[fi], &f, sizeof(Face));

                fi ++;
            }

            l = 0;
            line_no++;
            memset(line, 0, 100);
        }
    }

    return m;
}

void mesh_print_mesh(Mesh* m)
{
    int i, j;
    printf("num vertices: %d\n", m->num_vertices);

    for (i = 0; i < m->num_vertices; i++) {
        printf("vertex #%d:\t[%d %d %d], [%d %d]\n", i,
                                m->vertices[i].position.vx, m->vertices[i].position.vy, m->vertices[i].position.vz,
                                m->vertices[i].uv.vx, m->vertices[i].uv.vy);
    }

    printf("num faces: %d\n", m->num_faces);


    for (i = 0; i < m->num_faces; i++) {
        printf("face #%d (%d):\t", i, m->faces[i].num_vertices);
        for (j = 0; j < m->faces[i].num_vertices; j++) {
            printf("%d ", m->faces[i].vertex_idx[j]);
        }
        printf("\n");
    }
}
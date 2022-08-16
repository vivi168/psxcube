#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "mesh.h"
#include "io.h"

#define MAX_DIGIT 10

Mesh parsemesh(char* buffer, uint32_t size);

int parse_single(char* line)
{
	char c;
	int j = 0;
	int k = 0;
	char number[MAX_DIGIT] = { 0 };

	while ((c = line[j++]) != '\0') {
		if (isdigit(c))
			number[k++] = c;
	}

	return atoi(number);
}

Vertex parse_vertex(char* line)
{
	Vertex v;

	char c;
	int num_read = 0;
	int j = 0;
	int k = 0;
	char tmp_num[MAX_DIGIT] = { 0 };

	// TODO: 3 position + 2 UV (3 normals missing)
	// NEED bone ids, weights
	// Two functions -> parse_vertex, parse_skinned_vertex
	int numbers[5];
	int n = 0;

	while ((c = line[j++]) != '\0') {
		if ((k == 0 && c == '-') || isdigit(c))
			tmp_num[k++] = c;

		if (isspace(c) && k > 0) {
			numbers[n] = atoi(tmp_num);

			n++;
			k = 0;
			memset(tmp_num, 0, MAX_DIGIT);
		}
	}

	numbers[n] = atoi(tmp_num);

	v.position.vx = numbers[0];
	v.position.vy = numbers[1];
	v.position.vz = numbers[2];
	v.position.pad = 0;

	v.uv.vx = numbers[3];
	v.uv.vy = numbers[4];

	return v;
}

Face parse_face(char* line)
{
	Face f;

	char c;
	int num_read = 0;
	int j = 0;
	int k = 0;
	char tmp_num[MAX_DIGIT] = { 0 };

	// face count + max 4 faces = 5
	int numbers[5];
	int n = 0;

	// TODO add face color information to model
	f.color.r = 255;
	f.color.g = 255;
	f.color.b = 255;

	while ((c = line[j++]) != '\0') {
		if ((k == 0 && c == '-') || isdigit(c))
			tmp_num[k++] = c;

		if (isspace(c) && k > 0) {
			numbers[n] = atoi(tmp_num);

			n++;
			k = 0;
			memset(tmp_num, 0, MAX_DIGIT);
		}
	}

	numbers[n] = atoi(tmp_num);

	f.num_vertices = numbers[0];

	f.vertex_idx = malloc3(sizeof(unsigned int) * f.num_vertices);

	// TODO: rearange here to fix winding order ?
	f.vertex_idx[0] = numbers[1];
	f.vertex_idx[1] = numbers[2];
	f.vertex_idx[2] = numbers[3];

	if (f.num_vertices == 4)
		f.vertex_idx[3] = numbers[4];

	return f;
}

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
	char line[100] = { 0 };
	int line_no = 0;
	int l = 0;

	int vi = 0;
	int fi = 0;

	m.num_vertices = 0;
	m.num_faces = 0;

	for (long i = 0; i < size; i++) {
		if (buffer[i] != '\n') {
			line[l++] = buffer[i];
			//printf("%c", buffer[i]);
		}

		else {
			if (line_no == 0) {
				// num vertices
				m.num_vertices = parse_single(line);
				m.vertices = malloc3(sizeof(Vertex) * m.num_vertices);

				printf("vertices # %d\n", m.num_vertices);
			}
			else if (line_no == 1) {
				// num faces
				m.num_faces = parse_single(line);
				m.faces = malloc3(sizeof(Face) * m.num_faces);

				printf("faces # %d\n", m.num_faces);
			}
			else if (line_no == 2) {
				// texture name
			}
			else if (line_no < 3 + m.num_vertices) {
				// read vertices
				m.vertices[vi] = parse_vertex(line);
				vi++;
			}
			else if (line_no < 3 + m.num_vertices + m.num_faces) {
				// read faces
				m.faces[fi] = parse_face(line);
				fi++;
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

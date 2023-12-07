#include "stdafx.h"

void read_objmesh(const char* filename, ObjMesh* mesh)
{
    unsigned long file_size;
    unsigned char *buff;
    int offset = 0, s;

    printf("[INFO]: loading mesh\n");

    buff = load_file(filename, &file_size);
    if (buff == NULL) {
        printf("[ERROR]: error while loading model file\n");
        while(1);
    }

    // header
    s = sizeof(ObjHeader);
    IO_memcpy(&mesh->header, buff, s);
    offset += s;

    // vertices
    s = sizeof(Vertex) * mesh->header.numVerts;
    mesh->vertices = malloc3(s);
    IO_memcpy(mesh->vertices, buff + offset, s);
    offset += s;

    // triangles (indices * 3)
    s = sizeof(unsigned int) * mesh->header.numTris * 3;
    mesh->indices = malloc3(s);
    IO_memcpy(mesh->indices, buff + offset, s);
    offset += s;

    s = sizeof(Subset) * mesh->header.numSubsets;
    mesh->subsets = malloc3(s);
    IO_memcpy(mesh->subsets, buff + offset, s);

    printf("[INFO]: Done reading mesh\n");

    free3(buff);
}

void mesh_print_mesh(ObjMesh* mesh)
{
	printf("*** Obj Mesh ***\n*** Header ***\n");
    printf("%d %d %d\n", mesh->header.numVerts, mesh->header.numTris, mesh->header.numSubsets);

    printf("*** Vertices ***\n");
    for (int i = 0; i < mesh->header.numVerts; i++) {
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
    for (int i = 0; i < mesh->header.numTris * 3; i += 3) {
        printf("%d %d %d\n",
               mesh->indices[i],
               mesh->indices[i+1],
               mesh->indices[i+2]
               );
    }

    printf("*** Subsets ***\n");
    for (int i = 0; i < mesh->header.numSubsets; i++) {
        printf("%d %d %s\n",
               mesh->subsets[i].start,
               mesh->subsets[i].count,
               mesh->subsets[i].name
               );
    }
}

// ****************
    // MD5
// ****************

void read_md5model(const char* filename, MD5Model* model)
{
    unsigned long file_size;
    unsigned char *buff;
    int offset = 0, s;

    printf("[INFO]: loading md5 model\n");

    buff = load_file(filename, &file_size);
    if (buff == NULL) {
        printf("[ERROR]: error while loading md5 model file\n");
        while(1);
    }

    // header
    s = sizeof(MD5ModelHeader);
    IO_memcpy(&model->header, buff, s);
    offset += s;

    printf("MD5 model\n%d, %d\n", model->header.numJoints, model->header.numMeshes);

    // joints
    s = sizeof(MD5Joint) * model->header.numJoints;
    model->joints = malloc3(s);
    IO_memcpy(model->joints, buff + offset, s);
    offset += s;

    for (int i = 0; i < model->header.numJoints; i++) {
		MD5Joint* j = &model->joints[i];

		printf("parent: %d (%d %d %d) (%d %d %d %d)\n",
			j->parent,
			j->pos[X], j->pos[Y], j->pos[Z],
			j->orient[X], j->orient[Y], j->orient[Z], j->orient[W]
		);
	}

	// meshes
	model->meshes = malloc3(sizeof(MD5Mesh) * model->header.numMeshes);

	for (int i = 0; i < model->header.numMeshes; i++) {
		MD5Mesh* mesh = &model->meshes[i];

		s = sizeof(MD5MeshHeader);
		IO_memcpy(&mesh->header, buff + offset, s);
		offset += s;

		printf("meshes[%d]: %d %d %d\n", i, mesh->header.numVerts, mesh->header.numTris, mesh->header.numWeights);

		// Verts
		s = sizeof(MD5Vertex) * mesh->header.numVerts;
		mesh->vertices = malloc3(s);
		IO_memcpy(mesh->vertices, buff + offset, s);
		offset += s;

		for (int v = 0; v < mesh->header.numVerts; v++) {
			printf("(%d %d) [%d %d]\n",
				mesh->vertices[v].st[X], mesh->vertices[v].st[Y],
				mesh->vertices[v].startWeight, mesh->vertices[v].countWeight
				);
		}

		// Tris
		s = sizeof(int) * mesh->header.numTris * 3;
		mesh->indices = malloc3(s);
		IO_memcpy(mesh->indices, buff + offset, s);
		offset += s;

		for (int t = 0; t < mesh->header.numTris * 3; t+=3) {
			printf("tri: %d %d %d\n",
				mesh->indices[t],
				mesh->indices[t+1],
				mesh->indices[t+2]
			);
		}

		// Weights
		s = sizeof(MD5Weight) * mesh->header.numWeights;
		mesh->weights = malloc3(s);
		IO_memcpy(mesh->weights, buff + offset, s);
		offset += s;

		for (int w = 0; w < mesh->header.numWeights; w++) {
			printf("%d %d (%d %d %d)\n",
				mesh->weights[w].jointIndex,
				mesh->weights[w].bias,
				mesh->weights[w].pos[X], mesh->weights[w].pos[Y], mesh->weights[w].pos[Z]
			);
		}

		s = sizeof(STRING20);
		IO_memcpy(&mesh->name, buff + offset, s);
		offset += s;

		printf("shader name: %s\n", mesh->name);
	}

    printf("[INFO]: Done reading md5 model\n");

    free3(buff);
}

void read_md5anim(const char* filename, MD5Anim* anim)
{
    unsigned long file_size;
    unsigned char *buff;
    int offset = 0, s;

    printf("[INFO]: loading md5 animation\n");

    buff = load_file(filename, &file_size);
    if (buff == NULL) {
        printf("[ERROR]: error while loading md5 animation file\n");
        while(1);
    }

    // header
    s = sizeof(MD5AnimHeader);
    IO_memcpy(&anim->header, buff, s);
    offset += s;

    printf("MD5 anim\n%d, %d, %d\n", anim->header.numFrames, anim->header.numJoints, anim->header.frameRate);

    anim->frameJoints = malloc3(sizeof(MD5Joint*) * anim->header.numFrames);

    for (int i = 0; i < anim->header.numFrames; i++) {
        s = sizeof(MD5Joint) * anim->header.numJoints;
        anim->frameJoints[i] = malloc3(s);
        IO_memcpy(anim->frameJoints[i], buff + offset, s);
        offset += s;

        for (int k = 0; k < anim->header.numJoints; k++) {
            MD5Joint* j = &anim->frameJoints[i][k];

            printf("parent: %d (%d %d %d) (%d %d %d %d)\n",
             j->parent,
             j->pos[X], j->pos[Y], j->pos[Z],
             j->orient[X], j->orient[Y], j->orient[Z], j->orient[W]
            );
        }
    }

    printf("[INFO]: Done reading md5 animation\n");

    free3(buff);
}

void prepare_vertices(const MD5Mesh* mesh, const MD5Joint* joints, Vertex** vertices, const int offset)
{
    for (int k = 0; k < mesh->header.numVerts; k++) {
        MD5Vertex* v = &mesh->vertices[k];
        vec3 finalPos = { 0, 0, 0 };

        for (int i = 0; i < v->countWeight; i++) {
            MD5Weight* w = &mesh->weights[v->startWeight + i];
            const MD5Joint* joint = &joints[w->jointIndex];

            vec3 wv;
            quat_rotate_point(joint->orient, w->pos, wv);

            finalPos[X] += FixedMulFixed((joint->pos[X] + wv[X]), w->bias);
            finalPos[Y] += FixedMulFixed((joint->pos[Y] + wv[Y]), w->bias);
            finalPos[Z] += FixedMulFixed((joint->pos[Z] + wv[Z]), w->bias);
        }

        (*vertices)[k + offset].position.vx = finalPos[X];
        (*vertices)[k + offset].position.vy = -finalPos[Z];
        (*vertices)[k + offset].position.vz = finalPos[Y];
        (*vertices)[k + offset].position.pad = 0;

        setVector(&(*vertices)[k + offset].normal, 0, 0, 0);

        (*vertices)[k + offset].uv.vx = v->st[X] >> SCALE;
        (*vertices)[k + offset].uv.vy = v->st[Y] >> SCALE;

        // printf("st: %d %d\n", v->st[X] >> SCALE, v->st[Y] >> SCALE);
    }
}

void init_mesh(const MD5Model* model, ObjMesh* mesh)
{
    int numVerts = 0;
    int numTris = 0;
    int start = 0;

    mesh->header.numSubsets = model->header.numMeshes;
    mesh->subsets = malloc3(sizeof(Subset) * mesh->header.numSubsets);

    for (int i = 0; i < model->header.numMeshes; i++) {
        numVerts += model->meshes[i].header.numVerts;
        numTris += model->meshes[i].header.numTris;

        mesh->subsets[i].start = start;
        mesh->subsets[i].count = model->meshes[i].header.numTris * 3;
        IO_memcpy(mesh->subsets[i].name, model->meshes[i].name, sizeof(STRING20));

        start += model->meshes[i].header.numTris * 3;
    }

    mesh->header.numVerts = numVerts;
    mesh->header.numTris = numTris;

    mesh->vertices = malloc3(sizeof(Vertex) * numVerts);
    mesh->indices = malloc3(sizeof(unsigned int) * numTris * 3);
}

void update_mesh(const MD5Model* model, const MD5Joint* joints, ObjMesh* mesh)
{
    int vertOffset = 0;
    int triOffset = 0;
    for (int i = 0; i < model->header.numMeshes; i++) {
        prepare_vertices(&model->meshes[i], joints, &mesh->vertices, vertOffset);

        for (int t = 0; t < model->meshes[i].header.numTris * 3; t++) {
            mesh->indices[triOffset + t] = model->meshes[i].indices[t] + vertOffset;
        }

        vertOffset += model->meshes[i].header.numVerts;
        triOffset += model->meshes[i].header.numTris * 3;
    }
}

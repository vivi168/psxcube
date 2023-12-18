#include "stdafx.h"

void read_obj(const char* filename, Mesh3D* mesh)
{
    unsigned long file_size;
    unsigned char *buff;
    int offset = 0, s;

    printf("[INFO]: loading mesh\n");

    buff = load_file(filename, &file_size);
    assert(buff != NULL);

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

void print_mesh3d(Mesh3D* mesh)
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
    assert(buff != NULL);

    // header
    s = sizeof(MD5ModelHeader);
    IO_memcpy(&model->header, buff, s);
    offset += s;

    // joints
    s = sizeof(MD5Joint) * model->header.numJoints;
    model->joints = malloc3(s);
    IO_memcpy(model->joints, buff + offset, s);
    offset += s;

	// meshes
	model->meshes = malloc3(sizeof(MD5Mesh) * model->header.numMeshes);

	for (int i = 0; i < model->header.numMeshes; i++) {
		MD5Mesh* mesh = &model->meshes[i];

		s = sizeof(MD5MeshHeader);
		IO_memcpy(&mesh->header, buff + offset, s);
		offset += s;

		// Verts
		s = sizeof(MD5Vertex) * mesh->header.numVerts;
		mesh->vertices = malloc3(s);
		IO_memcpy(mesh->vertices, buff + offset, s);
		offset += s;

		// Tris
		s = sizeof(int) * mesh->header.numTris * 3;
		mesh->indices = malloc3(s);
		IO_memcpy(mesh->indices, buff + offset, s);
		offset += s;

		// Weights
		s = sizeof(MD5Weight) * mesh->header.numWeights;
		mesh->weights = malloc3(s);
		IO_memcpy(mesh->weights, buff + offset, s);
		offset += s;

        // Texture name
		s = sizeof(STRING20);
		IO_memcpy(&mesh->name, buff + offset, s);
		offset += s;
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
    assert(buff != NULL);

    // header
    s = sizeof(MD5AnimHeader);
    IO_memcpy(&anim->header, buff, s);
    offset += s;

    // joints
    anim->frameJoints = malloc3(sizeof(MD5Joint*) * anim->header.numFrames);

    for (int i = 0; i < anim->header.numFrames; i++) {
        s = sizeof(MD5Joint) * anim->header.numJoints;
        anim->frameJoints[i] = malloc3(s);
        IO_memcpy(anim->frameJoints[i], buff + offset, s);
        offset += s;
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

        // obj model are scaled by 128 int(blender export coord * 128)
        (*vertices)[k + offset].position.vx = finalPos[X] >> 5;
        (*vertices)[k + offset].position.vy = -finalPos[Z] >> 5;
        (*vertices)[k + offset].position.vz = finalPos[Y] >> 5;
        (*vertices)[k + offset].position.pad = 0;

        setVector(&(*vertices)[k + offset].normal, 0, 0, 0);

        (*vertices)[k + offset].uv.vx = v->st[X];
        (*vertices)[k + offset].uv.vy = v->st[Y];
    }
}

void init_mesh3d(const MD5Model* model, Mesh3D* mesh)
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
        mesh->subsets[i].texture = NULL;

        // TODO: assert start & count % 3 == 0
        start += model->meshes[i].header.numTris * 3;
    }

    mesh->header.numVerts = numVerts;
    // TODO: assert % 3 == 0
    mesh->header.numTris = numTris;

    mesh->vertices = malloc3(sizeof(Vertex) * numVerts);
    mesh->indices = malloc3(sizeof(unsigned int) * numTris * 3);
}

void update_mesh3d(const MD5Model* model, const MD5Joint* joints, Mesh3D* mesh)
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

void print_md5model(const MD5Model* model)
{
    printf("*** MD5Model ***\n*** Header ***\n");
    printf("%d, %d\n", model->header.numJoints, model->header.numMeshes);

    printf("*** Joints ***\n");
    for (int i = 0; i < model->header.numJoints; i++) {
		MD5Joint* j = &model->joints[i];

		printf("parent: %d (%d %d %d) (%d %d %d %d)\n",
			j->parent,
			j->pos[X], j->pos[Y], j->pos[Z],
			j->orient[X], j->orient[Y], j->orient[Z], j->orient[W]
		);
	}

    printf("*** Meshes ***\n");
    for (int i = 0; i < model->header.numMeshes; i++) {
        MD5Mesh* mesh = &model->meshes[i];
		printf("\t*** Header ***\n\t[%d] %d %d %d\n", i,
            mesh->header.numVerts, mesh->header.numTris, mesh->header.numWeights
        );

        printf("\t*** Vertices ***\n");
        for (int v = 0; v < mesh->header.numVerts; v++) {
			printf("\t(%d %d) [%d %d]\n",
				mesh->vertices[v].st[X], mesh->vertices[v].st[Y],
				mesh->vertices[v].startWeight, mesh->vertices[v].countWeight
			);
		}

        printf("\t*** Triangles ***\n");
        for (int t = 0; t < mesh->header.numTris * 3; t+=3) {
			printf("\t%d %d %d\n",
				mesh->indices[t],
				mesh->indices[t+1],
				mesh->indices[t+2]
			);
		}

        printf("\t*** Weights ***\n");
        for (int w = 0; w < mesh->header.numWeights; w++) {
			printf("\t%d %d (%d %d %d)\n",
				mesh->weights[w].jointIndex, mesh->weights[w].bias,
				mesh->weights[w].pos[X], mesh->weights[w].pos[Y], mesh->weights[w].pos[Z]
			);
		}

        printf("\t*** Texture ***\n\t%s\n", mesh->name);
    }
}

void print_md5anim(const MD5Anim* anim)
{
    printf("*** MD5Anim ***\n*** Header ***\n");
    printf("%d %d %d\n", anim->header.numFrames, anim->header.numJoints, anim->header.frameRate);

    printf("*** Joints ***\n");
    for (int i = 0; i < anim->header.numFrames; i++) {
        printf("*** FrameJoint %d ***\n", i);
        for (int k = 0; k < anim->header.numJoints; k++) {
            MD5Joint* j = &anim->frameJoints[i][k];

            printf("parent: %d (%d %d %d) (%d %d %d %d)\n",
                j->parent,
                j->pos[X], j->pos[Y], j->pos[Z],
                j->orient[X], j->orient[Y], j->orient[Z], j->orient[W]
            );
        }
    }
}

void model_initStaticModel(Model3D* model, Mesh3D* mesh)
{
    model->animated = false;

    model->mesh = mesh;
}

// void model_initAnimatedModel(Model3D* model)
void model_initAnimatedModel(Model3D* model, MD5Model* md5_model, MD5Anim* md5_anim)
{
    model->animated = true;

    model->mesh = malloc3(sizeof(Mesh3D));
    model->md5_model = md5_model;
    /* model->md5_anims = malloc3(sizeof(MD5Anim) * 1); */
    model->md5_anim = md5_anim;


    init_mesh3d(model->md5_model, model->mesh);
    // TODO: here replace by initial anmation.
    update_mesh3d(model->md5_model,
                  model->md5_anim->frameJoints[0],
                  model->mesh);
}

void model_updateAnim(Model3D* model, int frame_counter)
{
    int duration = 60 / model->md5_anim->header.frameRate;
    if (frame_counter % duration == 0) {
        model->anim_info.curr_frame++;
        if (model->anim_info.curr_frame > model->md5_anim->header.numFrames - 1)
            model->anim_info.curr_frame = 0;

        update_mesh3d(model->md5_model,
                      model->md5_anim->frameJoints[model->anim_info.curr_frame],
                      model->mesh);
    }
}

void model_setScale(Model3D* model, int scale)
{
    setVector(&model->scale, scale, scale, scale);
}

void model_setRotation(Model3D* model, int x, int y, int z)
{
    setVector(&model->rotate, x, y, z);
}

void model_setTranslation(Model3D* model, int x, int y, int z)
{
    setVector(&model->translate, x, y, z);
}

void model_mat(const Model3D* model, MATRIX* mat)
{
    RotMatrix_gte(&model->rotate, mat);
    TransMatrix(mat, &model->translate);
    ScaleMatrix(mat, &model->scale);
}

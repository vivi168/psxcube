#ifndef MESH_H
#define MESH_H

typedef char STRING20[20];

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

struct texture_t;

typedef struct subset_t {
    unsigned int start, count;
    struct texture_t* texture;
    STRING20 name;
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


// ****************
    // MD5
// ****************



typedef struct md5_vertex_t {
    vec2 st; // UV
    int startWeight;
    int countWeight;
} MD5Vertex;

typedef struct MD5_weight_t {
    int jointIndex;
    FLOAT bias;
    vec3 pos;
} MD5Weight;

typedef struct md5_joint_t {
    int parent;
    vec3 pos;
    quat orient;
} MD5Joint;

typedef struct md5_mesh_header_t {
    int numVerts;
    int numTris;
    int numWeights;
} MD5MeshHeader;

typedef struct md5_mesh_t {
    MD5MeshHeader header;

    MD5Vertex* vertices;
    unsigned int* indices;
    MD5Weight* weights;
    STRING20 name;
    // TODO: add color
} MD5Mesh;

typedef struct md5_model_header_t {
    int numJoints;
    int numMeshes;
} MD5ModelHeader;

typedef struct md5_model_t {
    MD5ModelHeader header;

    MD5Joint* joints;
    MD5Mesh* meshes;
} MD5Model;

typedef struct md5_anim_header_t {
    int numFrames;
    int numJoints;
    int frameRate;
} MD5AnimHeader;

typedef struct md5_anim_t {
    MD5AnimHeader header;
    MD5Joint** frameJoints; // frameJoints[numFrames][numJoints]
} MD5Anim;

typedef struct md5_anim_info_t {
    int currFrame;
    int nextFrame;

    DOUBLE time;
    DOUBLE frameDuration;
} MD5AnimInfo;

void read_md5model(const char* filename, MD5Model* model);
void read_md5anim(const char* filename, MD5Anim* anim);

void init_mesh(const MD5Model* model, ObjMesh* mesh);
void update_mesh(const MD5Model* model, const MD5Joint* joints, ObjMesh* mesh);

void animate(const MD5Anim* anim, MD5AnimInfo* animInfo, int dt);




#endif

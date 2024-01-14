#ifndef MESH_H
#define MESH_H

typedef struct vertex_t
{
    SVECTOR position;
    SVECTOR normal;
    DVECTOR uv;
} Vertex;

struct texture_t;

typedef struct subset_t
{
    unsigned int      start, count; // indices (not tri)
    struct texture_t* texture;
    STRING20          name;
    // TODO: add color
} Subset;

typedef struct obj_mesh_t
{
    struct
    {
        int numVerts;
        int numIndices;
        int numSubsets;
    } header;

    Vertex*       vertices;
    unsigned int* indices;
    Subset*       subsets;
} Mesh3D;

void obj_readMesh(const char* filename, Mesh3D*);
void print_mesh3d(Mesh3D*);

// ****************
// MD5
// ****************

typedef struct md5_vertex_t
{
    vec2 st; // UV
    int  startWeight;
    int  countWeight;
} MD5Vertex;

typedef struct MD5_weight_t
{
    int  jointIndex;
    int  bias;
    vec3 pos;
} MD5Weight;

typedef struct md5_joint_t
{
    int  parent;
    vec3 pos;
    quat orient;
} MD5Joint;

typedef struct md5_mesh_t
{
    struct
    {
        int numVerts;
        int numTris;
        int numWeights;
    } header;

    MD5Vertex*    vertices;
    unsigned int* indices;
    MD5Weight*    weights;
    STRING20      name;
    // TODO: add color
} MD5Mesh;

typedef struct md5_model_t
{
    struct
    {
        int numJoints;
        int numMeshes;
    } header;

    MD5Joint* joints;
    MD5Mesh*  meshes;
} MD5Model;

typedef struct md5_anim_t
{
    struct
    {
        int numFrames;
        int numJoints;
        int frameRate;
    } header;

    MD5Joint** frameJoints; // frameJoints[numFrames][numJoints]
} MD5Anim;

void md5_readModel(const char* filename, MD5Model* model);
void md5_readAnim(const char* filename, MD5Anim* anim);

void md5_initMesh(const MD5Model* model, Mesh3D* mesh);
void md5_updateMesh(const MD5Model* model, const MD5Joint* joints,
                    Mesh3D* mesh);

void print_md5model(const MD5Model* model);
void print_md5anim(const MD5Anim* anim);

// MODEL

typedef struct model_3d_t
{
    Mesh3D*   mesh;
    MD5Model* md5_model;
    MD5Anim*  md5_anim; // TODO: support multiple animations

    bool animated;

    struct anim_info_t
    {
        int      anim_count;
        MD5Anim* curr_anim;
        int      curr_frame;
    } anim_info;

    SVECTOR rotate;
    VECTOR  translate;
    VECTOR  scale;
} Model3D;

void model_initStaticModel(Model3D* model, Mesh3D* mesh);
void model_initAnimatedModel(Model3D* model, MD5Model* md5_model,
                             MD5Anim* md5_anim);
void model_updateAnim(Model3D*, int frame_counter);

void model_setScale(Model3D*, int);
void model_setRotation(Model3D*, int, int, int);
void model_setTranslation(Model3D*, int, int, int);
void model_mat(const Model3D* model, MATRIX* mat);

#endif

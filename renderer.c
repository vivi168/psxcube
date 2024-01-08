#include "stdafx.h"

#define NEAR_PLANE 16
#define FAR_PLANE  4096

#define setCVector(v, _x, _y, _z) (v)->r = _x, (v)->g = _y, (v)->b = _z

unsigned int numTri, effectiveNumTri, numQuad;

typedef struct db_t
{
    DISPENV  disp;
    DRAWENV  draw;
    uint32_t ot[FAR_PLANE];
    int8_t   pribuff[32768];
} DB;

typedef struct texture_t
{
    uint32_t mode;
    uint8_t  u, v;
    RECT     prect, crect;

    uint16_t tpage, clut;
} Texture;

typedef struct scene_node_t
{
    Model3D*             model;
    struct scene_node_t* next;
} SceneNode;

// TODO: tree instead ? group by TPAGE ?
// TODO: separate list of models, chunk, billboard etc
typedef struct scene_list_t
{
    SceneNode* head;
    SceneNode* tail;

    Camera*  camera;
    Terrain* terrain;

    Model3D* weapon_r; // weapon right hand
} Scene;

// TODO: all of these static ?
// make struct to hold these following three ?
static DB  db[2];
static DB* cdb; // int instead. make macro to get current cdb ?
                // #define CBD (db[cdb])
                // swap buffer with cdb ^= 1
static int8_t* nextpri;

static Scene scene;
static RECT  screenClip;

// one column = one light source
static MATRIX color_matrix = {
    // 1 2 3
    3072, 0, 0, // Red
    3072, 0, 0, // Green
    3072, 0, 0  // Blue
};
// one row = one light source
// represents direction and intensity
static MATRIX light_matrix = {
    // x y z
    -2048, -2048, -2048, // 1
    0,     0,     0,     // 2
    0,     0,     0      // 3
};

static void createTexture(const char* filename, Texture* texture);
static void drawFaceNormal(SVECTOR* v1, SVECTOR* v2, SVECTOR* v3);
static void addMesh(Mesh3D* mesh);
static void addChunk(Chunk* chunk);
static void addOriginAxis(MATRIX* cam_mat);
static int  addTriangle(Vertex* v1, Vertex* v2, Vertex* v3, Texture* texture);
static int  addFlatTriangle(Vertex* v1, Vertex* v2, Vertex* v3, SVECTOR* color);
static void addLine(SVECTOR* org, SVECTOR* dest, CVECTOR* color);
static void addQuad(Vertex* v1, Vertex* v2, Vertex* v3, Vertex* v4,
                    SVECTOR* color);

void rdr_init()
{
    printf("[INFO]: init\n");

    scene.head = NULL;
    scene.tail = NULL;

    ResetGraph(0);
    InitGeom();

    gte_SetGeomOffset(SCREEN_W / 2, SCREEN_H / 2);
    gte_SetGeomScreen(SCREEN_Z);

    // First buffer
    SetDefDispEnv(&db[0].disp, 0, 0, SCREEN_W, SCREEN_H);
    SetDefDrawEnv(&db[0].draw, 0, SCREEN_H, SCREEN_W, SCREEN_H);

    db[0].draw.isbg = 1;
    setRGB0(&db[0].draw, 63, 0, 127);

    // Second buffer
    SetDefDispEnv(&db[1].disp, 0, SCREEN_H, SCREEN_W, SCREEN_H);
    SetDefDrawEnv(&db[1].draw, 0, 0, SCREEN_W, SCREEN_H);

    db[1].draw.isbg = 1;
    setRGB0(&db[1].draw, 63, 0, 127);

    cdb = &db[0];
    nextpri = cdb->pribuff;

    FntLoad(960, 0);
    FntOpen(0, 8, 320, 224, 0, 100);

    SetDispMask(1);

    setRECT(&screenClip, 0, 0, SCREEN_W, SCREEN_H);

    // ambient color
    gte_SetBackColor(131, 83, 34);
    gte_SetColorMatrix(&color_matrix);
}

void rdr_initMeshTextures(Mesh3D* mesh)
{
    for (int i = 0; i < mesh->header.numSubsets; i++) {
        STRING20 tmp;
        sprintf(tmp, "\\%s.TIM;1", mesh->subsets[i].name);
        printf("Texture[%d]: %s\n", i, tmp);

        mesh->subsets[i].texture = malloc3(sizeof(Texture));
        // TODO: when loading/unloading mesh, don't forget to free everything
        createTexture(tmp, mesh->subsets[i].texture);
    }
}

void rdr_initTerrainTextures(Terrain* terrain)
{
    terrain->grassland_tex = malloc3(sizeof(Texture));
    createTexture("\\TERRAIN.TIM;1", terrain->grassland_tex);

    for (int i = 0; i < MAX_CHUNK; i++) {
        terrain->chunks[i].texture = terrain->grassland_tex;
    }
}

void rdr_draw()
{
    DrawSync(0);
    VSync(0);

    PutDrawEnv(&cdb->draw);
    PutDispEnv(&cdb->disp);

    DrawOTag(&cdb->ot[FAR_PLANE - 1]);
    FntFlush(-1);

    // TODO: extract to function swap_buffer ?
    cdb = (cdb == &db[0]) ? &db[1] : &db[0];
    nextpri = cdb->pribuff;
}

// Scene

void rdr_processScene()
{
    SceneNode* curr;
    // TODO: where to put this exactly, in relation to rdr_delay()?
    ClearOTagR(cdb->ot, FAR_PLANE);

    assert(scene.camera != NULL);

    numTri = 0;
    numQuad = 0;
    effectiveNumTri = 0;

    curr = scene.head;

    while (curr != NULL) {
        MATRIX mv, ll;

        model_mat(curr->model, &mv);
        MulMatrix0(&light_matrix, &mv, &ll);
        CompMatrixLV(&scene.camera->matrix, &mv, &mv);

        gte_SetRotMatrix(&mv);
        gte_SetTransMatrix(&mv);
        gte_SetLightMatrix(&ll);

        addMesh(curr->model->mesh);

        curr = curr->next;
    }

    for (int i = 0; i < MAX_CHUNK; i++) addChunk(&scene.terrain->chunks[i]);

    // draw weapon, don't use camera
    {

        MATRIX mv, ll;

        model_mat(scene.weapon_r, &mv);
        MulMatrix0(&light_matrix, &mv, &ll);

        gte_SetRotMatrix(&mv);
        gte_SetTransMatrix(&mv);
        gte_SetLightMatrix(&ll);

        addMesh(scene.weapon_r->mesh);
    }

#ifdef DRAW_ORIG_AXIS
    addOriginAxis(&scene.camera->matrix);
#endif

    /* FntPrint("MODEL LOADER\n"); */
    FntPrint("vsync %d\n", VSync(-1));
    /* int fps = 0; */
    /* if (tc > 0) fps = fc/tc; */
    /* FntPrint("vsync %d fc %d tc %d fps %d\n", VSync(-1), fc, tc, fps); */
    FntPrint("nt %d ent %d quad %d\n", numTri, effectiveNumTri, numQuad);
    FntPrint("cam rot x %06d y %06d z %06d\ncam pos x %06d y %06d z %06d\n",
             scene.camera->rotation.vx,
             scene.camera->rotation.vy,
             scene.camera->rotation.vz,
             scene.camera->translate.vx,
             scene.camera->translate.vy,
             scene.camera->translate.vz);

    int cx, cy, q;
    // q = terrain_chunkQuadrant(scene.camera->translate.vx,
    //                       scene.camera->translate.vz,
    //                       &cx,
    //                       &cy);
    // FntPrint("chunk %d %d q: %d\n", cx, cy, q);
}

void rdr_prependToScene(Model3D* model)
{
    SceneNode* new_node = malloc3(sizeof(SceneNode));
    new_node->model = model;

    new_node->next = scene.head;
    scene.tail = scene.head;
    scene.head = new_node;
}

void rdr_appendToScene(Model3D* model)
{
    SceneNode* new_node = malloc3(sizeof(SceneNode));
    new_node->model = model;
    new_node->next = NULL;

    if (scene.head == NULL) {
        scene.head = new_node;
        scene.tail = new_node;
        return;
    }

    scene.tail->next = new_node;
    scene.tail = new_node;
}

void rdr_setSceneCamera(Camera* camera) { scene.camera = camera; }

void rdr_setSceneTerrain(Terrain* terrain) { scene.terrain = terrain; }

void rdr_setSceneWeapon(Model3D* weap_r) { scene.weapon_r = weap_r; }

// Static

// TODO: do not reload already loaded textures shared between meshes
// keep a hash map of already loaded textures ?
// TODO: when unloading a mesh, also need to unload its texture if no longer
// used
static void createTexture(const char* filename, Texture* texture)
{
    uint32_t file_size;
    int8_t*  buff;

    TIM_IMAGE* image;

    buff = load_file(filename, &file_size);
    // TODO: if not able to load texture fallback to rendering face color?
    assert(buff != NULL);

    OpenTIM((uint32_t*)buff);
    ReadTIM(image);

    // upload pixel data to framebuffer
    LoadImage(image->prect, image->paddr);
    DrawSync(0);

    // upload CLUT to framebuffer if any
    if (image->mode & 0x8) {
        LoadImage(image->crect, image->caddr);
        DrawSync(0);
    }

    // copy properties
    printf("[INFO]: %d %d %d\n", image->mode, image->prect->x, image->prect->y);
    texture->prect = *image->prect;
    texture->crect = *image->crect;
    texture->mode = image->mode;

    texture->u = (texture->prect.x % 0x40) << (2 - (texture->mode & 0x3));
    texture->v = (texture->prect.y & 0xff);

    texture->tpage =
        getTPage(texture->mode & 0x3, 0, texture->prect.x, texture->prect.y);
    texture->clut = getClut(texture->crect.x, texture->crect.y);

    printf("[INFO]: %d %d %d\n",
           texture->mode,
           texture->prect.x,
           texture->prect.y);

    free3(buff);
}

// TODO: don't compute normal here, precompute somehwere else.
static void drawFaceNormal(SVECTOR* v1, SVECTOR* v2, SVECTOR* v3)
{
    SVECTOR ctr, dst, n;
    CVECTOR clr;

    setCVector(&clr, 255, 0, 255);

    centroid(v1, v2, v3, &ctr);
    surfaceNormal(v1, v2, v3, &n);
    setVector(&dst,
              ctr.vx + (n.vx >> 5),
              ctr.vy + (n.vy >> 5),
              ctr.vz + (n.vz >> 5));
    addLine(&ctr, &dst, &clr);
}

static void addMesh(Mesh3D* mesh)
{
    // TODO: here use subset to render.
    for (int s = 0; s < mesh->header.numSubsets; s++) {
        unsigned int offset = mesh->subsets[s].start;

        for (int i = 0; i < mesh->subsets[s].count; i += 3) {
            int i1 = mesh->indices[i + offset];
            int i2 = mesh->indices[i + 1 + offset];
            int i3 = mesh->indices[i + 2 + offset];

            int t = addTriangle(&mesh->vertices[i1],
                                &mesh->vertices[i2],
                                &mesh->vertices[i3],
                                mesh->subsets[s].texture);

#ifdef DRAW_FACE_NORM
            if (t) {
                drawFaceNormal(&mesh->vertices[i1].position,
                               &mesh->vertices[i2].position,
                               &mesh->vertices[i3].position);
            }
#endif
        }
    }
}

static void addChunk(Chunk* chunk)
{
    MATRIX mv, lm;

    MulMatrix0(&light_matrix, &chunk->matrix, &lm);
    CompMatrixLV(&scene.camera->matrix, &chunk->matrix, &mv);

    gte_SetRotMatrix(&mv);
    gte_SetTransMatrix(&mv);
    gte_SetLightMatrix(&lm);

    for (int i = 0; i < (CELL_COUNT * 2 * 3); i += 3) {
        int i1 = chunk->indices[i];
        int i2 = chunk->indices[i + 1];
        int i3 = chunk->indices[i + 2];

        int t = addTriangle(&chunk->vertices[i1],
                            &chunk->vertices[i2],
                            &chunk->vertices[i3],
                            chunk->texture);

#ifdef DRAW_FACE_NORM
        if (t) {
            drawFaceNormal(&chunk->vertices[i1].position,
                            &chunk->vertices[i2].position,
                            &chunk->vertices[i3].position);
        }
#endif
    }
}

static void addOriginAxis(MATRIX* cam_mat)
{
    SVECTOR org;
    SVECTOR axis[3];

    SVECTOR rotate;
    VECTOR  translate;
    MATRIX  axis_mat;

    // ***

    setVector(&org, 0, 0, 0);
    setVector(&axis[0], 256, 0, 0);
    setVector(&axis[1], 0, 256, 0);
    setVector(&axis[2], 0, 0, 256);

    setVector(&rotate, 0, 0, 0);
    setVector(&translate, 0, 0, 0);

    RotMatrix_gte(&rotate, &axis_mat);
    TransMatrix(&axis_mat, &translate);

    CompMatrixLV(cam_mat, &axis_mat, &axis_mat);

    gte_SetRotMatrix(&axis_mat);
    gte_SetTransMatrix(&axis_mat);

    // ***
    for (int i = 0; i < 3; i++) {
        CVECTOR color;
        if (i == 0)
            setCVector(&color, 255, 0, 0);
        else if (i == 1)
            setCVector(&color, 0, 255, 0);
        else
            setCVector(&color, 0, 0, 255);
        addLine(&org, &axis[i], &color);
    }
}

static int addTriangle(Vertex* v1, Vertex* v2, Vertex* v3, Texture* texture)
{
    int32_t   otz, nclip, flg;
    POLY_FT3* poly;

    numTri++;
    // if (effectiveNumTri > 1600) return 0;

    // load first three vertices to GTE
    gte_ldv3(&v1->position, &v2->position, &v3->position);

    // rotation, translation, perspective transformation
    gte_rtpt();

    gte_stflg(&flg);
    if (flg & 0x80000000) return 0;

    // normal clip for backface culling
    gte_nclip();
    gte_stopz(&nclip);

    if (nclip <= 0) return 0;

    // average Z for depth sorting
    gte_avsz3();
    gte_stotz(&otz); // screen_z >>= 2

    if (otz <= 0 || otz >= FAR_PLANE) return 0;

    poly = (POLY_FT3*)nextpri;
    setPolyFT3(poly);

    // set projected vertices to the primitive
    gte_stsxy0(&poly->x0);
    gte_stsxy1(&poly->x1);
    gte_stsxy2(&poly->x2);

    if (tri_clip(&screenClip,
                 (DVECTOR*)&poly->x0,
                 (DVECTOR*)&poly->x1,
                 (DVECTOR*)&poly->x2))
        return 0;

    setUV3(poly,
           texture->u + v1->uv.vx,
           texture->v + v1->uv.vy,
           texture->u + v2->uv.vx,
           texture->v + v2->uv.vy,
           texture->u + v3->uv.vx,
           texture->v + v3->uv.vy);

    poly->tpage = texture->tpage;
    poly->clut = texture->clut;

    {
        gte_ldrgb(&poly->r0);
        gte_ldv3(&v1->normal, &v2->normal, &v3->normal);
        gte_nct();
        gte_strgb(&poly->r0);
    }

    addPrim(&cdb->ot[otz], poly);
    nextpri += sizeof(POLY_FT3);

    effectiveNumTri++;
    return 1;
}

static int addFlatTriangle(Vertex* v1, Vertex* v2, Vertex* v3, SVECTOR* color)
{
    int32_t  otz, nclip, flg;
    POLY_F3* poly;

    numTri++;
    // if (effectiveNumTri > 1600) return 0;

    // load first three vertices to GTE
    gte_ldv3(&v1->position, &v2->position, &v3->position);

    // rotation, translation, perspective transformation
    gte_rtpt();

    gte_stflg(&flg);
    if (flg & 0x80000000) return 0;

    // normal clip for backface culling
    gte_nclip();
    gte_stopz(&nclip);

    if (nclip <= 0) return 0;

    // average Z for depth sorting
    gte_avsz3();
    gte_stotz(&otz); // screen_z >>= 2

    if (otz <= 0 || otz >= FAR_PLANE) return 0;

    poly = (POLY_F3*)nextpri;
    setPolyF3(poly);

    // set projected vertices to the primitive
    gte_stsxy0(&poly->x0);
    gte_stsxy1(&poly->x1);
    gte_stsxy2(&poly->x2);

    if (tri_clip(&screenClip,
                 (DVECTOR*)&poly->x0,
                 (DVECTOR*)&poly->x1,
                 (DVECTOR*)&poly->x2))
        return 0;

    setRGB0(poly, color->vx, color->vy, color->vz);

    addPrim(&cdb->ot[otz], poly);
    nextpri += sizeof(POLY_F3);

    effectiveNumTri++;
    return 1;
}

static void addLine(SVECTOR* org, SVECTOR* dest, CVECTOR* color)
{
    int      p;
    LINE_F2* line;

    line = (LINE_F2*)nextpri;
    setLineF2(line);

    gte_ldv0(org);
    gte_rtps();

    gte_stflg(&p);
    if (p & 0x80000000) return;

    gte_stsxy(&line->x0);

    gte_ldv0(dest);
    gte_rtps();

    gte_stflg(&p);
    if (p & 0x80000000) return;

    gte_stsxy(&line->x1);

    setRGB0(line, color->r, color->g, color->b);
    // setRGB0(line, 255, 0, 255);

    addPrim(&cdb->ot[0], line);
    nextpri += sizeof(LINE_F2);
}

static void addQuad(Vertex* v1, Vertex* v2, Vertex* v3, Vertex* v4,
                    SVECTOR* color)
{
    int32_t  otz, nclip;
    POLY_F4* poly;

    // load first three vertices to GTE
    gte_ldv3(&v1->position, &v2->position, &v3->position);

    // rotation, translation, perspective transformation
    gte_rtpt();
    // normal clip for backface culling
    gte_nclip();
    gte_stopz(&nclip);

    if (nclip <= 0) return;

    // average Z for depth sorting
    gte_avsz4();
    gte_stotz(&otz); // screen_z >>= 2

    if (otz < NEAR_PLANE || otz >= FAR_PLANE) return;

    poly = (POLY_F4*)nextpri;
    setPolyF4(poly);

    // set projected vertices to the primitive
    gte_stsxy0(&poly->x0);
    gte_stsxy1(&poly->x1);
    gte_stsxy2(&poly->x2);

    // compute last projected vertice
    gte_ldv0(&v4->position);
    gte_rtps();
    gte_stsxy(&poly->x3);

    if (quad_clip(&screenClip,
                  (DVECTOR*)&poly->x0,
                  (DVECTOR*)&poly->x1,
                  (DVECTOR*)&poly->x2,
                  (DVECTOR*)&poly->x3))
        return;

    // setUV4(poly, texture->u + v1->uv.vx, texture->v + v1->uv.vy,
    //              texture->u + v2->uv.vx, texture->v + v2->uv.vy,
    //              texture->u + v3->uv.vx, texture->v + v3->uv.vy,
    //              texture->u + v4->uv.vx, texture->v + v4->uv.vy);

    // poly->tpage = texture->tpage;
    // poly->clut = texture->clut;
    setRGB0(poly, color->vx, color->vy, color->vz);

    // todo: add terrain to another ot specific for terrain, positionned lower
    // addPrim(&cdb->ot[FAR_PLANE-1], poly);
    addPrim(&cdb->ot[otz], poly);
    nextpri += sizeof(POLY_F4);

    numQuad++;
}

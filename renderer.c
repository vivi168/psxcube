#include "stdafx.h"

#define SCREEN_W 320
#define SCREEN_H 240
#define SCREEN_Z 512

#define OTLEN 4096

typedef struct db_t {
    DISPENV disp;
    DRAWENV draw;
    uint32_t ot[OTLEN];
    int8_t pribuff[32768];
} DB;

typedef struct texture_t {
    uint32_t mode;
    uint8_t u, v;
    RECT prect, crect;

    uint16_t tpage, clut;
} Texture;

Texture texture;
DB db[2];
DB *cdb;
int8_t *nextpri;

SVECTOR rotation;
VECTOR translation;
VECTOR scale;
MATRIX transform;

void create_texture();
void render_mesh(ObjMesh*);
void add_tri(Vertex*, Vertex*, Vertex*);

void rdr_init()
{
    printf("[INFO]: init\n");

    ResetGraph(0);
    InitGeom();

    int sc = 75;
    setVector(&scale, sc, sc, sc);

    rotation.vx = 0;
    rotation.vy = 0;
    rotation.vz = 0;
    rotation.pad = 0;

    translation.vx = 0;
    translation.vy = 50;
    translation.vz = (SCREEN_Z * 3) / 2;
    translation.pad = 0;

    SetGeomOffset(SCREEN_W / 2, SCREEN_H / 2);
    SetGeomScreen(SCREEN_Z);

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

    create_texture();

    FntLoad( 960, 0 );
    FntOpen( 0, 8, 320, 224, 0, 100 );

    SetDispMask(1);
}

void create_texture()
{
    uint32_t file_size;
    int i;
    int8_t *buff;

    TIM_IMAGE *image;

    buff = load_file("\\CUBE.TIM;1", &file_size);
    if (buff == NULL) {
        printf("[ERROR]: error while loading texture\n");
        while(1);
    }

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
    texture.prect = *image->prect;
    texture.crect = *image->crect;
    texture.mode = image->mode;

    texture.u = (texture.prect.x % 0x40) << ( 2 - (texture.mode & 0x3));
    texture.v = (texture.prect.y & 0xff);

    texture.tpage = getTPage(texture.mode & 0x3, 0, texture.prect.x, texture.prect.y);
    texture.clut = getClut(texture.crect.x, texture.crect.y);

    printf("[INFO]: %d %d %d\n", texture.mode, texture.prect.x, texture.prect.y);

    free3(buff);
}

void rdr_render(ObjMesh *mesh, SVECTOR *rotvec)
{
    ClearOTagR(cdb->ot, OTLEN);

    rotation.vx += rotvec->vx;
    rotation.vy += rotvec->vy;
    rotation.vz += rotvec->vz;

    RotMatrix(&rotation, &transform);
    TransMatrix(&transform, &translation);
    ScaleMatrix(&transform, &scale);

    render_mesh(mesh);

    FntPrint("MODEL LOADER");
    FntFlush(-1);
}

void render_mesh(ObjMesh *mesh)
{
    int i;

    gte_SetRotMatrix(&transform);
    gte_SetTransMatrix(&transform);

    for (i = 0; i < mesh->header.numTris * 3; i += 3) {
        int i1 = mesh->indices[i];
        int i2 = mesh->indices[i+1];
        int i3 = mesh->indices[i+2];

        add_tri(&mesh->vertices[i1],
                &mesh->vertices[i2],
                &mesh->vertices[i3]
                );
    }
}

void add_tri(Vertex* v1, Vertex* v2, Vertex* v3)
{
    int32_t otz, nclip;
    POLY_FT3 *poly;

    // load first three vertices to GTE (reverse order from blender export)
    gte_ldv3(&v1->position,
             &v2->position,
             &v3->position);

    // rotation, translation, perspective transformation
    gte_rtpt();
    // normal clip for backface culling
    gte_nclip();
    gte_stopz(&nclip);

    if (nclip <= 0) return;

    // average Z for depth sorting
    gte_avsz3();
    gte_stotz(&otz);

    if (otz >= OTLEN) return;

    poly = (POLY_FT3*)nextpri;
    setPolyFT3(poly);

    // set projected vertices to the primitive
    gte_stsxy0(&poly->x0);
    gte_stsxy1(&poly->x1);
    gte_stsxy2(&poly->x2);

    setUV3(poly, texture.u + v1->uv.vx, texture.v + v1->uv.vy,
                 texture.u + v2->uv.vx, texture.v + v2->uv.vy,
                 texture.u + v3->uv.vx, texture.v + v3->uv.vy);

    poly->tpage = texture.tpage;
    poly->clut = texture.clut;
    setRGB0(poly, 255,
                 255,
                 255);

    addPrim(&cdb->ot[otz], poly);
    nextpri += sizeof(POLY_FT3);
}


unsigned int rdr_getticks()
{
    // TODO
    return 0;
}

void rdr_delay(int frame_start)
{
    DrawSync(0);
    VSync(0);

    PutDispEnv(&cdb->disp);
    PutDrawEnv(&cdb->draw);

    DrawOTag(&cdb->ot[OTLEN - 1]);

    cdb = (cdb == &db[0]) ? &db[1] : &db[0];
    nextpri = cdb->pribuff;
}

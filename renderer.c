#include <stdlib.h>
#include <stdio.h>

#include "io.h"
#include "renderer.h"

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
MATRIX transform;

void rdr_create_texture();
void rdr_render_mesh(Mesh*);

void rdr_init()
{
    printf("[INFO]: init\n");

    ResetGraph(0);
    InitGeom();

    rotation.vx = 0;
    rotation.vy = 0;
    rotation.vz = 0;
    rotation.pad = 0;

    translation.vx = 0;
    translation.vy = 0;
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

    rdr_create_texture();

    FntLoad( 960, 0 );
    FntOpen( 0, 8, 320, 224, 0, 100 );

    SetDispMask(1);
}

void rdr_create_texture()
{
    uint32_t file_size;
    int i;
    int8_t *buff;

    TIM_IMAGE   *image;

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

void rdr_render(Mesh *mesh)
{
    ClearOTagR(cdb->ot, OTLEN);

    rotation.vy += 16;
    // rotation.vz += 16;

    RotMatrix(&rotation, &transform);
    TransMatrix(&transform, &translation);

    rdr_render_mesh(mesh);

    FntPrint("CUBE DEMO");
    FntFlush(-1);
}

void rdr_render_mesh(Mesh *mesh)
{
    long p, otz;
    int i, nclip;
    POLY_F4 *pf4;

    SetRotMatrix(&transform);
    SetTransMatrix(&transform);

    for (i = 0; i < mesh->num_faces; i ++) {
        pf4 = (POLY_F4*)nextpri;
        SetPolyF4(pf4);
        setRGB0(pf4, mesh->colors[i].r, mesh->colors[i].g, mesh->colors[i].b);


        nclip = RotAverageNclip4(&mesh->vertices[mesh->indices[i * 4 + 0]],
                                 &mesh->vertices[mesh->indices[i * 4 + 1]],
                                 &mesh->vertices[mesh->indices[i * 4 + 2]],
                                 &mesh->vertices[mesh->indices[i * 4 + 3]],
                                 (long *)&pf4->x0,
                                 (long *)&pf4->x1,
                                 (long *)&pf4->x3,
                                 (long *)&pf4->x2,
                                 &p,
                                 &otz,
                                 NULL);

        if (nclip <= 0) continue;

        if ((otz > 0) && (otz < OTLEN)) {
            addPrim(cdb->ot, pf4);
            nextpri += sizeof(POLY_F4);
        }
    }
}

void rdr_cleanup()
{
    // TODO
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

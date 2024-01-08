#include "stdafx.h"

#define setDVector(v, _x, _y) (v)->vx = _x, (v)->vy = _y

static const DVECTOR neighbors[MAX_CHUNK][MAX_CHUNK] = {
  // q0
    {{ .vx = 0, .vy = 0 },
     { .vx = -1, .vy = 0 },
     { .vx = 0, .vy = -1 },
     { .vx = -1, .vy = -1 }},
 // q1
    {{ .vx = 0, .vy = 0 },
     { .vx = 1, .vy = 0 },
     { .vx = 0, .vy = -1 },
     { .vx = 1, .vy = -1 }},
 // q2
    {{ .vx = 0, .vy = 0 },
     { .vx = -1, .vy = 0 },
     { .vx = 0, .vy = 1 },
     { .vx = -1, .vy = 1 }},
 // q3
    {{ .vx = 0, .vy = 0 },
     { .vx = 1, .vy = 0 },
     { .vx = 0, .vy = 1 },
     { .vx = 1, .vy = 1 }}
};

static bool alreadyThere(DVECTOR* v, Terrain* terrain);
static void initChunk(Chunk* chunk, int cx, int cy, int (*tf)(int, int));
static void updateChunk(Chunk* chunk, int cx, int cy, int (*tf)(int, int));
static void initHeightmap(Chunk* chunk, int cx, int cy, int (*tf)(int, int));

int terrain_flat(int x, int y) { return 0; }

int terrain_slope(int x, int y) { return -(y * 200); }

int terrain_fbm3(int x, int y) { return noise_fbm(3, x, y, 0, 4000) >> 1; }

int terrain_chunkQuadrant(int x, int y, int* cx, int* cy)
{
    // get current chunk
    *cx = x >> WORLD_TO_CHUNK;
    *cy = y >> WORLD_TO_CHUNK;

    // position relative to chunk
    int x_chunk = x - (*cx * 1 << WORLD_TO_CHUNK);
    int y_chunk = y - (*cy * 1 << WORLD_TO_CHUNK);

    // get current quadrant
    int qx = x_chunk >> CHUNK_TO_QUADRANT;
    int qy = y_chunk >> CHUNK_TO_QUADRANT;
    int q = qx + qy * 2;

    return q;
}

int terrain_currentHeight(Chunk* chunk, int x, int y)
{
    // first get triangle
    int x_chunk = x - (chunk->pos.vx * 1 << WORLD_TO_CHUNK);
    int y_chunk = y - (chunk->pos.vy * 1 << WORLD_TO_CHUNK);

    int tx = x_chunk >> CHUNK_TO_CELL;
    int ty = y_chunk >> CHUNK_TO_CELL;

    // get position relative to cell
    int x_cell = x_chunk - (tx * 1 << 10);
    int y_cell = y_chunk - (ty * 1 << 10);

    // bilinear_interpolation
    int h = 0;
    {
        VECTOR a, b, c, d;
        setVector(&a, 0, 0, chunk->heightmap[ty][tx]);
        setVector(&b, 1024, 0, chunk->heightmap[ty][tx+1]);
        setVector(&c, 1024, 1024, chunk->heightmap[ty+1][tx+1]);
        setVector(&d, 0, 1024, chunk->heightmap[ty+1][tx]);

        int u = IntToFixed(x_cell - a.vx) / (b.vx - a.vx);
        int v = IntToFixed(y_cell - a.vx) / (d.vy - a.vy);

        // FntPrint("%d %d\n", u, v);

        // h = (1 - u) * (1 - v) * a.vz + u * (1 - v) * b.vz + u * v * c.vz + (1 - u) * v * d.vz;

        h = FixedMulFixed(FixedMulFixed((ONE - u), (ONE - v)), a.vz) +
        FixedMulFixed(FixedMulFixed(u, (ONE - v)), b.vz) +
        FixedMulFixed(FixedMulFixed(u, v), c.vz) +
        FixedMulFixed(FixedMulFixed((ONE - u), v), d.vz);
    }

    // FntPrint("tc: %d %d\n", x_chunk, y_chunk);
    // FntPrint("tt: %d %d - %d\n", x_cell, y_cell, h);

    return h;
}

void terrain_init(Terrain* terrain, int cx, int cy, int q, int (*tf)(int, int))
{
    printf("INIT TERRAIN!\n");

    for (int i = 0; i < MAX_CHUNK; i++) {
        initChunk(&terrain->chunks[i],
                  cx + neighbors[q][i].vx,
                  cy + neighbors[q][i].vy,
                  tf);

        if (terrain->chunks[i].pos.vx == cx && terrain->chunks[i].pos.vy == cy) {
            terrain->current_chunk = &terrain->chunks[i];
        }
    }
}

void terrain_update(Terrain* terrain, int cx, int cy, int q,
                    int (*tf)(int, int))
{
    DVECTOR needed[MAX_CHUNK] = { 0 };
    int     ni = 0;
    int     ri = 0;

    printf("UPDATE TERRAIN!\n");

    for (int i = 0; i < MAX_CHUNK; i++) {
        terrain->chunks[i].needed = false;

        if (terrain->chunks[i].pos.vx == cx && terrain->chunks[i].pos.vy == cy) {
            terrain->current_chunk = &terrain->chunks[i];
        }
    }

    // construct needed list
    for (int i = 0; i < MAX_CHUNK; i++) {
        DVECTOR n =
            (DVECTOR){ neighbors[q][i].vx + cx, neighbors[q][i].vy + cy };
        if (!alreadyThere(&n, terrain)) {
            needed[ni] = n;
            ni++;
        }
    }

    // replace chunks marked as uneeded
    for (int i = 0; i < MAX_CHUNK; i++) {
        if (!terrain->chunks[i].needed) {
            printf("not needed: %d %d\n",
                   terrain->chunks[i].pos.vx,
                   terrain->chunks[i].pos.vy);
            printf("replace with: %d %d\n", needed[ri].vx, needed[ri].vy);

            assert(ri < ni);
            updateChunk(&terrain->chunks[i], needed[ri].vx, needed[ri].vy, tf);
            ri++;
        }
    }
}

// also mark chunk as needed
static bool alreadyThere(DVECTOR* v, Terrain* terrain)
{
    for (int i = 0; i < MAX_CHUNK; i++) {
        if (v->vx == terrain->chunks[i].pos.vx &&
            v->vy == terrain->chunks[i].pos.vy)
        {
            terrain->chunks[i].needed = true;
            return true;
        }
    }

    return false;
}

static void initChunk(Chunk* chunk, int cx, int cy, int (*tf)(int, int))
{
    initHeightmap(chunk, cx, cy, tf);

    int vi = 0;
    int ii = 0;
    for (int i = 0; i < CELL_COUNT; i++) {
        int hx = i % CHUNK_SIZE;
        int hy = i / CHUNK_SIZE;

        int tl_x = hx * CELL_SIZE;
        int tl_z = hy * CELL_SIZE;

        int y1 = chunk->heightmap[hy][hx];
        int y2 = chunk->heightmap[hy][hx+1];
        int y3 = chunk->heightmap[hy+1][hx];
        int y4 = chunk->heightmap[hy+1][hx+1];

        // top left
        setVector(&chunk->vertices[vi].position, tl_x, y1, tl_z);
        setDVector(&chunk->vertices[vi].uv, 0, 0);
        // top right
        setVector(&chunk->vertices[vi+1].position, tl_x+CELL_SIZE, y2, tl_z);
        setDVector(&chunk->vertices[vi+1].uv, 31, 0);
        // bottom left
        setVector(&chunk->vertices[vi+2].position, tl_x, y3, tl_z+CELL_SIZE);
        setDVector(&chunk->vertices[vi+2].uv, 0, 31);
        // bottom right
        setVector(&chunk->vertices[vi+3].position, tl_x+CELL_SIZE, y4, tl_z+CELL_SIZE);
        setDVector(&chunk->vertices[vi+3].uv, 31, 31);

        // normals
        SVECTOR n;
        surfaceNormal(&chunk->vertices[vi].position,
                      &chunk->vertices[vi+2].position,
                      &chunk->vertices[vi+1].position,
                      &n);

        // TODO: per vertex normal?
        copyVector(&chunk->vertices[vi].normal, &n);
        copyVector(&chunk->vertices[vi+1].normal, &n);
        copyVector(&chunk->vertices[vi+2].normal, &n);
        copyVector(&chunk->vertices[vi+3].normal, &n);

        // indices
        chunk->indices[ii] = vi;
        chunk->indices[ii+1] = vi + 2;
        chunk->indices[ii+2] = vi + 1;

        chunk->indices[ii+3] = vi + 2;
        chunk->indices[ii+4] = vi + 3;
        chunk->indices[ii+5] = vi + 1;

        vi += 4;
        ii += 6;
    }

    // init Matrix
    chunk->pos.vx = cx;
    chunk->pos.vy = cy;

    SVECTOR rotate;
    VECTOR  translate;
    VECTOR  scale;

    setVector(&rotate, 0, 0, 0);
    setVector(&translate, cx << WORLD_TO_CHUNK, 0, cy << WORLD_TO_CHUNK);
    setVector(&scale, ONE, ONE, ONE);

    RotMatrix_gte(&rotate, &chunk->matrix);
    TransMatrix(&chunk->matrix, &translate);
    ScaleMatrix(&chunk->matrix, &scale);
}

static void updateChunk(Chunk* chunk, int cx, int cy, int (*tf)(int, int))
{
    initHeightmap(chunk, cx, cy, tf);

    int vi = 0;
    for (int i = 0; i < CELL_COUNT; i++) {
        int hx = i % CHUNK_SIZE;
        int hy = i / CHUNK_SIZE;

        int y1 = chunk->heightmap[hy][hx];
        int y2 = chunk->heightmap[hy][hx+1];
        int y3 = chunk->heightmap[hy+1][hx];
        int y4 = chunk->heightmap[hy+1][hx+1];

        // top left
        chunk->vertices[vi].position.vy = y1;
        // setDVector(&chunk->vertices[vi].uv, 0, 0);
        // top right
        chunk->vertices[vi+1].position.vy = y2;
        // setDVector(&chunk->vertices[vi+1].uv, 31, 0);
        // bottom left
        chunk->vertices[vi+2].position.vy = y3;
        // setDVector(&chunk->vertices[vi+2].uv, 0, 31);
        // bottom right
        chunk->vertices[vi+3].position.vy = y4;
        // setDVector(&chunk->vertices[vi+3].uv, 31, 31);

        // normals
        SVECTOR n;
        surfaceNormal(&chunk->vertices[vi].position,
                      &chunk->vertices[vi+2].position,
                      &chunk->vertices[vi+1].position,
                      &n);

        // TODO: per vertex normal?
        copyVector(&chunk->vertices[vi].normal, &n);
        copyVector(&chunk->vertices[vi+1].normal, &n);
        copyVector(&chunk->vertices[vi+2].normal, &n);
        copyVector(&chunk->vertices[vi+3].normal, &n);

        vi += 4;
    }

    // update Matrix
    chunk->pos.vx = cx;
    chunk->pos.vy = cy;

    VECTOR  translate;
    setVector(&translate, cx << WORLD_TO_CHUNK, 0, cy << WORLD_TO_CHUNK);
    TransMatrix(&chunk->matrix, &translate);
}

static void initHeightmap(Chunk* chunk, int cx, int cy, int (*tf)(int, int))
{
    for (int j = 0; j <= CHUNK_SIZE; j++) {
        for (int i = 0; i <= CHUNK_SIZE; i++) {
            int y = tf(i + cx * CHUNK_SIZE, j + cy * CHUNK_SIZE);

            chunk->heightmap[j][i] = y;
        }
    }
}

#include "stdafx.h"

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

int terrain_flat(int x, int y) { return 0; }

int terrain_slope(int x, int y) { return -(y * 200); }

int terrain_fbm3(int x, int y) { return noise_fbm(3, x, y, 0, 4000) >> 1; }

int chunk_getQuadrant(int x, int y, int* cx, int* cy)
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

// TODO: also need to generate indices.
void chunk_initTerrain(Terrain* terrain, int cx, int cy, int q,
                       int (*tf)(int, int))
{
    printf("INIT TERRAIN!\n");

    for (int i = 0; i < MAX_CHUNK; i++) {
        initChunk(&terrain->chunks[i],
                  cx + neighbors[q][i].vx,
                  cy + neighbors[q][i].vy,
                  tf);
    }
}

void chunk_updateTerrain(Terrain* terrain, int cx, int cy, int q,
                         int (*tf)(int, int))
{
    DVECTOR needed[MAX_CHUNK] = { 0 };
    int     ni = 0;
    int     ri = 0;

    printf("UPDATE TERRAIN!\n");

    // mark all chunks as not needed
    for (int i = 0; i < MAX_CHUNK; i++) {
        terrain->chunks[i].needed = false;
    }

    // construct needed list
    // alreadyThere marks chunk as needed
    for (int i = 0; i < MAX_CHUNK; i++) {
        DVECTOR n =
            (DVECTOR){ neighbors[q][i].vx + cx, neighbors[q][i].vy + cy };
        if (!alreadyThere(&n, terrain)) {
            needed[ni] = n;
            ni++;
        }
    }

    // replace chunk marked as uneeded
    for (int i = 0; i < MAX_CHUNK; i++) {
        if (!terrain->chunks[i].needed) {
            printf("not needed: %d %d\n",
                   terrain->chunks[i].pos.vx,
                   terrain->chunks[i].pos.vy);
            printf("replace with: %d %d\n", needed[ri].vx, needed[ri].vy);

            assert(ri < ni);
            // TODO: updateChunk -> only update y component
            initChunk(&terrain->chunks[i], needed[ri].vx, needed[ri].vy, tf);
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

// TODO: here need to generate additional vertices for uv
static void initChunk(Chunk* chunk, int cx, int cy, int (*tf)(int, int))
{
    for (int j = 0; j <= CHUNK_SIZE; j++) {
        for (int i = 0; i <= CHUNK_SIZE; i++) {
            int y = tf(i + cx * CHUNK_SIZE, j + cy * CHUNK_SIZE);

            int x = i * CELL_SIZE;
            int z = j * CELL_SIZE;

            // try to fix seam
            if (j == CHUNK_SIZE) z += 16;
            if (i == CHUNK_SIZE) x += 16;
            setVector(&chunk->heightmap[j][i].position, x, y, z);
        }
    }

    for (int j = 0; j < CHUNK_SIZE; j++) {
        for (int i = 0; i < CHUNK_SIZE; i++) {
            SVECTOR n;
            surfaceNormal(&chunk->heightmap[j][i].position,
                          &chunk->heightmap[j + 1][i].position,
                          &chunk->heightmap[j][i + 1].position,
                          &n);

            copyVector(&chunk->heightmap[j][i].normal, &n);
            copyVector(&chunk->heightmap[j + 1][i].normal, &n);
            copyVector(&chunk->heightmap[j][i + 1].normal, &n);
            copyVector(&chunk->heightmap[j + 1][i + 1].normal, &n);
        }
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

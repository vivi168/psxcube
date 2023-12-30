#include "stdafx.h"

int terrain_flat(int x, int y) { return 0; }
int terrain_slope(int x, int y) { return -(y * 200); }
int terrain_fbm3(int x, int y) { return noise_fbm(3, x, y, 0, 4000) >> 1; }

// TODO: generate mesh instead?
void chunk_init(Chunk* chunk, int cx, int cy, int (*tf)(int, int))
{
    for (int j = 0; j <= CHUNK_SIZE; j++) {
        for (int i = 0; i <= CHUNK_SIZE; i++) {
            int y = tf(i + cx * CHUNK_SIZE, j + cy * CHUNK_SIZE);

            int x = cx * CHUNK_SIZE + i * CELL_SIZE;
            int z = cy * CHUNK_SIZE + j * CELL_SIZE;

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
                          &chunk->heightmap[j][i + 1].position, &n);

            copyVector(&chunk->heightmap[j][i].normal, &n);
            copyVector(&chunk->heightmap[j + 1][i].normal, &n);
            copyVector(&chunk->heightmap[j][i + 1].normal, &n);
            copyVector(&chunk->heightmap[j + 1][i + 1].normal, &n);
        }
    }

    // init Matrix
    chunk->x = cx;
    chunk->y = cy;

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

void chunk_initTerrain(Terrain* terrain, int cx, int cy, int q,
                       int (*tf)(int, int))
{
    chunk_init(&terrain->chunks[0], cx, cy, tf);

    if (q == 0) {
        chunk_init(&terrain->chunks[1], cx - 1, cy, tf);
        chunk_init(&terrain->chunks[2], cx, cy - 1, tf);
        chunk_init(&terrain->chunks[3], cx - 1, cy - 1, tf);
    } else if (q == 1) {
        chunk_init(&terrain->chunks[1], cx + 1, cy, tf);
        chunk_init(&terrain->chunks[2], cx, cy - 1, tf);
        chunk_init(&terrain->chunks[3], cx + 1, cy - 1, tf);
    } else if (q == 2) {
        chunk_init(&terrain->chunks[1], cx - 1, cy, tf);
        chunk_init(&terrain->chunks[2], cx, cy + 1, tf);
        chunk_init(&terrain->chunks[3], cx - 1, cy + 1, tf);
    } else if (q == 3) {
        chunk_init(&terrain->chunks[1], cx + 1, cy, tf);
        chunk_init(&terrain->chunks[2], cx, cy + 1, tf);
        chunk_init(&terrain->chunks[3], cx + 1, cy + 1, tf);
    }
}

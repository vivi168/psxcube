#include "stdafx.h"

int terrain_flat(int x, int y) { return 0; }
int terrain_slope(int x, int y)
{
    return -(y * 200);
}

int terrain_fbm3(int x, int y)
{
    return noise_fbm(3, x, y, 0, 4000) >> 1;
}

void chunk_init(Chunk* chunk, int cx, int cy, int (*tf)(int, int))
{
    for (int j = 0; j < CHUNK_SIZE + 1; j++) {
        for (int i = 0; i < CHUNK_SIZE + 1; i++) {
            int y = tf(i + cx * CHUNK_SIZE, j + cy * CHUNK_SIZE);

            int x = cx * CHUNK_SIZE + i * CELL_SIZE;
            int z = cy * CHUNK_SIZE + j * CELL_SIZE;

            setVector(&chunk->heightmap[j][i].position, x, y, z);
        }
    }

    // init Matrix
    chunk->x = cx;
    chunk->y = cy;

    SVECTOR rotate;
    VECTOR translate;
    VECTOR scale;

    setVector(&rotate, 0, 0, 0);
    setVector(&translate, cx << WORLD_TO_CHUNK, 0, cy << WORLD_TO_CHUNK);
    setVector(&scale, ONE, ONE, ONE);

    RotMatrix_gte(&rotate, &chunk->matrix);
    TransMatrix(&chunk->matrix, &translate);
    ScaleMatrix(&chunk->matrix, &scale);
}

void chunk_initTerrain(Terrain* terrain, int x, int y)
{
    // get current chunk
    int cx = x >> WORLD_TO_CHUNK;
    int cy = y >> WORLD_TO_CHUNK;

    // position relative to chunk
    int x_chunk = x - (cx * 1 << WORLD_TO_CHUNK);
    int y_chunk = y - (cy * 1 << WORLD_TO_CHUNK);

    // get current quadrant
    int qx = x_chunk >> CHUNK_TO_QUADRANT;
    int qy = y_chunk >> CHUNK_TO_QUADRANT;


    printf("chunk relative %d %d\n", x_chunk, y_chunk);
    printf("quadrant %d %d\n", qx, qy);
}

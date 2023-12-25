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

void terrain_heightmap(Chunk out, int cx, int cy, int (*tf)(int, int))
{
    for (int j = 0; j < CHUNK_SIZE + 1; j++) {
        for (int i = 0; i < CHUNK_SIZE + 1; i++) {
            int y = tf(i + cx * CHUNK_SIZE, j + cy * CHUNK_SIZE);

            int x = cx * CHUNK_SIZE + i * CELL_SIZE;
            int z = cy * CHUNK_SIZE + j * CELL_SIZE;

            setVector(&out[j][i].position, x, y, z);
        }
    }
}

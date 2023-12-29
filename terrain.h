#ifndef TERRAIN_H
#define TERRAIN_H

#define CHUNK_SIZE 16
#define CELL_SIZE 1024
#define WORLD_TO_CHUNK 14 // (16 * 1024 = 1 << 14)

typedef struct chunk_t {
    int x, y;
    Vertex heightmap[CHUNK_SIZE + 1][CHUNK_SIZE + 1];

    MATRIX matrix;
} Chunk;

int terrain_flat(int x, int y);
int terrain_slope(int x, int y);
int terrain_fbm3(int x, int y);

void chunk_init(Chunk *out, int cx, int cy, int (*f)(int, int));

#endif

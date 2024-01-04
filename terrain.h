#ifndef TERRAIN_H
#define TERRAIN_H

#define CHUNK_SIZE        16
#define QUADRANT_SIZE     8
#define CELL_SIZE         1024
#define WORLD_TO_CHUNK    14 // (16 * 1024 = 1 << 14)
#define CHUNK_TO_QUADRANT 13 // (8 * 1024 = 1 << 13)

#define MAX_CHUNK 4

struct texture_t;

typedef struct chunk_t
{
    DVECTOR pos; // TODO: DVECTOR is short, need 32 bits
    bool    needed;
    Vertex  heightmap[CHUNK_SIZE + 1][CHUNK_SIZE + 1];
    // Vertex terrain[1024]
    // int heightmap[CHUNK_SIZE + 1][CHUNK_SIZE + 1];
    struct texture_t* texture;

    MATRIX matrix;
} Chunk;

typedef struct terrain_t
{
    Chunk chunks[MAX_CHUNK];
    // int indices[1536];

    struct texture_t* grassland_tex;

    Chunk* current_chunk;
} Terrain;

int terrain_flat(int x, int y);
int terrain_slope(int x, int y);
int terrain_fbm3(int x, int y);

int terrain_chunkQuadrant(int x, int y, int* cx, int* cy);

// this one also initialize the indices
void terrain_init(Terrain* terrain, int cx, int cy, int q, int (*f)(int, int));
void terrain_update(Terrain* terrain, int cx, int cy, int q,
                    int (*f)(int, int));
#endif

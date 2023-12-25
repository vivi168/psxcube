#ifndef TERRAIN_H
#define TERRAIN_H

#define CHUNK_SIZE 16

typedef int Chunk[CHUNK_SIZE + 1][CHUNK_SIZE + 1];

int terrain_flat(int x, int y);
int terrain_slope(int x, int y);
int terrain_fbm3(int x, int y);

void terrain_heightmap(Chunk out, int cx, int cy, int (*f)(int, int));

#endif

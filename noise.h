#ifndef NOISE_H
#define NOISE_H

void noise_init();
int noise_3d(int x, int y, int z);
int noise_fbm(int iter, int x, int y, int z, int freq);
int noise_wrap(int noise, int low, int high);

#endif

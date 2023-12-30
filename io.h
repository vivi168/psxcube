#ifndef IO_H
#define IO_H

void* IO_memcpy(void* restrict dest, const void* restrict src, int count);
char* load_file(char*, u_long*);

#endif

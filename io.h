#pragma once

#ifdef PSX_VER
#define memcpy IO_memcpy
#define malloc malloc3
#define free free3
#endif

void* IO_memcpy(void* restrict dest, const void* restrict src, int count);
unsigned char* load_file(const char*, u_long*);

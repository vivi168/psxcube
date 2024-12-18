#pragma once

#define memcpy IO_memcpy

void* IO_memcpy(void* restrict dest, const void* restrict src, int count);
unsigned char* load_file(const char*, u_long*);

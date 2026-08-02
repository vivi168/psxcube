#include "embedded_assets.h"
#include "stdafx.h"

void* IO_memcpy(void* restrict dest, const void* restrict src, int count)
{
    unsigned char* pdest = (unsigned char*)dest;
    const unsigned char* psrc = (const unsigned char*)src;

    for (; count; count--) *(pdest++) = *(psrc++);

    return dest;
}

char* load_file(const char* filename, unsigned long* size)
{
    const EmbeddedAsset* asset = embedded_asset_find(filename);
    if (asset == NULL) {
        printf("[ERROR]: File not found %s\n", filename);
        return NULL;
    }
    *size = (unsigned long)asset->size;
    return (char*)asset->data;
}

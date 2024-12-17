#include "psxcube.h"

void* IO_memcpy(void* restrict dest, const void* restrict src, int count)
{
    unsigned char*       pdest = (unsigned char*)dest;
    const unsigned char* psrc = (const unsigned char*)src;

    for (; count; count--) *(pdest++) = *(psrc++);

    return dest;
}

unsigned char* load_file(const char* filename, u_long* size)
{
    CdlFILE file;
    int     sectors;
    unsigned char*   buff;

    buff = NULL;

    printf("[INFO]: looking for %s\n", filename);

    if (CdSearchFile(&file, (char *)filename) == NULL) {
        printf("[ERROR]: File not found %s\n", filename);
        return buff;
    }

    printf("[INFO]: found %s\n", filename);
    sectors = (file.size + 2047) / 2048;
    buff = (unsigned char*)malloc3(2048 * sectors);
    CdControl(CdlSetloc, (unsigned char*)&file.pos, 0);
    CdRead(sectors, (u_long*)buff, CdlModeSpeed);
    printf("[INFO]: sectors: %d, file size: %d\n", sectors, file.size);
    CdReadSync(0, 0);

    printf("[INFO]: read sync done\n");
    *size = file.size;

    return buff;
}

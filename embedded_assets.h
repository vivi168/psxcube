#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct {
    const char* name;
    const uint8_t* data;
    size_t size;
} EmbeddedAsset;

const EmbeddedAsset* embedded_asset_find(const char* psyq_path);

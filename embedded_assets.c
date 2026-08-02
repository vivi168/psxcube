#include "embedded_assets.h"
#include <string.h>

extern const uint8_t asset_cube_m3d[], asset_cube_m3d_end[];
extern const uint8_t asset_house_m3d[], asset_house_m3d_end[];
extern const uint8_t asset_cubeguy_md5m[], asset_cubeguy_md5m_end[];
extern const uint8_t asset_running_md5a[], asset_running_md5a_end[];
extern const uint8_t asset_cube_tim[], asset_cube_tim_end[];
extern const uint8_t asset_house_tim[], asset_house_tim_end[];
extern const uint8_t asset_terrain_tim[], asset_terrain_tim_end[];
extern const uint8_t asset_tree1_m3d[], asset_tree1_m3d_end[];
extern const uint8_t asset_sword1_m3d[], asset_sword1_m3d_end[];
extern const uint8_t asset_branch5_tim[], asset_branch5_tim_end[];
extern const uint8_t asset_trunk3_tim[], asset_trunk3_tim_end[];
extern const uint8_t asset_sword1_tim[], asset_sword1_tim_end[];

#define ASSET(name, symbol) {name, symbol, symbol##_end}

typedef struct {
    const char* name;
    const uint8_t *data, *end;
} AssetRange;

static const AssetRange assets[] = {
    ASSET("CUBE.M3D", asset_cube_m3d),
    ASSET("HOUSE.M3D", asset_house_m3d),
    ASSET("CUBEGUY.MD5M", asset_cubeguy_md5m),
    ASSET("RUNNING.MD5A", asset_running_md5a),
    ASSET("CUBE.TIM", asset_cube_tim),
    ASSET("HOUSE.TIM", asset_house_tim),
    ASSET("TERRAIN.TIM", asset_terrain_tim),
    ASSET("TREE1.M3D", asset_tree1_m3d),
    ASSET("SWORD1.M3D", asset_sword1_m3d),
    ASSET("BRANCH5.TIM", asset_branch5_tim),
    ASSET("TRUNK3.TIM", asset_trunk3_tim),
    ASSET("SWORD1.TIM", asset_sword1_tim),
};

const EmbeddedAsset* embedded_asset_find(const char* path)
{
    static EmbeddedAsset result;
    const char* name = path;
    if (*name == '\\') name++;
    for (size_t i = 0; i < sizeof(assets) / sizeof(*assets); i++) {
        size_t n = strlen(assets[i].name);
        if (!strncmp(name, assets[i].name, n) && (name[n] == 0 || name[n] == ';')) {
            result.name = assets[i].name;
            result.data = assets[i].data;
            result.size = (size_t)(assets[i].end - assets[i].data);
            return &result;
        }
    }
    return NULL;
}

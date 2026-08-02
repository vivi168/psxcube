#include "system.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ps1/cop0.h"
#include "ps1/gpucmd.h"
#include "ps1/registers.h"

#define DMA_MAX_CHUNK_SIZE 16u
#define FIXED_SHIFT 12
#define FIXED_ONE (1 << FIXED_SHIFT)

#define FONT_WIDTH 96
#define FONT_HEIGHT 56
#define FONT_CELL_WIDTH 6
#define FONT_CELL_HEIGHT 9
#define FONT_LINE_HEIGHT 10
#define FONT_BUFFER_SIZE 2048

typedef struct {
    uint32_t tag;
    uint32_t command;
} FontPagePacket;

typedef struct {
    uint32_t tag;
    uint32_t command;
    uint32_t position;
    uint32_t uv;
    uint32_t size;
} FontGlyphPacket;

extern const uint8_t asset_debug_font_texture[];
extern const uint8_t asset_debug_font_palette[];

static TIM_IMAGE tim;
static RECT tim_image_rect;
static RECT tim_clut_rect;
static const uint8_t* tim_cursor;
static const uint8_t* tim_end;

static unsigned int vsync_count;
static uint16_t hblanks_per_vsync;

static char fnt_buffer[FONT_BUFFER_SIZE];
static size_t fnt_length;
static int fnt_x;
static int fnt_y;
static int fnt_w;
static int fnt_h;
static uint16_t fnt_tpage;
static uint16_t fnt_clut;
static uint8_t fnt_u;
static uint8_t fnt_v;
static bool fnt_loaded;

static inline void wait_gpu(void)
{
    while (!(GPU_GP1 & GP1_STAT_CMD_READY)) __asm__ volatile("");
}

static inline void wait_dma(void)
{
    while (DMA_CHCR(DMA_GPU) & DMA_CHCR_ENABLE) __asm__ volatile("");
}

static inline void send_gp0(uint32_t command)
{
    wait_gpu();
    GPU_GP0 = command;
}

static inline uint32_t phys(const void* pointer)
{
    return (uint32_t)pointer & 0x00ffffffu;
}

static int clamp_s16(int64_t value)
{
    if (value < -32768) return -32768;
    if (value > 32767) return 32767;
    return (int)value;
}

static int sin12(int angle)
{
    enum { QN = 10,
           QA = 12,
           B = 19900,
           C = 3516 };

    int sign;
    int squared;
    int result;

    sign = angle << (30 - QN);
    angle -= 1 << QN;
    angle = angle << (31 - QN);
    angle = angle >> (31 - QN);
    squared = angle * angle >> (2 * QN - 14);
    result = B - (squared * C >> 14);
    result = (1 << QA) - (squared * result >> 16);

    return sign >= 0 ? result : -result;
}

static int cos12(int angle)
{
    return sin12(angle + 1024);
}

static int16_t matrix_value(int64_t value)
{
    return (int16_t)clamp_s16(value >> FIXED_SHIFT);
}

void InitHeap(void)
{
    initSerialIO(115200);
}

void ResetGraph(void)
{
    GP1VideoMode mode = ((GPU_GP1 & GP1_STAT_FB_MODE_BITMASK) == GP1_STAT_FB_MODE_PAL) ? GP1_MODE_PAL : GP1_MODE_NTSC;
    hblanks_per_vsync = mode == GP1_MODE_PAL ? 314 : 263;

    const int center_x = 0x760;
    const int center_y = mode == GP1_MODE_PAL ? 0xa3 : 0x88;
    const int offset_x = (320 * gp1_clockMultiplierH(GP1_HRES_320)) / 2;
    const int offset_y = (240 / gp1_clockDividerV(GP1_VRES_256)) / 2;

    GPU_GP1 = gp1_resetGPU();
    GPU_GP1 = gp1_fbRangeH(center_x - offset_x, center_x + offset_x);
    GPU_GP1 = gp1_fbRangeV(center_y - offset_y, center_y + offset_y);
    GPU_GP1 = gp1_fbMode(GP1_HRES_320, GP1_VRES_256, mode, false, GP1_COLOR_16BPP);
    GPU_GP1 = gp1_dispBlank(true);

    DMA_DPCR |= DMA_DPCR_CH_ENABLE(DMA_GPU) | DMA_DPCR_CH_ENABLE(DMA_OTC);
    DMA_CHCR(DMA_GPU) = 0;
    DMA_CHCR(DMA_OTC) = 0;

    IRQ_STAT = ~(1 << IRQ_VSYNC);
    vsync_count = 0;

    // rough frame time clock
    TIMER_CTRL(1) = 0;
    TIMER_VALUE(1) = 0;
    TIMER_CTRL(1) = TIMER_CTRL_EXT_CLOCK;
}

void InitGeom(void)
{
    cop0_setReg(COP0_STATUS, cop0_getReg(COP0_STATUS) | COP0_STATUS_CU2);

    // PsyQ's InitGeom maps average camera Z to OTZ as Z/4.  The ordering table
    // then covers camera depths up to roughly 16384 with 4096 entries.
    gte_setControlReg(GTE_ZSF3, FIXED_ONE / 12);
    gte_setControlReg(GTE_ZSF4, FIXED_ONE / 16);
}

void SetDefDispEnv(DISPENV* env, int x, int y, int width, int height)
{
    env->x = x;
    env->y = y;
    env->w = width;
    env->h = height;
}

void SetDefDrawEnv(DRAWENV* env, int x, int y, int width, int height)
{
    env->x = x;
    env->y = y;
    env->w = width;
    env->h = height;
    env->isbg = 0;
    env->r0 = 0;
    env->g0 = 0;
    env->b0 = 0;
}

void PutDispEnv(DISPENV* env)
{
    GPU_GP1 = gp1_fbOffset(env->x, env->y);
}

void PutDrawEnv(DRAWENV* env)
{
    send_gp0(gp0_setPage(0, true, false));
    send_gp0(gp0_fbOffset1(env->x, env->y));
    send_gp0(gp0_fbOffset2(env->x + env->w - 1, env->y + env->h - 1));
    send_gp0(gp0_fbOrigin(env->x, env->y));

    if (env->isbg) {
        send_gp0(gp0_vramFill() | gp0_rgb(env->r0, env->g0, env->b0));
        send_gp0(gp0_xy(env->x, env->y));
        send_gp0(gp0_xy(env->w, env->h));
    }
}

void SetDispMask(int enabled) { GPU_GP1 = gp1_dispBlank(!enabled); }

void DrawSync(void)
{
    wait_dma();
    wait_gpu();
}

int VSync(int mode)
{
    if (mode < 0) return (int)vsync_count;

    while (!(IRQ_STAT & (1 << IRQ_VSYNC))) __asm__ volatile("");
    IRQ_STAT = ~(1 << IRQ_VSYNC);

    return (int)++vsync_count;
}

uint16_t platform_hblank_counter(void) { return TIMER_VALUE(1); }

uint16_t platform_hblanks_per_vsync(void) { return hblanks_per_vsync; }

void DrawOTag(uint32_t* tag)
{
    assert(!((uintptr_t)tag & 3));
    wait_dma();

    GPU_GP1 = gp1_dmaRequestMode(GP1_DREQ_GP0_WRITE);
    DMA_MADR(DMA_GPU) = (uint32_t)tag;
    DMA_CHCR(DMA_GPU) = DMA_CHCR_WRITE | DMA_CHCR_MODE_LIST | DMA_CHCR_ENABLE;
}

void ClearOTagR(uint32_t* ordering_table, int length)
{
    assert(ordering_table != NULL);
    assert(length > 0);
    assert(!((uintptr_t)ordering_table & 3));

    DMA_MADR(DMA_OTC) = (uint32_t)&ordering_table[length - 1];
    DMA_BCR(DMA_OTC) = (uint32_t)length;
    DMA_CHCR(DMA_OTC) = DMA_CHCR_READ | DMA_CHCR_REVERSE | DMA_CHCR_MODE_BURST | DMA_CHCR_ENABLE | DMA_CHCR_TRIGGER;

    while (DMA_CHCR(DMA_OTC) & DMA_CHCR_ENABLE) __asm__ volatile("");
}

void addPrim(uint32_t* ordering_table_entry, void* primitive)
{
    uint32_t* tag = primitive;

    *tag = (*ordering_table_entry & 0x00ffffffu) | (*tag & 0xff000000u);
    *ordering_table_entry = phys(primitive);
}

void LoadImage(const RECT* rect, const void* data)
{
    assert(rect != NULL);
    assert(data != NULL);
    assert(rect->w > 0 && rect->h > 0);
    assert(!((uintptr_t)data & 3));

    size_t length = ((size_t)rect->w * (size_t)rect->h + 1) / 2;
    size_t chunk_size = length < DMA_MAX_CHUNK_SIZE ? length : DMA_MAX_CHUNK_SIZE;

    while (length % chunk_size) chunk_size--;

    size_t num_chunks = length / chunk_size;
    assert(chunk_size <= 0xffffu);
    assert(num_chunks <= 0xffffu);

    wait_dma();
    GPU_GP1 = gp1_dmaRequestMode(GP1_DREQ_NONE);

    send_gp0(gp0_vramWrite());
    send_gp0(gp0_xy(rect->x, rect->y));
    send_gp0(gp0_xy(rect->w, rect->h));

    GPU_GP1 = gp1_dmaRequestMode(GP1_DREQ_GP0_WRITE);
    DMA_MADR(DMA_GPU) = (uint32_t)data;
    DMA_BCR(DMA_GPU) = (uint32_t)(chunk_size | (num_chunks << 16));
    DMA_CHCR(DMA_GPU) = DMA_CHCR_WRITE | DMA_CHCR_MODE_SLICE | DMA_CHCR_ENABLE;

    wait_dma();
    send_gp0(gp0_flushCache());
}

void OpenTIM(const void* data, size_t size)
{
    tim_cursor = data;
    tim_end = tim_cursor ? tim_cursor + size : NULL;
}

static bool read_tim_block(const uint8_t** cursor, RECT* rect, uint32_t** payload)
{
    const uint8_t* block = *cursor;

    if (!block || !tim_end || (size_t)(tim_end - block) < 12) return false;

    uint32_t block_size;
    memcpy(&block_size, block, sizeof(block_size));

    if (block_size < 12 || block_size > (size_t)(tim_end - block)) return false;

    memcpy(rect, block + 4, sizeof(*rect));
    if (rect->w <= 0 || rect->h <= 0) return false;

    size_t payload_size = (size_t)rect->w * (size_t)rect->h * 2;
    if (payload_size > block_size - 12) return false;

    *payload = (uint32_t*)(block + 12);
    if ((uintptr_t)*payload & 3) return false;

    *cursor = block + block_size;
    return true;
}

int ReadTIM(TIM_IMAGE* output)
{
    if (!output || !tim_cursor || !tim_end) return 0;
    if ((size_t)(tim_end - tim_cursor) < 8) return 0;

    uint32_t magic;
    uint32_t flags;
    memcpy(&magic, tim_cursor, sizeof(magic));
    memcpy(&flags, tim_cursor + 4, sizeof(flags));

    if (magic != 0x10 || (flags & ~0x0fu) || ((flags & 7) > 3)) return 0;

    const uint8_t* cursor = tim_cursor + 8;
    memset(&tim, 0, sizeof(tim));
    tim.mode = flags;

    if (flags & 8) {
        if (!read_tim_block(&cursor, &tim_clut_rect, &tim.caddr)) return 0;
        tim.crect = &tim_clut_rect;
    }

    if (!read_tim_block(&cursor, &tim_image_rect, &tim.paddr)) return 0;
    tim.prect = &tim_image_rect;

    tim_cursor = cursor;
    *output = tim;
    return 1;
}

void FntLoad(int x, int y)
{
    RECT image = {x, y, FONT_WIDTH / 4, FONT_HEIGHT};
    RECT palette = {x, y + FONT_HEIGHT, 16, 1};

    assert(x >= 0 && x + image.w <= 1024);
    assert(y >= 0 && y + FONT_HEIGHT + 1 <= 512);
    assert(!(x % 16));

    LoadImage(&image, asset_debug_font_texture);
    LoadImage(&palette, asset_debug_font_palette);

    fnt_tpage = gp0_page(x / 64, y / 256, GP0_BLEND_SEMITRANS, GP0_COLOR_4BPP);
    fnt_clut = gp0_clut(x / 16, y + FONT_HEIGHT);
    fnt_u = (uint8_t)((x % 64) * 4);
    fnt_v = (uint8_t)(y % 256);
    fnt_loaded = true;
}

int FntOpen(int x, int y, int width, int height, int is_background, int max_chars)
{
    (void)is_background;
    (void)max_chars;

    fnt_x = x;
    fnt_y = y;
    fnt_w = width;
    fnt_h = height;
    fnt_length = 0;
    fnt_buffer[0] = 0;
    return 0;
}

int FntPrint(const char* format, ...)
{
    if (!format || fnt_length >= FONT_BUFFER_SIZE - 1) return 0;

    va_list args;
    va_start(args, format);
    int result = vsnprintf(&fnt_buffer[fnt_length], FONT_BUFFER_SIZE - fnt_length, format, args);
    va_end(args);

    if (result <= 0) return result;

    size_t appended = (size_t)result;
    size_t remaining = FONT_BUFFER_SIZE - fnt_length - 1;
    fnt_length += appended < remaining ? appended : remaining;
    return result;
}

void FntFlushToOT(uint32_t* ordering_table_entry, uint8_t** next, uint8_t* end)
{
    if (!fnt_loaded || !fnt_length || !ordering_table_entry || !next || !*next) {
        fnt_length = 0;
        return;
    }

    if ((size_t)(end - *next) < sizeof(FontPagePacket)) {
        fnt_length = 0;
        fnt_buffer[0] = 0;
        return;
    }

    int current_x = fnt_x;
    int current_y = fnt_y;
    bool emitted_glyph = false;

    for (size_t i = 0; i < fnt_length; i++) {
        unsigned char character = (unsigned char)fnt_buffer[i];

        if (character == '\n') {
            current_x = fnt_x;
            current_y += FONT_LINE_HEIGHT;
            continue;
        }
        if (character == '\t') {
            current_x += 31;
            current_x -= current_x % 32;
            continue;
        }
        if (character == ' ') {
            current_x += FONT_CELL_WIDTH;
            continue;
        }
        if (character < '!' || character > 0x7e) character = 0x7f;

        if (current_y + FONT_CELL_HEIGHT > fnt_y + fnt_h) break;

        unsigned int glyph = character - ' ';
        int glyph_u = (int)(glyph % 16) * FONT_CELL_WIDTH;
        int glyph_v = (int)(glyph / 16) * FONT_CELL_HEIGHT;

        if (current_x + FONT_CELL_WIDTH <= fnt_x + fnt_w) {
            if ((size_t)(end - *next) < sizeof(FontGlyphPacket) + sizeof(FontPagePacket)) break;

            FontGlyphPacket* packet = (FontGlyphPacket*)*next;
            packet->tag = 4u << 24;
            packet->command = gp0_rectangle(true, true, true);
            packet->position = gp0_xy(current_x, current_y);
            packet->uv = gp0_uv(fnt_u + glyph_u, fnt_v + glyph_v, fnt_clut);
            packet->size = gp0_xy(FONT_CELL_WIDTH, FONT_CELL_HEIGHT);
            addPrim(ordering_table_entry, packet);

            *next += sizeof(*packet);
            emitted_glyph = true;
        }

        current_x += FONT_CELL_WIDTH;
    }

    if (emitted_glyph) {
        FontPagePacket* packet = (FontPagePacket*)*next;
        packet->tag = 1u << 24;
        packet->command = gp0_setPage(fnt_tpage, false, false);
        addPrim(ordering_table_entry, packet);
        *next += sizeof(*packet);
    }

    fnt_length = 0;
    fnt_buffer[0] = 0;
}

void RotMatrix_gte(const SVECTOR* rotation, MATRIX* output)
{
    int sx = sin12(rotation->vx);
    int cx = cos12(rotation->vx);
    int sy = sin12(rotation->vy);
    int cy = cos12(rotation->vy);
    int sz = sin12(rotation->vz);
    int cz = cos12(rotation->vz);
    MATRIX result = {0};

    // PsyQ RotMatrix_gte() composes rotations as Rx * Ry * Rz.
    result.m[0][0] = matrix_value((int64_t)cy * cz);
    result.m[0][1] = matrix_value(-(int64_t)cy * sz);
    result.m[0][2] = (int16_t)sy;
    result.m[1][0] = matrix_value(((int64_t)sx * sy * cz >> FIXED_SHIFT) + (int64_t)cx * sz);
    result.m[1][1] = matrix_value((int64_t)cx * cz - ((int64_t)sx * sy * sz >> FIXED_SHIFT));
    result.m[1][2] = matrix_value(-(int64_t)sx * cy);
    result.m[2][0] = matrix_value((int64_t)sx * sz - ((int64_t)cx * sy * cz >> FIXED_SHIFT));
    result.m[2][1] = matrix_value(((int64_t)cx * sy * sz >> FIXED_SHIFT) + (int64_t)sx * cz);
    result.m[2][2] = matrix_value((int64_t)cx * cy);
    *output = result;
}

void TransMatrix(MATRIX* matrix, const VECTOR* translation)
{
    matrix->t = *translation;
}

void ScaleMatrix(MATRIX* matrix, const VECTOR* scale)
{
    MATRIX result = *matrix;

    for (int row = 0; row < 3; row++) {
        for (int column = 0; column < 3; column++) {
            result.m[row][column] = matrix_value(
                (int64_t)matrix->m[row][column] * (&scale->vx)[column]);
        }
    }

    *matrix = result;
}

void MulMatrix0(const MATRIX* left, const MATRIX* right, MATRIX* output)
{
    MATRIX left_copy = *left;
    MATRIX right_copy = *right;
    MATRIX result = {.t = right_copy.t};

    gte_loadRotationMatrix((const GTEMatrix*)&left_copy);

    for (int column = 0; column < 3; column++) {
        gte_setV0(right_copy.m[0][column], right_copy.m[1][column], right_copy.m[2][column]);
        gte_command(GTE_CMD_MVMVA | GTE_SF | GTE_MX_RT | GTE_V_V0 | GTE_CV_NONE);
        result.m[0][column] = (int16_t)gte_getDataReg(GTE_IR1);
        result.m[1][column] = (int16_t)gte_getDataReg(GTE_IR2);
        result.m[2][column] = (int16_t)gte_getDataReg(GTE_IR3);
    }

    *output = result;
}

static void split_long_component(int32_t value, int16_t* high, int16_t* low)
{
    if (value < 0) {
        uint32_t magnitude = 0u - (uint32_t)value;
        *high = (int16_t)-(int32_t)(magnitude >> 15);
        *low = (int16_t)-(int32_t)(magnitude & 0x7fffu);
    } else {
        *high = (int16_t)((uint32_t)value >> 15);
        *low = (int16_t)((uint32_t)value & 0x7fffu);
    }
}

void CompMatrixLV(const MATRIX* left, const MATRIX* right, MATRIX* output)
{
    MATRIX left_copy = *left;
    MATRIX right_copy = *right;
    MATRIX result;
    VECTOR translated;
    MulMatrix0(&left_copy, &right_copy, &result);

    ApplyMatrixLV(&left_copy, &right_copy.t, &translated);
    result.t.vx = translated.vx + left_copy.t.vx;
    result.t.vy = translated.vy + left_copy.t.vy;
    result.t.vz = translated.vz + left_copy.t.vz;
    result.t.pad = 0;

    *output = result;
}

void ApplyMatrixLV(const MATRIX* matrix, const VECTOR* vector, VECTOR* output)
{
    int16_t high[3], low[3];
    int32_t high_result[3];
    VECTOR result = {0};

    split_long_component(vector->vx, &high[0], &low[0]);
    split_long_component(vector->vy, &high[1], &low[1]);
    split_long_component(vector->vz, &high[2], &low[2]);

    gte_loadRotationMatrix((const GTEMatrix*)matrix);
    gte_setDataReg(GTE_IR1, (uint16_t)high[0]);
    gte_setDataReg(GTE_IR2, (uint16_t)high[1]);
    gte_setDataReg(GTE_IR3, (uint16_t)high[2]);
    gte_command(GTE_CMD_MVMVA | GTE_MX_RT | GTE_V_IR | GTE_CV_NONE);
    high_result[0] = (int32_t)gte_getDataReg(GTE_MAC1);
    high_result[1] = (int32_t)gte_getDataReg(GTE_MAC2);
    high_result[2] = (int32_t)gte_getDataReg(GTE_MAC3);

    gte_setDataReg(GTE_IR1, (uint16_t)low[0]);
    gte_setDataReg(GTE_IR2, (uint16_t)low[1]);
    gte_setDataReg(GTE_IR3, (uint16_t)low[2]);
    gte_command(GTE_CMD_MVMVA | GTE_SF | GTE_MX_RT | GTE_V_IR | GTE_CV_NONE);

    result.vx = (int32_t)gte_getDataReg(GTE_MAC1) + high_result[0] * 8;
    result.vy = (int32_t)gte_getDataReg(GTE_MAC2) + high_result[1] * 8;
    result.vz = (int32_t)gte_getDataReg(GTE_MAC3) + high_result[2] * 8;

    *output = result;
}

void VectorNormalS(const VECTOR* vector, SVECTOR* output)
{
    int16_t x = (int16_t)vector->vx;
    int16_t y = (int16_t)vector->vy;
    int16_t z = (int16_t)vector->vz;

    int max_component = x < 0 ? -x : x;
    int absolute = y < 0 ? -y : y;
    if (absolute > max_component) max_component = absolute;
    absolute = z < 0 ? -z : z;
    if (absolute > max_component) max_component = absolute;
    if (max_component > 16383) {
        x >>= 1;
        y >>= 1;
        z >>= 1;
    }

    gte_setDataReg(GTE_IR1, (uint16_t)x);
    gte_setDataReg(GTE_IR2, (uint16_t)y);
    gte_setDataReg(GTE_IR3, (uint16_t)z);
    gte_command(GTE_CMD_SQR);

    int32_t squared_length = (int32_t)gte_getDataReg(GTE_MAC1) + (int32_t)gte_getDataReg(GTE_MAC2) + (int32_t)gte_getDataReg(GTE_MAC3);
    int length = SquareRoot12(squared_length) >> 6;

    if (!length) {
        output->vx = 0;
        output->vy = 0;
        output->vz = 0;
        output->pad = 0;
        return;
    }

    int reciprocal = (FIXED_ONE * FIXED_ONE) / length;
    if (reciprocal > 32767) {
        output->vx = (int16_t)clamp_s16(((int32_t)x * FIXED_ONE) / length);
        output->vy = (int16_t)clamp_s16(((int32_t)y * FIXED_ONE) / length);
        output->vz = (int16_t)clamp_s16(((int32_t)z * FIXED_ONE) / length);
        output->pad = 0;
        return;
    }

    gte_setDataReg(GTE_IR0, (uint16_t)reciprocal);
    gte_setDataReg(GTE_IR1, (uint16_t)x);
    gte_setDataReg(GTE_IR2, (uint16_t)y);
    gte_setDataReg(GTE_IR3, (uint16_t)z);
    gte_command(GTE_CMD_GPF | GTE_SF);

    output->vx = (int16_t)clamp_s16((int32_t)gte_getDataReg(GTE_MAC1));
    output->vy = (int16_t)clamp_s16((int32_t)gte_getDataReg(GTE_MAC2));
    output->vz = (int16_t)clamp_s16((int32_t)gte_getDataReg(GTE_MAC3));
    output->pad = 0;
}

void gte_SetGeomOffset(int x, int y)
{
    gte_setControlReg(GTE_OFX, x << 16);
    gte_setControlReg(GTE_OFY, y << 16);
}

void gte_SetGeomScreen(int z) { gte_setControlReg(GTE_H, z); }

void gte_SetBackColor(int red, int green, int blue)
{
    gte_setControlReg(GTE_RBK, red << 4);
    gte_setControlReg(GTE_GBK, green << 4);
    gte_setControlReg(GTE_BBK, blue << 4);
}

void gte_SetColorMatrix(const MATRIX* matrix) { gte_loadLightColorMatrix((const GTEMatrix*)matrix); }

void gte_SetLightMatrix(const MATRIX* matrix) { gte_loadLightMatrix((const GTEMatrix*)matrix); }

void gte_SetRotMatrix(const MATRIX* matrix) { gte_loadRotationMatrix((const GTEMatrix*)matrix); }

void gte_SetTransMatrix(const MATRIX* matrix)
{
    gte_setControlReg(GTE_TRX, matrix->t.vx);
    gte_setControlReg(GTE_TRY, matrix->t.vy);
    gte_setControlReg(GTE_TRZ, matrix->t.vz);
}

void gte_ldv0(const SVECTOR* vector) { gte_loadV0((const GTEVector16*)vector); }

void gte_ldv3(const SVECTOR* a, const SVECTOR* b, const SVECTOR* c)
{
    gte_loadV0((const GTEVector16*)a);
    gte_loadV1((const GTEVector16*)b);
    gte_loadV2((const GTEVector16*)c);
}

void gte_rtps(void) { gte_command(GTE_CMD_RTPS | GTE_SF); }

void gte_rtpt(void) { gte_command(GTE_CMD_RTPT | GTE_SF); }

void gte_stflg(int32_t* value) { *value = gte_getControlReg(GTE_FLAG); }

void gte_nclip(void) { gte_command(GTE_CMD_NCLIP); }

void gte_stopz(int32_t* value) { *value = gte_getDataReg(GTE_MAC0); }

void gte_avsz3(void) { gte_command(GTE_CMD_AVSZ3 | GTE_SF); }

void gte_stotz(int32_t* value) { *value = gte_getDataReg(GTE_OTZ); }

static void store_xy(int16_t* output, uint32_t value)
{
    output[0] = (int16_t)value;
    output[1] = (int16_t)(value >> 16);
}

void gte_stsxy(int16_t* output) { store_xy(output, gte_getDataReg(GTE_SXY2)); }

void gte_stsxy0(int16_t* output) { store_xy(output, gte_getDataReg(GTE_SXY0)); }

void gte_stsxy1(int16_t* output) { store_xy(output, gte_getDataReg(GTE_SXY1)); }

void gte_stsxy2(int16_t* output) { store_xy(output, gte_getDataReg(GTE_SXY2)); }

void gte_ldrgb(const void* input)
{
    const CVECTOR* color = input;
    gte_setDataReg(GTE_RGBC, color->r | ((uint32_t)color->g << 8) | ((uint32_t)color->b << 16));
}

void gte_nct(void) { gte_command(GTE_CMD_NCT | GTE_SF | GTE_LM); }

void gte_strgb(void* output)
{
    CVECTOR* color = output;
    uint32_t value = gte_getDataReg(GTE_RGB2);
    color->r = (uint8_t)value;
    color->g = (uint8_t)(value >> 8);
    color->b = (uint8_t)(value >> 16);
}

#include "header.h"
#include "psxcube.h"

#define HEAP_SIZE (1024 * 1024)
static char heap[HEAP_SIZE];

// TODO: struct to hold this ?
unsigned long long vsyncCounter;
unsigned long long timeCounter;

static GameContext ctx;

void vsync_callback()
{
    // VSync(-1);
    vsyncCounter++;

    if (vsyncCounter % 60 == 59) {
        timeCounter++;
        // printf("Time: %d\n" , timeCounter);
    }
}

// libdxpsx
// typedef void (*Callback)(void* context);
// void Run(Callback init, Callback update, void* context);

void run() {
    OnInit(&ctx);

    while (1) {
        OnUpdate(&ctx);
    }
}

int main(void)
{
#ifdef PSX_VER
    InitHeap3((void*)&heap, HEAP_SIZE);
    CdInit();
    pad_init();

    vsyncCounter = 0;
    timeCounter = 0;

    VSyncCallback(vsync_callback);

    run();
#else
    LibDXPSX_Run(OnInit, OnUpdate, &ctx);
#endif

    return 0;
}

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <switch.h>
#include <deko3d.h>

#include "helper.h"

void wait_for_input()
{
    for (;;)
    {
        hidScanInput();

        if (hidKeysDown(CONTROLLER_P1_AUTO) & KEY_A)
            break;

        consoleUpdate(NULL);
    }

    printf("\33[2K\r");
    consoleUpdate(NULL);
}

DkMemBlock make_memory_block(DkDevice device, size_t size, uint32_t flags)
{
    size = (size + DK_MEMBLOCK_ALIGNMENT - 1) & ~(DK_MEMBLOCK_ALIGNMENT - 1);

    DkMemBlockMaker maker;
    dkMemBlockMakerDefaults(&maker, device, (uint32_t)size);
    maker.flags = flags;
    return dkMemBlockCreate(&maker);
}

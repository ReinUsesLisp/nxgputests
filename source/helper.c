#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <switch.h>
#include <deko3d.h>

#include "unit_test_report.h"
#include "helper.h"

void wait_for_input()
{
    while (true)
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

bool compare_array(
    char const* name, uint32_t* results, FILE* report_file, size_t num_entries,
    uint32_t const* expected)
{
    bool pass = !memcmp(results, expected, num_entries * sizeof(uint32_t));
    unit_test_report(report_file, name, pass, num_entries, expected, results);
    return pass;
}
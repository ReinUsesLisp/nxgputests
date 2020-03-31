#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <switch.h>
#include <deko3d.h>

#include <unistd.h>

#include "compute_tests.h"
#include "graphics_tests.h"
#include "helper.h"
#include "unit_test_report.h"

static int nxlink_socket = -1;

void userAppInit()
{
    (void)nxlink_socket;

    consoleInit(NULL);

    /*
    if (R_FAILED(socketInitializeDefault()))
        return;

    nxlink_socket = nxlinkStdio();
    if (nxlink_socket < 0)
        socketExit();
    */
}

void userAppExit()
{
    consoleExit(NULL);

    /*
    if (nxlink_socket < 0)
        return;

    close(nxlink_socket);
    socketExit();
    nxlink_socket = -1;
    */
}

int main(int argc, char **argv)
{
    // TODO: Do proper parsing
    bool is_automatic = false;
    if (argc > 1)
    {
        if (0 == strcmp(argv[1], "--automatic"))
            is_automatic = true;
    }

    DkDeviceMaker device_mk;
    dkDeviceMakerDefaults(&device_mk);
    DkDevice device = dkDeviceCreate(&device_mk);

    DkQueueMaker queue_mk;
    dkQueueMakerDefaults(&queue_mk, device);
    queue_mk.perWarpScratchMemorySize = 8 * DK_PER_WARP_SCRATCH_MEM_ALIGNMENT;
    DkQueue queue = dkQueueCreate(&queue_mk);

    FILE* report_file = begin_unit_test_report();

    run_graphics_tests(device, queue, report_file, is_automatic);

    if (!is_automatic)
    {
        printf("Press A to continue...");
        wait_for_input();
    }

    run_compute_tests(device, queue, report_file, is_automatic);

    finish_unit_test_report(report_file);

    printf("\nPress A to exit...");
    wait_for_input();

    dkQueueDestroy(queue);
    dkDeviceDestroy(device);
}
#include <stdio.h>

#include <switch.h>
#include <deko3d.h>

#include <unistd.h>

#include "compute_tests.h"
#include "unit_test_report.h"

static int nxlink_socket = -1;

void userAppInit()
{
	consoleInit(NULL);

    if (R_FAILED(socketInitializeDefault()))
        return;

    nxlink_socket = nxlinkStdio();
    if (nxlink_socket < 0)
        socketExit();
}

void userAppExit()
{
	consoleExit(NULL);

	if (nxlink_socket < 0)
		return;

	close(nxlink_socket);
	socketExit();
	nxlink_socket = -1;
}

int main()
{
	DkDeviceMaker device_mk;
	dkDeviceMakerDefaults(&device_mk);
	DkDevice device = dkDeviceCreate(&device_mk);

	DkQueueMaker queue_mk;
	dkQueueMakerDefaults(&queue_mk, device);
	DkQueue queue = dkQueueCreate(&queue_mk);

	FILE* report_file = begin_unit_test_report();
	run_compute_tests(device, queue, report_file);
	finish_unit_test_report(report_file);

	while (appletMainLoop())
	{
		hidScanInput();

		if (hidKeysDown(CONTROLLER_P1_AUTO) & KEY_PLUS)
			break;

		consoleUpdate(NULL);
	}

	dkQueueDestroy(queue);
	dkDeviceDestroy(device);
}
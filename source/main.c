#include <stdio.h>

#include <switch.h>
#include <deko3d.h>

#include "compute_tests.h"

void userAppInit()
{
	consoleInit(NULL);
}

void userAppExit()
{
	consoleExit(NULL);
}

int main()
{
	DkDeviceMaker device_mk;
	dkDeviceMakerDefaults(&device_mk);
	DkDevice device = dkDeviceCreate(&device_mk);

	DkQueueMaker queue_mk;
	dkQueueMakerDefaults(&queue_mk, device);
	DkQueue queue = dkQueueCreate(&queue_mk);

	run_compute_tests(device, queue);

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
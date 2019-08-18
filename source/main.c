#include <stdio.h>

#include <switch.h>
#include <deko3d.h>

#include "foo_nvbin.h"
#include "dksh_gen.h"

#define CMDMEM_SIZE  (3 * DK_MEMBLOCK_ALIGNMENT)
#define CODEMEM_SIZE (512 * 1024)

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

	DkMemBlockMaker mem_mk;
	dkMemBlockMakerDefaults(&mem_mk, device, CMDMEM_SIZE);
	mem_mk.flags = DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached;
	DkMemBlock blk_cmdbuf = dkMemBlockCreate(&mem_mk);

	DkCmdBufMaker cmd_mk;
	dkCmdBufMakerDefaults(&cmd_mk, device);
	DkCmdBuf cmd = dkCmdBufCreate(&cmd_mk);
	dkCmdBufAddMemory(cmd, blk_cmdbuf, 0, CMDMEM_SIZE);

	mem_mk.size = CODEMEM_SIZE;
	mem_mk.flags = DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached
		| DkMemBlockFlags_Code;
	DkMemBlock blk_code = dkMemBlockCreate(&mem_mk);

	printf("Code memory at 0x%010lx\n", dkMemBlockGetGpuAddr(blk_code));
	generate_compute_dksh((uint8_t*)dkMemBlockGetCpuAddr(blk_code),
		foo_nvbin_size, foo_nvbin, 8, 1, 1, 1, 0, 0, 0);

	DkShaderMaker shader_mk;
	dkShaderMakerDefaults(&shader_mk, blk_code, 0);
	DkShader shader;
	dkShaderInitialize(&shader, &shader_mk);

	mem_mk.size = DK_MEMBLOCK_ALIGNMENT;
	mem_mk.flags = DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached;
	DkMemBlock blk_ssbo = dkMemBlockCreate(&mem_mk);
	printf("SSBO at 0x%010lx\n", dkMemBlockGetGpuAddr(blk_ssbo));
	printf("Size %d\n", dkMemBlockGetSize(blk_ssbo));

	dkCmdBufBindShader(cmd, &shader);
	dkCmdBufBindStorageBuffer(cmd, DkStage_Compute, 0,
	dkMemBlockGetGpuAddr(blk_ssbo), dkMemBlockGetSize(blk_ssbo));
	dkCmdBufDispatchCompute(cmd, 1, 1, 1);
	DkCmdList list = dkCmdBufFinishList(cmd);

	dkQueueSubmitCommands(queue, list);
	dkQueueWaitIdle(queue);

	uint32_t* results = (uint32_t*)dkMemBlockGetCpuAddr(blk_ssbo);
	printf("0x%08x\n", *results);

	while (appletMainLoop())
	{
		hidScanInput();

		if (hidKeysDown(CONTROLLER_P1_AUTO) & KEY_PLUS)
			break;

		consoleUpdate(NULL);
	}

	dkMemBlockDestroy(blk_ssbo);
	dkMemBlockDestroy(blk_code);
	dkCmdBufDestroy(cmd);
	dkMemBlockDestroy(blk_cmdbuf);
	dkQueueDestroy(queue);
	dkDeviceDestroy(device);
}
#include <stdbool.h>
#include <stdio.h>

#include <switch.h>
#include <deko3d.h>

#include "compute_tests.h"
#include "dksh_gen.h"

#include "constant_nvbin.h"
#include "hadd2_r_mrg_h0_nvbin.h"
#include "hadd2_r_mrg_h1_nvbin.h"
#include "hset2_r_f32_f32_nvbin.h"
#include "hset2_r_h1_h0_f32_nvbin.h"
#include "hset2_r_h1_h0_h1_h0_nvbin.h"
#include "hset2_r_h0_h0_f32_nvbin.h"
#include "r2p_imm_b0_nvbin.h"

#define CMDMEM_SIZE (3 * DK_MEMBLOCK_ALIGNMENT)
#define CODEMEM_SIZE (512 * 1024)
#define SSBO_SIZE (DK_MEMBLOCK_ALIGNMENT)

struct compute_test_descriptor
{
	char name[28];
	uint32_t expected_value;

	uint32_t const* code_size;
	uint8_t const* code;
	uint8_t num_gprs;

	bool (*check_results)(void*, FILE*);
	uint8_t workgroup_x_minus_1;
	uint8_t workgroup_y_minus_1;
	uint8_t workgroup_z_minus_1;
	uint8_t num_invokes_x_minus_1;
	uint8_t num_invokes_y_minus_1;
	uint8_t num_invokes_z_minus_1;
	uint16_t local_mem_size;
	uint16_t shared_mem_size;
	uint16_t num_barriers;
};

#define TEST(name, expected, id, num_gprs) \
	{ name, expected, &id##_nvbin_size, id##_nvbin, num_gprs }

static struct compute_test_descriptor const test_descriptors[] =
{
	TEST("Constant",            0xdeadbeef, constant,            8),
	TEST("HADD2_R.MRG_H0",      0xaaaa4200, hadd2_r_mrg_h0,      8),
	TEST("HADD2_R.MRG_H1",      0x4200aaaa, hadd2_r_mrg_h1,      8),
	TEST("HSET2_R F32 F32",     0x3c003c00, hset2_r_f32_f32,     8),
	TEST("HSET2_R H1_H0 F32",   0x00003c00, hset2_r_h1_h0_f32,   8),
	TEST("HSET2_R H1_H0 H1_H0", 0xffff0000, hset2_r_h1_h0_h1_h0, 8),
	TEST("HSET2_R H0_H0 F32",   0x3c003c00, hset2_r_h0_h0_f32,   8),
	TEST("R2P_IMM.B0",          0x0000aaaa, r2p_imm_b0,          8),
};

#define NUM_TESTS (sizeof(test_descriptors) / sizeof(test_descriptors[0]))

static float to_seconds(u64 time)
{
    return (time * 625 / 12) / 1000000000.0f;
}

static DkMemBlock make_memory_block(DkDevice device, uint32_t size, uint32_t flags)
{
	DkMemBlockMaker maker;
	dkMemBlockMakerDefaults(&maker, device, size);
	maker.flags = flags;
	return dkMemBlockCreate(&maker);
}

static bool execute_test(
	struct compute_test_descriptor const* desc, DkQueue queue,
	DkMemBlock blk_code, uint8_t* code, DkCmdBuf cmdbuf,
	DkMemBlock blk_cmdbuf, void* results, FILE* error_file)
{
	generate_compute_dksh(code, *desc->code_size, desc->code, desc->num_gprs,
		desc->workgroup_x_minus_1 + 1, desc->workgroup_y_minus_1 + 1,
		desc->workgroup_z_minus_1 + 1, desc->local_mem_size,
		desc->shared_mem_size, desc->num_barriers);

	DkShader shader;
	DkShaderMaker shader_mk;
	dkShaderMakerDefaults(&shader_mk, blk_code, 0);
	dkShaderInitialize(&shader, &shader_mk);

	dkCmdBufAddMemory(cmdbuf, blk_cmdbuf, 0, CMDMEM_SIZE);
	dkCmdBufBindShader(cmdbuf, &shader);
	dkCmdBufDispatchCompute(cmdbuf, desc->num_invokes_x_minus_1 + 1,
		desc->num_invokes_y_minus_1 + 1, desc->num_invokes_z_minus_1 + 1);
	DkCmdList list = dkCmdBufFinishList(cmdbuf);

	dkQueueSubmitCommands(queue, list);
	dkQueueWaitIdle(queue);

	if (desc->check_results)
		return desc->check_results(results, error_file);

	uint32_t result = *(uint32_t*)results;
	if (result == desc->expected_value)
		return true;

	fprintf(error_file, "%s expected 0x%08x, got 0x%08x\n", desc->name,
		desc->expected_value, result);
	return false;
}

void run_compute_tests(DkDevice device, DkQueue queue)
{
	DkMemBlock blk_cmdbuf = make_memory_block(device, CMDMEM_SIZE,
		DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);

	DkMemBlock blk_code = make_memory_block(device, CODEMEM_SIZE,
		DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached
		| DkMemBlockFlags_Code);
	uint8_t* code_data = dkMemBlockGetCpuAddr(blk_code);

	DkMemBlock blk_ssbo = make_memory_block(device, SSBO_SIZE,
		DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
	DkGpuAddr ssbo_gpu_addr = dkMemBlockGetGpuAddr(blk_ssbo);
	uint8_t* ssbo_data = dkMemBlockGetCpuAddr(blk_ssbo);

	DkCmdBufMaker cmd_mk;
	dkCmdBufMakerDefaults(&cmd_mk, device);
	DkCmdBuf cmdbuf = dkCmdBufCreate(&cmd_mk);
	dkCmdBufAddMemory(cmdbuf, blk_cmdbuf, 0, CMDMEM_SIZE);

	dkCmdBufBindStorageBuffer(cmdbuf, DkStage_Compute, 0, ssbo_gpu_addr, SSBO_SIZE);
	DkCmdList list = dkCmdBufFinishList(cmdbuf);

	dkQueueSubmitCommands(queue, list);
	dkQueueWaitIdle(queue);

	printf("Running compute tests...\n\n");

	FILE* error_file = fopen("nxgputests_error.txt", "w");
	u64 real_start_time = armGetSystemTick();

	size_t failures = 0;
	for (size_t i = 0; i < NUM_TESTS; ++i)
	{
		struct compute_test_descriptor const* desc = &test_descriptors[i];

		int written_chars =
			printf("%2zd/%2zd Test: %s ", i + 1, NUM_TESTS, desc->name);
		for (int i = 0; i < 40 - written_chars; ++i)
			putc('.', stdout);
		putc(' ', stdout);

		consoleUpdate(NULL);

		if (execute_test(desc, queue, blk_code, code_data, cmdbuf,
				blk_cmdbuf, ssbo_data, error_file))
		{
			puts("Passed");
		}
		else
		{
			puts("Failed");
			++failures;
		}

		consoleUpdate(NULL);
	}

	printf("\n%3d%% tests passed, %zd tests failed out of %zd\n\n"
		"Total Test time (real) = %.2f sec\n",
		(int)((NUM_TESTS - failures) * 100 / (float)NUM_TESTS), failures,
		NUM_TESTS, to_seconds(armGetSystemTick() - real_start_time));

	fclose(error_file);

	consoleUpdate(NULL);

	dkMemBlockDestroy(blk_ssbo);
	dkMemBlockDestroy(blk_code);
	dkCmdBufDestroy(cmdbuf);
	dkMemBlockDestroy(blk_cmdbuf);
}
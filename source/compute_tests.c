#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <switch.h>
#include <deko3d.h>

#include "compute_tests.h"
#include "dksh_gen.h"
#include "unit_test_report.h"

#include "constant_nvbin.h"
#include "f2f_r_f32_f32_nvbin.h"
#include "f2f_r_f32_f32_abs_nvbin.h"
#include "f2f_r_f32_f32_neg_nvbin.h"
#include "f2f_r_f32_f32_sat_nvbin.h"
#include "f2f_r_f32_f32_sat_neg_nvbin.h"
#include "f2f_r_f32_f32_round_1_nvbin.h"
#include "f2f_r_f32_f32_round_2_nvbin.h"
#include "f2f_r_f32_f32_floor_nvbin.h"
#include "f2f_r_f32_f32_ceil_nvbin.h"
#include "f2f_r_f32_f32_trunc_nvbin.h"
#include "f2f_r_f32_f16_nvbin.h"
#include "f2f_r_f32_f16_abs_nvbin.h"
#include "f2f_r_f32_f16_neg_nvbin.h"
#include "hadd2_r_nvbin.h"
#include "hadd2_r_h1h1_h1h0_nvbin.h"
#include "hadd2_r_h0h0_h1h0_nvbin.h"
#include "hadd2_r_h1h0_h1h1_nvbin.h"
#include "hadd2_r_h1h0_h0h0_nvbin.h"
#include "hadd2_r_nh1h0_nh1h0_nvbin.h"
#include "hadd2_r_ah1h0_ah1h0_nvbin.h"
#include "hadd2_r_ah1h0_nh1h0_nvbin.h"
#include "hadd2_r_nah1h0_nah1h0_nvbin.h"
#include "hadd2_r_f32_f32_nvbin.h"
#include "hadd2_r_nf32_f32_nvbin.h"
#include "hadd2_r_f32_nf32_nvbin.h"
#include "hadd2_r_sat_f32_f32_nvbin.h"
#include "hadd2_r_mrg_h0_f32_f32_nvbin.h"
#include "hadd2_r_mrg_h1_f32_f32_nvbin.h"
#include "hadd2_imm_nvbin.h"
#include "hmul2_r_nvbin.h"
#include "hmul2_r_h1h1_h1h0_nvbin.h"
#include "hmul2_r_h0h0_h1h0_nvbin.h"
#include "hmul2_r_h1h0_h1h1_nvbin.h"
#include "hmul2_r_h1h0_h0h0_nvbin.h"
#include "hmul2_r_ah1h0_ah1h0_nvbin.h"
#include "hmul2_r_ah1h0_nh1h0_nvbin.h"
#include "hmul2_r_f32_f32_nvbin.h"
#include "hmul2_r_f32_nf32_nvbin.h"
#include "hmul2_r_sat_f32_f32_nvbin.h"
#include "hmul2_r_mrg_h0_f32_f32_nvbin.h"
#include "hmul2_r_mrg_h1_f32_f32_nvbin.h"
#include "hfma2_rr_nvbin.h"
#include "hfma2_rr_f32_nvbin.h"
#include "hfma2_rr_mrg_h0_nvbin.h"
#include "hfma2_rr_mrg_h1_nvbin.h"
#include "hfma2_rr_h1h0_nh1h0_h1h0_nvbin.h"
#include "hfma2_rr_h1h0_h1h0_nh1h0_nvbin.h"
#include "hset2_r_f32_f32_nvbin.h"
#include "hset2_r_h1h0_f32_nvbin.h"
#include "hset2_r_h1h0_h1h0_nvbin.h"
#include "hset2_r_h0h0_f32_nvbin.h"
#include "hset2_r_h0h0_h1h1_nvbin.h"
#include "hsetp2_r_p39_nvbin.h"
#include "hsetp2_r_f32_f32_nvbin.h"
#include "hsetp2_r_h1h0_f32_nvbin.h"
#include "hsetp2_r_f32_ah1h0_nvbin.h"
#include "hsetp2_r_f32_nh1h0_nvbin.h"
#include "hsetp2_r_hand_h1h0_f32_nvbin.h"
#include "r2p_imm_b0_nvbin.h"
#include "shfl_idx_nvbin.h"
#include "shfl_up_nvbin.h"
#include "shfl_down_nvbin.h"

#define CMDMEM_SIZE (3 * DK_MEMBLOCK_ALIGNMENT)
#define CODEMEM_SIZE (512 * 1024)
#define SSBO_SIZE (DK_MEMBLOCK_ALIGNMENT)

#define DECLARE_TEST(id) \
	static bool test_##id(char const* name, void* results, FILE* report_file);

#define TEST(name, expected, id, num_gprs) \
	{ name, expected, &id##_nvbin_size, id##_nvbin, num_gprs }

#define FULLTEST(name, id, num_gprs, workgroup_x, workgroup_y, workgroup_z, \
	num_invokes_x, num_invokes_y, num_invokes_z, local_mem_size,            \
	shared_mem_size, num_barriers)                                          \
	{ name, 0, &id##_nvbin_size, id##_nvbin, num_gprs, test_##id,           \
	  (workgroup_x) - 1, (workgroup_y) - 1, (workgroup_z) - 1,              \
	  (num_invokes_x) - 1, (num_invokes_y) - 1, (num_invokes_z) - 1,        \
	  local_mem_size, shared_mem_size, num_barriers }

struct compute_test_descriptor
{
	char name[28];
	uint32_t expected_value;

	uint32_t const* code_size;
	uint8_t const* code;
	uint8_t num_gprs;

	bool (*check_results)(char const*, void*, FILE*);
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

DECLARE_TEST(shfl_idx)
DECLARE_TEST(shfl_up)
DECLARE_TEST(shfl_down)

static struct compute_test_descriptor const test_descriptors[] =
{
	TEST("Constant",                    0xdeadbeef, constant,                 8),
	TEST("F2F_R.F32.F32",               0x40e00000, f2f_r_f32_f32,            8),
	TEST("F2F_R.F32.F32 |Ra|",          0x40e00000, f2f_r_f32_f32_abs,        8),
	TEST("F2F_R.F32.F32 -Ra",           0xc0e00000, f2f_r_f32_f32_neg,        8),
	TEST("F2F_R.F32.F32.SAT",           0x3f800000, f2f_r_f32_f32_sat,        8),
	TEST("F2F_R.F32.F32.SAT -Ra",       0x00000000, f2f_r_f32_f32_sat_neg,    8),
	TEST("F2F_R.F32.F32.ROUND 1",       0x40800000, f2f_r_f32_f32_round_1,    8),
	TEST("F2F_R.F32.F32.ROUND 2",       0x40800000, f2f_r_f32_f32_round_2,    8),
	TEST("F2F_R.F32.F32.FLOOR",         0xc2300000, f2f_r_f32_f32_floor,      8),
	TEST("F2F_R.F32.F32.CEIL",          0xc22c0000, f2f_r_f32_f32_ceil,       8),
	TEST("F2F_R.F32.F32.TRUNC",         0xc22c0000, f2f_r_f32_f32_trunc,      8),
	TEST("F2F_R.F32.F16",               0x00003c00, f2f_r_f32_f16,            8),
	TEST("F2F_R.F32.F16 |Ra|",          0x00004800, f2f_r_f32_f16_abs,        8),
	TEST("F2F_R.F32.F16 -Ra",           0x0000bc00, f2f_r_f32_f16_neg,        8),
	TEST("HADD2_R",                     0x40004400, hadd2_r,                  8),
	TEST("HADD2_R H1_H1 H1_H0",         0x40000000, hadd2_r_h1h1_h1h0,        8),
	TEST("HADD2_R H0_H0 H1_H0",         0x46004400, hadd2_r_h0h0_h1h0,        8),
	TEST("HADD2_R H1_H0 H1_H1",         0x40004600, hadd2_r_h1h0_h1h1,        8),
	TEST("HADD2_R H1_H0 H0_H0",         0x00004400, hadd2_r_h1h0_h0h0,        8),
	TEST("HADD2_R -H1_H0 -H1_H0",       0xc000c400, hadd2_r_nh1h0_nh1h0,      8),
	TEST("HADD2_R |H1_H0| |H1_H0|",     0x47004000, hadd2_r_ah1h0_ah1h0,      8),
	TEST("HADD2_R |H1_H0| -H1_H0",      0xc5004000, hadd2_r_ah1h0_nh1h0,      8),
	TEST("HADD2_R -|H1_H0| -|H1_H0|",   0xc700c000, hadd2_r_nah1h0_nah1h0,    8),
	TEST("HADD2_R F32 F32",             0x42004200, hadd2_r_f32_f32,          8),
	TEST("HADD2_R -F32 F32",            0xbc00bc00, hadd2_r_nf32_f32,         8),
	TEST("HADD2_R F32 -F32",            0xbc00bc00, hadd2_r_f32_nf32,         8),
	TEST("HADD2_R.SAT F32 F32",         0x3c003c00, hadd2_r_sat_f32_f32,      8),
	TEST("HADD2_R.MRG_H0 F32 F32",      0xaaaa4200, hadd2_r_mrg_h0_f32_f32,   8),
	TEST("HADD2_R.MRG_H1 F32 F32",      0x4200aaaa, hadd2_r_mrg_h1_f32_f32,   8),
	TEST("HADD2_IMM",                   0x4b804400, hadd2_imm,                8),
	TEST("HMUL2_R",                     0xc2004200, hmul2_r,                  8),
	TEST("HMUL2_R H1_H1 H1_H0",         0xc200bc00, hmul2_r_h1h1_h1h0,        8),
	TEST("HMUL2_R H0_H0 H1_H0",         0x48804200, hmul2_r_h0h0_h1h0,        8),
	TEST("HMUL2_R H1_H0 H1_H1",         0xc2004880, hmul2_r_h1h0_h1h1,        8),
	TEST("HMUL2_R H1_H0 H0_H0",         0xbc004200, hmul2_r_h1h0_h0h0,        8),
	TEST("HMUL2_R |H1_H0| |H1_H0|",     0x46004400, hmul2_r_ah1h0_ah1h0,      8),
	TEST("HMUL2_R |H1_H0| -H1_H0",      0xc6004410, hmul2_r_ah1h0_nh1h0,      8),
	TEST("HMUL2_R F32 F32",             0x40004000, hmul2_r_f32_f32,          8),
	TEST("HMUL2_R F32 -F32",            0xc000c000, hmul2_r_f32_nf32,         8),
	TEST("HMUL2_R.SAT F32 F32",         0x3c003c00, hmul2_r_sat_f32_f32,      8),
	TEST("HMUL2_R.MRG_H0 F32 F32",      0xaaaa4000, hmul2_r_mrg_h0_f32_f32,   8),
	TEST("HMUL2_R.MRG_H1 F32 F32",      0x4000aaaa, hmul2_r_mrg_h1_f32_f32,   8),
	TEST("HFMA2_RR",                    0x47004000, hfma2_rr,                 8),
	TEST("HFMA2_RR.F32",                0x40000000, hfma2_rr_f32,             8),
	TEST("HFMA2_RR.MRG_H0",             0xcccc4000, hfma2_rr_mrg_h0,          8),
	TEST("HFMA2_RR.MRG_H1",             0x4700cccc, hfma2_rr_mrg_h1,          8),
	TEST("HFMA2_RR H1_H0 -H1_H0 H1_H0", 0x48804400, hfma2_rr_h1h0_nh1h0_h1h0, 8),
	TEST("HFMA2_RR H1_H0 H1_H0 -H1_H0", 0xc880c400, hfma2_rr_h1h0_h1h0_nh1h0, 8),
	TEST("HSET2_R F32 F32",             0x3c003c00, hset2_r_f32_f32,          8),
	TEST("HSET2_R H1_H0 F32",           0x00003c00, hset2_r_h1h0_f32,         8),
	TEST("HSET2_R H0_H0 F32",           0x3c003c00, hset2_r_h0h0_f32,         8),
	TEST("HSET2_R H0_H0 H1_H1",         0xffffffff, hset2_r_h0h0_h1h1,        8),
	TEST("HSET2_R H1_H0 H1_H0",         0xffff0000, hset2_r_h1h0_h1h0,        8),
	TEST("HSETP2_R P39",                0xccccccc2, hsetp2_r_p39,             8),
	TEST("HSETP2_R F32 F32",            0x00010008, hsetp2_r_f32_f32,         8),
	TEST("HSETP2_R H1_H0 F32",          0x00000008, hsetp2_r_h1h0_f32,        8),
	TEST("HSETP2_R F32 |H1_H0|",        0x00010008, hsetp2_r_f32_ah1h0,       8),
	TEST("HSETP2_R F32 -H1_H0",         0x00010008, hsetp2_r_f32_nh1h0,       8),
	TEST("HSETP2_R.H_AND H1_H0 F32",    0x0000a008, hsetp2_r_hand_h1h0_f32,   8),
	TEST("R2P_IMM.B0",                  0x0000aaaa, r2p_imm_b0,               8),

	FULLTEST("SHFL.IDX",  shfl_idx,  8, 8, 1, 1, 1, 1, 1, 0, 0, 0),
	FULLTEST("SHFL.UP",   shfl_up,   8, 8, 1, 1, 1, 1, 1, 0, 0, 0),
	FULLTEST("SHFL.DOWN", shfl_down, 8, 8, 1, 1, 1, 1, 1, 0, 0, 0),
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
	DkMemBlock blk_cmdbuf, void* results, FILE* report_file)
{
	generate_compute_dksh(code, *desc->code_size, desc->code, desc->num_gprs,
		desc->workgroup_x_minus_1 + 1, desc->workgroup_y_minus_1 + 1,
		desc->workgroup_z_minus_1 + 1, desc->local_mem_size,
		desc->shared_mem_size, desc->num_barriers);

	DkShader shader;
	DkShaderMaker shader_mk;
	dkShaderMakerDefaults(&shader_mk, blk_code, 0);
	dkShaderInitialize(&shader, &shader_mk);

	dkCmdBufClear(cmdbuf);
	dkCmdBufBindShader(cmdbuf, &shader);
	dkCmdBufDispatchCompute(cmdbuf, desc->num_invokes_x_minus_1 + 1,
		desc->num_invokes_y_minus_1 + 1, desc->num_invokes_z_minus_1 + 1);
	DkCmdList list = dkCmdBufFinishList(cmdbuf);

	dkQueueSubmitCommands(queue, list);
	dkQueueWaitIdle(queue);

	if (desc->check_results)
		return desc->check_results(desc->name, results, report_file);

	bool pass = *(uint32_t*)results == desc->expected_value;
	unit_test_report(report_file, desc->name, pass, 1, &desc->expected_value,
		results);
	return pass;
}

void run_compute_tests(DkDevice device, DkQueue queue, FILE* report_file)
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

	u64 real_start_time = armGetSystemTick();

	size_t failures = 0;
	for (size_t i = 0; i < NUM_TESTS; ++i)
	{
		struct compute_test_descriptor const* desc = &test_descriptors[i];

		int written_chars =
			printf("%2zd/%2zd Test: %s", i + 1, NUM_TESTS, desc->name);
		for (int i = 0; i < 41 - written_chars; ++i)
			putc('.', stdout);
		putc(' ', stdout);

		consoleUpdate(NULL);

		bool pass = execute_test(desc, queue, blk_code, code_data, cmdbuf,
			blk_cmdbuf, ssbo_data, report_file);
		if (!pass)
			++failures;
		puts(pass ? "Passed" : "Failed");

		consoleUpdate(NULL);
	}

	printf("\n%3d%% tests passed, %zd tests failed out of %zd\n\n"
		"Total Test time (real) = %.2f sec\n",
		(int)((NUM_TESTS - failures) * 100 / (float)NUM_TESTS), failures,
		NUM_TESTS, to_seconds(armGetSystemTick() - real_start_time));

	consoleUpdate(NULL);

	dkMemBlockDestroy(blk_ssbo);
	dkMemBlockDestroy(blk_code);
	dkCmdBufDestroy(cmdbuf);
	dkMemBlockDestroy(blk_cmdbuf);
}

static bool compare_array(char const* name, void* results, FILE* report_file,
	size_t num_entries, uint32_t const* expected)
{
	bool pass = !memcmp(results, expected, num_entries * sizeof(uint32_t));
	unit_test_report(report_file, name, pass, num_entries, expected, results);
	return pass;
}

static bool test_shfl_idx(char const* name, void* results, FILE* report_file)
{
	static uint32_t const expected[] = {0x4444, 0x4444, 0x4444, 0x4444,
		                                0x4444, 0x4444, 0x4444, 0x4444};
	return compare_array(name, results, report_file, 8, expected);
}

static bool test_shfl_up(char const* name, void* results, FILE* report_file)
{
	static uint32_t const expected[] = {0xdead, 0xdead, 0xdead, 0x0000,
	                                    0x2222, 0x4444, 0x6666, 0x8888, };
	return compare_array(name, results, report_file, 8, expected);
}

static bool test_shfl_down(char const* name, void* results, FILE* report_file)
{
	static uint32_t const expected[] = {0x4444, 0x6666, 0x8888, 0xaaaa,
	                                    0xcccc, 0xeeee, 0xdead, 0xdead};
	return compare_array(name, results, report_file, 8, expected);
}

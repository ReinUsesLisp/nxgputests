#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <switch.h>
#include <deko3d.h>

#include "compute_tests.h"
#include "dksh_gen.h"
#include "unit_test_report.h"

#include "constant_nvbin.h"
#include "shr_r_s32_nvbin.h"
#include "shr_r_u32_nvbin.h"
#include "shr_r_u32_w_nvbin.h"
#include "shr_r_u32_clamped_nvbin.h"
#include "shr_imm_s32_nvbin.h"
#include "shr_imm_u32_nvbin.h"
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
#include "f2f_r_f16_f16_nvbin.h"
#include "f2f_r_f16_f16_h1_nvbin.h"
#include "f2f_r_f16_f16_abs_nvbin.h"
#include "f2f_r_f16_f16_neg_nvbin.h"
#include "f2f_r_f16_f16_sat_nvbin.h"
#include "f2f_r_f16_f16_sat_neg_nvbin.h"
#include "f2f_r_f16_f16_round_nvbin.h"
#include "f2f_r_f16_f16_floor_nvbin.h"
#include "f2f_r_f16_f16_ceil_nvbin.h"
#include "f2f_r_f16_f16_trunc_nvbin.h"
#include "f2i_r_u32_f16_h0_nvbin.h"
#include "f2i_r_u32_f16_h1_nvbin.h"
#include "f2i_r_u32_f16_ah1_nvbin.h"
#include "f2i_r_u32_f16_nh1_nvbin.h"
#include "f2i_r_u32_f16_rounding_nvbin.h"
#include "f2i_r_u32_f16_clamped_nvbin.h"
#include "f2i_r_u32_f16_floor_nvbin.h"
#include "f2i_r_u32_f16_ceil_nvbin.h"
#include "f2i_r_u32_f16_trunc_nvbin.h"
#include "f2i_r_s32_f16_h0_nvbin.h"
#include "f2i_r_s32_f16_h1_nvbin.h"
#include "f2i_r_s32_f16_ah1_nvbin.h"
#include "f2i_r_s32_f16_nh1_nvbin.h"
#include "f2i_r_s32_f16_floor_nvbin.h"
#include "f2i_r_s32_f16_ceil_nvbin.h"
#include "f2i_r_s32_f16_trunc_nvbin.h"
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
#include "r2p_imm_b0_pr_nvbin.h"
#include "r2p_imm_b1_pr_nvbin.h"
#include "r2p_imm_b2_pr_nvbin.h"
#include "r2p_imm_b3_pr_nvbin.h"
#include "shared_memory_nvbin.h"
#include "sts_indirect_nvbin.h"
#include "lds_indirect_nvbin.h"
#include "shfl_idx_nvbin.h"
#include "shfl_up_nvbin.h"
#include "shfl_down_nvbin.h"

#define CMDMEM_SIZE (3 * DK_MEMBLOCK_ALIGNMENT)
#define CODEMEM_SIZE (512 * 1024)
#define SSBO_SIZE (DK_MEMBLOCK_ALIGNMENT)

#define DECLARE_TEST(id) \
	static bool test_##id(char const* name, void* results, FILE* report_file);

#define TEST(name, expected, id, num_gprs)                    \
	{ name, expected, &id##_nvbin_size, id##_nvbin, num_gprs, \
	  .shared_mem_size = 512 }

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
	TEST("SHR_R.S32",                   0xff000000, shr_r_s32,                8),
	TEST("SHR_R.U32",                   0x0f000000, shr_r_u32,                8),
	TEST("SHR_R.U32.W",                 0x0000ff00, shr_r_u32_w,              8),
	TEST("SHR_R.U32 Clamped",           0x00000001, shr_r_u32_clamped,        8),
	TEST("SHR_IMM.S32",                 0xff000000, shr_imm_s32,              8),
	TEST("SHR_IMM.U32",                 0x0f000000, shr_imm_u32,              8),
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
	TEST("F2F_R.F16.F16",               0x00004a33, f2f_r_f16_f16,            8),
	TEST("F2F_R.F16.F16 Ra.H1",         0x000050c0, f2f_r_f16_f16_h1,         8),
	TEST("F2F_R.F16.F16 |Ra|",          0x00005595, f2f_r_f16_f16_abs,        8),
	TEST("F2F_R.F16.F16 -Ra",           0x000055c6, f2f_r_f16_f16_neg,        8),
	TEST("F2F_R.F16.F16.SAT",           0x00000000, f2f_r_f16_f16_sat,        8),
	TEST("F2F_R.F16.F16.SAT -Ra",       0x00003c00, f2f_r_f16_f16_sat_neg,    8),
	TEST("F2F_R.F16.F16.ROUND",         0x0000c900, f2f_r_f16_f16_round,      8),
	TEST("F2F_R.F16.F16.FLOOR",         0x0000c900, f2f_r_f16_f16_floor,      8),
	TEST("F2F_R.F16.F16.CEIL",          0x0000c880, f2f_r_f16_f16_ceil,       8),
	TEST("F2F_R.F16.F16.TRUNC",         0x0000cb00, f2f_r_f16_f16_trunc,      8),
	TEST("F2I_R.U32.F16 Ra.H0",         0x00000017, f2i_r_u32_f16_h0,         8),
	TEST("F2I_R.U32.F16 Ra.H1",         0x00000035, f2i_r_u32_f16_h1,         8),
	TEST("F2I_R.U32.F16 |Ra.H1|",       0x0000003f, f2i_r_u32_f16_ah1,        8),
	TEST("F2I_R.U32.F16 -Ra.H1",        0x0000003f, f2i_r_u32_f16_nh1,        8),
	TEST("F2I_R.U32.F16 Rounding",      0x00000005, f2i_r_u32_f16_rounding,   8),
	TEST("F2I_R.U32.F16 Clamped",       0x00000001, f2i_r_u32_f16_clamped,    8),
	TEST("F2I_R.U32.F16.FLOOR",         0x00000004, f2i_r_u32_f16_floor,      8),
	TEST("F2I_R.U32.F16.CEIL",          0x00000008, f2i_r_u32_f16_ceil,       8),
	TEST("F2I_R.U32.F16.TRUNC",         0x00000007, f2i_r_u32_f16_trunc,      8),
	TEST("F2I_R.S32.F16 Ra.H0",         0x00000008, f2i_r_s32_f16_h0,         8),
	TEST("F2I_R.S32.F16 Ra.H1",         0xfffffff8, f2i_r_s32_f16_h1,         8),
	TEST("F2I_R.S32.F16 |Ra.H1|",       0x00000020, f2i_r_s32_f16_ah1,        8),
	TEST("F2I_R.S32.F16 -Ra.H1",        0xffffffb4, f2i_r_s32_f16_nh1,        8),
	TEST("F2I_R.S32.F16.FLOOR",         0xfffffffd, f2i_r_s32_f16_floor,      8),
	TEST("F2I_R.S32.F16.CEIL",          0xfffffffe, f2i_r_s32_f16_ceil,       8),
	TEST("F2I_R.S32.F16.TRUNC",         0xfffffff7, f2i_r_s32_f16_trunc,      8),
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
	TEST("R2P_IMM.B0 PR",               0x0000aaaa, r2p_imm_b0_pr,            8),
	TEST("R2P_IMM.B1 PR",               0x0000bbbb, r2p_imm_b1_pr,            8),
	TEST("R2P_IMM.B2 PR",               0x0000cccc, r2p_imm_b2_pr,            8),
	TEST("R2P_IMM.B3 PR",               0x0000dddd, r2p_imm_b3_pr,            8),
	TEST("LDS+STS",                     0xa0a0a0a0, shared_memory,            8),
	TEST("STS Indirect",                0xdeadcafe, sts_indirect,             8),
	TEST("LDS Indirect",                0xcafedead, lds_indirect,             8),

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
	struct compute_test_descriptor const* test, DkQueue queue,
	DkMemBlock blk_code, uint8_t* code, DkCmdBuf cmdbuf,
	DkMemBlock blk_cmdbuf, void* results, FILE* report_file)
{
	generate_compute_dksh(code, *test->code_size, test->code, test->num_gprs,
		test->workgroup_x_minus_1 + 1, test->workgroup_y_minus_1 + 1,
		test->workgroup_z_minus_1 + 1, test->local_mem_size,
		test->shared_mem_size, test->num_barriers);

	DkShader shader;
	DkShaderMaker shader_mk;
	dkShaderMakerDefaults(&shader_mk, blk_code, 0);
	dkShaderInitialize(&shader, &shader_mk);

	dkCmdBufClear(cmdbuf);
	dkCmdBufBindShader(cmdbuf, &shader);
	dkCmdBufDispatchCompute(cmdbuf, test->num_invokes_x_minus_1 + 1,
		test->num_invokes_y_minus_1 + 1, test->num_invokes_z_minus_1 + 1);
	DkCmdList list = dkCmdBufFinishList(cmdbuf);

	dkQueueSubmitCommands(queue, list);
	dkQueueWaitIdle(queue);

	if (test->check_results)
		return test->check_results(test->name, results, report_file);

	bool pass = *(uint32_t*)results == test->expected_value;
	unit_test_report(report_file, test->name, pass, 1, &test->expected_value,
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
		struct compute_test_descriptor const* test = &test_descriptors[i];

		int written_chars =
			printf("%2zd/%2zd Test: %s", i + 1, NUM_TESTS, test->name);
		for (int i = 0; i < 41 - written_chars; ++i)
			putc('.', stdout);
		putc(' ', stdout);

		consoleUpdate(NULL);

		bool pass = execute_test(test, queue, blk_code, code_data, cmdbuf,
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
	static uint32_t const expected[] = {2, 2, 2, 2, 2, 2, 2, 2};
	return compare_array(name, results, report_file, 8, expected);
}

static bool test_shfl_up(char const* name, void* results, FILE* report_file)
{
	static uint32_t const expected[] = {0xdead, 0xdead, 0xdead, 0, 1, 2, 3, 4};
	return compare_array(name, results, report_file, 8, expected);
}

static bool test_shfl_down(char const* name, void* results, FILE* report_file)
{
	static uint32_t const expected[] = {2, 3, 4, 5, 6, 7, 0xdead, 0xdead};
	return compare_array(name, results, report_file, 8, expected);
}

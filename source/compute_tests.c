#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <switch.h>
#include <deko3d.h>

#include "compute_tests.h"
#include "dksh_gen.h"
#include "unit_test_report.h"
#include "helper.h"

#define CMDMEM_SIZE (3 * DK_MEMBLOCK_ALIGNMENT)
#define CODEMEM_SIZE (512 * 1024)
#define SSBO_SIZE (DK_MEMBLOCK_ALIGNMENT)

#define TEST(name, expected, id, num_gprs)                    \
	{ name, expected, &id##_nvbin_size, id##_nvbin, num_gprs, \
	  .shared_mem_size = 512 }

#define ETEST(name, expected, id, num_gprs)                   \
	{ name, expected, &id##_nvbin_size, id##_nvbin, num_gprs, \
	  execute_test_##id }

#define MTEST(name, id, num_gprs, workgroup_x, workgroup_y, workgroup_z, \
	num_invokes_x, num_invokes_y, num_invokes_z, local_mem_size,         \
	shared_mem_size, num_barriers)                                       \
	{ name, 0, &id##_nvbin_size, id##_nvbin, num_gprs, NULL, test_##id,  \
	  (workgroup_x) - 1, (workgroup_y) - 1, (workgroup_z) - 1,           \
	  (num_invokes_x) - 1, (num_invokes_y) - 1, (num_invokes_z) - 1,     \
	  local_mem_size, shared_mem_size, num_barriers }

struct compute_test_descriptor
{
	char const* name;
	uint32_t expected_value;

	uint32_t const* code_size;
	uint8_t const* code;
	uint8_t num_gprs;

	void (*execute)(DkDevice, DkQueue, DkCmdBuf, uint32_t* results);
	bool (*check_results)(char const*, uint32_t*, FILE*);
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

DECLARE_ETEST(sust_p_rgba)
DECLARE_ETEST(suld_p_rgba)

DECLARE_MTEST(shfl_idx)
DECLARE_MTEST(shfl_up)
DECLARE_MTEST(shfl_down)
DECLARE_MTEST(shfl_bfly)

static struct compute_test_descriptor const test_descriptors[] =
{
	TEST("Constant",                    0xdeadbeef, constant,                  8),
	TEST("SHR_R.S32",                   0xff000000, shr_r_s32,                 8),
	TEST("SHR_R.U32",                   0x0f000000, shr_r_u32,                 8),
	TEST("SHR_R.U32.W",                 0x0000ff00, shr_r_u32_w,               8),
	TEST("SHR_R.U32 Clamped",           0x00000001, shr_r_u32_clamped,         8),
	TEST("SHR_IMM.S32",                 0xff000000, shr_imm_s32,               8),
	TEST("SHR_IMM.U32",                 0x0f000000, shr_imm_u32,               8),
	TEST("XMAD_RR.MRG UU00 CBCC",       0xfe2060f0, xmad_rr_mrg_uu00_cbcc,     8),
	TEST("XMAD_RR.MRG UU00 CHI",        0xc34291d8, xmad_rr_mrg_uu00_chi,      8),
	TEST("XMAD_RR.MRG UU00 CLO",        0xb474384b, xmad_rr_mrg_uu00_clo,      8),
	TEST("XMAD_RR.MRG UU00 C",          0x2d418a91, xmad_rr_mrg_uu00_c,        8),
	TEST("XMAD_RR.MRG UU00 CSFU",       0xbe0ac267, xmad_rr_mrg_uu00_csfu,     8),
	TEST("XMAD_RR.PSL.MRG UU00 CBCC",   0xfe208530, xmad_rr_psl_mrg_uu00_cbcc, 8),
	TEST("XMAD_RR.PSL.MRG UU00 CHI",    0xc342cefa, xmad_rr_psl_mrg_uu00_chi,  8),
	TEST("XMAD_RR.PSL.MRG UU00 CLO",    0xb4749a3b, xmad_rr_psl_mrg_uu00_clo,  8),
	TEST("XMAD_RR.PSL UU00 C",          0x2d41820a, xmad_rr_psl_mrg_uu00_c,    8),
	TEST("XMAD_RR.PSL.MRG UU00 CSFU",   0xbe0aa033, xmad_rr_psl_mrg_uu00_csfu, 8),
	TEST("XMAD_RR.PSL.MRG UU01 CBCC",   0xfe208530, xmad_rr_psl_mrg_uu01_cbcc, 8),
	TEST("XMAD_RR.PSL.MRG UU01 CHI",    0xc342cefa, xmad_rr_psl_mrg_uu01_chi,  8),
	TEST("XMAD_RR.PSL.MRG UU01 CLO",    0xb4749a3b, xmad_rr_psl_mrg_uu01_clo,  8),
	TEST("XMAD_RR.PSL.MRG UU01 C",      0x2d41820a, xmad_rr_psl_mrg_uu01_c,    8),
	TEST("XMAD_RR.PSL.MRG UU01 CSFU",   0xbe0aa033, xmad_rr_psl_mrg_uu01_csfu, 8),
	TEST("XMAD_RR.PSL.MRG UU11 CBCC",   0xfe208530, xmad_rr_psl_mrg_uu11_cbcc, 8),
	TEST("XMAD_RR.PSL.MRG UU11 CHI",    0xc342cefa, xmad_rr_psl_mrg_uu11_chi,  8),
	TEST("XMAD_RR.PSL.MRG UU11 CLO",    0xb4749a3b, xmad_rr_psl_mrg_uu11_clo,  8),
	TEST("XMAD_RR.PSL.MRG UU11 C",      0x2d41820a, xmad_rr_psl_mrg_uu11_c,    8),
	TEST("XMAD_RR.PSL.MRG UU11 CSFU",   0xbe0aa033, xmad_rr_psl_mrg_uu11_csfu, 8),
	TEST("XMAD_RR.PSL UU00 CBCC",       0x7d0b8530, xmad_rr_psl_uu00_cbcc,     8),
	TEST("XMAD_RR.PSL UU00 CHI",        0xc2decefa, xmad_rr_psl_uu00_chi,      8),
	TEST("XMAD_RR.PSL UU00 CLO",        0x9e109a3b, xmad_rr_psl_uu00_clo,      8),
	TEST("XMAD_RR.PSL UU00 C",          0xf92b820a, xmad_rr_psl_uu00_c,        8),
	TEST("XMAD_RR.PSL UU00 CSFU",       0xe0c7a033, xmad_rr_psl_uu00_csfu,     8),
	TEST("XMAD_RR UU00 CBCC",           0x3ae760f0, xmad_rr_uu00_cbcc,         8),
	TEST("XMAD_RR UU00 CHI",            0x688a91d8, xmad_rr_uu00_chi,          8),
	TEST("XMAD_RR UU00 CLO",            0x726d384b, xmad_rr_uu00_clo,          8),
	TEST("XMAD_RR UU00 C",              0xf58d8a91, xmad_rr_uu00_c,            8),
	TEST("XMAD_RR UU00 CSFU",           0x5bd1c267, xmad_rr_uu00_csfu,         8),
	TEST("F2F_R.F32.F32",               0x40e00000, f2f_r_f32_f32,             8),
	TEST("F2F_R.F32.F32 |Ra|",          0x40e00000, f2f_r_f32_f32_abs,         8),
	TEST("F2F_R.F32.F32 -Ra",           0xc0e00000, f2f_r_f32_f32_neg,         8),
	TEST("F2F_R.F32.F32.SAT",           0x3f800000, f2f_r_f32_f32_sat,         8),
	TEST("F2F_R.F32.F32.SAT -Ra",       0x00000000, f2f_r_f32_f32_sat_neg,     8),
	TEST("F2F_R.F32.F32.ROUND 1",       0x40800000, f2f_r_f32_f32_round_1,     8),
	TEST("F2F_R.F32.F32.ROUND 2",       0x40800000, f2f_r_f32_f32_round_2,     8),
	TEST("F2F_R.F32.F32.FLOOR",         0xc2300000, f2f_r_f32_f32_floor,       8),
	TEST("F2F_R.F32.F32.CEIL",          0xc22c0000, f2f_r_f32_f32_ceil,        8),
	TEST("F2F_R.F32.F32.TRUNC",         0xc22c0000, f2f_r_f32_f32_trunc,       8),
	TEST("F2F_R.F32.F16",               0x00003c00, f2f_r_f32_f16,             8),
	TEST("F2F_R.F32.F16 |Ra|",          0x00004800, f2f_r_f32_f16_abs,         8),
	TEST("F2F_R.F32.F16 -Ra",           0x0000bc00, f2f_r_f32_f16_neg,         8),
	TEST("F2F_R.F16.F16",               0x00004a33, f2f_r_f16_f16,             8),
	TEST("F2F_R.F16.F16 Ra.H1",         0x000050c0, f2f_r_f16_f16_h1,          8),
	TEST("F2F_R.F16.F16 |Ra|",          0x00005595, f2f_r_f16_f16_abs,         8),
	TEST("F2F_R.F16.F16 -Ra",           0x000055c6, f2f_r_f16_f16_neg,         8),
	TEST("F2F_R.F16.F16.SAT",           0x00000000, f2f_r_f16_f16_sat,         8),
	TEST("F2F_R.F16.F16.SAT -Ra",       0x00003c00, f2f_r_f16_f16_sat_neg,     8),
	TEST("F2F_R.F16.F16.ROUND",         0x0000c900, f2f_r_f16_f16_round,       8),
	TEST("F2F_R.F16.F16.FLOOR",         0x0000c900, f2f_r_f16_f16_floor,       8),
	TEST("F2F_R.F16.F16.CEIL",          0x0000c880, f2f_r_f16_f16_ceil,        8),
	TEST("F2F_R.F16.F16.TRUNC",         0x0000cb00, f2f_r_f16_f16_trunc,       8),
	TEST("F2I_R.S32.F32",               0x7fffffff, f2i_r_s32_f32,             8),
	TEST("F2I_R.U32.F16 Ra.H0",         0x00000017, f2i_r_u32_f16_h0,          8),
	TEST("F2I_R.U32.F16 Ra.H1",         0x00000035, f2i_r_u32_f16_h1,          8),
	TEST("F2I_R.U32.F16 |Ra.H1|",       0x0000003f, f2i_r_u32_f16_ah1,         8),
	TEST("F2I_R.U32.F16 -Ra.H1",        0x0000003f, f2i_r_u32_f16_nh1,         8),
	TEST("F2I_R.U32.F16 Rounding",      0x00000005, f2i_r_u32_f16_rounding,    8),
	TEST("F2I_R.U32.F16 Clamped",       0x00000001, f2i_r_u32_f16_clamped,     8),
	TEST("F2I_R.U32.F16.FLOOR",         0x00000004, f2i_r_u32_f16_floor,       8),
	TEST("F2I_R.U32.F16.CEIL",          0x00000008, f2i_r_u32_f16_ceil,        8),
	TEST("F2I_R.U32.F16.TRUNC",         0x00000007, f2i_r_u32_f16_trunc,       8),
	TEST("F2I_R.S32.F16 Ra.H0",         0x00000008, f2i_r_s32_f16_h0,          8),
	TEST("F2I_R.S32.F16 Ra.H1",         0xfffffff8, f2i_r_s32_f16_h1,          8),
	TEST("F2I_R.S32.F16 |Ra.H1|",       0x00000020, f2i_r_s32_f16_ah1,         8),
	TEST("F2I_R.S32.F16 -Ra.H1",        0xffffffb4, f2i_r_s32_f16_nh1,         8),
	TEST("F2I_R.S32.F16.FLOOR",         0xfffffffd, f2i_r_s32_f16_floor,       8),
	TEST("F2I_R.S32.F16.CEIL",          0xfffffffe, f2i_r_s32_f16_ceil,        8),
	TEST("F2I_R.S32.F16.TRUNC",         0xfffffff7, f2i_r_s32_f16_trunc,       8),
	TEST("HADD2_R",                     0x40004400, hadd2_r,                   8),
	TEST("HADD2_R H1_H1 H1_H0",         0x40000000, hadd2_r_h1h1_h1h0,         8),
	TEST("HADD2_R H0_H0 H1_H0",         0x46004400, hadd2_r_h0h0_h1h0,         8),
	TEST("HADD2_R H1_H0 H1_H1",         0x40004600, hadd2_r_h1h0_h1h1,         8),
	TEST("HADD2_R H1_H0 H0_H0",         0x00004400, hadd2_r_h1h0_h0h0,         8),
	TEST("HADD2_R -H1_H0 -H1_H0",       0xc000c400, hadd2_r_nh1h0_nh1h0,       8),
	TEST("HADD2_R |H1_H0| |H1_H0|",     0x47004000, hadd2_r_ah1h0_ah1h0,       8),
	TEST("HADD2_R |H1_H0| -H1_H0",      0xc5004000, hadd2_r_ah1h0_nh1h0,       8),
	TEST("HADD2_R -|H1_H0| -|H1_H0|",   0xc700c000, hadd2_r_nah1h0_nah1h0,     8),
	TEST("HADD2_R F32 F32",             0x42004200, hadd2_r_f32_f32,           8),
	TEST("HADD2_R -F32 F32",            0xbc00bc00, hadd2_r_nf32_f32,          8),
	TEST("HADD2_R F32 -F32",            0xbc00bc00, hadd2_r_f32_nf32,          8),
	TEST("HADD2_R.SAT F32 F32",         0x3c003c00, hadd2_r_sat_f32_f32,       8),
	TEST("HADD2_R.MRG_H0 F32 F32",      0xaaaa4200, hadd2_r_mrg_h0_f32_f32,    8),
	TEST("HADD2_R.MRG_H1 F32 F32",      0x4200aaaa, hadd2_r_mrg_h1_f32_f32,    8),
	TEST("HADD2_IMM",                   0x4b804400, hadd2_imm,                 8),
	TEST("HMUL2_R",                     0xc2004200, hmul2_r,                   8),
	TEST("HMUL2_R H1_H1 H1_H0",         0xc200bc00, hmul2_r_h1h1_h1h0,         8),
	TEST("HMUL2_R H0_H0 H1_H0",         0x48804200, hmul2_r_h0h0_h1h0,         8),
	TEST("HMUL2_R H1_H0 H1_H1",         0xc2004880, hmul2_r_h1h0_h1h1,         8),
	TEST("HMUL2_R H1_H0 H0_H0",         0xbc004200, hmul2_r_h1h0_h0h0,         8),
	TEST("HMUL2_R |H1_H0| |H1_H0|",     0x46004400, hmul2_r_ah1h0_ah1h0,       8),
	TEST("HMUL2_R |H1_H0| -H1_H0",      0xc6004410, hmul2_r_ah1h0_nh1h0,       8),
	TEST("HMUL2_R F32 F32",             0x40004000, hmul2_r_f32_f32,           8),
	TEST("HMUL2_R F32 -F32",            0xc000c000, hmul2_r_f32_nf32,          8),
	TEST("HMUL2_R.SAT F32 F32",         0x3c003c00, hmul2_r_sat_f32_f32,       8),
	TEST("HMUL2_R.MRG_H0 F32 F32",      0xaaaa4000, hmul2_r_mrg_h0_f32_f32,    8),
	TEST("HMUL2_R.MRG_H1 F32 F32",      0x4000aaaa, hmul2_r_mrg_h1_f32_f32,    8),
	TEST("HFMA2_RR",                    0x47004000, hfma2_rr,                  8),
	TEST("HFMA2_RR.F32",                0x40000000, hfma2_rr_f32,              8),
	TEST("HFMA2_RR.MRG_H0",             0xcccc4000, hfma2_rr_mrg_h0,           8),
	TEST("HFMA2_RR.MRG_H1",             0x4700cccc, hfma2_rr_mrg_h1,           8),
	TEST("HFMA2_RR H1_H0 -H1_H0 H1_H0", 0x48804400, hfma2_rr_h1h0_nh1h0_h1h0,  8),
	TEST("HFMA2_RR H1_H0 H1_H0 -H1_H0", 0xc880c400, hfma2_rr_h1h0_h1h0_nh1h0,  8),
	TEST("HSET2_R F32 F32",             0x3c003c00, hset2_r_f32_f32,           8),
	TEST("HSET2_R H1_H0 F32",           0x00003c00, hset2_r_h1h0_f32,          8),
	TEST("HSET2_R H0_H0 F32",           0x3c003c00, hset2_r_h0h0_f32,          8),
	TEST("HSET2_R H0_H0 H1_H1",         0xffffffff, hset2_r_h0h0_h1h1,         8),
	TEST("HSET2_R H1_H0 H1_H0",         0xffff0000, hset2_r_h1h0_h1h0,         8),
	TEST("HSETP2_R P39",                0xccccccc2, hsetp2_r_p39,              8),
	TEST("HSETP2_R F32 F32",            0x00010008, hsetp2_r_f32_f32,          8),
	TEST("HSETP2_R H1_H0 F32",          0x00000008, hsetp2_r_h1h0_f32,         8),
	TEST("HSETP2_R F32 |H1_H0|",        0x00010008, hsetp2_r_f32_ah1h0,        8),
	TEST("HSETP2_R F32 -H1_H0",         0x00010008, hsetp2_r_f32_nh1h0,        8),
	TEST("HSETP2_R.H_AND H1_H0 F32",    0x0000a008, hsetp2_r_hand_h1h0_f32,    8),
	TEST("R2P_IMM.B0 PR",               0x0000aaaa, r2p_imm_b0_pr,             8),
	TEST("R2P_IMM.B1 PR",               0x0000bbbb, r2p_imm_b1_pr,             8),
	TEST("R2P_IMM.B2 PR",               0x0000cccc, r2p_imm_b2_pr,             8),
	TEST("R2P_IMM.B3 PR",               0x0000dddd, r2p_imm_b3_pr,             8),
	TEST("P2R_IMM PR",                  0x0000007f, p2r_imm,                   8),
	TEST("LDS+STS",                     0xa0a0a0a0, shared_memory,             8),
	TEST("STS Indirect",                0xdeadcafe, sts_indirect,              8),
	TEST("STS.B64",                     0xddddbbbb, sts_b64,                   8),
	TEST("STS.B128",                    0xddccbbaa, sts_b128,                  8),
	TEST("LDS Indirect",                0xcafedead, lds_indirect,              8),
	TEST("ATOMS.ADD.U32",               0x00000060, atoms_u32_add,             8),
	TEST("ATOMS.MIN.U32",               0x00000040, atoms_u32_min,             8),
	TEST("ATOMS.MAX.U32",               0x90000000, atoms_u32_max,             8),
	TEST("ATOMS.MIN.S32",               0x90000000, atoms_s32_min,             8),
	TEST("ATOMS.MAX.S32",               0x00000040, atoms_s32_max,             8),

	ETEST("SUST.P.RGBA", 0x40f00000, sust_p_rgba, 8),
	ETEST("SULD.P.RGBA", 0x42140000, suld_p_rgba, 8),

	MTEST("SHFL.IDX",  shfl_idx,  8, 8, 1, 1, 1, 1, 1, 0, 0, 0),
	MTEST("SHFL.UP",   shfl_up,   8, 8, 1, 1, 1, 1, 1, 0, 0, 0),
	MTEST("SHFL.DOWN", shfl_down, 8, 8, 1, 1, 1, 1, 1, 0, 0, 0),
	MTEST("SHFL.BFLY", shfl_bfly, 8, 8, 1, 1, 1, 1, 1, 0, 0, 0),
};

#define NUM_TESTS (sizeof(test_descriptors) / sizeof(test_descriptors[0]))

static float to_seconds(u64 time)
{
    return (time * 625 / 12) / 1000000000.0f;
}

static bool execute_test(
	struct compute_test_descriptor const* test, DkDevice device,
	DkQueue queue, DkMemBlock blk_code, uint8_t* code, DkCmdBuf cmdbuf,
	uint32_t* results, FILE* report_file)
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

	if (test->execute)
	{
		test->execute(device, queue, cmdbuf, results);
	}
	else
	{
		dkCmdBufDispatchCompute(cmdbuf, test->num_invokes_x_minus_1 + 1,
			test->num_invokes_y_minus_1 + 1, test->num_invokes_z_minus_1 + 1);

		dkQueueSubmitCommands(queue, dkCmdBufFinishList(cmdbuf));
		dkQueueWaitIdle(queue);
	}

	if (test->check_results)
		return test->check_results(test->name, results, report_file);

	bool pass = results[0] == test->expected_value;
	if (!pass)
		printf("exp %08x got %08x ", test->expected_value, *(uint32_t*)results);

	unit_test_report(report_file, test->name, pass, 1, &test->expected_value,
		results);
	return pass;
}

static void wait_for_input()
{
	printf("Press A to continue...");

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

void run_compute_tests(
	DkDevice device, DkQueue queue, FILE* report_file, bool automatic_mode)
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
	uint32_t* ssbo_data = dkMemBlockGetCpuAddr(blk_ssbo);

	DkCmdBufMaker cmd_mk;
	dkCmdBufMakerDefaults(&cmd_mk, device);
	DkCmdBuf cmdbuf = dkCmdBufCreate(&cmd_mk);
	dkCmdBufAddMemory(cmdbuf, blk_cmdbuf, 0, CMDMEM_SIZE);

	dkCmdBufBindStorageBuffer(cmdbuf, DkStage_Compute, 0, ssbo_gpu_addr, SSBO_SIZE);
	DkCmdList list = dkCmdBufFinishList(cmdbuf);

	dkQueueSubmitCommands(queue, list);
	dkQueueWaitIdle(queue);

	printf("Running compute tests...\n\n");

	u64 start_time = armGetSystemTick();
	u64 total_time = 0;

	size_t failures = 0;
	for (size_t i = 0; i < NUM_TESTS; ++i)
	{
		struct compute_test_descriptor const* test = &test_descriptors[i];

		int written_chars =
			printf("%3zd/%3zd Test: %s", i + 1, NUM_TESTS, test->name);
		for (int i = 0; i < 43 - written_chars; ++i)
			putc('.', stdout);
		putc(' ', stdout);

		consoleUpdate(NULL);

		bool pass = execute_test(test, device, queue, blk_code, code_data,
			cmdbuf, ssbo_data, report_file);
		if (!pass)
			++failures;
		puts(pass ? "Passed" : "Failed");

		consoleUpdate(NULL);

		if (!automatic_mode && i != 0 && i % 43 == 0)
		{
			total_time += armGetSystemTick() - start_time;

			wait_for_input();

			start_time = armGetSystemTick();
		}
	}

	total_time += armGetSystemTick() - start_time;

	printf("\n%3d%% tests passed, %zd tests failed out of %zd\n\n"
		"Total Test time (real) = %.2f sec\n",
		(int)((NUM_TESTS - failures) * 100 / (float)NUM_TESTS), failures,
		NUM_TESTS, to_seconds(total_time));

	consoleUpdate(NULL);

	dkMemBlockDestroy(blk_ssbo);
	dkMemBlockDestroy(blk_code);
	dkCmdBufDestroy(cmdbuf);
	dkMemBlockDestroy(blk_cmdbuf);
}

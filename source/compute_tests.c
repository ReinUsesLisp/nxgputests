#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <switch.h>
#include <deko3d.h>

#include "compute_tests.h"
#include "dksh_gen.h"
#include "helper.h"

#define CMDMEM_SIZE (3 * DK_MEMBLOCK_ALIGNMENT)
#define CODEMEM_SIZE (512 * 1024)
#define SSBO_SIZE (DK_MEMBLOCK_ALIGNMENT)

#define TEST(name, expected, id)                                               \
    { name, #id, expected, .shared_mem_size = 512, .local_mem_size = 16 }

#define ETEST(name, expected, id)                                              \
    { name, #id, expected, execute_test_##id }

#define MTEST(name, id, workgroup_x, workgroup_y, workgroup_z,                 \
    num_invokes_x, num_invokes_y, num_invokes_z, local_mem_size,               \
    shared_mem_size, num_barriers)                                             \
    { name, #id, 0, NULL, test_##id, (workgroup_x) - 1, (workgroup_y) - 1,     \
      (workgroup_z) - 1, (num_invokes_x) - 1, (num_invokes_y) - 1,             \
      (num_invokes_z) - 1, local_mem_size, shared_mem_size, num_barriers }

struct compute_test_descriptor
{
    char const* name;
    char const* sass_file;
    uint32_t expected_value;

    void (*execute)(DkDevice, DkQueue, DkCmdBuf, uint32_t* results);
    bool (*check_results)(uint32_t*);
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
DECLARE_ETEST(suld_d_32_r32f)
DECLARE_ETEST(suld_d_32_rgba8u)
DECLARE_ETEST(suld_d_32_bgra8u)
DECLARE_ETEST(suld_d_32_rgba8s)
DECLARE_ETEST(suld_d_32_rgba8ui)
DECLARE_ETEST(suld_d_32_rgba8i)
DECLARE_ETEST(suld_d_64_rg32f)
DECLARE_ETEST(suld_d_64_rgba16f)
DECLARE_ETEST(suld_d_64_rgba16s)
DECLARE_ETEST(suld_d_64_rgba16u)
DECLARE_ETEST(suld_d_64_rgba16i)
DECLARE_ETEST(suld_d_64_rgba16ui)

DECLARE_MTEST(shfl_idx)
DECLARE_MTEST(shfl_up)
DECLARE_MTEST(shfl_down)
DECLARE_MTEST(shfl_bfly)

static struct compute_test_descriptor const test_descriptors[] =
{
    TEST("Constant",                    0xdeadbeef, constant),
    TEST("FSETP.F",                     0x0000dead, fsetp_f),
    TEST("FSETP.LT 1",                  0x0000cafe, fsetp_lt_1),
    TEST("FSETP.LT 2",                  0x0000dead, fsetp_lt_2),
    TEST("FSETP.LT 3",                  0x0000dead, fsetp_lt_3),
    TEST("FSETP.LT 4",                  0x0000dead, fsetp_lt_4),
    TEST("FSETP.LT 5",                  0x0000dead, fsetp_lt_5),
    TEST("FSETP.EQ 1",                  0x0000cafe, fsetp_eq_1),
    TEST("FSETP.EQ 2",                  0x0000dead, fsetp_eq_2),
    TEST("FSETP.EQ 3",                  0x0000dead, fsetp_eq_3),
    TEST("FSETP.EQ 4",                  0x0000dead, fsetp_eq_4),
    TEST("FSETP.EQ 5",                  0x0000dead, fsetp_eq_5),
    TEST("FSETP.LE 1",                  0x0000dead, fsetp_le_1),
    TEST("FSETP.LE 2",                  0x0000cafe, fsetp_le_2),
    TEST("FSETP.LE 3",                  0x0000cafe, fsetp_le_3),
    TEST("FSETP.LE 4",                  0x0000dead, fsetp_le_4),
    TEST("FSETP.LE 5",                  0x0000dead, fsetp_le_5),
    TEST("FSETP.GT 1",                  0x0000cafe, fsetp_gt_1),
    TEST("FSETP.GT 2",                  0x0000dead, fsetp_gt_2),
    TEST("FSETP.GT 3",                  0x0000dead, fsetp_gt_3),
    TEST("FSETP.GT 4",                  0x0000dead, fsetp_gt_4),
    TEST("FSETP.GT 5",                  0x0000dead, fsetp_gt_5),
    TEST("FSETP.NE 1",                  0x0000cafe, fsetp_ne_1),
    TEST("FSETP.NE 2",                  0x0000dead, fsetp_ne_2),
    TEST("FSETP.NE 3",                  0x0000dead, fsetp_ne_3),
    TEST("FSETP.NE 4",                  0x0000dead, fsetp_ne_4),
    TEST("FSETP.GE 1",                  0x0000cafe, fsetp_ge_1),
    TEST("FSETP.GE 2",                  0x0000cafe, fsetp_ge_2),
    TEST("FSETP.GE 3",                  0x0000dead, fsetp_ge_3),
    TEST("FSETP.GE 4",                  0x0000dead, fsetp_ge_4),
    TEST("FSETP.GE 5",                  0x0000dead, fsetp_ge_5),
    TEST("FSETP.NUM 1",                 0x0000cafe, fsetp_num_1),
    TEST("FSETP.NUM 2",                 0x0000dead, fsetp_num_2),
    TEST("FSETP.NUM 3",                 0x0000dead, fsetp_num_3),
    TEST("FSETP.NUM 4",                 0x0000dead, fsetp_num_4),
    TEST("FSETP.NAN 1",                 0x0000dead, fsetp_nan_1),
    TEST("FSETP.NAN 2",                 0x0000cafe, fsetp_nan_2),
    TEST("FSETP.NAN 3",                 0x0000cafe, fsetp_nan_3),
    TEST("FSETP.NAN 4",                 0x0000cafe, fsetp_nan_4),
    TEST("FSETP.LTU 1",                 0x0000cafe, fsetp_ltu_1),
    TEST("FSETP.LTU 2",                 0x0000dead, fsetp_ltu_2),
    TEST("FSETP.LTU 3",                 0x0000cafe, fsetp_ltu_3),
    TEST("FSETP.LTU 4",                 0x0000cafe, fsetp_ltu_4),
    TEST("FSETP.LTU 5",                 0x0000cafe, fsetp_ltu_5),
    TEST("FSETP.EQU 1",                 0x0000cafe, fsetp_equ_1),
    TEST("FSETP.EQU 2",                 0x0000cafe, fsetp_equ_2),
    TEST("FSETP.EQU 3",                 0x0000cafe, fsetp_equ_3),
    TEST("FSETP.EQU 4",                 0x0000cafe, fsetp_equ_4),
    TEST("FSETP.EQU 5",                 0x0000dead, fsetp_equ_5),
    TEST("FSETP.LEU 1",                 0x0000dead, fsetp_leu_1),
    TEST("FSETP.LEU 2",                 0x0000cafe, fsetp_leu_2),
    TEST("FSETP.LEU 3",                 0x0000cafe, fsetp_leu_3),
    TEST("FSETP.LEU 4",                 0x0000cafe, fsetp_leu_4),
    TEST("FSETP.LEU 5",                 0x0000cafe, fsetp_leu_5),
    TEST("FSETP.GTU 1",                 0x0000cafe, fsetp_gtu_1),
    TEST("FSETP.GTU 2",                 0x0000dead, fsetp_gtu_2),
    TEST("FSETP.GTU 3",                 0x0000dead, fsetp_gtu_3),
    TEST("FSETP.GTU 4",                 0x0000cafe, fsetp_gtu_4),
    TEST("FSETP.GTU 5",                 0x0000cafe, fsetp_gtu_5),
    TEST("FSETP.NEU 1",                 0x0000cafe, fsetp_neu_1),
    TEST("FSETP.NEU 2",                 0x0000dead, fsetp_neu_2),
    TEST("FSETP.NEU 3",                 0x0000cafe, fsetp_neu_3),
    TEST("FSETP.NEU 4",                 0x0000cafe, fsetp_neu_4),
    TEST("FSETP.GEU 1",                 0x0000cafe, fsetp_geu_1),
    TEST("FSETP.GEU 2",                 0x0000cafe, fsetp_geu_2),
    TEST("FSETP.GEU 3",                 0x0000dead, fsetp_geu_3),
    TEST("FSETP.GEU 4",                 0x0000cafe, fsetp_geu_4),
    TEST("FSETP.GEU 5",                 0x0000cafe, fsetp_geu_5),
    TEST("FSETP.T",                     0x0000dead, fsetp_t),
    TEST("SHR_R.S32",                   0xff000000, shr_r_s32),
    TEST("SHR_R.U32",                   0x0f000000, shr_r_u32),
    TEST("SHR_R.U32.W",                 0x0000ff00, shr_r_u32_w),
    TEST("SHR_R.U32 Clamped",           0x00000001, shr_r_u32_clamped),
    TEST("SHR_IMM.S32",                 0xff000000, shr_imm_s32),
    TEST("SHR_IMM.U32",                 0x0f000000, shr_imm_u32),
    TEST("SHF_R.L 1",                   0xccbbaa1a, shf_r_left_1),
    TEST("SHF_R.L 2",                   0x0d218ae1, shf_r_left_2),
    TEST("SHF_R.L 3",                   0x1a4315c8, shf_r_left_3),
    TEST("SHF_R.R 1",                   0xde000085, shf_r_right_1),
    TEST("SHF_R.R 2",                   0x11fde5a9, shf_r_right_2),
    TEST("SHF_R.R 3",                   0x23fbcb53, shf_r_right_3),
    TEST("SHF_R.R 4",                   0xcafe8888, shf_r_right_4),
    TEST("SHF_R.R.W",                   0x44800000, shf_r_right_w),
    TEST("SHF_IMM.L",                   0xccbbaa1a, shf_imm_left),
    TEST("SHF_IMM.R",                   0xaa1a4315, shf_imm_right),
    TEST("SHF_R.L.S64",                 0x00000000, shf_r_left_s64),
    TEST("SHF_R.L.W.S64",               0x1020304c, shf_r_left_w_s64),
    TEST("SHF_IMM.L.S64 1",             0xd0000000, shf_imm_left_s64_1),
    TEST("SHF_IMM.L.S64 2",             0x2345aabb, shf_imm_left_s64_2),
    TEST("SHF_IMM.L.S64 3",             0x1020304c, shf_imm_left_s64_3),
    TEST("SHF_R.R.U64",                 0xcafe5555, shf_r_right_u64),
    TEST("SHF_IMM.R.S64",               0xffffff80, shf_imm_right_s64),
    TEST("SHF_IMM.R.U64",               0x00000080, shf_imm_right_u64),
    TEST("XMAD_RR.MRG UU00 CBCC",       0xfe2060f0, xmad_rr_mrg_uu00_cbcc),
    TEST("XMAD_RR.MRG UU00 CHI",        0xc34291d8, xmad_rr_mrg_uu00_chi),
    TEST("XMAD_RR.MRG UU00 CLO",        0xb474384b, xmad_rr_mrg_uu00_clo),
    TEST("XMAD_RR.MRG UU00 C",          0x2d418a91, xmad_rr_mrg_uu00_c),
    TEST("XMAD_RR.MRG UU00 CSFU",       0xbe0ac267, xmad_rr_mrg_uu00_csfu),
    TEST("XMAD_RR.PSL.MRG UU00 CBCC",   0xfe208530, xmad_rr_psl_mrg_uu00_cbcc),
    TEST("XMAD_RR.PSL.MRG UU00 CHI",    0xc342cefa, xmad_rr_psl_mrg_uu00_chi),
    TEST("XMAD_RR.PSL.MRG UU00 CLO",    0xb4749a3b, xmad_rr_psl_mrg_uu00_clo),
    TEST("XMAD_RR.PSL.MRG UU00 C",      0x2d41820a, xmad_rr_psl_mrg_uu00_c),
    TEST("XMAD_RR.PSL.MRG UU00 CSFU",   0xbe0aa033, xmad_rr_psl_mrg_uu00_csfu),
    TEST("XMAD_RR.PSL.MRG UU01 CBCC",   0xfe208530, xmad_rr_psl_mrg_uu01_cbcc),
    TEST("XMAD_RR.PSL.MRG UU01 CHI",    0xc342cefa, xmad_rr_psl_mrg_uu01_chi),
    TEST("XMAD_RR.PSL.MRG UU01 CLO",    0xb4749a3b, xmad_rr_psl_mrg_uu01_clo),
    TEST("XMAD_RR.PSL.MRG UU01 C",      0x2d41820a, xmad_rr_psl_mrg_uu01_c),
    TEST("XMAD_RR.PSL.MRG UU01 CSFU",   0xbe0aa033, xmad_rr_psl_mrg_uu01_csfu),
    TEST("XMAD_RR.PSL.MRG UU11 CBCC",   0xfe208530, xmad_rr_psl_mrg_uu11_cbcc),
    TEST("XMAD_RR.PSL.MRG UU11 CHI",    0xc342cefa, xmad_rr_psl_mrg_uu11_chi),
    TEST("XMAD_RR.PSL.MRG UU11 CLO",    0xb4749a3b, xmad_rr_psl_mrg_uu11_clo),
    TEST("XMAD_RR.PSL.MRG UU11 C",      0x2d41820a, xmad_rr_psl_mrg_uu11_c),
    TEST("XMAD_RR.PSL.MRG UU11 CSFU",   0xbe0aa033, xmad_rr_psl_mrg_uu11_csfu),
    TEST("XMAD_RR.PSL UU00 CBCC",       0x7d0b8530, xmad_rr_psl_uu00_cbcc),
    TEST("XMAD_RR.PSL UU00 CHI",        0xc2decefa, xmad_rr_psl_uu00_chi),
    TEST("XMAD_RR.PSL UU00 CLO",        0x9e109a3b, xmad_rr_psl_uu00_clo),
    TEST("XMAD_RR.PSL UU00 C",          0xf92b820a, xmad_rr_psl_uu00_c),
    TEST("XMAD_RR.PSL UU00 CSFU",       0xe0c7a033, xmad_rr_psl_uu00_csfu),
    TEST("XMAD_RR UU00 CBCC",           0x3ae760f0, xmad_rr_uu00_cbcc),
    TEST("XMAD_RR UU00 CHI",            0x688a91d8, xmad_rr_uu00_chi),
    TEST("XMAD_RR UU00 CLO",            0x726d384b, xmad_rr_uu00_clo),
    TEST("XMAD_RR UU00 C",              0xf58d8a91, xmad_rr_uu00_c),
    TEST("XMAD_RR UU00 CSFU",           0x5bd1c267, xmad_rr_uu00_csfu),
    TEST("FCMP_R 1",                    0x00000011, fcmp_1),
    TEST("FCMP_R 2",                    0x00000088, fcmp_2),
    TEST("FCMP_R 3",                    0x00000011, fcmp_3),
    TEST("FCMP_R 4",                    0x00000088, fcmp_4),
    TEST("F2F_R.F32.F32",               0x40e00000, f2f_r_f32_f32),
    TEST("F2F_R.F32.F32 |Ra|",          0x40e00000, f2f_r_f32_f32_abs),
    TEST("F2F_R.F32.F32 -Ra",           0xc0e00000, f2f_r_f32_f32_neg),
    TEST("F2F_R.F32.F32.SAT",           0x3f800000, f2f_r_f32_f32_sat),
    TEST("F2F_R.F32.F32.SAT -Ra",       0x00000000, f2f_r_f32_f32_sat_neg),
    TEST("F2F_R.F32.F32.ROUND 1",       0x40800000, f2f_r_f32_f32_round_1),
    TEST("F2F_R.F32.F32.ROUND 2",       0x40800000, f2f_r_f32_f32_round_2),
    TEST("F2F_R.F32.F32.FLOOR",         0xc2300000, f2f_r_f32_f32_floor),
    TEST("F2F_R.F32.F32.CEIL",          0xc22c0000, f2f_r_f32_f32_ceil),
    TEST("F2F_R.F32.F32.TRUNC",         0xc22c0000, f2f_r_f32_f32_trunc),
    TEST("F2F_R.F32.F16",               0x00003c00, f2f_r_f32_f16),
    TEST("F2F_R.F32.F16 |Ra|",          0x00004800, f2f_r_f32_f16_abs),
    TEST("F2F_R.F32.F16 -Ra",           0x0000bc00, f2f_r_f32_f16_neg),
    TEST("F2F_R.F16.F16",               0x00004a33, f2f_r_f16_f16),
    TEST("F2F_R.F16.F16 Ra.H1",         0x000050c0, f2f_r_f16_f16_h1),
    TEST("F2F_R.F16.F16 |Ra|",          0x00005595, f2f_r_f16_f16_abs),
    TEST("F2F_R.F16.F16 -Ra",           0x000055c6, f2f_r_f16_f16_neg),
    TEST("F2F_R.F16.F16.SAT",           0x00000000, f2f_r_f16_f16_sat),
    TEST("F2F_R.F16.F16.SAT -Ra",       0x00003c00, f2f_r_f16_f16_sat_neg),
    TEST("F2F_R.F16.F16.ROUND",         0x0000c900, f2f_r_f16_f16_round),
    TEST("F2F_R.F16.F16.FLOOR",         0x0000c900, f2f_r_f16_f16_floor),
    TEST("F2F_R.F16.F16.CEIL",          0x0000c880, f2f_r_f16_f16_ceil),
    TEST("F2F_R.F16.F16.TRUNC",         0x0000cb00, f2f_r_f16_f16_trunc),
    TEST("F2I_R.S32.F32",               0x7fffffff, f2i_r_s32_f32),
    TEST("F2I_R.U32.F16 Ra.H0",         0x00000017, f2i_r_u32_f16_h0),
    TEST("F2I_R.U32.F16 Ra.H1",         0x00000035, f2i_r_u32_f16_h1),
    TEST("F2I_R.U32.F16 |Ra.H1|",       0x0000003f, f2i_r_u32_f16_ah1),
    TEST("F2I_R.U32.F16 -Ra.H1",        0x0000003f, f2i_r_u32_f16_nh1),
    TEST("F2I_R.U32.F16 Rounding",      0x00000005, f2i_r_u32_f16_rounding),
    TEST("F2I_R.U32.F16 Clamped",       0x00000001, f2i_r_u32_f16_clamped),
    TEST("F2I_R.U32.F16.FLOOR",         0x00000004, f2i_r_u32_f16_floor),
    TEST("F2I_R.U32.F16.CEIL",          0x00000008, f2i_r_u32_f16_ceil),
    TEST("F2I_R.U32.F16.TRUNC",         0x00000007, f2i_r_u32_f16_trunc),
    TEST("F2I_R.S32.F16 Ra.H0",         0x00000008, f2i_r_s32_f16_h0),
    TEST("F2I_R.S32.F16 Ra.H1",         0xfffffff8, f2i_r_s32_f16_h1),
    TEST("F2I_R.S32.F16 |Ra.H1|",       0x00000020, f2i_r_s32_f16_ah1),
    TEST("F2I_R.S32.F16 -Ra.H1",        0xffffffb4, f2i_r_s32_f16_nh1),
    TEST("F2I_R.S32.F16.FLOOR",         0xfffffffd, f2i_r_s32_f16_floor),
    TEST("F2I_R.S32.F16.CEIL",          0xfffffffe, f2i_r_s32_f16_ceil),
    TEST("F2I_R.S32.F16.TRUNC",         0xfffffff7, f2i_r_s32_f16_trunc),
    TEST("I2F_R.U8.F32 Ra.B0",          0x43720000, i2f_u8_f32_b0),
    TEST("I2F_R.U8.F32 Ra.B1",          0x43700000, i2f_u8_f32_b1),
    TEST("I2F_R.U8.F32 Ra.B2",          0x437e0000, i2f_u8_f32_b2),
    TEST("I2F_R.U8.F32 Ra.B3",          0x437a0000, i2f_u8_f32_b3),
    TEST("I2I.S32.S32",                 0xace007de, i2i_s32_s32),
    TEST("I2I.U32.U32",                 0xbebeadad, i2i_u32_u32),
    TEST("I2I.U32.S16 Ra.H0",           0x00002f82, i2i_u32_s16_h0),
    TEST("I2I.U32.S16 Ra.H1",           0xffff93a0, i2i_u32_s16_h1),
    TEST("I2I.U32.U16 Ra.H0",           0x0000adad, i2i_u32_u16),
    TEST("I2I.U32.U16 Ra.H1",           0x0000bebe, i2i_u32_u16_h1),
    TEST("I2I.U32.S8 Ra.B0",            0xffffff82, i2i_u32_s8_b0),
    TEST("I2I.U32.S8 Ra.B1",            0x0000002f, i2i_u32_s8_b1),
    TEST("I2I.U32.S8 Ra.B2",            0xffffffa0, i2i_u32_s8_b2),
    TEST("I2I.U32.S8 Ra.B3",            0xffffff93, i2i_u32_s8_b3),
    TEST("I2I.U32.U8 Ra.B0",            0x00000060, i2i_u32_u8_b0),
    TEST("I2I.U32.U8 Ra.B1",            0x00000014, i2i_u32_u8_b1),
    TEST("I2I.U32.U8 Ra.B2",            0x00000095, i2i_u32_u8_b2),
    TEST("I2I.U32.U8 Ra.B3",            0x000000ea, i2i_u32_u8_b3),
    TEST("I2I.S16.S32",                 0x000007de, i2i_s16_s32),
    TEST("I2I.U16.S32",                 0x000007da, i2i_u16_s32),
    TEST("I2I.U16.U32",                 0x0000ddee, i2i_u16_u32),
    TEST("I2I.S8.S32",                  0x000000da, i2i_s8_s32),
    TEST("I2I.U8.U32",                  0x000000ee, i2i_u8_u32),
    TEST("I2I.U16.U32.SAT",             0x0000ffff, i2i_u16_u32_sat),
    TEST("I2I.U16.S32.SAT 1",           0x00000000, i2i_u16_s32_sat_1),
    TEST("I2I.U16.S32.SAT 2",           0x0000ffff, i2i_u16_s32_sat_2),
    TEST("I2I.S16.S32.SAT 1",           0xffff8000, i2i_s16_s32_sat_1),
    TEST("I2I.S16.S32.SAT 2",           0x00007fff, i2i_s16_s32_sat_2),
    TEST("I2I.S8.S32.SAT",              0xffffff80, i2i_s8_s32_sat),
    TEST("I2I.U32.S32",                 0xbebeadad, i2i_u32_s32),
    TEST("I2I.U32.S32.SAT",             0x000000aa, i2i_u32_s32_sat),
    TEST("I2I.S32.U32.SAT",             0x7fffffff, i2i_s32_u32_sat),
    TEST("I2I.U32.S16.SAT",             0x00000000, i2i_u32_s16_sat),
    TEST("I2I.U32.S8.SAT 1",            0x00000002, i2i_u32_s8_sat_1),
    TEST("I2I.U32.S8.SAT 2",            0x00000009, i2i_u32_s8_sat_2),
    TEST("I2I.S8.U32.SAT",              0x0000007f, i2i_s8_u32_sat),
    TEST("I2I.S32.S32 -Ra",             0xffffffd8, i2i_s32_s32_neg),
    TEST("I2I.S32.S32 |Ra|",            0x00000032, i2i_s32_s32_abs),
    TEST("I2I.S32.S32 -|Ra|",           0xffffffce, i2i_s32_s32_neg_abs),
    TEST("I2I.U32.U32 -Ra",             0xfffffffc, i2i_u32_u32_neg),
    TEST("I2I.U32.U32 |Ra|",            0x7ffffffc, i2i_u32_u32_abs),
    TEST("HADD2_R",                     0x40004400, hadd2_r),
    TEST("HADD2_R H1_H1 H1_H0",         0x40000000, hadd2_r_h1h1_h1h0),
    TEST("HADD2_R H0_H0 H1_H0",         0x46004400, hadd2_r_h0h0_h1h0),
    TEST("HADD2_R H1_H0 H1_H1",         0x40004600, hadd2_r_h1h0_h1h1),
    TEST("HADD2_R H1_H0 H0_H0",         0x00004400, hadd2_r_h1h0_h0h0),
    TEST("HADD2_R -H1_H0 -H1_H0",       0xc000c400, hadd2_r_nh1h0_nh1h0),
    TEST("HADD2_R |H1_H0| |H1_H0|",     0x47004000, hadd2_r_ah1h0_ah1h0),
    TEST("HADD2_R |H1_H0| -H1_H0",      0xc5004000, hadd2_r_ah1h0_nh1h0),
    TEST("HADD2_R -|H1_H0| -|H1_H0|",   0xc700c000, hadd2_r_nah1h0_nah1h0),
    TEST("HADD2_R F32 F32",             0x42004200, hadd2_r_f32_f32),
    TEST("HADD2_R -F32 F32",            0xbc00bc00, hadd2_r_nf32_f32),
    TEST("HADD2_R F32 -F32",            0xbc00bc00, hadd2_r_f32_nf32),
    TEST("HADD2_R.SAT F32 F32",         0x3c003c00, hadd2_r_sat_f32_f32),
    TEST("HADD2_R.MRG_H0 F32 F32",      0xaaaa4200, hadd2_r_mrg_h0_f32_f32),
    TEST("HADD2_R.MRG_H1 F32 F32",      0x4200aaaa, hadd2_r_mrg_h1_f32_f32),
    TEST("HMUL2_R",                     0xc2004200, hmul2_r),
    TEST("HMUL2_R H1_H1 H1_H0",         0xc200bc00, hmul2_r_h1h1_h1h0),
    TEST("HMUL2_R H0_H0 H1_H0",         0x48804200, hmul2_r_h0h0_h1h0),
    TEST("HMUL2_R H1_H0 H1_H1",         0xc2004880, hmul2_r_h1h0_h1h1),
    TEST("HMUL2_R H1_H0 H0_H0",         0xbc004200, hmul2_r_h1h0_h0h0),
    TEST("HMUL2_R |H1_H0| |H1_H0|",     0x46004400, hmul2_r_ah1h0_ah1h0),
    TEST("HMUL2_R |H1_H0| -H1_H0",      0xc6004410, hmul2_r_ah1h0_nh1h0),
    TEST("HMUL2_R F32 F32",             0x40004000, hmul2_r_f32_f32),
    TEST("HMUL2_R F32 -F32",            0xc000c000, hmul2_r_f32_nf32),
    TEST("HMUL2_R.SAT F32 F32",         0x3c003c00, hmul2_r_sat_f32_f32),
    TEST("HMUL2_R.MRG_H0 F32 F32",      0xaaaa4000, hmul2_r_mrg_h0_f32_f32),
    TEST("HMUL2_R.MRG_H1 F32 F32",      0x4000aaaa, hmul2_r_mrg_h1_f32_f32),
    TEST("HFMA2_RR",                    0x47004000, hfma2_rr),
    TEST("HFMA2_RR.F32",                0x40000000, hfma2_rr_f32),
    TEST("HFMA2_RR.MRG_H0",             0xcccc4000, hfma2_rr_mrg_h0),
    TEST("HFMA2_RR.MRG_H1",             0x4700cccc, hfma2_rr_mrg_h1),
    TEST("HFMA2_RR H1_H0 -H1_H0 H1_H0", 0x48804400, hfma2_rr_h1h0_nh1h0_h1h0),
    TEST("HFMA2_RR H1_H0 H1_H0 -H1_H0", 0xc880c400, hfma2_rr_h1h0_h1h0_nh1h0),
    TEST("HSET2_R F32 F32",             0x3c003c00, hset2_r_f32_f32),
    TEST("HSET2_R H1_H0 F32",           0x00003c00, hset2_r_h1h0_f32),
    TEST("HSET2_R H0_H0 F32",           0x3c003c00, hset2_r_h0h0_f32),
    TEST("HSET2_R H0_H0 H1_H1",         0xffffffff, hset2_r_h0h0_h1h1),
    TEST("HSET2_R H1_H0 H1_H0",         0xffff0000, hset2_r_h1h0_h1h0),
    TEST("HSETP2_R P39",                0xccccccc2, hsetp2_r_p39),
    TEST("HSETP2_R F32 F32",            0x00010008, hsetp2_r_f32_f32),
    TEST("HSETP2_R H1_H0 F32",          0x0000a008, hsetp2_r_h1h0_f32),
    TEST("HSETP2_R F32 |H1_H0|",        0x00010008, hsetp2_r_f32_ah1h0),
    TEST("HSETP2_R F32 -H1_H0",         0x00010000, hsetp2_r_f32_nh1h0),
    TEST("HSETP2_R.H_AND H1_H0 F32",    0x0000a008, hsetp2_r_hand_h1h0_f32),
    TEST("R2P_IMM.B0 PR",               0x0000aaaa, r2p_imm_b0_pr),
    TEST("R2P_IMM.B1 PR",               0x0000bbbb, r2p_imm_b1_pr),
    TEST("R2P_IMM.B2 PR",               0x0000cccc, r2p_imm_b2_pr),
    TEST("R2P_IMM.B3 PR",               0x0000dddd, r2p_imm_b3_pr),
    TEST("P2R_IMM PR",                  0x0000007f, p2r_imm),
    TEST("P2R_IMM PR Ra",               0xcccc7bcc, p2r_imm_ra),
    TEST("LDS+STS",                     0xa0a0a0a0, shared_memory),
    TEST("STS Indirect",                0xdeadcafe, sts_indirect),
    TEST("STS.B64",                     0xddddbbbb, sts_b64),
    TEST("STS.B128",                    0xddccbbaa, sts_b128),
    TEST("LDS Indirect",                0xcafedead, lds_indirect),
    TEST("STL.S16",                     0x0000cafe, stl_s16),
    TEST("STL.S16 Unaligned",           0xcafe0000, stl_s16_unaligned),
    TEST("LDL.S16",                     0xfffff000, ldl_s16),
    TEST("LDL.S16 Unaligned",           0xfffff005, ldl_s16_unaligned),
    TEST("ATOMS.ADD.U32",               0x00000060, atoms_u32_add),
    TEST("ATOMS.MIN.U32",               0x00000040, atoms_u32_min),
    TEST("ATOMS.MAX.U32",               0x90000000, atoms_u32_max),
    TEST("ATOMS.MIN.S32",               0x90000000, atoms_s32_min),
    TEST("ATOMS.MAX.S32",               0x00000040, atoms_s32_max),
    TEST("IADD_R",                      0xbbccddee, iadd_r),
    TEST("IADD_R -Ra",                  0x55443323, iadd_r_neg_a),
    TEST("IADD_R -Rb",                  0xaabbccdd, iadd_r_neg_b),
    TEST("IADD_R.SAT High",             0x7fffffff, iadd_r_high_sat),
    TEST("IADD_R.SAT Low",              0x80000000, iadd_r_low_sat),
    TEST("IADD_R.SAT Fake",             0xd0000000, iadd_r_fake_sat),
    TEST("IADD_R.CC Zero",              0x00000001, iadd_r_cc_zero),
    TEST("IADD_R.CC Sign",              0x9161ff02, iadd_r_cc_sign),
    TEST("IADD_R.CC Carry",             0x00000604, iadd_r_cc_carry),
    TEST("IADD_R.CC Overflow",          0xe000000a, iadd_r_cc_overflow),
    TEST("IADD_R.X Carry",              0xf0000601, iadd_r_x_carry),
    TEST("IADD_R.X Fake",               0x20000601, iadd_r_x_fake),
    TEST("IADD_R.X Flags",              0x0000000f, iadd_r_x_flags),
    TEST("IADD_R.X Rd.CC 1",            0x8000000a, iadd_r_x_cc_1),
    TEST("IADD_R.X Rd.CC 2",            0x00000005, iadd_r_x_cc_2),
    TEST("ISCADD_R",                    0x00050005, iscadd_r),
    TEST("ISCADD_R -Ra",                0x80050005, iscadd_r_na),
    TEST("ISCADD_R -Rb",                0x02d51aaf, iscadd_r_nb),
    TEST("ISCADD_IMM",                  0x02c3553b, iscadd_imm),
    TEST("ISCADD_IMM.CC v1",            0xaab15502, iscadd_imm_cc_v1),
    TEST("ISCADD_IMM.CC v2",            0x04000505, iscadd_imm_cc_v2),
    TEST("ISCADD_IMM.CC v3",            0x01000000, iscadd_imm_cc_v3),
    TEST("ISCADD_IMM.CC v4",            0x05000000, iscadd_imm_cc_v4),
    TEST("ISCADD_IMM.CC Fake",          0x5585ff00, iscadd_imm_cc_fake),
    TEST("FLO_R.S32",                   0x0000001e, flo_r_s32),
    TEST("FLO_R.S32 ~Ra",               0x0000001c, flo_r_s32_inv),
    TEST("FLO_R.U32",                   0x00000013, flo_r_u32),
    TEST("FLO_R.U32.SH",                0x00000013, flo_r_u32_sh),
    TEST("FLO_R Empty",                 0xffffffff, flo_r_empty),
    TEST("FLO_R.CC Fail",               0x02ffffff, flo_r_cc_fail),
    TEST("FLO_R.CC Zero",               0x02ffffff, flo_r_cc_zero),
    TEST("POPC_R",                      0x0000000f, popc_r),
    TEST("POPC_R ~Ra",                  0x00000011, popc_r_inv),
    TEST("BFE_R.S32",                   0xfffffff0, bfe_r_s32),
    TEST("BFE_R.S32.BREV",              0x0000007f, bfe_r_s32_brev),
    TEST("BFE_R.S32 Ra.CC",             0xfae83c02, bfe_r_s32_cc),
    TEST("BFE_R.U32",                   0x000001ae, bfe_r_u32),
    TEST("BFE_R.U32.BREV",              0x00000003, bfe_r_u32_brev),
    TEST("BFE_R.U32 Ra.CC",             0x7ae83c00, bfe_r_u32_cc),
    TEST("BFE_R Zero",                  0x00000000, bfe_r_zero),
    TEST("BFE_R Expand",                0xff800000, bfe_r_expand),
    TEST("LEA_R",                       0x0002015e, lea_r),
    TEST("LEA_R -Ra",                   0xfffdff5e, lea_r_neg),
    TEST("LEA_R.HI",                    0x6000775f, lea_r_hi),
    TEST("VMNMX.MX S32 S32",            0x00000008, vmnmx_mx_s32_s32),
    TEST("VMNMX.MX U32 U32",            0x90000000, vmnmx_mx_u32_u32),
    TEST("VMNMX.MX U32 S32",            0xc0000000, vmnmx_mx_u32_s32),
    TEST("VMNMX.MX S32 U32",            0x00000023, vmnmx_mx_s32_u32),
    TEST("VMNMX.MN S32 U32",            0xc5600000, vmnmx_mn_s32_u32),
    TEST("VMNMX.MX.MAX S32 S32",        0x00000008, vmnmx_mx_s32_s32_max),
    TEST("VMNMX.MX.MAX U32 U32",        0x00000010, vmnmx_mx_u32_u32_max),
    TEST("VMNMX.UD.MX.MAX U32 U32",     0xccccdede, vmnmx_ud_mx_u32_u32_max),
    TEST("VMNMX.MX.ACC U32 U32",        0x60005000, vmnmx_mx_u32_u32_acc),
    TEST("VMNMX.MX.SAT.ACC U32 U32",    0x80004fff, vmnmx_mx_u32_u32_sat_acc),
    TEST("VMNMX.UD.MX.SAT U32 U32",     0x80000000, vmnmx_ud_mx_u32_u32_sat),
    TEST("VMNMX.MX.MRG_16H U32 U32",    0x2000cccc, vmnmx_mx_u32_u32_mrg_16h),
    TEST("VMNMX.MX.MRG_16L U32 U32",    0xbbbb2000, vmnmx_mx_u32_u32_mrg_16l),
    TEST("VMNMX.MX.MRG_8B0 U32 U32",    0xaabbcc12, vmnmx_mx_u32_u32_mrg_8b0),
    TEST("VMNMX.MX.MRG_8B0 U32 U32",    0xaa12ccdd, vmnmx_mx_u32_u32_mrg_8b2),
    TEST("VMNMX.SAT 1",                 0x00000000, vmnmx_sat_1),
    TEST("VMNMX.SAT 2",                 0x00000000, vmnmx_sat_2),
    TEST("VMNMX.SAT 3",                 0x00000008, vmnmx_sat_3),
    TEST("VMNMX.SAT 4",                 0x7fffffff, vmnmx_sat_4),
    TEST("VMNMX.SAT 5",                 0x90000000, vmnmx_sat_5),
    TEST("VMNMX.SAT 6",                 0x7fffffff, vmnmx_sat_6),
    TEST("VMNMX.SAT 7",                 0x90000000, vmnmx_sat_7),
    TEST("BRA",                         0xcdcdacac, bra),
    TEST("SSY",                         0xa0f943de, ssy),
    TEST("BRK",                         0xbabadead, brk),
    TEST("SSY & BRK",                   0x0000dddd, ssy_brk),
    TEST("SSY & BRK 2",                 0x00000400, ssy_brk_2),
    TEST("CAL",                         0x00000008, cal),
    TEST("LDG.E.CI.U8",                 0x000000f2, ldg_e_ci_u8),
    TEST("LDG.E.CI.U8 Unaligned",       0x000000f0, ldg_e_ci_u8_unaligned),
    TEST("LDG.E.CI.U16",                0x0000f0f2, ldg_e_ci_u16),
    TEST("LDG.E.CI.U16 Unaligned",      0x0000fafe, ldg_e_ci_u16_unaligned),
    TEST("STG.E.U8",                    0xcccccccc, stg_e_u8),
    TEST("STG.E.U16",                   0xabcdabcd, stg_e_u16),
    TEST("ATOM.E.ADD.S32",              0x2468c2ef, atom_add_s32),
    TEST("RED.E.ADD",                   0xdadabab9, red_add),

    ETEST("SUST.P.RGBA",        0x40f00000, sust_p_rgba),
    ETEST("SULD.P.RGBA",        0x42140000, suld_p_rgba),
    ETEST("SULD.D.32 R32F",     0x42960000, suld_d_32_r32f),
    ETEST("SULD.D.32 RGBA8U",   0x20406080, suld_d_32_rgba8u),
    ETEST("SULD.D.32 BGRA8U",   0x21416181, suld_d_32_bgra8u),
    ETEST("SULD.D.32 RGBA8S",   0x65fe12ff, suld_d_32_rgba8s),
    ETEST("SULD.D.32 RGBA8UI",  0xdeadbeec, suld_d_32_rgba8ui),
    ETEST("SULD.D.32 RGBA8I",   0x11a220ff, suld_d_32_rgba8i),
    ETEST("SULD.D.64 RG32F",    0x377a5a7f, suld_d_64_rg32f),
    ETEST("SULD.D.64 RGBA16F",  0x44446666, suld_d_64_rgba16f),
    ETEST("SULD.D.64 RGBA16S",  0xa11fc428, suld_d_64_rgba16s),
    ETEST("SULD.D.64 RGBA16U",  0xec1dddf6, suld_d_64_rgba16u),
    ETEST("SULD.D.64 RGBA16I",  0x1898b5f7, suld_d_64_rgba16i),
    ETEST("SULD.D.64 RGBA16UI", 0xcebf735d, suld_d_64_rgba16ui),

    MTEST("SHFL.IDX",  shfl_idx,  8, 1, 1, 1, 1, 1, 0, 0, 0),
    MTEST("SHFL.UP",   shfl_up,   8, 1, 1, 1, 1, 1, 0, 0, 0),
    MTEST("SHFL.DOWN", shfl_down, 8, 1, 1, 1, 1, 1, 0, 0, 0),
    MTEST("SHFL.BFLY", shfl_bfly, 8, 1, 1, 1, 1, 1, 0, 0, 0),
};

#define NUM_TESTS (sizeof(test_descriptors) / sizeof(test_descriptors[0]))

static bool execute_test(
    struct compute_test_descriptor const* test, DkDevice device,
    DkQueue queue, DkMemBlock blk_code, uint8_t* code, DkCmdBuf cmdbuf,
    uint32_t* results)
{
    char path[64];
    snprintf(path, sizeof(path) - 1, "romfs:/%s.sass.bin", test->sass_file);
    FILE* const file = fopen(path, "rb");
    if (!file)
    {
        printf("File \"%s\" not found! Aborting...\n", path);
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    size_t const sass_size = (size_t)ftell(file);
    rewind(file);
    uint8_t* const sass = malloc(sass_size);
    if (!sass)
    {
        printf("Out of memory! Aborting...\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    if (fread(sass, 1, sass_size, file) != sass_size)
    {
        printf("Failed loading SASS code! Aborting...\n");
        fclose(file);
        free(sass);
        exit(EXIT_FAILURE);
    }
    fclose(file);

    generate_compute_dksh(code, sass_size, sass, 8,
        test->workgroup_x_minus_1 + 1, test->workgroup_y_minus_1 + 1,
        test->workgroup_z_minus_1 + 1, test->local_mem_size,
        test->shared_mem_size, test->num_barriers);
    free(sass);

    DkShader shader;
    DkShader const* shaders = &shader;
    DkShaderMaker shader_mk;
    dkShaderMakerDefaults(&shader_mk, blk_code, 0);
    dkShaderInitialize(&shader, &shader_mk);

    dkCmdBufClear(cmdbuf);
    dkCmdBufBindShaders(cmdbuf, DkStageFlag_Compute, &shaders, 1);

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
    {
        return test->check_results(results);
    }

    if (results[0] != test->expected_value)
    {
        printf("exp %08x got %08x ", test->expected_value, *(uint32_t*)results);
        return false;
    }
    return true;
}

void run_compute_tests(DkDevice device, DkQueue queue, bool automatic_mode)
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

        bool pass = execute_test(
            test, device, queue, blk_code, code_data, cmdbuf, ssbo_data);
        if (!pass)
            ++failures;
        puts(pass ? "Passed" : "Failed");

        consoleUpdate(NULL);

        if (!automatic_mode && i != 0 && i % 43 == 0)
        {
            printf("Press A to continue...");
            wait_for_input();
        }
    }

    printf("\n%3d%% tests passed, %zd tests failed out of %zd\n\n",
        (int)((NUM_TESTS - failures) * 100 / (float)NUM_TESTS), failures,
        NUM_TESTS);

    dkMemBlockDestroy(blk_ssbo);
    dkMemBlockDestroy(blk_code);
    dkCmdBufDestroy(cmdbuf);
    dkMemBlockDestroy(blk_cmdbuf);
}

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "dksh_gen.h"

struct dksh_header
{
    uint32_t magic; // DKSH_MAGIC
    uint32_t header_sz; // sizeof(DkshHeader)
    uint32_t control_sz;
    uint32_t code_sz;
    uint32_t programs_off;
    uint32_t num_programs;
};

struct dksh_program_header
{
    uint32_t type;
    uint32_t entrypoint;
    uint32_t num_gprs;
    uint32_t constbuf1_off;
    uint32_t constbuf1_sz;
    uint32_t per_warp_scratch_sz;
    union
    {
        struct
        {
            uint32_t alt_entrypoint;
            uint32_t alt_num_gprs;
        } vert;
        struct
        {
            bool has_table_3d1;
            bool early_fragment_tests;
            bool post_depth_coverage;
            bool sample_shading;
            uint32_t table_3d1[4];
            uint32_t param_d8;
            uint16_t param_65b;
            uint16_t param_489;
        } frag;
        struct
        {
            bool flag_47c;
            bool has_table_490;
            bool _padding[2];
            uint32_t table_490[8];
        } geom;
        struct
        {
            uint32_t param_c8;
        } tess_eval;
        struct
        {
            uint32_t block_dims[3];
            uint32_t shared_mem_sz;
            uint32_t local_pos_mem_sz;
            uint32_t local_neg_mem_sz;
            uint32_t crs_sz;
            uint32_t num_barriers;
        } comp;
    };
    uint32_t reserved;
};
static_assert(sizeof(struct dksh_program_header) == 64, "Wrong size");

static size_t align256(size_t n)
{
    return (n + 0xFF) & ~0xFF;
}

static size_t align8(size_t n)
{
    return (n + 0xF) & ~0xF;
}

size_t calculate_compute_dksh_size(size_t code_size)
{
    size_t headers = sizeof(struct dksh_header) + sizeof(struct dksh_program_header);
    return align256(headers) + code_size;
}

void generate_compute_dksh(
    uint8_t* dksh, size_t code_size, uint8_t const* code, int num_gprs,
    int block_dim_x, int block_dim_y, int block_dim_z, int local_mem_size,
    int shared_mem_size, int num_barriers)
{
    struct dksh_header header;
    header.magic = 0x48534B44; // DKSH
    header.header_sz = sizeof(struct dksh_header);
    header.control_sz = align256(sizeof(struct dksh_header) + sizeof(struct dksh_program_header));
    header.code_sz = align256(code_size);
    header.programs_off = sizeof(struct dksh_header);
    header.num_programs = 1;

    int local_pos_sz = align8(local_mem_size);
    int local_neg_sz = 0;
    int crs_sz = 0x800;

    struct dksh_program_header prog;
    prog.type = 5; // compute
    prog.entrypoint = 0;
    prog.num_gprs = num_gprs;
    prog.constbuf1_off = 0;
    prog.constbuf1_sz = 0;
    prog.per_warp_scratch_sz = (local_pos_sz + local_neg_sz) * 32 + crs_sz;
    prog.comp.block_dims[0] = block_dim_x;
    prog.comp.block_dims[1] = block_dim_y;
    prog.comp.block_dims[2] = block_dim_z;
    prog.comp.shared_mem_sz = align256(shared_mem_size);
    prog.comp.local_pos_mem_sz = local_pos_sz;
    prog.comp.local_neg_mem_sz = local_neg_sz;
    prog.comp.crs_sz = crs_sz;
    prog.comp.num_barriers = num_barriers;
    prog.reserved = 0;

    size_t begin_size = sizeof(header) + sizeof(prog);
    memcpy(dksh, &header, sizeof(header));
    memcpy(dksh + sizeof(header), &prog, sizeof(prog));
    memset(dksh + begin_size, 0, align256(begin_size) - begin_size);
    memcpy(dksh + align256(begin_size), code, code_size);
}

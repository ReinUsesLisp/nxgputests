#pragma once
#include <stddef.h>
#include <stdint.h>

size_t calculate_compute_dksh_size(size_t code_size);

void generate_compute_dksh(
    uint8_t* dksh, size_t code_size, uint8_t const* code, int num_gprs,
    int block_dim_x, int block_dim_y, int block_dim_z, int local_mem_size,
    int shared_mem_size, int num_barriers);

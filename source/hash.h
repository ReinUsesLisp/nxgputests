#pragma once

#include <stdio.h>

#include <switch.h>
#include <deko3d.h>

struct sha256_hash
{
	u8 bytes[SHA256_HASH_SIZE];
};

void print_sha256(FILE* stream, struct sha256_hash hash);

void print_sha256_in_c(FILE* stream, struct sha256_hash hash);

struct sha256_hash hash_memblock(DkMemBlock memblock);

bool compare_hash(DkMemBlock memblock, u64 v1, u64 v2, u64 v3, u64 v4);

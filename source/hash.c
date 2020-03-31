#include <stdio.h>
#include <string.h>

#include <switch.h>
#include <deko3d.h>

#include "hash.h"

void print_sha256(FILE* stream, struct sha256_hash hash)
{
    fprintf(stream, "0x");
    for (int i = 0; i < SHA256_HASH_SIZE; ++i) {
        fprintf(stream, "%02x", hash.bytes[i]);
    }
}

void print_sha256_in_c(FILE* stream, struct sha256_hash hash)
{
    fprintf(stream, "static const u64 expected_hash[] = { ");
    for (size_t i = 0; i < SHA256_HASH_SIZE; i += sizeof(u64)) {
        uint64_t value;
        memcpy(&value, hash.bytes + i, sizeof(value));
        fprintf(stream, "0x%016lx, ", value);
    }
    fprintf(stream, "};\n");
}

struct sha256_hash hash_memblock(DkMemBlock memblock)
{
    size_t size = (size_t)dkMemBlockGetSize(memblock);
    uint64_t* data = dkMemBlockGetCpuAddr(memblock);

    struct sha256_hash hash;
    sha256CalculateHash(&hash, data, size);
    return hash;
}

bool compare_hash(DkMemBlock memblock, u64 v1, u64 v2, u64 v3, u64 v4)
{
    u64 const expected[] = {v1, v2, v3, v4};

    struct sha256_hash hash = hash_memblock(memblock);
    return memcmp(&hash, expected, sizeof(hash)) == 0;
}

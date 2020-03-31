#include <switch.h>
#include <deko3d.h>

#include "hash.h"

u64 hash_memblock(DkMemBlock memblock)
{
    size_t size = (size_t)dkMemBlockGetSize(memblock);
    uint64_t* data = dkMemBlockGetCpuAddr(memblock);

    u64 sha256[4];
    sha256CalculateHash(&sha256, data, size);
    return sha256[0] ^ sha256[1] ^ sha256[2] ^ sha256[3];
}

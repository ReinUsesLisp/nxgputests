#include <stdbool.h>
#include <stdint.h>

#include <deko3d.h>

#include "helper.h"

DkMemBlock make_memory_block(DkDevice device, size_t size, uint32_t flags)
{
	size = (size + DK_MEMBLOCK_ALIGNMENT - 1) & ~(DK_MEMBLOCK_ALIGNMENT - 1);

	DkMemBlockMaker maker;
	dkMemBlockMakerDefaults(&maker, device, (uint32_t)size);
	maker.flags = flags;
	return dkMemBlockCreate(&maker);
}
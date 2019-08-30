#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <deko3d.h>

#include "helper.h"

struct descriptor_set
{
	DkMemBlock memblock;
	DkGpuAddr gpu_addr;
	DkImageDescriptor* descriptors;
};

struct image
{
	DkImage image;
	DkMemBlock memblock;
	void* memory;
};

static struct descriptor_set make_image_descriptor_set(DkDevice device, size_t num)
{
	struct descriptor_set obj;
	obj.memblock = make_memory_block(device, num * sizeof(DkImageDescriptor),
		DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
	obj.gpu_addr = dkMemBlockGetGpuAddr(obj.memblock);
	obj.descriptors = dkMemBlockGetCpuAddr(obj.memblock);
	return obj;
}

static void destroy_descriptor_set(struct descriptor_set const* obj)
{
	dkMemBlockDestroy(obj->memblock);
}

static struct image* make_image(DkDevice device, DkImageDescriptor* descriptor,
	DkImageType type, DkImageFormat format, uint32_t width, uint32_t height,
	uint32_t depth, uint32_t stride)
{
	DkImageLayoutMaker layout_maker = {
		.device = device,
		.type = type,
		.flags = DkImageFlags_PitchLinear | DkImageFlags_UsageLoadStore,
		.format = format,
		.dimensions = {width, height, depth},
		.mipLevels = 1,
		.pitchStride = stride,
	};
	DkImageLayout layout;
	dkImageLayoutInitialize(&layout, &layout_maker);

	uint64_t size = dkImageLayoutGetSize(&layout);
	assert(size < DK_MEMBLOCK_ALIGNMENT);

	struct image* obj = malloc(sizeof *obj);
	obj->memblock = make_memory_block(device, DK_MEMBLOCK_ALIGNMENT,
		DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached
			| DkMemBlockFlags_ZeroFillInit | DkMemBlockFlags_Image);
	obj->memory = dkMemBlockGetCpuAddr(obj->memblock);

	dkImageInitialize(&obj->image, &layout, obj->memblock, 0);

	DkImageView image_view = {
		.pImage = &obj->image,
		.type = type,
		.format = format,
		.swizzle =
			{DkSwizzle_Red, DkSwizzle_Green, DkSwizzle_Blue, DkSwizzle_Alpha},
	};
	dkImageDescriptorInitialize(descriptor, &image_view, true, false);

	return obj;
}

static void destroy_image(struct image* obj)
{
	dkMemBlockDestroy(obj->memblock);
	free(obj);
}

DEFINE_ETEST(sust_p_rgba)
{
	struct descriptor_set set = make_image_descriptor_set(device, 1);
	struct image *image = make_image(
		device, &set.descriptors[0],
		DkImageType_2D, DkImageFormat_R32_Float, 1, 1, 1, 32);

	dkCmdBufBindImageDescriptorSet(cmdbuf, set.gpu_addr, 1);
	dkCmdBufBindImage(cmdbuf, DkStage_Compute, 0, dkMakeImageHandle(0));
	dkCmdBufDispatchCompute(cmdbuf, 1, 1, 1);

	dkQueueSubmitCommands(queue, dkCmdBufFinishList(cmdbuf));
	dkQueueWaitIdle(queue);

	memcpy(results, image->memory, sizeof(uint32_t));

	destroy_image(image);
	destroy_descriptor_set(&set);
}

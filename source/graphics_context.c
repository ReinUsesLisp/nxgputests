#include <string.h>
#include <assert.h>

#include <deko3d.h>

#include "graphics_context.h"
#include "helper.h"

static int bpp(DkImageFormat format)
{
	switch (format)
	{
	case DkImageFormat_RGBA8_Unorm:
		return 4;
	case DkImageFormat_RGBA32_Float:
		return 16;
	default:
		assert(false);
		return 1;
	}
}

void reset_context(struct gfx_context* ctx)
{
	for (size_t i = 0; i < ctx->num_memblocks; ++i)
		dkMemBlockDestroy(ctx->memblocks[i]);
	ctx->num_memblocks = 0;

	for (size_t i = 0; i < ctx->num_cmdbufs; ++i)
		dkCmdBufDestroy(ctx->cmdbufs[i]);
	ctx->num_cmdbufs = 0;
}

DkMemBlock make_memblock(struct gfx_context* ctx, size_t size)
{
	DkMemBlock memblock = make_memory_block(ctx->device, size,
		DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
	size_t real_size = (size_t)dkMemBlockGetSize(memblock);
	void* data = dkMemBlockGetCpuAddr(memblock);
	memset(data, 0xcc, real_size);

	ctx->memblocks[ctx->num_memblocks++] = memblock;
	return memblock;
}

DkCmdBuf make_cmdbuf(struct gfx_context* ctx, size_t size)
{
	DkCmdBufMaker cmdbuf_mk;
    dkCmdBufMakerDefaults(&cmdbuf_mk, ctx->device);
    DkCmdBuf const cmdbuf = dkCmdBufCreate(&cmdbuf_mk);
    dkCmdBufAddMemory(cmdbuf, make_memblock(ctx, size), 0, size);

	ctx->cmdbufs[ctx->num_cmdbufs++] = cmdbuf;
	return cmdbuf;
}

void make_linear_render_target(
	struct gfx_context* ctx, DkImageFormat format, int width, int height,
	DkImage* image, DkMemBlock* memblock)
{
	DkImageLayoutMaker layout_mk;
	dkImageLayoutMakerDefaults(&layout_mk, ctx->device);
	layout_mk.flags = DkImageFlags_PitchLinear | DkImageFlags_UsageRender;
	layout_mk.format = format;
	layout_mk.dimensions[0] = width;
	layout_mk.dimensions[1] = height;
	layout_mk.mipLevels = 1;
	layout_mk.pitchStride = width * bpp(format);

	DkImageLayout layout;
	dkImageLayoutInitialize(&layout, &layout_mk);

	*memblock = make_memblock(ctx, dkImageLayoutGetSize(&layout));

	dkImageInitialize(image, &layout, *memblock, 0);
}

DkImageView make_image_view(DkImage const* image)
{
	DkImageView image_view;
	dkImageViewDefaults(&image_view, image);
	return image_view;
}

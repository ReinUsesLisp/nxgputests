#pragma once

#include <deko3d.h>

struct gfx_context
{
	DkDevice device;
	DkQueue queue;
	size_t num_memblocks;
	size_t num_cmdbufs;
	DkMemBlock memblocks[128];
	DkCmdBuf cmdbufs[4];
};

void reset_context(struct gfx_context* ctx);

DkMemBlock make_memblock(struct gfx_context* ctx, size_t size);

DkCmdBuf make_cmdbuf(struct gfx_context* ctx, size_t size);

void make_linear_render_target(
	struct gfx_context* ctx, DkImageFormat format, int width, int height,
	DkImage* image, DkMemBlock* memblock);

DkImageView make_image_view(DkImage const* image);

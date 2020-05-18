#pragma once

#include <deko3d.h>

#define BLOCK_NONE 0
#define BLOCK_IMAGE 1
#define BLOCK_CODE 2

struct gfx_context
{
	DkDevice device;
	DkQueue queue;
	size_t num_memblocks;
	size_t num_cmdbufs;
	size_t num_shaders;
	DkMemBlock memblocks[128];
	DkCmdBuf cmdbufs[4];
	DkShader shaders[16];
};

void reset_context(struct gfx_context* ctx);

DkMemBlock make_memblock(struct gfx_context* ctx, size_t size, int type);

DkCmdBuf make_cmdbuf(struct gfx_context* ctx, size_t size);

void make_linear_render_target(
	struct gfx_context* ctx, DkImageFormat format, int width, int height,
	DkImage* image, DkMemBlock* memblock);

DkImageView make_image_view(DkImage const* image);

DkShader make_shader(struct gfx_context* ctx, char const* glsl_name);

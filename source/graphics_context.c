#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <deko3d.h>

#include "graphics_context.h"
#include "helper.h"

static int bpp(DkImageFormat format)
{
    switch (format)
    {
    case DkImageFormat_S8:
        return 1;
    case DkImageFormat_Z16:
        return 2;
    case DkImageFormat_RGBA8_Unorm:
    case DkImageFormat_Z24X8:
    case DkImageFormat_ZF32:
    case DkImageFormat_Z24S8:
        return 4;
    case DkImageFormat_ZF32_X24S8:
        return 8;
    case DkImageFormat_RGBA32_Float:
        return 16;
    default:
        printf("no format %d\n", format);
        return 1;
    }
}

static int blk_flags(int type)
{
    int const generic = DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached;

    switch (type)
    {
    case BLOCK_NONE:
        return generic;
    case BLOCK_IMAGE:
        return generic | DkMemBlockFlags_Image;
    case BLOCK_CODE:
        return generic | DkMemBlockFlags_Code;
    default:
        printf("invalid type %d\n", type);
        return generic;
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

DkMemBlock make_memblock(struct gfx_context* ctx, size_t size, int type)
{
    DkMemBlock memblock = make_memory_block(ctx->device, size, blk_flags(type));
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
    dkCmdBufAddMemory(cmdbuf, make_memblock(ctx, size, BLOCK_NONE), 0, size);

    ctx->cmdbufs[ctx->num_cmdbufs++] = cmdbuf;
    return cmdbuf;
}

void make_linear_render_target(
    struct gfx_context* ctx, DkImageFormat format, int width, int height,
    DkImage* image, DkMemBlock* memblock)
{
    DkImageLayoutMaker layout_mk;
    dkImageLayoutMakerDefaults(&layout_mk, ctx->device);
    layout_mk.flags = DkImageFlags_UsageRender;
    layout_mk.format = format;
    layout_mk.dimensions[0] = width;
    layout_mk.dimensions[1] = height;
    layout_mk.mipLevels = 1;
    layout_mk.pitchStride = width * bpp(format);

    DkImageLayout layout;
    dkImageLayoutInitialize(&layout, &layout_mk);

    *memblock = make_memblock(ctx, dkImageLayoutGetSize(&layout), BLOCK_IMAGE);

    dkImageInitialize(image, &layout, *memblock, 0);
}

DkImageView make_image_view(DkImage const* image)
{
    DkImageView image_view;
    dkImageViewDefaults(&image_view, image);
    return image_view;
}

DkShader make_shader(struct gfx_context* ctx, char const* glsl_name)
{
    char path[64];
    snprintf(path, sizeof(path) - 1, "romfs:/%s.dksh", glsl_name);
    FILE* const file = fopen(path, "rb");
    if (!file)
    {
        printf("Failed to open shader \"%s\"! Aborting...\n", path);
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    size_t const dksh_size = (size_t)ftell(file);
    rewind(file);

    DkMemBlock const dksh_blk = make_memblock(ctx, dksh_size, BLOCK_CODE);
    uint8_t* const dksh = dkMemBlockGetCpuAddr(dksh_blk);
    if (fread(dksh, 1, dksh_size, file) != dksh_size)
    {
        printf("Failed to read DKSH shader! Aborting...\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    fclose(file);

    DkShader shader;
    DkShaderMaker shader_mk;
    dkShaderMakerDefaults(&shader_mk, dksh_blk, 0);
    dkShaderInitialize(&shader, &shader_mk);
    return shader;
}

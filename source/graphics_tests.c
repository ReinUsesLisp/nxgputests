#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <deko3d.h>

#include "graphics_tests.h"
#include "graphics_context.h"
#include "helper.h"
#include "hash.h"

#define TEST(name, expected) { name_##name, name, expected }

#define DEFINE_TEST(name) \
    static char name_##name[] = #name; \
    static DkMemBlock name(struct gfx_context* ctx)

struct gfx_test_descriptor
{
    char* name;
    DkMemBlock (*func)(struct gfx_context*);
    u64 expected;
};

static void bind_texture(
    DkCmdBuf cmdbuf, DkGpuAddr tic_addr, DkGpuAddr tsc_addr,
    DkImageDescriptor image_desc, DkSamplerDescriptor sampler_desc,
    DkStage stage, uint32_t index)
{
    DkResHandle const handle = dkMakeTextureHandle(index, index);
    tic_addr += index * sizeof(image_desc);
    tsc_addr += index * sizeof(sampler_desc);

    dkCmdBufPushData(cmdbuf, tic_addr, &image_desc, sizeof(image_desc)); 
    dkCmdBufPushData(cmdbuf, tsc_addr, &sampler_desc, sizeof(sampler_desc));
    dkCmdBufBindTexture(cmdbuf, stage, index, handle);
}

#define BASIC_INIT(format, is_color)                                   \
    DkImage render_target;                                             \
    DkMemBlock render_target_memblock;                                 \
    make_render_target(                                                \
        ctx, DkImageFormat_##format, 64, 64, &render_target,           \
        &render_target_memblock);                                      \
    DkImageView render_target_view = make_image_view(&render_target);  \
    DkCmdBuf const cmdbuf = make_cmdbuf(ctx, 1024);                    \
    DkImageView const* const color_rt_view[] = {&render_target_view};  \
    DkImageView* zeta_rt_view = is_color ? NULL : &render_target_view; \
    dkCmdBufBindRenderTargets(                                         \
        cmdbuf, color_rt_view, is_color ? 1 : 0, zeta_rt_view);        \
    {                                                                  \
        DkViewport viewport = {0, 0, 64, 64, 0, 1};                    \
        dkCmdBufSetViewports(cmdbuf, 0, &viewport, 1);                 \
    }

#define BIND_SHADER(type, name) \
    do {                                                              \
        DkShader const shader = make_shader(ctx, name);               \
        DkShader const* const shaders = &shader;                      \
        dkCmdBufBindShaders(cmdbuf, DkStageFlag_##type, &shaders, 1); \
    } while (0);

#define BASIC_END                                                  \
    dkQueueSubmitCommands(ctx->queue, dkCmdBufFinishList(cmdbuf)); \
    dkQueueWaitIdle(ctx->queue);                                   \
    return render_target_memblock;

#define BIND_TEXTURE_POOLS \
    DkGpuAddr const tic_addr = bind_tic_pool(ctx, cmdbuf, 32); \
    DkGpuAddr const tsc_addr = bind_tsc_pool(ctx, cmdbuf, 1);

#define MAKE_IMAGE2D(name, format, width, height)                            \
    DkImage name;                                                            \
    DkImageView name ## _view;                                               \
    DkMemBlock name ## _blk;                                                 \
    make_image2d(                                                            \
        ctx, DkImageFormat_ ## format, width, height, &name, &name ## _blk); \
    dkImageViewDefaults(&name##_view, &name);

#define MAKE_SAMPLER(name)    \
    DkSampler name;           \
    dkSamplerDefaults(&name);

#define REGISTER_IMAGE(name)                                                 \
    DkImageDescriptor name ## _desc;                                         \
    dkImageDescriptorInitialize(&name ## _desc, &name##_view, false, false);

#define REGISTER_SAMPLER(name)                            \
    DkSamplerDescriptor name ## _desc;                    \
    dkSamplerDescriptorInitialize(&name ## _desc, &name);

#define BIND_TEXTURE(image, sampler, stage, index)                \
    bind_texture(                                                 \
        cmdbuf, tic_addr, tsc_addr, image##_desc, sampler##_desc, \
        DkStage_##stage, index);

DEFINE_TEST(clear)
{
    BASIC_INIT(RGBA8_Unorm, true)

    static float const clear_color[4] = {1.0f, 0.8f, 0.25f, 0.125f};
    dkCmdBufClearColor(cmdbuf, 0, DkColorMask_RGBA, clear_color);

    BASIC_END
}

DEFINE_TEST(clear_scissor)
{
    BASIC_INIT(RGBA32_Float, true)

    static float const background_color[4] = {0.13f, 0.54f, 65.2f, -3.5f};
    static float const scissor_color[4] = {102.4f, 0.0f, -43.2f, 8.13f};
    static DkScissor const scissor = {13, 17, 23, 45};
    static DkScissor const default_scissor = {0, 0, 65535, 65535};
    dkCmdBufClearColor(cmdbuf, 0, DkColorMask_RGBA, background_color);
    dkCmdBufSetScissors(cmdbuf, 0, &scissor, 1);
    dkCmdBufClearColor(cmdbuf, 0, DkColorMask_RGBA, scissor_color);
    dkCmdBufSetScissors(cmdbuf, 0, &default_scissor, 1);

    BASIC_END
}

DEFINE_TEST(clear_scissor_masked)
{
    BASIC_INIT(RGBA32_Float, true)

    static float const background_color[4] = {0.13f, 0.54f, 65.2f, -3.5f};
    static float const scissor_color[4] = {102.4f, 0.0f, -43.2f, 8.13f};
    static DkScissor const scissor = {13, 17, 23, 45};
    static DkScissor const default_scissor = {0, 0, 65535, 65535};
    dkCmdBufClearColor(cmdbuf, 0, DkColorMask_RGBA, background_color);
    dkCmdBufSetScissors(cmdbuf, 0, &scissor, 1);
    dkCmdBufClearColor(cmdbuf, 0, DkColorMask_RGB, scissor_color);
    dkCmdBufSetScissors(cmdbuf, 0, &default_scissor, 1);

    BASIC_END
}

DEFINE_TEST(clear_depth)
{
    DkImage render_target;
    DkMemBlock render_target_memblock;
    make_render_target(
        ctx, DkImageFormat_ZF32, 64, 64, &render_target, &render_target_memblock);
    DkImageView render_target_view = make_image_view(&render_target);

    DkCmdBuf cmdbuf = make_cmdbuf(ctx, 1024);

    DkDepthStencilState ds;
    dkDepthStencilStateDefaults(&ds);
    ds.depthTestEnable = 0;
    ds.depthWriteEnable = 0;
    dkCmdBufBindDepthStencilState(cmdbuf, &ds);

    dkCmdBufBindRenderTargets(cmdbuf, NULL, 0, &render_target_view);

    dkCmdBufClearDepthStencil(cmdbuf, true, 32.15f, 0, 0);

    BASIC_END
}

DEFINE_TEST(basic_draw)
{
    BASIC_INIT(RGBA32_Float, true)

    BIND_SHADER(Vertex, "full_screen_tri.vert")
    BIND_SHADER(Fragment, "red.frag")

    dkCmdBufDraw(cmdbuf, DkPrimitive_Triangles, 3, 1, 0, 0);

    BASIC_END
}

DEFINE_TEST(sample_depth)
{
    BASIC_INIT(RGBA32_Float, true)

    BIND_SHADER(Vertex, "full_screen_tri.vert")
    BIND_SHADER(Fragment, "sample.frag")

    BIND_TEXTURE_POOLS

    MAKE_SAMPLER(sampler)
    MAKE_IMAGE2D(image, Z24S8, 32, 32)
    image_view.dsSource = DkDsSource_Depth;

    memset(dkMemBlockGetCpuAddr(image_blk), 0xaa, dkMemBlockGetSize(image_blk));

    REGISTER_IMAGE(image)
    REGISTER_SAMPLER(sampler)

    BIND_TEXTURE(image, sampler, Fragment, 0)

    dkCmdBufDraw(cmdbuf, DkPrimitive_Triangles, 3, 1, 0, 0);

    BASIC_END
}

DEFINE_TEST(sample_stencil)
{
    BASIC_INIT(RGBA32_Float, true)

    BIND_SHADER(Vertex, "full_screen_tri.vert")
    BIND_SHADER(Fragment, "sample.frag")

    BIND_TEXTURE_POOLS

    MAKE_SAMPLER(sampler)
    MAKE_IMAGE2D(image, Z24S8, 32, 32)
    image_view.dsSource = DkDsSource_Stencil;

    memset(dkMemBlockGetCpuAddr(image_blk), 0xaa, dkMemBlockGetSize(image_blk));

    REGISTER_IMAGE(image)
    REGISTER_SAMPLER(sampler)

    BIND_TEXTURE(image, sampler, Fragment, 0)

    dkCmdBufDraw(cmdbuf, DkPrimitive_Triangles, 3, 1, 0, 0);

    BASIC_END
}

static DkMemBlock sampler_template(struct gfx_context* ctx, DkSampler sampler)
{
    BASIC_INIT(RGBA8_Unorm, true)

    BIND_SHADER(Vertex, "full_screen_tri.vert")
    BIND_SHADER(Fragment, "sample_wrap.frag")

    BIND_TEXTURE_POOLS

    MAKE_IMAGE2D(image, RGBA8_Unorm, 64, 64)

    sampler.minFilter = DkFilter_Linear;
    sampler.magFilter = DkFilter_Linear;

    uint8_t* data = dkMemBlockGetCpuAddr(image_blk);
    size_t size = dkMemBlockGetSize(image_blk);
    memset(data, 0, size);

    uint32_t const values[] = {0xdeadbeef, 0xcafecafe, 0xdedede00, 0xacdc0000};
    for (size_t i = 0; i < size; i += sizeof(uint32_t)) {
        size_t seed = (i >> 2) + i / 73 + i / 103;
        uint32_t value = values[seed % 4];
        memcpy(&data[i], &value, sizeof(uint32_t));
    }

    REGISTER_IMAGE(image)
    REGISTER_SAMPLER(sampler)

    BIND_TEXTURE(image, sampler, Fragment, 0)

    dkCmdBufDraw(cmdbuf, DkPrimitive_Triangles, 3, 1, 0, 0);

    BASIC_END
}

static void set_arbitrary_border(DkSampler* sampler)
{
    sampler->borderColor[0].value_f = 0.2;
    sampler->borderColor[1].value_f = 0.8;
    sampler->borderColor[2].value_f = 0.3;
    sampler->borderColor[3].value_f = 0.5;
}

DEFINE_TEST(sampler_repeat)
{
    MAKE_SAMPLER(sampler);
    sampler.wrapMode[0] = DkWrapMode_Repeat;
    sampler.wrapMode[1] = DkWrapMode_Repeat;
    return sampler_template(ctx, sampler);
}

DEFINE_TEST(sampler_mirrored_repeat)
{
    MAKE_SAMPLER(sampler);
    sampler.wrapMode[0] = DkWrapMode_MirroredRepeat;
    sampler.wrapMode[1] = DkWrapMode_MirroredRepeat;
    return sampler_template(ctx, sampler);
}

DEFINE_TEST(sampler_clamp_to_edge)
{
    MAKE_SAMPLER(sampler);
    sampler.wrapMode[0] = DkWrapMode_ClampToEdge;
    sampler.wrapMode[1] = DkWrapMode_ClampToEdge;
    return sampler_template(ctx, sampler);
}

DEFINE_TEST(sampler_clamp_to_border)
{
    MAKE_SAMPLER(sampler);
    sampler.wrapMode[0] = DkWrapMode_ClampToBorder;
    sampler.wrapMode[1] = DkWrapMode_ClampToBorder;
    return sampler_template(ctx, sampler);
}

DEFINE_TEST(sampler_clamp_to_border_custom)
{
    MAKE_SAMPLER(sampler);
    sampler.wrapMode[0] = DkWrapMode_ClampToBorder;
    sampler.wrapMode[1] = DkWrapMode_ClampToBorder;
    set_arbitrary_border(&sampler);
    return sampler_template(ctx, sampler);
}

DEFINE_TEST(sampler_clamp)
{
    MAKE_SAMPLER(sampler);
    sampler.wrapMode[0] = DkWrapMode_Clamp;
    sampler.wrapMode[1] = DkWrapMode_Clamp;
    set_arbitrary_border(&sampler);
    return sampler_template(ctx, sampler);
}

DEFINE_TEST(sampler_mirror_clamp_to_edge)
{
    MAKE_SAMPLER(sampler);
    sampler.wrapMode[0] = DkWrapMode_MirrorClampToEdge;
    sampler.wrapMode[1] = DkWrapMode_MirrorClampToEdge;
    return sampler_template(ctx, sampler);
}

DEFINE_TEST(sampler_mirror_clamp_to_border)
{
    MAKE_SAMPLER(sampler);
    sampler.wrapMode[0] = DkWrapMode_MirrorClampToBorder;
    sampler.wrapMode[1] = DkWrapMode_MirrorClampToBorder;
    return sampler_template(ctx, sampler);
}

DEFINE_TEST(sampler_mirror_clamp)
{
    MAKE_SAMPLER(sampler);
    sampler.wrapMode[0] = DkWrapMode_MirrorClamp;
    sampler.wrapMode[1] = DkWrapMode_MirrorClamp;
    set_arbitrary_border(&sampler);
    return sampler_template(ctx, sampler);
}

DEFINE_TEST(robust_vertex_buffer)
{
    BASIC_INIT(RGBA8_Unorm, true)

    BIND_SHADER(Vertex, "basic.vert")
    BIND_SHADER(Fragment, "color.frag")

    static float const positions[] = {
        -1.0f, -1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
    };
    static float const colors[] = {
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
    };
    static DkVtxAttribState const vtx_attrib_state[] = {
        { .bufferId = 1, .size = DkVtxAttribSize_4x32, .type = DkVtxAttribType_Float, },
        { .bufferId = 0, .size = DkVtxAttribSize_4x32, .type = DkVtxAttribType_Float, },
    };
    static DkVtxBufferState const vtx_buffer_state[] = {
        { .stride = 4 * sizeof(float), },
        { .stride = 4 * sizeof(float), },
    };

    DkMemBlock positions_blk = make_memblock(ctx, sizeof(positions), BLOCK_NONE);
    DkMemBlock colors_blk = make_memblock(ctx, sizeof(colors), BLOCK_NONE);

    memcpy(dkMemBlockGetCpuAddr(positions_blk), positions, sizeof(positions));
    memcpy(dkMemBlockGetCpuAddr(colors_blk), colors, sizeof(colors));

    DkBufExtents const buffer_extents[] = {
        { .addr = dkMemBlockGetGpuAddr(colors_blk),    .size = sizeof(float) * 4 * 2, },
        { .addr = dkMemBlockGetGpuAddr(positions_blk), .size = sizeof(float) * 4 * 4, },
    };

    dkCmdBufBindVtxAttribState(cmdbuf, vtx_attrib_state, 2);
    dkCmdBufBindVtxBufferState(cmdbuf, vtx_buffer_state, 2);
    dkCmdBufBindVtxBuffers(cmdbuf, 0, buffer_extents, 2);
    dkCmdBufDraw(cmdbuf, DkPrimitive_Quads, 4, 1, 0, 0);

    BASIC_END
}

static void rendertarget_test_template(struct gfx_context* ctx, DkCmdBuf cmdbuf)
{
    BIND_SHADER(Vertex, "full_screen_tri.vert")
    BIND_SHADER(Fragment, "fuzz_color.frag")

    dkCmdBufDraw(cmdbuf, DkPrimitive_Triangles, 3, 1, 0, 0);
}

#define DEFINE_RT_FORMAT_TEST(format)            \
    DEFINE_TEST(rendertarget_ ## format)         \
    {                                            \
        BASIC_INIT(format, true)                 \
                                                 \
        rendertarget_test_template(ctx, cmdbuf); \
                                                 \
        BASIC_END                                \
    }

DEFINE_RT_FORMAT_TEST(R8_Unorm)
DEFINE_RT_FORMAT_TEST(R8_Snorm)
DEFINE_RT_FORMAT_TEST(R8_Uint)
DEFINE_RT_FORMAT_TEST(R8_Sint)
DEFINE_RT_FORMAT_TEST(R16_Float)
DEFINE_RT_FORMAT_TEST(R16_Unorm)
DEFINE_RT_FORMAT_TEST(R16_Snorm)
DEFINE_RT_FORMAT_TEST(R16_Uint)
DEFINE_RT_FORMAT_TEST(R16_Sint)
DEFINE_RT_FORMAT_TEST(R32_Float)
DEFINE_RT_FORMAT_TEST(R32_Uint)
DEFINE_RT_FORMAT_TEST(R32_Sint)
DEFINE_RT_FORMAT_TEST(RG8_Unorm)
DEFINE_RT_FORMAT_TEST(RG8_Snorm)
DEFINE_RT_FORMAT_TEST(RG8_Uint)
DEFINE_RT_FORMAT_TEST(RG8_Sint)
DEFINE_RT_FORMAT_TEST(RG16_Float)
DEFINE_RT_FORMAT_TEST(RG16_Unorm)
DEFINE_RT_FORMAT_TEST(RG16_Snorm)
DEFINE_RT_FORMAT_TEST(RG16_Uint)
DEFINE_RT_FORMAT_TEST(RG16_Sint)
DEFINE_RT_FORMAT_TEST(RG32_Float)
DEFINE_RT_FORMAT_TEST(RG32_Uint)
DEFINE_RT_FORMAT_TEST(RG32_Sint)
DEFINE_RT_FORMAT_TEST(RGBA8_Unorm)
DEFINE_RT_FORMAT_TEST(RGBA8_Snorm)
DEFINE_RT_FORMAT_TEST(RGBA8_Uint)
DEFINE_RT_FORMAT_TEST(RGBA8_Sint)
DEFINE_RT_FORMAT_TEST(RGBA16_Float)
DEFINE_RT_FORMAT_TEST(RGBA16_Unorm)
DEFINE_RT_FORMAT_TEST(RGBA16_Snorm)
DEFINE_RT_FORMAT_TEST(RGBA16_Uint)
DEFINE_RT_FORMAT_TEST(RGBA16_Sint)
DEFINE_RT_FORMAT_TEST(RGBA32_Float)
DEFINE_RT_FORMAT_TEST(RGBA32_Uint)
DEFINE_RT_FORMAT_TEST(RGBA32_Sint)
DEFINE_RT_FORMAT_TEST(RGBA8_Unorm_sRGB)
DEFINE_RT_FORMAT_TEST(RGB10A2_Unorm)
DEFINE_RT_FORMAT_TEST(RGB10A2_Uint)
DEFINE_RT_FORMAT_TEST(RG11B10_Float)
DEFINE_RT_FORMAT_TEST(BGR565_Unorm)
DEFINE_RT_FORMAT_TEST(BGR5A1_Unorm)

static DkMemBlock attrib_format_test_template(
    struct gfx_context* ctx, DkVtxAttribSize size, DkVtxAttribType type,
    int bgra)
{
    static float const data[] = { -0.3432, 0.8934, 23.1392, -0.4356 };
    static DkVtxBufferState const buffer_state = {0};

    BASIC_INIT(RGBA16_Float, true)

    BIND_SHADER(Vertex, "colored_full_screen_tri.vert");
    BIND_SHADER(Fragment, "color.frag");

    DkMemBlock color_blk = make_memblock(ctx, sizeof(data), BLOCK_NONE);
    memcpy(dkMemBlockGetCpuAddr(color_blk), data, sizeof(data));
    DkBufExtents const extent = {
        .addr = dkMemBlockGetGpuAddr(color_blk),
        .size = sizeof(data),
    };
    DkVtxAttribState state = {
        .size = size,
        .type = type,
        .isBgra = bgra,
    };

    dkCmdBufBindVtxAttribState(cmdbuf, &state, 1);
    dkCmdBufBindVtxBufferState(cmdbuf, &buffer_state, 1);
    dkCmdBufBindVtxBuffers(cmdbuf, 0, &extent, 1);
    dkCmdBufDraw(cmdbuf, DkPrimitive_Triangles, 3, 1, 0, 0);

    BASIC_END
}

#define DEFINE_ATTRIB_FORMAT_TEST_IMPL(name, type, size, bgra)             \
    DEFINE_TEST(attrib_format_ ## name ## _ ## type)                       \
    {                                                                      \
        return attrib_format_test_template(                                \
            ctx, DkVtxAttribSize ## size, DkVtxAttribType_ ## type, bgra); \
    }

#define DEFINE_ATTRIB_FORMAT_TEST(name, type, size)              \
    DEFINE_ATTRIB_FORMAT_TEST_IMPL(name, type, size, 0)          \
    DEFINE_ATTRIB_FORMAT_TEST_IMPL(name ## f, type, size, 1)

DEFINE_ATTRIB_FORMAT_TEST(R32,     Snorm,   _1x32)
DEFINE_ATTRIB_FORMAT_TEST(R32,     Unorm,   _1x32)
DEFINE_ATTRIB_FORMAT_TEST(R32,     Sint,    _1x32)
DEFINE_ATTRIB_FORMAT_TEST(R32,     Uint,    _1x32)
DEFINE_ATTRIB_FORMAT_TEST(R32,     Sscaled, _1x32)
DEFINE_ATTRIB_FORMAT_TEST(R32,     Uscaled, _1x32)
DEFINE_ATTRIB_FORMAT_TEST(R32,     Float,   _1x32)
DEFINE_ATTRIB_FORMAT_TEST(RG32,    Snorm,   _2x32)
DEFINE_ATTRIB_FORMAT_TEST(RG32,    Unorm,   _2x32)
DEFINE_ATTRIB_FORMAT_TEST(RG32,    Sint,    _2x32)
DEFINE_ATTRIB_FORMAT_TEST(RG32,    Uint,    _2x32)
DEFINE_ATTRIB_FORMAT_TEST(RG32,    Sscaled, _2x32)
DEFINE_ATTRIB_FORMAT_TEST(RG32,    Uscaled, _2x32)
DEFINE_ATTRIB_FORMAT_TEST(RG32,    Float,   _3x32)
DEFINE_ATTRIB_FORMAT_TEST(RGB32,   Snorm,   _3x32)
DEFINE_ATTRIB_FORMAT_TEST(RGB32,   Unorm,   _3x32)
DEFINE_ATTRIB_FORMAT_TEST(RGB32,   Sint,    _3x32)
DEFINE_ATTRIB_FORMAT_TEST(RGB32,   Uint,    _3x32)
DEFINE_ATTRIB_FORMAT_TEST(RGB32,   Sscaled, _3x32)
DEFINE_ATTRIB_FORMAT_TEST(RGB32,   Uscaled, _3x32)
DEFINE_ATTRIB_FORMAT_TEST(RGB32,   Float,   _3x32)
DEFINE_ATTRIB_FORMAT_TEST(RGBA32,  Snorm,   _4x32)
DEFINE_ATTRIB_FORMAT_TEST(RGBA32,  Unorm,   _4x32)
DEFINE_ATTRIB_FORMAT_TEST(RGBA32,  Sint,    _4x32)
DEFINE_ATTRIB_FORMAT_TEST(RGBA32,  Uint,    _4x32)
DEFINE_ATTRIB_FORMAT_TEST(RGBA32,  Sscaled, _4x32)
DEFINE_ATTRIB_FORMAT_TEST(RGBA32,  Uscaled, _4x32)
DEFINE_ATTRIB_FORMAT_TEST(RGBA32,  Float,   _4x32)
DEFINE_ATTRIB_FORMAT_TEST(R16,     Snorm,   _1x16)
DEFINE_ATTRIB_FORMAT_TEST(R16,     Unorm,   _1x16)
DEFINE_ATTRIB_FORMAT_TEST(R16,     Sint,    _1x16)
DEFINE_ATTRIB_FORMAT_TEST(R16,     Uint,    _1x16)
DEFINE_ATTRIB_FORMAT_TEST(R16,     Sscaled, _1x16)
DEFINE_ATTRIB_FORMAT_TEST(R16,     Uscaled, _1x16)
DEFINE_ATTRIB_FORMAT_TEST(R16,     Float,   _1x16)
DEFINE_ATTRIB_FORMAT_TEST(RG16,    Snorm,   _2x16)
DEFINE_ATTRIB_FORMAT_TEST(RG16,    Unorm,   _2x16)
DEFINE_ATTRIB_FORMAT_TEST(RG16,    Sint,    _2x16)
DEFINE_ATTRIB_FORMAT_TEST(RG16,    Uint,    _2x16)
DEFINE_ATTRIB_FORMAT_TEST(RG16,    Sscaled, _2x16)
DEFINE_ATTRIB_FORMAT_TEST(RG16,    Uscaled, _2x16)
DEFINE_ATTRIB_FORMAT_TEST(RG16,    Float,   _3x16)
DEFINE_ATTRIB_FORMAT_TEST(RGB16,   Snorm,   _3x16)
DEFINE_ATTRIB_FORMAT_TEST(RGB16,   Unorm,   _3x16)
DEFINE_ATTRIB_FORMAT_TEST(RGB16,   Sint,    _3x16)
DEFINE_ATTRIB_FORMAT_TEST(RGB16,   Uint,    _3x16)
DEFINE_ATTRIB_FORMAT_TEST(RGB16,   Sscaled, _3x16)
DEFINE_ATTRIB_FORMAT_TEST(RGB16,   Uscaled, _3x16)
DEFINE_ATTRIB_FORMAT_TEST(RGB16,   Float,   _3x16)
DEFINE_ATTRIB_FORMAT_TEST(RGBA16,  Snorm,   _4x16)
DEFINE_ATTRIB_FORMAT_TEST(RGBA16,  Unorm,   _4x16)
DEFINE_ATTRIB_FORMAT_TEST(RGBA16,  Sint,    _4x16)
DEFINE_ATTRIB_FORMAT_TEST(RGBA16,  Uint,    _4x16)
DEFINE_ATTRIB_FORMAT_TEST(RGBA16,  Sscaled, _4x16)
DEFINE_ATTRIB_FORMAT_TEST(RGBA16,  Uscaled, _4x16)
DEFINE_ATTRIB_FORMAT_TEST(RGBA16,  Float,   _4x16)
DEFINE_ATTRIB_FORMAT_TEST(R8,      Snorm,   _1x8)
DEFINE_ATTRIB_FORMAT_TEST(R8,      Unorm,   _1x8)
DEFINE_ATTRIB_FORMAT_TEST(R8,      Sint,    _1x8)
DEFINE_ATTRIB_FORMAT_TEST(R8,      Uint,    _1x8)
DEFINE_ATTRIB_FORMAT_TEST(R8,      Sscaled, _1x8)
DEFINE_ATTRIB_FORMAT_TEST(R8,      Uscaled, _1x8)
DEFINE_ATTRIB_FORMAT_TEST(RG8,     Snorm,   _2x8)
DEFINE_ATTRIB_FORMAT_TEST(RG8,     Unorm,   _2x8)
DEFINE_ATTRIB_FORMAT_TEST(RG8,     Sint,    _2x8)
DEFINE_ATTRIB_FORMAT_TEST(RG8,     Uint,    _2x8)
DEFINE_ATTRIB_FORMAT_TEST(RG8,     Sscaled, _2x8)
DEFINE_ATTRIB_FORMAT_TEST(RG8,     Uscaled, _2x8)
DEFINE_ATTRIB_FORMAT_TEST(RGB8,    Snorm,   _3x8)
DEFINE_ATTRIB_FORMAT_TEST(RGB8,    Unorm,   _3x8)
DEFINE_ATTRIB_FORMAT_TEST(RGB8,    Sint,    _3x8)
DEFINE_ATTRIB_FORMAT_TEST(RGB8,    Uint,    _3x8)
DEFINE_ATTRIB_FORMAT_TEST(RGB8,    Uscaled, _3x8)
DEFINE_ATTRIB_FORMAT_TEST(RGB8,    Sscaled, _3x8)
DEFINE_ATTRIB_FORMAT_TEST(RGBA8,   Snorm,   _4x8)
DEFINE_ATTRIB_FORMAT_TEST(RGBA8,   Unorm,   _4x8)
DEFINE_ATTRIB_FORMAT_TEST(RGBA8,   Sint,    _4x8)
DEFINE_ATTRIB_FORMAT_TEST(RGBA8,   Uint,    _4x8)
DEFINE_ATTRIB_FORMAT_TEST(RGBA8,   Sscaled, _4x8)
DEFINE_ATTRIB_FORMAT_TEST(RGBA8,   Uscaled, _4x8)
DEFINE_ATTRIB_FORMAT_TEST(RGB10A2, Snorm,   _10_10_10_2)
DEFINE_ATTRIB_FORMAT_TEST(RGB10A2, Unorm,   _10_10_10_2)
DEFINE_ATTRIB_FORMAT_TEST(RGB10A2, Sint,    _10_10_10_2)
DEFINE_ATTRIB_FORMAT_TEST(RGB10A2, Uint,    _10_10_10_2)
DEFINE_ATTRIB_FORMAT_TEST(RGB10A2, Sscaled, _10_10_10_2)
DEFINE_ATTRIB_FORMAT_TEST(RGB10A2, Uscaled, _10_10_10_2)
DEFINE_ATTRIB_FORMAT_TEST(RG11B10, Snorm,   _11_11_10)
DEFINE_ATTRIB_FORMAT_TEST(RG11B10, Unorm,   _11_11_10)
DEFINE_ATTRIB_FORMAT_TEST(RG11B10, Sint,    _11_11_10)
DEFINE_ATTRIB_FORMAT_TEST(RG11B10, Uint,    _11_11_10)
DEFINE_ATTRIB_FORMAT_TEST(RG11B10, Sscaled, _11_11_10)
DEFINE_ATTRIB_FORMAT_TEST(RG11B10, Uscaled, _11_11_10)
DEFINE_ATTRIB_FORMAT_TEST(RG11B10, Float,   _11_11_10)

static DkMemBlock sample_color_template(
    struct gfx_context* ctx, DkImageFormat format)
{
    BASIC_INIT(RGBA16_Float, true)

    BIND_SHADER(Vertex, "full_screen_tri.vert")
    BIND_SHADER(Fragment, "sample.frag")

    BIND_TEXTURE_POOLS

    MAKE_SAMPLER(sampler)

    DkImage image;
    DkImageView image_view;
    DkMemBlock image_blk;
    make_image2d(ctx, format, 32, 32, &image, &image_blk);

    dkImageViewDefaults(&image_view, &image);

    uint8_t* data = dkMemBlockGetCpuAddr(image_blk);
    size_t size = dkMemBlockGetSize(image_blk);
    memset(data, 0, size);
    for (size_t i = 0; i < size; i += sizeof(uint32_t)) {
        uint32_t value = 0xa82c2c11;
        memcpy(&data[i], &value, sizeof(uint32_t));
    }

    REGISTER_IMAGE(image)
    REGISTER_SAMPLER(sampler)

    BIND_TEXTURE(image, sampler, Fragment, 0)

    dkCmdBufDraw(cmdbuf, DkPrimitive_Triangles, 3, 1, 0, 0);

    BASIC_END
}

#define DEFINE_TEX_FORMAT_TEST(format)                               \
    DEFINE_TEST(texformat_ ## format)                                \
    {                                                                \
        return sample_color_template(ctx, DkImageFormat_ ## format); \
    }

DEFINE_TEX_FORMAT_TEST(R8_Unorm)
DEFINE_TEX_FORMAT_TEST(R8_Snorm)
DEFINE_TEX_FORMAT_TEST(R8_Uint)
DEFINE_TEX_FORMAT_TEST(R8_Sint)
DEFINE_TEX_FORMAT_TEST(R16_Float)
DEFINE_TEX_FORMAT_TEST(R16_Unorm)
DEFINE_TEX_FORMAT_TEST(R16_Snorm)
DEFINE_TEX_FORMAT_TEST(R16_Uint)
DEFINE_TEX_FORMAT_TEST(R16_Sint)
DEFINE_TEX_FORMAT_TEST(R32_Float)
DEFINE_TEX_FORMAT_TEST(R32_Uint)
DEFINE_TEX_FORMAT_TEST(R32_Sint)
DEFINE_TEX_FORMAT_TEST(RG8_Unorm)
DEFINE_TEX_FORMAT_TEST(RG8_Snorm)
DEFINE_TEX_FORMAT_TEST(RG8_Uint)
DEFINE_TEX_FORMAT_TEST(RG8_Sint)
DEFINE_TEX_FORMAT_TEST(RG16_Float)
DEFINE_TEX_FORMAT_TEST(RG16_Unorm)
DEFINE_TEX_FORMAT_TEST(RG16_Snorm)
DEFINE_TEX_FORMAT_TEST(RG16_Uint)
DEFINE_TEX_FORMAT_TEST(RG16_Sint)
DEFINE_TEX_FORMAT_TEST(RG32_Float)
DEFINE_TEX_FORMAT_TEST(RG32_Uint)
DEFINE_TEX_FORMAT_TEST(RG32_Sint)
DEFINE_TEX_FORMAT_TEST(RGB32_Float)
DEFINE_TEX_FORMAT_TEST(RGB32_Uint)
DEFINE_TEX_FORMAT_TEST(RGB32_Sint)
DEFINE_TEX_FORMAT_TEST(RGBA8_Unorm)
DEFINE_TEX_FORMAT_TEST(RGBA8_Snorm)
DEFINE_TEX_FORMAT_TEST(RGBA8_Uint)
DEFINE_TEX_FORMAT_TEST(RGBA8_Sint)
DEFINE_TEX_FORMAT_TEST(RGBA16_Float)
DEFINE_TEX_FORMAT_TEST(RGBA16_Unorm)
DEFINE_TEX_FORMAT_TEST(RGBA16_Snorm)
DEFINE_TEX_FORMAT_TEST(RGBA16_Uint)
DEFINE_TEX_FORMAT_TEST(RGBA16_Sint)
DEFINE_TEX_FORMAT_TEST(RGBA32_Float)
DEFINE_TEX_FORMAT_TEST(RGBA32_Uint)
DEFINE_TEX_FORMAT_TEST(RGBA32_Sint)
DEFINE_TEX_FORMAT_TEST(RGBX8_Unorm_sRGB)
DEFINE_TEX_FORMAT_TEST(RGBA8_Unorm_sRGB)
DEFINE_TEX_FORMAT_TEST(RGBA4_Unorm)
DEFINE_TEX_FORMAT_TEST(RGB5_Unorm)
DEFINE_TEX_FORMAT_TEST(RGB5A1_Unorm)
DEFINE_TEX_FORMAT_TEST(RGB565_Unorm)
DEFINE_TEX_FORMAT_TEST(RGB10A2_Unorm)
DEFINE_TEX_FORMAT_TEST(RGB10A2_Uint)
DEFINE_TEX_FORMAT_TEST(RG11B10_Float)
DEFINE_TEX_FORMAT_TEST(E5BGR9_Float)
DEFINE_TEX_FORMAT_TEST(RGB_BC1)
DEFINE_TEX_FORMAT_TEST(RGBA_BC1)
DEFINE_TEX_FORMAT_TEST(RGBA_BC2)
DEFINE_TEX_FORMAT_TEST(RGBA_BC3)
DEFINE_TEX_FORMAT_TEST(RGB_BC1_sRGB)
DEFINE_TEX_FORMAT_TEST(RGBA_BC1_sRGB)
DEFINE_TEX_FORMAT_TEST(RGBA_BC2_sRGB)
DEFINE_TEX_FORMAT_TEST(RGBA_BC3_sRGB)
DEFINE_TEX_FORMAT_TEST(R_BC4_Unorm)
DEFINE_TEX_FORMAT_TEST(R_BC4_Snorm)
DEFINE_TEX_FORMAT_TEST(RG_BC5_Unorm)
DEFINE_TEX_FORMAT_TEST(RG_BC5_Snorm)
DEFINE_TEX_FORMAT_TEST(RGBA_BC7_Unorm)
DEFINE_TEX_FORMAT_TEST(RGBA_BC7_Unorm_sRGB)
DEFINE_TEX_FORMAT_TEST(RGBA_BC6H_SF16_Float)
DEFINE_TEX_FORMAT_TEST(RGBA_BC6H_UF16_Float)
DEFINE_TEX_FORMAT_TEST(RGBX8_Unorm)
DEFINE_TEX_FORMAT_TEST(RGBX8_Snorm)
DEFINE_TEX_FORMAT_TEST(RGBX8_Uint)
DEFINE_TEX_FORMAT_TEST(RGBX8_Sint)
DEFINE_TEX_FORMAT_TEST(RGBX16_Float)
DEFINE_TEX_FORMAT_TEST(RGBX16_Unorm)
DEFINE_TEX_FORMAT_TEST(RGBX16_Snorm)
DEFINE_TEX_FORMAT_TEST(RGBX16_Uint)
DEFINE_TEX_FORMAT_TEST(RGBX16_Sint)
DEFINE_TEX_FORMAT_TEST(RGBX32_Float)
DEFINE_TEX_FORMAT_TEST(RGBX32_Uint)
DEFINE_TEX_FORMAT_TEST(RGBX32_Sint)
DEFINE_TEX_FORMAT_TEST(BGR565_Unorm)
DEFINE_TEX_FORMAT_TEST(BGR5_Unorm)
DEFINE_TEX_FORMAT_TEST(BGR5A1_Unorm)
DEFINE_TEX_FORMAT_TEST(A5BGR5_Unorm)
DEFINE_TEX_FORMAT_TEST(BGRX8_Unorm)
DEFINE_TEX_FORMAT_TEST(BGRA8_Unorm)
DEFINE_TEX_FORMAT_TEST(BGRX8_Unorm_sRGB)
DEFINE_TEX_FORMAT_TEST(BGRA8_Unorm_sRGB)
DEFINE_TEX_FORMAT_TEST(R_ETC2_Unorm)
DEFINE_TEX_FORMAT_TEST(R_ETC2_Snorm)
DEFINE_TEX_FORMAT_TEST(RG_ETC2_Unorm)
DEFINE_TEX_FORMAT_TEST(RG_ETC2_Snorm)
DEFINE_TEX_FORMAT_TEST(RGB_ETC2)
DEFINE_TEX_FORMAT_TEST(RGB_PTA_ETC2)
DEFINE_TEX_FORMAT_TEST(RGBA_ETC2)
DEFINE_TEX_FORMAT_TEST(RGB_ETC2_sRGB)
DEFINE_TEX_FORMAT_TEST(RGB_PTA_ETC2_sRGB)
DEFINE_TEX_FORMAT_TEST(RGBA_ETC2_sRGB)

static struct gfx_test_descriptor test_descriptors[] =
{
    TEST(clear,                          0xbe7e7dc089ef7f01),
    TEST(clear_scissor,                  0x27750a82ffacde28),
    TEST(clear_scissor_masked,           0x69292f52fbda15c1),
    TEST(clear_depth,                    0x139f492006278563),
    TEST(basic_draw,                     0xfed0c683282b8c9b),
    TEST(sample_depth,                   0x8f8453b80d43b141),
    TEST(sample_stencil,                 0x890be20f007a5d63),
    TEST(robust_vertex_buffer,           0x03406eca6029fae3),
    TEST(sampler_repeat,                 0x4b001fb424249863),
    TEST(sampler_mirrored_repeat,        0xf73c778ff8e0627d),
    TEST(sampler_clamp_to_edge,          0x4ba0154827a52fd0),
    TEST(sampler_clamp_to_border,        0x320ea6e9a42b7446),
    TEST(sampler_clamp_to_border_custom, 0x9bca3c0b1e30f2cd),
    TEST(sampler_clamp,                  0xe0f3047488987068),
    TEST(sampler_mirror_clamp_to_edge,   0x9698b023de1b3ab6),
    TEST(sampler_mirror_clamp_to_border, 0x8fb5abb331f10c30),
    TEST(sampler_mirror_clamp,           0x9519c4e1f29b47f6),
    TEST(rendertarget_R8_Unorm,          0xd0aa953f5a8e22ad),
    TEST(rendertarget_R8_Snorm,          0x2ea4001217fe238e),
    TEST(rendertarget_R8_Uint,           0x5d69079daa54ffeb),
    TEST(rendertarget_R8_Sint,           0xb79de734743a4689),
    TEST(rendertarget_R16_Float,         0x79fbae9bc6d3972d),
    TEST(rendertarget_R16_Unorm,         0xb343d24ed714334f),
    TEST(rendertarget_R16_Snorm,         0xfbda7c8204645069),
    TEST(rendertarget_R16_Uint,          0x6f079795eb063566),
    TEST(rendertarget_R16_Sint,          0xb1c56727af9a35cb),
    TEST(rendertarget_R32_Float,         0x564d61cb1bab8347),
    TEST(rendertarget_R32_Uint,          0x564d61cb1bab8347),
    TEST(rendertarget_R32_Sint,          0x564d61cb1bab8347),
    TEST(rendertarget_RG8_Unorm,         0x6ade1edbebf83393),
    TEST(rendertarget_RG8_Snorm,         0x486522ba74fedf60),
    TEST(rendertarget_RG8_Uint,          0x6f079795eb063566),
    TEST(rendertarget_RG8_Sint,          0xca9113540fb0b566),
    TEST(rendertarget_RG16_Float,        0x30f92e5c2b309aa9),
    TEST(rendertarget_RG16_Unorm,        0x4420cce8f63b8afe),
    TEST(rendertarget_RG16_Snorm,        0xb63a5b92d0ab9d68),
    TEST(rendertarget_RG16_Uint,         0x390880fe5b0f2af2),
    TEST(rendertarget_RG16_Sint,         0xc839dbd13e3da371),
    TEST(rendertarget_RG32_Float,        0x8585df754b87fe15),
    TEST(rendertarget_RG32_Uint,         0x8585df754b87fe15),
    TEST(rendertarget_RG32_Sint,         0x8585df754b87fe15),
    TEST(rendertarget_RGBA8_Unorm,       0x12c91e487df61323),
    TEST(rendertarget_RGBA8_Snorm,       0x2782acd49ca7a0e5),
    TEST(rendertarget_RGBA8_Uint,        0x390880fe5b0f2af2),
    TEST(rendertarget_RGBA8_Sint,        0xe884ffdc9e1a3208),
    TEST(rendertarget_RGBA16_Float,      0xf345aa894c06a1af),
    TEST(rendertarget_RGBA16_Unorm,      0x2216ddb02a79dbb4),
    TEST(rendertarget_RGBA16_Snorm,      0x60a5d95e7117142c),
    TEST(rendertarget_RGBA16_Uint,       0xdd6a90bf7212daf3),
    TEST(rendertarget_RGBA16_Sint,       0x46aee7b6ceab87e2),
    TEST(rendertarget_RGBA32_Float,      0x255868473ad4e375),
    TEST(rendertarget_RGBA32_Uint,       0x255868473ad4e375),
    TEST(rendertarget_RGBA32_Sint,       0x255868473ad4e375),
    TEST(rendertarget_RGBA8_Unorm_sRGB,  0x1d57ef21e3f576c8),
    TEST(rendertarget_RGB10A2_Unorm,     0x45eda0ecf04bfa88),
    TEST(rendertarget_RGB10A2_Uint,      0x390880fe5b0f2af2),
    TEST(rendertarget_RG11B10_Float,     0x39438fa833bda8e8),
    TEST(rendertarget_BGR565_Unorm,      0xb343d24ed714334f),
    TEST(rendertarget_BGR5A1_Unorm,      0x9f97010b1deb3324),
    TEST(attrib_format_R32_Snorm,        0xac8b885a95f4b1f2),
    TEST(attrib_format_R32_Unorm,        0xdbebe7f43c0e07e3),
    TEST(attrib_format_R32_Sint,         0xfc847fe364b926a9),
    TEST(attrib_format_R32_Uint,         0xfc847fe364b926a9),
    TEST(attrib_format_R32_Sscaled,      0x742c1b0c2645dc09),
    TEST(attrib_format_R32_Uscaled,      0x9055b4bc438f7d1f),
    TEST(attrib_format_R32_Float,        0xcbe6f18cf2d0bd14),
    TEST(attrib_format_RG32_Snorm,       0x219af538f06e161c),
    TEST(attrib_format_RG32_Unorm,       0xa90a4c73c2cc7969),
    TEST(attrib_format_RG32_Sint,        0x76b2d092cf3f319a),
    TEST(attrib_format_RG32_Uint,        0x76b2d092cf3f319a),
    TEST(attrib_format_RG32_Sscaled,     0x3f50544b1eb249f9),
    TEST(attrib_format_RG32_Uscaled,     0x121311240e72adb8),
    TEST(attrib_format_RG32_Float,       0xe5c1e06e0cc570b7),
    TEST(attrib_format_RGB32_Snorm,      0x5296175effed908a),
    TEST(attrib_format_RGB32_Unorm,      0xe587c948fe9dcef4),
    TEST(attrib_format_RGB32_Sint,       0x34d5f13cca5a5497),
    TEST(attrib_format_RGB32_Uint,       0x34d5f13cca5a5497),
    TEST(attrib_format_RGB32_Sscaled,    0x7e834c51f36ef7fb),
    TEST(attrib_format_RGB32_Uscaled,    0x72c5acb40681a05d),
    TEST(attrib_format_RGB32_Float,      0xe5c1e06e0cc570b7),
    TEST(attrib_format_RGBA32_Snorm,     0x41c290bd29e9eac8),
    TEST(attrib_format_RGBA32_Unorm,     0x445adf2b031bed41),
    TEST(attrib_format_RGBA32_Sint,      0x4410e220a1e7ea4e),
    TEST(attrib_format_RGBA32_Uint,      0x4410e220a1e7ea4e),
    TEST(attrib_format_RGBA32_Sscaled,   0x3ef837cd3f1f900d),
    TEST(attrib_format_RGBA32_Uscaled,   0x7b8886a8e333dc3e),
    TEST(attrib_format_RGBA32_Float,     0x4410e220a1e7ea4e),
    TEST(attrib_format_R16_Snorm,        0x3ac5a6ccdacce005),
    TEST(attrib_format_R16_Unorm,        0xc4577629aa797514),
    TEST(attrib_format_R16_Sint,         0x5264aad6a17e68f3),
    TEST(attrib_format_R16_Uint,         0x8b899afc1decd881),
    TEST(attrib_format_R16_Sscaled,      0x015e1c5853d6c3b6),
    TEST(attrib_format_R16_Uscaled,      0x2333486cd4c8ecb6),
    TEST(attrib_format_R16_Float,        0x424a48d4f856e8a1),
    TEST(attrib_format_RG16_Snorm,       0xbb3aa12d21e9bd3d),
    TEST(attrib_format_RG16_Unorm,       0xf2feb165ce33e3c8),
    TEST(attrib_format_RG16_Sint,        0x763e023a45b14864),
    TEST(attrib_format_RG16_Uint,        0x8b899afc1decd881),
    TEST(attrib_format_RG16_Sscaled,     0xbc78dd2fd3cd220e),
    TEST(attrib_format_RG16_Uscaled,     0x55bec76cb85fa88f),
    TEST(attrib_format_RG16_Float,       0x9cd3e890053bed1b),
    TEST(attrib_format_RGB16_Snorm,      0x646c7dd20c2545bc),
    TEST(attrib_format_RGB16_Unorm,      0x9e165a015d7e90c7),
    TEST(attrib_format_RGB16_Sint,       0x1e6fe4644256d71f),
    TEST(attrib_format_RGB16_Uint,       0x8b899afc1decd881),
    TEST(attrib_format_RGB16_Sscaled,    0x43868cd944e5b519),
    TEST(attrib_format_RGB16_Uscaled,    0x7f581fa0bf8f563f),
    TEST(attrib_format_RGB16_Float,      0x9cd3e890053bed1b),
    TEST(attrib_format_RGBA16_Snorm,     0xcf4a90a507ca3619),
    TEST(attrib_format_RGBA16_Unorm,     0xb14d45ca68d65a7a),
    TEST(attrib_format_RGBA16_Sint,      0x1e6fe4644256d71f),
    TEST(attrib_format_RGBA16_Uint,      0x8b899afc1decd881),
    TEST(attrib_format_RGBA16_Sscaled,   0x4972c078392d2a20),
    TEST(attrib_format_RGBA16_Uscaled,   0x60497844e7b5f191),
    TEST(attrib_format_RGBA16_Float,     0x195e8f5423409a81),
    TEST(attrib_format_R8_Snorm,         0xcc82df7e466e0dcf),
    TEST(attrib_format_R8_Unorm,         0x3b8401b57edfb17f),
    TEST(attrib_format_R8_Sint,          0x5264aad6a17e68f3),
    TEST(attrib_format_R8_Uint,          0x8b899afc1decd881),
    TEST(attrib_format_R8_Sscaled,       0x3eaf549f164fe2a7),
    TEST(attrib_format_R8_Uscaled,       0x6416492daecf57ce),
    TEST(attrib_format_RG8_Snorm,        0xca361e9a4c508772),
    TEST(attrib_format_RG8_Unorm,        0xe022f7bf27dec2a5),
    TEST(attrib_format_RG8_Sint,         0x763e023a45b14864),
    TEST(attrib_format_RG8_Uint,         0x8b899afc1decd881),
    TEST(attrib_format_RG8_Sscaled,      0xc943d44907e49f24),
    TEST(attrib_format_RG8_Uscaled,      0x5928adc0783c9522),
    TEST(attrib_format_RGB8_Snorm,       0x8d1381891809213f),
    TEST(attrib_format_RGB8_Unorm,       0x67cc0c8e98de59d3),
    TEST(attrib_format_RGB8_Sint,        0x1e6fe4644256d71f),
    TEST(attrib_format_RGB8_Uint,        0x8b899afc1decd881),
    TEST(attrib_format_RGB8_Sscaled,     0x064a68c8bbab1c8c),
    TEST(attrib_format_RGB8_Uscaled,     0x47365f3d0e506aa8),
    TEST(attrib_format_RGBA8_Snorm,      0x9b228a4cbd0f65b5),
    TEST(attrib_format_RGBA8_Unorm,      0xe1ce3ea09eb8ffdf),
    TEST(attrib_format_RGBA8_Sint,       0x46aee7b6ceab87e2),
    TEST(attrib_format_RGBA8_Uint,       0x8b899afc1decd881),
    TEST(attrib_format_RGBA8_Sscaled,    0xee1c664afa5b479e),
    TEST(attrib_format_RGBA8_Uscaled,    0x3329262221696da0),
    TEST(attrib_format_RGB10A2_Snorm,    0xed223da517dd2513),
    TEST(attrib_format_RGB10A2_Unorm,    0xfbb226cb16893e37),
    TEST(attrib_format_RGB10A2_Sint,     0x46aee7b6ceab87e2),
    TEST(attrib_format_RGB10A2_Uint,     0x8b899afc1decd881),
    TEST(attrib_format_RGB10A2_Sscaled,  0x247dc83bb48b022a),
    TEST(attrib_format_RGB10A2_Uscaled,  0x6d63e1fc5105b389),
    TEST(attrib_format_RG11B10_Snorm,    0x8b7694c55a9b3be8),
    TEST(attrib_format_RG11B10_Unorm,    0x111c33327b90f3bf),
    TEST(attrib_format_RG11B10_Sint,     0x1e6fe4644256d71f),
    TEST(attrib_format_RG11B10_Uint,     0x8b899afc1decd881),
    TEST(attrib_format_RG11B10_Sscaled,  0x37471594d80f4768),
    TEST(attrib_format_RG11B10_Uscaled,  0x399b5aa102afb376),
    TEST(attrib_format_RG11B10_Float,    0x3d78a00d09498fe0),
    TEST(attrib_format_R32f_Snorm,       0xac8b885a95f4b1f2),
    TEST(attrib_format_R32f_Unorm,       0xdbebe7f43c0e07e3),
    TEST(attrib_format_R32f_Sint,        0xfc847fe364b926a9),
    TEST(attrib_format_R32f_Uint,        0xfc847fe364b926a9),
    TEST(attrib_format_R32f_Sscaled,     0x742c1b0c2645dc09),
    TEST(attrib_format_R32f_Uscaled,     0x9055b4bc438f7d1f),
    TEST(attrib_format_R32f_Float,       0xcbe6f18cf2d0bd14),
    TEST(attrib_format_RG32f_Snorm,      0x219af538f06e161c),
    TEST(attrib_format_RG32f_Unorm,      0xa90a4c73c2cc7969),
    TEST(attrib_format_RG32f_Sint,       0x76b2d092cf3f319a),
    TEST(attrib_format_RG32f_Uint,       0x76b2d092cf3f319a),
    TEST(attrib_format_RG32f_Sscaled,    0x3f50544b1eb249f9),
    TEST(attrib_format_RG32f_Uscaled,    0x121311240e72adb8),
    TEST(attrib_format_RG32f_Float,      0x5cfb4d80843fff54),
    TEST(attrib_format_RGB32f_Snorm,     0x1e7461b1834e0607),
    TEST(attrib_format_RGB32f_Unorm,     0xf49ae6590b10b112),
    TEST(attrib_format_RGB32f_Sint,      0x8a2bd81ff2ae0d14),
    TEST(attrib_format_RGB32f_Uint,      0x8a2bd81ff2ae0d14),
    TEST(attrib_format_RGB32f_Sscaled,   0x056325b78aa6153a),
    TEST(attrib_format_RGB32f_Uscaled,   0x72c5acb40681a05d),
    TEST(attrib_format_RGB32f_Float,     0x5cfb4d80843fff54),
    TEST(attrib_format_RGBA32f_Snorm,    0xbe3218c6f8cd7bda),
    TEST(attrib_format_RGBA32f_Unorm,    0x34474577c467f8b0),
    TEST(attrib_format_RGBA32f_Sint,     0x98c13ce4daf31d69),
    TEST(attrib_format_RGBA32f_Uint,     0x98c13ce4daf31d69),
    TEST(attrib_format_RGBA32f_Sscaled,  0x7fb55ab10c845cc3),
    TEST(attrib_format_RGBA32f_Uscaled,  0x7b8886a8e333dc3e),
    TEST(attrib_format_RGBA32f_Float,    0x98c13ce4daf31d69),
    TEST(attrib_format_R16f_Snorm,       0x3ac5a6ccdacce005),
    TEST(attrib_format_R16f_Unorm,       0xc4577629aa797514),
    TEST(attrib_format_R16f_Sint,        0x5264aad6a17e68f3),
    TEST(attrib_format_R16f_Uint,        0x8b899afc1decd881),
    TEST(attrib_format_R16f_Sscaled,     0x015e1c5853d6c3b6),
    TEST(attrib_format_R16f_Uscaled,     0x2333486cd4c8ecb6),
    TEST(attrib_format_R16f_Float,       0x424a48d4f856e8a1),
    TEST(attrib_format_RG16f_Snorm,      0xbb3aa12d21e9bd3d),
    TEST(attrib_format_RG16f_Unorm,      0xf2feb165ce33e3c8),
    TEST(attrib_format_RG16f_Sint,       0x763e023a45b14864),
    TEST(attrib_format_RG16f_Uint,       0x8b899afc1decd881),
    TEST(attrib_format_RG16f_Sscaled,    0xbc78dd2fd3cd220e),
    TEST(attrib_format_RG16f_Uscaled,    0x55bec76cb85fa88f),
    TEST(attrib_format_RG16f_Float,      0x9c57250f289e9add),
    TEST(attrib_format_RGB16f_Snorm,     0xc4922092f4755886),
    TEST(attrib_format_RGB16f_Unorm,     0x131538952fc56266),
    TEST(attrib_format_RGB16f_Sint,      0x1e6fe4644256d71f),
    TEST(attrib_format_RGB16f_Uint,      0x8b899afc1decd881),
    TEST(attrib_format_RGB16f_Sscaled,   0xb8cefa68bd9d9bed),
    TEST(attrib_format_RGB16f_Uscaled,   0x63cad87f9d4c5260),
    TEST(attrib_format_RGB16f_Float,     0x9c57250f289e9add),
    TEST(attrib_format_RGBA16f_Snorm,    0x47f9af6b7f171d71),
    TEST(attrib_format_RGBA16f_Unorm,    0xea19d104fd71426c),
    TEST(attrib_format_RGBA16f_Sint,     0x1e6fe4644256d71f),
    TEST(attrib_format_RGBA16f_Uint,     0x8b899afc1decd881),
    TEST(attrib_format_RGBA16f_Sscaled,  0xd59b5bea9b4e4b94),
    TEST(attrib_format_RGBA16f_Uscaled,  0x123f6e57c8e95918),
    TEST(attrib_format_RGBA16f_Float,    0x861a003f1df68083),
    TEST(attrib_format_R8f_Snorm,        0xcc82df7e466e0dcf),
    TEST(attrib_format_R8f_Unorm,        0x3b8401b57edfb17f),
    TEST(attrib_format_R8f_Sint,         0x5264aad6a17e68f3),
    TEST(attrib_format_R8f_Uint,         0x8b899afc1decd881),
    TEST(attrib_format_R8f_Sscaled,      0x3eaf549f164fe2a7),
    TEST(attrib_format_R8f_Uscaled,      0x6416492daecf57ce),
    TEST(attrib_format_RG8f_Snorm,       0xca361e9a4c508772),
    TEST(attrib_format_RG8f_Unorm,       0xe022f7bf27dec2a5),
    TEST(attrib_format_RG8f_Sint,        0x763e023a45b14864),
    TEST(attrib_format_RG8f_Uint,        0x8b899afc1decd881),
    TEST(attrib_format_RG8f_Sscaled,     0xc943d44907e49f24),
    TEST(attrib_format_RG8f_Uscaled,     0x5928adc0783c9522),
    TEST(attrib_format_RGB8f_Snorm,      0x6779e80abe6798b3),
    TEST(attrib_format_RGB8f_Unorm,      0x5988adfbf8f31d94),
    TEST(attrib_format_RGB8f_Sint,       0x1e6fe4644256d71f),
    TEST(attrib_format_RGB8f_Uint,       0x8b899afc1decd881),
    TEST(attrib_format_RGB8f_Sscaled,    0xda5c6873ab87fb4e),
    TEST(attrib_format_RGB8f_Uscaled,    0x3521807b9d191c58),
    TEST(attrib_format_RGBA8f_Snorm,     0x658a5a02dca3eeb6),
    TEST(attrib_format_RGBA8f_Unorm,     0x2aed11ba7161e208),
    TEST(attrib_format_RGBA8f_Sint,      0x46aee7b6ceab87e2),
    TEST(attrib_format_RGBA8f_Uint,      0x8b899afc1decd881),
    TEST(attrib_format_RGBA8f_Sscaled,   0xccf31688b64ac96c),
    TEST(attrib_format_RGBA8f_Uscaled,   0x517573b1ce9704ef),
    TEST(attrib_format_RGB10A2f_Snorm,   0x439ed88ee025d83d),
    TEST(attrib_format_RGB10A2f_Unorm,   0xd95e5fbd17f22226),
    TEST(attrib_format_RGB10A2f_Sint,    0x46aee7b6ceab87e2),
    TEST(attrib_format_RGB10A2f_Uint,    0x8b899afc1decd881),
    TEST(attrib_format_RGB10A2f_Sscaled, 0x12fcbdb2fb97a306),
    TEST(attrib_format_RGB10A2f_Uscaled, 0xe50eb91dab619c53),
    TEST(attrib_format_RG11B10f_Snorm,   0x3b7b94abcfd9bf57),
    TEST(attrib_format_RG11B10f_Unorm,   0x4cbe03fa93610b4e),
    TEST(attrib_format_RG11B10f_Sint,    0x1e6fe4644256d71f),
    TEST(attrib_format_RG11B10f_Uint,    0x8b899afc1decd881),
    TEST(attrib_format_RG11B10f_Sscaled, 0xd14293b7508f5e02),
    TEST(attrib_format_RG11B10f_Uscaled, 0xa27d87ca940e1e91),
    TEST(attrib_format_RG11B10f_Float,   0x97faca746b3e9d67),
    TEST(texformat_R8_Unorm,             0x38a872341106306b),
    TEST(texformat_R8_Snorm,             0x3a73d531f971ec27),
    TEST(texformat_R8_Uint,              0x8b899afc1decd881),
    TEST(texformat_R8_Sint,              0x9746371ef4be161f),
    TEST(texformat_R16_Float,            0xe2fab6c42db4ad17),
    TEST(texformat_R16_Unorm,            0x776b716e0f3c7ca0),
    TEST(texformat_R16_Snorm,            0xcf8fd7ddf68e75de),
    TEST(texformat_R16_Uint,             0x8b899afc1decd881),
    TEST(texformat_R16_Sint,             0x19646da2dff6a75c),
    TEST(texformat_R32_Float,            0xe5c9202223675223),
    TEST(texformat_R32_Uint,             0x85cbafe88b6511d0),
    TEST(texformat_R32_Sint,             0x85cbafe88b6511d0),
    TEST(texformat_RG8_Unorm,            0x7ddf23b30ec20893),
    TEST(texformat_RG8_Snorm,            0xfaba82176b188055),
    TEST(texformat_RG8_Uint,             0x8b899afc1decd881),
    TEST(texformat_RG8_Sint,             0xca44d28dca7d5903),
    TEST(texformat_RG16_Float,           0x0ec45258257f6103),
    TEST(texformat_RG16_Unorm,           0x46b15946c77a5890),
    TEST(texformat_RG16_Snorm,           0xe4e3764efe369736),
    TEST(texformat_RG16_Uint,            0x8b899afc1decd881),
    TEST(texformat_RG16_Sint,            0x22a802a4efa84029),
    TEST(texformat_RG32_Float,           0x76f95eac3e27aa02),
    TEST(texformat_RG32_Uint,            0xcf9e1d1fa98ea4aa),
    TEST(texformat_RG32_Sint,            0xcf9e1d1fa98ea4aa),
    TEST(texformat_RGB32_Float,          0x1d448d0238b8f839),
    TEST(texformat_RGB32_Uint,           0xc6b856228f7b3f76),
    TEST(texformat_RGB32_Sint,           0xc6b856228f7b3f76),
    TEST(texformat_RGBA8_Unorm,          0x0a85e8d275651cca),
    TEST(texformat_RGBA8_Snorm,          0x2e8814956307e14d),
    TEST(texformat_RGBA8_Uint,           0x8b899afc1decd881),
    TEST(texformat_RGBA8_Sint,           0x0205b9c09069ea73),
    TEST(texformat_RGBA16_Float,         0x6bacf54692efc41b),
    TEST(texformat_RGBA16_Unorm,         0xf694da903a75b390),
    TEST(texformat_RGBA16_Snorm,         0xd2999b920822d231),
    TEST(texformat_RGBA16_Uint,          0x8b899afc1decd881),
    TEST(texformat_RGBA16_Sint,          0x4716e88916780e48),
    TEST(texformat_RGBA32_Float,         0x9416d278e177c3c4),
    TEST(texformat_RGBA32_Uint,          0x9416d278e177c3c4),
    TEST(texformat_RGBA32_Sint,          0x9416d278e177c3c4),
    TEST(texformat_RGBX8_Unorm_sRGB,     0xcb2b2f3970d3a95f),
    TEST(texformat_RGBA8_Unorm_sRGB,     0xef52feffe2b50891),
    TEST(texformat_RGBA4_Unorm,          0x6ca5df2ea6ff0f69),
    TEST(texformat_RGB5_Unorm,           0x88233040036d752c),
    TEST(texformat_RGB5A1_Unorm,         0x84f47991ae6956f7),
    TEST(texformat_RGB565_Unorm,         0x3fedc6c8b3354418),
    TEST(texformat_RGB10A2_Unorm,        0xd4e22542691de3b3),
    TEST(texformat_RGB10A2_Uint,         0x8b899afc1decd881),
    TEST(texformat_RG11B10_Float,        0x3fde9157463dccf5),
    TEST(texformat_E5BGR9_Float,         0xea5d1ba2c6c3ad74),
    TEST(texformat_RGB_BC1,              0xf92a23cb4afd91d6),
    TEST(texformat_RGBA_BC1,             0x6b5db93056db0fb5),
    TEST(texformat_RGBA_BC2,             0x5a16100528cb1dc9),
    TEST(texformat_RGBA_BC3,             0x29e4193bc9bade9f),
    TEST(texformat_RGB_BC1_sRGB,         0xc4815efdbb335db3),
    TEST(texformat_RGBA_BC1_sRGB,        0xe3875cc8cd0bbe21),
    TEST(texformat_RGBA_BC2_sRGB,        0x093dc9fbb215fcec),
    TEST(texformat_RGBA_BC3_sRGB,        0xc1dfb35871d8304a),
    TEST(texformat_R_BC4_Unorm,          0x53fb04a9c2d40011),
    TEST(texformat_R_BC4_Snorm,          0x92a9044fc6394c75),
    TEST(texformat_RG_BC5_Unorm,         0xf1ed8d0b6f9e73d9),
    TEST(texformat_RG_BC5_Snorm,         0x8d94636f1b2f140e),
    TEST(texformat_RGBA_BC7_Unorm,       0xa064cc418c36aace),
    TEST(texformat_RGBA_BC7_Unorm_sRGB,  0xdec62ff805037d88),
    TEST(texformat_RGBA_BC6H_SF16_Float, 0x28d4f459a4d58ab2),
    TEST(texformat_RGBA_BC6H_UF16_Float, 0x7e8f4d590746a1ff),
    TEST(texformat_RGBX8_Unorm,          0x4b771c9a9104e034),
    TEST(texformat_RGBX8_Snorm,          0x4f0324b6323b0cd3),
    TEST(texformat_RGBX8_Uint,           0x8b899afc1decd881),
    TEST(texformat_RGBX8_Sint,           0x8b899afc1decd881),
    TEST(texformat_RGBX16_Float,         0x32c689f0da2fe21c),
    TEST(texformat_RGBX16_Unorm,         0xefb8251f1b3165c4),
    TEST(texformat_RGBX16_Snorm,         0x655bedc61718f71b),
    TEST(texformat_RGBX16_Uint,          0x8b899afc1decd881),
    TEST(texformat_RGBX16_Sint,          0x22a802a4efa84029),
    TEST(texformat_RGBX32_Float,         0xdc486b29d7dfebb1),
    TEST(texformat_RGBX32_Uint,          0xffde34b4351cd60a),
    TEST(texformat_RGBX32_Sint,          0xffde34b4351cd60a),
    TEST(texformat_BGR565_Unorm,         0x2e276972f60eb4f9),
    TEST(texformat_BGR5_Unorm,           0x22e95dddbb492b7c),
    TEST(texformat_BGR5A1_Unorm,         0x65d77cdb6b548233),
    TEST(texformat_A5BGR5_Unorm,         0x923cbdb0498d86e0),
    TEST(texformat_BGRX8_Unorm,          0xbd5a0a9bd2444ad0),
    TEST(texformat_BGRA8_Unorm,          0xdea22ad0c163bf93),
    TEST(texformat_BGRX8_Unorm_sRGB,     0x37d86ae217d86784),
    TEST(texformat_BGRA8_Unorm_sRGB,     0x97051709a437b05a),
    TEST(texformat_R_ETC2_Unorm,         0x31409308094e1301),
    TEST(texformat_R_ETC2_Snorm,         0xc599fe9696e1c7ea),
    TEST(texformat_RG_ETC2_Unorm,        0xec623fd0ebdb3f78),
    TEST(texformat_RG_ETC2_Snorm,        0x4ba027a98cd88e74),
    TEST(texformat_RGB_ETC2,             0xf4a83650da0f4b20),
    TEST(texformat_RGB_PTA_ETC2,         0x98f385018ed2871d),
    TEST(texformat_RGBA_ETC2,            0x8ef97f0af0288021),
    TEST(texformat_RGB_ETC2_sRGB,        0x08817ad848f0eace),
    TEST(texformat_RGB_PTA_ETC2_sRGB,    0x40cd807f8ba4e0c2),
    TEST(texformat_RGBA_ETC2_sRGB,       0x2afa3a8f61258107),
};
#define NUM_TESTS (sizeof(test_descriptors) / sizeof(test_descriptors[0]))

void run_graphics_tests(DkDevice device, DkQueue queue, bool automatic_mode)
{
    struct gfx_context ctx = {0};
    ctx.device = device;
    ctx.queue = queue;

    printf("Running graphics tests...\n\n");

    size_t failures = 0;
    for (size_t i = 0; i < NUM_TESTS; ++i)
    {
        struct gfx_test_descriptor* const test = &test_descriptors[i];
        int written_chars =
            printf("%3zd/%3zd Test: %s", i + 1, NUM_TESTS, test->name);
        for (int i = 0; i < 45 - written_chars; ++i)
            putc('.', stdout);
        putc(' ', stdout);

        consoleUpdate(NULL);

        u64 const hash = hash_memblock(test->func(&ctx));
        reset_context(&ctx);

        if (test->expected == hash)
        {
            puts("Passed");
        }
        else
        {
            printf("Failed (0x%016"PRIx64")\n", hash);
            ++failures;
        }

        consoleUpdate(NULL);

        if (!automatic_mode && i != 0 && i % 43 == 0)
        {
            printf("Press A to continue...");
            wait_for_input();
        }
    }

    printf("\n%3d%% tests passed, %zd tests failed out of %zd\n\n",
        (int)((NUM_TESTS - failures) * 100 / (float)NUM_TESTS), failures,
        NUM_TESTS);
}

#include <stdbool.h>
#include <stdio.h>
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

#define BASIC_INIT(format, is_color)                                   \
    DkImage render_target;                                             \
    DkMemBlock render_target_memblock;                                 \
    make_linear_render_target(                                         \
        ctx, format, 64, 64, &render_target, &render_target_memblock); \
    DkImageView render_target_view = make_image_view(&render_target);  \
    DkCmdBuf const cmdbuf = make_cmdbuf(ctx, 1024);                    \
    DkImageView const* const color_rt_view[] = {&render_target_view};  \
    DkImageView* zeta_rt_view = is_color ? NULL : &render_target_view; \
    dkCmdBufBindRenderTargets(                                         \
        cmdbuf, color_rt_view, is_color ? 1 : 0, zeta_rt_view);

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

DEFINE_TEST(clear)
{
    BASIC_INIT(DkImageFormat_RGBA8_Unorm, true)

    static float const clear_color[4] = {1.0f, 0.8f, 0.25f, 0.125f};
    dkCmdBufClearColor(cmdbuf, 0, DkColorMask_RGBA, clear_color);

    BASIC_END
}

DEFINE_TEST(clear_scissor)
{
    BASIC_INIT(DkImageFormat_RGBA32_Float, true)

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
    BASIC_INIT(DkImageFormat_RGBA32_Float, true)

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
    make_linear_render_target(
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
    BASIC_INIT(DkImageFormat_RGBA32_Float, true)

    BIND_SHADER(Vertex, "full_screen_tri.vert")
    BIND_SHADER(Fragment, "red.frag")

    dkCmdBufDraw(cmdbuf, DkPrimitive_Triangles, 3, 1, 0, 0);

    BASIC_END
}

static struct gfx_test_descriptor test_descriptors[] =
{
    TEST(clear,                0xbe7e7dc089ef7f01),
    TEST(clear_scissor,        0x27750a82ffacde28),
    TEST(clear_scissor_masked, 0x69292f52fbda15c1),
    TEST(clear_depth,          0x139f492006278563),
    TEST(basic_draw,           0x1137a01933ff0447),
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
        for (int i = 0; i < 43 - written_chars; ++i)
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

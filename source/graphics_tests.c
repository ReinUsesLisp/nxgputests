#include <stdbool.h>
#include <stdio.h>

#include <deko3d.h>

#include "graphics_tests.h"
#include "graphics_context.h"
#include "helper.h"
#include "hash.h"

#define TEST(name) { name_##name, name }

#define DEFINE_TEST(name) \
	static char name_##name[] = #name; \
	static bool name(struct gfx_context* ctx)

struct gfx_test_descriptor
{
	char* name;
	bool (*func)(struct gfx_context*);
};

DEFINE_TEST(clear_test)
{
	static float const clear_color[4] = {1.0f, 0.8f, 0.25f, 0.125f};

	DkImage render_target;
	DkMemBlock render_target_memblock;
	make_linear_render_target(
		ctx, DkImageFormat_RGBA8_Unorm, 64, 64, &render_target,
		&render_target_memblock);
	
	DkImageView render_target_view = make_image_view(&render_target);

	DkCmdBuf cmdbuf = make_cmdbuf(ctx, 1024);
	dkCmdBufBindRenderTarget(cmdbuf, &render_target_view, NULL);
	dkCmdBufClearColor(cmdbuf, 0, DkColorMask_RGBA, clear_color);
	dkQueueSubmitCommands(ctx->queue, dkCmdBufFinishList(cmdbuf));
	dkQueueWaitIdle(ctx->queue);

	return compare_hash(render_target_memblock, 0x69fe987f05982ddc, 0xfb40d3da43148e2e, 0x5f55d3218691f6f9, 0x7395e54449f22a0a);
}

static struct gfx_test_descriptor test_descriptors[] =
{
	TEST(clear_test),
};
#define NUM_TESTS (sizeof(test_descriptors) / sizeof(test_descriptors[0]))

void run_graphics_tests(
    DkDevice device, DkQueue queue, FILE* report_file, bool automatic_mode)
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

        bool const pass = test->func(&ctx);
        reset_context(&ctx);

        if (!pass)
        	++failures;

        puts(pass ? "Passed" : "Failed");

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

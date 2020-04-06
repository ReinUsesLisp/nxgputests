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

struct test_image
{
    struct descriptor_set set;
    struct image* image;
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
    uint32_t depth)
{
    DkImageLayoutMaker layout_maker = {
        .device = device,
        .type = type,
        .flags = DkImageFlags_UsageLoadStore,
        .format = format,
        .dimensions = {width, height, depth},
        .mipLevels = 1,
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
        .swizzle = {DkImageSwizzle_Red, DkImageSwizzle_Green,
                    DkImageSwizzle_Blue, DkImageSwizzle_Alpha},
    };
    dkImageDescriptorInitialize(descriptor, &image_view, true, false);

    return obj;
}

static void destroy_image(struct image* obj)
{
    dkMemBlockDestroy(obj->memblock);
    free(obj);
}

static void destroy_test_image(struct test_image test_image)
{
    destroy_descriptor_set(&test_image.set);
    destroy_image(test_image.image);
}

static struct test_image image_test(DkDevice device, DkQueue queue,
    DkCmdBuf cmdbuf, uint32_t type, uint32_t format, uint32_t width,
    uint32_t height, uint32_t depth,
    void (*image_writer)(struct image*, void const*), void const *userdata)
{
    struct descriptor_set set = make_image_descriptor_set(device, 1);
    struct image *image = make_image(device, &set.descriptors[0], type,
        format, width, height, depth);

    if (image_writer)
        image_writer(image, userdata);

    dkCmdBufBindImageDescriptorSet(cmdbuf, set.gpu_addr, 1);
    dkCmdBufBindImage(cmdbuf, DkStage_Compute, 0, dkMakeImageHandle(0));
    dkCmdBufDispatchCompute(cmdbuf, 1, 1, 1);

    dkQueueSubmitCommands(queue, dkCmdBufFinishList(cmdbuf));
    dkQueueWaitIdle(queue);

    struct test_image test_image = { set, image };
    return test_image;
}

static void simple_image_test(DkDevice device, DkQueue queue,
    DkCmdBuf cmdbuf, uint32_t type, uint32_t format, uint32_t width,
    uint32_t height, uint32_t depth,
    void (*image_writer)(struct image*, void const*), void const *userdata)
{
    destroy_test_image(image_test(device, queue, cmdbuf, type, format, width,
        height, depth, image_writer, userdata));
}

static void write32(struct image* image, void const* userdata)
{
    memcpy(image->memory, userdata, sizeof(uint32_t));
}

static void write64(struct image* image, void const* userdata)
{
    memcpy(image->memory, userdata, sizeof(uint64_t));
}

DEFINE_ETEST(sust_p_rgba)
{
    struct test_image image = image_test(device, queue, cmdbuf, DkImageType_2D,
        DkImageFormat_R32_Float, 1, 1, 1, NULL, NULL);
    memcpy(results, image.image->memory, sizeof(uint32_t));
    destroy_test_image(image);
}

DEFINE_ETEST(suld_p_rgba)
{
    float const data = 36.0f;
    simple_image_test(device, queue, cmdbuf, DkImageType_2D,
        DkImageFormat_R32_Float, 1, 1, 1, write32, &data);
}

DEFINE_ETEST(suld_d_32_r32f)
{
    float const data = 75.0f;
    simple_image_test(device, queue, cmdbuf, DkImageType_2D,
        DkImageFormat_R32_Float, 1, 1, 1, write32, &data);
}

DEFINE_ETEST(suld_d_32_rgba8u)
{
    uint32_t const data = 0x20406080;
    simple_image_test(device, queue, cmdbuf, DkImageType_2D,
        DkImageFormat_RGBA8_Unorm, 1, 1, 1, write32, &data);
}

DEFINE_ETEST(suld_d_32_bgra8u)
{
    uint32_t const data = 0x21416181;
    simple_image_test(device, queue, cmdbuf, DkImageType_2D,
        DkImageFormat_BGRA8_Unorm, 1, 1, 1, write32, &data);
}

DEFINE_ETEST(suld_d_32_rgba8s)
{
    uint32_t const data = 0x65fe12ff;
    simple_image_test(device, queue, cmdbuf, DkImageType_2D,
        DkImageFormat_RGBA8_Snorm, 1, 1, 1, write32, &data);
}

DEFINE_ETEST(suld_d_32_rgba8ui)
{
    uint32_t const data = 0xdeadbeec;
    simple_image_test(device, queue, cmdbuf, DkImageType_2D,
        DkImageFormat_RGBA8_Uint, 1, 1, 1, write32, &data);
}

DEFINE_ETEST(suld_d_32_rgba8i)
{
    uint32_t const data = 0x11a220ff;
    simple_image_test(device, queue, cmdbuf, DkImageType_2D,
        DkImageFormat_RGBA8_Sint, 1, 1, 1, write32, &data);
}

DEFINE_ETEST(suld_d_64_rg32f)
{
    uint64_t const data = 0x013275ab32452ffcc;
    simple_image_test(device, queue, cmdbuf, DkImageType_2D,
        DkImageFormat_RG32_Float, 1, 1, 1, write64, &data);
}

DEFINE_ETEST(suld_d_64_rgba16f)
{
    uint64_t const data = 0x1111222233334444;
    simple_image_test(device, queue, cmdbuf, DkImageType_2D,
        DkImageFormat_RGBA16_Float, 1, 1, 1, write64, &data);
}

DEFINE_ETEST(suld_d_64_rgba16s)
{
    uint64_t const data = 0x00ff1365a020b0c3;
    simple_image_test(device, queue, cmdbuf, DkImageType_2D,
        DkImageFormat_RGBA16_Snorm, 1, 1, 1, write64, &data);
}

DEFINE_ETEST(suld_d_64_rgba16u)
{
    uint64_t const data = 0xa8943bc24389a234;
    simple_image_test(device, queue, cmdbuf, DkImageType_2D,
        DkImageFormat_RGBA16_Unorm, 1, 1, 1, write64, &data);
}

DEFINE_ETEST(suld_d_64_rgba16i)
{
    uint64_t const data = 0xc235abe456630a13;
    simple_image_test(device, queue, cmdbuf, DkImageType_2D,
        DkImageFormat_RGBA16_Sint, 1, 1, 1, write64, &data);
}

DEFINE_ETEST(suld_d_64_rgba16ui)
{
    uint64_t const data = 0x157c4deab9432573;
    simple_image_test(device, queue, cmdbuf, DkImageType_2D,
        DkImageFormat_RGBA16_Uint, 1, 1, 1, write64, &data);
}

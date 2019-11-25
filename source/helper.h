#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <deko3d.h>

#define DEFINE_ETEST(id)    \
	void execute_test_##id( \
		DkDevice device, DkQueue queue, DkCmdBuf cmdbuf, uint32_t* results)

#define DEFINE_MTEST(id) \
	bool test_##id(char const* name, uint32_t* results, FILE* report_file)

#define DECLARE_ETEST(id) DEFINE_ETEST(id);
#define DECLARE_MTEST(id) DEFINE_MTEST(id);

DkMemBlock make_memory_block(DkDevice device, size_t size, uint32_t flags);
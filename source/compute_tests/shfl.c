#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "helper.h"
#include "unit_test_report.h"

static bool compare_array(char const* name, void* results, FILE* report_file,
	size_t num_entries, uint32_t const* expected)
{
	bool pass = !memcmp(results, expected, num_entries * sizeof(uint32_t));
	unit_test_report(report_file, name, pass, num_entries, expected, results);
	return pass;
}

DEFINE_MTEST(shfl_idx)
{
	static uint32_t const expected[] = {2, 2, 2, 2, 2, 2, 2, 2};
	return compare_array(name, results, report_file, 8, expected);
}

DEFINE_MTEST(shfl_up)
{
	static uint32_t const expected[] = {0xdead, 0xdead, 0xdead, 0, 1, 2, 3, 4};
	return compare_array(name, results, report_file, 8, expected);
}

DEFINE_MTEST(shfl_down)
{
	static uint32_t const expected[] = {2, 3, 4, 5, 6, 7, 0xdead, 0xdead};
	return compare_array(name, results, report_file, 8, expected);
}

DEFINE_MTEST(shfl_bfly)
{
	static uint32_t const expected[] = {3, 2, 1, 0, 7, 6, 5, 4};
	return compare_array(name, results, report_file, 8, expected);
}

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "helper.h"
#include "unit_test_report.h"

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

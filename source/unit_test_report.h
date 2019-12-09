#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

FILE* begin_unit_test_report();

void unit_test_report(FILE* report_file, char const* test_name, bool pass,
	size_t num_entries, uint32_t const* expected, uint32_t const* results);

void finish_unit_test_report(FILE* report_file);

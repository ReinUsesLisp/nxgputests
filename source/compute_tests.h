#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <deko3d.h>

u64 run_compute_tests(
    DkDevice device, DkQueue queue, FILE* report_file, bool automatic_mode);

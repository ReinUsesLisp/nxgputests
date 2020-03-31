#pragma once

#include <stdbool.h>

#include <switch.h>
#include <deko3d.h>

void run_compute_tests(
    DkDevice device, DkQueue queue, FILE* report_file, bool automatic_mode);

#pragma once

#include <stdio.h>

#include "position.h"

/*
 * Perft testing suites.
 */

void nc_perft_run(FILE* out, nc_position* p, int maxdepth);

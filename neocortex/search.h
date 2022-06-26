#pragma once

#include "position.h"
#include "types.h"

#define NC_SEARCH_EVAL_THRESHOLD 1000
#define NC_SEARCH_EVAL_WEIGHT    0.85

ncMove ncSearch(ncPosition* root, int nodes, float exploration);

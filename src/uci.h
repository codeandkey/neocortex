#pragma once

#include <stdio.h>

/*
 * UCI control
 */

#define NC_UCI_BUFLEN 2048
#define NC_UCI_NAME   "neocortex-nc3 0.1"
#define NC_UCI_AUTHOR "codeandkey"

#define NC_UCI_MAXDEPTH 10
#define NC_UCI_MAX_MOVETIME 30000

/* Interesting time control: accept an early iteration if it scores better than (remaining_ms / score_fraction) */
#define NC_UCI_ACCEPTABLE_SCORE_FRACTION 10

#define NC_UCI_MOVETIME_DIV 12
#define NC_UCI_TIME_FACTOR 40 /* don't start the next iteration if we've exceed N percent of allocated time */

int nc_uci_start(FILE* in, FILE* out);

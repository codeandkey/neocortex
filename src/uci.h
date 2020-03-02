#pragma once

#include <stdio.h>

/*
 * UCI control
 */

#define NC_UCI_BUFLEN 2048
#define NC_UCI_NAME   "neocortex-nc3 0.1"
#define NC_UCI_AUTHOR "codeandkey"

#define NC_UCI_MAXDEPTH 10
#define NC_UCI_MAX_MOVETIME 25000
#define NC_UCI_TIME_FACTOR 8

int nc_uci_start(FILE* in, FILE* out);

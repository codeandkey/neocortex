#pragma once

#include <stdio.h>

/*
 * UCI control
 */

#define NC_UCI_BUFLEN 2048
#define NC_UCI_NAME   "neocortex-nc3 0.1"
#define NC_UCI_AUTHOR "codeandkey"

int nc_uci_start(FILE* in, FILE* out);

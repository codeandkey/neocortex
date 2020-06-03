#pragma once

/*
 * Search control.
 *
 * Controls the lifetime and manages search threads.
 */

#include "position.h"

/* the search will abort after consuming 1/nth of the remaining time */
#define NC_SEARCHCTL_MAXTIME_DIVISOR 5

/* the search will not execute the next depth if it is expected to take more than 1/nth of the remaining time */
#define NC_SEARCHCTL_EARLYSTOP_DIVISOR 2

typedef struct {
	int wtime, btime, winc, binc;
	int movestogo, depth, nodes;
	int mate, movetime, infinite;
} nc_searchopts;

void nc_searchctl_init();
void nc_searchctl_position(nc_position p);
void nc_searchctl_go();
void nc_searchctl_stop();

#pragma once

/*
 * Search control.
 *
 * Controls the lifetime and manages search threads.
 */

#include "position.h"

typedef struct {
	int wtime, btime, winc, binc;
	int movestogo, depth, nodes;
	int mate, movetime, infinite;
} nc_searchopts;

void nc_searchctl_init();
void nc_searchctl_position(nc_position p);
void nc_searchctl_go();
void nc_searchctl_stop();

#include "magic.h"
#include "util.h"
#include "zobrist.h"
#include "position.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

int main(int argc, char** argv) {
    srand(time(NULL));

    nc_magic_init();
    nc_zobrist_init(0xdeadbeef);

    nc_position p;
    nc_position_init(&p);

    nc_position_dump(&p, stderr);

    nc_position_make_move(&p, nc_move_make(NC_SQ_A2, NC_SQ_A4) | NC_PAWNJUMP);
    nc_position_dump(&p, stderr);

    nc_position_make_move(&p, nc_move_make(NC_SQ_A7, NC_SQ_A5) | NC_PAWNJUMP);
    nc_position_dump(&p, stderr);

    nc_position_unmake_move(&p, nc_move_make(NC_SQ_A7, NC_SQ_A5) | NC_PAWNJUMP);
    nc_position_dump(&p, stderr);

    nc_position_unmake_move(&p, nc_move_make(NC_SQ_A2, NC_SQ_A4) | NC_PAWNJUMP);
    nc_position_dump(&p, stderr);

    nc_magic_free();

    return 0;
}

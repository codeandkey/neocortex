#include "magic.h"
#include "util.h"
#include "zobrist.h"
#include "position.h"
#include "basic.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

int main(int argc, char** argv) {
    srand(time(NULL));

    nc_magic_init();
    nc_basic_init();
    nc_zobrist_init(0xdeadbeef);

    nc_position p;
    nc_position_init(&p);

    nc_position_dump(&p, stderr);

    nc_movelist lst;
    nc_movelist_clear(&lst);
    nc_position_legal_moves(&p, &lst);

    for (int i = 0; i < lst.len; ++i) {
        nc_debug("legal move: %s", nc_move_tostr(lst.moves[i]));
    }

    nc_magic_free();

    return 0;
}

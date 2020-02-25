#include "magic.h"
#include "util.h"
#include "zobrist.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

int main(int argc, char** argv) {
    srand(time(NULL));

    nc_magic_init();
    nc_zobrist_init(0xdeadbeef);

    nc_magic_free();

    return 0;
}

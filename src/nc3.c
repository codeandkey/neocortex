#include "magic.h"
#include "util.h"
#include "zobrist.h"
#include "position.h"
#include "basic.h"
#include "perft.h"
#include "uci.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

int main(int argc, char** argv) {
	srand(time(NULL));

	nc_pst_init();
	nc_magic_init();
	nc_basic_init();
	nc_zobrist_init(0x82fd8a72);

	nc_uci_start(stdin, stdout);

	nc_magic_free();

	return 0;
}

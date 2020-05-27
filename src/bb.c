#include "bb.h"

static char _nc_bb_strbuf[73];

const char* nc_bb_tostr(nc_bb bb) {
	int i = 0;
	for (int r = 7; r >= 0; --r) {
		for (int f = 0; f < 8; ++f) {
			_nc_bb_strbuf[i++] = ((bb >> nc_square_at(r, f)) & 1) ? '#' : '.';
		}

		_nc_bb_strbuf[i++] = '\n';
	}

	return (const char*) _nc_bb_strbuf;
}

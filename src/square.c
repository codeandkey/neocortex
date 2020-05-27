#include "square.h"

static char _nc_square_strbuf[3];

const char* nc_square_tostr(nc_square in) {
	if (!nc_square_is_valid(in)) return "-";
	_nc_square_strbuf[0] = nc_square_file(in) + 'a';
	_nc_square_strbuf[1] = nc_square_rank(in) + '1';
	return (const char*) _nc_square_strbuf;
}

nc_square nc_square_fromstr(const char* in) {
	nc_assert(in);
	if (in[0] == '-') return NC_SQ_NULL;
	return nc_square_at(in[1] - '1', in[0] - 'a');
}

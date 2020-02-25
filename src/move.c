#include "move.h"
#include "square.h"

#include <string.h>

static char _nc_move_strbuf[6];

const char* nc_move_tostr(nc_move_t in) {
    if (in == NC_MOVE_NULL) return "0000";

    const char* fromstr = nc_square_tostr(nc_move_get_src(in));

    _nc_move_strbuf[0] = fromstr[0];
    _nc_move_strbuf[1] = fromstr[1];

    const char* tostr = nc_square_tostr(nc_move_get_dst(in));

    _nc_move_strbuf[2] = tostr[0];
    _nc_move_strbuf[3] = tostr[1];

    if (in & NC_PROMOTION) {
        _nc_move_strbuf[4] = nc_ptype_tochar(nc_move_get_ptype(in));
    } else {
        _nc_move_strbuf[4] = '\0';
    }

    return _nc_move_strbuf;
}

nc_move_t nc_move_fromstr(const char* in) {
    int len = strlen(in);
    if (len < 4 || len > 6) return NC_MOVE_NULL;

    nc_square_t src = nc_square_fromstr(in);
    nc_square_t dst = nc_square_fromstr(in + 2);

    if (src == NC_SQ_NULL || dst == NC_SQ_NULL) return NC_MOVE_NULL;

    nc_move_t out = nc_move_make(src, dst);

    if (len == 5) {
        out = nc_move_promotion(out, nc_ptype_fromchar(in[4]));
    }

    return out;
}

nc_move_t nc_movelist_match(nc_movelist* lst, nc_move_t in) {
    for (int i = 0; i < lst->len; ++i) {
        nc_move_t cur = lst->moves[i];

        if ((cur & 0xFFF) == (in & 0xFFF) && (cur & NC_PROMOTION) == (in & NC_PROMOTION)) {
            if (nc_move_get_ptype(cur) == nc_move_get_ptype(in)) {
                return cur;
            }
        }
    }

    return NC_MOVE_NULL;
}

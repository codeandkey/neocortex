#include "tt.h"

static nc_ttentry _nc_ttable_data[NC_TT_WIDTH];

nc_ttentry* nc_tt_lookup(nc_zkey key) {
    return _nc_ttable_data + (key % NC_TT_WIDTH);
}

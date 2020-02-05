#pragma once

/*
 * Transposition table indexing.
 */

#include "types.h"

namespace nc2 {
    namespace ttable {
        void initialize_indices(u32 seed);

        u32 get_piece_key(u8 s, u8 p);
        u32 get_castle_key(u8 color, u8 side);
        u32 get_black_to_move_key();
        u32 get_en_passant_file_key(u8 file);
    }
}

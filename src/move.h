#pragma once

/*
 * Basic move structure.
 * Only stores to, from squares and promotion type.
 */

#include <string>

#include "piece.h"
#include "types.h"

namespace nc2 {
    class Move {
        public:
            Move();
            Move(u8 from, u8 to, u8 ptype = piece::Type::NONE);

            u8 get_from();
            u8 get_to();
            u8 get_ptype();

            std::string to_string();

        private:
            u8 from, to, ptype;
    };
}

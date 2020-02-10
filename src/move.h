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
            /**
             * Initializes a null move.
             */
            Move();

            /**
             * Initializes a uci move.
             *
             * @param from From square.
             * @param to To square.
             * @param ptype Promotion type.
             */
            Move(u8 from, u8 to, u8 ptype = piece::Type::NONE);

            /**
             * Gets the source square.
             *
             * @return From square.
             */
            u8 get_from();

            /**
             * Gets the destination square.
             *
             * @return To square.
             */
            u8 get_to();

            /**
             * Gets the promotion type.
             *
             * @return Promotion piece type.
             */
            u8 get_ptype();

            /**
             * Gets the move as a printable string (UCI notation)
             *
             * @return Printable move string.
             */
            std::string to_string();

        private:
            u8 from, to, ptype;
    };
}

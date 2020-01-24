#pragma once

#include <string>

#include "square.h"

namespace nc {
    class Move {
    public:
        Move(Square from, Square to, char promote_type = '\0');
        Move(std::string uci);
        Move();

        std::string to_string();
        operator std::string();

        Square get_from();
        Square get_to();
        char get_promote_type();

    private:
        Square from, to;
        char promote_type;
    };
}

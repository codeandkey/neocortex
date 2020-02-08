#pragma once

#include <string>
#include <list>

#include "position.h"
#include "eval_type.h"
#include "move.h"

namespace nc2 {
    namespace search {
        class Result {
            public:
                Result(Position pos = Position(), Evaluation score = Evaluation(), std::list<Move> pv = std::list<Move>());
                Move get_bestmove();
                Evaluation get_score();
                float get_current();
                Position get_position();
                std::list<Move> get_pv();
                std::string get_pv_string();
                int get_depth();
                bool check_position(Position* pos);

                void insert_move(Position before_pos, Move m, int depth_inc = 1);

                void update(search::Result& from);

            private:
                Position pos;
                Evaluation score;
                std::list<Move> pv;
                int depth;
        };
    }
}

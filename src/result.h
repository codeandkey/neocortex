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
                /**
                 * Construct a new Result.
                 *
                 * @param pos Result source position
                 * @param score Search score
                 * @param pv Principal variation line
                 */
                Result(Position pos = Position(), Evaluation score = Evaluation(), std::list<Move> pv = std::list<Move>());

                /**
                 * Gets the best move in this line.
                 *
                 * @return Best move, if any.
                 */
                Move get_bestmove();

                /**
                 * Gets the score for this line.
                 *
                 * @return Line evaluation.
                 */
                Evaluation get_score();

                /**
                 * Gets the immediate heuristic for the source position.
                 *
                 * @return Evaluation heuristic.
                 */
                float get_current();

                /**
                 * Gets the source position.
                 *
                 * @return Position pointer
                 */
                Position* get_position();

                /**
                 * Gets the PV line.
                 *
                 * @return PV moves.
                 */
                std::list<Move> get_pv();

                /**
                 * Gets the PV in a printable format.
                 *
                 * @return PV string
                 */
                std::string get_pv_string();

                /**
                 * Gets the depth of this result.
                 *
                 * @return Result depth.
                 */
                int get_depth();

                /**
                 * Tests if the line source position matches some other position.
                 *
                 * @param pos Position to test.
                 * @return true if pos is equal to the source position, false otherwise
                 */
                bool check_position(Position* pos);

                /**
                 * Inserts a new move into the line at the beginning and updates the score.
                 *
                 * @param before_pos Position which `m` is made in.
                 * @param m Move that results in this Result.
                 * @param depth_inc Depth increment.
                 */
                void insert_move(Position before_pos, Move m, int depth_inc = 1);

                /**
                 * Updates the result from another. Called when a new result is being stored in our place.
                 *
                 * @param from New result information.
                 */
                void update(search::Result& from);

            private:
                Position pos;
                Evaluation score;
                std::list<Move> pv;
                int depth;
        };
    }
}

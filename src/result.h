#pragma once

#include <string>
#include <list>

#include "eval_type.h"
#include "move.h"

namespace nc2 {
    class Result {
        public:
            /**
             * Construct a new Result.
             *
             * @param key Transposition table key for the current position.
             * @param score Search score
             * @param pv Principal variation line
             */
            Result(u64 key = 0, Evaluation score = Evaluation(), std::list<Move> pv = std::list<Move>());

            /**
             * Gets the transposition key for the result.
             *
             * @return Transposition key.
             */
            u64 get_key();

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
             * Inserts a new move into the line at the beginning and updates the score.
             *
             * @param new_key New transposition key (Key from position moved from)
             * @param m Move that results in this Result.
             * @param depth_inc Depth increment.
             */
            void insert_move(u64 new_key, Move m, int depth_inc = 1);

            /**
             * Updates the result from another one.
             * Replaces all fields.
             *
             * @param from Search result to update from.
             */
            void update(Result& from);

        private:
            u64 key;
            Evaluation score;
            std::list<Move> pv;
            int depth;
    };
}

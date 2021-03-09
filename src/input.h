#pragma once

/**
 * Network input layer class.
 * The network input is stored as one giant block of floats for efficient upload.
 */

#include "board.h"
#include "move.h"

#include <vector>

namespace neocortex {
	class Input {
	public:
		Input(int max_batches);

		void write_frame(int batch_id, Board& b, int pov, int reps, int move_number, int halfmove_clock);
		void write_square(int batch_id, int sq, int bit, int rb1, int rb2);
		void push_frame(int batch_id);
		void pop_frame(int batch_id);

		void clear_lmm(int batch_id);
		void write_lmm(int batch_id, int m);

		std::vector<float>& get_board_input();
		std::vector<float>& get_lmm_input();
	private:
		int max_batches;
		std::vector<float> input_board, input_lmm;
		std::vector<std::vector<std::vector<std::vector<std::vector<float>>>>> hist_frames; // [batch][ply][rank][file][squarebit] holy crap!
	};
}
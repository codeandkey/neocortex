#include "input.h"
#include "square.h"
#include "color.h"

#include <algorithm>

using namespace neocortex;

Input::Input(int max_batches) {
	input_board.resize((size_t) max_batches * 8 * 8 * 85, 0.0f);
	input_lmm.resize((size_t) max_batches * 4096, 0.0f);
	hist_frames.resize(max_batches);

	this->max_batches = max_batches;
}

void Input::write_frame(int batch_id, Board& b, int pov, int reps, int move_number, int halfmove_clock) {
	float rb1 = reps & 1;
	float rb2 = reps >> 1;

	float header[15];

	for (int i = 0; i < 9; ++i) {
		header[i] = (float) ((move_number >> i) & 1);
	}

	for (int i = 0; i < 6; ++i) {
		header[9 + i] = (float)((halfmove_clock >> i) & 1);
	}

	for (size_t r = 0; r < 8; ++r) {
		for (size_t f = 0; f < 8; ++f) {
			size_t offset;

			if (pov == color::WHITE) {
				offset = (size_t) batch_id * 5440 + r * 680 + f * 85;
			} else {
				offset = (size_t)batch_id * 5440 + (7 - r) * 680 + (7 - f) * 85;
			}

			// Write square header
			memcpy(&input_board[offset], header, sizeof(float) * 15);

			// Write square piece data
			int p = b.get_piece(square::at(r, f));

			if (!piece::is_null(p)) {
				input_board[offset + 15 + 6 * (size_t) (piece::color(p) ^ pov) + piece::type(p)] = 1.0f;
			}

			// Write repetition bits
			input_board[offset + 27] = rb1;
			input_board[offset + 28] = rb2;
		}
	}
}

void Input::push_frame(int batch_id) {
	std::vector<std::vector<std::vector<float>>> frame;

	for (size_t r = 0; r < 8; ++r) {
		std::vector<std::vector<float>> rank;

		for (size_t f = 0; f < 8; ++f) {
			size_t offset = (size_t) batch_id * 5440 + r * 680 + f * 85;

			// Grab last frame
			std::vector<float> bits(&input_board[offset + 71], &input_board[offset + 71] + 14);

			// Shift frames back
			memmove(&input_board[offset + 29], &input_board[offset + 15], sizeof(float) * 14 * 4);

			// Zero most recent frame
			for (int i = 0; i < 14; ++i) {
				input_board[offset + 15 + i] = 0.0f;
			}

			rank.push_back(bits);
		}

		frame.push_back(rank);
	}

	hist_frames[batch_id].push_back(frame);
}

void Input::pop_frame(int batch_id) {
	auto ranks = hist_frames[batch_id].back();

	for (size_t r = 0; r < 8; ++r) {
		for (size_t f = 0; f < 8; ++f) {
			size_t offset = (size_t)batch_id * 5440 + r * 680 + f * 85;

			// Shift frames forward
			memmove(&input_board[offset + 15], &input_board[offset + 29], sizeof(float) * 14 * 4);

			// Grab last frame
			memcpy(&input_board[offset + 71], &ranks[r][f][0], sizeof(float) * 14);
		}
	}

	hist_frames[batch_id].pop_back();
}

void Input::clear_lmm(int batch_id) {
	std::fill_n(input_lmm.begin() + ((size_t) batch_id * 4096L), 4096, 0.0f);
}

void Input::write_lmm(int batch_id, int m) {
	input_lmm[(size_t) batch_id * 4096 + (size_t) move::src(m) * 64 + move::dst(m)] = 1.0f;
}

std::vector<float>& Input::get_board_input() {
	return input_board;
}

std::vector<float>& Input::get_lmm_input() {
	return input_lmm;
}
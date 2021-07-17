/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include <cstdint>

namespace neocortex {
	namespace zobrist {
		typedef uint64_t Key;

		/**
		 * Initializes Zobrist keys.
		 * Must be called before querying any keys.
		 */
		void init();
		
		/**
		 * Confirms whether or not the Zobrist keys have been initialized.
		 *
		 * @return true if Zobrist keys are initialized, false otherwise.
		 */
	  bool is_init();

		/**
		 * Gets the Zobrist key for a piece on a square.
		 *
		 * @param sq Input square.
		 * @param piece Input piece.
		 * @return Zobrist key for piece on square.
		 */
		Key piece(int sq, int piece);

		/**
		 * Gets the Zobrist key for a castle rights key.
		 *
		 * @param rights Castle rights.
		 * @return Associated Zobrist key.
		 */
		Key castle(int rights);

		/**
		 * Gets the zobrist key for an en-passant state.
		 *
		 * @param file Square for en-passant.
		 * @return Associated Zobrist key.
		 */
		Key en_passant(int sq);

		/**
		 * Gets the black-to-move Zobrist key.
		 *
		 * @return Zobrist key.
		 */
		Key black_to_move();
	}
}

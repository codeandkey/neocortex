#include "lookup_pawn.h"
#include "piece.h"
#include "square.h"

using namespace nc2;

std::vector<Move> _nc2_pawn_advance_table[64][2];
std::vector<Move> _nc2_pawn_capture_table[64][2];

void lookup::initialize_pawn_lookup() {
    /* Initialize white lookups */
    for (int r = 1; r < 7; ++r) {
        for (int f = 0; f < 8; ++f) {
            /* Check for left captures */
            if (f > 0) {
                if (r == 7) {
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::WHITE].push_back(Move(square::at(r, f), square::at(r + 1, f - 1), piece::Type::QUEEN));
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::WHITE].push_back(Move(square::at(r, f), square::at(r + 1, f - 1), piece::Type::KNIGHT));
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::WHITE].push_back(Move(square::at(r, f), square::at(r + 1, f - 1), piece::Type::ROOK));
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::WHITE].push_back(Move(square::at(r, f), square::at(r + 1, f - 1), piece::Type::BISHOP));
                } else {
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::WHITE].push_back(Move(square::at(r, f), square::at(r + 1, f - 1)));
                }
            }

            /* Check for right captures */
            if (f < 7) {
                if (r == 7) {
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::WHITE].push_back(Move(square::at(r, f), square::at(r + 1, f + 1), piece::Type::QUEEN));
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::WHITE].push_back(Move(square::at(r, f), square::at(r + 1, f + 1), piece::Type::KNIGHT));
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::WHITE].push_back(Move(square::at(r, f), square::at(r + 1, f + 1), piece::Type::ROOK));
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::WHITE].push_back(Move(square::at(r, f), square::at(r + 1, f + 1), piece::Type::BISHOP));
                } else {
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::WHITE].push_back(Move(square::at(r, f), square::at(r + 1, f + 1)));
                }
            }

            /* Add advances */
            if (r == 7) {
                _nc2_pawn_advance_table[square::at(r, f)][piece::Color::WHITE].push_back(Move(square::at(r, f), square::at(r + 1, f), piece::Type::QUEEN));
                _nc2_pawn_advance_table[square::at(r, f)][piece::Color::WHITE].push_back(Move(square::at(r, f), square::at(r + 1, f), piece::Type::KNIGHT));
                _nc2_pawn_advance_table[square::at(r, f)][piece::Color::WHITE].push_back(Move(square::at(r, f), square::at(r + 1, f), piece::Type::ROOK));
                _nc2_pawn_advance_table[square::at(r, f)][piece::Color::WHITE].push_back(Move(square::at(r, f), square::at(r + 1, f), piece::Type::BISHOP));
            } else {
                _nc2_pawn_advance_table[square::at(r, f)][piece::Color::WHITE].push_back(Move(square::at(r, f), square::at(r + 1, f)));

                /* Add double advance if on starting rank */
                if (r == 1) {
                    _nc2_pawn_advance_table[square::at(r, f)][piece::Color::WHITE].push_back(Move(square::at(r, f), square::at(r + 2, f)));
                }
            }
        }
    }
    
    /* Initialize black lookups */
    for (int r = 1; r < 7; ++r) {
        for (int f = 0; f < 8; ++f) {
            /* Check for left captures */
            if (f > 0) {
                if (r == 1) {
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::BLACK].push_back(Move(square::at(r, f), square::at(r - 1, f - 1), piece::Type::QUEEN));
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::BLACK].push_back(Move(square::at(r, f), square::at(r - 1, f - 1), piece::Type::KNIGHT));
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::BLACK].push_back(Move(square::at(r, f), square::at(r - 1, f - 1), piece::Type::ROOK));
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::BLACK].push_back(Move(square::at(r, f), square::at(r - 1, f - 1), piece::Type::BISHOP));
                } else {
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::BLACK].push_back(Move(square::at(r, f), square::at(r - 1, f - 1)));
                }
            }

            /* Check for right captures */
            if (f < 7) {
                if (r == 1) {
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::BLACK].push_back(Move(square::at(r, f), square::at(r - 1, f + 1), piece::Type::QUEEN));
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::BLACK].push_back(Move(square::at(r, f), square::at(r - 1, f + 1), piece::Type::KNIGHT));
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::BLACK].push_back(Move(square::at(r, f), square::at(r - 1, f + 1), piece::Type::ROOK));
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::BLACK].push_back(Move(square::at(r, f), square::at(r - 1, f + 1), piece::Type::BISHOP));
                } else {
                    _nc2_pawn_capture_table[square::at(r, f)][piece::Color::BLACK].push_back(Move(square::at(r, f), square::at(r - 1, f + 1)));
                }
            }

            /* Add advances */
            if (r == 1) {
                _nc2_pawn_advance_table[square::at(r, f)][piece::Color::BLACK].push_back(Move(square::at(r, f), square::at(r - 1, f), piece::Type::QUEEN));
                _nc2_pawn_advance_table[square::at(r, f)][piece::Color::BLACK].push_back(Move(square::at(r, f), square::at(r - 1, f), piece::Type::KNIGHT));
                _nc2_pawn_advance_table[square::at(r, f)][piece::Color::BLACK].push_back(Move(square::at(r, f), square::at(r - 1, f), piece::Type::ROOK));
                _nc2_pawn_advance_table[square::at(r, f)][piece::Color::BLACK].push_back(Move(square::at(r, f), square::at(r - 1, f), piece::Type::BISHOP));
            } else {
                _nc2_pawn_advance_table[square::at(r, f)][piece::Color::BLACK].push_back(Move(square::at(r, f), square::at(r - 1, f)));

                /* Add double advance if on starting rank */
                if (r == 6) {
                    _nc2_pawn_advance_table[square::at(r, f)][piece::Color::BLACK].push_back(Move(square::at(r, f), square::at(r - 2, f)));
                }
            }
        }
    }
}

const std::vector<Move>& lookup::pawn_advances(u8 s, u8 col) {
    return _nc2_pawn_advance_table[s][col];
}

const std::vector<Move>& lookup::pawn_captures(u8 s, u8 col) {
    return _nc2_pawn_capture_table[s][col];
}

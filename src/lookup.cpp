#include "lookup.h"

using namespace nc;

static u8 _nc_sliding_attacks[8][64];       /* [pos][rocc] */
static u64 _nc_rank_attacks[8][8][64];      /* [rank][pos][rocc] */
static u64 _nc_file_attacks[8][8][64];      /* [file][pos][rocc] */
static u64 _nc_diag_attacks[15][8][64];     /* [diag][rank][rocc] */
static u64 _nc_antidiag_attacks[15][8][64]; /* [antidiag][rank][rocc] */
static u64 _nc_knight_attacks[64];          /* [square] */
static u64 _nc_king_attacks[64];            /* [square] */
static u64 _nc_white_pawn_attacks[64];      /* [square] */
static u64 _nc_black_pawn_attacks[64];      /* [square] */

void lookup::init() {
    /* Initialize sliding attacks. */
    for (int pos = 0; pos < 8; ++pos) {
        for (int rocc = 0; rocc < 64; ++rocc) {
            u8 val = 0;
            u8 occ = rocc << 1;

            for (int x = pos + 1; x < 8; ++x) {
                val |= (1 << x);
                if ((occ >> x) & 1) break;
            }

            for (int x = pos - 1; x >= 0; --x) {
                val |= (1 << x);
                if ((occ >> x) & 1) break;
            }

            _nc_sliding_attacks[pos][rocc] = val;
        }
    }

    /* Initialize rank attacks. */
    for (int rank = 0; rank < 8; ++rank) {
        for (int pos = 0; pos < 8; ++pos) {
            for (int rocc = 0; rocc < 64; ++rocc) {
                u8 mask = lookup::sliding_attack(pos, rocc);
                _nc_rank_attacks[rank][pos][rocc] = ((u64) mask) << (rank * 8);
            }
        }
    }

    /* Initialize file attacks. */
    for (int file = 0; file < 8; ++file) {
        for (int pos = 0; pos < 8; ++pos) {
            for (int rocc = 0; rocc < 64; ++rocc) {
                u8 mask = lookup::sliding_attack(pos, rocc);
                _nc_file_attacks[file][pos][rocc] = 0;

                for (int r = 0; r < 8; ++r) {
                    _nc_file_attacks[file][pos][rocc] |= ((u64) ((mask>>r)&1)) << (file + 8*r);
                }
            }
        }
    }

    /* Initialize diagonal attacks. */
    for (int file = 0; file < 8; ++file) {
        int d = 7 - file;
        int dlen = 8 - file;

        for (int pos = 0; pos < dlen; ++pos) {
            for (int rocc = 0; rocc < 64; ++rocc) {
                u8 mask = lookup::sliding_attack(pos, rocc);
                _nc_diag_attacks[d][pos][rocc] = 0;

                for (int r = 0; r < dlen; ++r) {
                    _nc_diag_attacks[d][pos][rocc] |= ((u64) ((mask>>r)&1)) << (file + 9*r);
                }
            }
        }
    }

    for (int rank = 1; rank < 8; ++rank) {
        int d = rank + 7;
        int dlen = 8 - rank;

        for (int pos = rank; pos < 8; ++pos) {
            for (int rocc = 0; rocc < 64; ++rocc) {
                u8 mask = lookup::sliding_attack(pos, rocc);
                _nc_diag_attacks[d][pos][rocc] = 0;

                for (int r = 0; r < dlen; ++r) {
                    _nc_diag_attacks[d][pos][rocc] |= ((u64) ((mask>>(rank+r))&1)) << (8*rank + 9*r);
                }
            }
        }
    }

    /* Initialize antidiagonal attacks. */
    for (int file = 0; file < 8; ++file) {
        int d = file;
        int dlen = file + 1;

        for (int pos = 0; pos < dlen; ++pos) {
            for (int rocc = 0; rocc < 64; ++rocc) {
                u8 mask = lookup::sliding_attack(pos, rocc);
                _nc_antidiag_attacks[d][pos][rocc] = 0;

                for (int r = 0; r < dlen; ++r) {
                    _nc_antidiag_attacks[d][pos][rocc] |= ((u64) ((mask>>r)&1)) << (file + 7*r);
                }
            }
        }
    }

    for (int rank = 1; rank < 8; ++rank) {
        int d = rank + 7;
        int dlen = 8 - rank;

        for (int pos = rank; pos < 8; ++pos) {
            for (int rocc = 0; rocc < 64; ++rocc) {
                u8 mask = lookup::sliding_attack(pos, rocc);
                _nc_antidiag_attacks[d][pos][rocc] = 0;

                for (int r = 0; r < dlen; ++r) {
                    _nc_antidiag_attacks[d][pos][rocc] |= ((u64) ((mask>>(rank+r))&1)) << (8*rank + 7*r + 7);
                }
            }
        }
    }

    /* Initialize knight attacks */
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            u64 mask = 0;

            for (int a = -1; a <= 1; a += 2) {
                for (int b = -2; b <= 2; b += 4) {
                    int ar = r + a;
                    int af = f + b;

                    if (ar >= 0 && ar < 8 && af >= 0 && af < 8) {
                        mask |= ((u64) 1 << (ar*8+af));
                    }

                    int br = r + b;
                    int bf = f + a;

                    if (br >= 0 && br < 8 && bf >= 0 && bf < 8) {
                        mask |= ((u64) 1 << (br*8+bf));
                    }
                }
            }

            _nc_knight_attacks[r*8+f] = mask;
        }
    }

    /* Generate king attacks */
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8 ; ++f) {
            u64 mask = 0;

            for (int x = -1; x <= 1; ++x) {
                for (int y = -1; y <= 1; ++y) {
                    if (!x && !y) continue;

                    int df = f + x, dr = r + y;
                    if (df < 0 || df >= 8 || dr < 0 || dr >= 8) continue;

                    mask |= ((u64) 1 << (dr * 8 + df));
                }
            }

            _nc_king_attacks[r * 8 + f] = mask;
        }
    }

    /* Generate white pawn attacks */
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8 ; ++f) {
            u64 mask = 0;

            for (int x = -1; x <= 1; ++x) {
                int df = f + x, dr = r + 1;
                if (df < 0 || df >= 8 || dr < 0 || dr >= 8) continue;

                mask |= ((u64) 1 << (dr * 8 + df));
            }

            _nc_white_pawn_attacks[r * 8 + f] = mask;
        }
    }

    /* Generate black pawn attacks */
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8 ; ++f) {
            u64 mask = 0;

            for (int x = -1; x <= 1; ++x) {
                int df = f + x, dr = r - 1;
                if (df < 0 || df >= 8 || dr < 0 || dr >= 8) continue;

                mask |= ((u64) 1 << (dr * 8 + df));
            }

            _nc_black_pawn_attacks[r * 8 + f] = mask;
        }
    }

    /* All done! */
}

u8 lookup::sliding_attack(u8 pos, u8 rocc) {
    return _nc_sliding_attacks[pos][rocc];
}

u64 lookup::rank_attack(Square s, Occtable* occ) {
    u8 rank_occ = occ->get_rank(s.get_rank());
    return _nc_rank_attacks[s.get_rank()][s.get_file()][rank_occ];
}

u64 lookup::file_attack(Square s, Occtable* occ) {
    u8 file_occ = occ->get_file(s.get_file());
    return _nc_file_attacks[s.get_file()][s.get_rank()][file_occ];
}

u64 lookup::diag_attack(Square s, Occtable* occ) {
    u8 diag_occ = occ->get_diag(s.get_diag());
    return _nc_diag_attacks[s.get_diag()][s.get_rank()][diag_occ];
}

u64 lookup::antidiag_attack(Square s, Occtable* occ) {
    u8 antidiag_occ = occ->get_antidiag(s.get_antidiag());
    return _nc_antidiag_attacks[s.get_antidiag()][s.get_rank()][antidiag_occ];
}

u64 lookup::rook_attack(Square s, Occtable* occ) {
    return lookup::rank_attack(s, occ) | lookup::file_attack(s, occ);
}

u64 lookup::bishop_attack(Square s, Occtable* occ) {
    return lookup::diag_attack(s, occ) | lookup::antidiag_attack(s, occ);
}

u64 lookup::queen_attack(Square s, Occtable* occ) {
    return lookup::rook_attack(s, occ) | lookup::bishop_attack(s, occ);
}

u64 lookup::knight_attack(Square from) {
    return _nc_knight_attacks[from.get_index()];
}

u64 lookup::king_attack(Square from) {
    return _nc_king_attacks[from.get_index()];
}

u64 lookup::white_pawn_attack(Square from) {
    return _nc_white_pawn_attacks[from.get_index()];
}

u64 lookup::black_pawn_attack(Square from) {
    return _nc_black_pawn_attacks[from.get_index()];
}

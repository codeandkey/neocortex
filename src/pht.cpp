#include "pht.h"
#include "eval_consts.h"

#include <cassert>

using namespace neocortex;

pht::PawnEval* pht::pht_buffer = NULL;
int pht::pht_buffer_len = 0;
std::mutex pht::pht_mutex;

void pht::init() {
    pht::resize(pht::PHT_DEFAULT_SIZE);
}

void pht::resize(unsigned long mb) {
    lock();

    if (pht_buffer) {
        delete[] pht_buffer;
        pht_buffer = NULL;
    }

    pht_buffer_len = (mb * 1048576) / sizeof(PawnEval);
    pht_buffer = new PawnEval[pht_buffer_len];

    unlock();
}

pht::PawnEval pht::evaluate(Board& b) {
    pht::PawnEval output;

    // Pass through PHT key
    output.key = b.get_pht_key();

    // Compute common evaluation masks
    bitboard pawns = b.get_piece_occ(piece::PAWN);
    bitboard wpawns = b.get_color_occ(piece::WHITE) & pawns;
    bitboard bpawns = b.get_color_occ(piece::BLACK) & pawns;

    bitboard wstops = bb::shift(wpawns, NORTH);
    bitboard bstops = bb::shift(bpawns, SOUTH);

    bitboard wattacks = bb::shift(wpawns & ~FILE_A, NORTHWEST) | bb::shift(wpawns & ~FILE_H, NORTHEAST);
    bitboard battacks = bb::shift(bpawns & ~FILE_A, SOUTHWEST) | bb::shift(bpawns & ~FILE_H, SOUTHEAST);

    bitboard wattackspans = b.attack_spans(piece::WHITE);
    bitboard battackspans = b.attack_spans(piece::BLACK);

    bitboard wfrontspans = b.front_spans(piece::WHITE);
    bitboard bfrontspans = b.front_spans(piece::BLACK);

    bitboard wopen = wpawns & ~bfrontspans;
    bitboard bopen = bpawns & ~wfrontspans;

    // Compute backward pawns
    bitboard wbackward = bb::shift(wstops & battacks & ~wattackspans, SOUTH);
    bitboard bbackward = bb::shift(bstops & wattacks & ~battackspans, NORTH);

    output.backward = bb::popcount(wbackward) - bb::popcount(bbackward);
    output.backward *= eval::PAWN_BACKWARD;

    // Compute stragglers from backward
    bitboard wstraggler = wbackward & wopen & (RANK_2 | RANK_3);
    bitboard bstraggler = bbackward & bopen & (RANK_6 | RANK_7);

    output.straggler = bb::popcount(wstraggler) - bb::popcount(bstraggler);
    output.straggler *= eval::PAWN_STRAGGLER;

    // Compute passers
    bitboard wpassed = wopen & ~battackspans;
    bitboard bpassed = bopen & ~wattackspans;

    output.passed = bb::popcount(wpassed) - bb::popcount(bpassed);
    output.passed *= eval::PAWN_PASSED;

    // Compute candidates
    bitboard tmp_candidates = wopen & ~wpassed;
    bitboard wcandidates = 0;

    while (tmp_candidates) {
        int sq = bb::poplsb(tmp_candidates);

        bitboard atsp = attacks::pawn_attackspans(piece::WHITE, sq + NORTH);
        bitboard adj_files = bb::neighbor_files(sq);

        int helpers = adj_files & wpawns;
        int sentries = atsp & bpawns;

        if (helpers >= sentries) {
            wcandidates |= bb::mask(sq);
        }
    }

    tmp_candidates = bopen & ~bpassed;
    bitboard bcandidates = 0;

    while (tmp_candidates) {
        int sq = bb::poplsb(tmp_candidates);

        bitboard atsp = attacks::pawn_attackspans(piece::BLACK, sq + NORTH);
        bitboard adj_files = bb::neighbor_files(sq);

        int helpers = adj_files & bpawns;
        int sentries = atsp & wpawns;

        if (helpers >= sentries) {
            bcandidates |= bb::mask(sq);
        }
    }

    output.candidates = bb::popcount(wcandidates) - bb::popcount(bcandidates);
    output.candidates *= eval::PAWN_CANDIDATE;

    // Find doubled pawns
    bitboard wdoubled = wfrontspans & wpawns;
    bitboard bdoubled = bfrontspans & bpawns;

    output.doubled = bb::popcount(wdoubled) - bb::popcount(bdoubled);
    output.doubled *= eval::PAWN_DOUBLED;

    // Find isolated pawns (TODO: setwise calculation)
    bitboard tmp_isolated = wpawns;
    bitboard wisolated = 0;

    while (tmp_isolated) {
        int sq = bb::poplsb(tmp_isolated);
        bitboard adj = bb::neighbor_files(sq);

        if (adj & wpawns) continue;
        wisolated |= bb::mask(sq);
    }

    tmp_isolated = bpawns;
    bitboard bisolated = 0;

    while (tmp_isolated) {
        int sq = bb::poplsb(tmp_isolated);
        bitboard adj = bb::neighbor_files(sq);

        if (adj & bpawns) continue;
        bisolated |= bb::mask(sq);
    }

    output.isolated = bb::popcount(wisolated) - bb::popcount(bisolated);
    output.isolated *= eval::PAWN_ISOLATED;

    // Sum individual evaluation elements
    output.total += output.backward;
    output.total += output.straggler;
    output.total += output.passed;
    output.total += output.candidates;
    output.total += output.doubled;
    output.total += output.isolated;

    return output;
}

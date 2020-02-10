#include "test_mates.h"

#include "../searcher_st.h"
#include "../position.h"

using namespace nc2;

tests::MatesTest::MatesTest() : UnitTest("Mates") {}

void tests::MatesTest::run() {
    SearcherST searcher(std::cerr);

    /* Try some mating positions. */
    Position mate_in_1(std::string("r1bqkbnr/pp1ppppp/2p5/2n5/2B1P3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 4 4"));

    search::Result line = searcher.run_search(&mate_in_1, 3);
    Evaluation res = line.get_score();

    if (!res.get_forced_mate() || (res.get_mate_in() != 1)) {
        throw std::runtime_error(std::string("Test failed! Mate in 1 expected, but got ") + res.to_string());
    }

    Position mate_in_2(std::string("2bqkbn1/2pppp2/np2N3/r3P1p1/p2N2B1/5Q2/PPPPKPP1/RNB2r2 w - - 0 1"));

    line = searcher.run_search(&mate_in_2, 5);
    res = line.get_score();

    if (!res.get_forced_mate() || (res.get_mate_in() != 2)) {
        throw std::runtime_error(std::string("Test failed! Mate in 2 expected, but got ") + res.to_string() + " : " + line.get_pv_string());
    }
}

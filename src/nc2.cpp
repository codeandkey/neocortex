#include <iostream>
#include <string>
#include <vector>

#include "eval.h"
#include "engine.h"
#include "tindex.h"
#include "unit_test.h"
#include "lookup_pawn.h"
#include "lookup_king.h"
#include "lookup_knight.h"
#include "lookup_bishop.h"
#include "lookup_rook.h"
#include "tests/test_dummy.h"
#include "tests/test_mates.h"
#include "square.h"
#include "piece.h"
#include "occ.h"
#include "position.h"

void run_game();
void run_engine();

int main(int argc, char** argv) {
    nc2::eval::init();

    nc2::lookup::initialize_pawn_lookup();
    nc2::lookup::initialize_knight_lookup();
    nc2::lookup::initialize_king_lookup();
    nc2::lookup::initialize_rook_lookup();
    nc2::lookup::initialize_bishop_lookup();
    nc2::ttable::initialize_indices(0xdeadbeef);

    if (argc == 2 && std::string(argv[1]) == "test") {
        /* Run all tests. */

        int count = 0, ok = 0, failed = 0;
        std::vector<nc2::UnitTest*> tests;

        tests.push_back(new nc2::tests::DummyTest());
        tests.push_back(new nc2::tests::MatesTest());

        for (auto t : tests) {
            ++count;

            try {
                t->run();

                std::cerr << t->get_name() << " OK\n";
                ++ok;
            } catch (std::exception& e) {
                std::cerr << t->get_name() << " FAILED : " << e.what() << "\n";
                ++failed;
            }
        }

        std::cerr << "Done testing. " << failed << " out of " << count << " tests failed.\n";

        if (failed) return -1;
        return 0;
    }

    run_engine();

    return 0;
}

void run_engine() {
    nc2::Engine e(std::cout, std::cin);
    e.start_uci();
}

void run_game() {
    std::cerr << "black to move key: " << nc2::ttable::get_black_to_move_key() << "\n";

    nc2::Position p;
    while (1) {
        std::cout << "position debug: " << p.get_debug_string();

        std::vector<nc2::Position::Transition> legal_moves = p.gen_legal_moves();

        std::cout << "Legal moves: ";
        for (auto i : legal_moves) {
            std::cout << i.first.to_string() << "\n";
        }

        std::cout << "Enter a move: ";
        std::string move;
        std::cin >> move;

        bool updated = false;
        for (auto i : legal_moves) {
            if (i.first.to_string() == move) {
                std::cerr << "Applying move " << move << "\n";
                p = i.second;
                updated = true;
                break;
            }
        }

        if (!updated) {
            std::cerr << "No matched move " << move << "!\n";
        }
    }
}

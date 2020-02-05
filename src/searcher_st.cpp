#include "searcher_st.h"

using namespace nc2;

SearcherST::SearcherST(std::ostream& uci_out) : uci_out(uci_out) {}

void SearcherST::set_position(Position p) {
    root = p;
}

void SearcherST::go() {
    std::vector<Position::Transition> moves = root.gen_legal_moves();

    if (moves.size()) {
        uci_out << "bestmove " << moves[0].first.to_string() << "\n";
    }
}

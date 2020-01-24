#include "search.h"

using namespace nc;

Search::Search(std::ostream& uci_out) : uci_out(uci_out) {}

Search::~Search() {}

void Search::write_info(int depth, Evaluation eval, Move* currmove) {
}

void Search::write_bestmove(Move bestmove) {
    uci_out << "bestmove " << bestmove.to_string() << "\n";
}

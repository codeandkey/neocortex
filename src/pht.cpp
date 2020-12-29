#include "pht.h"

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

    return output;
}

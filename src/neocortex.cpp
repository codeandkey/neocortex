#include "log.h"
#include "lookup.h"
#include "bitboard.h"

#include <iostream>

int main(int argc, char** argv) {
    nc::log_init();
    nc_info("Started neocortex.");

    nc::lookup::init();
    nc_info("Built attack lookups.");

    nc::Occtable occ = nc::Occtable::standard();

    u64 b;

    b = nc::lookup::queen_attack(std::string("h2"), &occ);
    std::cout << nc::bitboard::to_string(b);

    return 0;
}

# Neocortex

Neocortex is a UCI chess engine designed from the ground up.

You can play against it [here!](https://lichess.org/?user=cortexbot#friend)

## Architecture
- Negamax alpha-beta search with iterative deepening
- Magic bitboard staged move generation
- Transposition table with Zobrist hashing
- Time control management
- *almost* complete UCI support

## Dependencies

Neocortex builds on any system with a compiler supporting C++14 or above.

## Building

To build Neocortex, execute `make` in the project root.

To build Neocortex in debugging mode, execute `make debug` instead.

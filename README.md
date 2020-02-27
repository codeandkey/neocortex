## neocortex

### about
neocortex is a UCI chess engine, designed to be efficient and powerful.

You can play against it [here!](https://lichess.org/?user=cortexbot#friend)

### architecture
- Prinipcal variation search with iterative deepening
- Bitboard representation, magic bitboard move generation
- Transposition table with Zobrist hashing
- Incremental position update, lazy PST evaluation
- Time control management
- UCI interface

### dependencies
neocortex will build on any gcc supporting c99.

To build docs, you will need [Doxygen](http://www.doxygen.nl/) installed and available in your `PATH`.

### building
To build neocortex, execute `make` in the project root.

To install the neocortex binary to `/usr/bin`, execute `make install` as superuser.

### docs
To build the html documentation, execute `make doc` in the project root.
The documentation is then available at `doc/html/index.html`.

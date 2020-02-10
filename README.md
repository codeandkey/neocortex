## neocortex

### about
neocortex is an improved total rewrite of [cortex](https://github.com/codeandkey/cortex/). It aims to be
a significantly stronger and faster engine.

### implemented features
- Alpha-beta iterative deepening search
- Bitboard move generation
- UCI interface
- Transposition table with Zobrist keys
- Time control management

### dependencies
To compile neocortex, you need a compiler supporting c++11 (gcc >= 4.8.1).

To build docs, you will need [Doxygen](http://www.doxygen.nl/) installed and available in your `PATH`.

### building
To build neocortex, execute `make` in the project root.

To install the neocortex binary to `/usr/bin`, execute `make install` as superuser.

### docs
To build the html documentation, execute `make doc` in the project root.
The documentation is then available at `doc/html/index.html`.
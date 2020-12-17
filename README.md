# Neocortex

[![Build Status](https://travis-ci.com/codeandkey/neocortex.svg?branch=master)](https://travis-ci.com/codeandkey/neocortex) [![Coverage Status](https://coveralls.io/repos/github/codeandkey/neocortex/badge.svg?branch=master&kill_cache=1)](https://coveralls.io/github/codeandkey/neocortex?branch=master)

A homemade C++ chess engine.

Play a game with me [here!](https://neocortex-cortexbot.herokuapp.com)

## architecture

- Negamax alpha-beta search with iterative deepening
- Lazy SMP parallel search
- Transposition table with Zobrist hashing
- Time control management
- UCI support

## methods

Many of the methods used in this engine are inspired by the awesome resources available at the [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page):

- Guard heuristic
- Static exchange evaluation
- Specialized quiescence/evasion movegen
- History heuristic

## dependencies

Neocortex builds on GCC 7.3+ or MSVC 2015+. To run the test suite [GoogleTest](https://github.com/google/googletest) must be available on the host.

## building

**Linux**<br>
Execute `make` in the project root to build in release mode.<br>
Execute `make debug` to build in debug mode, and `make test` to build the test suite.<br>

*(Note: test suite builds require GoogleTest to be installed in the gcc include and lib paths.)*

**Windows**<br>
Open `neocortex.vcxproj` in Visual Studio 2015 or newer. Build the solution in either Debug or Release mode.

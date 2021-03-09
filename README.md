# Neocortex

[![Build Status](https://travis-ci.com/codeandkey/nczero.svg?branch=master)](https://travis-ci.com/codeandkey/nczero) [![Coverage Status](https://coveralls.io/repos/github/codeandkey/nczero/badge.svg?branch=master&kill_cache=1)](https://coveralls.io/github/codeandkey/nczero?branch=master)

A self-learning chess engine.

## architecture

Many techniques in this engine are inspired by the revolutionary [AlphaZero](https://arxiv.org/pdf/1712.01815.pdf) as well as the extensive [Chess Programming Wiki](https://www.chessprogramming.org).

- Parallel Monte Carlo Tree Search
- Bitboard move generation
- Incremental input layers

## dependencies

- GCC 8+ on \*NIX, or MSVC 15.7+ on Windows
- [TensorFlow C API](https://www.tensorflow.org/install/lang_c)
- [GoogleTest](https://github.com/google/googletest) to run tests

## building

**Linux**<br>
Execute `make` in the project root to build in release mode.<br>
Execute `make debug` to build in debug mode, and `make test` to build the test suite.<br>

**Windows**<br>
Open `neocortex.vcxproj` in Visual Studio. Build the solution in either Debug or Release mode.<br>
To run the test suite, open `neocortex_test.vcxproj` and build in Debug mode.<br>

> NOTE: Building the test suite requires GoogleTest to be available on the host.

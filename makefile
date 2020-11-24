CXX = g++
CXXFLAGS = -std=c++14 -Wall -Werror -O3
LDFLAGS = -pthread

SOURCES = $(filter-out src/main.cpp, $(wildcard src/*.cpp))
OBJECTS = $(SOURCES:.cpp=.o)

TEST_OUTPUT = neocortex_test
OUTPUT = neocortex

.PHONY: clean debug release
.DEFAULT_GOAL := release

debug: CXXFLAGS+=-g
debug: clean $(OUTPUT)

test: CXXFLAGS+=-g -fprofile-arcs -ftest-coverage
test: LDFLAGS+=-lgtest -lgcov
test: clean $(TEST_OUTPUT)

release: CXXFLAGS+=-DNDEBUG
release: clean $(OUTPUT)

all: release

$(OUTPUT): $(OBJECTS) src/main.o
	$(CXX) $^ $(LDFLAGS) -o $@

$(TEST_OUTPUT): $(OBJECTS) test/test.o
	$(CXX) $^ $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OUTPUT) $(TEST_OUTPUT) src/*.o test/*.o src/*.gcda src/*.gcno src/*.gcov test/*.gcda test/*.gcno test/*.gcov

CXX = g++
CXXFLAGS = -std=c++14 -Wall -Werror -O3
LDFLAGS = -pthread

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

OUTPUT = neocortex

.PHONY: clean debug release
.DEFAULT_GOAL := release

debug: CXXFLAGS+=-g
release: CXXFLAGS+=-DNDEBUG

debug: clean $(OUTPUT)
	@if [ -e $(dirname $0)/build-release ]; then make clean; fi
	@rm -f .build-release
	@touch .build-debug

release: clean $(OUTPUT)
	@if [ -e $(dirname $0)/build-debug ]; then make clean; fi
	@rm -f .build-debug
	@touch .build-release
	
all: release

$(OUTPUT): $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OUTPUT) $(OBJECTS)

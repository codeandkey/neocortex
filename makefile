CXX = g++
CXXFLAGS = -std=c++14 -Wall -Werror -O3
LDFLAGS = -pthread

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

OUTPUT = neocortex

.PHONY: clean debug release
.DEFAULT_GOAL := release

debug: CXXFLAGS+=-g
debug: clean $(OUTPUT)

profile: CXXFLAGS+=-g
profile: CXXFLAGS+=-pg
profile: LDFLAGS+=-pg
profile: clean $(OUTPUT)

release: CXXFLAGS+=-DNDEBUG
release: clean $(OUTPUT)

all: release

$(OUTPUT): $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OUTPUT) $(OBJECTS)

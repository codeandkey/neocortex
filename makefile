CXX = g++
CXXFLAGS = -std=c++14 -Wall -Werror -O3 -DNDEBUG
LDFLAGS = -pthread

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

OUTPUT = neocortex

.PHONY: clean debug release

debug: $(OUTPUT) CXXFLAGS+=-g
release: CXXFLAGS+=-DNDEBUG

debug:
	-[ -e $(dirname $0)/build-release ] && make clean
	rm -f .build-release

release:
	-[ -e $(dirname $0)/build-release ] && make clean
	rm -f .build-debug
	
all: release $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OUTPUT) $(OBJECTS)

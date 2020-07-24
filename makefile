CXX = g++
CXXFLAGS = -std=c++14 -Wall -Werror -O3
LDFLAGS = -pthread

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

OUTPUT = pine

.PHONY: clean

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OUTPUT) $(OBJECTS)

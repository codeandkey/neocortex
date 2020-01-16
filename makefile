CXX      = g++
CXXFLAGS = -std=c++11 -Wall -Werror -g
LDFLAGS  =
OUTPUT   = neocortex

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: clean

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	@echo ld $@
	@$(CXX) $^ $(LDFLAGS) -o $@

%.o: %.cpp
	@echo cc $<
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo clean
	@rm -rf $(OUTPUT) $(OBJECTS) doc

doc:
	@echo building docs..
	@doxygen doxygen.ini

install:
	@echo installing..
	cp neocortex /usr/bin

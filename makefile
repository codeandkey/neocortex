CC      = gcc
CCFLAGS = -std=gnu99 -Wall -Werror -g -DNC_DEBUG
LDFLAGS  = -pthread -rdynamic
OUTPUT   = nc3

SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:.c=.o)

.PHONY: clean

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	@echo ld $@
	@$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c
	@echo cc $<
	@$(CC) $(CCFLAGS) -c $< -o $@

clean:
	@echo clean
	@rm -rf $(OUTPUT) $(OBJECTS) doc

doc:
	@echo building docs..
	@doxygen doxygen.ini

install:
	@echo installing..
	cp $(OUTPUT) /usr/bin

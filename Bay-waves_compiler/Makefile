CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g
EXEC = bay
SOURCES = $(shell find src -type f -name '*.c' | sort)
OBJECTS = $(patsubst src/%.c,src/%.o,$(SOURCES))

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EXEC)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(EXEC)
	cp ./$(EXEC) /usr/local/bin/bay

clean:
	rm -f $(EXEC) $(OBJECTS) *.out

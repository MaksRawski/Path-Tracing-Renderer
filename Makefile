CC = gcc
CFLAGS = -g -Wall -Wextra -Wno-unused -Werror -Ilib/include
LDFLAGS = -lglfw -ldl -lm
TARGET = bin/main
SRC := $(wildcard src/*.c)
OBJ := $(SRC:src/%.c=bin/%.o)
GLAD_SRC = ./lib/src/gl.c

all: $(TARGET)

$(TARGET): $(OBJ) $(GLAD_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $(TARGET)

bin/%.o: src/%.c
	@mkdir -p bin
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf bin/*

.PHONY: all clean

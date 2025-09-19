CC = clang
CFLAGS = -g -Wall -Wextra -Wno-unused -Werror -Ilib/include -Iinclude
LDFLAGS = -lglfw -ldl -lm
TARGET = build/main
SRC := $(shell find src -name '*.c')
OBJ := $(SRC:src/%.c=build/%.o)
GLAD_SRC = ./lib/src/gl.c

all: $(TARGET)

$(TARGET): $(OBJ) $(GLAD_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $(TARGET)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build/*

.PHONY: all clean

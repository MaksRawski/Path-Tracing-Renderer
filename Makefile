CC = clang
CFLAGS = -g -Wall -Wextra -Wno-unused -Werror -Wpedantic -Ilib/include -Iinclude
LDFLAGS = -lglfw -ldl -lm

MAIN := src/main.c
GLAD_SRC := ./lib/src/gl.c

SRC := $(shell find src -name '*.c')
OBJ := $(SRC:src/%.c=build/%.o)
DEPS := $(SRC:src/%.c=build/%.d)
TARGET := build/$(shell basename $(MAIN) .c)
BUILD_DEPS = $(OBJ) $(GLAD_SRC)

all: $(TARGET)

$(TARGET): $(BUILD_DEPS) $(MAIN)
	$(CC) $(CFLAGS) $(LDFLAGS) $(BUILD_DEPS) -o $@

-include $(DEPENDS)

build/%.o: src/%.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf build/*

.PHONY: all clean


CC = clang
CFLAGS = -Wall -Wextra -Wno-unused -Werror -Wpedantic -Ilib/include -Iinclude
DEBUG_FLAGS = -g
RELEASE_FLAGS = -O2
LDFLAGS = -lglfw -ldl -lm

# must be either: debug release
MODE = debug

ifeq ($(MODE), debug)
	CFLAGS += $(DEBUG_FLAGS)
endif
ifeq ($(MODE), release)
	CFLAGS += $(RELEASE_FLAGS)
endif


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

# ------------------------------TESTS------------------------------
TESTS_MAIN := tests/main.c

TESTS_TARGET := build/$(TESTS_MAIN:.c=)
TESTS_DEPS = $(filter-out $(TARGET).o,$(BUILD_DEPS)) $(TESTS_MAIN)

tests: $(TESTS_TARGET)
	./$(TESTS_TARGET)

$(TESTS_TARGET): $(TESTS_DEPS) tests/asserts.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) $(TESTS_DEPS) -o $@

clean:
	rm -rf build/*

.PHONY: all clean tests

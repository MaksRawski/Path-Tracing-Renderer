CC = clang
CFLAGS = -Wall -Wextra -Wno-unused -Werror -Wpedantic -Ilib/include -Iinclude
DEBUG_FLAGS = -g
RELEASE_FLAGS = -O2 -DNDEBUG
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

TARGET := build/$(shell basename $(MAIN) .c)
SRC := $(shell find src -name '*.c')
OBJ := $(SRC:src/%.c=build/%.o)
BUILD_DEPS := $(OBJ) $(GLAD_SRC)
D_FILES := $(OBJ:%.o=%.d)

all: $(TARGET)

$(TARGET): $(BUILD_DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(BUILD_DEPS) -o $@

-include $(D_FILES)

build/%.o: src/%.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# ------------------------------TESTS------------------------------
TESTS_MAIN := tests/main.c

TESTS_TARGET := build/$(TESTS_MAIN:.c=)
TESTS_SRC := $(shell find tests -name '*.c')
TESTS_OBJ := $(filter-out $(TARGET).o,$(OBJ)) $(TESTS_SRC:tests/%.c=build/tests/%.o)
TESTS_DEPS := $(TESTS_OBJ) $(GLAD_SRC)
TESTS_D_FILES := $(TESTS_OBJ:%.o=%.d)

CFLAGS += -Itests

tests: $(TESTS_TARGET)
	./$(TESTS_TARGET)

-include $(TESTS_D_FILES)

build/tests/%.o: tests/%.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(TESTS_TARGET): $(TESTS_DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(TESTS_DEPS) -o $@

clean:
	rm -rf build/*

.PHONY: all clean tests

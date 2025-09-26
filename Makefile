CC = clang
CFLAGS = -Wall -Wextra -Wno-unused -Werror -Wpedantic -Ilib/include -Iinclude
DEBUG_FLAGS = -g
RELEASE_FLAGS = -O2 -DNDEBUG
LDFLAGS = -lglfw -ldl -lm

# must be either debug or release
MODE = debug

ifeq ($(MODE), debug)
	CFLAGS += $(DEBUG_FLAGS)
	BUILD_DIR = build/debug
endif
ifeq ($(MODE), release)
	CFLAGS += $(RELEASE_FLAGS)
	BUILD_DIR = build/release
endif


MAIN := src/main.c
GLAD_SRC := ./lib/src/gl.c

TARGET := $(BUILD_DIR)/$(shell basename $(MAIN) .c)
SRC := $(shell find src -name '*.c')
OBJ := $(SRC:src/%.c=$(BUILD_DIR)/%.o)
BUILD_DEPS := $(OBJ) $(GLAD_SRC)
D_FILES := $(OBJ:%.o=%.d)

all: $(TARGET)

$(TARGET): $(BUILD_DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(BUILD_DEPS) -o $@

-include $(D_FILES)

$(BUILD_DIR)/%.o: src/%.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# ------------------------------TESTS------------------------------
TESTS_MAIN := tests/main.c

TESTS_TARGET := $(BUILD_DIR)/$(TESTS_MAIN:.c=)
TESTS_SRC := $(shell find tests -name '*.c')
TESTS_OBJ := $(filter-out $(TARGET).o,$(OBJ)) $(TESTS_SRC:tests/%.c=$(BUILD_DIR)/tests/%.o)
TESTS_DEPS := $(TESTS_OBJ) $(GLAD_SRC)
TESTS_D_FILES := $(TESTS_OBJ:%.o=%.d)

CFLAGS += -Itests

tests: $(TESTS_TARGET)
	./$(TESTS_TARGET)

-include $(TESTS_D_FILES)

$(BUILD_DIR)/tests/%.o: tests/%.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(TESTS_TARGET): $(TESTS_DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(TESTS_DEPS) -o $@

clean:
	rm -rf build/*

.PHONY: all clean tests

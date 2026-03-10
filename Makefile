CC = clang
CXX = clang++

CFLAGS = -std=c11 -Wall -Wextra -pedantic-errors
CFLAGS += -Wcast-align -Wpointer-arith -Wcast-qual -Wunreachable-code -Wshadow 
CFLAGS += -Iinclude
DEBUG_FLAGS = -g
RELEASE_FLAGS = -O2 -DNDEBUG -march=native
TARGET_OS = linux

include lib/vars.mk
CFLAGS += $(LIB_INCLUDE_PATHS:%=-Ilib/%) -flto
LDFLAGS = $(LIB_TARGETS:%=lib/%) 

ifeq ($(TARGET_OS), linux)
	CFLAGS += `pkg-config --cflags gtk+-3.0` 
	LDFLAGS += -ldl -lm -lX11 `pkg-config --libs gtk+-3.0`
endif

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

TARGET := $(BUILD_DIR)/$(shell basename $(MAIN) .c)
SRC := $(shell find src -name '*.c')
OBJ := $(SRC:src/%.c=$(BUILD_DIR)/%.o)
D_FILES := $(OBJ:%.o=%.d)
BUILD_DEPS := $(OBJ)

all: $(TARGET)

$(TARGET): $(BUILD_DEPS)
	$(CXX) $(CFLAGS) $(BUILD_DEPS) $(LDFLAGS) -o $@

-include $(D_FILES)

$(BUILD_DIR)/%.o: src/%.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# ------------------------------TESTS------------------------------
TESTS_MAIN := tests/main.c

TESTS_TARGET := $(BUILD_DIR)/$(TESTS_MAIN:.c=)
TESTS_SRC := $(shell find tests -name '*.c')
TESTS_OBJ := $(filter-out $(TARGET).o,$(OBJ)) $(TESTS_SRC:tests/%.c=$(BUILD_DIR)/tests/%.o)
TESTS_DEPS := $(TESTS_OBJ)
TESTS_D_FILES := $(TESTS_OBJ:%.o=%.d)

CFLAGS += -Itests

tests: $(TESTS_TARGET)
	./$(TESTS_TARGET)

-include $(TESTS_D_FILES)

$(BUILD_DIR)/tests/%.o: tests/%.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(TESTS_TARGET): $(TESTS_DEPS)
	$(CXX) $(CFLAGS) $(TESTS_DEPS) $(LDFLAGS) -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean tests

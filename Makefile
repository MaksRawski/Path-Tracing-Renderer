CC = clang
# NOTE: because C++ library is used (Dear ImGui) a C++ compiler is necessary for linking with it
CXX = clang++

CFLAGS = -std=c11 -Wall -Wextra -pedantic-errors
CFLAGS += -Wcast-align -Wpointer-arith -Wcast-qual -Wunreachable-code -Wshadow 
CFLAGS += -Iinclude 
DEBUG_FLAGS = -g
RELEASE_FLAGS = -O2 -DNDEBUG -march=native -flto
TARGET_OS = linux
BUILD_DIR_PREFIX = build

include lib/vars.mk
CFLAGS += $(LIB_INCLUDE_PATHS:%=-Ilib/%)
LDFLAGS = $(LIB_TARGETS:%=lib/%) 

ifeq ($(TARGET_OS), linux)
	LDFLAGS += -ldl -lm -lX11 -lpthread
endif

# must be either debug or release
MODE = debug

ifeq ($(MODE), debug)
	CFLAGS += $(DEBUG_FLAGS)
	BUILD_DIR = $(BUILD_DIR_PREFIX)/debug
endif
ifeq ($(MODE), release)
	CFLAGS += $(RELEASE_FLAGS)
	BUILD_DIR = $(BUILD_DIR_PREFIX)/release
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

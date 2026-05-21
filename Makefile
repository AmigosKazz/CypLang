# CypLang Makefile

# Compiler and flags
CC = gcc

# LLVM detection: prefer llvm-config in PATH, fall back to Homebrew keg-only path.
LLVM_CONFIG ?= $(shell command -v llvm-config 2>/dev/null || echo /opt/homebrew/opt/llvm/bin/llvm-config)
LLVM_CFLAGS := $(shell $(LLVM_CONFIG) --cflags 2>/dev/null)
LLVM_LDFLAGS := $(shell $(LLVM_CONFIG) --ldflags --libs core --system-libs 2>/dev/null)

CFLAGS = -Wall -Wextra -std=c11 -I./include $(LLVM_CFLAGS)
LDFLAGS = -lm $(LLVM_LDFLAGS)

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin
OBJ_DIR = $(BUILD_DIR)/obj
INCLUDE_DIR = include

# Default input file
FILE ?= input.cyp

# The final executable
TARGET = $(BIN_DIR)/cyplang

# Source files (recursive: src/, src/frontend/**, src/middle/, src/backend/)
SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Create directories if they don't exist
$(shell mkdir -p $(BIN_DIR) $(OBJ_DIR) 2>/dev/null)

# Default target
all: $(TARGET)

# Linking the executable
$(TARGET): $(OBJS)
	@echo "Linking $@..."
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "Build complete!"

# Compiling source files (mkdir handles nested obj subdirs)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Run the program with input file
run: $(TARGET)
	@echo "Running CypLang on $(FILE)..."
	$(TARGET) $(FILE)

# Clean build
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR)
	@echo "Clean complete!"

# Rebuild
rebuild: clean all

# Run integration tests (tests/cases/*.cyp)
test: $(TARGET)
	@./tests/run.sh

# Regenerate test baselines (use with care — review the diff before committing)
test-update: $(TARGET)
	@UPDATE=1 ./tests/run.sh

.PHONY: all run clean rebuild test test-update
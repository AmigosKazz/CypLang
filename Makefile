# CypLang Makefile

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I./include
LDFLAGS = -lm

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

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
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

# Compiling source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
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

.PHONY: all run clean rebuild
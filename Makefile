# NeuroLock Makefile

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -std=c11 -O2
LDFLAGS = -lssl -lcrypto -lm

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# Target executable
TARGET = $(BIN_DIR)/neurolock

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Debug build settings
DEBUG_FLAGS = -g -DDEBUG -O0

# Default target
all: directories $(TARGET)

# Create necessary directories
directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)
	@mkdir -p templates

# Link object files to create executable
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean all

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(OBJ_DIR)
	@rm -rf $(BIN_DIR)
	@echo "Clean complete"

# Clean templates (be careful!)
clean-templates:
	@echo "WARNING: This will delete all user templates!"
	@read -p "Are you sure? (yes/no): " confirm && [ "$$confirm" = "yes" ]
	@rm -rf templates
	@mkdir -p templates
	@echo "Templates deleted"

# Install (copy to system bin)
install: all
	@echo "Installing $(TARGET) to /usr/local/bin..."
	@sudo cp $(TARGET) /usr/local/bin/neurolock
	@echo "Installation complete"

# Uninstall
uninstall:
	@echo "Uninstalling neurolock..."
	@sudo rm -f /usr/local/bin/neurolock
	@echo "Uninstallation complete"

# Run tests
test: all
	@echo "Running system tests..."
	$(TARGET) test

# Help
help:
	@echo "NeuroLock Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all              Build the project (default)"
	@echo "  debug            Build with debug symbols"
	@echo "  clean            Remove build artifacts"
	@echo "  clean-templates  Delete all user templates"
	@echo "  install          Install to /usr/local/bin"
	@echo "  uninstall        Remove from /usr/local/bin"
	@echo "  test             Run system tests"
	@echo "  help             Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make              # Build project"
	@echo "  make debug        # Build with debug info"
	@echo "  make clean        # Clean build"
	@echo "  make install      # Install system-wide"

.PHONY: all directories debug clean clean-templates install uninstall test help

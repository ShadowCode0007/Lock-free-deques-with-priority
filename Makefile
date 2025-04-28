# Makefile for Priority-Aware Work Stealing Tests

# Compiler and flags
CC = gcc
CFLAGS = -Wall -O2 -g -DDEBUG
LDFLAGS = -lpthread -lrt

# Source files and directories
SRC_DIR = .
TEST_DIR = ./tests
BUILD_DIR = ./build
LOG_DIR = ./logs

# Source files
SRC_FILES = $(SRC_DIR)/taskqueue.c

# Get all test files
TEST_FILES = $(wildcard $(TEST_DIR)/*.c)
TEST_EXECS = $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/%,$(TEST_FILES))

# Default target - compile and run all tests
.PHONY: all
all: setup $(TEST_EXECS) run_tests

# Setup directories
setup:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(LOG_DIR)

# Compile rule for each test executable
$(BUILD_DIR)/%: $(TEST_DIR)/%.c $(SRC_FILES)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $(SRC_FILES) $< -o $@ $(LDFLAGS)

# Run all tests
run_tests: $(TEST_EXECS)
	@echo "Running all tests..."
	@for test in $(TEST_EXECS); do \
		echo "Running $${test}..."; \
		base_name=$$(basename $$test); \
		$$test > $(LOG_DIR)/$${base_name}.log 2> $(LOG_DIR)/$${base_name}.debug.log; \
		if [ $$? -eq 0 ]; then \
			echo "$${base_name}: SUCCESS"; \
		else \
			echo "$${base_name}: FAILED"; \
		fi; \
	done

# Clean up build artifacts and logs
.PHONY: clean
clean:
	@echo "Cleaning up everything..."
	@rm -rf $(BUILD_DIR)/*
	@rm -rf $(LOG_DIR)/*
	@echo "All build artifacts and logs have been removed."

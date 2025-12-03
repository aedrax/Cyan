# Makefile for Cyan library examples
#
# Usage:
#   make          - Build all examples
#   make clean    - Remove built executables
#   make run      - Build and run all examples

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -I../include
LDFLAGS =
BUILD_DIR = build

# List of example programs
EXAMPLES = \
	00_primitive_types \
	01_option_basics \
	02_result_error_handling \
	03_vector_collections \
	04_defer_cleanup \
	05_smart_pointers \
	06_pattern_matching \
	07_functional \
	08_vtable_api

# Add build directory prefix to examples
EXAMPLE_BINS = $(addprefix $(BUILD_DIR)/,$(EXAMPLES))

.PHONY: all clean run

all: $(BUILD_DIR) $(EXAMPLE_BINS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Pattern rule for building examples into build directory
$(BUILD_DIR)/%: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)

run: all
	@echo "=== Running all examples ===\n"
	@for example in $(EXAMPLES); do \
		echo "\n>>> Running $$example <<<\n"; \
		./$(BUILD_DIR)/$$example; \
		echo ""; \
	done

# Cyan Library Makefile
# Header-only C11 library with property-based testing

CC = gcc
# Use gnu11 to support GCC extensions (nested functions for defer, cleanup attribute)
CFLAGS = -std=gnu11 -Wall -Wextra -I include -I vendor/theft/inc
LDFLAGS = -L vendor/theft/build -ltheft -lm

# Sanitizer flags (enabled with SANITIZE=1)
ifdef SANITIZE
CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address,undefined
endif

# Debug flags (enabled with DEBUG=1)
ifdef DEBUG
CFLAGS += -g -O0
else
CFLAGS += -O2
endif

# Directories
SRC_DIR = tests
BUILD_DIR = build
INCLUDE_DIR = include/cyan

# Test sources and objects
TEST_SRCS = $(wildcard $(SRC_DIR)/*.c)
TEST_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(TEST_SRCS))
TEST_BIN = $(BUILD_DIR)/test_runner

# Theft library
THEFT_DIR = vendor/theft
THEFT_LIB = $(THEFT_DIR)/build/libtheft.a

.PHONY: all test clean theft dirs

all: dirs theft $(TEST_BIN)

dirs:
	@mkdir -p $(BUILD_DIR)

# Build theft library
theft: $(THEFT_LIB)

# Theft source files
THEFT_SRCS = $(wildcard $(THEFT_DIR)/src/*.c)
THEFT_OBJS = $(patsubst $(THEFT_DIR)/src/%.c,$(THEFT_DIR)/build/%.o,$(THEFT_SRCS))

$(THEFT_LIB): $(THEFT_OBJS)
	ar rcs $@ $^

$(THEFT_DIR)/build/%.o: $(THEFT_DIR)/src/%.c
	@mkdir -p $(THEFT_DIR)/build
	$(CC) -c $< -I $(THEFT_DIR)/inc -I $(THEFT_DIR)/src -o $@

# Build test objects
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | dirs
	$(CC) $(CFLAGS) -c $< -o $@

# Link test runner
$(TEST_BIN): $(TEST_OBJS) $(THEFT_LIB)
	$(CC) $(TEST_OBJS) $(LDFLAGS) -o $@

# Run tests
test: all
	@echo "Running tests..."
	./$(TEST_BIN)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(THEFT_DIR)/build

# Help target
help:
	@echo "Cyan Library Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build test runner (default)"
	@echo "  test     - Build and run tests"
	@echo "  clean    - Remove build artifacts"
	@echo "  help     - Show this help"
	@echo ""
	@echo "Options:"
	@echo "  SANITIZE=1  - Enable AddressSanitizer and UBSan"
	@echo "  DEBUG=1     - Enable debug symbols, disable optimization"

# Cyan - Modern C11 Header Library

A header-only C11 library that brings modern programming paradigms to C, including Option/Result types, functional primitives, smart pointers, and more.

## Features

- **Option Type** - Explicit nullable value handling
- **Result Type** - Explicit error handling without errno
- **Vector** - Generic dynamic arrays with bounds checking
- **Slice** - Safe array views with bounds information
- **HashMap** - Type-safe hash maps with O(1) lookups
- **String** - Dynamic strings with safe operations
- **Functional Primitives** - map, filter, reduce, foreach
- **Smart Pointers** - Unique and shared pointers with automatic cleanup
- **Defer** - Scope-based resource cleanup (RAII-style)
- **Coroutines** - Stackful cooperative multitasking
- **Channels** - CSP-style communication primitives
- **Pattern Matching** - Ergonomic Option/Result handling
- **Serialization** - Text-based data serialization

## Quick Start

### Installation

Copy the `include/cyan/` directory to your project and include the headers:

```c
#include <cyan/cyan.h>  // Include everything
// Or include individual headers:
#include <cyan/option.h>
#include <cyan/vector.h>
```

### Basic Usage

#### Option Type

```c
#include <cyan/option.h>

OPTION_DEFINE(int);  // Define Option_int type

Option_int find_value(int arr[], size_t len, int target) {
    for (size_t i = 0; i < len; i++) {
        if (arr[i] == target) return Some(int, arr[i]);
    }
    return None(int);
}

int main(void) {
    int arr[] = {1, 2, 3, 4, 5};
    Option_int result = find_value(arr, 5, 3);
    
    if (is_some(result)) {
        printf("Found: %d\n", unwrap(result));
    } else {
        printf("Not found\n");
    }
    
    // Or use unwrap_or for a default value
    int val = unwrap_or(result, -1);
    return 0;
}
```

#### Result Type

```c
#include <cyan/result.h>

RESULT_DEFINE(int, const_charp);  // Define Result_int_const_charp

Result_int_const_charp parse_positive(const char *str) {
    int val = atoi(str);
    if (val <= 0) return Err(int, const_charp, "must be positive");
    return Ok(int, const_charp, val);
}

int main(void) {
    Result_int_const_charp res = parse_positive("42");
    
    if (is_ok(res)) {
        printf("Parsed: %d\n", unwrap_ok(res));
    } else {
        printf("Error: %s\n", unwrap_err(res));
    }
    return 0;
}
```

#### Vector (Dynamic Array)

```c
#include <cyan/option.h>
#include <cyan/vector.h>

OPTION_DEFINE(int);
VECTOR_DEFINE(int);

int main(void) {
    Vec_int v = vec_int_new();
    
    // Push elements
    vec_int_push(&v, 10);
    vec_int_push(&v, 20);
    vec_int_push(&v, 30);
    
    // Access with bounds checking
    Option_int elem = vec_int_get(&v, 1);
    if (is_some(elem)) {
        printf("Element at 1: %d\n", unwrap(elem));
    }
    
    // Pop elements
    while (is_some(elem = vec_int_pop(&v))) {
        printf("Popped: %d\n", unwrap(elem));
    }
    
    vec_int_free(&v);
    return 0;
}
```

#### Defer (Automatic Cleanup)

```c
#include <cyan/defer.h>

int main(void) {
    FILE *f = fopen("test.txt", "r");
    if (!f) return 1;
    
    defer({ fclose(f); });  // Will execute when scope exits
    
    // Work with file...
    // No need to remember to close - defer handles it
    
    return 0;  // fclose(f) called automatically here
}
```

#### Smart Pointers

```c
#include <cyan/smartptr.h>

OPTION_DEFINE(int);
UNIQUE_PTR_DEFINE(int);
SHARED_PTR_DEFINE(int);

int main(void) {
    // Unique pointer - exclusive ownership
    {
        unique_ptr(int, p, 42);  // Auto-cleanup on scope exit
        printf("Value: %d\n", unique_int_deref(&p));
    }  // Memory freed here
    
    // Shared pointer - reference counted
    SharedPtr_int s1 = shared_int_new(100);
    SharedPtr_int s2 = shared_int_clone(&s1);  // Count = 2
    
    printf("Count: %zu\n", shared_int_count(&s1));  // Prints 2
    
    shared_int_release(&s1);  // Count = 1
    shared_int_release(&s2);  // Count = 0, memory freed
    
    return 0;
}
```

#### Pattern Matching

```c
#include <cyan/match.h>

OPTION_DEFINE(int);
RESULT_DEFINE(int, const_charp);

int main(void) {
    Option_int opt = Some(int, 42);
    
    match_option(opt, int,
        some(val) {
            printf("Got value: %d\n", val);
        },
        none {
            printf("No value\n");
        }
    );
    
    Result_int_const_charp res = Ok(int, const_charp, 100);
    
    match_result(res, int, const_charp,
        ok(val) {
            printf("Success: %d\n", val);
        },
        err(e) {
            printf("Error: %s\n", e);
        }
    );
    
    return 0;
}
```

## Configuration

Configure the library by defining macros before including headers:

```c
// Custom panic handler
#define CYAN_PANIC(msg) my_panic_handler(msg)

// Collection settings
#define CYAN_DEFAULT_CAPACITY 8
#define CYAN_GROWTH_FACTOR 2

// Coroutine stack size
#define CYAN_CORO_STACK_SIZE (128 * 1024)

// Enable thread-safe channels
#define CYAN_CHANNEL_THREADSAFE

#include <cyan/cyan.h>
```

## Requirements

- C11 compatible compiler (GCC, Clang, or MSVC with C11 support)
- GCC/Clang recommended for `defer` and auto-cleanup features (uses `__attribute__((cleanup))`)

## Building Tests

```bash
make test
```

## License

MIT License - see LICENSE file for details.

## Examples

See the `examples/` directory for complete example programs demonstrating each feature.

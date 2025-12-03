# Cyan - Modern C11 Header Library
> Pronounced "See-yan" because I'm a monster

A header-only C11 library that brings modern programming paradigms to C, including Option/Result types, functional primitives, smart pointers, and more.

## Features

- **Primitive Type Aliases** - Concise, predictable-size type names (i32, u64, f32, etc.)
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

#### Primitive Type Aliases

```c
#include <cyan/common.h>

// Fixed-width integers
i8  a = 127;           // int8_t
i16 b = 32767;         // int16_t
i32 c = 2147483647;    // int32_t
i64 d = 9223372036854775807LL;  // int64_t

u8  e = 255;           // uint8_t
u16 f = 65535;         // uint16_t
u32 g = 4294967295U;   // uint32_t
u64 h = 18446744073709551615ULL;  // uint64_t

// Pointer-sized integers
usize len = sizeof(array) / sizeof(array[0]);  // size_t compatible
isize offset = -100;   // signed pointer-sized

// Floating-point
f32 pi_f = 3.14159f;   // float
f64 pi_d = 3.14159265358979;  // double

// Type-erased pointer
anyopaque* generic_ptr = &some_data;
```

**Available Types:**

| Category | Types |
|----------|-------|
| Signed integers | `i8`, `i16`, `i32`, `i64`, `i128`* |
| Unsigned integers | `u8`, `u16`, `u32`, `u64`, `u128`* |
| Pointer-sized | `isize`, `usize` |
| Floating-point | `f16`*, `f32`, `f64`, `f80`*, `f128`* |
| Special | `bool`, `anyopaque` |

*Platform-dependent. Check `CYAN_HAS_INT128`, `CYAN_HAS_FLOAT16`, `CYAN_HAS_FLOAT80`, `CYAN_HAS_FLOAT128` macros.

#### Option Type

```c
#include <cyan/common.h>
#include <cyan/option.h>

OPTION_DEFINE(i32);  // Define Option_i32 type

Option_i32 find_value(i32 arr[], usize len, i32 target) {
    for (usize i = 0; i < len; i++) {
        if (arr[i] == target) return Some(i32, arr[i]);
    }
    return None(i32);
}

i32 main(void) {
    i32 arr[] = {1, 2, 3, 4, 5};
    Option_i32 result = find_value(arr, 5, 3);
    
    if (is_some(result)) {
        printf("Found: %d\n", unwrap(result));
    } else {
        printf("Not found\n");
    }
    
    // Or use unwrap_or for a default value
    i32 val = unwrap_or(result, -1);
    return 0;
}
```

#### Result Type

```c
#include <cyan/common.h>
#include <cyan/result.h>

RESULT_DEFINE(i32, const_charp);  // Define Result_i32_const_charp

Result_i32_const_charp parse_positive(const char *str) {
    i32 val = atoi(str);
    if (val <= 0) return Err(i32, const_charp, "must be positive");
    return Ok(i32, const_charp, val);
}

i32 main(void) {
    Result_i32_const_charp res = parse_positive("42");
    
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
#include <cyan/common.h>
#include <cyan/option.h>
#include <cyan/vector.h>

OPTION_DEFINE(i32);
VECTOR_DEFINE(i32);

i32 main(void) {
    Vec_i32 v = vec_i32_new();
    
    // Push elements
    vec_i32_push(&v, 10);
    vec_i32_push(&v, 20);
    vec_i32_push(&v, 30);
    
    // Access with bounds checking
    Option_i32 elem = vec_i32_get(&v, 1);
    if (is_some(elem)) {
        printf("Element at 1: %d\n", unwrap(elem));
    }
    
    // Pop elements
    while (is_some(elem = vec_i32_pop(&v))) {
        printf("Popped: %d\n", unwrap(elem));
    }
    
    vec_i32_free(&v);
    return 0;
}
```

#### Defer (Automatic Cleanup)

```c
#include <cyan/common.h>
#include <cyan/defer.h>

i32 main(void) {
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
#include <cyan/common.h>
#include <cyan/smartptr.h>

OPTION_DEFINE(i32);
UNIQUE_PTR_DEFINE(i32);
SHARED_PTR_DEFINE(i32);

i32 main(void) {
    // Unique pointer - exclusive ownership
    {
        unique_ptr(i32, p, 42);  // Auto-cleanup on scope exit
        printf("Value: %d\n", unique_i32_deref(&p));
    }  // Memory freed here
    
    // Shared pointer - reference counted
    SharedPtr_i32 s1 = shared_i32_new(100);
    SharedPtr_i32 s2 = shared_i32_clone(&s1);  // Count = 2
    
    printf("Count: %zu\n", shared_i32_count(&s1));  // Prints 2
    
    shared_i32_release(&s1);  // Count = 1
    shared_i32_release(&s2);  // Count = 0, memory freed
    
    return 0;
}
```

#### Pattern Matching

```c
#include <cyan/common.h>
#include <cyan/match.h>

OPTION_DEFINE(i32);
RESULT_DEFINE(i32, const_charp);

i32 main(void) {
    Option_i32 opt = Some(i32, 42);
    
    match_option(opt, i32, val,
        { printf("Got value: %d\n", val); },
        { printf("No value\n"); }
    );
    
    Result_i32_const_charp res = Ok(i32, const_charp, 100);
    
    match_result(res, i32, const_charp, val, e,
        { printf("Success: %d\n", val); },
        { printf("Error: %s\n", e); }
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

// Suppress warnings for unavailable platform-specific types (i128, f16, etc.)
#define CYAN_SUPPRESS_TYPE_WARNINGS

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

See the `examples/` directory for complete example programs:

| Example | Description |
|---------|-------------|
| `00_primitive_types.c` | Primitive type aliases (i32, u64, f32, etc.) |
| `01_option_basics.c` | Option type for nullable values |
| `02_result_error_handling.c` | Result type for error handling |
| `03_vector_collections.c` | Dynamic arrays with bounds checking |
| `04_defer_cleanup.c` | Automatic resource cleanup |
| `05_smart_pointers.c` | Unique and shared pointers |
| `06_pattern_matching.c` | Pattern matching on Option/Result |
| `07_functional.c` | Functional primitives (map, filter, reduce) |

Build and run examples:
```bash
cd examples
make
make run  # Run all examples
```

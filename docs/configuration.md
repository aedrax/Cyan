# Configuration

Configure the library by defining macros before including headers:

```c
// Custom panic handler
#define CYAN_PANIC(msg) my_panic_handler(msg)

// Custom allocator hooks (override all three together)
#define CYAN_MALLOC(size)        my_malloc(size)
#define CYAN_REALLOC(ptr, size)  my_realloc(ptr, size)
#define CYAN_FREE(ptr)           my_free(ptr)

// Suppress short lowercase names (is_some, unwrap, map, filter, try_ok, ...)
// The uppercase OPT_*/RES_* macros and the cyan_map/cyan_filter/cyan_reduce/
// cyan_foreach names remain available.
#define CYAN_NO_SHORT_NAMES

// Maximum S-expression nesting depth accepted by parse_sexp (default 1000)
#define CYAN_SEXP_MAX_DEPTH 1000

// Collection settings
#define CYAN_DEFAULT_CAPACITY 4   // Initial capacity for vectors, strings
#define CYAN_GROWTH_FACTOR 2      // Growth multiplier when resizing

// HashMap settings
#define CYAN_HASHMAP_INITIAL_CAPACITY 16
#define CYAN_HASHMAP_LOAD_FACTOR 70  // Resize at 70% full

// Coroutine stack size
#define CYAN_CORO_STACK_SIZE (64 * 1024)  // 64KB

// Enable thread-safe channels
#define CYAN_CHANNEL_THREADSAFE

// Opt in to #warning diagnostics when platform-specific types
// (i128, f16, f80, f128) are unavailable (silent by default)
#define CYAN_ENABLE_TYPE_WARNINGS

#include <cyan/cyan.h>
```

## Feature detection

Check for available features at compile time:

```c
#include <cyan/cyan.h>

// Library version (version macros live in common.h)
#if CYAN_VERSION_AT_LEAST(0, 2, 0)
    // Use features from v0.2.0+
#endif

// Platform-specific types
#if CYAN_HAS_INT128
    i128 big = ...;
#endif

#if CYAN_HAS_FLOAT128
    f128 precise = ...;
#endif

// Compiler features
#if CYAN_HAS_CLEANUP_ATTR
    // defer and smart pointers available
#endif

#if CYAN_HAS_GENERIC
    // _Generic-based macros available
#endif

#if CYAN_HAS_STMT_EXPR
    // Statement expressions available
#endif
```

# Panic handler

The panic handler is invoked for unrecoverable errors in the Cyan library.
When a panic occurs, the default behavior is to print diagnostic
information (file, line number, and error message) to stderr and then
abort the program.

```c
// Default panic output format:
// PANIC at filename.c:42: error message
```

The default `CYAN_PANIC` macro prints the file name, line number, and a
descriptive message before calling `abort()`.

## What triggers a panic

| Scenario | Description |
|----------|-------------|
| `unwrap()` on None | Attempting to extract a value from an empty Option |
| `unwrap_ok()` on Err | Attempting to extract a success value from an error Result |
| `unwrap_err()` on Ok | Attempting to extract an error value from a success Result |
| Memory allocation failure | When `malloc()` or `realloc()` returns NULL in collection operations |
| Resuming finished coroutine | Attempting to resume a coroutine that has already completed |

## Custom panic handler

Override the default behavior by defining `CYAN_PANIC` before including
any Cyan headers:

```c
// Define custom panic handler BEFORE including Cyan headers
#define CYAN_PANIC(msg) do { \
    fprintf(stderr, "[FATAL] %s:%d - %s\n", __FILE__, __LINE__, msg); \
    /* Add custom logging, cleanup, or crash reporting here */ \
    abort(); \
} while(0)

#include <cyan/cyan.h>

// Now all panics will use your custom handler
```

The custom handler must be defined before any Cyan header is included,
because the panic macro is checked with `#ifndef` and only defined if not
already present.

## API reference

| Macro | Description |
|-------|-------------|
| `CYAN_PANIC(msg)` | Trigger a panic with the given message. Prints file, line, and message to stderr, then calls `abort()`. Can be overridden by user. |
| `CYAN_PANIC_EXPR(msg, dummy)` | Helper for panics in expression contexts, where a value must be produced (e.g., ternary operators). The dummy value is returned only if the panic handler itself returns; for handlers that abort or longjmp it is unreachable. Overridable the same way as `CYAN_PANIC`. |

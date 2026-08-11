# Coroutines (experimental)

Stackful cooperative multitasking using POSIX ucontext. Defined in
`<cyan/coro.h>`.

> **EXPERIMENTAL:** the implementation relies on POSIX ucontext, which has
> been deprecated on macOS since 10.6 and does not exist on non-POSIX
> platforms (Windows). It works on current Linux and macOS toolchains, but
> the underlying primitive has no long-term platform guarantees. The API
> may change if the backend is replaced.

```c
#include <cyan/coro.h>

// Generator coroutine
void fibonacci(Coro *self, void *arg) {
    i32 a = 0, b = 1;
    for (i32 i = 0; i < 10; i++) {
        coro_yield_value(self, a);
        i32 next = a + b;
        a = b;
        b = next;
    }
}

// Coroutine with argument
void counter(Coro *self, void *arg) {
    i32 max = *(i32*)arg;
    for (i32 i = 0; i < max; i++) {
        printf("Count: %d\n", i);
        coro_yield(self);  // Yield without value
    }
}

i32 main(void) {
    // Create and run generator
    Coro *fib = coro_new(fibonacci, NULL, 0);  // 0 = default stack size
    
    printf("Fibonacci: ");
    while (coro_resume(fib)) {
        printf("%d ", coro_get_yield(fib, i32));
    }
    printf("\n");
    
    // Check status
    printf("Status: %s\n", coro_is_finished(fib) ? "finished" : "running");
    
    coro_free(fib);
    
    // Coroutine with argument
    i32 max = 5;
    Coro *cnt = coro_new(counter, &max, 0);
    while (coro_resume(cnt)) {
        // Process between yields
    }
    coro_free(cnt);
    
    return 0;
}
```

To run several coroutines against channels with the `coro_run` scheduler,
see [coroutine integration in Channels](channels.md#coroutine-integration-csp).

## API reference

| Function | Description |
|----------|-------------|
| `coro_new(fn, arg, stack_size)` | Create new coroutine (0 = default stack) |
| `coro_resume(c)` | Resume execution (returns true if yielded) |
| `coro_yield(c)` | Yield without value |
| `coro_yield_value(c, val)` | Yield with value |
| `coro_get_yield(c, T)` | Get yielded value (panics if the coroutine yielded without one; guard with `coro_has_yield`) |
| `coro_has_yield(c)` | Check if a yielded value is available |
| `coro_is_finished(c)` | Check if coroutine completed |
| `coro_status(c)` | Get current status |
| `coro_free(c)` | Free coroutine resources |
| `coro_run(coros, n)` | Round-robin scheduler; returns `false` on deadlock (see [Channels](channels.md#coroutine-integration-csp)) |
| `coro_current()` | The coroutine currently executing, or NULL outside one |
| `coro_mark_progress()` | Tell `coro_run` this pass made progress (for coroutines that yield without channel traffic) |

## Status values

- `CORO_CREATED` - Created but never resumed
- `CORO_RUNNING` - Currently executing
- `CORO_SUSPENDED` - Yielded, waiting to resume
- `CORO_FINISHED` - Completed execution

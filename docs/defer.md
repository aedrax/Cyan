# Defer

Scope-based resource cleanup using GCC/Clang's cleanup attribute. Defined
in `<cyan/defer.h>`.

```c
#include <cyan/defer.h>

i32 main(void) {
    // Basic defer - executes when scope exits
    FILE *f = fopen("test.txt", "r");
    if (!f) return 1;
    
    defer({ fclose(f); });
    
    // Multiple defers execute in LIFO order
    defer({ printf("First declared, last executed\n"); });
    defer({ printf("Last declared, first executed\n"); });
    
    // defer_free - convenience for freeing memory
    char *buf = malloc(1024);
    defer_free(buf);  // Automatically freed and set to NULL
    
    // defer_capture_int - capture value at declaration time
    i32 x = 10;
    defer_capture_int(x, { printf("Captured: %d\n", _captured_val); });
    x = 20;  // Change doesn't affect deferred code
    // Prints "Captured: 10" on scope exit
    
    // Works with all exit paths: return, break, continue
    for (i32 i = 0; i < 5; i++) {
        char *temp = malloc(100);
        defer_free(temp);
        
        if (i == 3) break;  // temp still freed!
    }
    
    return 0;  // All defers execute here
}
```

On Clang, `defer` blocks capture variables by value. A variable that the
deferred code needs to mutate must be declared with
`defer_var(type, name, init)` to behave the same on both GCC and Clang.

## API reference

| Macro | Description |
|-------|-------------|
| `defer({ code })` | Execute code block on scope exit |
| `defer_var(type, name, init)` | Declare a variable that deferred code can mutate on both GCC and Clang |
| `defer_free(ptr)` | Free pointer on scope exit (sets to NULL) |
| `defer_capture_int(val, { code })` | Defer with captured integer value |

# Option

Explicit nullable value handling that makes absence part of the type
instead of a convention like `NULL` or `-1`. Defined in `<cyan/option.h>`.

```c
#include <cyan/option.h>

OPTION_DEFINE(i32);  // Define Option_i32 type

i32 double_fn(i32 x) { return x * 2; }

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
    }
    
    // Use unwrap_or for a default value
    i32 val = unwrap_or(result, -1);
    
    // Transform with map_option
    Option_i32 doubled = map_option(result, i32, double_fn);
    return 0;
}
```

`Option_size_t` is pre-defined by `option.h`; do not `OPTION_DEFINE(size_t)`
in user code, or you'll get a redefinition error.

## Combinators

`ok_or` produces a Result, so it needs `result.h` and a `RESULT_DEFINE` for
the target type. The error type is token-pasted into the type name, so
pointer types need a typedef first:

```c
#include <cyan/result.h>

typedef const char *const_charp;
RESULT_DEFINE(i32, const_charp);  // Needed for ok_or

Option_i32 checked_half(i32 x) {  // Fallible transform for and_then
    return (x % 2 == 0) ? Some(i32, x / 2) : None(i32);
}
Option_i32 default_value(void) { return Some(i32, 0); }

Option_i32 half_plus_one(Option_i32 in) {
    i32 v = try_some(in);  // Rust-`?` style: returns None(i32) to the
                           // caller if `in` is empty
    return Some(i32, v / 2 + 1);
}

i32 main(void) {
    Option_i32 opt = Some(i32, 42);
    
    // Unwrap with a custom panic message
    i32 v = expect(opt, "expected a value");
    
    // Chain a fallible transformation (fn returns an Option)
    Option_i32 halved = and_then(opt, i32, checked_half);
    
    // Provide a fallback Option when empty
    Option_i32 with_fallback = or_else(halved, default_value);
    
    // Convert Option -> Result, supplying the error for None
    Result_i32_const_charp res = ok_or(opt, i32, const_charp, "was empty");
    
    return 0;
}
```

`try_some` requires the enclosing function to return the same `Option_T`
type (GCC/Clang only).

## API reference

| Function | Description |
|----------|-------------|
| `Some(T, val)` | Create Option containing a value |
| `None(T)` | Create empty Option |
| `is_some(opt)` | Check if Option has value |
| `is_none(opt)` | Check if Option is empty |
| `unwrap(opt)` | Extract value (panics if None) |
| `unwrap_or(opt, default)` | Extract value or return default |
| `expect(opt, msg)` | Extract value (panics with `msg` if None) |
| `map_option(opt, T_out, fn)` | Transform the contained value |
| `and_then(opt, T_out, fn)` | Chain a function returning `Option_T_out` |
| `or_else(opt, fn)` | Fall back to `fn()` (returns same Option type) if None |
| `ok_or(opt, T, E, err_val)` | Convert to `Result_T_E` (Err on None) |
| `try_some(opt)` | Extract value, or early-return the None (like Rust's `?`) |

`OPTION_DEFINE(T)` also generates standalone functions that take a pointer
and survive `CYAN_NO_SHORT_NAMES`: `option_T_is_some(&opt)`,
`option_T_is_none(&opt)`, `option_T_unwrap(&opt)`, and
`option_T_unwrap_or(&opt, def)`.

## Convenience macros

The short names above are default-on aliases for these uppercase macros,
which are always available (even with `CYAN_NO_SHORT_NAMES` defined). The
one exception is `OPT_TRY`, which, like `try_some`, is GCC/Clang only:

| Macro | Description |
|-------|-------------|
| `OPT_IS_SOME(opt)` | Check if Option has value |
| `OPT_IS_NONE(opt)` | Check if Option is empty |
| `OPT_UNWRAP(opt)` | Extract value (panics if None) |
| `OPT_UNWRAP_OR(opt, def)` | Extract value or return default |
| `OPT_EXPECT(opt, msg)` | Extract value (panics with `msg` if None) |
| `OPT_MAP(opt, T_out, fn)` | Transform the contained value |
| `OPT_AND_THEN(opt, T_out, fn)` | Chain a function returning `Option_T_out` |
| `OPT_OR_ELSE(opt, fn)` | Fall back to `fn()` if None |
| `OPT_OK_OR(opt, T, E, err_val)` | Convert to `Result_T_E` (Err on None) |
| `OPT_TRY(opt)` | Extract value, or early-return the None |

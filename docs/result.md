# Result

Explicit error handling without relying on errno or error codes.
`Result_T_E` carries either a success value of type `T` or an error of
type `E`. Defined in `<cyan/result.h>`.

```c
#include <cyan/result.h>

// The error type is token-pasted into the type name, so pointer types
// need a typedef first
typedef const char *const_charp;
RESULT_DEFINE(i32, const_charp);  // Define Result_i32_const_charp

i32 double_fn(i32 x) { return x * 2; }

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
    
    // Use unwrap_ok_or for default
    i32 val = unwrap_ok_or(res, 0);
    
    // Transform success value
    Result_i32_const_charp doubled = map_result(res, i32, const_charp, double_fn);
    return 0;
}
```

## Combinators

```c
Result_i32_const_charp checked_double(i32 x) {  // For and_then_result
    if (x > 1000000) return Err(i32, const_charp, "too large");
    return Ok(i32, const_charp, x * 2);
}

Result_i32_const_charp parse_and_double(const char *str) {
    // try_ok: Rust-`?` style early return. If parse_positive returns
    // an Err, that Err is returned to our caller immediately.
    i32 val = try_ok(parse_positive(str));
    return Ok(i32, const_charp, val * 2);
}

i32 main(void) {
    Result_i32_const_charp res = parse_positive("42");
    
    // Unwrap with a custom panic message
    i32 v = expect_ok(res, "expected a parsed value");
    
    // Chain a fallible transformation (fn returns a Result)
    Result_i32_const_charp chained = and_then_result(res, i32, const_charp, checked_double);
    
    return 0;
}
```

`try_ok` requires the enclosing function to return the same `Result_T_E`
type (GCC/Clang only).

## API reference

| Function | Description |
|----------|-------------|
| `Ok(T, E, val)` | Create Result with success value |
| `Err(T, E, err)` | Create Result with error value |
| `is_ok(res)` | Check if Result is success |
| `is_err(res)` | Check if Result is error |
| `unwrap_ok(res)` | Extract success value (panics if Err) |
| `unwrap_err(res)` | Extract error value (panics if Ok) |
| `unwrap_ok_or(res, default)` | Extract success or return default |
| `expect_ok(res, msg)` | Extract success value (panics with `msg` if Err) |
| `map_result(res, T_out, E, fn)` | Transform success value |
| `map_err(res, T, E_out, fn)` | Transform error value |
| `and_then_result(res, T_out, E, fn)` | Chain a function returning `Result_T_out_E` |
| `try_ok(res)` | Extract Ok value, or early-return the Err (like Rust's `?`) |

`RESULT_DEFINE(T, E)` also generates standalone functions that take a
pointer and survive `CYAN_NO_SHORT_NAMES`: `result_T_E_is_ok(&res)`,
`result_T_E_is_err(&res)`, `result_T_E_unwrap_ok(&res)`,
`result_T_E_unwrap_err(&res)`, and `result_T_E_unwrap_ok_or(&res, def)`.

## Convenience macros

The short names above are default-on aliases for these uppercase macros,
which are always available (even with `CYAN_NO_SHORT_NAMES` defined). The
one exception is `RES_TRY`, which, like `try_ok`, is GCC/Clang only:

| Macro | Description |
|-------|-------------|
| `RES_IS_OK(res)` | Check if Result is success |
| `RES_IS_ERR(res)` | Check if Result is error |
| `RES_UNWRAP_OK(res)` | Extract success value (panics if Err) |
| `RES_UNWRAP_ERR(res)` | Extract error value (panics if Ok) |
| `RES_UNWRAP_OK_OR(res, def)` | Extract success or return default |
| `RES_EXPECT_OK(res, msg)` | Extract success value (panics with `msg` if Err) |
| `RES_MAP(res, T_out, E, fn)` | Transform success value |
| `RES_MAP_ERR(res, T, E_out, fn)` | Transform error value |
| `RES_AND_THEN(res, T_out, E, fn)` | Chain a function returning `Result_T_out_E` |
| `RES_TRY(res)` | Extract Ok value, or early-return the Err |

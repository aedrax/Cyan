# Pattern matching

Ergonomic handling of [Option](option.md) and [Result](result.md) values,
in both statement and expression form. Defined in `<cyan/match.h>`.

```c
#include <cyan/match.h>

typedef const char *const_charp;

OPTION_DEFINE(i32);
RESULT_DEFINE(i32, const_charp);

i32 main(void) {
    // === Option Matching (statement form) ===
    Option_i32 opt = Some(i32, 42);
    
    match_option(opt, i32, val,
        { printf("Got value: %d\n", val); },
        { printf("No value\n"); }
    );
    
    // === Option Matching (expression form) ===
    i32 doubled = match_option_expr(opt, i32, i32, v, v * 2, 0);
    
    // === Result Matching (statement form) ===
    Result_i32_const_charp res = Ok(i32, const_charp, 100);
    
    match_result(res, i32, const_charp, val, e,
        { printf("Success: %d\n", val); },
        { printf("Error: %s\n", e); }
    );
    
    // === Result Matching (expression form) ===
    i32 value = match_result_expr(res, i32, const_charp, i32, v, e, v * 2, -1);
    
    return 0;
}
```

The expression forms (`match_option_expr`, `match_result_expr`) use GNU
statement expressions, so they are GCC/Clang only. The statement forms
deliberately avoid a `do/while(0)` wrapper, so `break` and `continue`
inside a branch apply to your enclosing loop or switch.

## API reference

| Macro | Description |
|-------|-------------|
| `match_option(opt, T, var, some_branch, none_branch)` | Match Option (statement) |
| `match_option_expr(opt, T, T_out, var, some_expr, none_expr)` | Match Option (expression) |
| `match_result(res, T, E, ok_var, err_var, ok_branch, err_branch)` | Match Result (statement) |
| `match_result_expr(res, T, E, T_out, ok_var, err_var, ok_expr, err_expr)` | Match Result (expression) |

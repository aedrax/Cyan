# Method-style macros

Every Cyan collection type pairs its generated functions with uppercase
convenience macros. The macros follow a single **type-first convention**:
the type parameter comes first, exactly as it does in constructors like
`Some(i32, 42)` or `Ok(i32, const_charp, val)`.

```c
VEC_PUSH(i32, v, 42);      // mirrors Some(i32, 42)
MAP_GET(i32, i32, m, key); // mirrors Ok/Err's (T, E, ...) ordering
```

The macros are zero-cost aliases: each one expands directly to a call to
the corresponding generated function (`VEC_PUSH(i32, v, 42)` becomes
`vec_i32_push(&v, 42)`), so there is no indirection and no runtime
overhead. On GNU-compatible compilers every argument is evaluated exactly
once; the non-GNU fallbacks for some `OPT_*`/`RES_*` accessors may
evaluate their argument more than once. Cyan types are
plain structs: an `Option_i32` is just a `bool` plus an `i32`, a `Vec_i32`
is just `data` + `len` + `cap`. There are no embedded function pointers or
vtables of any kind.

## Two ways to call operations

```c
#include <cyan/vector.h>

OPTION_DEFINE(i32);
VECTOR_DEFINE(i32);

i32 main(void) {
    Vec_i32 v = vec_i32_new();
    
    // 1. Standalone function (traditional)
    vec_i32_push(&v, 42);
    
    // 2. Convenience macro (type-first, expands to the call above)
    VEC_PUSH(i32, v, 42);
    
    // Both are equivalent!
    
    vec_i32_free(&v);
    return 0;
}
```

The same pattern applies to every container. For a `HashMap`, both type
parameters are passed:

```c
HashMap_i32_i32 m = hashmap_i32_i32_new();

hashmap_i32_i32_insert(&m, 1, 100);   // Function style
MAP_INSERT(i32, i32, m, 2, 200);      // Macro style

Option_i32 val = MAP_GET(i32, i32, m, 1);
MAP_FREE(i32, i32, m);
```

Monomorphic types (`String`) and typeless-accessor macros (`OPT_*`,
`RES_*`) take no type argument, since the member layout is the same for
every instantiation. See each type's page for its full macro table.

## Complete example

```c
#include <cyan/cyan.h>

OPTION_DEFINE(i32);
VECTOR_DEFINE(i32);
HASHMAP_DEFINE(i32, i32);

i32 main(void) {
    // Vector with convenience macros
    Vec_i32 nums = vec_i32_new();
    VEC_PUSH(i32, nums, 10);
    VEC_PUSH(i32, nums, 20);
    VEC_PUSH(i32, nums, 30);
    
    printf("Vector length: %zu\n", VEC_LEN(i32, nums));
    
    Option_i32 elem = VEC_GET(i32, nums, 1);
    if (OPT_IS_SOME(elem)) {
        printf("Element at 1: %d\n", OPT_UNWRAP(elem));
    }
    
    // HashMap with convenience macros
    HashMap_i32_i32 scores = hashmap_i32_i32_new();
    MAP_INSERT(i32, i32, scores, 1, 100);
    MAP_INSERT(i32, i32, scores, 2, 200);
    
    if (MAP_CONTAINS(i32, i32, scores, 1)) {
        Option_i32 score = MAP_GET(i32, i32, scores, 1);
        printf("Score for 1: %d\n", OPT_UNWRAP_OR(score, 0));
    }
    
    VEC_FREE(i32, nums);
    MAP_FREE(i32, i32, scores);
    return 0;
}
```

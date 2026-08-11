# Vector

Generic dynamic arrays with automatic growth and bounds-checked access.
Defined in `<cyan/vector.h>`.

```c
#include <cyan/vector.h>

OPTION_DEFINE(i32);   // Required for Option_i32
VECTOR_DEFINE(i32);   // Define Vec_i32 type

i32 main(void) {
    Vec_i32 v = vec_i32_new();
    
    // Push elements
    vec_i32_push(&v, 10);
    vec_i32_push(&v, 20);
    vec_i32_push(&v, 30);
    
    // Access with bounds checking (returns Option)
    Option_i32 elem = vec_i32_get(&v, 1);
    if (is_some(elem)) {
        printf("Element at 1: %d\n", unwrap(elem));
    }
    
    // Out of bounds returns None
    Option_i32 invalid = vec_i32_get(&v, 100);  // None
    
    // Pop elements (LIFO)
    while (is_some(elem = vec_i32_pop(&v))) {
        printf("Popped: %d\n", unwrap(elem));
    }
    
    // Pre-allocate capacity
    Vec_i32 preallocated = vec_i32_with_capacity(100);
    
    vec_i32_free(&v);
    vec_i32_free(&preallocated);
    return 0;
}
```

## Insert, remove, extend, reserve, clear

```c
Vec_i32 v = vec_i32_new();
vec_i32_reserve(&v, 16);          // Ensure capacity for at least 16 elements

i32 batch[] = {1, 2, 3, 4};
vec_i32_extend(&v, batch, 4);     // Append a whole array: {1, 2, 3, 4}

vec_i32_insert(&v, 1, 99);        // Shift-insert at index: {1, 99, 2, 3, 4}

Option_i32 removed = vec_i32_remove(&v, 1);  // Some(99), rest shifts down
                                             // Out-of-bounds index returns None

vec_i32_clear(&v);                // Length back to 0, capacity kept
vec_i32_free(&v);
```

## Search, sort, and iteration

```c
static bool over_9000(i32 x) { return x > 9000; }
static int cmp_i32(const void *a, const void *b) {
    return *(const i32 *)a - *(const i32 *)b;
}

Option_size_t at = vec_i32_find(&v, over_9000);   // index of first match
bool any        = vec_i32_contains(&v, over_9000);
vec_i32_sort(&v, cmp_i32);                        // qsort-style comparator

VEC_FOREACH(i32, v, it) {                         // it is an i32*
    printf("%d\n", *it);                          // break/continue work normally
}
```

| Macro | Description |
|-------|-------------|
| `VEC_FIND(T, v, pred)` | First index satisfying pred, as `Option_size_t` |
| `VEC_CONTAINS(T, v, pred)` | Whether any element satisfies pred |
| `VEC_SORT(T, v, cmp)` | In-place sort (qsort-style comparator) |
| `VEC_FOREACH(T, v, it)` | Pointer-iterator loop over the elements |

## API reference

| Function | Description |
|----------|-------------|
| `vec_T_new()` | Create empty vector |
| `vec_T_with_capacity(cap)` | Create vector with pre-allocated capacity |
| `vec_T_push(v, elem)` | Append element (auto-grows) |
| `vec_T_pop(v)` | Remove and return last element as Option |
| `vec_T_get(v, idx)` | Get element at index as Option |
| `vec_T_len(v)` | Get current length |
| `vec_T_insert(v, idx, elem)` | Insert element at index (shifts tail right) |
| `vec_T_remove(v, idx)` | Remove element at index as Option (shifts tail left) |
| `vec_T_extend(v, src, n)` | Append `n` elements from a C array |
| `vec_T_reserve(v, min_cap)` | Ensure capacity of at least `min_cap` |
| `vec_T_clear(v)` | Reset length to 0 (keeps capacity) |
| `vec_T_find(v, pred)` | First index satisfying pred, as `Option_size_t` |
| `vec_T_contains(v, pred)` | Whether any element satisfies pred |
| `vec_T_sort(v, cmp)` | In-place sort (qsort-style comparator) |
| `vec_T_free(v)` | Free vector memory |

## Convenience macros

Macros take the element type first, mirroring `Some(T, val)`:

| Macro | Description |
|-------|-------------|
| `VEC_PUSH(T, v, elem)` | Append element |
| `VEC_POP(T, v)` | Remove and return last element |
| `VEC_GET(T, v, idx)` | Get element at index |
| `VEC_LEN(T, v)` | Get current length |
| `VEC_INSERT(T, v, idx, elem)` | Insert element at index |
| `VEC_REMOVE(T, v, idx)` | Remove element at index as Option |
| `VEC_EXTEND(T, v, src, n)` | Append `n` elements from a C array |
| `VEC_RESERVE(T, v, cap)` | Ensure capacity of at least `cap` |
| `VEC_CLEAR(T, v)` | Reset length to 0 |
| `VEC_FREE(T, v)` | Free vector memory |

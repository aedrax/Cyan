# Slice

Non-owning views into contiguous sequences with bounds information.
Defined in `<cyan/slice.h>`.

```c
#include <cyan/slice.h>

OPTION_DEFINE(i32);
VECTOR_DEFINE(i32);
SLICE_DEFINE(i32);

i32 main(void) {
    // Create slice from C array
    i32 arr[] = {1, 2, 3, 4, 5};
    Slice_i32 s = slice_i32_from_array(arr, 5);
    
    // Bounds-checked access
    Option_i32 elem = slice_i32_get(s, 2);  // Some(3)
    
    // Create subslice
    Slice_i32 sub = slice_i32_subslice(s, 1, 4);  // {2, 3, 4}
    
    // Create slice from vector
    Vec_i32 v = vec_i32_new();
    vec_i32_push(&v, 10);
    Slice_i32 vs = slice_i32_from_vec(&v);
    
    printf("Length: %zu\n", slice_i32_len(s));
    
    vec_i32_free(&v);
    return 0;
}
```

## API reference

| Function | Description |
|----------|-------------|
| `slice_T_from_array(arr, len)` | Create slice from C array |
| `slice_T_from_vec(v)` | Create slice from vector |
| `slice_T_get(s, idx)` | Get element at index as Option |
| `slice_T_subslice(s, start, end)` | Create subslice view |
| `slice_T_len(s)` | Get slice length |

## Convenience macros

| Macro | Description |
|-------|-------------|
| `SLICE_GET(T, s, idx)` | Get element at index |
| `SLICE_SUBSLICE(T, s, start, end)` | Create subslice view |
| `SLICE_LEN(T, s)` | Get slice length |

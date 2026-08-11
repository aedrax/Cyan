# Functional

Higher-order functions for declarative data transformation. Defined in
`<cyan/functional.h>`.

The canonical names are `cyan_map`, `cyan_filter`, `cyan_reduce`, and
`cyan_foreach`; the short aliases `map`, `filter`, `reduce`, and `foreach`
are enabled by default and can be suppressed with
`#define CYAN_NO_SHORT_NAMES` (the `cyan_*` names remain available).

```c
#include <cyan/functional.h>

// Transformation functions
i32 square(i32 x) { return x * x; }
i32 double_it(i32 x) { return x * 2; }

// Predicate functions
bool is_even(i32 x) { return x % 2 == 0; }

// Accumulator functions
i32 add(i32 a, i32 b) { return a + b; }

// Side-effect function
void print_i32(i32 x) { printf("%d ", x); }

i32 main(void) {
    i32 numbers[] = {1, 2, 3, 4, 5};
    usize len = 5;
    
    // Map - transform each element
    i32 squared[5];
    map(numbers, len, squared, square);
    // squared = {1, 4, 9, 16, 25}
    
    // Filter - select matching elements
    i32 evens[5];
    usize evens_len;
    filter(numbers, len, evens, &evens_len, is_even);
    // evens = {2, 4}, evens_len = 2
    
    // Reduce - combine into single value
    i32 sum;
    reduce(sum, numbers, len, 0, add);
    // sum = 15
    
    // Foreach - execute side effect
    foreach(numbers, len, print_i32);
    // prints: 1 2 3 4 5
    
    // The canonical names work identically (and survive CYAN_NO_SHORT_NAMES)
    cyan_foreach(numbers, len, print_i32);
    
    return 0;
}
```

## Vector-specific operations

```c
OPTION_DEFINE(i32);
OPTION_DEFINE(f64);
VECTOR_DEFINE(i32);
VECTOR_DEFINE(f64);
VEC_MAP_DEFINE(i32, f64);     // vec_map_i32_to_f64
VEC_FILTER_DEFINE(i32);        // vec_filter_i32
VEC_REDUCE_DEFINE(i32, i32);   // vec_reduce_i32_to_i32
VEC_FOREACH_DEFINE(i32);       // vec_foreach_i32

f64 to_double(i32 x) { return (f64)x; }

Vec_i32 v = vec_i32_new();
// ... push elements ...

Vec_f64 doubles = vec_map_i32_to_f64(&v, to_double);
Vec_i32 filtered = vec_filter_i32(&v, is_even);
i32 total = vec_reduce_i32_to_i32(&v, 0, add);
vec_foreach_i32(&v, print_i32);
```

## API reference

| Macro | Short alias | Description |
|-------|-------------|-------------|
| `cyan_map(arr, len, out, fn)` | `map` | Transform each element |
| `cyan_filter(arr, len, out, out_len, pred)` | `filter` | Select elements matching predicate |
| `cyan_reduce(result, arr, len, init, acc_fn)` | `reduce` | Combine elements into single value |
| `cyan_foreach(arr, len, fn)` | `foreach` | Execute function on each element |

| Macro | Description |
|-------|-------------|
| `VEC_MAP_DEFINE(T_in, T_out)` | Generate vector map function |
| `VEC_FILTER_DEFINE(T)` | Generate vector filter function |
| `VEC_REDUCE_DEFINE(T, R)` | Generate vector reduce function |
| `VEC_FOREACH_DEFINE(T)` | Generate vector foreach function |

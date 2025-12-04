# Cyan - Modern C11 Header Library
> Pronounced "See-yan" because I'm a monster

A header-only C11 library that brings modern programming paradigms to C, including Option/Result types, functional primitives, smart pointers, coroutines, channels, and more.

## Features

| Feature | Description |
|---------|-------------|
| **Primitive Type Aliases** | Concise, predictable-size type names (i32, u64, f32, etc.) |
| **Option Type** | Explicit nullable value handling |
| **Result Type** | Explicit error handling without errno |
| **Vector** | Generic dynamic arrays with bounds checking |
| **Slice** | Safe array views with bounds information |
| **HashMap** | Type-safe hash maps with O(1) lookups |
| **String** | Dynamic strings with safe operations |
| **Functional Primitives** | map, filter, reduce, foreach |
| **Smart Pointers** | Unique and shared pointers with automatic cleanup |
| **Defer** | Scope-based resource cleanup (RAII-style) |
| **Coroutines** | Stackful cooperative multitasking |
| **Channels** | CSP-style communication primitives |
| **Pattern Matching** | Ergonomic Option/Result handling |
| **Serialization** | Text-based data serialization with S-expression format |

## Quick Start

### Installation

Copy the `include/cyan/` directory to your project and include the headers:

```c
#include <cyan/cyan.h>  // Include everything
// Or include individual headers:
#include <cyan/option.h>
#include <cyan/vector.h>
```

---

## Primitive Type Aliases

Concise type names with predictable sizes, inspired by Rust and Zig.

```c
#include <cyan/common.h>

// Fixed-width signed integers
i8  a = 127;           // int8_t
i16 b = 32767;         // int16_t
i32 c = 2147483647;    // int32_t
i64 d = 9223372036854775807LL;  // int64_t

// Fixed-width unsigned integers
u8  e = 255;           // uint8_t
u16 f = 65535;         // uint16_t
u32 g = 4294967295U;   // uint32_t
u64 h = 18446744073709551615ULL;  // uint64_t

// Pointer-sized integers
usize len = sizeof(array) / sizeof(array[0]);  // size_t compatible
isize offset = -100;   // signed pointer-sized

// Floating-point
f32 pi_f = 3.14159f;   // float
f64 pi_d = 3.14159265358979;  // double

// Type-erased pointer
any* generic_ptr = &some_data;
```

**Available Types:**

| Category | Types |
|----------|-------|
| Signed integers | `i8`, `i16`, `i32`, `i64`, `i128`* |
| Unsigned integers | `u8`, `u16`, `u32`, `u64`, `u128`* |
| Pointer-sized | `isize`, `usize` |
| Floating-point | `f16`*, `f32`, `f64`, `f80`*, `f128`* |
| Special | `bool`, `any` |

*Platform-dependent. Check `CYAN_HAS_INT128`, `CYAN_HAS_FLOAT16`, `CYAN_HAS_FLOAT80`, `CYAN_HAS_FLOAT128` macros.

---

## Panic Handler

The panic handler is invoked for unrecoverable errors in the Cyan library. When a panic occurs, the default behavior is to print diagnostic information (file, line number, and error message) to stderr and then abort the program.

**Default Behavior:**

```c
// Default panic output format:
// PANIC at filename.c:42: error message
```

The default `CYAN_PANIC` macro prints the file name, line number, and a descriptive message before calling `abort()`. This provides clear debugging information when something goes wrong.

### Panic Trigger Scenarios

Panics are triggered in the following situations:

| Scenario | Description |
|----------|-------------|
| `unwrap()` on None | Attempting to extract a value from an empty Option |
| `unwrap_ok()` on Err | Attempting to extract a success value from an error Result |
| `unwrap_err()` on Ok | Attempting to extract an error value from a success Result |
| Memory allocation failure | When `malloc()` or `realloc()` returns NULL in collection operations |
| Resuming finished coroutine | Attempting to resume a coroutine that has already completed |

### Custom Panic Handler

You can override the default panic behavior by defining `CYAN_PANIC` before including any Cyan headers:

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

**Important:** The custom handler must be defined before any Cyan header is included, as the panic macro is checked with `#ifndef` and only defined if not already present.

### Panic API

| Macro | Description |
|-------|-------------|
| `CYAN_PANIC(msg)` | Trigger a panic with the given message. Prints file, line, and message to stderr, then calls `abort()`. Can be overridden by user. |
| `CYAN_PANIC_EXPR(msg, dummy)` | Internal helper for panics in expression contexts. Used where a value must be returned (e.g., ternary operators). The dummy value satisfies type requirements but is never returned. |

---

## Option Type

Explicit nullable value handling that makes absence explicit in code.

```c
#include <cyan/option.h>

OPTION_DEFINE(i32);  // Define Option_i32 type

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

**Option API:**

| Function | Description |
|----------|-------------|
| `Some(T, val)` | Create Option containing a value |
| `None(T)` | Create empty Option |
| `is_some(opt)` | Check if Option has value |
| `is_none(opt)` | Check if Option is empty |
| `unwrap(opt)` | Extract value (panics if None) |
| `unwrap_or(opt, default)` | Extract value or return default |
| `map_option(opt, T_out, fn)` | Transform the contained value |

---

## Result Type

Explicit error handling without relying on errno or error codes.

```c
#include <cyan/result.h>

RESULT_DEFINE(i32, const_charp);  // Define Result_i32_const_charp

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

**Result API:**

| Function | Description |
|----------|-------------|
| `Ok(T, E, val)` | Create Result with success value |
| `Err(T, E, err)` | Create Result with error value |
| `is_ok(res)` | Check if Result is success |
| `is_err(res)` | Check if Result is error |
| `unwrap_ok(res)` | Extract success value (panics if Err) |
| `unwrap_err(res)` | Extract error value (panics if Ok) |
| `unwrap_ok_or(res, default)` | Extract success or return default |
| `map_result(res, T_out, E, fn)` | Transform success value |
| `map_err(res, T, E_out, fn)` | Transform error value |

---

## Vector (Dynamic Array)

Generic dynamic arrays with automatic growth and bounds-checked access.

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

**Vector API:**

| Function | Description |
|----------|-------------|
| `vec_T_new()` | Create empty vector |
| `vec_T_with_capacity(cap)` | Create vector with pre-allocated capacity |
| `vec_T_push(v, elem)` | Append element (auto-grows) |
| `vec_T_pop(v)` | Remove and return last element as Option |
| `vec_T_get(v, idx)` | Get element at index as Option |
| `vec_T_len(v)` | Get current length |
| `vec_T_free(v)` | Free vector memory |

---

## Slice (Array Views)

Non-owning views into contiguous sequences with bounds information.

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

**Slice API:**

| Function | Description |
|----------|-------------|
| `slice_T_from_array(arr, len)` | Create slice from C array |
| `slice_T_from_vec(v)` | Create slice from vector |
| `slice_T_get(s, idx)` | Get element at index as Option |
| `slice_T_subslice(s, start, end)` | Create subslice view |
| `slice_T_len(s)` | Get slice length |

---

## HashMap

Type-safe hash maps with O(1) average lookups using FNV-1a hashing.

```c
#include <cyan/hashmap.h>

OPTION_DEFINE(i32);
HASHMAP_DEFINE(i32, i32);      // HashMap_i32_i32
HASHMAP_ITER_DEFINE(i32, i32); // Iterator support

i32 main(void) {
    HashMap_i32_i32 m = hashmap_i32_i32_new();
    
    // Insert key-value pairs
    hashmap_i32_i32_insert(&m, 1, 100);
    hashmap_i32_i32_insert(&m, 2, 200);
    hashmap_i32_i32_insert(&m, 3, 300);
    
    // Lookup (returns Option)
    Option_i32 val = hashmap_i32_i32_get(&m, 2);
    if (is_some(val)) {
        printf("Key 2 -> %d\n", unwrap(val));
    }
    
    // Check existence
    if (hashmap_i32_i32_contains(&m, 1)) {
        printf("Key 1 exists\n");
    }
    
    // Remove entry
    Option_i32 removed = hashmap_i32_i32_remove(&m, 1);
    
    // Iterate over entries
    HashMapIter_i32_i32 it = hashmap_i32_i32_iter(&m);
    Option_MapPair_i32_i32 pair;
    while ((pair = hashmap_i32_i32_iter_next(&it)).has_value) {
        printf("%d -> %d\n", pair.value.key, pair.value.value);
    }
    
    printf("Size: %zu\n", hashmap_i32_i32_len(&m));
    
    hashmap_i32_i32_free(&m);
    return 0;
}
```

**HashMap API:**

| Function | Description |
|----------|-------------|
| `hashmap_K_V_new()` | Create empty map |
| `hashmap_K_V_with_capacity(cap)` | Create map with initial capacity |
| `hashmap_K_V_insert(m, key, value)` | Insert or update entry |
| `hashmap_K_V_get(m, key)` | Get value as Option |
| `hashmap_K_V_contains(m, key)` | Check if key exists |
| `hashmap_K_V_remove(m, key)` | Remove entry, return value as Option |
| `hashmap_K_V_len(m)` | Get number of entries |
| `hashmap_K_V_iter(m)` | Create iterator |
| `hashmap_K_V_iter_next(it)` | Get next key-value pair |
| `hashmap_K_V_free(m)` | Free map memory |

---

## String (Dynamic Strings)

Heap-allocated, growable strings with safe operations.

```c
#include <cyan/string.h>

i32 main(void) {
    // Create strings
    String s = string_from("Hello");
    String empty = string_new();
    String preallocated = string_with_capacity(100);
    
    // Append content
    string_append(&s, " World");
    string_push(&s, '!');
    
    // Append another String
    String suffix = string_from(" - Cyan");
    string_append_str(&s, &suffix);
    
    // Get C string for printing
    printf("%s\n", string_cstr(&s));  // "Hello World! - Cyan"
    printf("Length: %zu\n", string_len(&s));
    
    // Character access with bounds checking
    Option_char ch = string_get(&s, 0);  // Some('H')
    
    // Slicing
    Slice_char slice = string_slice(&s, 0, 5);  // "Hello"
    
    // Formatting
    String formatted = string_formatted("Value: %d, Pi: %.2f", 42, 3.14);
    
    // Concatenation
    String a = string_from("Hello");
    String b = string_from(" World");
    String combined = string_concat(&a, &b);
    
    // Auto-cleanup (GCC/Clang)
    string_auto(auto_str, string_from("Auto cleanup!"));
    // auto_str freed automatically at scope exit
    
    string_free(&s);
    string_free(&suffix);
    string_free(&formatted);
    string_free(&a);
    string_free(&b);
    string_free(&combined);
    return 0;
}
```

**String API:**

| Function | Description |
|----------|-------------|
| `string_new()` | Create empty string |
| `string_from(cstr)` | Create from C string |
| `string_with_capacity(cap)` | Create with pre-allocated capacity |
| `string_push(s, char)` | Append single character |
| `string_append(s, cstr)` | Append C string |
| `string_append_str(s, other)` | Append another String |
| `string_clear(s)` | Clear content (keeps capacity) |
| `string_format(s, fmt, ...)` | Append formatted content |
| `string_formatted(fmt, ...)` | Create new formatted string |
| `string_cstr(s)` | Get null-terminated C string |
| `string_len(s)` | Get length |
| `string_get(s, idx)` | Get character as Option |
| `string_slice(s, start, end)` | Create slice view |
| `string_concat(a, b)` | Concatenate two strings |
| `string_free(s)` | Free string memory |
| `string_auto(name, init)` | Declare with auto-cleanup |

---

## Functional Primitives

Higher-order functions for declarative data transformation.

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
    
    return 0;
}
```

**Vector-specific functional operations:**

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

**Functional API:**

| Macro | Description |
|-------|-------------|
| `map(arr, len, out, fn)` | Transform each element |
| `filter(arr, len, out, out_len, pred)` | Select elements matching predicate |
| `reduce(result, arr, len, init, acc_fn)` | Combine elements into single value |
| `foreach(arr, len, fn)` | Execute function on each element |
| `VEC_MAP_DEFINE(T_in, T_out)` | Generate vector map function |
| `VEC_FILTER_DEFINE(T)` | Generate vector filter function |
| `VEC_REDUCE_DEFINE(T, R)` | Generate vector reduce function |
| `VEC_FOREACH_DEFINE(T)` | Generate vector foreach function |

---

## Defer (Automatic Cleanup)

Scope-based resource cleanup using GCC/Clang's cleanup attribute.

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

**Defer API:**

| Macro | Description |
|-------|-------------|
| `defer({ code })` | Execute code block on scope exit |
| `defer_free(ptr)` | Free pointer on scope exit (sets to NULL) |
| `defer_capture_int(val, { code })` | Defer with captured integer value |

---

## Smart Pointers

Automatic memory management with unique and shared ownership semantics.

```c
#include <cyan/smartptr.h>

OPTION_DEFINE(i32);
UNIQUE_PTR_DEFINE(i32);
SHARED_PTR_DEFINE(i32);

// Custom destructor
void cleanup_resource(void *ptr) {
    printf("Cleaning up: %d\n", *(i32*)ptr);
}

i32 main(void) {
    // === Unique Pointer (exclusive ownership) ===
    {
        unique_ptr(i32, p, 42);  // Auto-cleanup on scope exit
        printf("Value: %d\n", unique_i32_deref(&p));
        
        // Get raw pointer (doesn't transfer ownership)
        i32 *raw = unique_i32_get(&p);
        
        // Move ownership
        UniquePtr_i32 moved = unique_i32_move(&p);
        // p is now NULL, moved owns the memory
        
        unique_i32_free(&moved);
    }  // p would be freed here if not moved
    
    // With custom destructor
    unique_ptr_with_dtor(i32, p2, 100, cleanup_resource);
    
    // === Shared Pointer (reference counted) ===
    SharedPtr_i32 s1 = shared_i32_new(100);
    printf("Count: %zu\n", shared_i32_count(&s1));  // 1
    
    SharedPtr_i32 s2 = shared_i32_clone(&s1);
    printf("Count: %zu\n", shared_i32_count(&s1));  // 2
    
    printf("Value: %d\n", shared_i32_deref(&s1));
    
    shared_i32_release(&s1);  // Count = 1
    shared_i32_release(&s2);  // Count = 0, memory freed

    // === Weak Pointer (non-owning reference) ===
    SharedPtr_i32 owner = shared_i32_new(200);
    WeakPtr_i32 weak = weak_i32_from_shared(&owner);
    
    // Check if target still exists (standalone function)
    if (!weak_i32_is_expired(&weak)) {
        // Upgrade to shared pointer
        Option_SharedPtr_i32 upgraded = weak_i32_upgrade(&weak);
        if (upgraded.has_value) {
            printf("Upgraded: %d\n", shared_i32_deref(&upgraded.value));
            shared_i32_release(&upgraded.value);
        }
    }
    
    // Vtable method calls (equivalent to standalone functions)
    if (!weak.vt->wptr_is_expired(&weak)) {
        Option_SharedPtr_i32 upgraded = weak.vt->wptr_upgrade(&weak);
        if (upgraded.has_value) {
            printf("Upgraded via vtable: %d\n", shared_i32_deref(&upgraded.value));
            shared_i32_release(&upgraded.value);
        }
    }
    
    // Convenience macros (concise vtable access)
    if (!WPTR_IS_EXPIRED(weak)) {
        Option_SharedPtr_i32 upgraded = WPTR_UPGRADE(weak);
        if (upgraded.has_value) {
            printf("Upgraded via macro: %d\n", shared_i32_deref(&upgraded.value));
            shared_i32_release(&upgraded.value);
        }
    }
    
    shared_i32_release(&owner);  // Memory freed
    // weak_i32_is_expired(&weak) now returns true
    
    WPTR_RELEASE(weak);  // Release weak reference (via convenience macro)
    return 0;
}
```

**Smart Pointer API:**

| Function | Description |
|----------|-------------|
| `unique_ptr(T, name, value)` | Declare unique pointer with auto-cleanup |
| `unique_T_new(value)` | Create unique pointer |
| `unique_T_deref(u)` | Dereference |
| `unique_T_get(u)` | Get raw pointer |
| `unique_T_move(u)` | Transfer ownership |
| `unique_T_free(u)` | Free memory |
| `shared_ptr(T, name, value)` | Declare shared pointer with auto-cleanup |
| `shared_T_new(value)` | Create shared pointer |
| `shared_T_clone(s)` | Clone (increment ref count) |
| `shared_T_deref(s)` | Dereference |
| `shared_T_count(s)` | Get reference count |
| `shared_T_release(s)` | Release (decrement ref count) |
| `weak_T_from_shared(s)` | Create weak reference |
| `weak_T_is_expired(w)` | Check if target freed |
| `weak_T_upgrade(w)` | Upgrade to shared pointer |
| `weak_T_release(w)` | Release weak reference |
| `w.vt->wptr_is_expired(&w)` | Check if target freed (vtable) |
| `w.vt->wptr_upgrade(&w)` | Upgrade to shared pointer (vtable) |
| `w.vt->wptr_release(&w)` | Release weak reference (vtable) |
| `WPTR_IS_EXPIRED(w)` | Check if target freed (macro) |
| `WPTR_UPGRADE(w)` | Upgrade to shared pointer (macro) |
| `WPTR_RELEASE(w)` | Release weak reference (macro) |

---

## Pattern Matching

Ergonomic handling of Option and Result types.

```c
#include <cyan/match.h>

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

**Pattern Matching API:**

| Macro | Description |
|-------|-------------|
| `match_option(opt, T, var, some_branch, none_branch)` | Match Option (statement) |
| `match_option_expr(opt, T, T_out, var, some_expr, none_expr)` | Match Option (expression) |
| `match_result(res, T, E, ok_var, err_var, ok_branch, err_branch)` | Match Result (statement) |
| `match_result_expr(res, T, E, T_out, ok_var, err_var, ok_expr, err_expr)` | Match Result (expression) |

---

## Coroutines

Stackful cooperative multitasking using POSIX ucontext.

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

**Coroutine API:**

| Function | Description |
|----------|-------------|
| `coro_new(fn, arg, stack_size)` | Create new coroutine (0 = default stack) |
| `coro_resume(c)` | Resume execution (returns true if yielded) |
| `coro_yield(c)` | Yield without value |
| `coro_yield_value(c, val)` | Yield with value |
| `coro_get_yield(c, T)` | Get yielded value |
| `coro_is_finished(c)` | Check if coroutine completed |
| `coro_status(c)` | Get current status |
| `coro_free(c)` | Free coroutine resources |

**Coroutine Status:**
- `CORO_CREATED` - Created but never resumed
- `CORO_RUNNING` - Currently executing
- `CORO_SUSPENDED` - Yielded, waiting to resume
- `CORO_FINISHED` - Completed execution

---

## Channels

CSP-style communication primitives for message passing.

```c
#include <cyan/channel.h>

CHANNEL_DEFINE(i32);  // Define Channel_i32

i32 main(void) {
    // Create buffered channel with capacity 10
    Channel_i32 *ch = chan_i32_new(10);
    
    // Send values
    chan_i32_send(ch, 1);
    chan_i32_send(ch, 2);
    chan_i32_send(ch, 3);
    
    // Receive values (returns Option)
    Option_i32 val = chan_i32_recv(ch);
    if (is_some(val)) {
        printf("Received: %d\n", unwrap(val));
    }
    
    // Non-blocking operations
    ChanStatus status = chan_i32_try_send(ch, 42);
    if (status == CHAN_OK) {
        printf("Sent successfully\n");
    } else if (status == CHAN_WOULD_BLOCK) {
        printf("Channel full\n");
    }
    
    Option_i32 maybe = chan_i32_try_recv(ch);
    
    // Close channel (no more sends allowed)
    chan_i32_close(ch);
    
    // Drain remaining values
    while (is_some(val = chan_i32_recv(ch))) {
        printf("Drained: %d\n", unwrap(val));
    }
    
    // Check if closed
    if (chan_i32_is_closed(ch)) {
        printf("Channel is closed\n");
    }
    
    chan_i32_free(ch);
    return 0;
}
```

**Thread-Safe Channels:**

```c
// Enable thread safety before including
#define CYAN_CHANNEL_THREADSAFE
#include <cyan/channel.h>

// Now channels use pthread mutexes and condition variables
// for safe concurrent access from multiple threads
```

**Channel API:**

| Function | Description |
|----------|-------------|
| `chan_T_new(capacity)` | Create channel (0 = unbuffered) |
| `chan_T_send(ch, value)` | Send value (blocks if full) |
| `chan_T_recv(ch)` | Receive value as Option (blocks if empty) |
| `chan_T_try_send(ch, value)` | Non-blocking send |
| `chan_T_try_recv(ch)` | Non-blocking receive |
| `chan_T_close(ch)` | Close channel |
| `chan_T_is_closed(ch)` | Check if closed |
| `chan_T_free(ch)` | Free channel |

**Channel Status:**
- `CHAN_OK` - Operation succeeded
- `CHAN_CLOSED` - Channel is closed
- `CHAN_WOULD_BLOCK` - Operation would block (try_* variants)

---

## Vtable Method-Style API

All Cyan collection types support a method-style API through vtables (virtual method tables). This provides an object-oriented feel while maintaining C's efficiency.

### How It Works

Each type instance contains a pointer to a shared static vtable. All instances of the same type share the same vtable, so the memory overhead is just one pointer per instance.

```
┌─────────────────────────────────────────────────────────┐
│                    Static Memory                         │
│  ┌─────────────────┐                                    │
│  │ _vec_i32_vt     │ ◄── Single vtable per type         │
│  │ (static const)  │                                    │
│  └────────┬────────┘                                    │
│           │                                              │
└───────────┼─────────────────────────────────────────────┘
            │
            ▼
┌───────────────────────────────────────────────────────────┐
│                    Heap/Stack Memory                       │
│  ┌──────────┐   ┌──────────┐   ┌──────────┐              │
│  │ Vec_i32  │   │ Vec_i32  │   │ Vec_i32  │              │
│  │ v1.vt ───┼───┼──────────┼───┼──────────┼──► shared    │
│  └──────────┘   └──────────┘   └──────────┘              │
└───────────────────────────────────────────────────────────┘
```

### Three Ways to Call Operations

```c
#include <cyan/vector.h>

OPTION_DEFINE(i32);
VECTOR_DEFINE(i32);

i32 main(void) {
    Vec_i32 v = vec_i32_new();
    
    // 1. Standalone function (traditional)
    vec_i32_push(&v, 42);
    
    // 2. Vtable method call (OOP-style)
    v.vt->push(&v, 42);
    
    // 3. Convenience macro (concise)
    VEC_PUSH(v, 42);
    
    // All three are equivalent!
    
    vec_i32_free(&v);
    return 0;
}
```

### Vector Vtable

```c
// Vtable method calls
v.vt->push(&v, elem);      // Append element
v.vt->pop(&v);             // Remove and return last element
v.vt->get(&v, idx);        // Get element at index
v.vt->len(&v);             // Get length
v.vt->free(&v);            // Free memory

// Convenience macros
VEC_PUSH(v, elem)          // v.vt->push(&v, elem)
VEC_POP(v)                 // v.vt->pop(&v)
VEC_GET(v, idx)            // v.vt->get(&v, idx)
VEC_LEN(v)                 // v.vt->len(&v)
VEC_FREE(v)                // v.vt->free(&v)
```

### HashMap Vtable

```c
// Vtable method calls
m.vt->insert(&m, key, val);  // Insert key-value pair
m.vt->get(&m, key);          // Get value by key
m.vt->contains(&m, key);     // Check if key exists
m.vt->remove(&m, key);       // Remove and return value
m.vt->len(&m);               // Get number of entries
m.vt->free(&m);              // Free memory

// Convenience macros
MAP_INSERT(m, k, v)        // m.vt->insert(&m, k, v)
MAP_GET(m, k)              // m.vt->get(&m, k)
MAP_CONTAINS(m, k)         // m.vt->contains(&m, k)
MAP_REMOVE(m, k)           // m.vt->remove(&m, k)
MAP_LEN(m)                 // m.vt->len(&m)
MAP_FREE(m)                // m.vt->free(&m)
```

### Slice Vtable

```c
// Vtable method calls
s.vt->get(s, idx);           // Get element at index
s.vt->subslice(s, start, end); // Create subslice
s.vt->len(s);                // Get length

// Convenience macros
SLICE_GET(s, idx)          // s.vt->get(s, idx)
SLICE_SUBSLICE(s, st, end) // s.vt->subslice(s, st, end)
SLICE_LEN(s)               // s.vt->len(s)
```

### String Vtable

```c
// Vtable method calls
s.vt->push(&s, c);           // Append character
s.vt->append(&s, cstr);      // Append C string
s.vt->clear(&s);             // Clear content
s.vt->get(&s, idx);          // Get character at index
s.vt->len(&s);               // Get length
s.vt->cstr(&s);              // Get C string
s.vt->slice(&s, start, end); // Create slice
s.vt->free(&s);              // Free memory

// Convenience macros
STR_PUSH(s, c)             // s.vt->push(&s, c)
STR_APPEND(s, cstr)        // s.vt->append(&s, cstr)
STR_CLEAR(s)               // s.vt->clear(&s)
STR_GET(s, idx)            // s.vt->get(&s, idx)
STR_LEN(s)                 // s.vt->len(&s)
STR_CSTR(s)                // s.vt->cstr(&s)
STR_SLICE(s, st, end)      // s.vt->slice(&s, st, end)
STR_FREE(s)                // s.vt->free(&s)
```

### Option Vtable

```c
// Vtable method calls
opt.vt->is_some(&opt);       // Check if has value
opt.vt->is_none(&opt);       // Check if empty
opt.vt->unwrap(&opt);        // Extract value (panics if None)
opt.vt->unwrap_or(&opt, def); // Extract value or default

// Convenience macros
OPT_IS_SOME(opt)           // opt.vt->is_some(&opt)
OPT_IS_NONE(opt)           // opt.vt->is_none(&opt)
OPT_UNWRAP(opt)            // opt.vt->unwrap(&opt)
OPT_UNWRAP_OR(opt, def)    // opt.vt->unwrap_or(&opt, def)
```

### Result Vtable

```c
// Vtable method calls
res.vt->is_ok(&res);         // Check if success
res.vt->is_err(&res);        // Check if error
res.vt->unwrap_ok(&res);     // Extract success value
res.vt->unwrap_err(&res);    // Extract error value
res.vt->unwrap_ok_or(&res, def); // Extract success or default

// Convenience macros
RES_IS_OK(res)             // res.vt->is_ok(&res)
RES_IS_ERR(res)            // res.vt->is_err(&res)
RES_UNWRAP_OK(res)         // res.vt->unwrap_ok(&res)
RES_UNWRAP_ERR(res)        // res.vt->unwrap_err(&res)
RES_UNWRAP_OK_OR(res, def) // res.vt->unwrap_ok_or(&res, def)
```

### UniquePtr Vtable

```c
// Vtable method calls
u.vt->get(&u);               // Get raw pointer
u.vt->deref(&u);             // Dereference
u.vt->move(&u);              // Transfer ownership
u.vt->free(&u);              // Free memory

// Convenience macros
UPTR_GET(u)                // u.vt->get(&u)
UPTR_DEREF(u)              // u.vt->deref(&u)
UPTR_MOVE(u)               // u.vt->move(&u)
UPTR_FREE(u)               // u.vt->free(&u)
```

### SharedPtr Vtable

```c
// Vtable method calls
s.vt->get(&s);               // Get raw pointer
s.vt->deref(&s);             // Dereference
s.vt->clone(&s);             // Clone (increment ref count)
s.vt->count(&s);             // Get reference count
s.vt->release(&s);           // Release (decrement ref count)

// Convenience macros
SPTR_GET(s)                // s.vt->get(&s)
SPTR_DEREF(s)              // s.vt->deref(&s)
SPTR_CLONE(s)              // s.vt->clone(&s)
SPTR_COUNT(s)              // s.vt->count(&s)
SPTR_RELEASE(s)            // s.vt->release(&s)
```

### WeakPtr Vtable

```c
// Vtable method calls
w.vt->wptr_is_expired(&w);   // Check if target has been freed
w.vt->wptr_upgrade(&w);      // Upgrade to SharedPtr (returns Option)
w.vt->wptr_release(&w);      // Release weak reference

// Convenience macros
WPTR_IS_EXPIRED(w)         // w.vt->wptr_is_expired(&w)
WPTR_UPGRADE(w)            // w.vt->wptr_upgrade(&w)
WPTR_RELEASE(w)            // w.vt->wptr_release(&w)
```

### Channel Vtable

```c
// Vtable method calls (channel is pointer-based)
ch->vt->send(ch, val);       // Send value
ch->vt->recv(ch);            // Receive value
ch->vt->try_send(ch, val);   // Non-blocking send
ch->vt->try_recv(ch);        // Non-blocking receive
ch->vt->close(ch);           // Close channel
ch->vt->is_closed(ch);       // Check if closed
ch->vt->free(ch);            // Free channel

// Convenience macros
CHAN_SEND(ch, val)         // ch->vt->send(ch, val)
CHAN_RECV(ch)              // ch->vt->recv(ch)
CHAN_TRY_SEND(ch, val)     // ch->vt->try_send(ch, val)
CHAN_TRY_RECV(ch)          // ch->vt->try_recv(ch)
CHAN_CLOSE(ch)             // ch->vt->close(ch)
CHAN_IS_CLOSED(ch)         // ch->vt->is_closed(ch)
CHAN_FREE(ch)              // ch->vt->free(ch)
```

### Complete Example

```c
#include <cyan/cyan.h>

OPTION_DEFINE(i32);
VECTOR_DEFINE(i32);
HASHMAP_DEFINE(i32, i32);

i32 main(void) {
    // Vector with convenience macros
    Vec_i32 nums = vec_i32_new();
    VEC_PUSH(nums, 10);
    VEC_PUSH(nums, 20);
    VEC_PUSH(nums, 30);
    
    printf("Vector length: %zu\n", VEC_LEN(nums));
    
    Option_i32 elem = VEC_GET(nums, 1);
    if (OPT_IS_SOME(elem)) {
        printf("Element at 1: %d\n", OPT_UNWRAP(elem));
    }
    
    // HashMap with convenience macros
    HashMap_i32_i32 scores = hashmap_i32_i32_new();
    MAP_INSERT(scores, 1, 100);
    MAP_INSERT(scores, 2, 200);
    
    if (MAP_CONTAINS(scores, 1)) {
        Option_i32 score = MAP_GET(scores, 1);
        printf("Score for 1: %d\n", OPT_UNWRAP_OR(score, 0));
    }
    
    VEC_FREE(nums);
    MAP_FREE(scores);
    return 0;
}
```

---

## Serialization

Text-based serialization using S-expression-like format.

```c
#include <cyan/serialize.h>

i32 main(void) {
    // === Serialization ===
    char *int_str = serialize_int(42);        // "42"
    char *dbl_str = serialize_double(3.14);   // "3.14"
    char *str_str = serialize_string("hello\nworld");  // "\"hello\\nworld\""
    
    // Generic serialize macro (uses _Generic)
    char *s1 = serialize(42);       // Uses serialize_int
    char *s2 = serialize(3.14);     // Uses serialize_double
    char *s3 = serialize("hello");  // Uses serialize_string
    
    // === Parsing ===
    const char *end;
    
    // Parse integer
    Result_int_ParseError int_res = parse_int("42 rest", &end);
    if (is_ok(int_res)) {
        printf("Parsed: %d\n", unwrap_ok(int_res));
        // end points to " rest"
    }
    
    // Parse double (handles nan, inf, -inf)
    Result_double_ParseError dbl_res = parse_double("3.14", NULL);
    
    // Parse quoted string (handles escape sequences)
    Result_ParsedString_ParseError str_res = parse_string("\"hello\\nworld\"", &end);
    if (is_ok(str_res)) {
        char *parsed = unwrap_ok(str_res);
        printf("Parsed: %s\n", parsed);  // "hello\nworld"
        free(parsed);  // Caller must free
    }
    
    // === Pretty Printing ===
    char *pretty = pretty_print("(1 2 (3 4) 5)", 2);
    printf("%s\n", pretty);
    // Output:
    // (
    //   1
    //   2
    //   (
    //     3
    //     4
    //   )
    //   5
    // )
    
    free(int_str);
    free(dbl_str);
    free(str_str);
    free(s1);
    free(s2);
    free(s3);
    free(pretty);
    return 0;
}
```

**Serialization Grammar:**
```
value    := atom | list
atom     := number | string | symbol
number   := ['-'] digit+ ['.' digit+]
string   := '"' char* '"'
symbol   := alpha (alpha | digit | '_')*
list     := '(' value* ')'
```

**Serialization API:**

| Function | Description |
|----------|-------------|
| `serialize_int(val)` | Serialize integer to string |
| `serialize_long(val)` | Serialize long to string |
| `serialize_float(val)` | Serialize float to string |
| `serialize_double(val)` | Serialize double to string |
| `serialize_string(str)` | Serialize string with escaping |
| `serialize(val)` | Generic serialize (auto-selects type) |
| `parse_int(input, end)` | Parse integer, return Result |
| `parse_double(input, end)` | Parse double, return Result |
| `parse_string(input, end)` | Parse quoted string, return Result |
| `pretty_print(str, indent)` | Format with indentation |

---

## Configuration

Configure the library by defining macros before including headers:

```c
// Custom panic handler
#define CYAN_PANIC(msg) my_panic_handler(msg)

// Collection settings
#define CYAN_DEFAULT_CAPACITY 8   // Initial capacity for vectors, strings
#define CYAN_GROWTH_FACTOR 2      // Growth multiplier when resizing

// HashMap settings
#define CYAN_HASHMAP_INITIAL_CAPACITY 16
#define CYAN_HASHMAP_LOAD_FACTOR 70  // Resize at 70% full

// Coroutine stack size
#define CYAN_CORO_STACK_SIZE (128 * 1024)  // 128KB

// Enable thread-safe channels
#define CYAN_CHANNEL_THREADSAFE

// Suppress warnings for unavailable platform-specific types
#define CYAN_SUPPRESS_TYPE_WARNINGS

#include <cyan/cyan.h>
```

## Feature Detection

Check for available features at compile time:

```c
#include <cyan/cyan.h>

// Library version
#if CYAN_VERSION_AT_LEAST(0, 1, 0)
    // Use features from v0.1.0+
#endif

// Platform-specific types
#if CYAN_HAS_INT128
    i128 big = ...;
#endif

#if CYAN_HAS_FLOAT128
    f128 precise = ...;
#endif

// Compiler features
#if CYAN_HAS_CLEANUP_ATTR
    // defer and smart pointers available
#endif

#if CYAN_HAS_GENERIC
    // _Generic-based macros available
#endif

#if CYAN_HAS_STMT_EXPR
    // Statement expressions available
#endif
```

## Requirements

- C11 compatible compiler (GCC, Clang, or MSVC with C11 support)
- GCC/Clang recommended for:
  - `defer` and auto-cleanup features (uses `__attribute__((cleanup))`)
  - Statement expressions in pattern matching
  - Nested functions in defer
- POSIX system for coroutines (uses `ucontext.h`)
- pthreads for thread-safe channels

## Building Tests

```bash
make test
```

## Examples

See the `examples/` directory for complete example programs:

| Example | Description |
|---------|-------------|
| `00_primitive_types.c` | Primitive type aliases (i32, u64, f32, etc.) |
| `01_option_basics.c` | Option type for nullable values |
| `02_result_error_handling.c` | Result type for error handling |
| `03_vector_collections.c` | Dynamic arrays with bounds checking |
| `04_defer_cleanup.c` | Automatic resource cleanup |
| `05_smart_pointers.c` | Unique and shared pointers |
| `06_pattern_matching.c` | Pattern matching on Option/Result |
| `07_functional.c` | Functional primitives (map, filter, reduce) |
| `08_vtable_api.c` | Vtable method-style API and convenience macros |
| `09_panic_handler.c` | Panic handler behavior and customization |

Build and run examples:
```bash
cd examples
make
make run  # Run all examples
```

## License

MIT License - see LICENSE file for details.

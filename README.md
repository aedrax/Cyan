<div align="center">

<img src="assets/logo.svg" alt="cyan.h" width="300">

### Rust-grade ergonomics for C11, in a header-only include

Options and Results instead of sentinels. Panics instead of undefined
behavior. Vectors, slices, strings, hash maps and sets that check their
bounds. `defer`, smart pointers, pattern matching — and Go-style CSP with
coroutines and channels.

[![version](https://img.shields.io/badge/version-0.3.0-00bcd4?style=flat-square)](#changelog)
[![standard](https://img.shields.io/badge/C11-GNU%20extensions-0891b2?style=flat-square)](#requirements)
[![header-only](https://img.shields.io/badge/header--only-yes-22d3ee?style=flat-square)](#installation)
[![tests](https://img.shields.io/badge/tests-141%2F141-brightgreen?style=flat-square)](#building-and-testing)
[![sanitizers](https://img.shields.io/badge/ASan%2FUBSan-clean-brightgreen?style=flat-square)](#building-and-testing)
[![platforms](https://img.shields.io/badge/platforms-linux%20%7C%20macOS-64748b?style=flat-square)](#requirements)
[![license](https://img.shields.io/badge/license-MIT-64748b?style=flat-square)](#license)

> Pronounced "See-yan" because I'm a monster

</div>

---

```c
#include <cyan/cyan.h>

RESULT_DEFINE(i32, ParseError);

// `?`-style early return: on Err, the whole Result propagates to the caller
Result_i32_ParseError parse_port(const char *s) {
    i32 port = try_ok(parse_int(s, NULL));
    if (port < 1 || port > 65535)
        return Err(i32, ParseError, "port out of range");
    return Ok(i32, ParseError, port);
}

int main(void) {
    match_result(parse_port("8080"), i32, ParseError, port, err,
        { printf("listening on %d\n", port); },
        { fprintf(stderr, "bad config: %s\n", err); }
    );
}
```

## Why Cyan

- **Absence and failure are types, not conventions.** `Option_T` and
  `Result_T_E` make "no value" and "error" impossible to ignore silently —
  with `unwrap`, `expect`, `and_then`, `ok_or`, and `try_ok`/`try_some`
  early returns.
- **Plain structs, zero machinery.** No vtables, no hidden pointers, no
  runtime. An `Option_i32` is a `bool` and an `i32`. Every generated
  function is `static inline`.
- **Macros you can trust.** Every convenience macro evaluates each argument
  exactly once; type-first naming (`VEC_PUSH(i32, v, 42)`) mirrors the
  constructors (`Some(i32, 42)`).
- **Checked by construction.** Bounds-checked access returns Options;
  capacity math is overflow-guarded; allocation failures panic instead of
  corrupting. 141 property-based tests (theft), clean under ASan/UBSan.
- **CSP that composes.** Channels called inside coroutines yield instead of
  failing — `coro_run` schedules them and detects deadlock.
- **Yours to configure.** Custom panic handler, custom allocator hooks,
  `CYAN_NO_SHORT_NAMES` if `map`/`filter`/`unwrap` would collide.

## Installation

Copy `include/cyan/` into your project. That's it — header-only.

```c
#include <cyan/cyan.h>     // everything
// or pick modules:
#include <cyan/option.h>
#include <cyan/vector.h>
```

Compile with GCC or Clang (`-std=gnu11`, or `-std=c11` — the GNU extensions
used are accepted by both). On macOS add `-D_XOPEN_SOURCE=700` if you use
coroutines (see [Requirements](#requirements)).

## Sixty-second tour

```c
// Options and Results ------------------------------------------------
Option_i32 found = vec_i32_find(&v, wanted);        // search returns Option
i32 x = unwrap_or(found, -1);                       // never a stray NULL

// Collections --------------------------------------------------------
Vec_i32 v = vec_i32_new();                          // growable, bounds-checked
VEC_PUSH(i32, v, 42);
VEC_FOREACH(i32, v, it) printf("%d ", *it);

HashMap_str_i32 counts = hashmap_str_i32_new();     // content-hashed str keys
hashmap_str_i32_insert(&counts, "apple", 1);        // key is copied & owned

// Strings ------------------------------------------------------------
String s = string_from("a,b,c");
Slice_char rest = string_as_slice(&s), part;
while (string_split_next(&rest, ',', &part))        // zero-copy split
    printf("%.*s\n", (int)part.len, part.data);

// Cleanup ------------------------------------------------------------
defer({ close_thing(&thing); });                    // runs on scope exit
string_auto(tmp, string_from("freed automatically"));
```

Every feature has a full reference below and a runnable program in
[`examples/`](#examples).

## Module reference

<details>
<summary><b>Option</b> — explicit nullable values with combinators</summary>

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

**Combinators (new in 0.2.0):**

```c
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

`try_some` requires the enclosing function to return the same `Option_T` type (GCC/Clang only).

**Option API:**

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

**Convenience Macros:**

The short names above are default-on aliases for these uppercase macros, which are always available (even with `CYAN_NO_SHORT_NAMES` defined):

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

---

</details>

<details>
<summary><b>Result</b> — explicit error handling with <code>try_ok</code> early returns</summary>

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

**Combinators (new in 0.2.0):**

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

`try_ok` requires the enclosing function to return the same `Result_T_E` type (GCC/Clang only).

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
| `expect_ok(res, msg)` | Extract success value (panics with `msg` if Err) |
| `map_result(res, T_out, E, fn)` | Transform success value |
| `map_err(res, T, E_out, fn)` | Transform error value |
| `and_then_result(res, T_out, E, fn)` | Chain a function returning `Result_T_out_E` |
| `try_ok(res)` | Extract Ok value, or early-return the Err (like Rust's `?`) |

**Convenience Macros:**

The short names above are default-on aliases for these uppercase macros, which are always available (even with `CYAN_NO_SHORT_NAMES` defined):

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

---

</details>

<details>
<summary><b>Pattern Matching</b> — ergonomic <code>match</code> over Option/Result</summary>

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

</details>

<details>
<summary><b>Vector</b> — growable arrays: push/pop/insert/remove, find, sort, iterate</summary>

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

**New in 0.2.0 — insert, remove, extend, reserve, clear:**

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

**Vector API:**

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
| `vec_T_free(v)` | Free vector memory |

**Convenience Macros:**

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

---

### New in 0.3.0: search, sort, iteration

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

</details>

<details>
<summary><b>Slice</b> — non-owning bounds-checked views</summary>

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

**Convenience Macros:**

| Macro | Description |
|-------|-------------|
| `SLICE_GET(T, s, idx)` | Get element at index |
| `SLICE_SUBSLICE(T, s, start, end)` | Create subslice view |
| `SLICE_LEN(T, s)` | Get slice length |

---

</details>

<details>
<summary><b>String</b> — growable text: search, trim, split, compare</summary>

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

**New in 0.2.0 — search, compare, trim, split:**

```c
String s = string_from("  Hello World  ");

// Search (byte index of first match)
Option_size_t pos = string_find(&s, "World");   // Some(8)
bool has = string_contains(&s, "World");        // true

// Prefix/suffix checks and trimming
string_trim(&s);                                // In place: "Hello World"
bool starts = string_starts_with(&s, "Hello");  // true
bool ends = string_ends_with(&s, "World");      // true

// Content equality (NULL-tolerant)
String other = string_from("Hello World");
bool same = string_eq(&s, &other);              // true

// Splitting: iterate delimiter-separated parts as Slice_char views
String csv = string_from("a,b,");
Slice_char rest = string_as_slice(&csv);
Slice_char part;
while (string_split_next(&rest, ',', &part)) {
    printf("part: %.*s\n", (int)part.len, part.data);
}
// Yields "a", "b", "" (a trailing delimiter produces an empty part)

string_free(&s);
string_free(&other);
string_free(&csv);
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
| `string_find(s, needle)` | Find substring, return index as `Option_size_t` |
| `string_contains(s, needle)` | Check if substring occurs |
| `string_starts_with(s, prefix)` | Check prefix |
| `string_ends_with(s, suffix)` | Check suffix |
| `string_trim(s)` | Strip leading/trailing whitespace in place |
| `string_eq(a, b)` | Content equality (NULL-tolerant) |
| `string_split_next(rest, delim, out)` | Advance split iterator over a `Slice_char` |
| `string_free(s)` | Free string memory |
| `string_auto(name, init)` | Declare with auto-cleanup |

**Convenience Macros:**

String is monomorphic, so its macros take no type argument:

| Macro | Description |
|-------|-------------|
| `STR_PUSH(s, c)` | Append single character |
| `STR_APPEND(s, cstr)` | Append C string |
| `STR_CLEAR(s)` | Clear content |
| `STR_GET(s, idx)` | Get character as Option |
| `STR_LEN(s)` | Get length |
| `STR_CSTR(s)` | Get null-terminated C string |
| `STR_SLICE(s, start, end)` | Create slice view |
| `STR_FIND(s, needle)` | Find substring, return `Option_size_t` |
| `STR_CONTAINS(s, needle)` | Check if substring occurs |
| `STR_FREE(s)` | Free string memory |

---

### New in 0.3.0: materializing split pieces

`string_split_next` yields non-null-terminated `Slice_char` views. Turn one
into an owned C string with `string_from_slice`, or compare in place with
`string_slice_eq`:

```c
Slice_char rest = string_as_slice(&csv), part;
while (string_split_next(&rest, ',', &part)) {
    if (string_slice_eq(part, "skip")) continue;   // no allocation
    String field = string_from_slice(part);        // owned + null-terminated
    use(string_cstr(&field));
    string_free(&field);
}
```

</details>

<details>
<summary><b>HashMap</b> — open-addressing maps, string-keyed variant, iteration</summary>

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

**Convenience Macros:**

Macros take the key and value types first:

| Macro | Description |
|-------|-------------|
| `MAP_INSERT(K, V, m, k, val)` | Insert or update entry |
| `MAP_GET(K, V, m, k)` | Get value as Option |
| `MAP_CONTAINS(K, V, m, k)` | Check if key exists |
| `MAP_REMOVE(K, V, m, k)` | Remove entry, return value |
| `MAP_LEN(K, V, m)` | Get number of entries |
| `MAP_FREE(K, V, m)` | Free map memory |

> **Warning:** `HASHMAP_DEFINE` hashes and compares the raw bytes of the key type. For pointer keys such as `char *`, that means the *pointer value* is hashed, not the pointed-to contents — two identical strings at different addresses are different keys. Use `HASHMAP_STR_DEFINE` for string keys.

### String-Keyed HashMap (new in 0.2.0)

`HASHMAP_STR_DEFINE(V)` defines `HashMap_str_V` with content-hashed `char *` keys:

```c
#include <cyan/hashmap.h>

OPTION_DEFINE(i32);        // Required before HASHMAP_STR_DEFINE(i32)
HASHMAP_STR_DEFINE(i32);   // HashMap_str_i32

i32 main(void) {
    HashMap_str_i32 ages = hashmap_str_i32_new();
    
    // insert COPIES the key — the map owns its copy
    char name[] = "alice";
    hashmap_str_i32_insert(&ages, name, 30);
    name[0] = 'A';  // Safe: the map's key copy is unaffected
    
    // Lookup is by content, not by pointer
    Option_i32 age = hashmap_str_i32_get(&ages, "alice");  // Some(30)
    
    if (hashmap_str_i32_contains(&ages, "alice")) {
        printf("alice is %d\n", unwrap(age));
    }
    
    // remove and free release the map's key copies
    hashmap_str_i32_remove(&ages, "alice");
    hashmap_str_i32_free(&ages);
    return 0;
}
```

**Key ownership rules:**
- `hashmap_str_V_insert` copies the key string; the caller keeps ownership of the original.
- `hashmap_str_V_remove` and `hashmap_str_V_free` free the map's key copies.
- Keys are hashed and compared by content, so heap strings, stack buffers, and literals all work.

**String-Keyed HashMap API:** `hashmap_str_V_new()`, `hashmap_str_V_insert(m, key, value)`, `hashmap_str_V_get(m, key)`, `hashmap_str_V_contains(m, key)`, `hashmap_str_V_remove(m, key)`, `hashmap_str_V_len(m)`, `hashmap_str_V_free(m)` — same shapes as the generic map.

---

### New in 0.3.0: iteration macro

```c
HASHMAP_ITER_DEFINE(i32, i32);   // also defines MapPair_i32_i32

MapPair_i32_i32 pair;
MAP_FOREACH(i32, i32, m, pair) {
    printf("%d -> %d\n", pair.key, pair.value);   // break/continue work normally
}
```

</details>

<details>
<summary><b>HashSet</b> — membership testing with HashMap's engine</summary>

Type-safe hash sets sharing HashMap's open-addressing design (power-of-two
capacity, tombstones, load-factor resizing, optional custom `hash_fn`/`equal_fn`).
No `OPTION_DEFINE` prerequisite.

```c
#include <cyan/hashset.h>

HASHSET_DEFINE(i32);
HASHSET_ITER_DEFINE(i32);   // optional: iteration support

HashSet_i32 seen = hashset_i32_new();
hashset_i32_add(&seen, 42);        // true  (newly added)
hashset_i32_add(&seen, 42);        // false (already present)
hashset_i32_contains(&seen, 42);   // true
hashset_i32_remove(&seen, 42);     // true  (was present)

i32 item;
SET_FOREACH(i32, seen, item) {     // pre-declare item; break/continue work
    printf("%d\n", item);
}
hashset_i32_free(&seen);
```

| Function / Macro | Description |
|------------------|-------------|
| `hashset_T_new()` | Create empty set |
| `hashset_T_add(&s, x)` / `SET_ADD(T, s, x)` | Add; returns `true` if newly added |
| `hashset_T_contains(&s, x)` / `SET_CONTAINS(T, s, x)` | Membership test |
| `hashset_T_remove(&s, x)` / `SET_REMOVE(T, s, x)` | Remove; returns `true` if it was present |
| `hashset_T_len(&s)` / `SET_LEN(T, s)` | Number of elements |
| `hashset_T_free(&s)` / `SET_FREE(T, s)` | Free all memory |
| `hashset_T_iter(&s)` / `hashset_T_iter_next(&it)` | Explicit iterator |
| `SET_FOREACH(T, s, item)` | Iteration loop macro |

Like `HASHMAP_DEFINE`, elements are hashed by their raw bytes — the pointer
caveat for `char *` elements applies here too.

</details>

<details>
<summary><b>Functional</b> — map / filter / reduce / foreach</summary>

Higher-order functions for declarative data transformation. The canonical names are `cyan_map`, `cyan_filter`, `cyan_reduce`, and `cyan_foreach`; the short aliases `map`, `filter`, `reduce`, and `foreach` are enabled by default and can be suppressed with `#define CYAN_NO_SHORT_NAMES` (the `cyan_*` names remain available).

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

---

</details>

<details>
<summary><b>Defer</b> — scope-exit cleanup, RAII-style</summary>

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

</details>

<details>
<summary><b>Smart Pointers</b> — unique / shared / weak with auto-release</summary>

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
    
    // Convenience macros (equivalent to the standalone functions)
    if (!WPTR_IS_EXPIRED(i32, weak)) {
        Option_SharedPtr_i32 upgraded = WPTR_UPGRADE(i32, weak);
        if (upgraded.has_value) {
            printf("Upgraded via macro: %d\n", shared_i32_deref(&upgraded.value));
            shared_i32_release(&upgraded.value);
        }
    }
    
    // Clone a weak reference (new in 0.2.0)
    WeakPtr_i32 weak2 = WPTR_CLONE(i32, weak);
    
    shared_i32_release(&owner);  // Memory freed
    // weak_i32_is_expired(&weak) now returns true
    
    WPTR_RELEASE(i32, weak);   // Release weak references
    WPTR_RELEASE(i32, weak2);
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
| `weak_T_clone(w)` | Clone weak reference (new in 0.2.0) |
| `weak_T_release(w)` | Release weak reference |

**Convenience Macros:**

Macros take the pointee type first:

| Macro | Description |
|-------|-------------|
| `UPTR_GET(T, u)` | Get raw pointer from UniquePtr |
| `UPTR_DEREF(T, u)` | Dereference UniquePtr |
| `UPTR_MOVE(T, u)` | Transfer ownership from UniquePtr |
| `UPTR_FREE(T, u)` | Free UniquePtr |
| `SPTR_GET(T, s)` | Get raw pointer from SharedPtr |
| `SPTR_DEREF(T, s)` | Dereference SharedPtr |
| `SPTR_CLONE(T, s)` | Clone SharedPtr (increment ref count) |
| `SPTR_COUNT(T, s)` | Get reference count |
| `SPTR_RELEASE(T, s)` | Release SharedPtr (decrement ref count) |
| `WPTR_IS_EXPIRED(T, w)` | Check if WeakPtr target freed |
| `WPTR_UPGRADE(T, w)` | Upgrade WeakPtr to SharedPtr |
| `WPTR_CLONE(T, w)` | Clone WeakPtr (new in 0.2.0) |
| `WPTR_RELEASE(T, w)` | Release WeakPtr |

---

</details>

<details>
<summary><b>Coroutines (experimental)</b> — stackful cooperative multitasking over ucontext</summary>

Stackful cooperative multitasking using POSIX ucontext.

> **EXPERIMENTAL:** the implementation relies on POSIX ucontext, which has been deprecated on macOS since 10.6 and does not exist on non-POSIX platforms (Windows). It works on current Linux and macOS toolchains, but the underlying primitive has no long-term platform guarantees. The API may change if the backend is replaced.

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
| `coro_has_yield(c)` | Check if a yielded value is available |
| `coro_is_finished(c)` | Check if coroutine completed |
| `coro_status(c)` | Get current status |
| `coro_free(c)` | Free coroutine resources |

**Coroutine Status:**
- `CORO_CREATED` - Created but never resumed
- `CORO_RUNNING` - Currently executing
- `CORO_SUSPENDED` - Yielded, waiting to resume
- `CORO_FINISHED` - Completed execution

---

</details>

<details>
<summary><b>Channels</b> — CSP-style communication, coroutine-aware</summary>

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

**Convenience Macros:**

Macros take the element type first (`ch` is a pointer):

| Macro | Description |
|-------|-------------|
| `CHAN_SEND(T, ch, val)` | Send value to channel |
| `CHAN_RECV(T, ch)` | Receive value from channel |
| `CHAN_TRY_SEND(T, ch, val)` | Non-blocking send |
| `CHAN_TRY_RECV(T, ch)` | Non-blocking receive |
| `CHAN_CLOSE(T, ch)` | Close channel |
| `CHAN_IS_CLOSED(T, ch)` | Check if closed |
| `CHAN_FREE(T, ch)` | Free channel |

Channel functions are NULL-safe: sending on a NULL channel returns `CHAN_CLOSED`, receiving returns None, and `is_closed` reports true.

**Channel Status:**
- `CHAN_OK` - Operation succeeded
- `CHAN_CLOSED` - Channel is closed
- `CHAN_WOULD_BLOCK` - Operation would block (try_* variants)

---

### New in 0.3.0: coroutine integration (CSP)

In single-threaded builds, a blocking channel operation called **inside a
coroutine** yields and retries instead of returning `CHAN_WOULD_BLOCK`/`None`.
Drive the coroutines with `coro_run` for Go-style CSP:

```c
CHANNEL_DEFINE(i32);
static Channel_i32 *ch;

static void producer(Coro *self, void *arg) {
    for (i32 i = 1; i <= 5; i++) chan_i32_send(ch, i * 10);  // rendezvous
    chan_i32_close(ch);
}
static void consumer(Coro *self, void *arg) {
    for (;;) {
        Option_i32 v = chan_i32_recv(ch);   // yields until a value arrives
        if (is_none(v)) break;
        printf("got %d\n", unwrap(v));
    }
}

ch = chan_i32_new(0);                        // capacity 0: unbuffered
Coro *cs[] = { coro_new(producer, NULL, 0), coro_new(consumer, NULL, 0) };
bool ok = coro_run(cs, 2);                   // false would mean deadlock
```

`coro_run` detects deadlock: if a full pass resumes coroutines but none
finishes and no channel makes progress, it returns `false`. Coroutines that
yield repeatedly without channel traffic should call `coro_mark_progress()`.
Everything (coroutines, channels, `coro_run`) must live in one translation
unit. Outside coroutines — and for `try_send`/`try_recv` — single-threaded
behavior is unchanged.

</details>

<details>
<summary><b>Serialization</b> — S-expressions: atoms, symbols, nested lists, round-trip</summary>

Text-based serialization using an S-expression format. Alongside the scalar helpers below, 0.2.0 adds a full S-expression tree API (`SExp`) that can parse, build, compare, and serialize arbitrarily nested lists.

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

### S-Expression Trees (new in 0.2.0)

```c
#include <cyan/serialize.h>

i32 main(void) {
    // === Parsing ===
    Result_SExpPtr_ParseError r = parse_sexp("(add 1 2.5 \"three\" (nested list))", NULL);
    if (is_ok(r)) {
        SExp *e = unwrap_ok(r);
        // e->type is one of SEXP_INT, SEXP_DOUBLE, SEXP_STRING,
        // SEXP_SYMBOL, SEXP_LIST
        // Access: e->i (int), e->d (double), e->str (string/symbol),
        //         e->list.items / e->list.len (list)
        printf("List with %zu items\n", e->list.len);
        sexp_free(e);  // Recursively frees the whole tree
    }
    
    // === Building trees programmatically ===
    SExp *list = sexp_list_new();
    sexp_list_push(list, sexp_symbol("point"));  // push takes ownership
    sexp_list_push(list, sexp_int(3));
    sexp_list_push(list, sexp_double(1.5));
    sexp_list_push(list, sexp_string("label"));
    
    // === Serializing ===
    char *text = serialize_sexp(list);  // "(point 3 1.5 \"label\")"
    printf("%s\n", text);
    
    // === Round-trip guarantee ===
    Result_SExpPtr_ParseError back = parse_sexp(text, NULL);
    // sexp_eq is structural equality (NaN == NaN is true here)
    assert(sexp_eq(unwrap_ok(back), list));
    
    sexp_free(unwrap_ok(back));
    sexp_free(list);
    free(text);
    return 0;
}
```

`parse_sexp` rejects input nested deeper than `CYAN_SEXP_MAX_DEPTH` (default 1000, overridable before including headers).

**Serialization Grammar:**
```
value    := atom | list
atom     := number | string | symbol
number   := ['-'] digit+ ['.' digit+]
string   := '"' char* '"'
symbol   := alpha (alpha | digit | '_')*
list     := '(' value* ')'
```

As of 0.2.0 this grammar is fully implemented: `parse_sexp` handles every production, including symbols and arbitrarily nested lists.

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
| `parse_sexp(input, end)` | Parse full S-expression tree, return `Result_SExpPtr_ParseError` |
| `serialize_sexp(e)` | Serialize tree back to text (caller frees) |
| `sexp_int(v)` / `sexp_double(v)` / `sexp_string(s)` / `sexp_symbol(s)` | Construct atom nodes |
| `sexp_list_new()` | Construct empty list node |
| `sexp_list_push(list, child)` | Append child to list (takes ownership) |
| `sexp_eq(a, b)` | Structural equality (NaN == NaN is true) |
| `sexp_free(e)` | Recursively free a tree |

---

</details>

<details>
<summary><b>Bit-Width Integers</b> — Zig-style <code>uN</code>/<code>iN</code> wrap-around integers</summary>

Zig-inspired integer types with arbitrary bit widths (1-64 bits). Define custom unsigned integers with `UINT_DEFINE(N)` and signed integers with `INT_DEFINE(N)`.

```c
#include <cyan/bitint.h>

// Define custom bit-width types
UINT_DEFINE(6);   // u6: 6-bit unsigned (0-63)
UINT_DEFINE(12);  // u12: 12-bit unsigned (0-4095)
INT_DEFINE(6);    // i6: 6-bit signed (-32 to 31)
INT_DEFINE(12);   // i12: 12-bit signed (-2048 to 2047)

i32 main(void) {
    // Unsigned integers - values are masked to fit
    u6 val = u6_new(42);           // 42
    u6 overflow = u6_new(100);     // 36 (100 & 0x3F)
    printf("Value: %u\n", u6_get(&val));
    
    // Arithmetic operations (results masked to N bits)
    u6 a = u6_new(30);
    u6 b = u6_new(40);
    u6 sum = u6_add(a, b);         // 6 (70 wraps at 64)
    
    // Bitwise operations
    u6 masked = u6_and(a, b);
    u6 shifted = u6_shl(a, 2);
    
    // Signed integers with sign extension
    i6 pos = i6_new(20);
    i6 neg = i6_new(-15);
    i6 diff = i6_sub(pos, neg);    // Wraps in 6-bit signed range
    i6 negated = i6_neg(pos);      // -20
    
    // Min/max values
    u6 max_u6 = u6_max();          // 63
    i6 min_i6 = i6_min();          // -32
    
    return 0;
}
```

**Unsigned Integer API (UINT_DEFINE):**

| Function | Description |
|----------|-------------|
| `uN_new(value)` | Create N-bit unsigned integer (value masked to N bits) |
| `uN_get(ptr)` | Get value as backing type |
| `uN_raw(ptr)` | Get raw backing value |
| `uN_add(a, b)` | Add two values (result masked) |
| `uN_sub(a, b)` | Subtract two values (result masked) |
| `uN_mul(a, b)` | Multiply two values (result masked) |
| `uN_and(a, b)` | Bitwise AND |
| `uN_or(a, b)` | Bitwise OR |
| `uN_xor(a, b)` | Bitwise XOR |
| `uN_not(a)` | Bitwise NOT (masked to N bits) |
| `uN_shl(a, shift)` | Left shift (result masked) |
| `uN_shr(a, shift)` | Right shift |
| `uN_eq(a, b)` | Equality comparison |
| `uN_lt(a, b)` | Less than comparison |
| `uN_le(a, b)` | Less than or equal comparison |
| `uN_max()` | Maximum value (2^N - 1) |
| `uN_min()` | Minimum value (0) |

**Signed Integer API (INT_DEFINE):**

| Function | Description |
|----------|-------------|
| `iN_new(value)` | Create N-bit signed integer (sign-extended) |
| `iN_get(ptr)` | Get sign-extended value |
| `iN_add(a, b)` | Add two values |
| `iN_sub(a, b)` | Subtract two values |
| `iN_mul(a, b)` | Multiply two values |
| `iN_neg(a)` | Negate value |
| `iN_eq(a, b)` | Equality comparison |
| `iN_lt(a, b)` | Less than comparison |
| `iN_le(a, b)` | Less than or equal comparison |
| `iN_max()` | Maximum value (2^(N-1) - 1) |
| `iN_min()` | Minimum value (-2^(N-1)) |

**Backing Type Selection:**

| Bit Width | Unsigned Backing | Signed Backing |
|-----------|------------------|----------------|
| 1-8       | u8               | i8             |
| 9-16      | u16              | i16            |
| 17-32     | u32              | i32            |
| 33-64     | u64              | i64            |

---

</details>

<details>
<summary><b>Bitset</b> — fixed-size flag collections with named flags</summary>

Fixed-size bit collections for efficient flag management. Define bitsets with `BITSET_DEFINE(N)` for N bits (1-64).

```c
#include <cyan/bitset.h>

BITSET_DEFINE(8);   // Bitset_8: 8-bit bitset
BITSET_DEFINE(16);  // Bitset_16: 16-bit bitset

i32 main(void) {
    // Create empty bitset
    Bitset_8 bs = bitset_8_new();
    
    // Set, clear, toggle bits
    bitset_8_set(&bs, 0);      // Set bit 0
    bitset_8_set(&bs, 3);      // Set bit 3
    bitset_8_toggle(&bs, 3);   // Toggle bit 3 (now clear)
    bitset_8_clear(&bs, 0);    // Clear bit 0
    
    // Query bits
    bool is_set = bitset_8_get(&bs, 0);  // false
    
    // Create from raw value
    Bitset_8 set1 = bitset_8_from_raw(0b00001111);  // Bits 0-3
    Bitset_8 set2 = bitset_8_from_raw(0b00111100);  // Bits 2-5
    
    // Set operations
    Bitset_8 union_set = bitset_8_union(&set1, &set2);      // OR
    Bitset_8 intersect = bitset_8_intersect(&set1, &set2);  // AND
    Bitset_8 diff = bitset_8_diff(&set1, &set2);            // set1 & ~set2
    Bitset_8 comp = bitset_8_complement(&set1);             // ~set1 (masked)
    
    // Utility functions
    u8 count = bitset_8_count(&bs);     // Number of set bits
    bool all = bitset_8_all(&bs);       // All bits set?
    bool any = bitset_8_any(&bs);       // Any bit set?
    bool none = bitset_8_none(&bs);     // No bits set?
    bool equal = bitset_8_eq(&set1, &set2);  // Equality check
    
    return 0;
}
```

**Bitset API:**

| Function | Description |
|----------|-------------|
| `bitset_N_new()` | Create bitset with all bits cleared |
| `bitset_N_from_raw(value)` | Create bitset from raw integer value |
| `bitset_N_set(bs, index)` | Set bit at index (panics if out of bounds) |
| `bitset_N_clear(bs, index)` | Clear bit at index |
| `bitset_N_get(bs, index)` | Get bit at index (returns bool) |
| `bitset_N_toggle(bs, index)` | Toggle bit at index |
| `bitset_N_union(a, b)` | Union of two bitsets (OR) |
| `bitset_N_intersect(a, b)` | Intersection of two bitsets (AND) |
| `bitset_N_diff(a, b)` | Difference (a AND NOT b) |
| `bitset_N_complement(bs)` | Complement (NOT, masked to N bits) |
| `bitset_N_eq(a, b)` | Check equality |
| `bitset_N_count(bs)` | Count set bits (popcount) |
| `bitset_N_all(bs)` | Check if all N bits are set |
| `bitset_N_any(bs)` | Check if any bit is set |
| `bitset_N_none(bs)` | Check if no bits are set |

**Convenience Macros:**

| Macro | Description |
|-------|-------------|
| `BS_SET(N, bs, i)` | Set bit at index |
| `BS_CLEAR(N, bs, i)` | Clear bit at index |
| `BS_GET(N, bs, i)` | Get bit at index |
| `BS_TOGGLE(N, bs, i)` | Toggle bit at index |
| `BS_UNION(N, a, b)` | Union of two bitsets |
| `BS_INTERSECT(N, a, b)` | Intersection of two bitsets |
| `BS_DIFF(N, a, b)` | Difference of two bitsets |
| `BS_COMPLEMENT(N, bs)` | Complement of bitset |
| `BS_EQ(N, a, b)` | Check equality |
| `BS_COUNT(N, bs)` | Count set bits |
| `BS_ALL(N, bs)` | Check if all bits set |
| `BS_ANY(N, bs)` | Check if any bit set |
| `BS_NONE(N, bs)` | Check if no bits set |

### Named Flags

Define named flags with `FLAGS_DEFINE` for type-safe flag manipulation:

```c
#include <cyan/bitset.h>

// Define named flags
FLAGS_DEFINE(Permissions, READ, WRITE, EXECUTE, HIDDEN);
// Creates: Permissions_READ = 0, Permissions_WRITE = 1, etc.
// Creates: Permissions_COUNT = 4

BITSET_DEFINE(4);  // Bitset for 4 flags

i32 main(void) {
    Bitset_4 perms = bitset_4_new();
    
    // Set flags using names
    FLAGS_SET(4, perms, Permissions_READ);
    FLAGS_SET(4, perms, Permissions_WRITE);
    
    // Check flags
    if (FLAGS_HAS(4, perms, Permissions_READ)) {
        printf("Has read permission\n");
    }
    
    // Clear flags
    FLAGS_CLEAR(4, perms, Permissions_WRITE);
    
    return 0;
}
```

**Named Flags API:**

| Macro | Description |
|-------|-------------|
| `FLAGS_DEFINE(Name, ...)` | Define named flags with sequential bit positions |
| `FLAGS_SET(N, bs, flag)` | Set the specified flag |
| `FLAGS_CLEAR(N, bs, flag)` | Clear the specified flag |
| `FLAGS_HAS(N, bs, flag)` | Check if flag is set |

---

</details>

<details>
<summary><b>Primitive Types</b> — i32 / u64 / f32 and friends</summary>

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
```

For type-erased pointers, use plain `void *` (the `any` alias was removed in 0.2.0).

**Available Types:**

| Category | Types |
|----------|-------|
| Signed integers | `i8`, `i16`, `i32`, `i64`, `i128`* |
| Unsigned integers | `u8`, `u16`, `u32`, `u64`, `u128`* |
| Pointer-sized | `isize`, `usize` |
| Floating-point | `f16`*, `f32`, `f64`, `f80`*, `f128`* |
| Special | `bool` |

*Platform-dependent. Check `CYAN_HAS_INT128`, `CYAN_HAS_FLOAT16`, `CYAN_HAS_FLOAT80`, `CYAN_HAS_FLOAT128` macros.

---

</details>

<details>
<summary><b>Panic Handler</b> — what panics, and how to override it</summary>

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

</details>

<details>
<summary><b>Method-Style Macros</b> — the type-first macro convention</summary>

Every Cyan collection type pairs its generated functions with uppercase convenience macros. The macros follow a single **type-first convention**: the type parameter comes first, exactly as it does in constructors like `Some(i32, 42)` or `Ok(i32, const_charp, val)`.

```c
VEC_PUSH(i32, v, 42);      // mirrors Some(i32, 42)
MAP_GET(i32, i32, m, key); // mirrors Ok/Err's (T, E, ...) ordering
```

The macros are zero-cost aliases: each one expands directly to a call to the corresponding generated function (`VEC_PUSH(i32, v, 42)` becomes `vec_i32_push(&v, 42)`), so there is no indirection and no runtime overhead, and every argument is evaluated exactly once. Since 0.2.0, Cyan types are plain structs — an `Option_i32` is just a `bool` plus an `i32`, a `Vec_i32` is just `data` + `len` + `cap`. There are no embedded function pointers or vtables of any kind.

### Two Ways to Call Operations

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

The same pattern applies to every container. For a `HashMap`, both type parameters are passed:

```c
HashMap_i32_i32 m = hashmap_i32_i32_new();

hashmap_i32_i32_insert(&m, 1, 100);   // Function style
MAP_INSERT(i32, i32, m, 2, 200);      // Macro style

Option_i32 val = MAP_GET(i32, i32, m, 1);
MAP_FREE(i32, i32, m);
```

Monomorphic types (`String`) and typeless-accessor macros (`OPT_*`, `RES_*`) take no type argument, since the member layout is the same for every instantiation. See each type's section above for its full macro table.

### Complete Example

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

---

</details>

## Configuration
Configure the library by defining macros before including headers:

```c
// Custom panic handler
#define CYAN_PANIC(msg) my_panic_handler(msg)

// Custom allocator hooks (override all three together)
#define CYAN_MALLOC(size)        my_malloc(size)
#define CYAN_REALLOC(ptr, size)  my_realloc(ptr, size)
#define CYAN_FREE(ptr)           my_free(ptr)

// Suppress short lowercase names (is_some, unwrap, map, filter, try_ok, ...)
// The uppercase OPT_*/RES_* macros and the cyan_map/cyan_filter/cyan_reduce/
// cyan_foreach names remain available.
#define CYAN_NO_SHORT_NAMES

// Maximum S-expression nesting depth accepted by parse_sexp (default 1000)
#define CYAN_SEXP_MAX_DEPTH 1000

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

// Opt in to #warning diagnostics when platform-specific types
// (i128, f16, f80, f128) are unavailable (silent by default)
#define CYAN_ENABLE_TYPE_WARNINGS

#include <cyan/cyan.h>
```

## Feature Detection
Check for available features at compile time:

```c
#include <cyan/cyan.h>

// Library version (version macros live in common.h)
#if CYAN_VERSION_AT_LEAST(0, 2, 0)
    // Use features from v0.2.0+
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
- **macOS note:** for the coroutine `ucontext` APIs to be visible, either include Cyan headers before any system header, or compile with `-D_XOPEN_SOURCE=700` (the project Makefile does this)

## Building and Testing

```bash
make test              # 141 property-based tests (theft, vendored submodule)
make test SANITIZE=1   # the same suite under AddressSanitizer + UBSan
```

Property-based testing via [theft](https://github.com/silentbicycle/theft)
(`git submodule update --init` on first clone).

## Examples

Twenty runnable programs in [`examples/`](examples/), each an API tour that
ends with a realistic "putting it together" scenario:

| Example | Shows |
|---------|-------|
| `00_primitive_types.c` | Type aliases; parsing a binary sensor packet |
| `01_option_basics.c` | Options; a config lookup chain with `and_then`/`or_else` |
| `02_result_error_handling.c` | Results; `try_ok` order-validation pipeline |
| `03_vector_collections.c` | Vectors; a sorted top-N scoreboard |
| `04_defer_cleanup.c` | Defer; a resource pyramid with early returns |
| `05_smart_pointers.c` | Unique/shared/weak; a cache with weak observers |
| `06_pattern_matching.c` | `match_*`; a connection state machine |
| `07_functional.c` | map/filter/reduce; a sensor data pipeline |
| `08_method_macros.c` | The two call styles; type sizes without vtables |
| `09_panic_handler.c` | Panics and custom handlers |
| `10_channel_communication.c` | Channels; a bounded work queue |
| `11_hashmap_dictionary.c` | Maps incl. string keys; an office phone book |
| `12_serialize_parsing.c` | S-expressions; a config round-trip |
| `13_slice_views.c` | Slices; a zero-copy tokenizer |
| `14_string_manipulation.c` | Strings; a CSV line parser |
| `15_hashset_membership.c` | HashSet; dedupe and set intersection |
| `16_bitset_integers.c` | Bitsets/uN; permissions and a saturating counter |
| `17_coro_channels.c` | **CSP**: producers/consumers under `coro_run` |
| `18_result_pipeline.c` | A realistic config-file parsing pipeline |
| `19_word_count.c` | Capstone: strings + str-map + vec sort, top-5 words |

```bash
cd examples
make        # build all
make run    # run all
```

## Changelog
### 0.3.0

**Additions:**

- **Coroutine-aware channels (headline).** In single-threaded mode, `chan_T_send`/`chan_T_recv` called inside a coroutine now yield-and-retry instead of returning `CHAN_WOULD_BLOCK`/`None`, giving Go-style CSP. Drive coroutines with the new `coro_run(coros, n)` round-robin scheduler, which returns `false` on deadlock (no completion and no channel progress in a full pass). New `coro_current()` and `coro_mark_progress()`. Behavior outside coroutines, for `try_*` variants, and in thread-safe mode is unchanged. Coroutines, their channels, and `coro_run` must share one translation unit.
- **HashSet** (`hashset.h`, included by `cyan.h`, `CYAN_HAS_HASHSET`): `HASHSET_DEFINE(T)` / `HASHSET_ITER_DEFINE(T)` with `add` (returns whether newly added), `contains`, `remove`, `len`, `free`, iteration, and type-first `SET_*` macros including `SET_FOREACH`.
- **Slice materialization:** `string_from_slice(Slice_char)` produces an owned, null-terminated `String` from a split piece; `string_slice_eq(slice, cstr)` compares without allocating.
- **Vector search/sort/iteration:** `vec_T_find(v, pred)` → `Option_size_t`, `vec_T_contains(v, pred)`, `vec_T_sort(v, cmp)` (qsort-style comparator), plus `VEC_FIND`/`VEC_CONTAINS`/`VEC_SORT` and the `VEC_FOREACH(T, v, it)` loop macro.
- **HashMap iteration macro:** `MAP_FOREACH(K, V, m, pair)` over a pre-declared `MapPair_K_V pair;` (requires `HASHMAP_ITER_DEFINE`; single loop, so `break`/`continue` behave normally).

**Breaking change:**

- `Option_size_t` is now defined by `option.h` itself; remove any `OPTION_DEFINE(size_t)` from user code.

### 0.2.0

**Breaking changes:**

- **Vtable system removed.** All types are now plain structs (an `Option_i32` is just a `bool` + `i32`; a `Vec_i32` is just `data` + `len` + `cap`). Any `x.vt->fn(...)` or `ch->vt->fn(...)` call must become a standalone function call (`vec_i32_push(&v, x)`) or a convenience macro call (`VEC_PUSH(i32, v, x)`).
- **Container macros are now type-first.** `VEC_PUSH(v, 42)` becomes `VEC_PUSH(i32, v, 42)`; likewise for `MAP_*(K, V, ...)`, `SLICE_*(T, ...)`, `CHAN_*(T, ...)`, `BS_*(N, ...)`, `FLAGS_*(N, ...)`, `UPTR_*/SPTR_*/WPTR_*(T, ...)`. `STR_*` (monomorphic) and `OPT_*`/`RES_*` (typeless member access) are unchanged.
- **match.h sugar removed:** the no-op `some(var)`, `none()`, `ok(var)`, `err(var)` macros are gone; the `match_option`/`match_result` macros themselves are unchanged.
- **`any` type alias removed** from common.h — use plain `void *`.
- **Version macros moved to common.h** (they are no longer duplicated in other headers).

**Additions:**

- Option/Result combinators: `expect`, `and_then`, `or_else`, `ok_or`, `try_some`, `expect_ok`, `and_then_result`, `try_ok` (plus their always-available `OPT_*`/`RES_*` forms)
- String utilities: `string_find`, `string_contains`, `string_starts_with`, `string_ends_with`, `string_trim`, `string_eq`, `string_split_next`
- String-keyed hashmap: `HASHMAP_STR_DEFINE(V)` with content-hashed, owned keys
- Vector operations: `insert`, `remove`, `clear`, `reserve`, `extend`
- Full S-expression parser: `parse_sexp`/`serialize_sexp`/`sexp_eq`/`sexp_free` and tree constructors — the documented grammar is now fully implemented
- Allocator hooks: `CYAN_MALLOC`/`CYAN_REALLOC`/`CYAN_FREE`
- `CYAN_NO_SHORT_NAMES` to suppress the short lowercase names
- `WPTR_CLONE`/`weak_T_clone` for weak pointers

**Fixes:** 0.1.x -> 0.2.0 also includes the fixes from the 37-bug audit, among them: `defer_free` now NULLs the pointer after freeing, unbuffered channels get correct rendezvous/`CHAN_WOULD_BLOCK` semantics, overflow guards in growth and parsing paths, and single-evaluation convenience macros.

## License
MIT License - see LICENSE file for details.

# String

Heap-allocated, growable strings with safe operations. Defined in
`<cyan/string.h>`.

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

## Search, compare, trim, and split

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

## Materializing split pieces

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

## API reference

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
| `string_as_slice(s)` | View the whole string as a `Slice_char` |
| `string_concat(a, b)` | Concatenate two strings |
| `string_find(s, needle)` | Find substring, return index as `Option_size_t` |
| `string_contains(s, needle)` | Check if substring occurs |
| `string_starts_with(s, prefix)` | Check prefix |
| `string_ends_with(s, suffix)` | Check suffix |
| `string_trim(s)` | Strip leading/trailing whitespace in place |
| `string_eq(a, b)` | Content equality (NULL-tolerant) |
| `string_split_next(rest, delim, out)` | Advance split iterator over a `Slice_char` |
| `string_from_slice(slice)` | Owned, null-terminated String from a slice |
| `string_slice_eq(slice, cstr)` | Compare a slice to a C string without allocating |
| `string_free(s)` | Free string memory |
| `string_auto(name, init)` | Declare with auto-cleanup |

## Convenience macros

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

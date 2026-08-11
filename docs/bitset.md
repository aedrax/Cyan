# Bitset

Fixed-size bit collections for efficient flag management. Define bitsets
with `BITSET_DEFINE(N)` for N bits (1-64). Defined in `<cyan/bitset.h>`.

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

## Named flags

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

| Macro | Description |
|-------|-------------|
| `FLAGS_DEFINE(Name, ...)` | Define named flags with sequential bit positions |
| `FLAGS_SET(N, bs, flag)` | Set the specified flag |
| `FLAGS_CLEAR(N, bs, flag)` | Clear the specified flag |
| `FLAGS_HAS(N, bs, flag)` | Check if flag is set |

## API reference

| Function | Description |
|----------|-------------|
| `bitset_N_new()` | Create bitset with all bits cleared |
| `bitset_N_from_raw(value)` | Create bitset from raw integer value |
| `bitset_N_set(bs, index)` | Set bit at index (panics if out of bounds) |
| `bitset_N_clear(bs, index)` | Clear bit at index (panics if out of bounds) |
| `bitset_N_get(bs, index)` | Get bit at index as bool (panics if out of bounds) |
| `bitset_N_toggle(bs, index)` | Toggle bit at index (panics if out of bounds) |
| `bitset_N_union(a, b)` | Union of two bitsets (OR) |
| `bitset_N_intersect(a, b)` | Intersection of two bitsets (AND) |
| `bitset_N_diff(a, b)` | Difference (a AND NOT b) |
| `bitset_N_complement(bs)` | Complement (NOT, masked to N bits) |
| `bitset_N_eq(a, b)` | Check equality |
| `bitset_N_count(bs)` | Count set bits (popcount) |
| `bitset_N_all(bs)` | Check if all N bits are set |
| `bitset_N_any(bs)` | Check if any bit is set |
| `bitset_N_none(bs)` | Check if no bits are set |

## Convenience macros

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

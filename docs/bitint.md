# Bit-width integers

Zig-inspired integer types with arbitrary bit widths. Define custom
unsigned integers with `UINT_DEFINE(N)` (1-64 bits) and signed integers
with `INT_DEFINE(N)` (2-64 bits; a 1-bit signed integer is a compile
error). Defined in `<cyan/bitint.h>`.

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

## Unsigned integer API (UINT_DEFINE)

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

## Signed integer API (INT_DEFINE)

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

## Backing type selection

| Bit Width | Unsigned Backing | Signed Backing |
|-----------|------------------|----------------|
| 1-8       | u8               | i8             |
| 9-16      | u16              | i16            |
| 17-32     | u32              | i32            |
| 33-64     | u64              | i64            |

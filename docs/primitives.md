# Primitive types

Concise type names with predictable sizes, inspired by Rust and Zig.
Defined in `<cyan/common.h>`.

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

For type-erased pointers, use plain `void *`.

## Available types

| Category | Types |
|----------|-------|
| Signed integers | `i8`, `i16`, `i32`, `i64`, `i128`* |
| Unsigned integers | `u8`, `u16`, `u32`, `u64`, `u128`* |
| Pointer-sized | `isize`, `usize` |
| Floating-point | `f16`*, `f32`, `f64`, `f80`*, `f128`* |
| Special | `bool` |

*Platform-dependent. Check `CYAN_HAS_INT128`, `CYAN_HAS_FLOAT16`,
`CYAN_HAS_FLOAT80`, `CYAN_HAS_FLOAT128` macros (see
[Configuration](configuration.md)).

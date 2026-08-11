<div align="center">

<img src="assets/logo.svg" alt="cyan.h" width="300">

### Rust-grade ergonomics for C11, in a header-only include

Options and Results instead of sentinels. Panics instead of undefined
behavior. Vectors, slices, strings, hash maps and sets that check their
bounds. `defer`, smart pointers, pattern matching, and Go-style CSP with
coroutines and channels.

[![version](https://img.shields.io/badge/version-0.3.0-00bcd4?style=flat-square)](CHANGELOG.md)
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

- `Option_T` and `Result_T_E` make "no value" and "error" impossible to
  ignore silently, with `unwrap`, `expect`, `and_then`, `ok_or`, and
  `try_ok`/`try_some` early returns.
- Plain structs, zero machinery: no vtables, no hidden pointers, no
  runtime. An `Option_i32` is a `bool` and an `i32`, and every generated
  function is `static inline`.
- Every convenience macro evaluates each argument exactly once, and
  type-first naming (`VEC_PUSH(i32, v, 42)`) mirrors the constructors
  (`Some(i32, 42)`).
- Bounds-checked access returns Options, capacity math is
  overflow-guarded, and allocation failures panic instead of corrupting.
  141 property-based tests (theft) pass clean under ASan/UBSan.
- Channels called inside coroutines yield instead of failing; `coro_run`
  schedules them and detects deadlock.
- Configurable: custom panic handler, custom allocator hooks, and
  `CYAN_NO_SHORT_NAMES` if `map`/`filter`/`unwrap` would collide.

## Installation

Copy `include/cyan/` into your project. That's the whole install.

```c
#include <cyan/cyan.h>     // everything
// or pick modules:
#include <cyan/option.h>
#include <cyan/vector.h>
```

Compile with GCC or Clang (`-std=gnu11` or `-std=c11`; both accept the GNU
extensions used). On macOS add `-D_XOPEN_SOURCE=700` if you use coroutines
(see [Requirements](#requirements)).

## Sixty-second tour

```c
// Options and Results ------------------------------------------------
Option_i32 third = vec_i32_get(&v, 2);              // bounds-checked: Option
i32 x = unwrap_or(third, -1);                       // never a stray NULL

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

## Documentation

Every module has its own page in [`docs/`](docs/README.md), with runnable
counterparts in [`examples/`](examples/).

| Area | Pages |
|------|-------|
| Core | [Option](docs/option.md) · [Result](docs/result.md) · [Pattern matching](docs/match.md) · [Primitive types](docs/primitives.md) |
| Collections | [Vector](docs/vector.md) · [Slice](docs/slice.md) · [String](docs/string.md) · [HashMap](docs/hashmap.md) · [HashSet](docs/hashset.md) |
| Memory | [Defer](docs/defer.md) · [Smart pointers](docs/smartptr.md) |
| Concurrency | [Coroutines](docs/coroutines.md) · [Channels](docs/channels.md) |
| Utilities | [Functional](docs/functional.md) · [Serialization](docs/serialization.md) · [Bit-width integers](docs/bitint.md) · [Bitset](docs/bitset.md) |
| Library | [Method-style macros](docs/macros.md) · [Panic handler](docs/panic.md) · [Configuration](docs/configuration.md) · [Examples guide](docs/examples.md) |

Version history lives in the [changelog](CHANGELOG.md).

## Requirements

- C11 compatible compiler (GCC, Clang, or MSVC with C11 support)
- GCC/Clang recommended for:
  - `defer` and auto-cleanup features (uses `__attribute__((cleanup))`)
  - Statement expressions in pattern matching
  - Nested functions in defer
- POSIX system for coroutines (uses `ucontext.h`)
- pthreads for thread-safe channels
- **macOS note:** for the coroutine `ucontext` APIs to be visible, either include Cyan headers before any system header, or compile with `-D_XOPEN_SOURCE=700` (the project Makefile does this)

## Building and testing

```bash
make test              # 141 property-based tests (theft, vendored submodule)
make test SANITIZE=1   # the same suite under AddressSanitizer + UBSan
```

Property-based testing via [theft](https://github.com/silentbicycle/theft)
(`git submodule update --init` on first clone).

## Examples

Twenty runnable programs in [`examples/`](examples/), each an API tour
that ends with a realistic "putting it together" scenario. The
[examples guide](docs/examples.md) describes what each one covers.

```bash
cd examples
make        # build all
make run    # run all
```

## License

MIT License. See [LICENSE](LICENSE) for details.

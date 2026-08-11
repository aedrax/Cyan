# Cyan documentation

One page per module. Every module is a standalone header under
`include/cyan/`; include `<cyan/cyan.h>` to pull in everything at once, or
include individual headers to keep compile times down.

## Core

- [Primitive types](primitives.md): `i32`, `u64`, `f32` and friends
- [Option](option.md): explicit nullable values with combinators
- [Result](result.md): explicit error handling with `try_ok` early returns
- [Pattern matching](match.md): `match` over Option and Result

## Collections

- [Vector](vector.md): growable arrays with push/pop/insert/remove, find, sort, and iteration
- [Slice](slice.md): non-owning bounds-checked views
- [String](string.md): growable text with search, trim, split, and compare
- [HashMap](hashmap.md): open-addressing maps, a string-keyed variant, and iteration
- [HashSet](hashset.md): membership testing built on HashMap's engine

## Memory

- [Defer](defer.md): scope-exit cleanup, RAII-style
- [Smart pointers](smartptr.md): unique, shared, and weak pointers with auto-release

## Concurrency

- [Coroutines](coroutines.md): stackful cooperative multitasking over ucontext (experimental)
- [Channels](channels.md): CSP-style communication, coroutine-aware

## Utilities

- [Functional](functional.md): map, filter, reduce, foreach
- [Serialization](serialization.md): S-expressions with atoms, symbols, nested lists, and round-tripping
- [Bit-width integers](bitint.md): Zig-style `uN`/`iN` wrap-around integers
- [Bitset](bitset.md): fixed-size flag collections with named flags

## Library-wide

- [Method-style macros](macros.md): the type-first macro convention
- [Panic handler](panic.md): what panics, and how to override it
- [Configuration](configuration.md): compile-time options and feature detection
- [Examples](examples.md): the twenty runnable programs in `examples/`

Version history lives in the [changelog](../CHANGELOG.md).

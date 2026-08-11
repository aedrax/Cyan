# Changelog

## 0.3.0

**Additions:**

- **Coroutine-aware channels (headline).** In single-threaded mode, `chan_T_send`/`chan_T_recv` called inside a coroutine now yield-and-retry instead of returning `CHAN_WOULD_BLOCK`/`None`, giving Go-style CSP. Drive coroutines with the new `coro_run(coros, n)` round-robin scheduler, which returns `false` on deadlock (no completion and no channel progress in a full pass). New `coro_current()` and `coro_mark_progress()`. Behavior outside coroutines, for `try_*` variants, and in thread-safe mode is unchanged. Coroutines, their channels, and `coro_run` must share one translation unit.
- **HashSet** (`hashset.h`, included by `cyan.h`, `CYAN_HAS_HASHSET`): `HASHSET_DEFINE(T)` / `HASHSET_ITER_DEFINE(T)` with `add` (returns whether newly added), `contains`, `remove`, `len`, `free`, iteration, and type-first `SET_*` macros including `SET_FOREACH`.
- **Slice materialization:** `string_from_slice(Slice_char)` produces an owned, null-terminated `String` from a split piece; `string_slice_eq(slice, cstr)` compares without allocating.
- **Vector search/sort/iteration:** `vec_T_find(v, pred)` returning `Option_size_t`, `vec_T_contains(v, pred)`, `vec_T_sort(v, cmp)` (qsort-style comparator), plus `VEC_FIND`/`VEC_CONTAINS`/`VEC_SORT` and the `VEC_FOREACH(T, v, it)` loop macro.
- **HashMap iteration macro:** `MAP_FOREACH(K, V, m, pair)` over a pre-declared `MapPair_K_V pair;` (requires `HASHMAP_ITER_DEFINE`; single loop, so `break`/`continue` behave normally).

**Breaking change:**

- `Option_size_t` is now defined by `option.h` itself; remove any `OPTION_DEFINE(size_t)` from user code.

## 0.2.0

**Breaking changes:**

- **Vtable system removed.** All types are now plain structs (an `Option_i32` is just a `bool` + `i32`; a `Vec_i32` is just `data` + `len` + `cap`). Any `x.vt->fn(...)` or `ch->vt->fn(...)` call must become a standalone function call (`vec_i32_push(&v, x)`) or a convenience macro call (`VEC_PUSH(i32, v, x)`).
- **Container macros are now type-first.** `VEC_PUSH(v, 42)` becomes `VEC_PUSH(i32, v, 42)`; likewise for `MAP_*(K, V, ...)`, `SLICE_*(T, ...)`, `CHAN_*(T, ...)`, `BS_*(N, ...)`, `FLAGS_*(N, ...)`, `UPTR_*/SPTR_*/WPTR_*(T, ...)`. `STR_*` (monomorphic) and `OPT_*`/`RES_*` (typeless member access) are unchanged.
- **match.h sugar removed:** the no-op `some(var)`, `none()`, `ok(var)`, `err(var)` macros are gone; the `match_option`/`match_result` macros themselves are unchanged.
- **`any` type alias removed** from common.h; use plain `void *`.
- **Version macros moved to common.h** (they are no longer duplicated in other headers).

**Additions:**

- Option/Result combinators: `expect`, `and_then`, `or_else`, `ok_or`, `try_some`, `expect_ok`, `and_then_result`, `try_ok` (plus their always-available `OPT_*`/`RES_*` forms)
- String utilities: `string_find`, `string_contains`, `string_starts_with`, `string_ends_with`, `string_trim`, `string_eq`, `string_split_next`
- String-keyed hashmap: `HASHMAP_STR_DEFINE(V)` with content-hashed, owned keys
- Vector operations: `insert`, `remove`, `clear`, `reserve`, `extend`
- Full S-expression parser: `parse_sexp`/`serialize_sexp`/`sexp_eq`/`sexp_free` and tree constructors; the documented grammar is now fully implemented
- Allocator hooks: `CYAN_MALLOC`/`CYAN_REALLOC`/`CYAN_FREE`
- `CYAN_NO_SHORT_NAMES` to suppress the short lowercase names
- `WPTR_CLONE`/`weak_T_clone` for weak pointers

**Fixes:** 0.1.x -> 0.2.0 also includes the fixes from the 37-bug audit, among them: `defer_free` now NULLs the pointer after freeing, unbuffered channels get correct rendezvous/`CHAN_WOULD_BLOCK` semantics, overflow guards in growth and parsing paths, and single-evaluation convenience macros.

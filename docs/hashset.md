# HashSet

Type-safe hash sets sharing [HashMap](hashmap.md)'s open-addressing design
(power-of-two capacity, tombstones, load-factor resizing, optional custom
`hash_fn`/`equal_fn`). No `OPTION_DEFINE` prerequisite. Defined in
`<cyan/hashset.h>` and included by `cyan.h` (`CYAN_HAS_HASHSET`).

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

## API reference

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

Like `HASHMAP_DEFINE`, elements are hashed by their raw bytes, so the
pointer caveat for `char *` elements applies here too. `SET_FOREACH` uses
a GNU statement expression, so it is GCC/Clang only.

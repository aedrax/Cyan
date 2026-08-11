# HashMap

Type-safe hash maps with O(1) average lookups using FNV-1a hashing.
Defined in `<cyan/hashmap.h>`.

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

> **Warning:** `HASHMAP_DEFINE` hashes and compares the raw bytes of the
> key type. For pointer keys such as `char *`, that means the *pointer
> value* is hashed, not the pointed-to contents: two identical strings at
> different addresses are different keys. Use `HASHMAP_STR_DEFINE` for
> string keys.

Each map carries `hash_fn` and `equal_fn` fields (typedefs
`HashFn`/`EqualFn`) that default to FNV-1a over the key's bytes and
byte-wise equality; assign your own after creating the map to customize
hashing. The initial capacity (16) and the resize load factor (70%) are
tunable with `CYAN_HASHMAP_INITIAL_CAPACITY` and
`CYAN_HASHMAP_LOAD_FACTOR` (see [Configuration](configuration.md)).

## String-keyed HashMap

`HASHMAP_STR_DEFINE(V)` defines `HashMap_str_V` with content-hashed
`char *` keys:

```c
#include <cyan/hashmap.h>

OPTION_DEFINE(i32);        // Required before HASHMAP_STR_DEFINE(i32)
HASHMAP_STR_DEFINE(i32);   // HashMap_str_i32

i32 main(void) {
    HashMap_str_i32 ages = hashmap_str_i32_new();
    
    // insert COPIES the key: the map owns its copy
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

Key ownership rules:

- `hashmap_str_V_insert` copies the key string when the key is new (overwriting an existing key reuses the map's existing copy); the caller keeps ownership of the original.
- `hashmap_str_V_remove` and `hashmap_str_V_free` free the map's key copies.
- Keys are hashed and compared by content, so heap strings, stack buffers, and literals all work.

The function set matches the generic map, minus `with_capacity` and the
custom hash hooks (string keys always hash by content with FNV-1a and
compare with `strcmp`): `hashmap_str_V_new()`,
`hashmap_str_V_insert(m, key, value)`, `hashmap_str_V_get(m, key)`,
`hashmap_str_V_contains(m, key)`, `hashmap_str_V_remove(m, key)`,
`hashmap_str_V_len(m)`, `hashmap_str_V_free(m)`.

## Iteration with `MAP_FOREACH`

```c
HASHMAP_ITER_DEFINE(i32, i32);   // also defines MapPair_i32_i32

MapPair_i32_i32 pair;
MAP_FOREACH(i32, i32, m, pair) {
    printf("%d -> %d\n", pair.key, pair.value);   // break/continue work normally
}
```

`MAP_FOREACH` expands to a single loop, so `break` and `continue` behave
normally. It requires `HASHMAP_ITER_DEFINE`, a pre-declared pair variable,
and GCC/Clang (it uses a GNU statement expression).

## API reference

| Function | Description |
|----------|-------------|
| `hashmap_K_V_new()` | Create empty map |
| `hashmap_K_V_with_capacity(cap)` | Create map with initial capacity (rounded up to a power of two, minimum 16) |
| `hashmap_K_V_insert(m, key, value)` | Insert or update entry |
| `hashmap_K_V_get(m, key)` | Get value as Option |
| `hashmap_K_V_contains(m, key)` | Check if key exists |
| `hashmap_K_V_remove(m, key)` | Remove entry, return value as Option |
| `hashmap_K_V_len(m)` | Get number of entries |
| `hashmap_K_V_iter(m)` | Create iterator |
| `hashmap_K_V_iter_next(it)` | Get next key-value pair |
| `hashmap_K_V_free(m)` | Free map memory |

## Convenience macros

Macros take the key and value types first:

| Macro | Description |
|-------|-------------|
| `MAP_INSERT(K, V, m, k, val)` | Insert or update entry |
| `MAP_GET(K, V, m, k)` | Get value as Option |
| `MAP_CONTAINS(K, V, m, k)` | Check if key exists |
| `MAP_REMOVE(K, V, m, k)` | Remove entry, return value |
| `MAP_LEN(K, V, m)` | Get number of entries |
| `MAP_FOREACH(K, V, m, pair)` | Iterate over entries (see above) |
| `MAP_FREE(K, V, m)` | Free map memory |

/**
 * @file hashmap.h
 * @brief Type-safe hash map (dictionary) for the Cyan library
 *
 * This header provides a generic hash map implementation using open addressing
 * with linear probing for collision resolution. The map automatically resizes
 * when the load factor exceeds a threshold.
 *
 * Usage:
 *   OPTION_DEFINE(int);           // Required for Option_int
 *   HASHMAP_DEFINE(int, int);     // Define HashMap_int_int type
 *
 *   HashMap_int_int m = hashmap_int_int_new();
 *   hashmap_int_int_insert(&m, 42, 100);
 *   Option_int val = hashmap_int_int_get(&m, 42);
 *   hashmap_int_int_free(&m);
 *
 * String keys:
 *   HASHMAP_DEFINE hashes the raw bytes of the key type — for pointer keys
 *   (char *) that means the POINTER VALUE, not the pointed-to text. Use
 *   HASHMAP_STR_DEFINE(V) for content-hashed, owned string keys:
 *
 *   OPTION_DEFINE(int);
 *   HASHMAP_STR_DEFINE(int);      // Define HashMap_str_int
 *   HashMap_str_int m = hashmap_str_int_new();
 *   hashmap_str_int_insert(&m, "apple", 1);   // key is copied
 *   Option_int v = hashmap_str_int_get(&m, "apple");
 *   hashmap_str_int_free(&m);                 // frees owned keys
 */

#ifndef CYAN_HASHMAP_H
#define CYAN_HASHMAP_H

#include "common.h"
#include "option.h"
#include <string.h>

/*============================================================================
 * Hash Function Types
 *============================================================================*/

/**
 * @brief Hash function type
 * @param key Pointer to the key data
 * @param key_size Size of the key in bytes
 * @return Hash value
 */
typedef size_t (*HashFn)(const void *key, size_t key_size);

/**
 * @brief Equality function type
 * @param a Pointer to first key
 * @param b Pointer to second key
 * @param size Size of keys in bytes
 * @return true if keys are equal
 */
typedef bool (*EqualFn)(const void *a, const void *b, size_t size);

/*============================================================================
 * Default Hash Function (FNV-1a)
 *============================================================================*/

/**
 * @brief FNV-1a hash function
 *
 * A fast, non-cryptographic hash function with good distribution.
 * Uses the 64-bit FNV-1a algorithm.
 */
static inline size_t _cyan_fnv1a_hash(const void *key, size_t key_size) {
    const unsigned char *data = (const unsigned char *)key;
    size_t hash = 14695981039346656037ULL;  /* FNV offset basis */

    for (size_t i = 0; i < key_size; i++) {
        hash ^= data[i];
        hash *= 1099511628211ULL;  /* FNV prime */
    }

    return hash;
}

/**
 * @brief FNV-1a over a null-terminated string's content
 */
static inline size_t _cyan_fnv1a_hash_str(const char *s) {
    size_t hash = 14695981039346656037ULL;
    while (*s) {
        hash ^= (unsigned char)*s++;
        hash *= 1099511628211ULL;
    }
    return hash;
}

/**
 * @brief Default equality function using memcmp
 */
static inline bool _cyan_default_equal(const void *a, const void *b, size_t size) {
    return memcmp(a, b, size) == 0;
}

/**
 * @brief Duplicate a null-terminated string via CYAN_MALLOC
 */
static inline char *_cyan_strdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *copy = (char *)CYAN_MALLOC(n);
    if (!copy) CYAN_PANIC("allocation failed");
    memcpy(copy, s, n);
    return copy;
}

/*============================================================================
 * Configuration
 *============================================================================*/

/**
 * @brief Default initial capacity for hash maps
 */
#ifndef CYAN_HASHMAP_INITIAL_CAPACITY
#define CYAN_HASHMAP_INITIAL_CAPACITY 16
#endif

/**
 * @brief Load factor threshold for resizing (as percentage)
 * When (len * 100 / capacity) exceeds this, the map resizes
 */
#ifndef CYAN_HASHMAP_LOAD_FACTOR
#define CYAN_HASHMAP_LOAD_FACTOR 70
#endif

/*============================================================================
 * Entry States
 *============================================================================*/

/**
 * @brief State of a hash map entry
 */
typedef enum {
    _CYAN_ENTRY_EMPTY,     /* Slot never used */
    _CYAN_ENTRY_OCCUPIED,  /* Slot contains valid entry */
    _CYAN_ENTRY_DELETED    /* Slot was deleted (tombstone) */
} _CyanEntryState;

/*============================================================================
 * HashMap Type Definition Macro
 *============================================================================*/

/**
 * @brief Generate a HashMap type for given key and value types
 * @param K The key type
 * @param V The value type
 *
 * Creates:
 * - _MapEntry_K_V: Internal entry structure
 * - HashMap_K_V: The hash map structure
 * - hashmap_K_V_new(): Create empty map
 * - hashmap_K_V_with_capacity(cap): Create map with initial capacity
 * - hashmap_K_V_insert(m, key, value): Insert or update entry
 * - hashmap_K_V_get(m, key): Get value as Option
 * - hashmap_K_V_contains(m, key): Check if key exists
 * - hashmap_K_V_remove(m, key): Remove entry
 * - hashmap_K_V_len(m): Get number of entries
 * - hashmap_K_V_free(m): Free map memory
 *
 * Keys are hashed by their raw bytes (memcmp equality) unless custom
 * hash_fn/equal_fn are assigned before the first insert.
 *
 * Requires: OPTION_DEFINE(V) must be called before HASHMAP_DEFINE(K, V)
 */
#define HASHMAP_DEFINE(K, V) \
    /* Entry structure */ \
    typedef struct { \
        _CyanEntryState state; \
        K key; \
        V value; \
    } _MapEntry_##K##_##V; \
    \
    /** \
     * @brief HashMap structure \
     */ \
    typedef struct { \
        _MapEntry_##K##_##V *buckets; \
        size_t capacity; \
        size_t len; \
        HashFn hash_fn; \
        EqualFn equal_fn; \
    } HashMap_##K##_##V; \
    \
    /* Forward declaration for resize */ \
    CYAN_UNUSED static inline void _hashmap_##K##_##V##_resize(HashMap_##K##_##V *m, size_t new_cap); \
    \
    /* Internal: allocate a zeroed bucket array */ \
    CYAN_UNUSED static inline _MapEntry_##K##_##V *_hashmap_##K##_##V##_alloc_buckets(size_t n) { \
        if (n > SIZE_MAX / sizeof(_MapEntry_##K##_##V)) CYAN_PANIC("hashmap capacity overflow"); \
        _MapEntry_##K##_##V *b = \
            (_MapEntry_##K##_##V *)CYAN_MALLOC(n * sizeof(_MapEntry_##K##_##V)); \
        if (!b) CYAN_PANIC("allocation failed"); \
        memset(b, 0, n * sizeof(_MapEntry_##K##_##V)); \
        return b; \
    } \
    \
    /** \
     * @brief Create an empty hash map \
     * @return A new empty HashMap_K_V \
     */ \
    CYAN_UNUSED static inline HashMap_##K##_##V hashmap_##K##_##V##_new(void) { \
        HashMap_##K##_##V m = { \
            .buckets = NULL, \
            .capacity = 0, \
            .len = 0, \
            .hash_fn = _cyan_fnv1a_hash, \
            .equal_fn = _cyan_default_equal \
        }; \
        return m; \
    } \
    \
    /** \
     * @brief Create a hash map with pre-allocated capacity \
     * @param cap Initial capacity (will be rounded up to power of 2) \
     * @return A new HashMap_K_V with allocated storage \
     */ \
    CYAN_UNUSED static inline HashMap_##K##_##V hashmap_##K##_##V##_with_capacity(size_t cap) { \
        /* Round up to power of 2 */ \
        size_t actual_cap = CYAN_HASHMAP_INITIAL_CAPACITY; \
        while (actual_cap < cap) { \
            if (actual_cap > SIZE_MAX / 2) CYAN_PANIC("hashmap capacity overflow"); \
            actual_cap *= 2; \
        } \
        \
        HashMap_##K##_##V m = { \
            .buckets = _hashmap_##K##_##V##_alloc_buckets(actual_cap), \
            .capacity = actual_cap, \
            .len = 0, \
            .hash_fn = _cyan_fnv1a_hash, \
            .equal_fn = _cyan_default_equal \
        }; \
        return m; \
    } \
    \
    /** \
     * @brief Find the bucket index for a key \
     * @param m Pointer to the map \
     * @param key The key to find \
     * @param for_insert If true, returns first available slot; if false, returns exact match only \
     * @return Bucket index, or capacity if not found (when for_insert is false) \
     */ \
    CYAN_UNUSED static inline size_t _hashmap_##K##_##V##_find_bucket( \
        HashMap_##K##_##V *m, K key, bool for_insert \
    ) { \
        if (m->capacity == 0) return 0; \
        \
        size_t hash = m->hash_fn(&key, sizeof(K)); \
        size_t idx = hash & (m->capacity - 1); /* capacity is power of 2 */ \
        size_t first_deleted = m->capacity; /* sentinel for "not found" */ \
        \
        for (size_t i = 0; i < m->capacity; i++) { \
            size_t probe_idx = (idx + i) & (m->capacity - 1); \
            _MapEntry_##K##_##V *entry = &m->buckets[probe_idx]; \
            \
            if (entry->state == _CYAN_ENTRY_EMPTY) { \
                /* Empty slot - key not in map */ \
                if (for_insert) { \
                    return (first_deleted < m->capacity) ? first_deleted : probe_idx; \
                } \
                return m->capacity; /* Not found */ \
            } \
            \
            if (entry->state == _CYAN_ENTRY_DELETED) { \
                /* Remember first deleted slot for insertion */ \
                if (for_insert && first_deleted == m->capacity) { \
                    first_deleted = probe_idx; \
                } \
                continue; \
            } \
            \
            /* Occupied slot - check if key matches */ \
            if (m->equal_fn(&entry->key, &key, sizeof(K))) { \
                return probe_idx; /* Found */ \
            } \
        } \
        \
        /* Table is full (shouldn't happen with proper load factor) */ \
        if (for_insert && first_deleted < m->capacity) { \
            return first_deleted; \
        } \
        return m->capacity; \
    } \
    \
    /** \
     * @brief Resize the hash map \
     * @param m Pointer to the map \
     * @param new_cap New capacity (must be power of 2) \
     */ \
    CYAN_UNUSED static inline void _hashmap_##K##_##V##_resize(HashMap_##K##_##V *m, size_t new_cap) { \
        _MapEntry_##K##_##V *old_buckets = m->buckets; \
        size_t old_cap = m->capacity; \
        \
        m->buckets = _hashmap_##K##_##V##_alloc_buckets(new_cap); \
        m->capacity = new_cap; \
        m->len = 0; \
        \
        /* Rehash all existing entries */ \
        for (size_t i = 0; i < old_cap; i++) { \
            if (old_buckets[i].state == _CYAN_ENTRY_OCCUPIED) { \
                size_t idx = _hashmap_##K##_##V##_find_bucket(m, old_buckets[i].key, true); \
                m->buckets[idx].state = _CYAN_ENTRY_OCCUPIED; \
                m->buckets[idx].key = old_buckets[i].key; \
                m->buckets[idx].value = old_buckets[i].value; \
                m->len++; \
            } \
        } \
        \
        CYAN_FREE(old_buckets); \
    } \
    \
    /** \
     * @brief Insert or update a key-value pair \
     * @param m Pointer to the map \
     * @param key The key \
     * @param value The value \
     * @note Overwrites existing value if key already exists \
     */ \
    CYAN_UNUSED static inline void hashmap_##K##_##V##_insert(HashMap_##K##_##V *m, K key, V value) { \
        /* Initialize buckets in place if empty, preserving any custom \
         * hash_fn/equal_fn the user configured before the first insert */ \
        if (m->capacity == 0) { \
            m->buckets = _hashmap_##K##_##V##_alloc_buckets(CYAN_HASHMAP_INITIAL_CAPACITY); \
            m->capacity = CYAN_HASHMAP_INITIAL_CAPACITY; \
            m->len = 0; \
            if (!m->hash_fn) m->hash_fn = _cyan_fnv1a_hash; \
            if (!m->equal_fn) m->equal_fn = _cyan_default_equal; \
        } \
        \
        size_t idx = _hashmap_##K##_##V##_find_bucket(m, key, true); \
        bool is_new = idx >= m->capacity || \
                      m->buckets[idx].state != _CYAN_ENTRY_OCCUPIED; \
        \
        /* Only a new entry can push the load factor over the threshold; \
         * overwriting an existing key never requires a resize */ \
        if (is_new && (m->len + 1) * 100 / m->capacity > CYAN_HASHMAP_LOAD_FACTOR) { \
            _hashmap_##K##_##V##_resize(m, m->capacity * 2); \
            idx = _hashmap_##K##_##V##_find_bucket(m, key, true); \
        } \
        \
        if (m->buckets[idx].state != _CYAN_ENTRY_OCCUPIED) { \
            /* New entry */ \
            m->len++; \
        } \
        \
        m->buckets[idx].state = _CYAN_ENTRY_OCCUPIED; \
        m->buckets[idx].key = key; \
        m->buckets[idx].value = value; \
    } \
    \
    /** \
     * @brief Get value by key \
     * @param m Pointer to the map \
     * @param key The key to look up \
     * @return Option_V containing the value if found, None otherwise \
     */ \
    CYAN_UNUSED static inline Option_##V hashmap_##K##_##V##_get(HashMap_##K##_##V *m, K key) { \
        if (m->capacity == 0) return None(V); \
        \
        size_t idx = _hashmap_##K##_##V##_find_bucket(m, key, false); \
        if (idx >= m->capacity) return None(V); \
        \
        return Some(V, m->buckets[idx].value); \
    } \
    \
    /** \
     * @brief Check if key exists in map \
     * @param m Pointer to the map \
     * @param key The key to check \
     * @return true if key exists, false otherwise \
     */ \
    CYAN_UNUSED static inline bool hashmap_##K##_##V##_contains(HashMap_##K##_##V *m, K key) { \
        if (m->capacity == 0) return false; \
        \
        size_t idx = _hashmap_##K##_##V##_find_bucket(m, key, false); \
        return idx < m->capacity; \
    } \
    \
    /** \
     * @brief Remove a key from the map \
     * @param m Pointer to the map \
     * @param key The key to remove \
     * @return Option_V containing the removed value if found, None otherwise \
     */ \
    CYAN_UNUSED static inline Option_##V hashmap_##K##_##V##_remove(HashMap_##K##_##V *m, K key) { \
        if (m->capacity == 0) return None(V); \
        \
        size_t idx = _hashmap_##K##_##V##_find_bucket(m, key, false); \
        if (idx >= m->capacity) return None(V); \
        \
        V value = m->buckets[idx].value; \
        m->buckets[idx].state = _CYAN_ENTRY_DELETED; \
        m->len--; \
        \
        return Some(V, value); \
    } \
    \
    /** \
     * @brief Get the number of entries in the map \
     * @param m Pointer to the map \
     * @return Number of entries \
     */ \
    CYAN_UNUSED static inline size_t hashmap_##K##_##V##_len(HashMap_##K##_##V *m) { \
        return m->len; \
    } \
    \
    /** \
     * @brief Free all memory associated with the map \
     * @param m Pointer to the map \
     */ \
    CYAN_UNUSED static inline void hashmap_##K##_##V##_free(HashMap_##K##_##V *m) { \
        CYAN_FREE(m->buckets); \
        m->buckets = NULL; \
        m->capacity = 0; \
        m->len = 0; \
    } \
    /* Dummy typedef to absorb trailing semicolon */ \
    typedef HashMap_##K##_##V HashMap_##K##_##V##_defined

/*============================================================================
 * HashMap Iterator Definition Macro
 *============================================================================*/

/**
 * @brief Generate an iterator type for a HashMap
 * @param K The key type
 * @param V The value type
 *
 * Creates:
 * - HashMapIter_K_V: Iterator structure
 * - hashmap_K_V_iter(m): Create iterator from map
 * - hashmap_K_V_iter_next(it): Get next entry as Option
 *
 * Requires: HASHMAP_DEFINE(K, V) must be called first
 */
#define HASHMAP_ITER_DEFINE(K, V) \
    /* Key-value pair for iteration */ \
    typedef struct { \
        K key; \
        V value; \
    } _MapPair_##K##_##V; \
    \
    /* Public alias for user declarations (e.g. the MAP_FOREACH loop var) */ \
    typedef _MapPair_##K##_##V MapPair_##K##_##V; \
    \
    /* Define Option for the pair type */ \
    typedef struct { \
        bool has_value; \
        _MapPair_##K##_##V value; \
    } Option_MapPair_##K##_##V; \
    \
    /* Iterator structure */ \
    typedef struct { \
        HashMap_##K##_##V *map; \
        size_t index; \
    } HashMapIter_##K##_##V; \
    \
    /** \
     * @brief Create an iterator for a hash map \
     * @param m Pointer to the map \
     * @return Iterator positioned before the first element \
     */ \
    CYAN_UNUSED static inline HashMapIter_##K##_##V hashmap_##K##_##V##_iter(HashMap_##K##_##V *m) { \
        return (HashMapIter_##K##_##V){ .map = m, .index = 0 }; \
    } \
    \
    /** \
     * @brief Get the next entry from the iterator \
     * @param it Pointer to the iterator \
     * @return Option containing key-value pair, or None if iteration complete \
     */ \
    CYAN_UNUSED static inline Option_MapPair_##K##_##V hashmap_##K##_##V##_iter_next( \
        HashMapIter_##K##_##V *it \
    ) { \
        while (it->index < it->map->capacity) { \
            _MapEntry_##K##_##V *entry = &it->map->buckets[it->index]; \
            it->index++; \
            \
            if (entry->state == _CYAN_ENTRY_OCCUPIED) { \
                _MapPair_##K##_##V pair = { .key = entry->key, .value = entry->value }; \
                return (Option_MapPair_##K##_##V){ .has_value = true, .value = pair }; \
            } \
        } \
        \
        return (Option_MapPair_##K##_##V){ .has_value = false }; \
    } \
    /* Dummy typedef to absorb trailing semicolon */ \
    typedef HashMapIter_##K##_##V HashMapIter_##K##_##V##_defined

/*============================================================================
 * String-Keyed HashMap Definition Macro
 *============================================================================*/

/**
 * @brief Generate a string-keyed HashMap type
 * @param V The value type
 *
 * Creates HashMap_str_V with:
 * - Keys hashed by string CONTENT (FNV-1a) and compared with strcmp
 * - OWNED keys: the map copies the key string on a new-key insert and frees
 *   it on remove/free; overwriting an existing key keeps the original copy
 *
 * Functions:
 * - hashmap_str_V_new(), hashmap_str_V_insert(m, key, value),
 *   hashmap_str_V_get(m, key), hashmap_str_V_contains(m, key),
 *   hashmap_str_V_remove(m, key), hashmap_str_V_len(m), hashmap_str_V_free(m)
 *
 * Requires: OPTION_DEFINE(V) must be called before HASHMAP_STR_DEFINE(V)
 */
#define HASHMAP_STR_DEFINE(V) \
    typedef struct { \
        _CyanEntryState state; \
        char *key; \
        V value; \
    } _MapEntry_str_##V; \
    \
    typedef struct { \
        _MapEntry_str_##V *buckets; \
        size_t capacity; \
        size_t len; \
    } HashMap_str_##V; \
    \
    CYAN_UNUSED static inline _MapEntry_str_##V *_hashmap_str_##V##_alloc_buckets(size_t n) { \
        if (n > SIZE_MAX / sizeof(_MapEntry_str_##V)) CYAN_PANIC("hashmap capacity overflow"); \
        _MapEntry_str_##V *b = (_MapEntry_str_##V *)CYAN_MALLOC(n * sizeof(_MapEntry_str_##V)); \
        if (!b) CYAN_PANIC("allocation failed"); \
        memset(b, 0, n * sizeof(_MapEntry_str_##V)); \
        return b; \
    } \
    \
    CYAN_UNUSED static inline HashMap_str_##V hashmap_str_##V##_new(void) { \
        return (HashMap_str_##V){ .buckets = NULL, .capacity = 0, .len = 0 }; \
    } \
    \
    /** \
     * @brief Find bucket for a key (content comparison) \
     * @return Match/insert index, or capacity when not found (lookup mode) \
     */ \
    CYAN_UNUSED static inline size_t _hashmap_str_##V##_find_bucket( \
        HashMap_str_##V *m, const char *key, bool for_insert \
    ) { \
        if (m->capacity == 0) return 0; \
        \
        size_t hash = _cyan_fnv1a_hash_str(key); \
        size_t idx = hash & (m->capacity - 1); \
        size_t first_deleted = m->capacity; \
        \
        for (size_t i = 0; i < m->capacity; i++) { \
            size_t probe_idx = (idx + i) & (m->capacity - 1); \
            _MapEntry_str_##V *entry = &m->buckets[probe_idx]; \
            \
            if (entry->state == _CYAN_ENTRY_EMPTY) { \
                if (for_insert) { \
                    return (first_deleted < m->capacity) ? first_deleted : probe_idx; \
                } \
                return m->capacity; \
            } \
            if (entry->state == _CYAN_ENTRY_DELETED) { \
                if (for_insert && first_deleted == m->capacity) { \
                    first_deleted = probe_idx; \
                } \
                continue; \
            } \
            if (strcmp(entry->key, key) == 0) { \
                return probe_idx; \
            } \
        } \
        \
        if (for_insert && first_deleted < m->capacity) { \
            return first_deleted; \
        } \
        return m->capacity; \
    } \
    \
    CYAN_UNUSED static inline void _hashmap_str_##V##_resize(HashMap_str_##V *m, size_t new_cap) { \
        _MapEntry_str_##V *old_buckets = m->buckets; \
        size_t old_cap = m->capacity; \
        \
        m->buckets = _hashmap_str_##V##_alloc_buckets(new_cap); \
        m->capacity = new_cap; \
        m->len = 0; \
        \
        for (size_t i = 0; i < old_cap; i++) { \
            if (old_buckets[i].state == _CYAN_ENTRY_OCCUPIED) { \
                size_t idx = _hashmap_str_##V##_find_bucket(m, old_buckets[i].key, true); \
                m->buckets[idx].state = _CYAN_ENTRY_OCCUPIED; \
                m->buckets[idx].key = old_buckets[i].key; /* move ownership */ \
                m->buckets[idx].value = old_buckets[i].value; \
                m->len++; \
            } \
        } \
        \
        CYAN_FREE(old_buckets); \
    } \
    \
    /** \
     * @brief Insert or update; the key string is copied on a new insert \
     * @note key must be a null-terminated string (not NULL) \
     */ \
    CYAN_UNUSED static inline void hashmap_str_##V##_insert(HashMap_str_##V *m, const char *key, V value) { \
        if (!key) CYAN_PANIC("hashmap_str insert: NULL key"); \
        if (m->capacity == 0) { \
            m->buckets = _hashmap_str_##V##_alloc_buckets(CYAN_HASHMAP_INITIAL_CAPACITY); \
            m->capacity = CYAN_HASHMAP_INITIAL_CAPACITY; \
            m->len = 0; \
        } \
        \
        size_t idx = _hashmap_str_##V##_find_bucket(m, key, true); \
        bool is_new = idx >= m->capacity || \
                      m->buckets[idx].state != _CYAN_ENTRY_OCCUPIED; \
        \
        if (is_new && (m->len + 1) * 100 / m->capacity > CYAN_HASHMAP_LOAD_FACTOR) { \
            _hashmap_str_##V##_resize(m, m->capacity * 2); \
            idx = _hashmap_str_##V##_find_bucket(m, key, true); \
        } \
        \
        if (m->buckets[idx].state != _CYAN_ENTRY_OCCUPIED) { \
            m->buckets[idx].key = _cyan_strdup(key); \
            m->len++; \
        } \
        /* Overwrite keeps the existing owned key copy */ \
        m->buckets[idx].state = _CYAN_ENTRY_OCCUPIED; \
        m->buckets[idx].value = value; \
    } \
    \
    CYAN_UNUSED static inline Option_##V hashmap_str_##V##_get(HashMap_str_##V *m, const char *key) { \
        if (m->capacity == 0 || !key) return None(V); \
        size_t idx = _hashmap_str_##V##_find_bucket(m, key, false); \
        if (idx >= m->capacity) return None(V); \
        return Some(V, m->buckets[idx].value); \
    } \
    \
    CYAN_UNUSED static inline bool hashmap_str_##V##_contains(HashMap_str_##V *m, const char *key) { \
        if (m->capacity == 0 || !key) return false; \
        return _hashmap_str_##V##_find_bucket(m, key, false) < m->capacity; \
    } \
    \
    /** \
     * @brief Remove a key; frees the owned key copy \
     */ \
    CYAN_UNUSED static inline Option_##V hashmap_str_##V##_remove(HashMap_str_##V *m, const char *key) { \
        if (m->capacity == 0 || !key) return None(V); \
        size_t idx = _hashmap_str_##V##_find_bucket(m, key, false); \
        if (idx >= m->capacity) return None(V); \
        \
        V value = m->buckets[idx].value; \
        CYAN_FREE(m->buckets[idx].key); \
        m->buckets[idx].key = NULL; \
        m->buckets[idx].state = _CYAN_ENTRY_DELETED; \
        m->len--; \
        \
        return Some(V, value); \
    } \
    \
    CYAN_UNUSED static inline size_t hashmap_str_##V##_len(HashMap_str_##V *m) { \
        return m->len; \
    } \
    \
    /** \
     * @brief Free the map and every owned key copy \
     */ \
    CYAN_UNUSED static inline void hashmap_str_##V##_free(HashMap_str_##V *m) { \
        for (size_t i = 0; i < m->capacity; i++) { \
            if (m->buckets[i].state == _CYAN_ENTRY_OCCUPIED) { \
                CYAN_FREE(m->buckets[i].key); \
            } \
        } \
        CYAN_FREE(m->buckets); \
        m->buckets = NULL; \
        m->capacity = 0; \
        m->len = 0; \
    } \
    /* Dummy typedef to absorb trailing semicolon */ \
    typedef HashMap_str_##V HashMap_str_##V##_defined

/*============================================================================
 * HashMap Convenience Macros (type-first, like Ok(T, E, ...))
 *============================================================================
 * Each argument is evaluated exactly once.
 */

/**
 * @brief Insert a key-value pair into the map
 * @param K The key type
 * @param V The value type
 * @param m The map (an lvalue, not a pointer)
 * @param k The key
 * @param val The value
 */
#define MAP_INSERT(K, V, m, k, val) hashmap_##K##_##V##_insert(&(m), (k), (val))

/**
 * @brief Get value by key
 * @param K The key type
 * @param V The value type
 * @param m The map (an lvalue, not a pointer)
 * @param k The key
 * @return Option containing the value, or None if not found
 */
#define MAP_GET(K, V, m, k) hashmap_##K##_##V##_get(&(m), (k))

/**
 * @brief Check if key exists in map
 * @param K The key type
 * @param V The value type
 * @param m The map (an lvalue, not a pointer)
 * @param k The key
 * @return true if key exists, false otherwise
 */
#define MAP_CONTAINS(K, V, m, k) hashmap_##K##_##V##_contains(&(m), (k))

/**
 * @brief Remove a key from the map
 * @param K The key type
 * @param V The value type
 * @param m The map (an lvalue, not a pointer)
 * @param k The key
 * @return Option containing the removed value, or None if not found
 */
#define MAP_REMOVE(K, V, m, k) hashmap_##K##_##V##_remove(&(m), (k))

/**
 * @brief Get the number of entries
 * @param K The key type
 * @param V The value type
 * @param m The map (an lvalue, not a pointer)
 * @return Number of entries
 */
#define MAP_LEN(K, V, m) hashmap_##K##_##V##_len(&(m))

/**
 * @brief Free all memory associated with the map
 * @param K The key type
 * @param V The value type
 * @param m The map (an lvalue, not a pointer)
 */
#define MAP_FREE(K, V, m) hashmap_##K##_##V##_free(&(m))

/**
 * @brief Iterate over the map's live entries
 * @param K The key type
 * @param V The value type
 * @param m The map (an lvalue; must not be modified during iteration)
 * @param pair A pre-declared MapPair_K_V variable receiving each entry
 *
 * Requires HASHMAP_ITER_DEFINE(K, V). This is a single loop, so break and
 * continue behave like a normal for loop. GNU C only (statement expression
 * in the loop condition).
 *
 * Example:
 *   MapPair_int_int pair;
 *   MAP_FOREACH(int, int, m, pair) {
 *       printf("%d -> %d\n", pair.key, pair.value);
 *   }
 */
#if defined(__GNUC__) || defined(__clang__)
#define MAP_FOREACH(K, V, m, pair) \
    for (HashMapIter_##K##_##V CYAN_UNIQUE(_cyan_mit) = hashmap_##K##_##V##_iter(&(m)); \
         ({ Option_MapPair_##K##_##V _cyan_mo = \
                hashmap_##K##_##V##_iter_next(&CYAN_UNIQUE(_cyan_mit)); \
            if (_cyan_mo.has_value) (pair) = _cyan_mo.value; \
            _cyan_mo.has_value; }); )
#endif

#endif /* CYAN_HASHMAP_H */

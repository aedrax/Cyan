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
 * @brief Default equality function using memcmp
 */
static inline bool _cyan_default_equal(const void *a, const void *b, size_t size) {
    return memcmp(a, b, size) == 0;
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
 * - HashMapVT_K_V: Vtable structure with function pointers
 * - hashmap_K_V_new(): Create empty map
 * - hashmap_K_V_with_capacity(cap): Create map with initial capacity
 * - hashmap_K_V_insert(m, key, value): Insert or update entry
 * - hashmap_K_V_get(m, key): Get value as Option
 * - hashmap_K_V_contains(m, key): Check if key exists
 * - hashmap_K_V_remove(m, key): Remove entry
 * - hashmap_K_V_len(m): Get number of entries
 * - hashmap_K_V_free(m): Free map memory
 * 
 * Also generates a vtable struct HashMapVT_K_V and convenience macros:
 * - MAP_INSERT(m, k, v), MAP_GET(m, k), MAP_CONTAINS(m, k), MAP_REMOVE(m, k), MAP_LEN(m), MAP_FREE(m)
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
    /* Forward declare HashMap_K_V for use in vtable */ \
    typedef struct HashMap_##K##_##V HashMap_##K##_##V; \
    \
    /** \
     * @brief Vtable structure for HashMap_K_V containing function pointers \
     */ \
    typedef struct { \
        void (*insert)(HashMap_##K##_##V *m, K key, V value); \
        Option_##V (*get)(HashMap_##K##_##V *m, K key); \
        bool (*contains)(HashMap_##K##_##V *m, K key); \
        Option_##V (*remove)(HashMap_##K##_##V *m, K key); \
        size_t (*len)(HashMap_##K##_##V *m); \
        void (*free)(HashMap_##K##_##V *m); \
    } HashMapVT_##K##_##V; \
    \
    /** \
     * @brief HashMap structure with vtable pointer \
     */ \
    struct HashMap_##K##_##V { \
        _MapEntry_##K##_##V *buckets; \
        size_t capacity; \
        size_t len; \
        HashFn hash_fn; \
        EqualFn equal_fn; \
        const HashMapVT_##K##_##V *vt; \
    }; \
    \
    /* Forward declare vtable instance */ \
    static const HashMapVT_##K##_##V _hashmap_##K##_##V##_vt; \
    \
    /* Forward declaration for resize */ \
    static inline void _hashmap_##K##_##V##_resize(HashMap_##K##_##V *m, size_t new_cap); \
    \
    /** \
     * @brief Create an empty hash map \
     * @return A new empty HashMap_K_V \
     */ \
    static inline HashMap_##K##_##V hashmap_##K##_##V##_new(void) { \
        HashMap_##K##_##V m = { \
            .buckets = NULL, \
            .capacity = 0, \
            .len = 0, \
            .hash_fn = _cyan_fnv1a_hash, \
            .equal_fn = _cyan_default_equal, \
            .vt = &_hashmap_##K##_##V##_vt \
        }; \
        return m; \
    } \
    \
    /** \
     * @brief Create a hash map with pre-allocated capacity \
     * @param cap Initial capacity (will be rounded up to power of 2) \
     * @return A new HashMap_K_V with allocated storage \
     */ \
    static inline HashMap_##K##_##V hashmap_##K##_##V##_with_capacity(size_t cap) { \
        /* Round up to power of 2 */ \
        size_t actual_cap = CYAN_HASHMAP_INITIAL_CAPACITY; \
        while (actual_cap < cap) actual_cap *= 2; \
        \
        HashMap_##K##_##V m = { \
            .buckets = (_MapEntry_##K##_##V *)calloc(actual_cap, sizeof(_MapEntry_##K##_##V)), \
            .capacity = actual_cap, \
            .len = 0, \
            .hash_fn = _cyan_fnv1a_hash, \
            .equal_fn = _cyan_default_equal, \
            .vt = &_hashmap_##K##_##V##_vt \
        }; \
        if (!m.buckets) CYAN_PANIC("allocation failed"); \
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
    static inline size_t _hashmap_##K##_##V##_find_bucket( \
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
    static inline void _hashmap_##K##_##V##_resize(HashMap_##K##_##V *m, size_t new_cap) { \
        _MapEntry_##K##_##V *old_buckets = m->buckets; \
        size_t old_cap = m->capacity; \
        \
        m->buckets = (_MapEntry_##K##_##V *)calloc(new_cap, sizeof(_MapEntry_##K##_##V)); \
        if (!m->buckets) { \
            m->buckets = old_buckets; \
            CYAN_PANIC("allocation failed"); \
        } \
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
        free(old_buckets); \
    } \
    \
    /** \
     * @brief Insert or update a key-value pair \
     * @param m Pointer to the map \
     * @param key The key \
     * @param value The value \
     * @note Overwrites existing value if key already exists \
     */ \
    static inline void hashmap_##K##_##V##_insert(HashMap_##K##_##V *m, K key, V value) { \
        /* Initialize if empty */ \
        if (m->capacity == 0) { \
            *m = hashmap_##K##_##V##_with_capacity(CYAN_HASHMAP_INITIAL_CAPACITY); \
        } \
        \
        /* Check load factor and resize if needed */ \
        if ((m->len + 1) * 100 / m->capacity > CYAN_HASHMAP_LOAD_FACTOR) { \
            _hashmap_##K##_##V##_resize(m, m->capacity * 2); \
        } \
        \
        size_t idx = _hashmap_##K##_##V##_find_bucket(m, key, true); \
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
    static inline Option_##V hashmap_##K##_##V##_get(HashMap_##K##_##V *m, K key) { \
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
    static inline bool hashmap_##K##_##V##_contains(HashMap_##K##_##V *m, K key) { \
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
    static inline Option_##V hashmap_##K##_##V##_remove(HashMap_##K##_##V *m, K key) { \
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
    static inline size_t hashmap_##K##_##V##_len(HashMap_##K##_##V *m) { \
        return m->len; \
    } \
    \
    /** \
     * @brief Free all memory associated with the map \
     * @param m Pointer to the map \
     */ \
    static inline void hashmap_##K##_##V##_free(HashMap_##K##_##V *m) { \
        free(m->buckets); \
        m->buckets = NULL; \
        m->capacity = 0; \
        m->len = 0; \
    } \
    \
    /** \
     * @brief Static const vtable instance shared by all HashMap_K_V instances \
     */ \
    static const HashMapVT_##K##_##V _hashmap_##K##_##V##_vt = { \
        .insert = hashmap_##K##_##V##_insert, \
        .get = hashmap_##K##_##V##_get, \
        .contains = hashmap_##K##_##V##_contains, \
        .remove = hashmap_##K##_##V##_remove, \
        .len = hashmap_##K##_##V##_len, \
        .free = hashmap_##K##_##V##_free \
    }; \
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
    static inline HashMapIter_##K##_##V hashmap_##K##_##V##_iter(HashMap_##K##_##V *m) { \
        return (HashMapIter_##K##_##V){ .map = m, .index = 0 }; \
    } \
    \
    /** \
     * @brief Get the next entry from the iterator \
     * @param it Pointer to the iterator \
     * @return Option containing key-value pair, or None if iteration complete \
     */ \
    static inline Option_MapPair_##K##_##V hashmap_##K##_##V##_iter_next( \
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
 * HashMap Convenience Macros
 *============================================================================*/

/**
 * @brief Insert a key-value pair into the map via vtable
 * @param m The map (not a pointer)
 * @param k The key
 * @param val The value
 */
#define MAP_INSERT(m, k, val) ((m).vt->insert(&(m), (k), (val)))

/**
 * @brief Get value by key via vtable
 * @param m The map (not a pointer)
 * @param k The key
 * @return Option containing the value, or None if not found
 */
#define MAP_GET(m, k) ((m).vt->get(&(m), (k)))

/**
 * @brief Check if key exists in map via vtable
 * @param m The map (not a pointer)
 * @param k The key
 * @return true if key exists, false otherwise
 */
#define MAP_CONTAINS(m, k) ((m).vt->contains(&(m), (k)))

/**
 * @brief Remove a key from the map via vtable
 * @param m The map (not a pointer)
 * @param k The key
 * @return Option containing the removed value, or None if not found
 */
#define MAP_REMOVE(m, k) ((m).vt->remove(&(m), (k)))

/**
 * @brief Get the number of entries via vtable
 * @param m The map (not a pointer)
 * @return Number of entries
 */
#define MAP_LEN(m) ((m).vt->len(&(m)))

/**
 * @brief Free all memory associated with the map via vtable
 * @param m The map (not a pointer)
 */
#define MAP_FREE(m) ((m).vt->free(&(m)))

#endif /* CYAN_HASHMAP_H */

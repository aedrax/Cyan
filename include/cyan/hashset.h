/**
 * @file hashset.h
 * @brief Type-safe hash set for the Cyan library
 *
 * This header provides a generic hash set implementation using open
 * addressing with linear probing, sharing the collision and resize
 * strategy of hashmap.h.
 *
 * Usage:
 *   HASHSET_DEFINE(int);          // Define HashSet_int type
 *
 *   HashSet_int s = hashset_int_new();
 *   hashset_int_add(&s, 42);      // true (newly added)
 *   hashset_int_add(&s, 42);      // false (already present)
 *   bool has = hashset_int_contains(&s, 42);
 *   hashset_int_remove(&s, 42);   // true (was present)
 *   hashset_int_free(&s);
 *
 * Like HASHMAP_DEFINE, elements are hashed by their raw bytes (memcmp
 * equality) unless custom hash_fn/equal_fn are assigned before the
 * first add.
 */

#ifndef CYAN_HASHSET_H
#define CYAN_HASHSET_H

#include "common.h"
#include "hashmap.h" /* HashFn/EqualFn, _CyanEntryState, FNV-1a, config */
#include <string.h>

/*============================================================================
 * HashSet Type Definition Macro
 *============================================================================*/

/**
 * @brief Generate a HashSet type for a given element type
 * @param T The element type
 *
 * Creates:
 * - _SetEntry_T: Internal entry structure
 * - HashSet_T: The hash set structure
 * - hashset_T_new(): Create empty set
 * - hashset_T_add(s, elem): Add element; returns true if newly added
 * - hashset_T_contains(s, elem): Membership test
 * - hashset_T_remove(s, elem): Remove element; returns true if it was present
 * - hashset_T_len(s): Number of elements
 * - hashset_T_free(s): Free set memory
 */
#define HASHSET_DEFINE(T) \
    /* Entry structure */ \
    typedef struct { \
        _CyanEntryState state; \
        T key; \
    } _SetEntry_##T; \
    \
    /** \
     * @brief HashSet structure \
     */ \
    typedef struct { \
        _SetEntry_##T *buckets; \
        size_t capacity; \
        size_t len; \
        HashFn hash_fn; \
        EqualFn equal_fn; \
    } HashSet_##T; \
    \
    /* Internal: allocate a zeroed bucket array */ \
    CYAN_UNUSED static inline _SetEntry_##T *_hashset_##T##_alloc_buckets(size_t n) { \
        if (n > SIZE_MAX / sizeof(_SetEntry_##T)) CYAN_PANIC("hashset capacity overflow"); \
        _SetEntry_##T *b = (_SetEntry_##T *)CYAN_MALLOC(n * sizeof(_SetEntry_##T)); \
        if (!b) CYAN_PANIC("allocation failed"); \
        memset(b, 0, n * sizeof(_SetEntry_##T)); \
        return b; \
    } \
    \
    /** \
     * @brief Create an empty hash set \
     * @return A new empty HashSet_T \
     */ \
    CYAN_UNUSED static inline HashSet_##T hashset_##T##_new(void) { \
        return (HashSet_##T){ \
            .buckets = NULL, \
            .capacity = 0, \
            .len = 0, \
            .hash_fn = _cyan_fnv1a_hash, \
            .equal_fn = _cyan_default_equal \
        }; \
    } \
    \
    /** \
     * @brief Find the bucket index for an element \
     * @param for_insert If true, returns first available slot; otherwise \
     *        exact match only (capacity when not found) \
     */ \
    CYAN_UNUSED static inline size_t _hashset_##T##_find_bucket( \
        HashSet_##T *s, T key, bool for_insert \
    ) { \
        if (s->capacity == 0) return 0; \
        \
        size_t hash = s->hash_fn(&key, sizeof(T)); \
        size_t idx = hash & (s->capacity - 1); /* capacity is power of 2 */ \
        size_t first_deleted = s->capacity; \
        \
        for (size_t i = 0; i < s->capacity; i++) { \
            size_t probe_idx = (idx + i) & (s->capacity - 1); \
            _SetEntry_##T *entry = &s->buckets[probe_idx]; \
            \
            if (entry->state == _CYAN_ENTRY_EMPTY) { \
                if (for_insert) { \
                    return (first_deleted < s->capacity) ? first_deleted : probe_idx; \
                } \
                return s->capacity; \
            } \
            if (entry->state == _CYAN_ENTRY_DELETED) { \
                if (for_insert && first_deleted == s->capacity) { \
                    first_deleted = probe_idx; \
                } \
                continue; \
            } \
            if (s->equal_fn(&entry->key, &key, sizeof(T))) { \
                return probe_idx; \
            } \
        } \
        \
        if (for_insert && first_deleted < s->capacity) { \
            return first_deleted; \
        } \
        return s->capacity; \
    } \
    \
    /** \
     * @brief Resize the set (new_cap must be a power of 2) \
     */ \
    CYAN_UNUSED static inline void _hashset_##T##_resize(HashSet_##T *s, size_t new_cap) { \
        _SetEntry_##T *old_buckets = s->buckets; \
        size_t old_cap = s->capacity; \
        \
        s->buckets = _hashset_##T##_alloc_buckets(new_cap); \
        s->capacity = new_cap; \
        s->len = 0; \
        \
        for (size_t i = 0; i < old_cap; i++) { \
            if (old_buckets[i].state == _CYAN_ENTRY_OCCUPIED) { \
                size_t idx = _hashset_##T##_find_bucket(s, old_buckets[i].key, true); \
                s->buckets[idx].state = _CYAN_ENTRY_OCCUPIED; \
                s->buckets[idx].key = old_buckets[i].key; \
                s->len++; \
            } \
        } \
        \
        CYAN_FREE(old_buckets); \
    } \
    \
    /** \
     * @brief Add an element to the set \
     * @param s Pointer to the set \
     * @param key The element to add \
     * @return true if the element was newly added, false if already present \
     */ \
    CYAN_UNUSED static inline bool hashset_##T##_add(HashSet_##T *s, T key) { \
        /* Initialize buckets in place, preserving custom hash_fn/equal_fn */ \
        if (s->capacity == 0) { \
            s->buckets = _hashset_##T##_alloc_buckets(CYAN_HASHMAP_INITIAL_CAPACITY); \
            s->capacity = CYAN_HASHMAP_INITIAL_CAPACITY; \
            s->len = 0; \
            if (!s->hash_fn) s->hash_fn = _cyan_fnv1a_hash; \
            if (!s->equal_fn) s->equal_fn = _cyan_default_equal; \
        } \
        \
        size_t idx = _hashset_##T##_find_bucket(s, key, true); \
        bool is_new = idx >= s->capacity || \
                      s->buckets[idx].state != _CYAN_ENTRY_OCCUPIED; \
        if (!is_new) return false; \
        \
        /* Only a new element can push the load factor over the threshold */ \
        if ((s->len + 1) * 100 / s->capacity > CYAN_HASHMAP_LOAD_FACTOR) { \
            _hashset_##T##_resize(s, s->capacity * 2); \
            idx = _hashset_##T##_find_bucket(s, key, true); \
        } \
        \
        s->buckets[idx].state = _CYAN_ENTRY_OCCUPIED; \
        s->buckets[idx].key = key; \
        s->len++; \
        return true; \
    } \
    \
    /** \
     * @brief Check whether an element is in the set \
     * @param s Pointer to the set \
     * @param key The element to look up \
     * @return true if present \
     */ \
    CYAN_UNUSED static inline bool hashset_##T##_contains(HashSet_##T *s, T key) { \
        if (s->capacity == 0) return false; \
        return _hashset_##T##_find_bucket(s, key, false) < s->capacity; \
    } \
    \
    /** \
     * @brief Remove an element from the set \
     * @param s Pointer to the set \
     * @param key The element to remove \
     * @return true if the element was present and removed \
     */ \
    CYAN_UNUSED static inline bool hashset_##T##_remove(HashSet_##T *s, T key) { \
        if (s->capacity == 0) return false; \
        \
        size_t idx = _hashset_##T##_find_bucket(s, key, false); \
        if (idx >= s->capacity) return false; \
        \
        s->buckets[idx].state = _CYAN_ENTRY_DELETED; \
        s->len--; \
        return true; \
    } \
    \
    /** \
     * @brief Get the number of elements in the set \
     */ \
    CYAN_UNUSED static inline size_t hashset_##T##_len(HashSet_##T *s) { \
        return s->len; \
    } \
    \
    /** \
     * @brief Free all memory associated with the set \
     */ \
    CYAN_UNUSED static inline void hashset_##T##_free(HashSet_##T *s) { \
        CYAN_FREE(s->buckets); \
        s->buckets = NULL; \
        s->capacity = 0; \
        s->len = 0; \
    } \
    /* Dummy typedef to absorb trailing semicolon */ \
    typedef HashSet_##T HashSet_##T##_defined

/*============================================================================
 * HashSet Iterator Definition Macro
 *============================================================================*/

/**
 * @brief Generate an iterator type for a HashSet
 * @param T The element type
 *
 * Creates:
 * - HashSetIter_T: Iterator structure
 * - hashset_T_iter(s): Create iterator from set
 * - hashset_T_iter_next(it): Get next element as an option-like struct
 *
 * Requires: HASHSET_DEFINE(T) must be called first
 */
#define HASHSET_ITER_DEFINE(T) \
    /* Option-like result for iteration (independent of OPTION_DEFINE) */ \
    typedef struct { \
        bool has_value; \
        T value; \
    } Option_SetItem_##T; \
    \
    /* Iterator structure */ \
    typedef struct { \
        HashSet_##T *set; \
        size_t index; \
    } HashSetIter_##T; \
    \
    /** \
     * @brief Create an iterator for a hash set \
     */ \
    CYAN_UNUSED static inline HashSetIter_##T hashset_##T##_iter(HashSet_##T *s) { \
        return (HashSetIter_##T){ .set = s, .index = 0 }; \
    } \
    \
    /** \
     * @brief Get the next element from the iterator \
     * @return has_value == false when iteration is complete \
     */ \
    CYAN_UNUSED static inline Option_SetItem_##T hashset_##T##_iter_next( \
        HashSetIter_##T *it \
    ) { \
        while (it->index < it->set->capacity) { \
            _SetEntry_##T *entry = &it->set->buckets[it->index]; \
            it->index++; \
            if (entry->state == _CYAN_ENTRY_OCCUPIED) { \
                return (Option_SetItem_##T){ .has_value = true, .value = entry->key }; \
            } \
        } \
        return (Option_SetItem_##T){ .has_value = false }; \
    } \
    /* Dummy typedef to absorb trailing semicolon */ \
    typedef HashSetIter_##T HashSetIter_##T##_defined

/*============================================================================
 * HashSet Convenience Macros (type-first, like Some(T, ...))
 *============================================================================
 * Each argument is evaluated exactly once.
 */

/**
 * @brief Add an element to the set
 * @param T The element type
 * @param s The set (an lvalue, not a pointer)
 * @param key The element
 * @return true if newly added
 */
#define SET_ADD(T, s, key) hashset_##T##_add(&(s), (key))

/**
 * @brief Check whether an element is in the set
 * @param T The element type
 * @param s The set (an lvalue, not a pointer)
 * @param key The element
 * @return true if present
 */
#define SET_CONTAINS(T, s, key) hashset_##T##_contains(&(s), (key))

/**
 * @brief Remove an element from the set
 * @param T The element type
 * @param s The set (an lvalue, not a pointer)
 * @param key The element
 * @return true if it was present
 */
#define SET_REMOVE(T, s, key) hashset_##T##_remove(&(s), (key))

/**
 * @brief Get the number of elements
 * @param T The element type
 * @param s The set (an lvalue, not a pointer)
 */
#define SET_LEN(T, s) hashset_##T##_len(&(s))

/**
 * @brief Free all memory associated with the set
 * @param T The element type
 * @param s The set (an lvalue, not a pointer)
 */
#define SET_FREE(T, s) hashset_##T##_free(&(s))

/**
 * @brief Iterate over the set's elements
 * @param T The element type
 * @param s The set (an lvalue; must not be modified during iteration)
 * @param item A pre-declared T variable receiving each element
 *
 * Requires HASHSET_ITER_DEFINE(T). Single loop: break/continue behave
 * normally. GNU C only (statement expression in the loop condition).
 *
 * Example:
 *   int item;
 *   SET_FOREACH(int, s, item) { printf("%d\n", item); }
 */
#if defined(__GNUC__) || defined(__clang__)
#define SET_FOREACH(T, s, item) \
    for (HashSetIter_##T CYAN_UNIQUE(_cyan_sit) = hashset_##T##_iter(&(s)); \
         ({ Option_SetItem_##T _cyan_so = \
                hashset_##T##_iter_next(&CYAN_UNIQUE(_cyan_sit)); \
            if (_cyan_so.has_value) (item) = _cyan_so.value; \
            _cyan_so.has_value; }); )
#endif

#endif /* CYAN_HASHSET_H */

/**
 * @file vector.h
 * @brief Generic dynamic array (vector) type for the Cyan library
 *
 * This header provides a type-safe dynamic array implementation using macros.
 * Vectors automatically grow as elements are added and provide bounds-checked
 * access returning Option types.
 *
 * Usage:
 *   OPTION_DEFINE(int);   // Required for Option_int
 *   VECTOR_DEFINE(int);   // Define Vec_int type
 *
 *   Vec_int v = vec_int_new();
 *   vec_int_push(&v, 42);
 *   Option_int elem = vec_int_get(&v, 0);
 *   vec_int_free(&v);
 */

#ifndef CYAN_VECTOR_H
#define CYAN_VECTOR_H

#include "common.h"
#include "option.h"
#include <string.h>

/*============================================================================
 * Vector Type Definition
 *============================================================================*/

/**
 * @brief Generate a Vector type for a given element type
 * @param T The element type
 *
 * Creates a struct Vec_T with:
 * - data: pointer to element array
 * - len: current number of elements
 * - cap: current capacity
 *
 * Also generates the following functions:
 * - vec_T_new(): Create empty vector
 * - vec_T_with_capacity(cap): Create vector with initial capacity
 * - vec_T_push(v, elem): Append element
 * - vec_T_pop(v): Remove and return last element as Option
 * - vec_T_get(v, idx): Get element at index as Option
 * - vec_T_insert(v, idx, elem): Insert element at index (shifts tail up)
 * - vec_T_remove(v, idx): Remove element at index as Option (shifts tail down)
 * - vec_T_extend(v, data, n): Append n elements from an array/slice data
 * - vec_T_reserve(v, min_cap): Ensure capacity of at least min_cap
 * - vec_T_clear(v): Set length to 0 (keeps capacity)
 * - vec_T_len(v): Get current length
 * - vec_T_free(v): Free vector memory
 *
 * Requires: OPTION_DEFINE(T) must be called before VECTOR_DEFINE(T)
 *
 * Example:
 *   OPTION_DEFINE(int);
 *   VECTOR_DEFINE(int);  // Creates Vec_int and vec_int_* functions
 */
#define VECTOR_DEFINE(T) \
    typedef struct { \
        T *data; \
        size_t len; \
        size_t cap; \
    } Vec_##T; \
    \
    /** \
     * @brief Create an empty vector \
     * @return A new empty Vec_T with no allocated storage \
     */ \
    CYAN_UNUSED static inline Vec_##T vec_##T##_new(void) { \
        return (Vec_##T){ .data = NULL, .len = 0, .cap = 0 }; \
    } \
    \
    /** \
     * @brief Create a vector with pre-allocated capacity \
     * @param cap Initial capacity \
     * @return A new Vec_T with allocated storage for cap elements \
     * @note Panics if allocation fails \
     */ \
    CYAN_UNUSED static inline Vec_##T vec_##T##_with_capacity(size_t cap) { \
        Vec_##T v = { .data = NULL, .len = 0, .cap = cap }; \
        if (cap > 0) { \
            if (cap > SIZE_MAX / sizeof(T)) CYAN_PANIC("vector capacity overflow"); \
            v.data = (T *)CYAN_MALLOC(cap * sizeof(T)); \
            if (!v.data) CYAN_PANIC("allocation failed"); \
        } \
        return v; \
    } \
    \
    /** \
     * @brief Ensure capacity of at least min_cap elements \
     * @param v Pointer to the vector \
     * @param min_cap Minimum required capacity \
     * @note Panics on overflow or allocation failure; never shrinks \
     */ \
    CYAN_UNUSED static inline void vec_##T##_reserve(Vec_##T *v, size_t min_cap) { \
        if (min_cap <= v->cap) return; \
        if (min_cap > SIZE_MAX / sizeof(T)) CYAN_PANIC("vector capacity overflow"); \
        T *new_data = (T *)CYAN_REALLOC(v->data, min_cap * sizeof(T)); \
        if (!new_data) CYAN_PANIC("allocation failed"); \
        v->data = new_data; \
        v->cap = min_cap; \
    } \
    \
    /* Internal: grow geometrically to fit one more element */ \
    CYAN_UNUSED static inline void _vec_##T##_grow_for_push(Vec_##T *v) { \
        if (v->len < v->cap) return; \
        size_t new_cap; \
        if (v->cap == 0) { \
            new_cap = CYAN_DEFAULT_CAPACITY; \
        } else if (v->cap > SIZE_MAX / CYAN_GROWTH_FACTOR) { \
            CYAN_PANIC("vector capacity overflow"); \
            return; \
        } else { \
            new_cap = v->cap * CYAN_GROWTH_FACTOR; \
        } \
        vec_##T##_reserve(v, new_cap); \
    } \
    \
    /** \
     * @brief Append an element to the vector \
     * @param v Pointer to the vector \
     * @param elem Element to append \
     * @note Automatically grows capacity if needed \
     * @note Panics if allocation fails during growth \
     */ \
    CYAN_UNUSED static inline void vec_##T##_push(Vec_##T *v, T elem) { \
        _vec_##T##_grow_for_push(v); \
        v->data[v->len++] = elem; \
    } \
    \
    /** \
     * @brief Remove and return the last element \
     * @param v Pointer to the vector \
     * @return Option_T containing the last element, or None if empty \
     */ \
    CYAN_UNUSED static inline Option_##T vec_##T##_pop(Vec_##T *v) { \
        if (v->len == 0) return None(T); \
        return Some(T, v->data[--v->len]); \
    } \
    \
    /** \
     * @brief Get element at index with bounds checking \
     * @param v Pointer to the vector \
     * @param idx Index to access \
     * @return Option_T containing the element, or None if out of bounds \
     */ \
    CYAN_UNUSED static inline Option_##T vec_##T##_get(Vec_##T *v, size_t idx) { \
        if (idx >= v->len) return None(T); \
        return Some(T, v->data[idx]); \
    } \
    \
    /** \
     * @brief Insert an element at an index, shifting later elements up \
     * @param v Pointer to the vector \
     * @param idx Insertion index (clamped to len, i.e. idx == len appends) \
     * @param elem Element to insert \
     */ \
    CYAN_UNUSED static inline void vec_##T##_insert(Vec_##T *v, size_t idx, T elem) { \
        if (idx > v->len) idx = v->len; \
        _vec_##T##_grow_for_push(v); \
        memmove(v->data + idx + 1, v->data + idx, (v->len - idx) * sizeof(T)); \
        v->data[idx] = elem; \
        v->len++; \
    } \
    \
    /** \
     * @brief Remove the element at an index, shifting later elements down \
     * @param v Pointer to the vector \
     * @param idx Index to remove \
     * @return Option_T containing the removed element, or None if out of bounds \
     */ \
    CYAN_UNUSED static inline Option_##T vec_##T##_remove(Vec_##T *v, size_t idx) { \
        if (idx >= v->len) return None(T); \
        T removed = v->data[idx]; \
        memmove(v->data + idx, v->data + idx + 1, (v->len - idx - 1) * sizeof(T)); \
        v->len--; \
        return Some(T, removed); \
    } \
    \
    /** \
     * @brief Append n elements from an array (works with slice data/len) \
     * @param v Pointer to the vector \
     * @param src Pointer to the source elements (may be NULL when n is 0) \
     * @param n Number of elements to append \
     */ \
    CYAN_UNUSED static inline void vec_##T##_extend(Vec_##T *v, const T *src, size_t n) { \
        if (!src || n == 0) return; \
        if (n > SIZE_MAX - v->len) CYAN_PANIC("vector capacity overflow"); \
        if (v->len + n > v->cap) { \
            size_t new_cap = v->cap == 0 ? CYAN_DEFAULT_CAPACITY : v->cap; \
            while (new_cap < v->len + n) { \
                if (new_cap > SIZE_MAX / CYAN_GROWTH_FACTOR) { \
                    new_cap = v->len + n; \
                    break; \
                } \
                new_cap *= CYAN_GROWTH_FACTOR; \
            } \
            vec_##T##_reserve(v, new_cap); \
        } \
        memmove(v->data + v->len, src, n * sizeof(T)); \
        v->len += n; \
    } \
    \
    /** \
     * @brief Remove all elements (keeps allocated capacity) \
     * @param v Pointer to the vector \
     */ \
    CYAN_UNUSED static inline void vec_##T##_clear(Vec_##T *v) { \
        v->len = 0; \
    } \
    \
    /** \
     * @brief Get the current number of elements \
     * @param v Pointer to the vector \
     * @return Current length \
     */ \
    CYAN_UNUSED static inline size_t vec_##T##_len(Vec_##T *v) { \
        return v->len; \
    } \
    \
    /** \
     * @brief Free all memory associated with the vector \
     * @param v Pointer to the vector \
     * @note Resets the vector to empty state \
     */ \
    CYAN_UNUSED static inline void vec_##T##_free(Vec_##T *v) { \
        CYAN_FREE(v->data); \
        v->data = NULL; \
        v->len = 0; \
        v->cap = 0; \
    } \
    /* Dummy typedef to absorb trailing semicolon */ \
    typedef Vec_##T Vec_##T##_defined

/*============================================================================
 * Vector Convenience Macros (type-first, like Some(T, ...))
 *============================================================================
 * Each argument is evaluated exactly once.
 */

/**
 * @brief Push an element to the vector
 * @param T The element type
 * @param v The vector (an lvalue, not a pointer)
 * @param elem Element to append
 */
#define VEC_PUSH(T, v, elem) vec_##T##_push(&(v), (elem))

/**
 * @brief Pop the last element from the vector
 * @param T The element type
 * @param v The vector (an lvalue, not a pointer)
 * @return Option containing the last element, or None if empty
 */
#define VEC_POP(T, v) vec_##T##_pop(&(v))

/**
 * @brief Get element at index
 * @param T The element type
 * @param v The vector (an lvalue, not a pointer)
 * @param idx Index to access
 * @return Option containing the element, or None if out of bounds
 */
#define VEC_GET(T, v, idx) vec_##T##_get(&(v), (idx))

/**
 * @brief Insert an element at an index
 * @param T The element type
 * @param v The vector (an lvalue, not a pointer)
 * @param idx Insertion index
 * @param elem Element to insert
 */
#define VEC_INSERT(T, v, idx, elem) vec_##T##_insert(&(v), (idx), (elem))

/**
 * @brief Remove the element at an index
 * @param T The element type
 * @param v The vector (an lvalue, not a pointer)
 * @param idx Index to remove
 * @return Option containing the removed element, or None if out of bounds
 */
#define VEC_REMOVE(T, v, idx) vec_##T##_remove(&(v), (idx))

/**
 * @brief Append n elements from an array
 * @param T The element type
 * @param v The vector (an lvalue, not a pointer)
 * @param src Pointer to source elements
 * @param n Number of elements
 */
#define VEC_EXTEND(T, v, src, n) vec_##T##_extend(&(v), (src), (n))

/**
 * @brief Ensure capacity of at least min_cap
 * @param T The element type
 * @param v The vector (an lvalue, not a pointer)
 * @param min_cap Minimum capacity
 */
#define VEC_RESERVE(T, v, min_cap) vec_##T##_reserve(&(v), (min_cap))

/**
 * @brief Remove all elements (keeps capacity)
 * @param T The element type
 * @param v The vector (an lvalue, not a pointer)
 */
#define VEC_CLEAR(T, v) vec_##T##_clear(&(v))

/**
 * @brief Get the current length
 * @param T The element type
 * @param v The vector (an lvalue, not a pointer)
 * @return Current length
 */
#define VEC_LEN(T, v) vec_##T##_len(&(v))

/**
 * @brief Free all memory associated with the vector
 * @param T The element type
 * @param v The vector (an lvalue, not a pointer)
 */
#define VEC_FREE(T, v) vec_##T##_free(&(v))

#endif /* CYAN_VECTOR_H */

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
 * - vt: pointer to shared vtable
 * 
 * Also generates the following functions:
 * - vec_T_new(): Create empty vector
 * - vec_T_with_capacity(cap): Create vector with initial capacity
 * - vec_T_push(v, elem): Append element
 * - vec_T_pop(v): Remove and return last element as Option
 * - vec_T_get(v, idx): Get element at index as Option
 * - vec_T_len(v): Get current length
 * - vec_T_free(v): Free vector memory
 * 
 * Also generates a vtable struct VecVT_T and convenience macros:
 * - VEC_PUSH(v, elem), VEC_POP(v), VEC_GET(v, idx), VEC_LEN(v), VEC_FREE(v)
 * 
 * Requires: OPTION_DEFINE(T) must be called before VECTOR_DEFINE(T)
 * 
 * Example:
 *   OPTION_DEFINE(int);
 *   VECTOR_DEFINE(int);  // Creates Vec_int, VecVT_int, and vec_int_* functions
 */
#define VECTOR_DEFINE(T) \
    /* Forward declare Vec_T for use in vtable */ \
    typedef struct Vec_##T Vec_##T; \
    \
    /** \
     * @brief Vtable structure for Vec_T containing function pointers \
     */ \
    typedef struct { \
        void (*push)(Vec_##T *v, T elem); \
        Option_##T (*pop)(Vec_##T *v); \
        Option_##T (*get)(Vec_##T *v, size_t idx); \
        size_t (*len)(Vec_##T *v); \
        void (*free)(Vec_##T *v); \
    } VecVT_##T; \
    \
    /** \
     * @brief Vector structure with vtable pointer \
     */ \
    struct Vec_##T { \
        T *data; \
        size_t len; \
        size_t cap; \
        const VecVT_##T *vt; \
    }; \
    \
    /* Forward declare vtable instance */ \
    static const VecVT_##T _vec_##T##_vt; \
    \
    /** \
     * @brief Create an empty vector \
     * @return A new empty Vec_T with no allocated storage \
     */ \
    static inline Vec_##T vec_##T##_new(void) { \
        return (Vec_##T){ .data = NULL, .len = 0, .cap = 0, .vt = &_vec_##T##_vt }; \
    } \
    \
    /** \
     * @brief Create a vector with pre-allocated capacity \
     * @param cap Initial capacity \
     * @return A new Vec_T with allocated storage for cap elements \
     * @note Panics if allocation fails \
     */ \
    static inline Vec_##T vec_##T##_with_capacity(size_t cap) { \
        Vec_##T v = { .data = NULL, .len = 0, .cap = cap, .vt = &_vec_##T##_vt }; \
        if (cap > 0) { \
            v.data = (T *)malloc(cap * sizeof(T)); \
            if (!v.data) CYAN_PANIC("allocation failed"); \
        } \
        return v; \
    } \
    \
    /** \
     * @brief Append an element to the vector \
     * @param v Pointer to the vector \
     * @param elem Element to append \
     * @note Automatically grows capacity if needed \
     * @note Panics if allocation fails during growth \
     */ \
    static inline void vec_##T##_push(Vec_##T *v, T elem) { \
        if (v->len >= v->cap) { \
            size_t new_cap = v->cap == 0 ? CYAN_DEFAULT_CAPACITY : v->cap * CYAN_GROWTH_FACTOR; \
            T *new_data = (T *)realloc(v->data, new_cap * sizeof(T)); \
            if (!new_data) CYAN_PANIC("allocation failed"); \
            v->data = new_data; \
            v->cap = new_cap; \
        } \
        v->data[v->len++] = elem; \
    } \
    \
    /** \
     * @brief Remove and return the last element \
     * @param v Pointer to the vector \
     * @return Option_T containing the last element, or None if empty \
     */ \
    static inline Option_##T vec_##T##_pop(Vec_##T *v) { \
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
    static inline Option_##T vec_##T##_get(Vec_##T *v, size_t idx) { \
        if (idx >= v->len) return None(T); \
        return Some(T, v->data[idx]); \
    } \
    \
    /** \
     * @brief Get the current number of elements \
     * @param v Pointer to the vector \
     * @return Current length \
     */ \
    static inline size_t vec_##T##_len(Vec_##T *v) { \
        return v->len; \
    } \
    \
    /** \
     * @brief Free all memory associated with the vector \
     * @param v Pointer to the vector \
     * @note Resets the vector to empty state \
     */ \
    static inline void vec_##T##_free(Vec_##T *v) { \
        free(v->data); \
        v->data = NULL; \
        v->len = 0; \
        v->cap = 0; \
    } \
    \
    /** \
     * @brief Static const vtable instance shared by all Vec_T instances \
     */ \
    static const VecVT_##T _vec_##T##_vt = { \
        .push = vec_##T##_push, \
        .pop = vec_##T##_pop, \
        .get = vec_##T##_get, \
        .len = vec_##T##_len, \
        .free = vec_##T##_free \
    }; \
    /* Dummy typedef to absorb trailing semicolon */ \
    typedef Vec_##T Vec_##T##_defined

/*============================================================================
 * Vector Convenience Macros
 *============================================================================*/

/**
 * @brief Push an element to the vector via vtable
 * @param v The vector (not a pointer)
 * @param elem Element to append
 */
#define VEC_PUSH(v, elem) ((v).vt->push(&(v), (elem)))

/**
 * @brief Pop the last element from the vector via vtable
 * @param v The vector (not a pointer)
 * @return Option containing the last element, or None if empty
 */
#define VEC_POP(v) ((v).vt->pop(&(v)))

/**
 * @brief Get element at index via vtable
 * @param v The vector (not a pointer)
 * @param idx Index to access
 * @return Option containing the element, or None if out of bounds
 */
#define VEC_GET(v, idx) ((v).vt->get(&(v), (idx)))

/**
 * @brief Get the current length via vtable
 * @param v The vector (not a pointer)
 * @return Current length
 */
#define VEC_LEN(v) ((v).vt->len(&(v)))

/**
 * @brief Free all memory associated with the vector via vtable
 * @param v The vector (not a pointer)
 */
#define VEC_FREE(v) ((v).vt->free(&(v)))

#endif /* CYAN_VECTOR_H */

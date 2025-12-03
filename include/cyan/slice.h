/**
 * @file slice.h
 * @brief Slice type for safe array views with bounds information
 * 
 * This header provides a type-safe slice implementation using macros.
 * Slices are non-owning views into contiguous sequences of elements,
 * providing bounds-checked access without copying data.
 * 
 * Usage:
 *   OPTION_DEFINE(int);   // Required for Option_int
 *   VECTOR_DEFINE(int);   // Required for Vec_int (if using slice_from_vec)
 *   SLICE_DEFINE(int);    // Define Slice_int type
 *   
 *   int arr[] = {1, 2, 3, 4, 5};
 *   Slice_int s = slice_int_from_array(arr, 5);
 *   Option_int elem = slice_int_get(s, 2);
 */

#ifndef CYAN_SLICE_H
#define CYAN_SLICE_H

#include "common.h"
#include "option.h"

/*============================================================================
 * Slice Type Definition
 *============================================================================*/

/**
 * @brief Generate a Slice type for a given element type
 * @param T The element type
 * 
 * Creates a struct Slice_T with:
 * - data: const pointer to element array (non-owning)
 * - len: number of elements in the slice
 * - vt: pointer to shared vtable
 * 
 * Also generates the following functions:
 * - slice_T_from_array(arr, len): Create slice from C array
 * - slice_T_from_vec(v): Create slice from Vec_T
 * - slice_T_get(s, idx): Get element at index as Option
 * - slice_T_subslice(s, start, end): Create a subslice view
 * - slice_T_len(s): Get slice length
 * 
 * Also generates a vtable struct SliceVT_T and convenience macros:
 * - SLICE_GET(s, idx), SLICE_SUBSLICE(s, start, end), SLICE_LEN(s)
 * 
 * Requires: OPTION_DEFINE(T) must be called before SLICE_DEFINE(T)
 * 
 * Example:
 *   OPTION_DEFINE(int);
 *   SLICE_DEFINE(int);  // Creates Slice_int, SliceVT_int, and slice_int_* functions
 */
#define SLICE_DEFINE(T) \
    /* Forward declare Slice_T for use in vtable */ \
    typedef struct Slice_##T Slice_##T; \
    \
    /** \
     * @brief Vtable structure for Slice_T containing function pointers \
     */ \
    typedef struct { \
        Option_##T (*get)(Slice_##T s, size_t idx); \
        Slice_##T (*subslice)(Slice_##T s, size_t start, size_t end); \
        size_t (*len)(Slice_##T s); \
    } SliceVT_##T; \
    \
    /** \
     * @brief Slice structure with vtable pointer \
     */ \
    struct Slice_##T { \
        const T *data; \
        size_t len; \
        const SliceVT_##T *vt; \
    }; \
    \
    /* Forward declare vtable instance */ \
    static const SliceVT_##T _slice_##T##_vt; \
    \
    /** \
     * @brief Create a slice from a C array \
     * @param arr Pointer to the array \
     * @param len Number of elements \
     * @return A new Slice_T viewing the array \
     */ \
    static inline Slice_##T slice_##T##_from_array(const T *arr, size_t len) { \
        return (Slice_##T){ .data = arr, .len = len, .vt = &_slice_##T##_vt }; \
    } \
    \
    /** \
     * @brief Create a slice from a vector \
     * @param v Pointer to the vector \
     * @return A new Slice_T viewing the vector's data \
     * @note The slice becomes invalid if the vector is modified or freed \
     */ \
    static inline Slice_##T slice_##T##_from_vec(const Vec_##T *v) { \
        return (Slice_##T){ .data = v->data, .len = v->len, .vt = &_slice_##T##_vt }; \
    } \
    \
    /** \
     * @brief Get element at index with bounds checking \
     * @param s The slice \
     * @param idx Index to access \
     * @return Option_T containing the element, or None if out of bounds \
     */ \
    static inline Option_##T slice_##T##_get(Slice_##T s, size_t idx) { \
        if (idx >= s.len) return None(T); \
        return Some(T, s.data[idx]); \
    } \
    \
    /** \
     * @brief Create a subslice view into the original slice \
     * @param s The source slice \
     * @param start Start index (inclusive) \
     * @param end End index (exclusive) \
     * @return A new Slice_T viewing the specified range \
     * @note Indices are clamped to valid bounds \
     * @note The returned subslice shares the same vtable as the source \
     */ \
    static inline Slice_##T slice_##T##_subslice(Slice_##T s, size_t start, size_t end) { \
        if (start > s.len) start = s.len; \
        if (end > s.len) end = s.len; \
        if (start > end) start = end; \
        return (Slice_##T){ .data = s.data + start, .len = end - start, .vt = &_slice_##T##_vt }; \
    } \
    \
    /** \
     * @brief Get the length of the slice \
     * @param s The slice \
     * @return Number of elements in the slice \
     */ \
    static inline size_t slice_##T##_len(Slice_##T s) { \
        return s.len; \
    } \
    \
    /** \
     * @brief Static const vtable instance shared by all Slice_T instances \
     */ \
    static const SliceVT_##T _slice_##T##_vt = { \
        .get = slice_##T##_get, \
        .subslice = slice_##T##_subslice, \
        .len = slice_##T##_len \
    }; \
    /* Dummy typedef to absorb trailing semicolon */ \
    typedef Slice_##T Slice_##T##_defined

/*============================================================================
 * Slice Convenience Macros
 *============================================================================*/

/**
 * @brief Get element at index via vtable
 * @param s The slice (not a pointer)
 * @param idx Index to access
 * @return Option containing the element, or None if out of bounds
 */
#define SLICE_GET(s, idx) ((s).vt->get((s), (idx)))

/**
 * @brief Create a subslice view via vtable
 * @param s The slice (not a pointer)
 * @param start Start index (inclusive)
 * @param end End index (exclusive)
 * @return A new Slice viewing the specified range
 */
#define SLICE_SUBSLICE(s, start, end) ((s).vt->subslice((s), (start), (end)))

/**
 * @brief Get the length of the slice via vtable
 * @param s The slice (not a pointer)
 * @return Number of elements in the slice
 */
#define SLICE_LEN(s) ((s).vt->len((s)))

#endif /* CYAN_SLICE_H */

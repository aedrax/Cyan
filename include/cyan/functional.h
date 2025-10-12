/**
 * @file functional.h
 * @brief Higher-order function macros for functional programming in C
 * 
 * This header provides functional programming primitives like map, filter,
 * reduce, and foreach for working with arrays and vectors in a declarative style.
 * 
 * Usage:
 *   int arr[] = {1, 2, 3, 4, 5};
 *   int out[5];
 *   map(arr, 5, out, double_fn);
 *   
 *   size_t out_len;
 *   filter(arr, 5, out, &out_len, is_even);
 *   
 *   int sum = reduce(arr, 5, 0, add);
 */

#ifndef CYAN_FUNCTIONAL_H
#define CYAN_FUNCTIONAL_H

#include "common.h"
#include "option.h"
#include "vector.h"

/*============================================================================
 * Map - Transform each element
 *============================================================================*/

/**
 * @brief Apply a transformation function to each element of an array
 * @param arr Source array
 * @param len Length of source array
 * @param out Output array (must be pre-allocated with at least len elements)
 * @param fn Transformation function taking element and returning transformed value
 * 
 * Property 9: map preserves length and applies function
 * For any array of length n and transformation function f, map produces
 * an array of length n where element i equals f(original[i]).
 * 
 * Example:
 *   int double_it(int x) { return x * 2; }
 *   int arr[] = {1, 2, 3};
 *   int out[3];
 *   map(arr, 3, out, double_it);
 *   // out = {2, 4, 6}
 */
#define map(arr, len, out, fn) do { \
    for (size_t _cyan_map_i = 0; _cyan_map_i < (len); _cyan_map_i++) { \
        (out)[_cyan_map_i] = fn((arr)[_cyan_map_i]); \
    } \
} while(0)

/*============================================================================
 * Filter - Select elements matching predicate
 *============================================================================*/

/**
 * @brief Filter array elements that satisfy a predicate
 * @param arr Source array
 * @param len Length of source array
 * @param out Output array (must be pre-allocated with at least len elements)
 * @param out_len Pointer to size_t to store output length
 * @param pred Predicate function returning true for elements to keep
 * 
 * Property 10: filter preserves predicate satisfaction
 * All elements in the filtered result satisfy the predicate, and the result
 * contains all elements from the original that satisfy the predicate.
 * 
 * Example:
 *   bool is_even(int x) { return x % 2 == 0; }
 *   int arr[] = {1, 2, 3, 4, 5};
 *   int out[5];
 *   size_t out_len;
 *   filter(arr, 5, out, &out_len, is_even);
 *   // out = {2, 4}, out_len = 2
 */
#define filter(arr, len, out, out_len, pred) do { \
    size_t _cyan_filter_j = 0; \
    for (size_t _cyan_filter_i = 0; _cyan_filter_i < (len); _cyan_filter_i++) { \
        if (pred((arr)[_cyan_filter_i])) { \
            (out)[_cyan_filter_j++] = (arr)[_cyan_filter_i]; \
        } \
    } \
    *(out_len) = _cyan_filter_j; \
} while(0)

/*============================================================================
 * Reduce - Combine elements into single value
 *============================================================================*/

/**
 * @brief Reduce array to a single value using an accumulator function
 * @param result Variable to store the result (must be declared beforehand)
 * @param arr Source array
 * @param len Length of source array
 * @param init Initial accumulator value
 * @param acc_fn Accumulator function taking (accumulator, element) and returning new accumulator
 * 
 * Property 11: reduce equivalence to sequential fold
 * For any array, initial value, and accumulator function, reduce produces
 * the same result as sequentially applying the accumulator from left to right.
 * 
 * Example:
 *   int add(int a, int b) { return a + b; }
 *   int arr[] = {1, 2, 3, 4, 5};
 *   int sum;
 *   reduce(sum, arr, 5, 0, add);
 *   // sum = 15
 */
#define reduce(result, arr, len, init, acc_fn) do { \
    (result) = (init); \
    for (size_t _cyan_reduce_i = 0; _cyan_reduce_i < (len); _cyan_reduce_i++) { \
        (result) = acc_fn((result), (arr)[_cyan_reduce_i]); \
    } \
} while(0)

/*============================================================================
 * Foreach - Execute side effect on each element
 *============================================================================*/

/**
 * @brief Execute a side-effect function on each element in order
 * @param arr Source array
 * @param len Length of source array
 * @param fn Function to call on each element
 * 
 * Property 12: foreach visits each element exactly once in order
 * For any array of length n, foreach invokes the callback exactly n times,
 * once for each element in index order.
 * 
 * Example:
 *   void print_int(int x) { printf("%d\n", x); }
 *   int arr[] = {1, 2, 3};
 *   foreach(arr, 3, print_int);
 *   // prints: 1, 2, 3
 */
#define foreach(arr, len, fn) do { \
    for (size_t _cyan_foreach_i = 0; _cyan_foreach_i < (len); _cyan_foreach_i++) { \
        fn((arr)[_cyan_foreach_i]); \
    } \
} while(0)

/*============================================================================
 * Vector-specific functional operations
 *============================================================================*/

/**
 * @brief Generate vector map function for a given input and output type
 * @param T_in Input element type
 * @param T_out Output element type
 * 
 * Creates a function vec_map_T_in_to_T_out that transforms a vector
 * of T_in to a new vector of T_out using a transformation function.
 * 
 * Requires: OPTION_DEFINE and VECTOR_DEFINE for both T_in and T_out
 * 
 * Example:
 *   OPTION_DEFINE(int);
 *   OPTION_DEFINE(double);
 *   VECTOR_DEFINE(int);
 *   VECTOR_DEFINE(double);
 *   VEC_MAP_DEFINE(int, double);
 *   
 *   double int_to_double(int x) { return (double)x; }
 *   Vec_int v = vec_int_new();
 *   vec_int_push(&v, 1);
 *   Vec_double result = vec_map_int_to_double(&v, int_to_double);
 */
#define VEC_MAP_DEFINE(T_in, T_out) \
    typedef T_out (*_vec_map_fn_##T_in##_to_##T_out)(T_in); \
    \
    static inline Vec_##T_out vec_map_##T_in##_to_##T_out( \
        Vec_##T_in *v, \
        _vec_map_fn_##T_in##_to_##T_out fn \
    ) { \
        Vec_##T_out result = vec_##T_out##_with_capacity(v->len); \
        for (size_t _i = 0; _i < v->len; _i++) { \
            vec_##T_out##_push(&result, fn(v->data[_i])); \
        } \
        return result; \
    } \
    /* Dummy typedef to absorb trailing semicolon */ \
    typedef Vec_##T_out _vec_map_##T_in##_to_##T_out##_defined

/**
 * @brief Generate vector filter function for a given type
 * @param T Element type
 * 
 * Creates a function vec_filter_T that filters a vector using a predicate.
 * 
 * Requires: OPTION_DEFINE and VECTOR_DEFINE for T
 */
#define VEC_FILTER_DEFINE(T) \
    typedef bool (*_vec_filter_pred_##T)(T); \
    \
    static inline Vec_##T vec_filter_##T( \
        Vec_##T *v, \
        _vec_filter_pred_##T pred \
    ) { \
        Vec_##T result = vec_##T##_new(); \
        for (size_t _i = 0; _i < v->len; _i++) { \
            if (pred(v->data[_i])) { \
                vec_##T##_push(&result, v->data[_i]); \
            } \
        } \
        return result; \
    } \
    /* Dummy typedef to absorb trailing semicolon */ \
    typedef Vec_##T _vec_filter_##T##_defined

/**
 * @brief Generate vector reduce function for a given type
 * @param T Element type
 * @param R Result type
 * 
 * Creates a function vec_reduce_T_to_R that reduces a vector to a single value.
 * 
 * Requires: OPTION_DEFINE and VECTOR_DEFINE for T
 */
#define VEC_REDUCE_DEFINE(T, R) \
    typedef R (*_vec_reduce_fn_##T##_to_##R)(R, T); \
    \
    static inline R vec_reduce_##T##_to_##R( \
        Vec_##T *v, \
        R init, \
        _vec_reduce_fn_##T##_to_##R fn \
    ) { \
        R acc = init; \
        for (size_t _i = 0; _i < v->len; _i++) { \
            acc = fn(acc, v->data[_i]); \
        } \
        return acc; \
    } \
    /* Dummy typedef to absorb trailing semicolon */ \
    typedef Vec_##T _vec_reduce_##T##_to_##R##_defined

/**
 * @brief Generate vector foreach function for a given type
 * @param T Element type
 * 
 * Creates a function vec_foreach_T that executes a function on each element.
 * 
 * Requires: OPTION_DEFINE and VECTOR_DEFINE for T
 */
#define VEC_FOREACH_DEFINE(T) \
    typedef void (*_vec_foreach_fn_##T)(T); \
    \
    static inline void vec_foreach_##T( \
        Vec_##T *v, \
        _vec_foreach_fn_##T fn \
    ) { \
        for (size_t _i = 0; _i < v->len; _i++) { \
            fn(v->data[_i]); \
        } \
    } \
    /* Dummy typedef to absorb trailing semicolon */ \
    typedef Vec_##T _vec_foreach_##T##_defined

#endif /* CYAN_FUNCTIONAL_H */

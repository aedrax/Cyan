/**
 * @file option.h
 * @brief Option type for explicit nullable value handling
 *
 * This header provides Option types that explicitly represent the presence
 * or absence of a value, avoiding null pointer issues and making absence
 * explicit in code.
 *
 * Usage:
 *   OPTION_DEFINE(int);  // Define Option_int type
 *   Option_int maybe = Some(int, 42);
 *   if (is_some(maybe)) {
 *       int val = unwrap(maybe);
 *   }
 *
 * Define CYAN_NO_SHORT_NAMES before including to suppress the short
 * lowercase macros (is_some, unwrap, map_option, ...); the uppercase
 * OPT_* macros and the generated option_T_* functions are always available.
 */

#ifndef CYAN_OPTION_H
#define CYAN_OPTION_H

#include "common.h"

/*============================================================================
 * Option Type Definition
 *============================================================================*/

/**
 * @brief Generate an Option type for a given base type
 * @param T The base type to wrap
 *
 * Creates a struct Option_T with:
 * - has_value: bool indicating presence of value
 * - value: the wrapped value of type T
 *
 * Also generates the functions:
 * - option_T_is_some(opt), option_T_is_none(opt)
 * - option_T_unwrap(opt) (panics on None)
 * - option_T_unwrap_or(opt, default_val)
 *
 * Example:
 *   OPTION_DEFINE(int);    // Creates Option_int
 *   OPTION_DEFINE(double); // Creates Option_double
 */
#define OPTION_DEFINE(T) \
    typedef struct { \
        bool has_value; \
        T value; \
    } Option_##T; \
    \
    CYAN_UNUSED static inline bool option_##T##_is_some(const Option_##T *opt) { \
        return opt->has_value; \
    } \
    \
    CYAN_UNUSED static inline bool option_##T##_is_none(const Option_##T *opt) { \
        return !opt->has_value; \
    } \
    \
    CYAN_UNUSED static inline T option_##T##_unwrap(const Option_##T *opt) { \
        if (!opt->has_value) CYAN_PANIC("unwrap called on None"); \
        return opt->value; \
    } \
    \
    CYAN_UNUSED static inline T option_##T##_unwrap_or(const Option_##T *opt, T default_val) { \
        return opt->has_value ? opt->value : default_val; \
    } \
    /* Dummy typedef to absorb trailing semicolon */ \
    typedef Option_##T Option_##T##_defined

/*============================================================================
 * Constructors
 *============================================================================*/

/**
 * @brief Create an Option containing a value
 * @param T The type of the Option
 * @param val The value to wrap
 * @return Option_T with has_value = true
 *
 * Example:
 *   Option_int x = Some(int, 42);
 */
#define Some(T, val) ((Option_##T){ .has_value = true, .value = (val) })

/**
 * @brief Create an empty Option (no value)
 * @param T The type of the Option
 * @return Option_T with has_value = false
 *
 * Example:
 *   Option_int x = None(int);
 */
#define None(T) ((Option_##T){ .has_value = false })

/*============================================================================
 * Common Instances
 *============================================================================*/

/* Option_size_t is used across the library for indices and lengths
 * (string_find, vec_T_find, ...), so it is defined here once.
 * Do not OPTION_DEFINE(size_t) in user code. */
OPTION_DEFINE(size_t);

/*============================================================================
 * Core Macros (always available)
 *============================================================================
 * These are type-generic via member access and evaluate each argument
 * exactly once on GNU-compatible compilers.
 */

/**
 * @brief Check if an Option contains a value
 * @param opt The Option to check
 * @return true if the Option contains a value, false otherwise
 */
#define OPT_IS_SOME(opt) ((opt).has_value)

/**
 * @brief Check if an Option is empty
 * @param opt The Option to check
 * @return true if the Option is empty, false otherwise
 */
#define OPT_IS_NONE(opt) (!(opt).has_value)

/**
 * @brief Extract the value from an Option, panicking if empty
 * @param opt The Option to unwrap
 * @return The contained value
 * @note Panics if the Option is None
 */
#if defined(__GNUC__) || defined(__clang__)
#define OPT_UNWRAP(opt) \
    ({ __typeof__(opt) _cyan_opt = (opt); \
       _cyan_opt.has_value ? _cyan_opt.value \
                           : CYAN_PANIC_EXPR("unwrap called on None", _cyan_opt.value); })
#else
/* Fallback: evaluates opt more than once */
#define OPT_UNWRAP(opt) \
    ((opt).has_value ? (opt).value : CYAN_PANIC_EXPR("unwrap called on None", (opt).value))
#endif

/**
 * @brief Extract the value from an Option, panicking with a custom message
 * @param opt The Option to unwrap
 * @param msg The panic message used if the Option is None
 * @return The contained value
 */
#if defined(__GNUC__) || defined(__clang__)
#define OPT_EXPECT(opt, msg) \
    ({ __typeof__(opt) _cyan_opt = (opt); \
       _cyan_opt.has_value ? _cyan_opt.value \
                           : CYAN_PANIC_EXPR((msg), _cyan_opt.value); })
#else
/* Fallback: evaluates opt more than once */
#define OPT_EXPECT(opt, msg) \
    ((opt).has_value ? (opt).value : CYAN_PANIC_EXPR((msg), (opt).value))
#endif

/**
 * @brief Extract the value from an Option, or return a default
 * @param opt The Option to unwrap
 * @param default_val The default value if Option is empty
 * @return The contained value if present, otherwise default_val
 */
#if defined(__GNUC__) || defined(__clang__)
#define OPT_UNWRAP_OR(opt, default_val) \
    ({ __typeof__(opt) _cyan_opt = (opt); \
       _cyan_opt.has_value ? _cyan_opt.value : (default_val); })
#else
/* Fallback: evaluates opt more than once */
#define OPT_UNWRAP_OR(opt, default_val) \
    ((opt).has_value ? (opt).value : (default_val))
#endif

/**
 * @brief Transform the value inside an Option
 * @param opt The Option to transform
 * @param T_out The output type
 * @param fn The transformation function (T -> T_out)
 * @return Option_T_out containing transformed value, or None if input was None
 */
#if defined(__GNUC__) || defined(__clang__)
#define OPT_MAP(opt, T_out, fn) \
    ({ __typeof__(opt) _cyan_opt = (opt); \
       _cyan_opt.has_value ? Some(T_out, fn(_cyan_opt.value)) : None(T_out); })
#else
/* Fallback: evaluates opt more than once */
#define OPT_MAP(opt, T_out, fn) \
    ((opt).has_value ? Some(T_out, fn((opt).value)) : None(T_out))
#endif

/**
 * @brief Chain a fallible transformation
 * @param opt The Option to chain from
 * @param T_out The output type
 * @param fn Function taking the contained value and returning Option_T_out
 * @return fn's result if opt is Some, otherwise None(T_out)
 *
 * Example:
 *   Option_int parsed = and_then(input, int, checked_parse);
 */
#if defined(__GNUC__) || defined(__clang__)
#define OPT_AND_THEN(opt, T_out, fn) \
    ({ __typeof__(opt) _cyan_opt = (opt); \
       _cyan_opt.has_value ? fn(_cyan_opt.value) : None(T_out); })
#else
/* Fallback: evaluates opt more than once */
#define OPT_AND_THEN(opt, T_out, fn) \
    ((opt).has_value ? fn((opt).value) : None(T_out))
#endif

/**
 * @brief Provide a fallback Option when empty
 * @param opt The Option to check
 * @param fn Zero-argument function returning an Option of the same type
 * @return opt if it contains a value, otherwise fn()
 */
#if defined(__GNUC__) || defined(__clang__)
#define OPT_OR_ELSE(opt, fn) \
    ({ __typeof__(opt) _cyan_opt = (opt); \
       _cyan_opt.has_value ? _cyan_opt : fn(); })
#else
/* Fallback: evaluates opt more than once */
#define OPT_OR_ELSE(opt, fn) \
    ((opt).has_value ? (opt) : fn())
#endif

/**
 * @brief Convert an Option into a Result
 * @param opt The Option to convert
 * @param T The success type (the Option's contained type)
 * @param E The error type
 * @param err_val Error value used when the Option is None
 * @return Ok(T, E, value) if Some, Err(T, E, err_val) if None
 * @note Requires result.h and RESULT_DEFINE(T, E)
 */
#if defined(__GNUC__) || defined(__clang__)
#define OPT_OK_OR(opt, T, E, err_val) \
    ({ __typeof__(opt) _cyan_opt = (opt); \
       _cyan_opt.has_value ? Ok(T, E, _cyan_opt.value) : Err(T, E, (err_val)); })
#else
/* Fallback: evaluates opt more than once */
#define OPT_OK_OR(opt, T, E, err_val) \
    ((opt).has_value ? Ok(T, E, (opt).value) : Err(T, E, (err_val)))
#endif

/**
 * @brief Early-return propagation for Options (like Rust's `?`)
 * @param opt The Option to unwrap
 * @return The contained value; if the Option is None, the enclosing
 *         function returns the None immediately
 * @note The enclosing function must return the same Option_T type
 * @note GNU C only (statement expressions with return)
 *
 * Example:
 *   Option_int step(Option_int in) {
 *       int v = try_some(in);   // returns None(int) to the caller on None
 *       return Some(int, v * 2);
 *   }
 */
#if defined(__GNUC__) || defined(__clang__)
#define OPT_TRY(opt) \
    ({ __typeof__(opt) _cyan_opt = (opt); \
       if (!_cyan_opt.has_value) return _cyan_opt; \
       _cyan_opt.value; })
#endif

/*============================================================================
 * Short Names (suppress with CYAN_NO_SHORT_NAMES)
 *============================================================================*/

#ifndef CYAN_NO_SHORT_NAMES

#define is_some(opt) OPT_IS_SOME(opt)
#define is_none(opt) OPT_IS_NONE(opt)
#define unwrap(opt) OPT_UNWRAP(opt)
#define expect(opt, msg) OPT_EXPECT(opt, msg)
#define unwrap_or(opt, default_val) OPT_UNWRAP_OR(opt, default_val)
#define map_option(opt, T_out, fn) OPT_MAP(opt, T_out, fn)
#define and_then(opt, T_out, fn) OPT_AND_THEN(opt, T_out, fn)
#define or_else(opt, fn) OPT_OR_ELSE(opt, fn)
#define ok_or(opt, T, E, err_val) OPT_OK_OR(opt, T, E, err_val)
#if defined(__GNUC__) || defined(__clang__)
#define try_some(opt) OPT_TRY(opt)
#endif

#endif /* CYAN_NO_SHORT_NAMES */

#endif /* CYAN_OPTION_H */

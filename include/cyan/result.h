/**
 * @file result.h
 * @brief Result type for explicit error handling
 *
 * This header provides Result types that represent either a successful value
 * or an error, enabling explicit error handling without relying on error codes
 * or errno.
 *
 * Usage:
 *   RESULT_DEFINE(int, const char*);  // Define Result_int_const_charp type
 *   Result_int_const_charp res = Ok(int, const_charp, 42);
 *   if (is_ok(res)) {
 *       int val = unwrap_ok(res);
 *   } else {
 *       const char *err = unwrap_err(res);
 *   }
 *
 * Define CYAN_NO_SHORT_NAMES before including to suppress the short
 * lowercase macros (is_ok, unwrap_ok, map_result, ...); the uppercase
 * RES_* macros and the generated result_T_E_* functions are always
 * available.
 */

#ifndef CYAN_RESULT_H
#define CYAN_RESULT_H

#include "common.h"

/*============================================================================
 * Result Type Definition
 *============================================================================*/

/**
 * @brief Generate a Result type for given value and error types
 * @param T The success value type
 * @param E The error value type
 *
 * Creates a struct Result_T_E with:
 * - is_ok_flag: bool indicating success or failure
 * - ok_value: the success value of type T (in union)
 * - err_value: the error value of type E (in union)
 *
 * Also generates the functions:
 * - result_T_E_is_ok(res), result_T_E_is_err(res)
 * - result_T_E_unwrap_ok(res) (panics on Err)
 * - result_T_E_unwrap_err(res) (panics on Ok)
 * - result_T_E_unwrap_ok_or(res, default_val)
 *
 * Example:
 *   RESULT_DEFINE(int, const char*);  // Creates Result_int_const_charp
 *   RESULT_DEFINE(double, int);       // Creates Result_double_int
 */
#define RESULT_DEFINE(T, E) \
    typedef struct { \
        bool is_ok_flag; \
        union { \
            T ok_value; \
            E err_value; \
        }; \
    } Result_##T##_##E; \
    \
    CYAN_UNUSED static inline bool result_##T##_##E##_is_ok(const Result_##T##_##E *res) { \
        return res->is_ok_flag; \
    } \
    \
    CYAN_UNUSED static inline bool result_##T##_##E##_is_err(const Result_##T##_##E *res) { \
        return !res->is_ok_flag; \
    } \
    \
    CYAN_UNUSED static inline T result_##T##_##E##_unwrap_ok(const Result_##T##_##E *res) { \
        if (!res->is_ok_flag) CYAN_PANIC("unwrap_ok called on Err"); \
        return res->ok_value; \
    } \
    \
    CYAN_UNUSED static inline E result_##T##_##E##_unwrap_err(const Result_##T##_##E *res) { \
        if (res->is_ok_flag) CYAN_PANIC("unwrap_err called on Ok"); \
        return res->err_value; \
    } \
    \
    CYAN_UNUSED static inline T result_##T##_##E##_unwrap_ok_or(const Result_##T##_##E *res, T default_val) { \
        return res->is_ok_flag ? res->ok_value : default_val; \
    } \
    /* Dummy typedef to absorb trailing semicolon */ \
    typedef Result_##T##_##E Result_##T##_##E##_defined

/*============================================================================
 * Constructors
 *============================================================================*/

/**
 * @brief Create a Result containing a success value
 * @param T The success type
 * @param E The error type
 * @param val The success value to wrap
 * @return Result_T_E with is_ok_flag = true
 *
 * Example:
 *   Result_int_const_charp x = Ok(int, const_charp, 42);
 */
#define Ok(T, E, val) ((Result_##T##_##E){ .is_ok_flag = true, .ok_value = (val) })

/**
 * @brief Create a Result containing an error value
 * @param T The success type
 * @param E The error type
 * @param err The error value to wrap
 * @return Result_T_E with is_ok_flag = false
 *
 * Example:
 *   Result_int_const_charp x = Err(int, const_charp, "parse error");
 */
#define Err(T, E, err) ((Result_##T##_##E){ .is_ok_flag = false, .err_value = (err) })

/*============================================================================
 * Core Macros (always available)
 *============================================================================
 * These are type-generic via member access and evaluate each argument
 * exactly once on GNU-compatible compilers.
 */

/**
 * @brief Check if a Result contains a success value
 * @param res The Result to check
 * @return true if the Result is Ok, false otherwise
 */
#define RES_IS_OK(res) ((res).is_ok_flag)

/**
 * @brief Check if a Result contains an error value
 * @param res The Result to check
 * @return true if the Result is Err, false otherwise
 */
#define RES_IS_ERR(res) (!(res).is_ok_flag)

/**
 * @brief Extract the success value from a Result, panicking if error
 * @param res The Result to unwrap
 * @return The contained success value
 * @note Panics if the Result is Err
 */
#if defined(__GNUC__) || defined(__clang__)
#define RES_UNWRAP_OK(res) \
    ({ __typeof__(res) _cyan_res = (res); \
       _cyan_res.is_ok_flag ? _cyan_res.ok_value \
                            : CYAN_PANIC_EXPR("unwrap_ok called on Err", _cyan_res.ok_value); })
#else
/* Fallback: evaluates res more than once */
#define RES_UNWRAP_OK(res) \
    ((res).is_ok_flag ? (res).ok_value \
                      : CYAN_PANIC_EXPR("unwrap_ok called on Err", (res).ok_value))
#endif

/**
 * @brief Extract the success value, panicking with a custom message
 * @param res The Result to unwrap
 * @param msg The panic message used if the Result is Err
 * @return The contained success value
 */
#if defined(__GNUC__) || defined(__clang__)
#define RES_EXPECT_OK(res, msg) \
    ({ __typeof__(res) _cyan_res = (res); \
       _cyan_res.is_ok_flag ? _cyan_res.ok_value \
                            : CYAN_PANIC_EXPR((msg), _cyan_res.ok_value); })
#else
/* Fallback: evaluates res more than once */
#define RES_EXPECT_OK(res, msg) \
    ((res).is_ok_flag ? (res).ok_value : CYAN_PANIC_EXPR((msg), (res).ok_value))
#endif

/**
 * @brief Extract the error value from a Result, panicking if success
 * @param res The Result to unwrap
 * @return The contained error value
 * @note Panics if the Result is Ok
 */
#if defined(__GNUC__) || defined(__clang__)
#define RES_UNWRAP_ERR(res) \
    ({ __typeof__(res) _cyan_res = (res); \
       !_cyan_res.is_ok_flag ? _cyan_res.err_value \
                             : CYAN_PANIC_EXPR("unwrap_err called on Ok", _cyan_res.err_value); })
#else
/* Fallback: evaluates res more than once */
#define RES_UNWRAP_ERR(res) \
    (!(res).is_ok_flag ? (res).err_value \
                       : CYAN_PANIC_EXPR("unwrap_err called on Ok", (res).err_value))
#endif

/**
 * @brief Extract the success value from a Result, or return a default
 * @param res The Result to unwrap
 * @param default_val The default value if Result is Err
 * @return The contained success value if Ok, otherwise default_val
 */
#if defined(__GNUC__) || defined(__clang__)
#define RES_UNWRAP_OK_OR(res, default_val) \
    ({ __typeof__(res) _cyan_res = (res); \
       _cyan_res.is_ok_flag ? _cyan_res.ok_value : (default_val); })
#else
/* Fallback: evaluates res more than once */
#define RES_UNWRAP_OK_OR(res, default_val) \
    ((res).is_ok_flag ? (res).ok_value : (default_val))
#endif

/**
 * @brief Transform the success value inside a Result
 * @param res The Result to transform
 * @param T_out The output success type
 * @param E The error type (unchanged)
 * @param fn The transformation function (T -> T_out)
 * @return Result_T_out_E containing transformed value, or original Err
 */
#if defined(__GNUC__) || defined(__clang__)
#define RES_MAP(res, T_out, E, fn) \
    ({ __typeof__(res) _cyan_res = (res); \
       _cyan_res.is_ok_flag ? Ok(T_out, E, fn(_cyan_res.ok_value)) \
                            : Err(T_out, E, _cyan_res.err_value); })
#else
/* Fallback: evaluates res more than once */
#define RES_MAP(res, T_out, E, fn) \
    ((res).is_ok_flag ? Ok(T_out, E, fn((res).ok_value)) : Err(T_out, E, (res).err_value))
#endif

/**
 * @brief Transform the error value inside a Result
 * @param res The Result to transform
 * @param T The success type (unchanged)
 * @param E_out The output error type
 * @param fn The transformation function (E -> E_out)
 * @return Result_T_E_out containing original Ok, or transformed Err
 */
#if defined(__GNUC__) || defined(__clang__)
#define RES_MAP_ERR(res, T, E_out, fn) \
    ({ __typeof__(res) _cyan_res = (res); \
       _cyan_res.is_ok_flag ? Ok(T, E_out, _cyan_res.ok_value) \
                            : Err(T, E_out, fn(_cyan_res.err_value)); })
#else
/* Fallback: evaluates res more than once */
#define RES_MAP_ERR(res, T, E_out, fn) \
    ((res).is_ok_flag ? Ok(T, E_out, (res).ok_value) : Err(T, E_out, fn((res).err_value)))
#endif

/**
 * @brief Chain a fallible transformation on the success value
 * @param res The Result to chain from
 * @param T_out The output success type
 * @param E The error type (unchanged)
 * @param fn Function taking the Ok value and returning Result_T_out_E
 * @return fn's result if res is Ok, otherwise the original error
 */
#if defined(__GNUC__) || defined(__clang__)
#define RES_AND_THEN(res, T_out, E, fn) \
    ({ __typeof__(res) _cyan_res = (res); \
       _cyan_res.is_ok_flag ? fn(_cyan_res.ok_value) \
                            : Err(T_out, E, _cyan_res.err_value); })
#else
/* Fallback: evaluates res more than once */
#define RES_AND_THEN(res, T_out, E, fn) \
    ((res).is_ok_flag ? fn((res).ok_value) : Err(T_out, E, (res).err_value))
#endif

/**
 * @brief Early-return propagation for Results (like Rust's `?`)
 * @param res The Result to unwrap
 * @return The Ok value; if the Result is Err, the enclosing function
 *         returns the whole Result immediately
 * @note The enclosing function must return the same Result_T_E type
 * @note GNU C only (statement expressions with return)
 *
 * Example:
 *   Result_int_ParseError double_parse(const char *s) {
 *       int v = try_ok(parse_int(s, NULL));  // propagates the Err
 *       return Ok(int, ParseError, v * 2);
 *   }
 */
#if defined(__GNUC__) || defined(__clang__)
#define RES_TRY(res) \
    ({ __typeof__(res) _cyan_res = (res); \
       if (!_cyan_res.is_ok_flag) return _cyan_res; \
       _cyan_res.ok_value; })
#endif

/*============================================================================
 * Short Names (suppress with CYAN_NO_SHORT_NAMES)
 *============================================================================*/

#ifndef CYAN_NO_SHORT_NAMES

#define is_ok(res) RES_IS_OK(res)
#define is_err(res) RES_IS_ERR(res)
#define unwrap_ok(res) RES_UNWRAP_OK(res)
#define expect_ok(res, msg) RES_EXPECT_OK(res, msg)
#define unwrap_err(res) RES_UNWRAP_ERR(res)
#define unwrap_ok_or(res, default_val) RES_UNWRAP_OK_OR(res, default_val)
#define map_result(res, T_out, E, fn) RES_MAP(res, T_out, E, fn)
#define map_err(res, T, E_out, fn) RES_MAP_ERR(res, T, E_out, fn)
#define and_then_result(res, T_out, E, fn) RES_AND_THEN(res, T_out, E, fn)
#if defined(__GNUC__) || defined(__clang__)
#define try_ok(res) RES_TRY(res)
#endif

#endif /* CYAN_NO_SHORT_NAMES */

#endif /* CYAN_RESULT_H */

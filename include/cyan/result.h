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
 */

#ifndef CYAN_RESULT_H
#define CYAN_RESULT_H

#include "common.h"

/*============================================================================
 * Result Type Definition
 *============================================================================*/

/* Forward declare vtable struct */
#define RESULT_VT_FORWARD(T, E) \
    typedef struct ResultVT_##T##_##E ResultVT_##T##_##E

/**
 * @brief Generate a Result type for given value and error types
 * @param T The success value type
 * @param E The error value type
 * 
 * Creates a struct Result_T_E with:
 * - is_ok: bool indicating success or failure
 * - ok_value: the success value of type T (in union)
 * - err_value: the error value of type E (in union)
 * - vt: pointer to shared vtable
 * 
 * Example:
 *   RESULT_DEFINE(int, const char*);  // Creates Result_int_const_charp
 *   RESULT_DEFINE(double, int);       // Creates Result_double_int
 */
#define RESULT_DEFINE(T, E) \
    RESULT_VT_FORWARD(T, E); \
    \
    typedef struct { \
        bool is_ok_flag; \
        union { \
            T ok_value; \
            E err_value; \
        }; \
        const ResultVT_##T##_##E *vt; \
    } Result_##T##_##E; \
    \
    /* Vtable function implementations */ \
    static inline bool _result_##T##_##E##_is_ok(const Result_##T##_##E *res) { \
        return res->is_ok_flag; \
    } \
    \
    static inline bool _result_##T##_##E##_is_err(const Result_##T##_##E *res) { \
        return !res->is_ok_flag; \
    } \
    \
    static inline T _result_##T##_##E##_unwrap_ok(const Result_##T##_##E *res) { \
        if (!res->is_ok_flag) CYAN_PANIC("unwrap_ok called on Err"); \
        return res->ok_value; \
    } \
    \
    static inline E _result_##T##_##E##_unwrap_err(const Result_##T##_##E *res) { \
        if (res->is_ok_flag) CYAN_PANIC("unwrap_err called on Ok"); \
        return res->err_value; \
    } \
    \
    static inline T _result_##T##_##E##_unwrap_ok_or(const Result_##T##_##E *res, T default_val) { \
        return res->is_ok_flag ? res->ok_value : default_val; \
    } \
    \
    /* Vtable structure */ \
    struct ResultVT_##T##_##E { \
        bool (*res_is_ok)(const Result_##T##_##E *res); \
        bool (*res_is_err)(const Result_##T##_##E *res); \
        T (*res_unwrap_ok)(const Result_##T##_##E *res); \
        E (*res_unwrap_err)(const Result_##T##_##E *res); \
        T (*res_unwrap_ok_or)(const Result_##T##_##E *res, T default_val); \
    }; \
    \
    /* Static const vtable instance */ \
    static const ResultVT_##T##_##E _result_##T##_##E##_vt __attribute__((unused)) = { \
        .res_is_ok = _result_##T##_##E##_is_ok, \
        .res_is_err = _result_##T##_##E##_is_err, \
        .res_unwrap_ok = _result_##T##_##E##_unwrap_ok, \
        .res_unwrap_err = _result_##T##_##E##_unwrap_err, \
        .res_unwrap_ok_or = _result_##T##_##E##_unwrap_ok_or \
    }

/*============================================================================
 * Constructors
 *============================================================================*/

/**
 * @brief Create a Result containing a success value
 * @param T The success type
 * @param E The error type
 * @param val The success value to wrap
 * @return Result_T_E with is_ok_flag = true and vt pointer set
 * 
 * Example:
 *   Result_int_const_charp x = Ok(int, const_charp, 42);
 */
#define Ok(T, E, val) ((Result_##T##_##E){ .is_ok_flag = true, .ok_value = (val), .vt = &_result_##T##_##E##_vt })

/**
 * @brief Create a Result containing an error value
 * @param T The success type
 * @param E The error type
 * @param err The error value to wrap
 * @return Result_T_E with is_ok_flag = false and vt pointer set
 * 
 * Example:
 *   Result_int_const_charp x = Err(int, const_charp, "parse error");
 */
#define Err(T, E, err) ((Result_##T##_##E){ .is_ok_flag = false, .err_value = (err), .vt = &_result_##T##_##E##_vt })

/*============================================================================
 * Predicates
 *============================================================================*/

/**
 * @brief Check if a Result contains a success value
 * @param res The Result to check
 * @return true if the Result is Ok, false otherwise
 * 
 * Example:
 *   if (is_ok(result)) { ... }
 */
#define is_ok(res) ((res).is_ok_flag)

/**
 * @brief Check if a Result contains an error value
 * @param res The Result to check
 * @return true if the Result is Err, false otherwise
 * 
 * Example:
 *   if (is_err(result)) { ... }
 */
#define is_err(res) (!(res).is_ok_flag)

/*============================================================================
 * Accessors
 *============================================================================*/

/**
 * @brief Extract the success value from a Result, panicking if error
 * @param res The Result to unwrap
 * @return The contained success value
 * @note Panics if the Result is Err
 * 
 * Example:
 *   int val = unwrap_ok(result);  // Panics if result is Err
 */
#define unwrap_ok(res) \
    (is_ok(res) ? (res).ok_value : CYAN_PANIC_EXPR("unwrap_ok called on Err", (res).ok_value))

/**
 * @brief Extract the error value from a Result, panicking if success
 * @param res The Result to unwrap
 * @return The contained error value
 * @note Panics if the Result is Ok
 * 
 * Example:
 *   const char *err = unwrap_err(result);  // Panics if result is Ok
 */
#define unwrap_err(res) \
    (is_err(res) ? (res).err_value : CYAN_PANIC_EXPR("unwrap_err called on Ok", (res).err_value))

/**
 * @brief Extract the success value from a Result, or return a default
 * @param res The Result to unwrap
 * @param default_val The default value if Result is Err
 * @return The contained success value if Ok, otherwise default_val
 * 
 * Example:
 *   int val = unwrap_ok_or(result, 0);  // Returns 0 if result is Err
 */
#define unwrap_ok_or(res, default_val) \
    (is_ok(res) ? (res).ok_value : (default_val))

/*============================================================================
 * Transformations
 *============================================================================*/

/**
 * @brief Transform the success value inside a Result
 * @param res The Result to transform
 * @param T_out The output success type
 * @param E The error type (unchanged)
 * @param fn The transformation function
 * @return Result_T_out_E containing transformed value, or original Err
 * 
 * Example:
 *   Result_int_const_charp x = Ok(int, const_charp, 5);
 *   Result_double_const_charp y = map_result(x, double, const_charp, int_to_double);
 */
#define map_result(res, T_out, E, fn) \
    (is_ok(res) ? Ok(T_out, E, fn((res).ok_value)) : Err(T_out, E, (res).err_value))

/**
 * @brief Transform the error value inside a Result
 * @param res The Result to transform
 * @param T The success type (unchanged)
 * @param E_out The output error type
 * @param fn The transformation function
 * @return Result_T_E_out containing original Ok, or transformed Err
 * 
 * Example:
 *   Result_int_const_charp x = Err(int, const_charp, "error");
 *   Result_int_int y = map_err(x, int, int, error_to_code);
 */
#define map_err(res, T, E_out, fn) \
    (is_ok(res) ? Ok(T, E_out, (res).ok_value) : Err(T, E_out, fn((res).err_value)))

/*============================================================================
 * Vtable Convenience Macros
 *============================================================================*/

/**
 * @brief Check if a Result is success (via vtable)
 * @param res The Result to check
 * @return true if the Result is Ok, false otherwise
 */
#define RES_IS_OK(res) ((res).vt->res_is_ok(&(res)))

/**
 * @brief Check if a Result is error (via vtable)
 * @param res The Result to check
 * @return true if the Result is Err, false otherwise
 */
#define RES_IS_ERR(res) ((res).vt->res_is_err(&(res)))

/**
 * @brief Extract the success value from a Result (via vtable), panicking if error
 * @param res The Result to unwrap
 * @return The contained success value
 * @note Panics if the Result is Err
 */
#define RES_UNWRAP_OK(res) ((res).vt->res_unwrap_ok(&(res)))

/**
 * @brief Extract the error value from a Result (via vtable), panicking if success
 * @param res The Result to unwrap
 * @return The contained error value
 * @note Panics if the Result is Ok
 */
#define RES_UNWRAP_ERR(res) ((res).vt->res_unwrap_err(&(res)))

/**
 * @brief Extract the success value from a Result (via vtable), or return a default
 * @param res The Result to unwrap
 * @param default_val The default value if Result is Err
 * @return The contained success value if Ok, otherwise default_val
 */
#define RES_UNWRAP_OK_OR(res, default_val) ((res).vt->res_unwrap_ok_or(&(res), (default_val)))

#endif /* CYAN_RESULT_H */

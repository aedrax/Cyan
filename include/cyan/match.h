/**
 * @file match.h
 * @brief Pattern matching macros for Option and Result types
 * 
 * This header provides ergonomic pattern matching macros that allow
 * handling Option and Result types more concisely than nested if/else.
 * 
 * Usage:
 *   OPTION_DEFINE(int);
 *   Option_int maybe = Some(int, 42);
 *   
 *   // Statement-based matching
 *   match_option(maybe, int, val,
 *     { printf("Got %d\n", val); },
 *     { printf("Nothing\n"); }
 *   );
 *   
 *   // Expression-based matching
 *   int result = match_option_expr(maybe, int, int, val, val * 2, -1);
 */

#ifndef CYAN_MATCH_H
#define CYAN_MATCH_H

#include "common.h"
#include "option.h"
#include "result.h"

/*============================================================================
 * Option Pattern Matching
 *============================================================================*/

/**
 * @brief Pattern match on an Option type (statement form)
 * @param opt The Option value to match on
 * @param T The type parameter of the Option
 * @param var The variable name to bind the unwrapped value to in the Some branch
 * @param some_branch Code block to execute if Option is Some
 * @param none_branch Code block to execute if Option is None
 * 
 * The some_branch has access to a variable named `var` containing the unwrapped value.
 * 
 * Example:
 *   match_option(maybe, int, val,
 *     { printf("Got %d\n", val); },
 *     { printf("Nothing\n"); }
 *   );
 */
#define match_option(opt, T, var, some_branch, none_branch) do { \
    Option_##T CYAN_UNIQUE(_opt_val_) = (opt); \
    if (is_some(CYAN_UNIQUE(_opt_val_))) { \
        T var = CYAN_UNIQUE(_opt_val_).value; \
        (void)var; \
        some_branch \
    } else { \
        none_branch \
    } \
} while(0)

/**
 * @brief Pattern match on an Option type (expression form)
 * @param opt The Option value to match on
 * @param T The type parameter of the Option
 * @param T_out The return type of the expression
 * @param var The variable name to bind the unwrapped value to in the Some expression
 * @param some_expr Expression to evaluate if Option is Some (can use `var`)
 * @param none_expr Expression to evaluate if Option is None
 * @return The result of evaluating either some_expr or none_expr
 * 
 * Example:
 *   int doubled = match_option_expr(maybe, int, int, v, v * 2, 0);
 */
#define match_option_expr(opt, T, T_out, var, some_expr, none_expr) \
    __extension__ ({ \
        Option_##T CYAN_UNIQUE(_opt_val_) = (opt); \
        T_out CYAN_UNIQUE(_result_); \
        if (is_some(CYAN_UNIQUE(_opt_val_))) { \
            T var = CYAN_UNIQUE(_opt_val_).value; \
            (void)var; \
            CYAN_UNIQUE(_result_) = (T_out)(some_expr); \
        } else { \
            CYAN_UNIQUE(_result_) = (T_out)(none_expr); \
        } \
        CYAN_UNIQUE(_result_); \
    })

/*============================================================================
 * Result Pattern Matching
 *============================================================================*/

/**
 * @brief Pattern match on a Result type (statement form)
 * @param res The Result value to match on
 * @param T The success type parameter of the Result
 * @param E The error type parameter of the Result
 * @param ok_var The variable name to bind the unwrapped Ok value to
 * @param err_var The variable name to bind the unwrapped Err value to
 * @param ok_branch Code block to execute if Result is Ok
 * @param err_branch Code block to execute if Result is Err
 * 
 * The ok_branch has access to `ok_var` containing the success value.
 * The err_branch has access to `err_var` containing the error value.
 * 
 * Example:
 *   match_result(result, int, const_charp, val, err,
 *     { printf("Success: %d\n", val); },
 *     { printf("Error: %s\n", err); }
 *   );
 */
#define match_result(res, T, E, ok_var, err_var, ok_branch, err_branch) do { \
    Result_##T##_##E CYAN_UNIQUE(_res_val_) = (res); \
    if (is_ok(CYAN_UNIQUE(_res_val_))) { \
        T ok_var = CYAN_UNIQUE(_res_val_).ok_value; \
        (void)ok_var; \
        ok_branch \
    } else { \
        E err_var = CYAN_UNIQUE(_res_val_).err_value; \
        (void)err_var; \
        err_branch \
    } \
} while(0)

/**
 * @brief Pattern match on a Result type (expression form)
 * @param res The Result value to match on
 * @param T The success type parameter of the Result
 * @param E The error type parameter of the Result
 * @param T_out The return type of the expression
 * @param ok_var The variable name to bind the unwrapped Ok value to
 * @param err_var The variable name to bind the unwrapped Err value to
 * @param ok_expr Expression to evaluate if Result is Ok (can use `ok_var`)
 * @param err_expr Expression to evaluate if Result is Err (can use `err_var`)
 * @return The result of evaluating either ok_expr or err_expr
 * 
 * Example:
 *   int value = match_result_expr(result, int, const_charp, int, v, e, v * 2, -1);
 */
#define match_result_expr(res, T, E, T_out, ok_var, err_var, ok_expr, err_expr) \
    __extension__ ({ \
        Result_##T##_##E CYAN_UNIQUE(_res_val_) = (res); \
        T_out CYAN_UNIQUE(_result_); \
        if (is_ok(CYAN_UNIQUE(_res_val_))) { \
            T ok_var = CYAN_UNIQUE(_res_val_).ok_value; \
            (void)ok_var; \
            CYAN_UNIQUE(_result_) = (T_out)(ok_expr); \
        } else { \
            E err_var = CYAN_UNIQUE(_res_val_).err_value; \
            (void)err_var; \
            CYAN_UNIQUE(_result_) = (T_out)(err_expr); \
        } \
        CYAN_UNIQUE(_result_); \
    })

/*============================================================================
 * Helper Macros (for more Rust-like syntax)
 *============================================================================*/

/**
 * @brief Helper to create a Some pattern in match context
 * This is syntactic sugar for documentation purposes.
 * 
 * Note: These helpers are provided for documentation and readability.
 * The actual pattern matching is done by match_option/match_result macros.
 */
#define some(var) var
#define none() /* empty - no binding needed */
#define ok(var) var
#define err(var) var

#endif /* CYAN_MATCH_H */

/**
 * @file common.h
 * @brief Common macros, types, and panic handler for the Cyan library
 * 
 * This header provides shared foundation components used across all
 * Cyan library modules including panic handling, utility macros,
 * and type definitions for pre-C23 compatibility.
 */

#ifndef CYAN_COMMON_H
#define CYAN_COMMON_H

#include <stdio.h>
#include <stdlib.h>

/*============================================================================
 * Panic Handler
 *============================================================================
 * The panic handler is invoked for unrecoverable errors such as:
 * - unwrap() on None
 * - unwrap_ok() on Err
 * - unwrap_err() on Ok
 * - Memory allocation failures
 * - Resuming a finished coroutine
 *
 * Users can override this by defining CYAN_PANIC before including any
 * Cyan headers.
 */

#ifndef CYAN_PANIC
/**
 * @brief Default panic handler that prints error info and aborts
 * @param msg The panic message describing the error
 * 
 * Prints file, line number, and message to stderr before aborting.
 * Override by defining CYAN_PANIC before including Cyan headers.
 */
#define CYAN_PANIC(msg) do { \
    fprintf(stderr, "PANIC at %s:%d: %s\n", __FILE__, __LINE__, msg); \
    abort(); \
} while(0)
#endif

/*============================================================================
 * Utility Macros
 *============================================================================*/

/**
 * @brief Internal concatenation helper (do not use directly)
 */
#define CYAN_CONCAT_(a, b) a##b

/**
 * @brief Concatenate two tokens
 * @param a First token
 * @param b Second token
 * @return Concatenated token
 * 
 * Example: CYAN_CONCAT(foo, bar) -> foobar
 */
#define CYAN_CONCAT(a, b) CYAN_CONCAT_(a, b)

/**
 * @brief Generate a unique identifier using line number
 * @param prefix The prefix for the identifier
 * @return A unique identifier like prefix_42 (where 42 is the line number)
 * 
 * Useful for generating unique variable names in macros to avoid
 * name collisions.
 */
#define CYAN_UNIQUE(prefix) CYAN_CONCAT(prefix##_, __LINE__)

/**
 * @brief Stringify a token
 * @param x Token to stringify
 * @return String literal of the token
 */
#define CYAN_STRINGIFY_(x) #x
#define CYAN_STRINGIFY(x) CYAN_STRINGIFY_(x)

/*============================================================================
 * Boolean Type (pre-C23 compatibility)
 *============================================================================
 * C23 introduces native bool, true, false keywords. For earlier standards,
 * we provide compatibility definitions using _Bool.
 */

#if !defined(__bool_true_false_are_defined) && !defined(__cplusplus)
#if __STDC_VERSION__ < 202311L
/**
 * @brief Boolean type for pre-C23 compatibility
 */
typedef _Bool bool;

/**
 * @brief Boolean true value
 */
#define true 1

/**
 * @brief Boolean false value
 */
#define false 0

#define __bool_true_false_are_defined 1
#endif
#endif

/*============================================================================
 * Configuration Guards
 *============================================================================
 * These macros allow users to configure library behavior via preprocessor
 * defines before including headers.
 */

/**
 * @brief Default initial capacity for dynamic collections
 * Override by defining CYAN_DEFAULT_CAPACITY before including headers.
 */
#ifndef CYAN_DEFAULT_CAPACITY
#define CYAN_DEFAULT_CAPACITY 4
#endif

/**
 * @brief Growth factor for dynamic collections (as a multiplier)
 * Override by defining CYAN_GROWTH_FACTOR before including headers.
 */
#ifndef CYAN_GROWTH_FACTOR
#define CYAN_GROWTH_FACTOR 2
#endif

/**
 * @brief Default coroutine stack size in bytes
 * Override by defining CYAN_CORO_STACK_SIZE before including headers.
 */
#ifndef CYAN_CORO_STACK_SIZE
#define CYAN_CORO_STACK_SIZE (64 * 1024)
#endif

/*============================================================================
 * Version Information
 *============================================================================*/

#define CYAN_VERSION_MAJOR 0
#define CYAN_VERSION_MINOR 1
#define CYAN_VERSION_PATCH 0
#define CYAN_VERSION_STRING "0.1.0"

#endif /* CYAN_COMMON_H */

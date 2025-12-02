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
#include <stdint.h>
#include <stddef.h>

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

/**
 * @brief Internal helper to panic and return a dummy value for expression context
 * @param msg The panic message
 * @param dummy A dummy value to satisfy type requirements (never returned)
 * 
 * This macro is used in expression contexts where a value must be returned
 * (e.g., ternary operators). The dummy value is never actually returned
 * because abort() is called first.
 */
#define CYAN_PANIC_EXPR(msg, dummy) \
    (fprintf(stderr, "PANIC at %s:%d: %s\n", __FILE__, __LINE__, msg), abort(), (dummy))

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

/*============================================================================
 * Primitive Type Aliases
 *============================================================================
 * Concise type names with predictable sizes for cleaner code.
 * Platform-specific types are conditionally available based on compiler
 * and architecture support.
 */

/*----------------------------------------------------------------------------
 * Signed Integer Aliases
 *----------------------------------------------------------------------------*/

/**
 * @brief 8-bit signed integer
 */
typedef int8_t i8;

/**
 * @brief 16-bit signed integer
 */
typedef int16_t i16;

/**
 * @brief 32-bit signed integer
 */
typedef int32_t i32;

/**
 * @brief 64-bit signed integer
 */
typedef int64_t i64;

/**
 * @brief 128-bit signed integer (platform-dependent)
 * Only available when CYAN_HAS_INT128 is 1
 */
#if defined(__SIZEOF_INT128__) || defined(__int128)
typedef __int128 i128;
#define CYAN_HAS_INT128 1
#else
#define CYAN_HAS_INT128 0
#if !defined(CYAN_SUPPRESS_TYPE_WARNINGS)
#warning "128-bit integer types not available on this platform"
#endif
#endif

/*----------------------------------------------------------------------------
 * Unsigned Integer Aliases
 *----------------------------------------------------------------------------*/

/**
 * @brief 8-bit unsigned integer
 */
typedef uint8_t u8;

/**
 * @brief 16-bit unsigned integer
 */
typedef uint16_t u16;

/**
 * @brief 32-bit unsigned integer
 */
typedef uint32_t u32;

/**
 * @brief 64-bit unsigned integer
 */
typedef uint64_t u64;

/**
 * @brief 128-bit unsigned integer (platform-dependent)
 * Only available when CYAN_HAS_INT128 is 1
 */
#if CYAN_HAS_INT128
typedef unsigned __int128 u128;
#endif

/*----------------------------------------------------------------------------
 * Pointer-Sized Integer Aliases
 *----------------------------------------------------------------------------*/

/**
 * @brief Signed pointer-sized integer
 * Guaranteed to be able to hold any pointer value when cast
 */
typedef intptr_t isize;

/**
 * @brief Unsigned pointer-sized integer
 * Compatible with size_t for array indexing
 */
typedef uintptr_t usize;

/*----------------------------------------------------------------------------
 * Floating-Point Type Aliases
 *----------------------------------------------------------------------------*/

/**
 * @brief 16-bit floating-point (platform-dependent)
 * Only available when CYAN_HAS_FLOAT16 is 1
 */
#if defined(__FLT16_MAX__) || defined(__STDC_IEC_60559_TYPES__)
typedef _Float16 f16;
#define CYAN_HAS_FLOAT16 1
#else
#define CYAN_HAS_FLOAT16 0
#if !defined(CYAN_SUPPRESS_TYPE_WARNINGS)
#warning "16-bit float type (_Float16) not available on this platform"
#endif
#endif

/**
 * @brief 32-bit floating-point (IEEE-754 binary32)
 */
typedef float f32;

/**
 * @brief 64-bit floating-point (IEEE-754 binary64)
 */
typedef double f64;

/**
 * @brief 80-bit extended precision floating-point (x86 only)
 * Only available when CYAN_HAS_FLOAT80 is 1
 */
#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)
typedef long double f80;
#define CYAN_HAS_FLOAT80 1
#else
#define CYAN_HAS_FLOAT80 0
#if !defined(CYAN_SUPPRESS_TYPE_WARNINGS)
#warning "80-bit float type not available on this platform"
#endif
#endif

/**
 * @brief 128-bit floating-point (IEEE-754 binary128, platform-dependent)
 * Only available when CYAN_HAS_FLOAT128 is 1
 */
#if defined(__FLT128_MAX__) || defined(__SIZEOF_FLOAT128__)
typedef _Float128 f128;
#define CYAN_HAS_FLOAT128 1
#else
#define CYAN_HAS_FLOAT128 0
#if !defined(CYAN_SUPPRESS_TYPE_WARNINGS)
#warning "128-bit float type (_Float128) not available on this platform"
#endif
#endif

/*----------------------------------------------------------------------------
 * Special Types
 *----------------------------------------------------------------------------*/

/**
 * @brief Type-erased pointer type
 * Use as anyopaque* for generic/opaque data pointers
 * 
 * Example:
 *   anyopaque* ptr = some_data;
 *   int* typed_ptr = (int*)ptr;
 */
typedef void anyopaque;

#endif /* CYAN_COMMON_H */

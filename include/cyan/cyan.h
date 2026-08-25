/**
 * @file cyan.h
 * @brief Umbrella header for the Cyan library
 * 
 * This header includes all Cyan library components. Include this single
 * header to access the complete library functionality, or include
 * individual headers for finer-grained control over compilation.
 * 
 * @section usage Usage
 * @code
 * #include <cyan/cyan.h>
 * 
 * // Now all Cyan types and macros are available:
 * // Option, Result, Vec, Slice, String, HashMap, etc.
 * @endcode
 * 
 * @section configuration Configuration
 * The library can be configured via preprocessor defines before including:
 * - CYAN_PANIC(msg) - Override the panic handler
 * - CYAN_DEFAULT_CAPACITY - Initial capacity for collections (default: 4)
 * - CYAN_GROWTH_FACTOR - Growth multiplier for collections (default: 2)
 * - CYAN_CORO_STACK_SIZE - Coroutine stack size in bytes (default: 64KB)
 * - CYAN_CHANNEL_THREADSAFE - Enable thread-safe channels
 */

#ifndef CYAN_H
#define CYAN_H

/*============================================================================
 * Version Information
 *============================================================================
 * CYAN_VERSION_MAJOR/MINOR/PATCH, CYAN_VERSION_STRING, CYAN_VERSION, and
 * CYAN_VERSION_AT_LEAST are defined in common.h (included below).
 */

/*============================================================================
 * Feature Detection Macros
 *============================================================================*/

/** @brief 1 when the defer mechanism is available (GCC nested functions or
 *  Clang blocks), 0 otherwise */
#if defined(__GNUC__) || defined(__clang__)
#define CYAN_HAS_DEFER 1
#else
#define CYAN_HAS_DEFER 0
#endif

/** @brief 1 when coroutines are available (requires POSIX ucontext), 0 otherwise */
#if defined(__unix__) || defined(__unix) || defined(__APPLE__)
#define CYAN_HAS_CORO 1
#else
#define CYAN_HAS_CORO 0
#endif

/** @brief Defined when HashSet is available */
#define CYAN_HAS_HASHSET 1

/*============================================================================
 * Compiler Feature Detection
 *============================================================================*/

/** @brief Check for GCC/Clang cleanup attribute support */
#if defined(__GNUC__) || defined(__clang__)
#define CYAN_HAS_CLEANUP_ATTR 1
#else
#define CYAN_HAS_CLEANUP_ATTR 0
#endif

/** @brief Check for C11 _Generic support */
#if __STDC_VERSION__ >= 201112L
#define CYAN_HAS_GENERIC 1
#else
#define CYAN_HAS_GENERIC 0
#endif

/** @brief Check for statement expressions (GCC extension) */
#if defined(__GNUC__) || defined(__clang__)
#define CYAN_HAS_STMT_EXPR 1
#else
#define CYAN_HAS_STMT_EXPR 0
#endif

/*============================================================================
 * Component Headers
 *============================================================================
 * Headers are included in dependency order for proper compilation.
 */

/* Foundation - must be first */
#include "common.h"

/* Core types - no dependencies beyond common.h */
#include "option.h"
#include "result.h"

/* Collections - depend on Option */
#include "vector.h"
#include "slice.h"
#include "string.h"
#include "hashmap.h"
#include "hashset.h"

/* Functional primitives - work with collections */
#include "functional.h"

/* Serialization - uses Result */
#include "serialize.h"

/* Resource management */
#if CYAN_HAS_DEFER
#include "defer.h"
#endif
#include "smartptr.h"

/* Pattern matching - works with Option and Result */
#include "match.h"

/* Concurrency */
#if CYAN_HAS_CORO
#include "coro.h"
#endif
#include "channel.h"

/* Bit manipulation */
#include "bitint.h"
#include "bitset.h"

#endif /* CYAN_H */

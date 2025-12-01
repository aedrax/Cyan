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
 *============================================================================*/

/** @brief Major version number */
#define CYAN_VERSION_MAJOR 0

/** @brief Minor version number */
#define CYAN_VERSION_MINOR 1

/** @brief Patch version number */
#define CYAN_VERSION_PATCH 0

/** @brief Full version as string */
#define CYAN_VERSION_STRING "0.1.0"

/** @brief Version as single integer for comparison: (major * 10000 + minor * 100 + patch) */
#define CYAN_VERSION ((CYAN_VERSION_MAJOR * 10000) + (CYAN_VERSION_MINOR * 100) + CYAN_VERSION_PATCH)

/** @brief Check if Cyan version is at least the specified version */
#define CYAN_VERSION_AT_LEAST(major, minor, patch) \
    (CYAN_VERSION >= ((major) * 10000 + (minor) * 100 + (patch)))

/*============================================================================
 * Feature Detection Macros
 *============================================================================*/

/** @brief Defined when Option type is available */
#define CYAN_HAS_OPTION 1

/** @brief Defined when Result type is available */
#define CYAN_HAS_RESULT 1

/** @brief Defined when Vector type is available */
#define CYAN_HAS_VECTOR 1

/** @brief Defined when Slice type is available */
#define CYAN_HAS_SLICE 1

/** @brief Defined when functional primitives (map, filter, reduce) are available */
#define CYAN_HAS_FUNCTIONAL 1

/** @brief Defined when defer mechanism is available */
#define CYAN_HAS_DEFER 1

/** @brief Defined when coroutines are available */
#define CYAN_HAS_CORO 1

/** @brief Defined when serialization is available */
#define CYAN_HAS_SERIALIZE 1

/** @brief Defined when smart pointers are available */
#define CYAN_HAS_SMARTPTR 1

/** @brief Defined when HashMap is available */
#define CYAN_HAS_HASHMAP 1

/** @brief Defined when dynamic String is available */
#define CYAN_HAS_STRING 1

/** @brief Defined when pattern matching macros are available */
#define CYAN_HAS_MATCH 1

/** @brief Defined when channels are available */
#define CYAN_HAS_CHANNEL 1

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

/* Functional primitives - work with collections */
#include "functional.h"

/* Serialization - uses Result */
#include "serialize.h"

/* Resource management */
#include "defer.h"
#include "smartptr.h"

/* Pattern matching - works with Option and Result */
#include "match.h"

/* Concurrency */
#include "coro.h"
#include "channel.h"

#endif /* CYAN_H */

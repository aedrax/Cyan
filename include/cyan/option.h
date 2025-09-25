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
 * Example:
 *   OPTION_DEFINE(int);    // Creates Option_int
 *   OPTION_DEFINE(double); // Creates Option_double
 */
#define OPTION_DEFINE(T) \
    typedef struct { \
        bool has_value; \
        T value; \
    } Option_##T

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
 * Predicates
 *============================================================================*/

/**
 * @brief Check if an Option contains a value
 * @param opt The Option to check
 * @return true if the Option contains a value, false otherwise
 * 
 * Example:
 *   if (is_some(maybe)) { ... }
 */
#define is_some(opt) ((opt).has_value)

/**
 * @brief Check if an Option is empty
 * @param opt The Option to check
 * @return true if the Option is empty, false otherwise
 * 
 * Example:
 *   if (is_none(maybe)) { ... }
 */
#define is_none(opt) (!(opt).has_value)

/*============================================================================
 * Accessors
 *============================================================================*/

/**
 * @brief Extract the value from an Option, panicking if empty
 * @param opt The Option to unwrap
 * @return The contained value
 * @note Panics if the Option is None
 * 
 * Example:
 *   int val = unwrap(maybe);  // Panics if maybe is None
 */
#define unwrap(opt) \
    (is_some(opt) ? (opt).value : CYAN_PANIC_EXPR("unwrap called on None", (opt).value))

/**
 * @brief Extract the value from an Option, or return a default
 * @param opt The Option to unwrap
 * @param default_val The default value if Option is empty
 * @return The contained value if present, otherwise default_val
 * 
 * Example:
 *   int val = unwrap_or(maybe, 0);  // Returns 0 if maybe is None
 */
#define unwrap_or(opt, default_val) \
    (is_some(opt) ? (opt).value : (default_val))

/*============================================================================
 * Transformations
 *============================================================================*/

/**
 * @brief Transform the value inside an Option
 * @param opt The Option to transform
 * @param T_out The output type
 * @param fn The transformation function
 * @return Option_T_out containing transformed value, or None if input was None
 * 
 * Example:
 *   Option_int x = Some(int, 5);
 *   Option_double y = map_option(x, double, int_to_double);
 */
#define map_option(opt, T_out, fn) \
    (is_some(opt) ? Some(T_out, fn((opt).value)) : None(T_out))

#endif /* CYAN_OPTION_H */

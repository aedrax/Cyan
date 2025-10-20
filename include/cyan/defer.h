/**
 * @file defer.h
 * @brief Scope-based cleanup mechanism for the Cyan library
 * 
 * This header provides defer functionality that makes sure cleanup code
 * executes when the current scope exits, regardless of the exit path
 * (normal, return, break, continue).
 * 
 * The implementation uses GCC/Clang's __attribute__((cleanup)) extension
 * for automatic scope-based cleanup.
 */

#ifndef CYAN_DEFER_H
#define CYAN_DEFER_H

#include "common.h"

/*============================================================================
 * Defer Context Structure
 *============================================================================
 * The _DeferCtx structure holds the deferred function and its context.
 * When the variable goes out of scope, the cleanup attribute triggers
 * _defer_cleanup which executes the deferred code.
 */

/**
 * @brief Function pointer type for deferred cleanup functions
 */
typedef void (*_cyan_defer_fn)(void *);

/**
 * @brief Context structure for defer mechanism
 * 
 * Holds the function to execute and optional context pointer.
 * The cleanup attribute makes sure _defer_cleanup is called when
 * this structure goes out of scope.
 */
typedef struct {
    _cyan_defer_fn fn;
    void *ctx;
} _CyanDeferCtx;

/**
 * @brief Cleanup handler called automatically when _CyanDeferCtx goes out of scope
 * @param d Pointer to the defer context
 * 
 * This function is invoked by the cleanup attribute when the defer
 * variable leaves scope. It executes the stored function if non-NULL.
 */
static inline void _cyan_defer_cleanup(_CyanDeferCtx *d) {
    if (d->fn) {
        d->fn(d->ctx);
    }
}

/*============================================================================
 * Defer Macros
 *============================================================================
 * These macros provide the user-facing defer functionality.
 * 
 * The defer() macro uses nested functions (a GCC extension) to capture
 * the deferred code block. The __attribute__((cleanup)) makes sure the
 * cleanup function is called when the scope exits.
 * 
 * Multiple defers in the same scope execute in LIFO order because
 * cleanup attributes are processed in reverse order of declaration.
 */

/**
 * @brief Defer execution of a code block until scope exit
 * @param code The code block to execute on scope exit
 * 
 * The deferred code executes when the enclosing scope exits via:
 * - Normal flow (reaching end of block)
 * - return statement
 * - break statement
 * - continue statement
 * 
 * Multiple defers execute in LIFO order (last declared, first executed).
 * 
 * Note: Variables are captured by value at the point of defer declaration.
 * This is because the nested function captures the current values.
 * 
 * Example:
 * @code
 * void example(void) {
 *     FILE *f = fopen("file.txt", "r");
 *     defer({ fclose(f); });
 *     // ... use file ...
 *     // fclose(f) is called automatically when scope exits
 * }
 * @endcode
 */
#define defer(code) \
    auto void CYAN_CONCAT(_cyan_defer_fn_, __LINE__)(void *_unused) { \
        (void)_unused; \
        code \
    } \
    __attribute__((cleanup(_cyan_defer_cleanup))) \
    _CyanDeferCtx CYAN_CONCAT(_cyan_defer_ctx_, __LINE__) = { \
        .fn = (_cyan_defer_fn)CYAN_CONCAT(_cyan_defer_fn_, __LINE__), \
        .ctx = NULL \
    }

/*============================================================================
 * Convenience Macros
 *============================================================================*/

/**
 * @brief Internal cleanup function for defer_free
 * @param p Pointer to the pointer to free
 */
static inline void _cyan_defer_free_impl(void **p) {
    if (*p) {
        free(*p);
        *p = NULL;
    }
}

/**
 * @brief Defer freeing a pointer until scope exit
 * @param ptr The pointer to free on scope exit
 * 
 * Convenience macro for the common pattern of freeing allocated memory.
 * The pointer is set to NULL after freeing to prevent double-free.
 * 
 * Example:
 * @code
 * void example(void) {
 *     char *buf = malloc(1024);
 *     defer_free(buf);
 *     // ... use buffer ...
 *     // free(buf) is called automatically when scope exits
 * }
 * @endcode
 */
#define defer_free(ptr) \
    __attribute__((cleanup(_cyan_defer_free_impl))) \
    void *CYAN_CONCAT(_cyan_df_, __LINE__) = (ptr)

/*============================================================================
 * Value Capture Defer
 *============================================================================
 * For cases where you need to capture a specific value at defer declaration
 * time (not the variable's value at execution time), use defer_capture.
 */

/**
 * @brief Defer context with captured integer value
 */
typedef struct {
    _cyan_defer_fn fn;
    int captured_value;
} _CyanDeferCaptureInt;

/**
 * @brief Cleanup handler for captured int defer
 */
static inline void _cyan_defer_capture_int_cleanup(_CyanDeferCaptureInt *d) {
    if (d->fn) {
        d->fn(&d->captured_value);
    }
}

/**
 * @brief Defer with explicit integer value capture
 * @param val The integer value to capture
 * @param code The code block to execute (use _captured_val to access the value)
 * 
 * This macro captures the value of an integer at declaration time,
 * ensuring the deferred code uses that value even if the original
 * variable changes before scope exit.
 * 
 * Example:
 * @code
 * void example(void) {
 *     int x = 10;
 *     defer_capture_int(x, { printf("captured: %d\n", _captured_val); });
 *     x = 20;  // This change doesn't affect the deferred code
 *     // Prints "captured: 10" on scope exit
 * }
 * @endcode
 */
#define defer_capture_int(val, code) \
    auto void CYAN_CONCAT(_cyan_defer_cap_fn_, __LINE__)(void *_cap_ptr) { \
        int _captured_val = *(int *)_cap_ptr; \
        (void)_captured_val; \
        code \
    } \
    __attribute__((cleanup(_cyan_defer_capture_int_cleanup))) \
    _CyanDeferCaptureInt CYAN_CONCAT(_cyan_defer_cap_ctx_, __LINE__) = { \
        .fn = (_cyan_defer_fn)CYAN_CONCAT(_cyan_defer_cap_fn_, __LINE__), \
        .captured_value = (val) \
    }

#endif /* CYAN_DEFER_H */

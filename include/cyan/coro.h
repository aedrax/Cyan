/**
 * @file coro.h
 * @brief Stackful coroutines for cooperative multitasking
 * 
 * This header provides stackful coroutine support that allows writing
 * asynchronous code in a sequential style. Coroutines can yield control
 * back to the caller and resume from where they left off.
 * 
 * The implementation uses POSIX ucontext for context switching on
 * supported platforms, providing true stackful coroutines.
 * 
 * Usage:
 *   void my_coro(Coro *self, void *arg) {
 *       for (int i = 0; i < 5; i++) {
 *           coro_yield_value(self, i);
 *       }
 *   }
 *   
 *   Coro *c = coro_new(my_coro, NULL, 0);
 *   while (!coro_is_finished(c)) {
 *       coro_resume(c);
 *       if (!coro_is_finished(c)) {
 *           int val = coro_get_yield(c, int);
 *       }
 *   }
 *   coro_free(c);
 */

#ifndef CYAN_CORO_H
#define CYAN_CORO_H

#include "common.h"
#include <string.h>

/* Platform detection for context switching */
#if defined(__unix__) || defined(__unix) || defined(__APPLE__)
#define CYAN_CORO_USE_UCONTEXT 1
#include <ucontext.h>
#else
#error "Coroutines require POSIX ucontext support"
#endif

/*============================================================================
 * Coroutine Status
 *============================================================================*/

/**
 * @brief Coroutine execution status
 */
typedef enum {
    CORO_CREATED,    /**< Coroutine created but never resumed */
    CORO_RUNNING,    /**< Coroutine currently executing */
    CORO_SUSPENDED,  /**< Coroutine yielded and waiting to resume */
    CORO_FINISHED    /**< Coroutine completed execution */
} CoroStatus;

/*============================================================================
 * Coroutine Structure
 *============================================================================*/

/* Forward declaration */
typedef struct Coro Coro;

/**
 * @brief Coroutine function signature
 * @param self Pointer to the coroutine structure
 * @param arg User-provided argument
 */
typedef void (*CoroFn)(Coro *self, void *arg);

/**
 * @brief Coroutine structure containing execution state
 */
struct Coro {
    CoroStatus status;       /**< Current execution status */
    void *stack;             /**< Allocated stack memory */
    size_t stack_size;       /**< Size of allocated stack */
    ucontext_t caller_ctx;   /**< Context of the caller */
    ucontext_t coro_ctx;     /**< Context of the coroutine */
    void *yield_value;       /**< Pointer to yielded value storage */
    size_t yield_size;       /**< Size of yielded value */
    CoroFn fn;               /**< Coroutine function */
    void *arg;               /**< User argument */
    char yield_buffer[64];   /**< Internal buffer for small yield values */
};

/*============================================================================
 * Internal Functions
 *============================================================================*/

/**
 * @brief Internal coroutine entry point wrapper
 * @param c Pointer to coroutine (passed as two ints for makecontext)
 */
static void _coro_entry(int lo, int hi) {
    /* Reconstruct pointer from two ints (required by makecontext) */
    Coro *c = (Coro *)(((uintptr_t)(unsigned int)hi << 32) | (unsigned int)lo);
    
    c->status = CORO_RUNNING;
    c->fn(c, c->arg);
    c->status = CORO_FINISHED;
    
    /* Return to caller when coroutine completes */
    setcontext(&c->caller_ctx);
}

/**
 * @brief Internal yield implementation
 * @param c Coroutine to yield from
 * @param value Pointer to value to yield (can be NULL)
 * @param size Size of the value
 */
static inline void _coro_yield_impl(Coro *c, const void *value, size_t size) {
    if (value && size > 0) {
        if (size <= sizeof(c->yield_buffer)) {
            memcpy(c->yield_buffer, value, size);
            c->yield_value = c->yield_buffer;
        } else {
            /* For large values, just store the pointer (caller must verify lifetime) */
            c->yield_value = (void *)value;
        }
        c->yield_size = size;
    } else {
        c->yield_value = NULL;
        c->yield_size = 0;
    }
    
    c->status = CORO_SUSPENDED;
    swapcontext(&c->coro_ctx, &c->caller_ctx);
    c->status = CORO_RUNNING;
}

/*============================================================================
 * Public API
 *============================================================================*/

/**
 * @brief Create a new coroutine
 * @param fn The coroutine function to execute
 * @param arg User argument passed to the coroutine function
 * @param stack_size Size of the coroutine stack (0 for default)
 * @return Pointer to the new coroutine, or NULL on failure
 * 
 * The coroutine is created in CORO_CREATED state and must be resumed
 * to start execution.
 * 
 * Example:
 * @code
 * Coro *c = coro_new(my_generator, NULL, 0);
 * @endcode
 */
static inline Coro *coro_new(CoroFn fn, void *arg, size_t stack_size) {
    if (!fn) {
        return NULL;
    }
    
    if (stack_size == 0) {
        stack_size = CYAN_CORO_STACK_SIZE;
    }
    
    Coro *c = (Coro *)malloc(sizeof(Coro));
    if (!c) {
        CYAN_PANIC("coro_new: allocation failed");
        return NULL;
    }
    
    c->stack = malloc(stack_size);
    if (!c->stack) {
        free(c);
        CYAN_PANIC("coro_new: stack allocation failed");
        return NULL;
    }
    
    c->status = CORO_CREATED;
    c->stack_size = stack_size;
    c->fn = fn;
    c->arg = arg;
    c->yield_value = NULL;
    c->yield_size = 0;
    
    /* Initialize coroutine context */
    if (getcontext(&c->coro_ctx) == -1) {
        free(c->stack);
        free(c);
        CYAN_PANIC("coro_new: getcontext failed");
        return NULL;
    }
    
    c->coro_ctx.uc_stack.ss_sp = c->stack;
    c->coro_ctx.uc_stack.ss_size = stack_size;
    c->coro_ctx.uc_link = NULL;  /* We handle return manually */
    
    /* Split pointer into two ints for makecontext (portable approach) */
    uintptr_t ptr = (uintptr_t)c;
    int lo = (int)(ptr & 0xFFFFFFFF);
    int hi = (int)((ptr >> 32) & 0xFFFFFFFF);
    
    makecontext(&c->coro_ctx, (void (*)(void))_coro_entry, 2, lo, hi);
    
    return c;
}

/**
 * @brief Resume coroutine execution
 * @param c The coroutine to resume
 * @return true if the coroutine yielded, false if it finished
 * 
 * Transfers control to the coroutine. Returns when the coroutine
 * yields or completes.
 * 
 * Example:
 * @code
 * while (coro_resume(c)) {
 *     // Process yielded value
 * }
 * @endcode
 */
static inline bool coro_resume(Coro *c) {
    if (!c) {
        CYAN_PANIC("coro_resume: NULL coroutine");
        return false;
    }
    
    if (c->status == CORO_FINISHED) {
        CYAN_PANIC("coro_resume: cannot resume finished coroutine");
        return false;
    }
    
    if (c->status == CORO_RUNNING) {
        CYAN_PANIC("coro_resume: coroutine already running");
        return false;
    }
    
    /* Save caller context and switch to coroutine */
    swapcontext(&c->caller_ctx, &c->coro_ctx);
    
    return c->status != CORO_FINISHED;
}

/**
 * @brief Yield from within a coroutine (no value)
 * @param c The coroutine to yield from
 * 
 * Suspends the coroutine and returns control to the caller.
 * 
 * Example:
 * @code
 * void my_coro(Coro *self, void *arg) {
 *     // Do some work
 *     coro_yield(self);
 *     // Continue after resume
 * }
 * @endcode
 */
#define coro_yield(c) _coro_yield_impl(c, NULL, 0)

/**
 * @brief Yield from within a coroutine with a value
 * @param c The coroutine to yield from
 * @param val The value to yield
 * 
 * Suspends the coroutine and makes the value available to the caller.
 * 
 * Example:
 * @code
 * void generator(Coro *self, void *arg) {
 *     for (int i = 0; i < 10; i++) {
 *         coro_yield_value(self, i);
 *     }
 * }
 * @endcode
 */
#define coro_yield_value(c, val) do { \
    __typeof__(val) _coro_tmp_val = (val); \
    _coro_yield_impl(c, &_coro_tmp_val, sizeof(_coro_tmp_val)); \
} while(0)

/**
 * @brief Get the value yielded by a coroutine
 * @param c The coroutine
 * @param T The type of the yielded value
 * @return The yielded value
 * 
 * Example:
 * @code
 * coro_resume(c);
 * int val = coro_get_yield(c, int);
 * @endcode
 */
#define coro_get_yield(c, T) (*((T *)((c)->yield_value)))

/**
 * @brief Check if a coroutine has finished
 * @param c The coroutine to check
 * @return true if the coroutine has completed, false otherwise
 */
static inline bool coro_is_finished(Coro *c) {
    return c && c->status == CORO_FINISHED;
}

/**
 * @brief Get the current status of a coroutine
 * @param c The coroutine to check
 * @return The current CoroStatus
 */
static inline CoroStatus coro_status(Coro *c) {
    if (!c) {
        CYAN_PANIC("coro_status: NULL coroutine");
        return CORO_FINISHED;
    }
    return c->status;
}

/**
 * @brief Free a coroutine and its resources
 * @param c The coroutine to free
 * 
 * Frees the coroutine's stack and structure. The coroutine should
 * not be resumed after being freed.
 * 
 * Example:
 * @code
 * coro_free(c);
 * c = NULL;
 * @endcode
 */
static inline void coro_free(Coro *c) {
    if (c) {
        if (c->stack) {
            free(c->stack);
        }
        free(c);
    }
}

#endif /* CYAN_CORO_H */

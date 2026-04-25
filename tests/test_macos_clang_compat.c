/**
 * @file test_macos_clang_compat.c
 * @brief Bug condition exploration test for macOS/Apple Clang compatibility
 *
 * Property 1: Bug Condition - macOS/Clang Compilation Failures
 *
 * This file exercises all code paths that trigger compilation failures
 * when building the Cyan library with Apple Clang on macOS:
 *
 *   Bug 1.1 - coro.h: ucontext functions undeclared without _XOPEN_SOURCE
 *   Bug 1.2 - defer.h: defer() uses GCC nested functions (auto void)
 *   Bug 1.3 - defer.h: defer_capture_int() uses GCC nested functions
 *
 * EXPECTED BEHAVIOR ON UNFIXED CODE:
 *   Compilation with clang FAILS - this confirms the bugs exist.
 *
 * EXPECTED BEHAVIOR AFTER FIX:
 *   Compilation with clang SUCCEEDS and the program runs correctly.
 *
 * Additional checks (run separately via shell):
 *   Bug 1.4 - common.h emits float type warnings on ARM64 macOS
 *   Bug 1.5 - Makefile hardcodes CC = gcc (not overridable)
 */

#include <stdio.h>
#include <cyan/defer.h>
#include <cyan/coro.h>

/*============================================================================
 * Bug 1.2 / 1.3: defer() and defer_capture_int() exercise
 *============================================================================
 * These use GCC nested functions (auto void) which Apple Clang rejects.
 */

/**
 * @brief Exercise defer() - triggers nested function compilation on Clang
 */
static void test_defer_basic(void) {
    defer_var(int, cleanup_ran, 0);
    {
        defer({
            cleanup_ran = 1;
        });
    }
    printf("  defer basic: cleanup_ran = %d (expected 1)\n", cleanup_ran);
}

/**
 * @brief Exercise defer() LIFO ordering - multiple defers in one scope
 */
static void test_defer_lifo(void) {
    defer_var(int, order, 0);
    defer_var(int, first, 0);
    defer_var(int, second, 0);
    {
        defer({
            first = ++order;
        });
        defer({
            second = ++order;
        });
    }
    /* LIFO: second defer runs first, then first defer */
    printf("  defer LIFO: second=%d first=%d (expected second=1, first=2)\n",
           second, first);
}

/**
 * @brief Exercise defer_capture_int() - triggers nested function compilation
 */
static void test_defer_capture(void) {
    defer_var(int, result, 0);
    {
        int val = 42;
        defer_capture_int(val, {
            result = _captured_val;
        });
        val = 999;  /* Should NOT affect captured value */
    }
    printf("  defer_capture_int: result = %d (expected 42)\n", result);
}

/*============================================================================
 * Bug 1.1: Coroutine exercise (ucontext)
 *============================================================================
 * On macOS without _XOPEN_SOURCE, ucontext functions are undeclared.
 */

/**
 * @brief Simple coroutine that yields integers 0..4
 */
static void counting_coro(Coro *self, void *arg) {
    int limit = *(int *)arg;
    for (int i = 0; i < limit; i++) {
        coro_yield_value(self, i);
    }
}

/**
 * @brief Exercise coroutine creation, resume, yield, and free
 */
static void test_coro_basic(void) {
    int limit = 5;
    Coro *c = coro_new(counting_coro, &limit, 0);
    if (!c) {
        printf("  coro basic: FAILED to create coroutine\n");
        return;
    }

    int sum = 0;
    while (coro_resume(c)) {
        int val = coro_get_yield(c, int);
        sum += val;
    }
    coro_free(c);

    /* 0+1+2+3+4 = 10 */
    printf("  coro basic: sum = %d (expected 10)\n", sum);
}

/*============================================================================
 * Main - run all exercises
 *============================================================================*/

int main(void) {
    printf("=== macOS/Clang Compatibility Exploration Test ===\n\n");

    printf("Bug 1.2 - defer() basic:\n");
    test_defer_basic();

    printf("\nBug 1.2 - defer() LIFO ordering:\n");
    test_defer_lifo();

    printf("\nBug 1.3 - defer_capture_int():\n");
    test_defer_capture();

    printf("\nBug 1.1 - coroutine (ucontext):\n");
    test_coro_basic();

    printf("\n=== All exercises completed successfully ===\n");
    return 0;
}

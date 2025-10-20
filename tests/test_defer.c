/**
 * @file test_defer.c
 * @brief Property-based tests for Defer mechanism
 * 
 * Tests validate correctness properties:
 * - Property 20: Defer executes on scope exit
 * - Property 21: Multiple defers execute in LIFO order
 * - Property 22: Nested scope defers execute inner-first
 * - Property 23: Defer captures values at declaration time
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theft.h"
#include <cyan/defer.h>

/* Minimum iterations for property tests */
#define MIN_TEST_TRIALS 100

/*============================================================================
 * Test Helpers
 *============================================================================*/

/* Global counters for tracking defer execution */
static int g_defer_counter = 0;
static int g_defer_order[10];
static int g_defer_order_idx = 0;

static void reset_counters(void) {
    g_defer_counter = 0;
    g_defer_order_idx = 0;
    for (int i = 0; i < 10; i++) {
        g_defer_order[i] = 0;
    }
}

/*============================================================================
 * Property 20: Defer executes on scope exit
 * For any scope containing a defer statement, the deferred code SHALL execute
 * when the scope exits via any path (normal, return, break, continue).
 *============================================================================*/

 /* Helper function to test defer with normal scope exit */
static void test_defer_block_exit(int val, int *out) {
    {
        defer({ *out = val; });
        /* Normal block exit */
    }
    /* Defer should have executed by now */
}

/* Helper function to test defer with break */
static void test_defer_break_exit(int val, int *out) {
    for (int i = 0; i < 1; i++) {
        defer({ *out = val; });
        break;
    }
    /* Defer should have executed by now */
}

/* Helper function to test defer with continue */
static void test_defer_continue_exit(int val, int *out) {
    for (int i = 0; i < 2; i++) {
        if (i == 0) {
            defer({ *out = val; });
            continue;
        }
    }
    /* Defer should have executed by now */
}

static enum theft_trial_res prop_defer_executes_on_scope_exit(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr % 10000);  /* Keep value reasonable */
    if (val == 0) val = 1;  /* Avoid zero to distinguish from uninitialized */
    
    int result = 0;
    
    /* Test 1: Normal block exit */
    result = 0;
    test_defer_block_exit(val, &result);
    if (result != val) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test 2: Break exit */
    result = 0;
    test_defer_break_exit(val, &result);
    if (result != val) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test 3: Continue exit */
    result = 0;
    test_defer_continue_exit(val, &result);
    if (result != val) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 21: Multiple defers execute in LIFO order
 * For any scope with n defer statements, they SHALL execute in reverse order
 * of declaration (last declared executes first).
 *============================================================================*/

static void test_lifo_order(void) {
    reset_counters();
    {
        defer({ g_defer_order[g_defer_order_idx++] = 1; });
        defer({ g_defer_order[g_defer_order_idx++] = 2; });
        defer({ g_defer_order[g_defer_order_idx++] = 3; });
    }
}

static enum theft_trial_res prop_defer_lifo_order(struct theft *t, void *arg1) {
    (void)t;
    (void)arg1;
    
    test_lifo_order();
    
    /* Defers should execute in reverse order: 3, 2, 1 */
    if (g_defer_order_idx != 3) {
        return THEFT_TRIAL_FAIL;
    }
    if (g_defer_order[0] != 3 || g_defer_order[1] != 2 || g_defer_order[2] != 1) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 22: Nested scope defers execute inner-first
 * For any nested scopes with defers, inner scope defers SHALL execute before
 * outer scope defers.
 *============================================================================*/

static void test_nested_order(void) {
    reset_counters();
    {
        defer({ g_defer_order[g_defer_order_idx++] = 1; });  /* Outer */
        {
            defer({ g_defer_order[g_defer_order_idx++] = 2; });  /* Inner */
        }
        /* Inner defer (2) should have executed here */
    }
    /* Outer defer (1) should have executed here */
}

static enum theft_trial_res prop_defer_nested_order(struct theft *t, void *arg1) {
    (void)t;
    (void)arg1;
    
    test_nested_order();
    
    /* Inner defer should execute first (2), then outer (1) */
    if (g_defer_order_idx != 2) {
        return THEFT_TRIAL_FAIL;
    }
    if (g_defer_order[0] != 2 || g_defer_order[1] != 1) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 23: Defer captures values at declaration time
 * For any defer statement referencing a variable, the deferred code SHALL use
 * the variable's value at the point of defer declaration, not at execution time.
 *============================================================================*/

static int g_captured_value = 0;

static void test_value_capture(int initial_val) {
    g_captured_value = 0;
    {
        int x = initial_val;
        defer_capture_int(x, { g_captured_value = _captured_val; });
        x = initial_val + 100;  /* Change x after defer declaration */
        (void)x;
    }
}

static enum theft_trial_res prop_defer_value_capture(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr % 10000);  /* Keep value reasonable */
    
    test_value_capture(val);
    
    /* The captured value should be the original, not the modified one */
    if (g_captured_value != val) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Test Registration
 *============================================================================*/

typedef struct {
    const char *name;
    theft_propfun1 *prop;
    enum theft_builtin_type_info type;
} DeferTest;

static DeferTest defer_tests[] = {
    {
        "Property 20: Defer executes on scope exit",
        prop_defer_executes_on_scope_exit,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 21: Multiple defers execute in LIFO order",
        prop_defer_lifo_order,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 22: Nested scope defers execute inner-first",
        prop_defer_nested_order,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 23: Defer captures values at declaration time",
        prop_defer_value_capture,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_DEFER_TESTS (sizeof(defer_tests) / sizeof(defer_tests[0]))

int run_defer_tests(theft_seed seed) {
    int failures = 0;
    
    printf("\nDefer Mechanism Tests:\n");
    
    for (size_t i = 0; i < NUM_DEFER_TESTS; i++) {
        DeferTest *test = &defer_tests[i];
        
        struct theft_run_config config = {
            .name = test->name,
            .prop1 = test->prop,
            .type_info = { theft_get_builtin_type_info(test->type) },
            .trials = MIN_TEST_TRIALS,
            .seed = seed ? seed : theft_seed_of_time(),
        };
        
        enum theft_run_res res = theft_run(&config);
        
        const char *status;
        switch (res) {
            case THEFT_RUN_PASS:
                status = "\033[32mPASS\033[0m";
                break;
            case THEFT_RUN_FAIL:
                status = "\033[31mFAIL\033[0m";
                failures++;
                break;
            default:
                status = "\033[31mERROR\033[0m";
                failures++;
                break;
        }
        printf("  [%s] %s\n", status, test->name);
    }
    
    return failures;
}

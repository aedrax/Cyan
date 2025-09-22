/**
 * @file test_option.c
 * @brief Property-based tests for Option type
 * 
 * Tests validate correctness properties:
 * - Property 1: Some round-trip
 * - Property 2: None behavior
 * - Property 3: is_some and is_none are inverses
 * - Property 4: unwrap_or returns value or default
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theft.h"
#include <cyan/option.h>

/* Define Option types for testing */
OPTION_DEFINE(int);
OPTION_DEFINE(uint64_t);

/*============================================================================
 * Property 1: Some round-trip
 * For any value, Some(val) unwrapped equals val, and is_some() returns true
 *============================================================================*/

static enum theft_trial_res prop_some_roundtrip(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    Option_int opt = Some(int, val);
    
    /* is_some must return true for Some */
    if (!is_some(opt)) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* unwrap must return the original value */
    int unwrapped = unwrap(opt);
    if (unwrapped != val) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 2: None behavior
 * For any type, None results in is_none() true and is_some() false
 *============================================================================*/

static enum theft_trial_res prop_none_behavior(struct theft *t, void *arg1) {
    (void)t;
    (void)arg1;  /* We don't need the generated value, just testing None */
    
    Option_int opt = None(int);
    
    /* is_none must return true for None */
    if (!is_none(opt)) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* is_some must return false for None */
    if (is_some(opt)) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 3: is_some and is_none are inverses
 * For any Option, is_some(opt) == !is_none(opt)
 *============================================================================*/

static enum theft_trial_res prop_some_none_inverse(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    /* Test with Some */
    Option_int some_opt = Some(int, val);
    if (is_some(some_opt) != !is_none(some_opt)) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test with None */
    Option_int none_opt = None(int);
    if (is_some(none_opt) != !is_none(none_opt)) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 4: unwrap_or returns value or default
 * For any Option and default, unwrap_or returns contained value if Some, else default
 *============================================================================*/

static enum theft_trial_res prop_unwrap_or(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    int default_val = val + 1;  /* Make sure default is different */
    
    /* Test with Some - should return contained value */
    Option_int some_opt = Some(int, val);
    int result_some = unwrap_or(some_opt, default_val);
    if (result_some != val) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test with None - should return default */
    Option_int none_opt = None(int);
    int result_none = unwrap_or(none_opt, default_val);
    if (result_none != default_val) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Test Registration
 *============================================================================*/

/* Minimum iterations for property tests */
#define MIN_TEST_TRIALS 100

typedef struct {
    const char *name;
    theft_propfun1 *prop;
    enum theft_builtin_type_info type;
} OptionTest;

static OptionTest option_tests[] = {
    {
        "Property 1: Some round-trip",
        prop_some_roundtrip,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 2: None behavior",
        prop_none_behavior,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 3: is_some/is_none inverse",
        prop_some_none_inverse,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 4: unwrap_or returns value or default",
        prop_unwrap_or,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_OPTION_TESTS (sizeof(option_tests) / sizeof(option_tests[0]))

int run_option_tests(theft_seed seed) {
    int failures = 0;
    
    printf("\nOption Type Tests:\n");
    
    for (size_t i = 0; i < NUM_OPTION_TESTS; i++) {
        OptionTest *test = &option_tests[i];
        
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

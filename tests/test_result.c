/**
 * @file test_result.c
 * @brief Property-based tests for Result type
 * 
 * Tests validate correctness properties:
 * - Property 5: Ok round-trip
 * - Property 6: Err round-trip
 * - Property 7: is_ok and is_err are inverses
 * - Property 8: unwrap_ok_or returns value or default
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theft.h"
#include <cyan/result.h>

/* Define Result types for testing */
RESULT_DEFINE(int, int);
RESULT_DEFINE(uint64_t, uint64_t);

/*============================================================================
 * Property 5: Ok round-trip
 * For any value, Ok(val) unwrapped equals val, and is_ok() returns true
 *============================================================================*/

static enum theft_trial_res prop_ok_roundtrip(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    Result_int_int res = Ok(int, int, val);
    
    /* is_ok must return true for Ok */
    if (!is_ok(res)) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* unwrap_ok must return the original value */
    int unwrapped = unwrap_ok(res);
    if (unwrapped != val) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6: Err round-trip
 * For any error, Err(err) unwrapped equals err, and is_err() returns true
 *============================================================================*/

static enum theft_trial_res prop_err_roundtrip(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int err_val = (int)(*val_ptr);
    
    Result_int_int res = Err(int, int, err_val);
    
    /* is_err must return true for Err */
    if (!is_err(res)) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* unwrap_err must return the original error value */
    int unwrapped = unwrap_err(res);
    if (unwrapped != err_val) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 7: is_ok and is_err are inverses
 * For any Result, is_ok(res) == !is_err(res)
 *============================================================================*/

static enum theft_trial_res prop_ok_err_inverse(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    /* Test with Ok */
    Result_int_int ok_res = Ok(int, int, val);
    if (is_ok(ok_res) != !is_err(ok_res)) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test with Err */
    Result_int_int err_res = Err(int, int, val);
    if (is_ok(err_res) != !is_err(err_res)) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 8: unwrap_ok_or returns value or default
 * For any Result and default, unwrap_ok_or returns Ok value if is_ok, else default
 *============================================================================*/

static enum theft_trial_res prop_unwrap_ok_or(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    int default_val = val + 1;  /* Make sure default is different */
    
    /* Test with Ok - should return contained value */
    Result_int_int ok_res = Ok(int, int, val);
    int result_ok = unwrap_ok_or(ok_res, default_val);
    if (result_ok != val) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test with Err - should return default */
    Result_int_int err_res = Err(int, int, val);
    int result_err = unwrap_ok_or(err_res, default_val);
    if (result_err != default_val) {
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
} ResultTest;

static ResultTest result_tests[] = {
    {
        "Property 5: Ok round-trip",
        prop_ok_roundtrip,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 6: Err round-trip",
        prop_err_roundtrip,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 7: is_ok/is_err inverse",
        prop_ok_err_inverse,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 8: unwrap_ok_or returns value or default",
        prop_unwrap_ok_or,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_RESULT_TESTS (sizeof(result_tests) / sizeof(result_tests[0]))

int run_result_tests(theft_seed seed) {
    int failures = 0;
    
    printf("\nResult Type Tests:\n");
    
    for (size_t i = 0; i < NUM_RESULT_TESTS; i++) {
        ResultTest *test = &result_tests[i];
        
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

/**
 * @file test_result.c
 * @brief Property-based tests for Result type
 * 
 * Tests validate correctness properties:
 * - Property 5: Ok round-trip
 * - Property 6: Err round-trip
 * - Property 7: is_ok and is_err are inverses
 * - Property 8: unwrap_ok_or returns value or default
 * - Property 9: Result macro == function behavioral equivalence
 * - Property 10: map/map_err/and_then_result combinators
 * - Property 11: expect_ok/try_ok behavior
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
 * Property 9: Result macro == function behavioral equivalence
 * For any Result, the RES_* macros produce identical results to the
 * generated result_T_E_* standalone functions.
 *============================================================================*/

static enum theft_trial_res prop_result_macro_fn_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    int default_val = val + 1;

    /* Test with Ok */
    Result_int_int ok_res = Ok(int, int, val);

    /* is_ok equivalence */
    if (RES_IS_OK(ok_res) != result_int_int_is_ok(&ok_res)) {
        return THEFT_TRIAL_FAIL;
    }

    /* is_err equivalence */
    if (RES_IS_ERR(ok_res) != result_int_int_is_err(&ok_res)) {
        return THEFT_TRIAL_FAIL;
    }

    /* unwrap_ok equivalence */
    if (RES_UNWRAP_OK(ok_res) != result_int_int_unwrap_ok(&ok_res)) {
        return THEFT_TRIAL_FAIL;
    }

    /* unwrap_ok_or equivalence */
    if (RES_UNWRAP_OK_OR(ok_res, default_val) != result_int_int_unwrap_ok_or(&ok_res, default_val)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test with Err */
    Result_int_int err_res = Err(int, int, val);

    /* is_ok equivalence */
    if (RES_IS_OK(err_res) != result_int_int_is_ok(&err_res)) {
        return THEFT_TRIAL_FAIL;
    }

    /* is_err equivalence */
    if (RES_IS_ERR(err_res) != result_int_int_is_err(&err_res)) {
        return THEFT_TRIAL_FAIL;
    }

    /* unwrap_err equivalence */
    if (RES_UNWRAP_ERR(err_res) != result_int_int_unwrap_err(&err_res)) {
        return THEFT_TRIAL_FAIL;
    }

    /* unwrap_ok_or equivalence for Err */
    if (RES_UNWRAP_OK_OR(err_res, default_val) != result_int_int_unwrap_ok_or(&err_res, default_val)) {
        return THEFT_TRIAL_FAIL;
    }

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 10: map/map_err/and_then_result combinators
 * RES_MAP transforms the Ok value, RES_MAP_ERR transforms the Err value,
 * and_then_result chains a fallible transformation on the Ok value.
 *============================================================================*/

static int _res_double(int x) { return x * 2; }

static int _res_negate(int e) { return -e; }

static Result_int_int _res_half_if_even(int x) {
    if (x % 2 == 0) return Ok(int, int, x / 2);
    return Err(int, int, -100);
}

static enum theft_trial_res prop_result_combinators(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr % 100000);  /* Keep doubling in range */

    Result_int_int ok_res = Ok(int, int, val);
    Result_int_int err_res = Err(int, int, val);

    /* RES_MAP transforms the Ok value */
    Result_int_int mapped = RES_MAP(ok_res, int, int, _res_double);
    if (!is_ok(mapped) || unwrap_ok(mapped) != val * 2) {
        return THEFT_TRIAL_FAIL;
    }

    /* map_result passes Err through unchanged */
    Result_int_int mapped_err = map_result(err_res, int, int, _res_double);
    if (!is_err(mapped_err) || unwrap_err(mapped_err) != val) {
        return THEFT_TRIAL_FAIL;
    }

    /* RES_MAP_ERR transforms the Err value */
    Result_int_int err_mapped = RES_MAP_ERR(err_res, int, int, _res_negate);
    if (!is_err(err_mapped) || unwrap_err(err_mapped) != -val) {
        return THEFT_TRIAL_FAIL;
    }

    /* map_err passes Ok through unchanged */
    Result_int_int ok_kept = map_err(ok_res, int, int, _res_negate);
    if (!is_ok(ok_kept) || unwrap_ok(ok_kept) != val) {
        return THEFT_TRIAL_FAIL;
    }

    /* and_then_result chains the fallible transformation on Ok */
    Result_int_int chained = and_then_result(ok_res, int, int, _res_half_if_even);
    if (val % 2 == 0) {
        if (!is_ok(chained) || unwrap_ok(chained) != val / 2) {
            return THEFT_TRIAL_FAIL;
        }
    } else {
        if (!is_err(chained) || unwrap_err(chained) != -100) {
            return THEFT_TRIAL_FAIL;
        }
    }

    /* and_then_result propagates Err without calling fn */
    Result_int_int chained_err = and_then_result(err_res, int, int, _res_half_if_even);
    if (!is_err(chained_err) || unwrap_err(chained_err) != val) {
        return THEFT_TRIAL_FAIL;
    }

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 11: expect_ok/try_ok behavior
 * expect_ok on Ok returns the value, try_ok unwraps Ok and early-returns
 * the whole Err Result from the enclosing function.
 *============================================================================*/

/* Helper for try_ok: enclosing function must return the same Result type */
static Result_int_int _res_try_double(Result_int_int in) {
    int v = try_ok(in);  /* Returns the Err to the caller on Err */
    return Ok(int, int, v * 2);
}

static enum theft_trial_res prop_result_expect_try(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr % 100000);  /* Keep doubling in range */

    Result_int_int ok_res = Ok(int, int, val);

    /* expect_ok on Ok returns the contained value (no panic) */
    if (expect_ok(ok_res, "expect_ok on Ok must not panic") != val) {
        return THEFT_TRIAL_FAIL;
    }

    /* try_ok unwraps an Ok and lets the helper continue */
    Result_int_int doubled = _res_try_double(Ok(int, int, val));
    if (!is_ok(doubled) || unwrap_ok(doubled) != val * 2) {
        return THEFT_TRIAL_FAIL;
    }

    /* try_ok early-returns the Err from the helper */
    Result_int_int propagated = _res_try_double(Err(int, int, val));
    if (!is_err(propagated) || unwrap_err(propagated) != val) {
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
    {
        "Property 9: Result macro == function behavioral equivalence",
        prop_result_macro_fn_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 10: map/map_err/and_then_result combinators",
        prop_result_combinators,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 11: expect_ok/try_ok behavior",
        prop_result_expect_try,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_RESULT_TESTS (sizeof(result_tests) / sizeof(result_tests[0]))

int run_result_tests(theft_seed seed, int *num_tests) {
    int failures = 0;
    
    *num_tests = (int)NUM_RESULT_TESTS;
    
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

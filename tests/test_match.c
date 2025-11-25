/**
 * @file test_match.c
 * @brief Property-based tests for pattern matching macros
 * 
 * Tests validate correctness properties:
 * - Property 51: Option match executes correct branch
 * - Property 52: Result match executes correct branch
 * - Property 53: Expression matching returns correct value
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theft.h"
#include <cyan/match.h>

/* Define Option and Result types for testing */
OPTION_DEFINE(int);
RESULT_DEFINE(int, int);

/*============================================================================
 * Property 51: Option match executes correct branch
 * For any Option value, match_option SHALL execute the Some branch with the
 * unwrapped value if is_some, otherwise the None branch.
 *============================================================================*/

static enum theft_trial_res prop_option_match_correct_branch(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    /* Test with Some - should execute Some branch */
    {
        Option_int some_opt = Some(int, val);
        int branch_executed = 0;
        int captured_value = 0;
        
        match_option(some_opt, int, v,
            {
                branch_executed = 1;
                captured_value = v;
            },
            {
                branch_executed = 2;
            }
        );
        
        if (branch_executed != 1) {
            return THEFT_TRIAL_FAIL;
        }
        if (captured_value != val) {
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test with None - should execute None branch */
    {
        Option_int none_opt = None(int);
        int branch_executed = 0;
        
        match_option(none_opt, int, v,
            {
                branch_executed = 1;
                (void)v;
            },
            {
                branch_executed = 2;
            }
        );
        
        if (branch_executed != 2) {
            return THEFT_TRIAL_FAIL;
        }
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 52: Result match executes correct branch
 * For any Result value, match_result SHALL execute the Ok branch with the
 * unwrapped value if is_ok, otherwise the Err branch with the error.
 *============================================================================*/

static enum theft_trial_res prop_result_match_correct_branch(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    int err_val = val + 100;  /* Different value for error */
    
    /* Test with Ok - should execute Ok branch */
    {
        Result_int_int ok_res = Ok(int, int, val);
        int branch_executed = 0;
        int captured_value = 0;
        
        match_result(ok_res, int, int, v, e,
            {
                branch_executed = 1;
                captured_value = v;
            },
            {
                branch_executed = 2;
                (void)e;
            }
        );
        
        if (branch_executed != 1) {
            return THEFT_TRIAL_FAIL;
        }
        if (captured_value != val) {
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test with Err - should execute Err branch */
    {
        Result_int_int err_res = Err(int, int, err_val);
        int branch_executed = 0;
        int captured_error = 0;
        
        match_result(err_res, int, int, v, e,
            {
                branch_executed = 1;
                (void)v;
            },
            {
                branch_executed = 2;
                captured_error = e;
            }
        );
        
        if (branch_executed != 2) {
            return THEFT_TRIAL_FAIL;
        }
        if (captured_error != err_val) {
            return THEFT_TRIAL_FAIL;
        }
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 53: Expression matching returns correct value
 * For any Option or Result, match_option_expr and match_result_expr SHALL
 * return the value computed by the executed branch.
 *============================================================================*/

static enum theft_trial_res prop_expression_matching(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    int default_val = -999;
    int err_val = val + 100;
    
    /* Test match_option_expr with Some */
    {
        Option_int some_opt = Some(int, val);
        int result = match_option_expr(some_opt, int, int, v, v * 2, default_val);
        
        if (result != val * 2) {
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test match_option_expr with None */
    {
        Option_int none_opt = None(int);
        int result = match_option_expr(none_opt, int, int, v, v * 2, default_val);
        
        if (result != default_val) {
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test match_result_expr with Ok */
    {
        Result_int_int ok_res = Ok(int, int, val);
        int result = match_result_expr(ok_res, int, int, int, v, e, v * 3, e + 1);
        
        if (result != val * 3) {
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test match_result_expr with Err */
    {
        Result_int_int err_res = Err(int, int, err_val);
        int result = match_result_expr(err_res, int, int, int, v, e, v * 3, e + 1);
        
        if (result != err_val + 1) {
            return THEFT_TRIAL_FAIL;
        }
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
} MatchTest;

static MatchTest match_tests[] = {
    {
        "Property 51: Option match executes correct branch",
        prop_option_match_correct_branch,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 52: Result match executes correct branch",
        prop_result_match_correct_branch,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 53: Expression matching returns correct value",
        prop_expression_matching,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_MATCH_TESTS (sizeof(match_tests) / sizeof(match_tests[0]))

int run_match_tests(theft_seed seed) {
    int failures = 0;
    
    printf("\nPattern Matching Tests:\n");
    
    for (size_t i = 0; i < NUM_MATCH_TESTS; i++) {
        MatchTest *test = &match_tests[i];
        
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

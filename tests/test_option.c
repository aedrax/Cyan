/**
 * @file test_option.c
 * @brief Property-based tests for Option type
 * 
 * Tests validate correctness properties:
 * - Property 1: Some round-trip
 * - Property 2: None behavior
 * - Property 3: is_some and is_none are inverses
 * - Property 4: unwrap_or returns value or default
 * - Property 5: Option macro == function behavioral equivalence
 * - Property 6: map/and_then/or_else combinators
 * - Property 7: expect/ok_or/try_some behavior
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theft.h"
#include <cyan/option.h>
#include <cyan/result.h>

/* Define Option types for testing */
OPTION_DEFINE(int);
OPTION_DEFINE(uint64_t);

/* Define Result type for ok_or conversion tests */
RESULT_DEFINE(int, int);

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
 * Property 5: Option macro == function behavioral equivalence
 * For any Option, the OPT_* macros produce identical results to the
 * generated option_T_* standalone functions.
 *============================================================================*/

static enum theft_trial_res prop_option_macro_fn_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    int default_val = val + 1;

    /* Test with Some */
    Option_int some_opt = Some(int, val);

    /* is_some equivalence */
    if (OPT_IS_SOME(some_opt) != option_int_is_some(&some_opt)) {
        return THEFT_TRIAL_FAIL;
    }

    /* is_none equivalence */
    if (OPT_IS_NONE(some_opt) != option_int_is_none(&some_opt)) {
        return THEFT_TRIAL_FAIL;
    }

    /* unwrap equivalence */
    if (OPT_UNWRAP(some_opt) != option_int_unwrap(&some_opt)) {
        return THEFT_TRIAL_FAIL;
    }

    /* unwrap_or equivalence */
    if (OPT_UNWRAP_OR(some_opt, default_val) != option_int_unwrap_or(&some_opt, default_val)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test with None */
    Option_int none_opt = None(int);

    /* is_some equivalence */
    if (OPT_IS_SOME(none_opt) != option_int_is_some(&none_opt)) {
        return THEFT_TRIAL_FAIL;
    }

    /* is_none equivalence */
    if (OPT_IS_NONE(none_opt) != option_int_is_none(&none_opt)) {
        return THEFT_TRIAL_FAIL;
    }

    /* unwrap_or equivalence for None */
    if (OPT_UNWRAP_OR(none_opt, default_val) != option_int_unwrap_or(&none_opt, default_val)) {
        return THEFT_TRIAL_FAIL;
    }

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6: map/and_then/or_else combinators
 * OPT_MAP transforms Some and passes None through, and_then chains fallible
 * transformations, or_else keeps Some and replaces None with a fallback.
 *============================================================================*/

static int _opt_double(int x) { return x * 2; }

static Option_int _opt_half_if_even(int x) {
    if (x % 2 == 0) return Some(int, x / 2);
    return None(int);
}

static Option_int _opt_fallback_seven(void) { return Some(int, 7); }

static enum theft_trial_res prop_option_combinators(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr % 100000);  /* Keep doubling in range */

    Option_int some_opt = Some(int, val);
    Option_int none_opt = None(int);

    /* OPT_MAP transforms the Some value */
    Option_int mapped = OPT_MAP(some_opt, int, _opt_double);
    if (!is_some(mapped) || unwrap(mapped) != val * 2) {
        return THEFT_TRIAL_FAIL;
    }

    /* map_option passes None through */
    Option_int mapped_none = map_option(none_opt, int, _opt_double);
    if (!is_none(mapped_none)) {
        return THEFT_TRIAL_FAIL;
    }

    /* and_then chains the fallible transformation on Some */
    Option_int chained = and_then(some_opt, int, _opt_half_if_even);
    if (val % 2 == 0) {
        if (!is_some(chained) || unwrap(chained) != val / 2) {
            return THEFT_TRIAL_FAIL;
        }
    } else {
        if (!is_none(chained)) {
            return THEFT_TRIAL_FAIL;
        }
    }

    /* and_then on None stays None without calling fn */
    Option_int chained_none = and_then(none_opt, int, _opt_half_if_even);
    if (!is_none(chained_none)) {
        return THEFT_TRIAL_FAIL;
    }

    /* or_else keeps an existing Some untouched */
    Option_int kept = or_else(some_opt, _opt_fallback_seven);
    if (!is_some(kept) || unwrap(kept) != val) {
        return THEFT_TRIAL_FAIL;
    }

    /* or_else replaces None with the fallback */
    Option_int replaced = or_else(none_opt, _opt_fallback_seven);
    if (!is_some(replaced) || unwrap(replaced) != 7) {
        return THEFT_TRIAL_FAIL;
    }

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 7: expect/ok_or/try_some behavior
 * expect on Some returns the value, ok_or converts Some/None into Ok/Err,
 * try_some unwraps Some and early-returns None from the enclosing function.
 *============================================================================*/

/* Helper for try_some: enclosing function must return the same Option type */
static Option_int _opt_try_double(Option_int in) {
    int v = try_some(in);  /* Returns the None to the caller on None */
    return Some(int, v * 2);
}

static enum theft_trial_res prop_option_expect_ok_or_try(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr % 100000);  /* Keep doubling in range */

    Option_int some_opt = Some(int, val);
    Option_int none_opt = None(int);

    /* expect on Some returns the contained value (no panic) */
    if (expect(some_opt, "expect on Some must not panic") != val) {
        return THEFT_TRIAL_FAIL;
    }

    /* ok_or converts Some into Ok */
    Result_int_int ok_res = ok_or(some_opt, int, int, -1);
    if (!is_ok(ok_res) || unwrap_ok(ok_res) != val) {
        return THEFT_TRIAL_FAIL;
    }

    /* ok_or converts None into Err with the given error value */
    Result_int_int err_res = ok_or(none_opt, int, int, -1);
    if (!is_err(err_res) || unwrap_err(err_res) != -1) {
        return THEFT_TRIAL_FAIL;
    }

    /* try_some unwraps a Some and lets the helper continue */
    Option_int doubled = _opt_try_double(Some(int, val));
    if (!is_some(doubled) || unwrap(doubled) != val * 2) {
        return THEFT_TRIAL_FAIL;
    }

    /* try_some early-returns the None from the helper */
    Option_int propagated = _opt_try_double(None(int));
    if (!is_none(propagated)) {
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
    {
        "Property 5: Option macro == function behavioral equivalence",
        prop_option_macro_fn_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 6: map/and_then/or_else combinators",
        prop_option_combinators,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 7: expect/ok_or/try_some behavior",
        prop_option_expect_ok_or_try,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_OPTION_TESTS (sizeof(option_tests) / sizeof(option_tests[0]))

int run_option_tests(theft_seed seed, int *num_tests) {
    int failures = 0;
    
    *num_tests = (int)NUM_OPTION_TESTS;
    
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

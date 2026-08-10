/**
 * @file test_slice.c
 * @brief Property-based tests for Slice type
 * 
 * Tests validate correctness properties:
 * - Property 13: slice get returns correct Option based on bounds
 * - Property 14: subslice elements match original
 * - Property 15: slice length matches source
 * - Property 16: Slice macro == function behavioral equivalence
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theft.h"
#include <cyan/option.h>
#include <cyan/vector.h>
#include <cyan/slice.h>

/* Define Option, Vector, and Slice types for testing */
OPTION_DEFINE(int);
VECTOR_DEFINE(int);
SLICE_DEFINE(int);

/*============================================================================
 * Property 13: slice get returns correct Option based on bounds
 * For any slice of length n and index i, get(i) returns Some with the correct element if i < n, otherwise None.
 *============================================================================*/

static enum theft_trial_res prop_slice_get_bounds(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int base_val = (int)(*val_ptr);
    
    /* Create an array with known values */
    int arr[5];
    for (int i = 0; i < 5; i++) {
        arr[i] = base_val + i;
    }
    
    Slice_int s = slice_int_from_array(arr, 5);

    /* Test valid indices return Some with correct value */
    for (size_t i = 0; i < 5; i++) {
        Option_int opt = slice_int_get(s, i);
        if (!is_some(opt)) {
            return THEFT_TRIAL_FAIL;
        }
        if (unwrap(opt) != arr[i]) {
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test out-of-bounds indices return None */
    Option_int at_len = slice_int_get(s, 5);
    if (!is_none(at_len)) {
        return THEFT_TRIAL_FAIL;
    }
    
    Option_int beyond_len = slice_int_get(s, 100);
    if (!is_none(beyond_len)) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test empty slice */
    Slice_int empty = slice_int_from_array(arr, 0);
    Option_int empty_get = slice_int_get(empty, 0);
    if (!is_none(empty_get)) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 14: subslice elements match original
 * For any slice and valid subslice range [start, end), elements in the subslice
 * equal corresponding elements in the original slice at indices start+i.
 *============================================================================*/

static enum theft_trial_res prop_subslice_correctness(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int base_val = (int)(*val_ptr);
    
    /* Create an array with known values */
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = base_val + i * 10;
    }
    
    Slice_int s = slice_int_from_array(arr, 10);

    /* Test subslice [2, 7) */
    size_t start = 2;
    size_t end = 7;
    Slice_int sub = slice_int_subslice(s, start, end);
    
    /* Verify subslice length */
    if (slice_int_len(sub) != end - start) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify each element matches original at start+i */
    for (size_t i = 0; i < slice_int_len(sub); i++) {
        Option_int sub_elem = slice_int_get(sub, i);
        Option_int orig_elem = slice_int_get(s, start + i);
        
        if (!is_some(sub_elem) || !is_some(orig_elem)) {
            return THEFT_TRIAL_FAIL;
        }
        if (unwrap(sub_elem) != unwrap(orig_elem)) {
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test edge case: subslice with start > end (should clamp) */
    Slice_int clamped = slice_int_subslice(s, 5, 3);
    if (slice_int_len(clamped) != 0) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test edge case: subslice beyond bounds (should clamp) */
    Slice_int beyond = slice_int_subslice(s, 8, 15);
    if (slice_int_len(beyond) != 2) {  /* Should be [8, 10) */
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify clamped subslice elements */
    for (size_t i = 0; i < slice_int_len(beyond); i++) {
        Option_int sub_elem = slice_int_get(beyond, i);
        Option_int orig_elem = slice_int_get(s, 8 + i);
        
        if (!is_some(sub_elem) || !is_some(orig_elem)) {
            return THEFT_TRIAL_FAIL;
        }
        if (unwrap(sub_elem) != unwrap(orig_elem)) {
            return THEFT_TRIAL_FAIL;
        }
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 15: slice length matches source
 * For any array of length n, creating a slice from it produces a slice with length n.
 *============================================================================*/

static enum theft_trial_res prop_slice_length(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    /* Use absolute value and limit to reasonable size */
    size_t len = (size_t)((*val_ptr < 0 ? -*val_ptr : *val_ptr) % 100);
    
    /* Test slice from array */
    int arr[100];
    for (size_t i = 0; i < len; i++) {
        arr[i] = (int)i;
    }
    
    Slice_int s = slice_int_from_array(arr, len);

    if (slice_int_len(s) != len) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test slice from vector */
    Vec_int v = vec_int_new();
    for (size_t i = 0; i < len; i++) {
        vec_int_push(&v, (int)i);
    }
    
    Slice_int sv = slice_int_from_vec(&v);
    if (slice_int_len(sv) != len) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify vector slice length matches vector length */
    if (slice_int_len(sv) != vec_int_len(&v)) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }
    
    vec_int_free(&v);
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
} SliceTest;

/*============================================================================
 * Property 16: Slice macro == function behavioral equivalence
 * For any Slice_T instance, any valid index, and any valid subslice range,
 * the type-first SLICE_* macros produce identical results to calling the
 * standalone slice_T_* functions.
 *============================================================================*/

static enum theft_trial_res prop_slice_macro_fn_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int base_val = (int)(*val_ptr);

    /* Create an array with known values */
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = base_val + i * 10;
    }

    Slice_int s = slice_int_from_array(arr, 10);

    /* Test len equivalence: macro vs standalone */
    if (SLICE_LEN(int, s) != slice_int_len(s)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test get equivalence for all valid indices */
    for (size_t i = 0; i < slice_int_len(s); i++) {
        Option_int opt_macro = SLICE_GET(int, s, i);
        Option_int opt_fn = slice_int_get(s, i);

        if (is_some(opt_macro) != is_some(opt_fn)) {
            return THEFT_TRIAL_FAIL;
        }

        if (is_some(opt_macro) && unwrap(opt_macro) != unwrap(opt_fn)) {
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Test get out-of-bounds equivalence */
    Option_int oob_macro = SLICE_GET(int, s, slice_int_len(s) + 10);
    Option_int oob_fn = slice_int_get(s, slice_int_len(s) + 10);

    if (is_none(oob_macro) != is_none(oob_fn)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test subslice equivalence */
    Slice_int sub_macro = SLICE_SUBSLICE(int, s, 2, 7);
    Slice_int sub_fn = slice_int_subslice(s, 2, 7);

    /* Verify subslice lengths are equal */
    if (SLICE_LEN(int, sub_macro) != slice_int_len(sub_fn)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Verify subslice elements are equal */
    for (size_t i = 0; i < slice_int_len(sub_fn); i++) {
        Option_int elem_macro = SLICE_GET(int, sub_macro, i);
        Option_int elem_fn = slice_int_get(sub_fn, i);

        if (is_some(elem_macro) != is_some(elem_fn)) {
            return THEFT_TRIAL_FAIL;
        }

        if (is_some(elem_macro) && unwrap(elem_macro) != unwrap(elem_fn)) {
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Test subslice with clamped bounds (start > end) */
    Slice_int clamped_macro = SLICE_SUBSLICE(int, s, 5, 3);
    Slice_int clamped_fn = slice_int_subslice(s, 5, 3);

    if (SLICE_LEN(int, clamped_macro) != slice_int_len(clamped_fn)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test subslice with bounds beyond length */
    Slice_int beyond_macro = SLICE_SUBSLICE(int, s, 8, 15);
    Slice_int beyond_fn = slice_int_subslice(s, 8, 15);

    if (SLICE_LEN(int, beyond_macro) != slice_int_len(beyond_fn)) {
        return THEFT_TRIAL_FAIL;
    }

    return THEFT_TRIAL_PASS;
}

static SliceTest slice_tests[] = {
    {
        "Property 13: slice get returns correct Option based on bounds",
        prop_slice_get_bounds,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 14: subslice elements match original",
        prop_subslice_correctness,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 15: slice length matches source",
        prop_slice_length,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 16: Slice macro == function behavioral equivalence",
        prop_slice_macro_fn_equivalence,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_SLICE_TESTS (sizeof(slice_tests) / sizeof(slice_tests[0]))

int run_slice_tests(theft_seed seed, int *num_tests) {
    int failures = 0;
    
    *num_tests = (int)NUM_SLICE_TESTS;
    
    printf("\nSlice Type Tests:\n");
    
    for (size_t i = 0; i < NUM_SLICE_TESTS; i++) {
        SliceTest *test = &slice_tests[i];
        
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

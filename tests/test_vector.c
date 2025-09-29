/**
 * @file test_vector.c
 * @brief Property-based tests for Vector type
 * 
 * Tests validate correctness properties:
 * - Property 9: push increases length and element is retrievable
 * - Property 10: pop returns last element and decreases length
 * - Property 11: get returns None for out-of-bounds indices
 * - Property 12: vector length equals pushes minus pops
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theft.h"
#include <cyan/option.h>
#include <cyan/vector.h>

/* Define Option and Vector types for testing */
OPTION_DEFINE(int);
VECTOR_DEFINE(int);

/*============================================================================
 * Property 9: push increases length and element is retrievable
 * For any vector and element, after push, length increases by 1 and get(len-1) returns Some with the pushed element
 *============================================================================*/

static enum theft_trial_res prop_push_and_get(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    Vec_int v = vec_int_new();
    size_t initial_len = vec_int_len(&v);
    
    /* Push the element */
    vec_int_push(&v, val);
    
    /* Length should increase by 1 */
    if (vec_int_len(&v) != initial_len + 1) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Element should be retrievable at len-1 */
    Option_int opt = vec_int_get(&v, vec_int_len(&v) - 1);
    if (!is_some(opt)) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Retrieved value should match pushed value */
    if (unwrap(opt) != val) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }
    
    vec_int_free(&v);
    return THEFT_TRIAL_PASS;
}


/*============================================================================
 * Property 10: pop returns last element and decreases length
 * For any non-empty vector, pop returns Some with last element and decreases length by 1.
 * For empty vectors, pop returns None.
 *============================================================================*/

static enum theft_trial_res prop_pop(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    /* Test pop on empty vector returns None */
    Vec_int empty_v = vec_int_new();
    Option_int empty_pop = vec_int_pop(&empty_v);
    if (!is_none(empty_pop)) {
        vec_int_free(&empty_v);
        return THEFT_TRIAL_FAIL;
    }
    vec_int_free(&empty_v);
    
    /* Test pop on non-empty vector */
    Vec_int v = vec_int_new();
    vec_int_push(&v, val);
    vec_int_push(&v, val + 1);
    vec_int_push(&v, val + 2);
    
    size_t len_before = vec_int_len(&v);
    int last_val = val + 2;
    
    Option_int popped = vec_int_pop(&v);
    
    /* Pop should return Some */
    if (!is_some(popped)) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Popped value should be the last element */
    if (unwrap(popped) != last_val) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Length should decrease by 1 */
    if (vec_int_len(&v) != len_before - 1) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }
    
    vec_int_free(&v);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 11: get returns None for out-of-bounds indices
 * For any vector and index >= length, get(index) returns None.
 *============================================================================*/

static enum theft_trial_res prop_out_of_bounds(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    Vec_int v = vec_int_new();
    
    /* Push some elements */
    vec_int_push(&v, val);
    vec_int_push(&v, val + 1);
    
    size_t len = vec_int_len(&v);
    
    /* Access at index == len should return None */
    Option_int at_len = vec_int_get(&v, len);
    if (!is_none(at_len)) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Access at index > len should return None */
    Option_int beyond_len = vec_int_get(&v, len + 100);
    if (!is_none(beyond_len)) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Access on empty vector should return None */
    Vec_int empty_v = vec_int_new();
    Option_int empty_get = vec_int_get(&empty_v, 0);
    if (!is_none(empty_get)) {
        vec_int_free(&v);
        vec_int_free(&empty_v);
        return THEFT_TRIAL_FAIL;
    }
    vec_int_free(&empty_v);
    
    vec_int_free(&v);
    return THEFT_TRIAL_PASS;
}


/*============================================================================
 * Property 12: vector length equals pushes minus pops
 * For any sequence of push and pop operations, length equals successful pushes minus successful pops.
 *============================================================================*/

static enum theft_trial_res prop_length_tracking(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    /* Use the value to determine number of operations */
    unsigned int seed = (unsigned int)((*val_ptr) & 0xFFFFFFFF);
    
    Vec_int v = vec_int_new();
    size_t expected_len = 0;
    
    /* Perform a sequence of push and pop operations */
    for (int i = 0; i < 50; i++) {
        /* Use seed to decide operation: push or pop */
        if ((seed + i) % 3 != 0) {
            /* Push operation */
            vec_int_push(&v, i);
            expected_len++;
        } else {
            /* Pop operation */
            Option_int popped = vec_int_pop(&v);
            if (is_some(popped)) {
                expected_len--;
            }
            /* If None, expected_len stays the same (pop on empty) */
        }
        
        /* Verify length after each operation */
        if (vec_int_len(&v) != expected_len) {
            vec_int_free(&v);
            return THEFT_TRIAL_FAIL;
        }
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
} VectorTest;

static VectorTest vector_tests[] = {
    {
        "Property 9: push increases length and element is retrievable",
        prop_push_and_get,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 10: pop returns last element and decreases length",
        prop_pop,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 11: get returns None for out-of-bounds indices",
        prop_out_of_bounds,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 12: vector length equals pushes minus pops",
        prop_length_tracking,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_VECTOR_TESTS (sizeof(vector_tests) / sizeof(vector_tests[0]))

int run_vector_tests(theft_seed seed) {
    int failures = 0;
    
    printf("\nVector Type Tests:\n");
    
    for (size_t i = 0; i < NUM_VECTOR_TESTS; i++) {
        VectorTest *test = &vector_tests[i];
        
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

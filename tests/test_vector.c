/**
 * @file test_vector.c
 * @brief Property-based tests for Vector type
 * 
 * Tests validate correctness properties:
 * - Property 9: push increases length and element is retrievable
 * - Property 10: pop returns last element and decreases length
 * - Property 11: get returns None for out-of-bounds indices
 * - Property 12: vector length equals pushes minus pops
 * - Property 13: insert/remove invariants
 * - Property 14: extend equals repeated push
 * - Property 15: reserve never loses elements
 * - Property 16: clear resets length, vector remains usable
 * - Property 17: Vector macro == function behavioral equivalence
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
 * Property 13: insert/remove invariants
 * Inserting at an index makes get(idx) return the element and shifts the
 * tail up; remove returns the element and shifts the tail back down.
 *============================================================================*/

static enum theft_trial_res prop_insert_remove(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr % 100000);

    Vec_int v = vec_int_new();

    /* Start with three known elements */
    vec_int_push(&v, val);
    vec_int_push(&v, val + 1);
    vec_int_push(&v, val + 2);

    /* Insert in the middle: [val, NEW, val+1, val+2] */
    int inserted = val + 100;
    vec_int_insert(&v, 1, inserted);

    if (vec_int_len(&v) != 4) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }

    /* get(idx) returns the inserted element */
    Option_int at_idx = vec_int_get(&v, 1);
    if (!is_some(at_idx) || unwrap(at_idx) != inserted) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }

    /* Tail elements shifted up by one */
    if (unwrap(vec_int_get(&v, 2)) != val + 1 || unwrap(vec_int_get(&v, 3)) != val + 2) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }

    /* Remove returns the element and shifts the tail back down */
    Option_int removed = vec_int_remove(&v, 1);
    if (!is_some(removed) || unwrap(removed) != inserted) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }

    if (vec_int_len(&v) != 3) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }

    if (unwrap(vec_int_get(&v, 0)) != val ||
        unwrap(vec_int_get(&v, 1)) != val + 1 ||
        unwrap(vec_int_get(&v, 2)) != val + 2) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }

    /* Remove out of bounds returns None and leaves the vector untouched */
    Option_int oob_remove = vec_int_remove(&v, vec_int_len(&v));
    if (!is_none(oob_remove) || vec_int_len(&v) != 3) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }

    vec_int_free(&v);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 14: extend equals repeated push
 * Extending a vector with n elements produces the same contents as pushing
 * the same n elements one at a time.
 *============================================================================*/

static enum theft_trial_res prop_extend_equals_push(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int base_val = (int)(*val_ptr % 100000);

    int src[20];
    size_t n = (size_t)(((*val_ptr < 0 ? -*val_ptr : *val_ptr) % 20) + 1);
    for (size_t i = 0; i < n; i++) {
        src[i] = base_val + (int)i;
    }

    Vec_int v_extend = vec_int_new();
    Vec_int v_push = vec_int_new();

    /* Seed both with one element to exercise appending onto content */
    vec_int_push(&v_extend, base_val - 1);
    vec_int_push(&v_push, base_val - 1);

    vec_int_extend(&v_extend, src, n);
    for (size_t i = 0; i < n; i++) {
        vec_int_push(&v_push, src[i]);
    }

    /* Lengths must match */
    if (vec_int_len(&v_extend) != vec_int_len(&v_push)) {
        vec_int_free(&v_extend);
        vec_int_free(&v_push);
        return THEFT_TRIAL_FAIL;
    }

    /* Contents must match element-wise */
    for (size_t i = 0; i < vec_int_len(&v_extend); i++) {
        if (unwrap(vec_int_get(&v_extend, i)) != unwrap(vec_int_get(&v_push, i))) {
            vec_int_free(&v_extend);
            vec_int_free(&v_push);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Extending with n == 0 is a no-op */
    size_t len_before = vec_int_len(&v_extend);
    vec_int_extend(&v_extend, NULL, 0);
    if (vec_int_len(&v_extend) != len_before) {
        vec_int_free(&v_extend);
        vec_int_free(&v_push);
        return THEFT_TRIAL_FAIL;
    }

    vec_int_free(&v_extend);
    vec_int_free(&v_push);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 15: reserve never loses elements
 * Reserving capacity (larger or smaller than current) preserves length and
 * all stored elements.
 *============================================================================*/

static enum theft_trial_res prop_reserve_preserves_elements(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int base_val = (int)(*val_ptr % 100000);

    Vec_int v = vec_int_new();
    for (int i = 0; i < 10; i++) {
        vec_int_push(&v, base_val + i);
    }

    /* Reserve well beyond the current capacity */
    vec_int_reserve(&v, 256);
    if (v.cap < 256 || vec_int_len(&v) != 10) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }
    for (int i = 0; i < 10; i++) {
        Option_int opt = vec_int_get(&v, (size_t)i);
        if (!is_some(opt) || unwrap(opt) != base_val + i) {
            vec_int_free(&v);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Reserving less than the current capacity never shrinks */
    size_t cap_before = v.cap;
    vec_int_reserve(&v, 1);
    if (v.cap != cap_before || vec_int_len(&v) != 10) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }
    for (int i = 0; i < 10; i++) {
        Option_int opt = vec_int_get(&v, (size_t)i);
        if (!is_some(opt) || unwrap(opt) != base_val + i) {
            vec_int_free(&v);
            return THEFT_TRIAL_FAIL;
        }
    }

    vec_int_free(&v);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 16: clear resets length, vector remains usable
 * After clear, length is 0 and get returns None; pushing again makes the
 * vector fully usable.
 *============================================================================*/

static enum theft_trial_res prop_clear_then_reuse(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr % 100000);

    Vec_int v = vec_int_new();
    vec_int_push(&v, val);
    vec_int_push(&v, val + 1);

    vec_int_clear(&v);

    /* Length is 0 and old elements are unreachable */
    if (vec_int_len(&v) != 0) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }
    if (!is_none(vec_int_get(&v, 0))) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }

    /* Re-push works after clear */
    vec_int_push(&v, val + 2);
    Option_int opt = vec_int_get(&v, 0);
    if (vec_int_len(&v) != 1 || !is_some(opt) || unwrap(opt) != val + 2) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }

    vec_int_free(&v);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 17: Vector macro == function behavioral equivalence
 * For any Vec_T instance, the type-first VEC_* macros produce identical
 * results to calling the standalone vec_T_* functions.
 *============================================================================*/

static enum theft_trial_res prop_macro_fn_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr % 100000);

    /* Create two identical vectors - one for macro ops, one for standalone ops */
    Vec_int v_macro = vec_int_new();
    Vec_int v_fn = vec_int_new();

    /* Test push equivalence: macro vs standalone */
    VEC_PUSH(int, v_macro, val);
    vec_int_push(&v_fn, val);

    VEC_PUSH(int, v_macro, val + 1);
    vec_int_push(&v_fn, val + 1);

    VEC_PUSH(int, v_macro, val + 2);
    vec_int_push(&v_fn, val + 2);

    /* Test len equivalence */
    if (VEC_LEN(int, v_macro) != vec_int_len(&v_fn)) {
        vec_int_free(&v_macro);
        vec_int_free(&v_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test insert equivalence */
    VEC_INSERT(int, v_macro, 1, val + 10);
    vec_int_insert(&v_fn, 1, val + 10);

    /* Test get equivalence for all indices */
    for (size_t i = 0; i < VEC_LEN(int, v_macro); i++) {
        Option_int opt_macro = VEC_GET(int, v_macro, i);
        Option_int opt_fn = vec_int_get(&v_fn, i);

        if (is_some(opt_macro) != is_some(opt_fn)) {
            vec_int_free(&v_macro);
            vec_int_free(&v_fn);
            return THEFT_TRIAL_FAIL;
        }

        if (is_some(opt_macro) && unwrap(opt_macro) != unwrap(opt_fn)) {
            vec_int_free(&v_macro);
            vec_int_free(&v_fn);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Test remove equivalence */
    Option_int rem_macro = VEC_REMOVE(int, v_macro, 1);
    Option_int rem_fn = vec_int_remove(&v_fn, 1);

    if (is_some(rem_macro) != is_some(rem_fn)) {
        vec_int_free(&v_macro);
        vec_int_free(&v_fn);
        return THEFT_TRIAL_FAIL;
    }

    if (is_some(rem_macro) && unwrap(rem_macro) != unwrap(rem_fn)) {
        vec_int_free(&v_macro);
        vec_int_free(&v_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test extend equivalence */
    int extra[3] = { val + 20, val + 21, val + 22 };
    VEC_EXTEND(int, v_macro, extra, 3);
    vec_int_extend(&v_fn, extra, 3);

    if (VEC_LEN(int, v_macro) != vec_int_len(&v_fn)) {
        vec_int_free(&v_macro);
        vec_int_free(&v_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test reserve equivalence */
    VEC_RESERVE(int, v_macro, 64);
    vec_int_reserve(&v_fn, 64);

    if (v_macro.cap != v_fn.cap) {
        vec_int_free(&v_macro);
        vec_int_free(&v_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test pop equivalence */
    Option_int pop_macro = VEC_POP(int, v_macro);
    Option_int pop_fn = vec_int_pop(&v_fn);

    if (is_some(pop_macro) != is_some(pop_fn)) {
        vec_int_free(&v_macro);
        vec_int_free(&v_fn);
        return THEFT_TRIAL_FAIL;
    }

    if (is_some(pop_macro) && unwrap(pop_macro) != unwrap(pop_fn)) {
        vec_int_free(&v_macro);
        vec_int_free(&v_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test clear equivalence */
    VEC_CLEAR(int, v_macro);
    vec_int_clear(&v_fn);

    if (VEC_LEN(int, v_macro) != vec_int_len(&v_fn) || VEC_LEN(int, v_macro) != 0) {
        vec_int_free(&v_macro);
        vec_int_free(&v_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Cleanup using VEC_FREE for macro, standalone for other */
    VEC_FREE(int, v_macro);
    vec_int_free(&v_fn);

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
    {
        "Property 13: insert/remove invariants",
        prop_insert_remove,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 14: extend equals repeated push",
        prop_extend_equals_push,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 15: reserve never loses elements",
        prop_reserve_preserves_elements,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 16: clear resets length, vector remains usable",
        prop_clear_then_reuse,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 17: Vector macro == function behavioral equivalence",
        prop_macro_fn_equivalence,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_VECTOR_TESTS (sizeof(vector_tests) / sizeof(vector_tests[0]))

int run_vector_tests(theft_seed seed, int *num_tests) {
    int failures = 0;
    
    *num_tests = (int)NUM_VECTOR_TESTS;
    
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

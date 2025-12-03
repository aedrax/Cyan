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
    
    /* Verify vt pointer is set after creation */
    if (v.vt == NULL) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }
    
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
    
    /* Verify vt pointer is set after creation */
    if (empty_v.vt == NULL) {
        vec_int_free(&empty_v);
        return THEFT_TRIAL_FAIL;
    }
    
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
    
    /* Verify vt pointer is set after creation */
    if (v.vt == NULL) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }
    
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
    
    /* Verify vt pointer is set after creation */
    if (v.vt == NULL) {
        vec_int_free(&v);
        return THEFT_TRIAL_FAIL;
    }
    
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
 * Property 1: Shared vtable instances (Vector)
 * For any two instances of Vec_T, their vtable pointers shall be equal
 * (point to the same address).
 *============================================================================*/

static enum theft_trial_res prop_shared_vtable(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    /* Create multiple vector instances using different constructors */
    Vec_int v1 = vec_int_new();
    Vec_int v2 = vec_int_new();
    Vec_int v3 = vec_int_with_capacity(10);
    Vec_int v4 = vec_int_with_capacity((size_t)(val > 0 ? val % 100 : (-val) % 100 + 1));
    
    /* All vtable pointers should be non-null */
    if (v1.vt == NULL || v2.vt == NULL || v3.vt == NULL || v4.vt == NULL) {
        vec_int_free(&v1);
        vec_int_free(&v2);
        vec_int_free(&v3);
        vec_int_free(&v4);
        return THEFT_TRIAL_FAIL;
    }
    
    /* All vtable pointers should point to the same address */
    if (v1.vt != v2.vt || v2.vt != v3.vt || v3.vt != v4.vt) {
        vec_int_free(&v1);
        vec_int_free(&v2);
        vec_int_free(&v3);
        vec_int_free(&v4);
        return THEFT_TRIAL_FAIL;
    }
    
    vec_int_free(&v1);
    vec_int_free(&v2);
    vec_int_free(&v3);
    vec_int_free(&v4);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 2: Vector vtable behavioral equivalence
 * For any Vec_T instance, any valid element, and any valid index, calling
 * operations through the vtable (v.vt->push, v.vt->pop, v.vt->get) shall
 * produce identical results to calling the standalone functions.
 *============================================================================*/

static enum theft_trial_res prop_vtable_behavioral_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    /* Create two identical vectors - one for vtable ops, one for standalone ops */
    Vec_int v_vtable = vec_int_new();
    Vec_int v_standalone = vec_int_new();
    
    /* Test push equivalence: vtable vs standalone */
    v_vtable.vt->push(&v_vtable, val);
    vec_int_push(&v_standalone, val);
    
    v_vtable.vt->push(&v_vtable, val + 1);
    vec_int_push(&v_standalone, val + 1);
    
    v_vtable.vt->push(&v_vtable, val + 2);
    vec_int_push(&v_standalone, val + 2);
    
    /* Test len equivalence */
    size_t len_vtable = v_vtable.vt->len(&v_vtable);
    size_t len_standalone = vec_int_len(&v_standalone);
    
    if (len_vtable != len_standalone) {
        vec_int_free(&v_vtable);
        vec_int_free(&v_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test get equivalence for all indices */
    for (size_t i = 0; i < len_vtable; i++) {
        Option_int opt_vtable = v_vtable.vt->get(&v_vtable, i);
        Option_int opt_standalone = vec_int_get(&v_standalone, i);
        
        if (is_some(opt_vtable) != is_some(opt_standalone)) {
            vec_int_free(&v_vtable);
            vec_int_free(&v_standalone);
            return THEFT_TRIAL_FAIL;
        }
        
        if (is_some(opt_vtable) && unwrap(opt_vtable) != unwrap(opt_standalone)) {
            vec_int_free(&v_vtable);
            vec_int_free(&v_standalone);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test get out-of-bounds equivalence */
    Option_int oob_vtable = v_vtable.vt->get(&v_vtable, len_vtable + 10);
    Option_int oob_standalone = vec_int_get(&v_standalone, len_standalone + 10);
    
    if (is_none(oob_vtable) != is_none(oob_standalone)) {
        vec_int_free(&v_vtable);
        vec_int_free(&v_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test pop equivalence */
    Option_int pop_vtable = v_vtable.vt->pop(&v_vtable);
    Option_int pop_standalone = vec_int_pop(&v_standalone);
    
    if (is_some(pop_vtable) != is_some(pop_standalone)) {
        vec_int_free(&v_vtable);
        vec_int_free(&v_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    if (is_some(pop_vtable) && unwrap(pop_vtable) != unwrap(pop_standalone)) {
        vec_int_free(&v_vtable);
        vec_int_free(&v_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify lengths are still equal after pop */
    if (v_vtable.vt->len(&v_vtable) != vec_int_len(&v_standalone)) {
        vec_int_free(&v_vtable);
        vec_int_free(&v_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test pop on empty vector equivalence */
    Vec_int empty_vtable = vec_int_new();
    Vec_int empty_standalone = vec_int_new();
    
    Option_int empty_pop_vtable = empty_vtable.vt->pop(&empty_vtable);
    Option_int empty_pop_standalone = vec_int_pop(&empty_standalone);
    
    if (is_none(empty_pop_vtable) != is_none(empty_pop_standalone)) {
        vec_int_free(&v_vtable);
        vec_int_free(&v_standalone);
        vec_int_free(&empty_vtable);
        vec_int_free(&empty_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Cleanup */
    v_vtable.vt->free(&v_vtable);
    vec_int_free(&v_standalone);
    vec_int_free(&empty_vtable);
    vec_int_free(&empty_standalone);
    
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
        "Property 1 (vtable): Shared vtable instances (Vector)",
        prop_shared_vtable,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 2 (vtable): Vector vtable behavioral equivalence",
        prop_vtable_behavioral_equivalence,
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

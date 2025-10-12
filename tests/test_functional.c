/**
 * @file test_functional.c
 * @brief Property-based tests for functional primitives
 * 
 * Tests validate correctness properties:
 * - Property 16: map preserves length and applies function
 * - Property 17: filter preserves predicate satisfaction
 * - Property 18: reduce equivalence to sequential fold
 * - Property 19: foreach visits each element exactly once in order
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theft.h"
#include <cyan/option.h>
#include <cyan/vector.h>
#include <cyan/functional.h>

/* Define types for testing */
OPTION_DEFINE(int);
VECTOR_DEFINE(int);

/*============================================================================
 * Test Helper Functions
 *============================================================================*/

/* Transformation function for map tests */
static int double_it(int x) {
    return x * 2;
}

/* Predicate function for filter tests */
static bool is_positive(int x) {
    return x > 0;
}

/* Accumulator function for reduce tests */
static int add(int a, int b) {
    return a + b;
}

/* Counter for foreach tests */
static int g_foreach_count;
static int g_foreach_values[256];

static void record_value(int x) {
    if (g_foreach_count < 256) {
        g_foreach_values[g_foreach_count++] = x;
    }
}

/*============================================================================
 * Custom Generator for Array of Integers
 *============================================================================*/

#define MAX_TEST_ARRAY_LEN 64

typedef struct {
    size_t len;
    int data[MAX_TEST_ARRAY_LEN];
} TestArray;

static enum theft_alloc_res alloc_test_array(
    struct theft *t,
    void *env,
    void **output
) {
    (void)env;
    
    TestArray *arr = malloc(sizeof(TestArray));
    if (!arr) return THEFT_ALLOC_ERROR;
    
    /* Generate random length between 0 and MAX_TEST_ARRAY_LEN */
    arr->len = theft_random_choice(t, MAX_TEST_ARRAY_LEN + 1);
    
    /* Generate random values */
    for (size_t i = 0; i < arr->len; i++) {
        /* Generate values in range [-1000, 1000] to have mix of positive/negative */
        arr->data[i] = (int)(theft_random_choice(t, 2001)) - 1000;
    }
    
    *output = arr;
    return THEFT_ALLOC_OK;
}

static void free_test_array(void *instance, void *env) {
    (void)env;
    free(instance);
}

static void print_test_array(FILE *f, const void *instance, void *env) {
    (void)env;
    const TestArray *arr = instance;
    fprintf(f, "[len=%zu: ", arr->len);
    for (size_t i = 0; i < arr->len && i < 10; i++) {
        fprintf(f, "%d%s", arr->data[i], i < arr->len - 1 ? ", " : "");
    }
    if (arr->len > 10) fprintf(f, "...");
    fprintf(f, "]");
}

static struct theft_type_info test_array_type_info = {
    .alloc = alloc_test_array,
    .free = free_test_array,
    .print = print_test_array,
};

/*============================================================================
 * Property 16: map preserves length and applies function
 * For any array of length n and transformation function f, map produces
 * an array of length n where element i equals f(original[i]).
 *============================================================================*/

static enum theft_trial_res prop_map_preserves_length_and_applies(
    struct theft *t,
    void *arg1
) {
    (void)t;
    TestArray *arr = (TestArray *)arg1;
    
    if (arr->len == 0) {
        /* Empty array case - map should produce empty output */
        return THEFT_TRIAL_PASS;
    }
    
    int *out = malloc(arr->len * sizeof(int));
    if (!out) return THEFT_TRIAL_ERROR;
    
    /* Apply map */
    map(arr->data, arr->len, out, double_it);
    
    /* Verify each element is transformed correctly */
    for (size_t i = 0; i < arr->len; i++) {
        int expected = double_it(arr->data[i]);
        if (out[i] != expected) {
            free(out);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    free(out);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 17: filter preserves predicate satisfaction

 * All elements in the filtered result satisfy the predicate, and the result
 * contains all elements from the original that satisfy the predicate.
 *============================================================================*/

static enum theft_trial_res prop_filter_preserves_predicate(
    struct theft *t,
    void *arg1
) {
    (void)t;
    TestArray *arr = (TestArray *)arg1;
    
    int *out = malloc((arr->len > 0 ? arr->len : 1) * sizeof(int));
    if (!out) return THEFT_TRIAL_ERROR;
    
    size_t out_len = 0;
    
    /* Apply filter */
    filter(arr->data, arr->len, out, &out_len, is_positive);
    
    /* Count how many elements should pass the predicate */
    size_t expected_count = 0;
    for (size_t i = 0; i < arr->len; i++) {
        if (is_positive(arr->data[i])) {
            expected_count++;
        }
    }
    
    /* Verify output length matches expected count */
    if (out_len != expected_count) {
        free(out);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify all output elements satisfy the predicate */
    for (size_t i = 0; i < out_len; i++) {
        if (!is_positive(out[i])) {
            free(out);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Verify output contains exactly the elements that satisfy predicate, in order */
    size_t out_idx = 0;
    for (size_t i = 0; i < arr->len; i++) {
        if (is_positive(arr->data[i])) {
            if (out_idx >= out_len || out[out_idx] != arr->data[i]) {
                free(out);
                return THEFT_TRIAL_FAIL;
            }
            out_idx++;
        }
    }
    
    free(out);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 18: reduce equivalence to sequential fold
 * For any array, initial value, and accumulator function, reduce produces
 * the same result as sequentially applying the accumulator from left to right.
 *============================================================================*/

static enum theft_trial_res prop_reduce_sequential_fold(
    struct theft *t,
    void *arg1
) {
    (void)t;
    TestArray *arr = (TestArray *)arg1;
    
    int init = 0;
    
    /* Compute expected result via manual sequential fold */
    int expected = init;
    for (size_t i = 0; i < arr->len; i++) {
        expected = add(expected, arr->data[i]);
    }
    
    /* Apply reduce */
    int result;
    reduce(result, arr->data, arr->len, init, add);
    
    /* Verify result matches expected */
    if (result != expected) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 19: foreach visits each element exactly once in order
 * For any array of length n, foreach invokes the callback exactly n times,
 * once for each element in index order.
 *============================================================================*/

static enum theft_trial_res prop_foreach_visits_each_once_in_order(
    struct theft *t,
    void *arg1
) {
    (void)t;
    TestArray *arr = (TestArray *)arg1;
    
    /* Reset global counter */
    g_foreach_count = 0;
    memset(g_foreach_values, 0, sizeof(g_foreach_values));
    
    /* Apply foreach */
    foreach(arr->data, arr->len, record_value);
    
    /* Verify callback was called exactly len times */
    if ((size_t)g_foreach_count != arr->len) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify elements were visited in order */
    for (size_t i = 0; i < arr->len; i++) {
        if (g_foreach_values[i] != arr->data[i]) {
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
} FunctionalTest;

static FunctionalTest functional_tests[] = {
    {
        "Property 16: map preserves length and applies function",
        prop_map_preserves_length_and_applies
    },
    {
        "Property 17: filter preserves predicate satisfaction",
        prop_filter_preserves_predicate
    },
    {
        "Property 18: reduce equivalence to sequential fold",
        prop_reduce_sequential_fold
    },
    {
        "Property 19: foreach visits each element exactly once in order",
        prop_foreach_visits_each_once_in_order
    },
};

#define NUM_FUNCTIONAL_TESTS (sizeof(functional_tests) / sizeof(functional_tests[0]))

int run_functional_tests(theft_seed seed) {
    int failures = 0;
    
    printf("\nFunctional Primitives Tests:\n");
    
    for (size_t i = 0; i < NUM_FUNCTIONAL_TESTS; i++) {
        FunctionalTest *test = &functional_tests[i];
        
        struct theft_run_config config = {
            .name = test->name,
            .prop1 = test->prop,
            .type_info = { &test_array_type_info },
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

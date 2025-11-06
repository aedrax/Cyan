/**
 * @file test_smartptr.c
 * @brief Property-based tests for Smart Pointer types
 * 
 * Tests validate correctness properties:
 * - Property 31: Unique pointer round-trip
 * - Property 32: Unique pointer move nullifies source
 * - Property 33: Unique pointer cleanup on scope exit
 * - Property 34: Shared pointer round-trip
 * - Property 35: Shared pointer clone increments count
 * - Property 36: Shared pointer release decrements count
 * - Property 37: Shared pointer cleanup at zero
 * - Property 38: Weak pointer does not affect strong count
 * - Property 39: Weak pointer upgrade when valid
 * - Property 40: Weak pointer upgrade when expired
 * - Property 41: Weak pointer is_expired correctness
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theft.h"
#include <cyan/smartptr.h>

/* Define smart pointer types for testing */
UNIQUE_PTR_DEFINE(int);
SHARED_PTR_DEFINE(int);

/* Global counters for tracking destructor calls */
static int g_dtor_call_count = 0;
static int g_last_dtor_value = 0;

static void test_destructor(void *ptr) {
    g_dtor_call_count++;
    g_last_dtor_value = *(int *)ptr;
}

static void reset_dtor_counters(void) {
    g_dtor_call_count = 0;
    g_last_dtor_value = 0;
}

/*============================================================================
 * Property 31: Unique pointer round-trip
 * For any value, creating a unique pointer and dereferencing returns original value
 *============================================================================*/

static enum theft_trial_res prop_unique_roundtrip(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    UniquePtr_int p = unique_int_new(val);
    
    /* Dereference must return the original value */
    int deref_val = unique_int_deref(&p);
    if (deref_val != val) {
        unique_int_free(&p);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Get must return a valid pointer to the value */
    int *raw_ptr = unique_int_get(&p);
    if (raw_ptr == NULL || *raw_ptr != val) {
        unique_int_free(&p);
        return THEFT_TRIAL_FAIL;
    }
    
    unique_int_free(&p);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 32: Unique pointer move nullifies source
 * After moving, source is null and destination contains original value
 *============================================================================*/

static enum theft_trial_res prop_unique_move(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    UniquePtr_int src = unique_int_new(val);
    UniquePtr_int dst = unique_int_move(&src);
    
    /* Source must be nullified */
    if (unique_int_get(&src) != NULL) {
        unique_int_free(&dst);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Destination must contain the original value */
    int dst_val = unique_int_deref(&dst);
    if (dst_val != val) {
        unique_int_free(&dst);
        return THEFT_TRIAL_FAIL;
    }
    
    unique_int_free(&dst);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 33: Unique pointer cleanup on scope exit
 * Custom destructor is invoked exactly once when pointer goes out of scope
 *============================================================================*/

static enum theft_trial_res prop_unique_cleanup(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    reset_dtor_counters();
    
    /* Create unique pointer with custom destructor in a scope */
    {
        unique_ptr_with_dtor(int, p, val, test_destructor);
        /* Verify value is accessible */
        if (unique_int_deref(&p) != val) {
            return THEFT_TRIAL_FAIL;
        }
    }
    /* Scope exit - destructor should have been called */
    
    /* Destructor must be called exactly once */
    if (g_dtor_call_count != 1) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Destructor must have received the correct value */
    if (g_last_dtor_value != val) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 34: Shared pointer round-trip
 * Creating a shared pointer and dereferencing returns original value, count is 1
 *============================================================================*/

static enum theft_trial_res prop_shared_roundtrip(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    SharedPtr_int s = shared_int_new(val);
    
    /* Dereference must return the original value */
    int deref_val = shared_int_deref(&s);
    if (deref_val != val) {
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Reference count must be 1 */
    if (shared_int_count(&s) != 1) {
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    shared_int_release(&s);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 35: Shared pointer clone increments count
 * Cloning a shared pointer increments reference count for both pointers
 *============================================================================*/

static enum theft_trial_res prop_shared_clone(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    SharedPtr_int s1 = shared_int_new(val);
    size_t initial_count = shared_int_count(&s1);
    
    SharedPtr_int s2 = shared_int_clone(&s1);
    
    /* Both pointers must have count = initial + 1 */
    if (shared_int_count(&s1) != initial_count + 1) {
        shared_int_release(&s2);
        shared_int_release(&s1);
        return THEFT_TRIAL_FAIL;
    }
    
    if (shared_int_count(&s2) != initial_count + 1) {
        shared_int_release(&s2);
        shared_int_release(&s1);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Both must point to the same value */
    if (shared_int_deref(&s1) != val || shared_int_deref(&s2) != val) {
        shared_int_release(&s2);
        shared_int_release(&s1);
        return THEFT_TRIAL_FAIL;
    }
    
    shared_int_release(&s2);
    shared_int_release(&s1);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 36: Shared pointer release decrements count
 * Releasing a shared pointer decrements count for remaining references
 *============================================================================*/

static enum theft_trial_res prop_shared_release(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    SharedPtr_int s1 = shared_int_new(val);
    SharedPtr_int s2 = shared_int_clone(&s1);
    SharedPtr_int s3 = shared_int_clone(&s1);
    
    /* Count should be 3 */
    if (shared_int_count(&s1) != 3) {
        shared_int_release(&s3);
        shared_int_release(&s2);
        shared_int_release(&s1);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Release s3, count should be 2 */
    shared_int_release(&s3);
    if (shared_int_count(&s1) != 2) {
        shared_int_release(&s2);
        shared_int_release(&s1);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Release s2, count should be 1 */
    shared_int_release(&s2);
    if (shared_int_count(&s1) != 1) {
        shared_int_release(&s1);
        return THEFT_TRIAL_FAIL;
    }
    
    shared_int_release(&s1);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 37: Shared pointer cleanup at zero
 * Custom destructor is invoked exactly once when last reference is released
 *============================================================================*/

static enum theft_trial_res prop_shared_cleanup_zero(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    reset_dtor_counters();
    
    SharedPtr_int s1 = shared_int_new_with_dtor(val, test_destructor);
    SharedPtr_int s2 = shared_int_clone(&s1);
    
    /* Release first reference - destructor should NOT be called yet */
    shared_int_release(&s1);
    if (g_dtor_call_count != 0) {
        shared_int_release(&s2);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Release last reference - destructor should be called */
    shared_int_release(&s2);
    if (g_dtor_call_count != 1) {
        return THEFT_TRIAL_FAIL;
    }
    
    if (g_last_dtor_value != val) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 38: Weak pointer does not affect strong count
 * Creating a weak pointer leaves strong count unchanged
 *============================================================================*/

static enum theft_trial_res prop_weak_no_strong_count(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    SharedPtr_int s = shared_int_new(val);
    size_t count_before = shared_int_count(&s);
    
    WeakPtr_int w = weak_int_from_shared(&s);
    
    /* Strong count must remain unchanged */
    if (shared_int_count(&s) != count_before) {
        weak_int_release(&w);
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    weak_int_release(&w);
    shared_int_release(&s);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 39: Weak pointer upgrade when valid
 * Upgrading a valid weak pointer returns Some with incremented count
 *============================================================================*/

static enum theft_trial_res prop_weak_upgrade_valid(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    SharedPtr_int s = shared_int_new(val);
    WeakPtr_int w = weak_int_from_shared(&s);
    
    size_t count_before = shared_int_count(&s);
    
    /* Upgrade should succeed */
    Option_SharedPtr_int upgraded = weak_int_upgrade(&w);
    if (!upgraded.has_value) {
        weak_int_release(&w);
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Count should be incremented */
    if (shared_int_count(&s) != count_before + 1) {
        shared_int_release(&upgraded.value);
        weak_int_release(&w);
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Upgraded pointer should have same value */
    if (shared_int_deref(&upgraded.value) != val) {
        shared_int_release(&upgraded.value);
        weak_int_release(&w);
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    shared_int_release(&upgraded.value);
    weak_int_release(&w);
    shared_int_release(&s);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 40: Weak pointer upgrade when expired
 * Upgrading an expired weak pointer returns None
 *============================================================================*/

static enum theft_trial_res prop_weak_upgrade_expired(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    SharedPtr_int s = shared_int_new(val);
    WeakPtr_int w = weak_int_from_shared(&s);
    
    /* Release the shared pointer - weak pointer becomes expired */
    shared_int_release(&s);
    
    /* Upgrade should fail */
    Option_SharedPtr_int upgraded = weak_int_upgrade(&w);
    if (upgraded.has_value) {
        shared_int_release(&upgraded.value);
        weak_int_release(&w);
        return THEFT_TRIAL_FAIL;
    }
    
    weak_int_release(&w);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 41: Weak pointer is_expired correctness
 * is_expired returns true iff strong count is zero
 *============================================================================*/

static enum theft_trial_res prop_weak_is_expired(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    SharedPtr_int s = shared_int_new(val);
    WeakPtr_int w = weak_int_from_shared(&s);
    
    /* While shared pointer exists, weak should not be expired */
    if (weak_int_is_expired(&w)) {
        weak_int_release(&w);
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Release shared pointer */
    shared_int_release(&s);
    
    /* Now weak should be expired */
    if (!weak_int_is_expired(&w)) {
        weak_int_release(&w);
        return THEFT_TRIAL_FAIL;
    }
    
    weak_int_release(&w);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Test Registration
 *============================================================================*/

#define MIN_TEST_TRIALS 100

typedef struct {
    const char *name;
    theft_propfun1 *prop;
    enum theft_builtin_type_info type;
} SmartPtrTest;

static SmartPtrTest smartptr_tests[] = {
    {
        "Property 31: Unique pointer round-trip",
        prop_unique_roundtrip,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 32: Unique pointer move nullifies source",
        prop_unique_move,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 33: Unique pointer cleanup on scope exit",
        prop_unique_cleanup,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 34: Shared pointer round-trip",
        prop_shared_roundtrip,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 35: Shared pointer clone increments count",
        prop_shared_clone,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 36: Shared pointer release decrements count",
        prop_shared_release,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 37: Shared pointer cleanup at zero",
        prop_shared_cleanup_zero,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 38: Weak pointer does not affect strong count",
        prop_weak_no_strong_count,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 39: Weak pointer upgrade when valid",
        prop_weak_upgrade_valid,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 40: Weak pointer upgrade when expired",
        prop_weak_upgrade_expired,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 41: Weak pointer is_expired correctness",
        prop_weak_is_expired,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_SMARTPTR_TESTS (sizeof(smartptr_tests) / sizeof(smartptr_tests[0]))

int run_smartptr_tests(theft_seed seed) {
    int failures = 0;
    
    printf("\nSmart Pointer Tests:\n");
    
    for (size_t i = 0; i < NUM_SMARTPTR_TESTS; i++) {
        SmartPtrTest *test = &smartptr_tests[i];
        
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

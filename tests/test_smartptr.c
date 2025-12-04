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
 * Property 1 (vtable): Shared vtable instances (UniquePtr)
 * For any two UniquePtr_T instances, their vtable pointers shall be equal
 *============================================================================*/

static enum theft_trial_res prop_unique_shared_vtable(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    UniquePtr_int p1 = unique_int_new(val);
    UniquePtr_int p2 = unique_int_new(val + 1);
    
    /* All vtable pointers should be equal (shared) */
    if (p1.vt != p2.vt) {
        unique_int_free(&p1);
        unique_int_free(&p2);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Vtable should not be NULL */
    if (p1.vt == NULL) {
        unique_int_free(&p1);
        unique_int_free(&p2);
        return THEFT_TRIAL_FAIL;
    }
    
    unique_int_free(&p1);
    unique_int_free(&p2);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 9 (vtable): UniquePtr vtable behavioral equivalence
 * For any UniquePtr_T instance, vtable operations produce identical results
 *============================================================================*/

static enum theft_trial_res prop_unique_vtable_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    UniquePtr_int p = unique_int_new(val);
    
    /* get equivalence */
    if (unique_int_get(&p) != UPTR_GET(p)) {
        unique_int_free(&p);
        return THEFT_TRIAL_FAIL;
    }
    if (unique_int_get(&p) != p.vt->uptr_get(&p)) {
        unique_int_free(&p);
        return THEFT_TRIAL_FAIL;
    }
    
    /* deref equivalence */
    if (unique_int_deref(&p) != UPTR_DEREF(p)) {
        unique_int_free(&p);
        return THEFT_TRIAL_FAIL;
    }
    if (unique_int_deref(&p) != p.vt->uptr_deref(&p)) {
        unique_int_free(&p);
        return THEFT_TRIAL_FAIL;
    }
    
    unique_int_free(&p);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 1 (vtable): Shared vtable instances (SharedPtr)
 * For any two SharedPtr_T instances, their vtable pointers shall be equal
 *============================================================================*/

static enum theft_trial_res prop_shared_shared_vtable(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    SharedPtr_int s1 = shared_int_new(val);
    SharedPtr_int s2 = shared_int_new(val + 1);
    SharedPtr_int s3 = shared_int_clone(&s1);
    
    /* All vtable pointers should be equal (shared) */
    if (s1.vt != s2.vt) {
        shared_int_release(&s1);
        shared_int_release(&s2);
        shared_int_release(&s3);
        return THEFT_TRIAL_FAIL;
    }
    if (s1.vt != s3.vt) {
        shared_int_release(&s1);
        shared_int_release(&s2);
        shared_int_release(&s3);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Vtable should not be NULL */
    if (s1.vt == NULL) {
        shared_int_release(&s1);
        shared_int_release(&s2);
        shared_int_release(&s3);
        return THEFT_TRIAL_FAIL;
    }
    
    shared_int_release(&s1);
    shared_int_release(&s2);
    shared_int_release(&s3);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 10 (vtable): SharedPtr vtable behavioral equivalence
 * For any SharedPtr_T instance, vtable operations produce identical results
 *============================================================================*/

static enum theft_trial_res prop_shared_vtable_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    SharedPtr_int s = shared_int_new(val);
    
    /* get equivalence */
    if (shared_int_get(&s) != SPTR_GET(s)) {
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }
    if (shared_int_get(&s) != s.vt->sptr_get(&s)) {
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    /* deref equivalence */
    if (shared_int_deref(&s) != SPTR_DEREF(s)) {
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }
    if (shared_int_deref(&s) != s.vt->sptr_deref(&s)) {
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    /* count equivalence */
    if (shared_int_count(&s) != SPTR_COUNT(s)) {
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }
    if (shared_int_count(&s) != s.vt->sptr_count(&s)) {
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    shared_int_release(&s);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 1: Shared vtable instances (WeakPtr)
 * For any two WeakPtr_T instances of the same type, their vtable pointers shall
 * be equal (point to the same address), and the vtable pointer shall be non-null.
 *============================================================================*/

static enum theft_trial_res prop_weak_shared_vtable(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    SharedPtr_int s1 = shared_int_new(val);
    SharedPtr_int s2 = shared_int_new(val + 1);
    
    WeakPtr_int w1 = weak_int_from_shared(&s1);
    WeakPtr_int w2 = weak_int_from_shared(&s1);
    WeakPtr_int w3 = weak_int_from_shared(&s2);
    
    /* All vtable pointers should be equal (shared) */
    if (w1.vt != w2.vt) {
        weak_int_release(&w1);
        weak_int_release(&w2);
        weak_int_release(&w3);
        shared_int_release(&s1);
        shared_int_release(&s2);
        return THEFT_TRIAL_FAIL;
    }
    if (w1.vt != w3.vt) {
        weak_int_release(&w1);
        weak_int_release(&w2);
        weak_int_release(&w3);
        shared_int_release(&s1);
        shared_int_release(&s2);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Vtable should not be NULL */
    if (w1.vt == NULL) {
        weak_int_release(&w1);
        weak_int_release(&w2);
        weak_int_release(&w3);
        shared_int_release(&s1);
        shared_int_release(&s2);
        return THEFT_TRIAL_FAIL;
    }
    
    weak_int_release(&w1);
    weak_int_release(&w2);
    weak_int_release(&w3);
    shared_int_release(&s1);
    shared_int_release(&s2);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 2: WeakPtr vtable behavioral equivalence
 * For any WeakPtr_T instance, calling operations through the vtable
 * (w.vt->wptr_is_expired, w.vt->wptr_upgrade) shall produce identical results
 * to calling the standalone functions (weak_T_is_expired, weak_T_upgrade).
 *============================================================================*/

static enum theft_trial_res prop_weak_vtable_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    /* Test with valid (non-expired) weak pointer */
    SharedPtr_int s = shared_int_new(val);
    WeakPtr_int w = weak_int_from_shared(&s);
    
    /* is_expired equivalence (valid case) */
    bool standalone_expired = weak_int_is_expired(&w);
    bool vtable_expired = w.vt->wptr_is_expired(&w);
    if (standalone_expired != vtable_expired) {
        weak_int_release(&w);
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    /* upgrade equivalence (valid case) */
    Option_SharedPtr_int standalone_upgrade = weak_int_upgrade(&w);
    Option_SharedPtr_int vtable_upgrade = w.vt->wptr_upgrade(&w);
    
    if (standalone_upgrade.has_value != vtable_upgrade.has_value) {
        if (standalone_upgrade.has_value) shared_int_release(&standalone_upgrade.value);
        if (vtable_upgrade.has_value) shared_int_release(&vtable_upgrade.value);
        weak_int_release(&w);
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    if (standalone_upgrade.has_value) {
        /* Both should have same value */
        if (shared_int_deref(&standalone_upgrade.value) != shared_int_deref(&vtable_upgrade.value)) {
            shared_int_release(&standalone_upgrade.value);
            shared_int_release(&vtable_upgrade.value);
            weak_int_release(&w);
            shared_int_release(&s);
            return THEFT_TRIAL_FAIL;
        }
        shared_int_release(&standalone_upgrade.value);
        shared_int_release(&vtable_upgrade.value);
    }
    
    /* Release shared pointer to make weak pointer expired */
    weak_int_release(&w);
    shared_int_release(&s);
    
    /* Test with expired weak pointer */
    SharedPtr_int s2 = shared_int_new(val);
    WeakPtr_int w2 = weak_int_from_shared(&s2);
    shared_int_release(&s2);  /* Now w2 is expired */
    
    /* is_expired equivalence (expired case) */
    standalone_expired = weak_int_is_expired(&w2);
    vtable_expired = w2.vt->wptr_is_expired(&w2);
    if (standalone_expired != vtable_expired) {
        weak_int_release(&w2);
        return THEFT_TRIAL_FAIL;
    }
    
    /* upgrade equivalence (expired case) */
    standalone_upgrade = weak_int_upgrade(&w2);
    vtable_upgrade = w2.vt->wptr_upgrade(&w2);
    
    if (standalone_upgrade.has_value != vtable_upgrade.has_value) {
        if (standalone_upgrade.has_value) shared_int_release(&standalone_upgrade.value);
        if (vtable_upgrade.has_value) shared_int_release(&vtable_upgrade.value);
        weak_int_release(&w2);
        return THEFT_TRIAL_FAIL;
    }
    
    weak_int_release(&w2);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 3: WeakPtr convenience macro equivalence
 * For any WeakPtr_T instance, calling convenience macros (WPTR_IS_EXPIRED, WPTR_UPGRADE)
 * shall produce identical results to calling the vtable function pointers directly.
 *============================================================================*/

static enum theft_trial_res prop_weak_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    /* Test with valid (non-expired) weak pointer */
    SharedPtr_int s = shared_int_new(val);
    WeakPtr_int w = weak_int_from_shared(&s);
    
    /* is_expired equivalence (valid case) */
    bool vtable_expired = w.vt->wptr_is_expired(&w);
    bool macro_expired = WPTR_IS_EXPIRED(w);
    if (vtable_expired != macro_expired) {
        weak_int_release(&w);
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    /* upgrade equivalence (valid case) */
    Option_SharedPtr_int vtable_upgrade = w.vt->wptr_upgrade(&w);
    Option_SharedPtr_int macro_upgrade = WPTR_UPGRADE(w);
    
    if (vtable_upgrade.has_value != macro_upgrade.has_value) {
        if (vtable_upgrade.has_value) shared_int_release(&vtable_upgrade.value);
        if (macro_upgrade.has_value) shared_int_release(&macro_upgrade.value);
        weak_int_release(&w);
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    if (vtable_upgrade.has_value) {
        /* Both should have same value */
        if (shared_int_deref(&vtable_upgrade.value) != shared_int_deref(&macro_upgrade.value)) {
            shared_int_release(&vtable_upgrade.value);
            shared_int_release(&macro_upgrade.value);
            weak_int_release(&w);
            shared_int_release(&s);
            return THEFT_TRIAL_FAIL;
        }
        shared_int_release(&vtable_upgrade.value);
        shared_int_release(&macro_upgrade.value);
    }
    
    /* Release shared pointer to make weak pointer expired */
    weak_int_release(&w);
    shared_int_release(&s);
    
    /* Test with expired weak pointer */
    SharedPtr_int s2 = shared_int_new(val);
    WeakPtr_int w2 = weak_int_from_shared(&s2);
    shared_int_release(&s2);  /* Now w2 is expired */
    
    /* is_expired equivalence (expired case) */
    vtable_expired = w2.vt->wptr_is_expired(&w2);
    macro_expired = WPTR_IS_EXPIRED(w2);
    if (vtable_expired != macro_expired) {
        weak_int_release(&w2);
        return THEFT_TRIAL_FAIL;
    }
    
    /* upgrade equivalence (expired case) */
    vtable_upgrade = w2.vt->wptr_upgrade(&w2);
    macro_upgrade = WPTR_UPGRADE(w2);
    
    if (vtable_upgrade.has_value != macro_upgrade.has_value) {
        if (vtable_upgrade.has_value) shared_int_release(&vtable_upgrade.value);
        if (macro_upgrade.has_value) shared_int_release(&macro_upgrade.value);
        weak_int_release(&w2);
        return THEFT_TRIAL_FAIL;
    }
    
    weak_int_release(&w2);
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
    {
        "Property 1 (vtable): Shared vtable instances (UniquePtr)",
        prop_unique_shared_vtable,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 9 (vtable): UniquePtr vtable behavioral equivalence",
        prop_unique_vtable_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 1 (vtable): Shared vtable instances (SharedPtr)",
        prop_shared_shared_vtable,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 10 (vtable): SharedPtr vtable behavioral equivalence",
        prop_shared_vtable_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 1 (vtable): Shared vtable instances (WeakPtr)",
        prop_weak_shared_vtable,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 2 (vtable): WeakPtr vtable behavioral equivalence",
        prop_weak_vtable_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 3 (vtable): WeakPtr convenience macro equivalence",
        prop_weak_macro_equivalence,
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

/**
 * @file test_hashset.c
 * @brief Property-based tests for HashSet type
 *
 * Tests validate correctness properties:
 * - Property 66: add/contains/remove agree with a naive reference array
 * - Property 67: duplicate add returns false and leaves len unchanged
 * - Property 68: growth keeps len exact and all elements contained
 * - Property 69: remove semantics and tombstone slot reuse
 * - Property 70: iteration yields exactly the live elements, each once
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theft.h"
#include <cyan/hashset.h>

/* Define the HashSet type and its iterator for testing */
HASHSET_DEFINE(int);
HASHSET_ITER_DEFINE(int);

/*============================================================================
 * Property 66: add/contains/remove agree with a naive reference array
 * Over a random sequence of add/contains/remove operations on a small key
 * domain, every result matches a naive boolean-array reference, and len
 * always equals the reference's live count.
 *============================================================================*/

static enum theft_trial_res prop_set_matches_reference(struct theft *t, void *arg1) {
    (void)arg1;

    enum { KEY_DOMAIN = 32 };

    HashSet_int s = hashset_int_new();
    bool ref[KEY_DOMAIN] = {false};
    size_t ref_len = 0;

    size_t num_ops = 20 + theft_random_choice(t, 80);
    for (size_t op = 0; op < num_ops; op++) {
        int key = (int)theft_random_choice(t, KEY_DOMAIN);
        switch (theft_random_choice(t, 3)) {
            case 0: {
                /* add: true iff not already present */
                bool added = hashset_int_add(&s, key);
                if (added == ref[key]) {
                    hashset_int_free(&s);
                    return THEFT_TRIAL_FAIL;
                }
                if (!ref[key]) {
                    ref[key] = true;
                    ref_len++;
                }
                break;
            }
            case 1: {
                /* contains: agrees with the reference */
                if (hashset_int_contains(&s, key) != ref[key]) {
                    hashset_int_free(&s);
                    return THEFT_TRIAL_FAIL;
                }
                break;
            }
            default: {
                /* remove: true iff present */
                bool removed = hashset_int_remove(&s, key);
                if (removed != ref[key]) {
                    hashset_int_free(&s);
                    return THEFT_TRIAL_FAIL;
                }
                if (ref[key]) {
                    ref[key] = false;
                    ref_len--;
                }
                break;
            }
        }

        /* len tracks the reference after every operation */
        if (hashset_int_len(&s) != ref_len) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Final membership sweep over the whole domain */
    for (int k = 0; k < KEY_DOMAIN; k++) {
        if (hashset_int_contains(&s, k) != ref[k]) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Macro forms agree with the functions */
    int probe = (int)theft_random_choice(t, KEY_DOMAIN);
    if (SET_CONTAINS(int, s, probe) != hashset_int_contains(&s, probe) ||
        SET_LEN(int, s) != hashset_int_len(&s)) {
        hashset_int_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    SET_FREE(int, s);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 67: duplicate add returns false and leaves len unchanged
 * Adding an element already in the set returns false, does not change len,
 * and the element remains contained.
 *============================================================================*/

static enum theft_trial_res prop_duplicate_add(struct theft *t, void *arg1) {
    int64_t *val_ptr = (int64_t *)arg1;
    int seed = (int)(*val_ptr % 100000);

    HashSet_int s = hashset_int_new();

    size_t n = 1 + theft_random_choice(t, 30);
    for (size_t i = 0; i < n; i++) {
        /* Distinct keys */
        if (!hashset_int_add(&s, seed + (int)i * 7)) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    }
    if (hashset_int_len(&s) != n) {
        hashset_int_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    /* Re-adding every element returns false and never changes len */
    for (size_t i = 0; i < n; i++) {
        int key = seed + (int)i * 7;
        if (hashset_int_add(&s, key)) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
        if (hashset_int_len(&s) != n || !hashset_int_contains(&s, key)) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* SET_ADD macro agrees on a duplicate */
    if (SET_ADD(int, s, seed) || SET_LEN(int, s) != n) {
        hashset_int_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    hashset_int_free(&s);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 68: growth keeps len exact and all elements contained
 * Adding 200 distinct ints (well past the initial capacity, forcing
 * multiple resizes) keeps len exact and every element contained; elements
 * never added are not contained.
 *============================================================================*/

static enum theft_trial_res prop_growth_preserves_elements(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int base = (int)(*val_ptr % 100000);

    enum { N = 200 };

    HashSet_int s = hashset_int_new();

    /* base + i*3 for i in [0, N) are pairwise distinct */
    for (int i = 0; i < N; i++) {
        if (!hashset_int_add(&s, base + i * 3)) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
        if (hashset_int_len(&s) != (size_t)(i + 1)) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* All elements survived the resizes */
    for (int i = 0; i < N; i++) {
        if (!hashset_int_contains(&s, base + i * 3)) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Values between the added ones are absent */
    for (int i = 0; i < N; i++) {
        if (hashset_int_contains(&s, base + i * 3 + 1)) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    }

    if (hashset_int_len(&s) != N) {
        hashset_int_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    hashset_int_free(&s);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 69: remove semantics and tombstone slot reuse
 * remove returns true only when the element is present; after remove,
 * contains is false; re-adding a removed element works (the tombstone is
 * reused) and duplicates of it are again rejected.
 *============================================================================*/

static enum theft_trial_res prop_remove_and_tombstone_reuse(struct theft *t, void *arg1) {
    int64_t *val_ptr = (int64_t *)arg1;
    int seed = (int)(*val_ptr % 100000);

    HashSet_int s = hashset_int_new();

    /* Removing from an empty (unallocated) set is safe and returns false */
    if (hashset_int_remove(&s, seed)) {
        hashset_int_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    size_t n = 5 + theft_random_choice(t, 20);
    for (size_t i = 0; i < n; i++) {
        hashset_int_add(&s, seed + (int)i);
    }

    /* Remove a random subset; each remove succeeds exactly once */
    for (size_t i = 0; i < n; i += 2) {
        int key = seed + (int)i;
        if (!hashset_int_remove(&s, key)) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
        if (hashset_int_contains(&s, key)) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
        /* Second remove of the same element fails */
        if (hashset_int_remove(&s, key)) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Untouched elements are still present */
    for (size_t i = 1; i < n; i += 2) {
        if (!hashset_int_contains(&s, seed + (int)i)) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Re-add the removed elements: tombstones are reused */
    size_t len_before = hashset_int_len(&s);
    size_t readded = 0;
    for (size_t i = 0; i < n; i += 2) {
        int key = seed + (int)i;
        if (!hashset_int_add(&s, key)) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
        readded++;
        if (!hashset_int_contains(&s, key)) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
        /* Re-added element is once again a duplicate */
        if (hashset_int_add(&s, key)) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    }

    if (hashset_int_len(&s) != len_before + readded || hashset_int_len(&s) != n) {
        hashset_int_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    /* SET_REMOVE macro agrees with the function */
    if (!SET_REMOVE(int, s, seed) || SET_REMOVE(int, s, seed)) {
        hashset_int_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    hashset_int_free(&s);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 70: iteration yields exactly the live elements, each once
 * After random adds and removes, both hashset_int_iter_next and
 * SET_FOREACH visit every live element exactly once and nothing else.
 *============================================================================*/

static enum theft_trial_res prop_iteration_yields_live_elements(struct theft *t, void *arg1) {
    (void)arg1;

    enum { KEY_DOMAIN = 24 };

    HashSet_int s = hashset_int_new();
    bool ref[KEY_DOMAIN] = {false};

    size_t num_ops = 10 + theft_random_choice(t, 50);
    for (size_t op = 0; op < num_ops; op++) {
        int key = (int)theft_random_choice(t, KEY_DOMAIN);
        if (theft_random_choice(t, 3) != 0) {
            hashset_int_add(&s, key);
            ref[key] = true;
        } else {
            hashset_int_remove(&s, key);
            ref[key] = false;
        }
    }

    size_t live = 0;
    for (int k = 0; k < KEY_DOMAIN; k++) {
        if (ref[k]) live++;
    }

    /* Explicit iterator: each live element exactly once */
    bool visited[KEY_DOMAIN] = {false};
    size_t count = 0;
    HashSetIter_int it = hashset_int_iter(&s);
    Option_SetItem_int item;
    while ((item = hashset_int_iter_next(&it)).has_value) {
        int key = item.value;
        if (key < 0 || key >= KEY_DOMAIN || !ref[key] || visited[key]) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
        visited[key] = true;
        count++;
    }
    if (count != live || count != hashset_int_len(&s)) {
        hashset_int_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    for (int k = 0; k < KEY_DOMAIN; k++) {
        if (ref[k] != visited[k]) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Exhausted iterator stays exhausted */
    if (hashset_int_iter_next(&it).has_value) {
        hashset_int_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    /* SET_FOREACH: same visitation, and break exits early */
    bool visited2[KEY_DOMAIN] = {false};
    size_t count2 = 0;
    int elem;
    SET_FOREACH(int, s, elem) {
        if (elem < 0 || elem >= KEY_DOMAIN || !ref[elem] || visited2[elem]) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
        visited2[elem] = true;
        count2++;
    }
    if (count2 != live) {
        hashset_int_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    for (int k = 0; k < KEY_DOMAIN; k++) {
        if (ref[k] != visited2[k]) {
            hashset_int_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    }

    size_t break_count = 0;
    SET_FOREACH(int, s, elem) {
        break_count++;
        break;
    }
    if (break_count > 1 || (live > 0 && break_count != 1)) {
        hashset_int_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    /* Iterating an empty set yields nothing */
    HashSet_int empty = hashset_int_new();
    HashSetIter_int eit = hashset_int_iter(&empty);
    if (hashset_int_iter_next(&eit).has_value) {
        hashset_int_free(&empty);
        hashset_int_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    size_t empty_count = 0;
    SET_FOREACH(int, empty, elem) {
        empty_count++;
    }
    if (empty_count != 0) {
        hashset_int_free(&empty);
        hashset_int_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    hashset_int_free(&empty);

    hashset_int_free(&s);
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
} HashSetTest;

static HashSetTest hashset_tests[] = {
    {
        "Property 66: add/contains/remove agree with a naive reference array",
        prop_set_matches_reference,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 67: duplicate add returns false and leaves len unchanged",
        prop_duplicate_add,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 68: growth keeps len exact and all elements contained",
        prop_growth_preserves_elements,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 69: remove semantics and tombstone slot reuse",
        prop_remove_and_tombstone_reuse,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 70: iteration yields exactly the live elements, each once",
        prop_iteration_yields_live_elements,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_HASHSET_TESTS (sizeof(hashset_tests) / sizeof(hashset_tests[0]))

int run_hashset_tests(theft_seed seed, int *num_tests) {
    int failures = 0;

    *num_tests = (int)NUM_HASHSET_TESTS;

    printf("\nHashSet Type Tests:\n");

    for (size_t i = 0; i < NUM_HASHSET_TESTS; i++) {
        HashSetTest *test = &hashset_tests[i];

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

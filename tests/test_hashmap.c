/**
 * @file test_hashmap.c
 * @brief Property-based tests for HashMap type
 * 
 * Tests validate correctness properties:
 * - Property 42: HashMap insert-get round-trip
 * - Property 43: HashMap get on missing key returns None
 * - Property 44: HashMap iteration visits all entries
 * - Property 45: HashMap remove then get returns None
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theft.h"
#include <cyan/option.h>
#include <cyan/hashmap.h>

/* Define Option and HashMap types for testing */
OPTION_DEFINE(int);
HASHMAP_DEFINE(int, int);
HASHMAP_ITER_DEFINE(int, int);

/*============================================================================
 * Property 42: HashMap insert-get round-trip
 * For any sequence of key-value insertions, getting each key returns Some with the most recently inserted value
 *============================================================================*/

static enum theft_trial_res prop_insert_get_roundtrip(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int seed = (int)(*val_ptr);
    
    HashMap_int_int m = hashmap_int_int_new();
    
    /* Insert multiple key-value pairs */
    int keys[20];
    int values[20];
    int num_entries = 10 + (seed % 10);  /* 10-19 entries */
    
    for (int i = 0; i < num_entries; i++) {
        keys[i] = seed + i * 7;  /* Generate distinct keys */
        values[i] = seed * 3 + i;
        hashmap_int_int_insert(&m, keys[i], values[i]);
    }
    
    /* Verify all entries are retrievable */
    for (int i = 0; i < num_entries; i++) {
        Option_int opt = hashmap_int_int_get(&m, keys[i]);
        if (!is_some(opt)) {
            hashmap_int_int_free(&m);
            return THEFT_TRIAL_FAIL;
        }
        if (unwrap(opt) != values[i]) {
            hashmap_int_int_free(&m);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test overwriting: insert new value for existing key */
    int overwrite_key = keys[0];
    int new_value = values[0] + 1000;
    hashmap_int_int_insert(&m, overwrite_key, new_value);
    
    Option_int opt = hashmap_int_int_get(&m, overwrite_key);
    if (!is_some(opt) || unwrap(opt) != new_value) {
        hashmap_int_int_free(&m);
        return THEFT_TRIAL_FAIL;
    }
    
    hashmap_int_int_free(&m);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 43: HashMap get on missing key returns None
 * For any hash map and key that was never inserted (or was removed), get(key) returns None
 *============================================================================*/

static enum theft_trial_res prop_get_missing_key(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int seed = (int)(*val_ptr);
    
    /* Test on empty map */
    HashMap_int_int empty_m = hashmap_int_int_new();
    Option_int empty_opt = hashmap_int_int_get(&empty_m, seed);
    if (!is_none(empty_opt)) {
        hashmap_int_int_free(&empty_m);
        return THEFT_TRIAL_FAIL;
    }
    hashmap_int_int_free(&empty_m);
    
    /* Test on map with entries but missing key */
    HashMap_int_int m = hashmap_int_int_new();
    
    /* Insert some entries */
    for (int i = 0; i < 10; i++) {
        hashmap_int_int_insert(&m, seed + i * 2, i);  /* Even offsets */
    }
    
    /* Query for keys that were never inserted (odd offsets) */
    for (int i = 0; i < 10; i++) {
        int missing_key = seed + i * 2 + 1;  /* Odd offsets */
        Option_int opt = hashmap_int_int_get(&m, missing_key);
        if (!is_none(opt)) {
            hashmap_int_int_free(&m);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Verify contains() also returns false for missing keys */
    if (hashmap_int_int_contains(&m, seed + 1)) {
        hashmap_int_int_free(&m);
        return THEFT_TRIAL_FAIL;
    }
    
    hashmap_int_int_free(&m);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 44: HashMap iteration visits all entries
 * For any hash map with n entries, iterating visits exactly n key-value pairs
 *============================================================================*/

static enum theft_trial_res prop_iteration(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int seed = (int)(*val_ptr);
    
    HashMap_int_int m = hashmap_int_int_new();
    
    /* Insert entries and track them - use fixed size arrays to avoid allocation issues */
    int num_entries = 5 + (((unsigned int)seed) % 15);  /* 5-19 entries */
    int inserted_keys[20];
    int inserted_values[20];
    bool visited[20] = {false};
    
    for (int i = 0; i < num_entries; i++) {
        inserted_keys[i] = seed + i * 13;  /* Spread out keys */
        inserted_values[i] = seed * 2 + i;
        hashmap_int_int_insert(&m, inserted_keys[i], inserted_values[i]);
    }
    
    /* Iterate and count */
    HashMapIter_int_int it = hashmap_int_int_iter(&m);
    int count = 0;
    
    Option_MapPair_int_int pair;
    while ((pair = hashmap_int_int_iter_next(&it)).has_value) {
        count++;
        
        /* Find this key in our inserted list */
        bool found = false;
        for (int i = 0; i < num_entries; i++) {
            if (inserted_keys[i] == pair.value.key) {
                if (visited[i]) {
                    /* Already visited - duplicate! */
                    hashmap_int_int_free(&m);
                    return THEFT_TRIAL_FAIL;
                }
                if (inserted_values[i] != pair.value.value) {
                    /* Value mismatch */
                    hashmap_int_int_free(&m);
                    return THEFT_TRIAL_FAIL;
                }
                visited[i] = true;
                found = true;
                break;
            }
        }
        
        if (!found) {
            /* Key not in our inserted list */
            hashmap_int_int_free(&m);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Verify count matches */
    if (count != num_entries) {
        hashmap_int_int_free(&m);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify all entries were visited */
    for (int i = 0; i < num_entries; i++) {
        if (!visited[i]) {
            hashmap_int_int_free(&m);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    hashmap_int_int_free(&m);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 45: HashMap remove then get returns None
 * For any hash map and key, after removing that key, get(key) returns None,
 * and all other keys remain accessible
 *============================================================================*/

static enum theft_trial_res prop_remove(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int seed = (int)(*val_ptr);
    
    HashMap_int_int m = hashmap_int_int_new();
    
    /* Insert entries */
    int num_entries = 10;
    int keys[10];
    int values[10];
    
    for (int i = 0; i < num_entries; i++) {
        keys[i] = seed + i * 11;
        values[i] = seed * 5 + i;
        hashmap_int_int_insert(&m, keys[i], values[i]);
    }
    
    /* Remove some entries and verify */
    for (int i = 0; i < num_entries; i += 2) {  /* Remove even indices */
        int key_to_remove = keys[i];
        int expected_value = values[i];
        
        /* Remove should return the value */
        Option_int removed = hashmap_int_int_remove(&m, key_to_remove);
        if (!is_some(removed) || unwrap(removed) != expected_value) {
            hashmap_int_int_free(&m);
            return THEFT_TRIAL_FAIL;
        }
        
        /* Get should now return None */
        Option_int opt = hashmap_int_int_get(&m, key_to_remove);
        if (!is_none(opt)) {
            hashmap_int_int_free(&m);
            return THEFT_TRIAL_FAIL;
        }
        
        /* Contains should return false */
        if (hashmap_int_int_contains(&m, key_to_remove)) {
            hashmap_int_int_free(&m);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Verify remaining entries are still accessible */
    for (int i = 1; i < num_entries; i += 2) {  /* Odd indices should remain */
        Option_int opt = hashmap_int_int_get(&m, keys[i]);
        if (!is_some(opt) || unwrap(opt) != values[i]) {
            hashmap_int_int_free(&m);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Removing non-existent key should return None */
    Option_int missing_remove = hashmap_int_int_remove(&m, seed + 1000000);
    if (!is_none(missing_remove)) {
        hashmap_int_int_free(&m);
        return THEFT_TRIAL_FAIL;
    }
    
    hashmap_int_int_free(&m);
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
} HashMapTest;

static HashMapTest hashmap_tests[] = {
    {
        "Property 42: HashMap insert-get round-trip",
        prop_insert_get_roundtrip,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 43: HashMap get on missing key returns None",
        prop_get_missing_key,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 44: HashMap iteration visits all entries",
        prop_iteration,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 45: HashMap remove then get returns None",
        prop_remove,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_HASHMAP_TESTS (sizeof(hashmap_tests) / sizeof(hashmap_tests[0]))

int run_hashmap_tests(theft_seed seed) {
    int failures = 0;
    
    printf("\nHashMap Type Tests:\n");
    
    for (size_t i = 0; i < NUM_HASHMAP_TESTS; i++) {
        HashMapTest *test = &hashmap_tests[i];
        
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

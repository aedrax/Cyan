/**
 * @file test_bitset.c
 * @brief Property-based tests for bitset types
 * 
 * Tests validate correctness properties:
 * - Property 5: Bitset new returns empty
 * - Property 4: Bitset set/clear/get consistency
 * - Property 10: From raw masking
 */

#include <stdio.h>
#include "theft.h"
#include <cyan/bitset.h>

/* Define test types for various bit widths */
BITSET_DEFINE(1);
BITSET_DEFINE(6);
BITSET_DEFINE(7);
BITSET_DEFINE(8);
BITSET_DEFINE(12);
BITSET_DEFINE(15);
BITSET_DEFINE(16);
BITSET_DEFINE(24);
BITSET_DEFINE(31);
BITSET_DEFINE(32);
BITSET_DEFINE(48);
BITSET_DEFINE(63);
BITSET_DEFINE(64);

/*============================================================================
 * Property 5: Bitset new returns empty
 * 
 * For any bit width N, bitset_N_new() SHALL return a bitset where none() 
 * returns true and count() returns 0.
 *============================================================================*/

static enum theft_trial_res prop_bitset_new_empty(struct theft *t, void *arg1) {
    (void)t;
    (void)arg1;
    
    /* Test Bitset_1 */
    {
        Bitset_1 bs = bitset_1_new();
        if (!bitset_1_none(&bs)) {
            fprintf(stderr, "Bitset_1: new() is not empty (none() returned false)\n");
            return THEFT_TRIAL_FAIL;
        }
        if (bitset_1_count(&bs) != 0) {
            fprintf(stderr, "Bitset_1: new() count is %u, expected 0\n", bitset_1_count(&bs));
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_6 */
    {
        Bitset_6 bs = bitset_6_new();
        if (!bitset_6_none(&bs)) {
            fprintf(stderr, "Bitset_6: new() is not empty\n");
            return THEFT_TRIAL_FAIL;
        }
        if (bitset_6_count(&bs) != 0) {
            fprintf(stderr, "Bitset_6: new() count is %u, expected 0\n", bitset_6_count(&bs));
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_8 */
    {
        Bitset_8 bs = bitset_8_new();
        if (!bitset_8_none(&bs)) {
            fprintf(stderr, "Bitset_8: new() is not empty\n");
            return THEFT_TRIAL_FAIL;
        }
        if (bitset_8_count(&bs) != 0) {
            fprintf(stderr, "Bitset_8: new() count is %u, expected 0\n", bitset_8_count(&bs));
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_16 */
    {
        Bitset_16 bs = bitset_16_new();
        if (!bitset_16_none(&bs)) {
            fprintf(stderr, "Bitset_16: new() is not empty\n");
            return THEFT_TRIAL_FAIL;
        }
        if (bitset_16_count(&bs) != 0) {
            fprintf(stderr, "Bitset_16: new() count is %u, expected 0\n", bitset_16_count(&bs));
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_32 */
    {
        Bitset_32 bs = bitset_32_new();
        if (!bitset_32_none(&bs)) {
            fprintf(stderr, "Bitset_32: new() is not empty\n");
            return THEFT_TRIAL_FAIL;
        }
        if (bitset_32_count(&bs) != 0) {
            fprintf(stderr, "Bitset_32: new() count is %u, expected 0\n", bitset_32_count(&bs));
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_64 */
    {
        Bitset_64 bs = bitset_64_new();
        if (!bitset_64_none(&bs)) {
            fprintf(stderr, "Bitset_64: new() is not empty\n");
            return THEFT_TRIAL_FAIL;
        }
        if (bitset_64_count(&bs) != 0) {
            fprintf(stderr, "Bitset_64: new() count is %u, expected 0\n", bitset_64_count(&bs));
            return THEFT_TRIAL_FAIL;
        }
    }
    
    return THEFT_TRIAL_PASS;
}


/*============================================================================
 * Property 4: Bitset set/clear/get consistency
 * 
 * For any Bitset_N and valid index i:
 * - After set(bs, i), get(bs, i) SHALL return true
 * - After clear(bs, i), get(bs, i) SHALL return false
 * - toggle(toggle(bs, i), i) SHALL equal the original bitset
 *============================================================================*/

static enum theft_trial_res prop_bitset_set_clear_get(struct theft *t, void *arg1, void *arg2) {
    (void)t;
    uint64_t raw_value = *(uint64_t *)arg1;
    uint64_t index_seed = *(uint64_t *)arg2;
    
    /* Test Bitset_8 */
    {
        u8 index = index_seed % 8;
        Bitset_8 bs = bitset_8_from_raw((u8)raw_value);
        Bitset_8 original = bs;
        
        /* Test set then get */
        bitset_8_set(&bs, index);
        if (!bitset_8_get(&bs, index)) {
            fprintf(stderr, "Bitset_8: set(%u) then get returned false\n", index);
            return THEFT_TRIAL_FAIL;
        }
        
        /* Test clear then get */
        bitset_8_clear(&bs, index);
        if (bitset_8_get(&bs, index)) {
            fprintf(stderr, "Bitset_8: clear(%u) then get returned true\n", index);
            return THEFT_TRIAL_FAIL;
        }
        
        /* Test toggle round-trip */
        bs = original;
        bitset_8_toggle(&bs, index);
        bitset_8_toggle(&bs, index);
        if (!bitset_8_eq(&bs, &original)) {
            fprintf(stderr, "Bitset_8: toggle twice did not restore original\n");
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_16 */
    {
        u8 index = index_seed % 16;
        Bitset_16 bs = bitset_16_from_raw((u16)raw_value);
        Bitset_16 original = bs;
        
        bitset_16_set(&bs, index);
        if (!bitset_16_get(&bs, index)) {
            fprintf(stderr, "Bitset_16: set(%u) then get returned false\n", index);
            return THEFT_TRIAL_FAIL;
        }
        
        bitset_16_clear(&bs, index);
        if (bitset_16_get(&bs, index)) {
            fprintf(stderr, "Bitset_16: clear(%u) then get returned true\n", index);
            return THEFT_TRIAL_FAIL;
        }
        
        bs = original;
        bitset_16_toggle(&bs, index);
        bitset_16_toggle(&bs, index);
        if (!bitset_16_eq(&bs, &original)) {
            fprintf(stderr, "Bitset_16: toggle twice did not restore original\n");
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_32 */
    {
        u8 index = index_seed % 32;
        Bitset_32 bs = bitset_32_from_raw((u32)raw_value);
        Bitset_32 original = bs;
        
        bitset_32_set(&bs, index);
        if (!bitset_32_get(&bs, index)) {
            fprintf(stderr, "Bitset_32: set(%u) then get returned false\n", index);
            return THEFT_TRIAL_FAIL;
        }
        
        bitset_32_clear(&bs, index);
        if (bitset_32_get(&bs, index)) {
            fprintf(stderr, "Bitset_32: clear(%u) then get returned true\n", index);
            return THEFT_TRIAL_FAIL;
        }
        
        bs = original;
        bitset_32_toggle(&bs, index);
        bitset_32_toggle(&bs, index);
        if (!bitset_32_eq(&bs, &original)) {
            fprintf(stderr, "Bitset_32: toggle twice did not restore original\n");
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_64 */
    {
        u8 index = index_seed % 64;
        Bitset_64 bs = bitset_64_from_raw(raw_value);
        Bitset_64 original = bs;
        
        bitset_64_set(&bs, index);
        if (!bitset_64_get(&bs, index)) {
            fprintf(stderr, "Bitset_64: set(%u) then get returned false\n", index);
            return THEFT_TRIAL_FAIL;
        }
        
        bitset_64_clear(&bs, index);
        if (bitset_64_get(&bs, index)) {
            fprintf(stderr, "Bitset_64: clear(%u) then get returned true\n", index);
            return THEFT_TRIAL_FAIL;
        }
        
        bs = original;
        bitset_64_toggle(&bs, index);
        bitset_64_toggle(&bs, index);
        if (!bitset_64_eq(&bs, &original)) {
            fprintf(stderr, "Bitset_64: toggle twice did not restore original\n");
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_6 (non-power-of-2) */
    {
        u8 index = index_seed % 6;
        Bitset_6 bs = bitset_6_from_raw((u8)raw_value);
        Bitset_6 original = bs;
        
        bitset_6_set(&bs, index);
        if (!bitset_6_get(&bs, index)) {
            fprintf(stderr, "Bitset_6: set(%u) then get returned false\n", index);
            return THEFT_TRIAL_FAIL;
        }
        
        bitset_6_clear(&bs, index);
        if (bitset_6_get(&bs, index)) {
            fprintf(stderr, "Bitset_6: clear(%u) then get returned true\n", index);
            return THEFT_TRIAL_FAIL;
        }
        
        bs = original;
        bitset_6_toggle(&bs, index);
        bitset_6_toggle(&bs, index);
        if (!bitset_6_eq(&bs, &original)) {
            fprintf(stderr, "Bitset_6: toggle twice did not restore original\n");
            return THEFT_TRIAL_FAIL;
        }
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 10: From raw masking
 * 
 * For any bit width N and raw value v, bitset_N_from_raw(v).bits SHALL 
 * equal v & MASK(N).
 *============================================================================*/

static enum theft_trial_res prop_from_raw_masking(struct theft *t, void *arg1) {
    (void)t;
    uint64_t value = *(uint64_t *)arg1;
    
    /* Test Bitset_1 */
    {
        Bitset_1 bs = bitset_1_from_raw((u8)value);
        u8 expected = value & 0x1;
        if (bs.bits != expected) {
            fprintf(stderr, "Bitset_1: from_raw(%lu).bits = %u, expected %u\n",
                    (unsigned long)value, bs.bits, expected);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_6 */
    {
        Bitset_6 bs = bitset_6_from_raw((u8)value);
        u8 expected = value & 0x3F;
        if (bs.bits != expected) {
            fprintf(stderr, "Bitset_6: from_raw(%lu).bits = %u, expected %u\n",
                    (unsigned long)value, bs.bits, expected);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_8 */
    {
        Bitset_8 bs = bitset_8_from_raw((u8)value);
        u8 expected = value & 0xFF;
        if (bs.bits != expected) {
            fprintf(stderr, "Bitset_8: from_raw(%lu).bits = %u, expected %u\n",
                    (unsigned long)value, bs.bits, expected);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_12 */
    {
        Bitset_12 bs = bitset_12_from_raw((u16)value);
        u16 expected = value & 0xFFF;
        if (bs.bits != expected) {
            fprintf(stderr, "Bitset_12: from_raw(%lu).bits = %u, expected %u\n",
                    (unsigned long)value, bs.bits, expected);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_16 */
    {
        Bitset_16 bs = bitset_16_from_raw((u16)value);
        u16 expected = value & 0xFFFF;
        if (bs.bits != expected) {
            fprintf(stderr, "Bitset_16: from_raw(%lu).bits = %u, expected %u\n",
                    (unsigned long)value, bs.bits, expected);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_24 */
    {
        Bitset_24 bs = bitset_24_from_raw((u32)value);
        u32 expected = value & 0xFFFFFF;
        if (bs.bits != expected) {
            fprintf(stderr, "Bitset_24: from_raw(%lu).bits = %u, expected %u\n",
                    (unsigned long)value, bs.bits, expected);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_32 */
    {
        Bitset_32 bs = bitset_32_from_raw((u32)value);
        u32 expected = value & 0xFFFFFFFF;
        if (bs.bits != expected) {
            fprintf(stderr, "Bitset_32: from_raw(%lu).bits = %u, expected %u\n",
                    (unsigned long)value, bs.bits, expected);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_48 */
    {
        Bitset_48 bs = bitset_48_from_raw(value);
        u64 expected = value & 0xFFFFFFFFFFFFULL;
        if (bs.bits != expected) {
            fprintf(stderr, "Bitset_48: from_raw(%lu).bits = %lu, expected %lu\n",
                    (unsigned long)value, (unsigned long)bs.bits, (unsigned long)expected);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_63 */
    {
        Bitset_63 bs = bitset_63_from_raw(value);
        u64 expected = value & 0x7FFFFFFFFFFFFFFFULL;
        if (bs.bits != expected) {
            fprintf(stderr, "Bitset_63: from_raw(%lu).bits = %lu, expected %lu\n",
                    (unsigned long)value, (unsigned long)bs.bits, (unsigned long)expected);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_64 */
    {
        Bitset_64 bs = bitset_64_from_raw(value);
        u64 expected = value;  /* No masking needed for 64 bits */
        if (bs.bits != expected) {
            fprintf(stderr, "Bitset_64: from_raw(%lu).bits = %lu, expected %lu\n",
                    (unsigned long)value, (unsigned long)bs.bits, (unsigned long)expected);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Test Registration
 *============================================================================*/

#define MIN_TEST_TRIALS 100

int run_bitset_tests(theft_seed seed) {
    int failures = 0;
    
    printf("\nBitset Type Tests:\n");
    
    /* Property 5: Bitset new returns empty */
    {
        struct theft_run_config config = {
            .name = "Property 5: Bitset new returns empty",
            .prop1 = prop_bitset_new_empty,
            .type_info = { theft_get_builtin_type_info(THEFT_BUILTIN_bool) },
            .trials = MIN_TEST_TRIALS,
            .seed = seed ? seed : theft_seed_of_time(),
        };
        
        enum theft_run_res res = theft_run(&config);
        const char *status = (res == THEFT_RUN_PASS) ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m";
        printf("  [%s] %s\n", status, config.name);
        if (res != THEFT_RUN_PASS) failures++;
    }
    
    /* Property 4: Bitset set/clear/get consistency */
    {
        struct theft_run_config config = {
            .name = "Property 4: Bitset set/clear/get consistency",
            .prop2 = prop_bitset_set_clear_get,
            .type_info = { 
                theft_get_builtin_type_info(THEFT_BUILTIN_uint64_t),
                theft_get_builtin_type_info(THEFT_BUILTIN_uint64_t)
            },
            .trials = MIN_TEST_TRIALS,
            .seed = seed ? seed : theft_seed_of_time(),
        };
        
        enum theft_run_res res = theft_run(&config);
        const char *status = (res == THEFT_RUN_PASS) ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m";
        printf("  [%s] %s\n", status, config.name);
        if (res != THEFT_RUN_PASS) failures++;
    }
    
    /* Property 10: From raw masking */
    {
        struct theft_run_config config = {
            .name = "Property 10: From raw masking",
            .prop1 = prop_from_raw_masking,
            .type_info = { theft_get_builtin_type_info(THEFT_BUILTIN_uint64_t) },
            .trials = MIN_TEST_TRIALS,
            .seed = seed ? seed : theft_seed_of_time(),
        };
        
        enum theft_run_res res = theft_run(&config);
        const char *status = (res == THEFT_RUN_PASS) ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m";
        printf("  [%s] %s\n", status, config.name);
        if (res != THEFT_RUN_PASS) failures++;
    }
    
    return failures;
}

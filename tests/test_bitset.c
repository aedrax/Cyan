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
 * Property 6: Set operations match bitwise equivalents
 * 
 * For any two Bitset_N values bs1 and bs2:
 * - union(bs1, bs2).bits SHALL equal bs1.bits | bs2.bits
 * - intersect(bs1, bs2).bits SHALL equal bs1.bits & bs2.bits
 * - diff(bs1, bs2).bits SHALL equal bs1.bits & ~bs2.bits
 * - complement(bs).bits SHALL equal ~bs.bits & MASK(N)
 *============================================================================*/

static enum theft_trial_res prop_set_ops_bitwise(struct theft *t, void *arg1, void *arg2) {
    (void)t;
    uint64_t val1 = *(uint64_t *)arg1;
    uint64_t val2 = *(uint64_t *)arg2;
    
    /* Test Bitset_8 */
    {
        Bitset_8 bs1 = bitset_8_from_raw((u8)val1);
        Bitset_8 bs2 = bitset_8_from_raw((u8)val2);
        u8 mask = 0xFF;
        
        /* Union */
        Bitset_8 result = bitset_8_union(&bs1, &bs2);
        u8 expected = (bs1.bits | bs2.bits) & mask;
        if (result.bits != expected) {
            fprintf(stderr, "Bitset_8: union(%u, %u).bits = %u, expected %u\n",
                    bs1.bits, bs2.bits, result.bits, expected);
            return THEFT_TRIAL_FAIL;
        }
        
        /* Intersect */
        result = bitset_8_intersect(&bs1, &bs2);
        expected = (bs1.bits & bs2.bits) & mask;
        if (result.bits != expected) {
            fprintf(stderr, "Bitset_8: intersect(%u, %u).bits = %u, expected %u\n",
                    bs1.bits, bs2.bits, result.bits, expected);
            return THEFT_TRIAL_FAIL;
        }
        
        /* Diff */
        result = bitset_8_diff(&bs1, &bs2);
        expected = (bs1.bits & ~bs2.bits) & mask;
        if (result.bits != expected) {
            fprintf(stderr, "Bitset_8: diff(%u, %u).bits = %u, expected %u\n",
                    bs1.bits, bs2.bits, result.bits, expected);
            return THEFT_TRIAL_FAIL;
        }
        
        /* Complement */
        result = bitset_8_complement(&bs1);
        expected = (~bs1.bits) & mask;
        if (result.bits != expected) {
            fprintf(stderr, "Bitset_8: complement(%u).bits = %u, expected %u\n",
                    bs1.bits, result.bits, expected);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_6 (non-power-of-2) */
    {
        Bitset_6 bs1 = bitset_6_from_raw((u8)val1);
        Bitset_6 bs2 = bitset_6_from_raw((u8)val2);
        u8 mask = 0x3F;
        
        /* Union */
        Bitset_6 result = bitset_6_union(&bs1, &bs2);
        u8 expected = (bs1.bits | bs2.bits) & mask;
        if (result.bits != expected) {
            fprintf(stderr, "Bitset_6: union(%u, %u).bits = %u, expected %u\n",
                    bs1.bits, bs2.bits, result.bits, expected);
            return THEFT_TRIAL_FAIL;
        }
        
        /* Intersect */
        result = bitset_6_intersect(&bs1, &bs2);
        expected = (bs1.bits & bs2.bits) & mask;
        if (result.bits != expected) {
            fprintf(stderr, "Bitset_6: intersect(%u, %u).bits = %u, expected %u\n",
                    bs1.bits, bs2.bits, result.bits, expected);
            return THEFT_TRIAL_FAIL;
        }
        
        /* Diff */
        result = bitset_6_diff(&bs1, &bs2);
        expected = (bs1.bits & ~bs2.bits) & mask;
        if (result.bits != expected) {
            fprintf(stderr, "Bitset_6: diff(%u, %u).bits = %u, expected %u\n",
                    bs1.bits, bs2.bits, result.bits, expected);
            return THEFT_TRIAL_FAIL;
        }
        
        /* Complement */
        result = bitset_6_complement(&bs1);
        expected = (~bs1.bits) & mask;
        if (result.bits != expected) {
            fprintf(stderr, "Bitset_6: complement(%u).bits = %u, expected %u\n",
                    bs1.bits, result.bits, expected);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_32 */
    {
        Bitset_32 bs1 = bitset_32_from_raw((u32)val1);
        Bitset_32 bs2 = bitset_32_from_raw((u32)val2);
        u32 mask = 0xFFFFFFFF;
        
        /* Union */
        Bitset_32 result = bitset_32_union(&bs1, &bs2);
        u32 expected = (bs1.bits | bs2.bits) & mask;
        if (result.bits != expected) {
            fprintf(stderr, "Bitset_32: union failed\n");
            return THEFT_TRIAL_FAIL;
        }
        
        /* Intersect */
        result = bitset_32_intersect(&bs1, &bs2);
        expected = (bs1.bits & bs2.bits) & mask;
        if (result.bits != expected) {
            fprintf(stderr, "Bitset_32: intersect failed\n");
            return THEFT_TRIAL_FAIL;
        }
        
        /* Diff */
        result = bitset_32_diff(&bs1, &bs2);
        expected = (bs1.bits & ~bs2.bits) & mask;
        if (result.bits != expected) {
            fprintf(stderr, "Bitset_32: diff failed\n");
            return THEFT_TRIAL_FAIL;
        }
        
        /* Complement */
        result = bitset_32_complement(&bs1);
        expected = (~bs1.bits) & mask;
        if (result.bits != expected) {
            fprintf(stderr, "Bitset_32: complement failed\n");
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_64 */
    {
        Bitset_64 bs1 = bitset_64_from_raw(val1);
        Bitset_64 bs2 = bitset_64_from_raw(val2);
        u64 mask = ~0ULL;
        
        /* Union */
        Bitset_64 result = bitset_64_union(&bs1, &bs2);
        u64 expected = (bs1.bits | bs2.bits) & mask;
        if (result.bits != expected) {
            fprintf(stderr, "Bitset_64: union failed\n");
            return THEFT_TRIAL_FAIL;
        }
        
        /* Intersect */
        result = bitset_64_intersect(&bs1, &bs2);
        expected = (bs1.bits & bs2.bits) & mask;
        if (result.bits != expected) {
            fprintf(stderr, "Bitset_64: intersect failed\n");
            return THEFT_TRIAL_FAIL;
        }
        
        /* Diff */
        result = bitset_64_diff(&bs1, &bs2);
        expected = (bs1.bits & ~bs2.bits) & mask;
        if (result.bits != expected) {
            fprintf(stderr, "Bitset_64: diff failed\n");
            return THEFT_TRIAL_FAIL;
        }
        
        /* Complement */
        result = bitset_64_complement(&bs1);
        expected = (~bs1.bits) & mask;
        if (result.bits != expected) {
            fprintf(stderr, "Bitset_64: complement failed\n");
            return THEFT_TRIAL_FAIL;
        }
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 7: Complement round-trip
 * 
 * For any Bitset_N value bs, complement(complement(bs)) SHALL equal bs.
 *============================================================================*/

static enum theft_trial_res prop_complement_roundtrip(struct theft *t, void *arg1) {
    (void)t;
    uint64_t value = *(uint64_t *)arg1;
    
    /* Test Bitset_8 */
    {
        Bitset_8 bs = bitset_8_from_raw((u8)value);
        Bitset_8 comp1 = bitset_8_complement(&bs);
        Bitset_8 comp2 = bitset_8_complement(&comp1);
        if (!bitset_8_eq(&bs, &comp2)) {
            fprintf(stderr, "Bitset_8: complement(complement(%u)) != original\n", bs.bits);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_6 */
    {
        Bitset_6 bs = bitset_6_from_raw((u8)value);
        Bitset_6 comp1 = bitset_6_complement(&bs);
        Bitset_6 comp2 = bitset_6_complement(&comp1);
        if (!bitset_6_eq(&bs, &comp2)) {
            fprintf(stderr, "Bitset_6: complement(complement(%u)) != original\n", bs.bits);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_16 */
    {
        Bitset_16 bs = bitset_16_from_raw((u16)value);
        Bitset_16 comp1 = bitset_16_complement(&bs);
        Bitset_16 comp2 = bitset_16_complement(&comp1);
        if (!bitset_16_eq(&bs, &comp2)) {
            fprintf(stderr, "Bitset_16: complement round-trip failed\n");
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_32 */
    {
        Bitset_32 bs = bitset_32_from_raw((u32)value);
        Bitset_32 comp1 = bitset_32_complement(&bs);
        Bitset_32 comp2 = bitset_32_complement(&comp1);
        if (!bitset_32_eq(&bs, &comp2)) {
            fprintf(stderr, "Bitset_32: complement round-trip failed\n");
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_64 */
    {
        Bitset_64 bs = bitset_64_from_raw(value);
        Bitset_64 comp1 = bitset_64_complement(&bs);
        Bitset_64 comp2 = bitset_64_complement(&comp1);
        if (!bitset_64_eq(&bs, &comp2)) {
            fprintf(stderr, "Bitset_64: complement round-trip failed\n");
            return THEFT_TRIAL_FAIL;
        }
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 8: Equality reflexivity and correctness
 * 
 * For any Bitset_N value bs, eq(bs, bs) SHALL return true. 
 * For any two Bitset_N values bs1 and bs2, eq(bs1, bs2) SHALL return true 
 * if and only if bs1.bits == bs2.bits.
 *============================================================================*/

static enum theft_trial_res prop_equality(struct theft *t, void *arg1, void *arg2) {
    (void)t;
    uint64_t val1 = *(uint64_t *)arg1;
    uint64_t val2 = *(uint64_t *)arg2;
    
    /* Test Bitset_8 */
    {
        Bitset_8 bs1 = bitset_8_from_raw((u8)val1);
        Bitset_8 bs2 = bitset_8_from_raw((u8)val2);
        
        /* Reflexivity */
        if (!bitset_8_eq(&bs1, &bs1)) {
            fprintf(stderr, "Bitset_8: eq(bs, bs) returned false (reflexivity)\n");
            return THEFT_TRIAL_FAIL;
        }
        
        /* Correctness */
        bool eq_result = bitset_8_eq(&bs1, &bs2);
        bool expected = (bs1.bits == bs2.bits);
        if (eq_result != expected) {
            fprintf(stderr, "Bitset_8: eq(%u, %u) = %d, expected %d\n",
                    bs1.bits, bs2.bits, eq_result, expected);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_6 */
    {
        Bitset_6 bs1 = bitset_6_from_raw((u8)val1);
        Bitset_6 bs2 = bitset_6_from_raw((u8)val2);
        
        if (!bitset_6_eq(&bs1, &bs1)) {
            fprintf(stderr, "Bitset_6: eq(bs, bs) returned false (reflexivity)\n");
            return THEFT_TRIAL_FAIL;
        }
        
        bool eq_result = bitset_6_eq(&bs1, &bs2);
        bool expected = (bs1.bits == bs2.bits);
        if (eq_result != expected) {
            fprintf(stderr, "Bitset_6: eq(%u, %u) = %d, expected %d\n",
                    bs1.bits, bs2.bits, eq_result, expected);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_32 */
    {
        Bitset_32 bs1 = bitset_32_from_raw((u32)val1);
        Bitset_32 bs2 = bitset_32_from_raw((u32)val2);
        
        if (!bitset_32_eq(&bs1, &bs1)) {
            fprintf(stderr, "Bitset_32: eq(bs, bs) returned false (reflexivity)\n");
            return THEFT_TRIAL_FAIL;
        }
        
        bool eq_result = bitset_32_eq(&bs1, &bs2);
        bool expected = (bs1.bits == bs2.bits);
        if (eq_result != expected) {
            fprintf(stderr, "Bitset_32: equality check failed\n");
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_64 */
    {
        Bitset_64 bs1 = bitset_64_from_raw(val1);
        Bitset_64 bs2 = bitset_64_from_raw(val2);
        
        if (!bitset_64_eq(&bs1, &bs1)) {
            fprintf(stderr, "Bitset_64: eq(bs, bs) returned false (reflexivity)\n");
            return THEFT_TRIAL_FAIL;
        }
        
        bool eq_result = bitset_64_eq(&bs1, &bs2);
        bool expected = (bs1.bits == bs2.bits);
        if (eq_result != expected) {
            fprintf(stderr, "Bitset_64: equality check failed\n");
            return THEFT_TRIAL_FAIL;
        }
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 9: Count predicates consistency
 * 
 * For any Bitset_N value bs:
 * - all(bs) SHALL be true if and only if count(bs) == N
 * - any(bs) SHALL be true if and only if count(bs) > 0
 * - none(bs) SHALL be true if and only if count(bs) == 0
 * - none(bs) SHALL equal !any(bs)
 *============================================================================*/

static enum theft_trial_res prop_count_predicates(struct theft *t, void *arg1) {
    (void)t;
    uint64_t value = *(uint64_t *)arg1;
    
    /* Test Bitset_8 */
    {
        Bitset_8 bs = bitset_8_from_raw((u8)value);
        u8 count = bitset_8_count(&bs);
        bool all = bitset_8_all(&bs);
        bool any = bitset_8_any(&bs);
        bool none = bitset_8_none(&bs);
        
        /* all(bs) iff count(bs) == N */
        if (all != (count == 8)) {
            fprintf(stderr, "Bitset_8: all() = %d but count = %u (expected all iff count==8)\n",
                    all, count);
            return THEFT_TRIAL_FAIL;
        }
        
        /* any(bs) iff count(bs) > 0 */
        if (any != (count > 0)) {
            fprintf(stderr, "Bitset_8: any() = %d but count = %u (expected any iff count>0)\n",
                    any, count);
            return THEFT_TRIAL_FAIL;
        }
        
        /* none(bs) iff count(bs) == 0 */
        if (none != (count == 0)) {
            fprintf(stderr, "Bitset_8: none() = %d but count = %u (expected none iff count==0)\n",
                    none, count);
            return THEFT_TRIAL_FAIL;
        }
        
        /* none(bs) == !any(bs) */
        if (none != !any) {
            fprintf(stderr, "Bitset_8: none() = %d but !any() = %d\n", none, !any);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_6 */
    {
        Bitset_6 bs = bitset_6_from_raw((u8)value);
        u8 count = bitset_6_count(&bs);
        bool all = bitset_6_all(&bs);
        bool any = bitset_6_any(&bs);
        bool none = bitset_6_none(&bs);
        
        if (all != (count == 6)) {
            fprintf(stderr, "Bitset_6: all() = %d but count = %u (expected all iff count==6)\n",
                    all, count);
            return THEFT_TRIAL_FAIL;
        }
        
        if (any != (count > 0)) {
            fprintf(stderr, "Bitset_6: any() = %d but count = %u\n", any, count);
            return THEFT_TRIAL_FAIL;
        }
        
        if (none != (count == 0)) {
            fprintf(stderr, "Bitset_6: none() = %d but count = %u\n", none, count);
            return THEFT_TRIAL_FAIL;
        }
        
        if (none != !any) {
            fprintf(stderr, "Bitset_6: none() = %d but !any() = %d\n", none, !any);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_32 */
    {
        Bitset_32 bs = bitset_32_from_raw((u32)value);
        u8 count = bitset_32_count(&bs);
        bool all = bitset_32_all(&bs);
        bool any = bitset_32_any(&bs);
        bool none = bitset_32_none(&bs);
        
        if (all != (count == 32)) {
            fprintf(stderr, "Bitset_32: all() = %d but count = %u\n", all, count);
            return THEFT_TRIAL_FAIL;
        }
        
        if (any != (count > 0)) {
            fprintf(stderr, "Bitset_32: any() = %d but count = %u\n", any, count);
            return THEFT_TRIAL_FAIL;
        }
        
        if (none != (count == 0)) {
            fprintf(stderr, "Bitset_32: none() = %d but count = %u\n", none, count);
            return THEFT_TRIAL_FAIL;
        }
        
        if (none != !any) {
            fprintf(stderr, "Bitset_32: none() != !any()\n");
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test Bitset_64 */
    {
        Bitset_64 bs = bitset_64_from_raw(value);
        u8 count = bitset_64_count(&bs);
        bool all = bitset_64_all(&bs);
        bool any = bitset_64_any(&bs);
        bool none = bitset_64_none(&bs);
        
        if (all != (count == 64)) {
            fprintf(stderr, "Bitset_64: all() = %d but count = %u\n", all, count);
            return THEFT_TRIAL_FAIL;
        }
        
        if (any != (count > 0)) {
            fprintf(stderr, "Bitset_64: any() = %d but count = %u\n", any, count);
            return THEFT_TRIAL_FAIL;
        }
        
        if (none != (count == 0)) {
            fprintf(stderr, "Bitset_64: none() = %d but count = %u\n", none, count);
            return THEFT_TRIAL_FAIL;
        }
        
        if (none != !any) {
            fprintf(stderr, "Bitset_64: none() != !any()\n");
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
    
    /* Property 6: Set operations match bitwise equivalents */
    {
        struct theft_run_config config = {
            .name = "Property 6: Set operations match bitwise equivalents",
            .prop2 = prop_set_ops_bitwise,
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
    
    /* Property 7: Complement round-trip */
    {
        struct theft_run_config config = {
            .name = "Property 7: Complement round-trip",
            .prop1 = prop_complement_roundtrip,
            .type_info = { theft_get_builtin_type_info(THEFT_BUILTIN_uint64_t) },
            .trials = MIN_TEST_TRIALS,
            .seed = seed ? seed : theft_seed_of_time(),
        };
        
        enum theft_run_res res = theft_run(&config);
        const char *status = (res == THEFT_RUN_PASS) ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m";
        printf("  [%s] %s\n", status, config.name);
        if (res != THEFT_RUN_PASS) failures++;
    }
    
    /* Property 8: Equality reflexivity and correctness */
    {
        struct theft_run_config config = {
            .name = "Property 8: Equality reflexivity and correctness",
            .prop2 = prop_equality,
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
    
    /* Property 9: Count predicates consistency */
    {
        struct theft_run_config config = {
            .name = "Property 9: Count predicates consistency",
            .prop1 = prop_count_predicates,
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

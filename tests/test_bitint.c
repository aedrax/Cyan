/**
 * @file test_bitint.c
 * @brief Property-based tests for custom bit-width integer types
 * 
 * Tests validate correctness properties:
 * - Property 1: Unsigned integer masking invariant
 * - Property 3: Arithmetic preserves bit width
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theft.h"
#include <cyan/bitint.h>

/* Define test types for various bit widths
 * Note: We avoid 8, 16, 32, 64 as those conflict with common.h typedefs
 */
UINT_DEFINE(1);
UINT_DEFINE(6);
UINT_DEFINE(7);
UINT_DEFINE(12);
UINT_DEFINE(15);
UINT_DEFINE(24);
UINT_DEFINE(31);
UINT_DEFINE(48);
UINT_DEFINE(63);

/* Signed integer types for testing */
INT_DEFINE(2);
INT_DEFINE(6);
INT_DEFINE(7);
INT_DEFINE(12);
INT_DEFINE(15);
INT_DEFINE(24);
INT_DEFINE(31);
INT_DEFINE(48);
INT_DEFINE(63);

/*============================================================================
 * Property 1: Unsigned integer masking invariant
 * 
 * For any bit width N (1-64) and any value v, creating a uN from v and 
 * reading it back SHALL return v & ((1 << N) - 1).
 *============================================================================*/

static enum theft_trial_res prop_uint_masking(struct theft *t, void *arg1) {
    (void)t;
    uint64_t value = *(uint64_t *)arg1;
    
    /* Test u1 (1-bit) */
    {
        u1 n = u1_new((u8)value);
        u8 expected = value & 0x1;
        if (u1_get(&n) != expected) {
            fprintf(stderr, "u1: got %u, expected %u for input %lu\n", 
                    u1_get(&n), expected, (unsigned long)value);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test u6 (6-bit) */
    {
        u6 n = u6_new((u8)value);
        u8 expected = value & 0x3F;
        if (u6_get(&n) != expected) {
            fprintf(stderr, "u6: got %u, expected %u for input %lu\n", 
                    u6_get(&n), expected, (unsigned long)value);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test u7 (7-bit) */
    {
        u7 n = u7_new((u8)value);
        u8 expected = value & 0x7F;
        if (u7_get(&n) != expected) {
            fprintf(stderr, "u7: got %u, expected %u for input %lu\n", 
                    u7_get(&n), expected, (unsigned long)value);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test u12 (12-bit) */
    {
        u12 n = u12_new((u16)value);
        u16 expected = value & 0xFFF;
        if (u12_get(&n) != expected) {
            fprintf(stderr, "u12: got %u, expected %u for input %lu\n", 
                    u12_get(&n), expected, (unsigned long)value);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test u15 (15-bit) */
    {
        u15 n = u15_new((u16)value);
        u16 expected = value & 0x7FFF;
        if (u15_get(&n) != expected) {
            fprintf(stderr, "u15: got %u, expected %u for input %lu\n", 
                    u15_get(&n), expected, (unsigned long)value);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test u24 (24-bit) */
    {
        u24 n = u24_new((u32)value);
        u32 expected = value & 0xFFFFFF;
        if (u24_get(&n) != expected) {
            fprintf(stderr, "u24: got %u, expected %u for input %lu\n", 
                    u24_get(&n), expected, (unsigned long)value);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test u31 (31-bit) */
    {
        u31 n = u31_new((u32)value);
        u32 expected = value & 0x7FFFFFFF;
        if (u31_get(&n) != expected) {
            fprintf(stderr, "u31: got %u, expected %u for input %lu\n", 
                    u31_get(&n), expected, (unsigned long)value);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test u48 (48-bit) */
    {
        u48 n = u48_new(value);
        u64 expected = value & 0xFFFFFFFFFFFFULL;
        if (u48_get(&n) != expected) {
            fprintf(stderr, "u48: got %lu, expected %lu for input %lu\n", 
                    (unsigned long)u48_get(&n), (unsigned long)expected, (unsigned long)value);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test u63 (63-bit) */
    {
        u63 n = u63_new(value);
        u64 expected = value & 0x7FFFFFFFFFFFFFFFULL;
        if (u63_get(&n) != expected) {
            fprintf(stderr, "u63: got %lu, expected %lu for input %lu\n", 
                    (unsigned long)u63_get(&n), (unsigned long)expected, (unsigned long)value);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 3: Arithmetic preserves bit width
 * 
 * For any uN values a and b, the results of add, sub, mul, and, or, xor, 
 * shl, shr SHALL all be valid uN values (masked to N bits).
 *============================================================================*/

/* Helper to check if value fits in N bits */
#define CHECK_FITS_IN_BITS(val, N) (((val) & ~_CYAN_MASK(N)) == 0)

static enum theft_trial_res prop_arithmetic_bitwidth(struct theft *t, void *arg1, void *arg2) {
    (void)t;
    uint64_t val_a = *(uint64_t *)arg1;
    uint64_t val_b = *(uint64_t *)arg2;
    
    /* Test with u6 (6-bit) - good test case for overflow */
    {
        u6 a = u6_new((u8)val_a);
        u6 b = u6_new((u8)val_b);
        
        /* Addition */
        u6 sum = u6_add(a, b);
        if (!CHECK_FITS_IN_BITS(u6_get(&sum), 6)) {
            fprintf(stderr, "u6_add overflow: result %u doesn't fit in 6 bits\n", u6_get(&sum));
            return THEFT_TRIAL_FAIL;
        }
        
        /* Subtraction */
        u6 diff = u6_sub(a, b);
        if (!CHECK_FITS_IN_BITS(u6_get(&diff), 6)) {
            fprintf(stderr, "u6_sub overflow: result %u doesn't fit in 6 bits\n", u6_get(&diff));
            return THEFT_TRIAL_FAIL;
        }
        
        /* Multiplication */
        u6 prod = u6_mul(a, b);
        if (!CHECK_FITS_IN_BITS(u6_get(&prod), 6)) {
            fprintf(stderr, "u6_mul overflow: result %u doesn't fit in 6 bits\n", u6_get(&prod));
            return THEFT_TRIAL_FAIL;
        }
        
        /* Bitwise AND */
        u6 and_result = u6_and(a, b);
        if (!CHECK_FITS_IN_BITS(u6_get(&and_result), 6)) {
            fprintf(stderr, "u6_and overflow: result %u doesn't fit in 6 bits\n", u6_get(&and_result));
            return THEFT_TRIAL_FAIL;
        }
        
        /* Bitwise OR */
        u6 or_result = u6_or(a, b);
        if (!CHECK_FITS_IN_BITS(u6_get(&or_result), 6)) {
            fprintf(stderr, "u6_or overflow: result %u doesn't fit in 6 bits\n", u6_get(&or_result));
            return THEFT_TRIAL_FAIL;
        }
        
        /* Bitwise XOR */
        u6 xor_result = u6_xor(a, b);
        if (!CHECK_FITS_IN_BITS(u6_get(&xor_result), 6)) {
            fprintf(stderr, "u6_xor overflow: result %u doesn't fit in 6 bits\n", u6_get(&xor_result));
            return THEFT_TRIAL_FAIL;
        }
        
        /* Bitwise NOT */
        u6 not_result = u6_not(a);
        if (!CHECK_FITS_IN_BITS(u6_get(&not_result), 6)) {
            fprintf(stderr, "u6_not overflow: result %u doesn't fit in 6 bits\n", u6_get(&not_result));
            return THEFT_TRIAL_FAIL;
        }
        
        /* Left shift */
        u8 shift = (u8)(val_b % 8);  /* Keep shift reasonable */
        u6 shl_result = u6_shl(a, shift);
        if (!CHECK_FITS_IN_BITS(u6_get(&shl_result), 6)) {
            fprintf(stderr, "u6_shl overflow: result %u doesn't fit in 6 bits\n", u6_get(&shl_result));
            return THEFT_TRIAL_FAIL;
        }
        
        /* Right shift */
        u6 shr_result = u6_shr(a, shift);
        if (!CHECK_FITS_IN_BITS(u6_get(&shr_result), 6)) {
            fprintf(stderr, "u6_shr overflow: result %u doesn't fit in 6 bits\n", u6_get(&shr_result));
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test with u12 (12-bit) */
    {
        u12 a = u12_new((u16)val_a);
        u12 b = u12_new((u16)val_b);
        
        u12 sum = u12_add(a, b);
        if (!CHECK_FITS_IN_BITS(u12_get(&sum), 12)) return THEFT_TRIAL_FAIL;
        
        u12 diff = u12_sub(a, b);
        if (!CHECK_FITS_IN_BITS(u12_get(&diff), 12)) return THEFT_TRIAL_FAIL;
        
        u12 prod = u12_mul(a, b);
        if (!CHECK_FITS_IN_BITS(u12_get(&prod), 12)) return THEFT_TRIAL_FAIL;
        
        u12 not_result = u12_not(a);
        if (!CHECK_FITS_IN_BITS(u12_get(&not_result), 12)) return THEFT_TRIAL_FAIL;
    }
    
    /* Test with u24 (24-bit) */
    {
        u24 a = u24_new((u32)val_a);
        u24 b = u24_new((u32)val_b);
        
        u24 sum = u24_add(a, b);
        if (!CHECK_FITS_IN_BITS(u24_get(&sum), 24)) return THEFT_TRIAL_FAIL;
        
        u24 diff = u24_sub(a, b);
        if (!CHECK_FITS_IN_BITS(u24_get(&diff), 24)) return THEFT_TRIAL_FAIL;
        
        u24 prod = u24_mul(a, b);
        if (!CHECK_FITS_IN_BITS(u24_get(&prod), 24)) return THEFT_TRIAL_FAIL;
        
        u24 not_result = u24_not(a);
        if (!CHECK_FITS_IN_BITS(u24_get(&not_result), 24)) return THEFT_TRIAL_FAIL;
    }
    
    /* Test with u48 (48-bit) */
    {
        u48 a = u48_new(val_a);
        u48 b = u48_new(val_b);
        
        u48 sum = u48_add(a, b);
        if (!CHECK_FITS_IN_BITS(u48_get(&sum), 48)) return THEFT_TRIAL_FAIL;
        
        u48 diff = u48_sub(a, b);
        if (!CHECK_FITS_IN_BITS(u48_get(&diff), 48)) return THEFT_TRIAL_FAIL;
        
        u48 prod = u48_mul(a, b);
        if (!CHECK_FITS_IN_BITS(u48_get(&prod), 48)) return THEFT_TRIAL_FAIL;
        
        u48 not_result = u48_not(a);
        if (!CHECK_FITS_IN_BITS(u48_get(&not_result), 48)) return THEFT_TRIAL_FAIL;
    }
    
    /* Test with u63 (63-bit) */
    {
        u63 a = u63_new(val_a);
        u63 b = u63_new(val_b);
        
        u63 sum = u63_add(a, b);
        if (!CHECK_FITS_IN_BITS(u63_get(&sum), 63)) return THEFT_TRIAL_FAIL;
        
        u63 diff = u63_sub(a, b);
        if (!CHECK_FITS_IN_BITS(u63_get(&diff), 63)) return THEFT_TRIAL_FAIL;
        
        u63 prod = u63_mul(a, b);
        if (!CHECK_FITS_IN_BITS(u63_get(&prod), 63)) return THEFT_TRIAL_FAIL;
        
        u63 not_result = u63_not(a);
        if (!CHECK_FITS_IN_BITS(u63_get(&not_result), 63)) return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 2: Signed integer sign-extension
 * 
 * For any bit width N (2-64) and any value v in the range [-2^(N-1), 2^(N-1)-1],
 * creating an iN from v and reading it back SHALL return v. Values outside this
 * range SHALL wrap using two's complement.
 *============================================================================*/

/* Helper: compute expected value after sign-extension for N-bit signed integer */
static i64 expected_signed_value(i64 value, int bits) {
    /* Mask to N bits */
    u64 mask = (bits >= 64) ? ~(u64)0 : ((u64)1 << bits) - 1;
    u64 masked = (u64)value & mask;
    
    /* Sign-extend: if bit N-1 is set, extend with 1s */
    u64 sign_bit = (u64)1 << (bits - 1);
    if (masked & sign_bit) {
        /* Negative: extend with 1s */
        return (i64)(masked | ~mask);
    } else {
        /* Positive: already correct */
        return (i64)masked;
    }
}

static enum theft_trial_res prop_int_sign_extend(struct theft *t, void *arg1) {
    (void)t;
    int64_t value = *(int64_t *)arg1;
    
    /* Test i2 (2-bit signed: -2 to 1) */
    {
        i2 n = i2_new((i8)value);
        i64 expected = expected_signed_value(value, 2);
        i8 got = i2_get(&n);
        if (got != expected) {
            fprintf(stderr, "i2: got %d, expected %ld for input %ld\n", 
                    got, (long)expected, (long)value);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test i6 (6-bit signed: -32 to 31) */
    {
        i6 n = i6_new((i8)value);
        i64 expected = expected_signed_value(value, 6);
        i8 got = i6_get(&n);
        if (got != expected) {
            fprintf(stderr, "i6: got %d, expected %ld for input %ld\n", 
                    got, (long)expected, (long)value);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test i7 (7-bit signed: -64 to 63) */
    {
        i7 n = i7_new((i8)value);
        i64 expected = expected_signed_value(value, 7);
        i8 got = i7_get(&n);
        if (got != expected) {
            fprintf(stderr, "i7: got %d, expected %ld for input %ld\n", 
                    got, (long)expected, (long)value);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test i12 (12-bit signed: -2048 to 2047) */
    {
        i12 n = i12_new((i16)value);
        i64 expected = expected_signed_value(value, 12);
        i16 got = i12_get(&n);
        if (got != expected) {
            fprintf(stderr, "i12: got %d, expected %ld for input %ld\n", 
                    got, (long)expected, (long)value);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test i15 (15-bit signed: -16384 to 16383) */
    {
        i15 n = i15_new((i16)value);
        i64 expected = expected_signed_value(value, 15);
        i16 got = i15_get(&n);
        if (got != expected) {
            fprintf(stderr, "i15: got %d, expected %ld for input %ld\n", 
                    got, (long)expected, (long)value);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test i24 (24-bit signed) */
    {
        i24 n = i24_new((i32)value);
        i64 expected = expected_signed_value(value, 24);
        i32 got = i24_get(&n);
        if (got != expected) {
            fprintf(stderr, "i24: got %d, expected %ld for input %ld\n", 
                    got, (long)expected, (long)value);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test i31 (31-bit signed) */
    {
        i31 n = i31_new((i32)value);
        i64 expected = expected_signed_value(value, 31);
        i32 got = i31_get(&n);
        if (got != expected) {
            fprintf(stderr, "i31: got %d, expected %ld for input %ld\n", 
                    got, (long)expected, (long)value);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test i48 (48-bit signed) */
    {
        i48 n = i48_new((i64)value);
        i64 expected = expected_signed_value(value, 48);
        i64 got = i48_get(&n);
        if (got != expected) {
            fprintf(stderr, "i48: got %ld, expected %ld for input %ld\n", 
                    (long)got, (long)expected, (long)value);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test i63 (63-bit signed) */
    {
        i63 n = i63_new((i64)value);
        i64 expected = expected_signed_value(value, 63);
        i64 got = i63_get(&n);
        if (got != expected) {
            fprintf(stderr, "i63: got %ld, expected %ld for input %ld\n", 
                    (long)got, (long)expected, (long)value);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Test Registration
 *============================================================================*/

#define MIN_TEST_TRIALS 100

int run_bitint_tests(theft_seed seed) {
    int failures = 0;
    
    printf("\nBitint Type Tests:\n");
    
    /* Property 1: Unsigned masking invariant */
    {
        struct theft_run_config config = {
            .name = "Property 1: Unsigned integer masking invariant",
            .prop1 = prop_uint_masking,
            .type_info = { theft_get_builtin_type_info(THEFT_BUILTIN_uint64_t) },
            .trials = MIN_TEST_TRIALS,
            .seed = seed ? seed : theft_seed_of_time(),
        };
        
        enum theft_run_res res = theft_run(&config);
        const char *status = (res == THEFT_RUN_PASS) ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m";
        printf("  [%s] %s\n", status, config.name);
        if (res != THEFT_RUN_PASS) failures++;
    }
    
    /* Property 2: Signed integer sign-extension */
    {
        struct theft_run_config config = {
            .name = "Property 2: Signed integer sign-extension",
            .prop1 = prop_int_sign_extend,
            .type_info = { theft_get_builtin_type_info(THEFT_BUILTIN_int64_t) },
            .trials = MIN_TEST_TRIALS,
            .seed = seed ? seed : theft_seed_of_time(),
        };
        
        enum theft_run_res res = theft_run(&config);
        const char *status = (res == THEFT_RUN_PASS) ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m";
        printf("  [%s] %s\n", status, config.name);
        if (res != THEFT_RUN_PASS) failures++;
    }
    
    /* Property 3: Arithmetic preserves bit width */
    {
        struct theft_run_config config = {
            .name = "Property 3: Arithmetic preserves bit width",
            .prop2 = prop_arithmetic_bitwidth,
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
    
    return failures;
}

/**
 * @file test_types.c
 * @brief Property-based tests for primitive type aliases
 * 
 * Tests validate correctness properties:
 * - Property 60: Integer type alias size correctness
 * - Property 61: Integer type alias value preservation
 * - Property 62: Pointer-sized integer alias correctness
 * - Property 63: Floating-point type alias size correctness
 * - Property 64: Floating-point type alias value preservation
 * - Property 65: Bool alias correctness
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <float.h>
#include <math.h>
#include "theft.h"
#include <cyan/common.h>

/*============================================================================
 * Property 60: Integer type alias size correctness
 * For any integer type alias, sizeof(alias) equals sizeof(underlying_type)
 *============================================================================*/

static enum theft_trial_res prop_integer_type_sizes(struct theft *t, void *arg1) {
    (void)t;
    (void)arg1;
    
    /* Signed integer sizes */
    if (sizeof(i8) != sizeof(int8_t)) return THEFT_TRIAL_FAIL;
    if (sizeof(i16) != sizeof(int16_t)) return THEFT_TRIAL_FAIL;
    if (sizeof(i32) != sizeof(int32_t)) return THEFT_TRIAL_FAIL;
    if (sizeof(i64) != sizeof(int64_t)) return THEFT_TRIAL_FAIL;
    
    /* Unsigned integer sizes */
    if (sizeof(u8) != sizeof(uint8_t)) return THEFT_TRIAL_FAIL;
    if (sizeof(u16) != sizeof(uint16_t)) return THEFT_TRIAL_FAIL;
    if (sizeof(u32) != sizeof(uint32_t)) return THEFT_TRIAL_FAIL;
    if (sizeof(u64) != sizeof(uint64_t)) return THEFT_TRIAL_FAIL;
    
    /* Verify exact byte sizes */
    if (sizeof(i8) != 1) return THEFT_TRIAL_FAIL;
    if (sizeof(i16) != 2) return THEFT_TRIAL_FAIL;
    if (sizeof(i32) != 4) return THEFT_TRIAL_FAIL;
    if (sizeof(i64) != 8) return THEFT_TRIAL_FAIL;
    
    if (sizeof(u8) != 1) return THEFT_TRIAL_FAIL;
    if (sizeof(u16) != 2) return THEFT_TRIAL_FAIL;
    if (sizeof(u32) != 4) return THEFT_TRIAL_FAIL;
    if (sizeof(u64) != 8) return THEFT_TRIAL_FAIL;
    
#if CYAN_HAS_INT128
    if (sizeof(i128) != 16) return THEFT_TRIAL_FAIL;
    if (sizeof(u128) != 16) return THEFT_TRIAL_FAIL;
#endif
    
    return THEFT_TRIAL_PASS;
}


/*============================================================================
 * Property 61: Integer type alias value preservation
 * For any value within valid range, assigning and reading back preserves value
 *============================================================================*/

static enum theft_trial_res prop_integer_value_preservation(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int64_t val = *val_ptr;
    
    /* Test i8 (range: -128 to 127) */
    i8 test_i8 = (i8)(val % 128);
    if ((int64_t)test_i8 != (val % 128)) return THEFT_TRIAL_FAIL;
    
    /* Test i16 */
    i16 test_i16 = (i16)(val % 32768);
    if ((int64_t)test_i16 != (val % 32768)) return THEFT_TRIAL_FAIL;
    
    /* Test i32 */
    i32 test_i32 = (i32)(val);
    if ((int64_t)test_i32 != (int32_t)val) return THEFT_TRIAL_FAIL;
    
    /* Test i64 */
    i64 test_i64 = (i64)val;
    if (test_i64 != val) return THEFT_TRIAL_FAIL;
    
    /* Test unsigned types with positive values */
    uint64_t uval = (uint64_t)(val < 0 ? -val : val);
    
    /* Test u8 */
    u8 test_u8 = (u8)(uval % 256);
    if ((uint64_t)test_u8 != (uval % 256)) return THEFT_TRIAL_FAIL;
    
    /* Test u16 */
    u16 test_u16 = (u16)(uval % 65536);
    if ((uint64_t)test_u16 != (uval % 65536)) return THEFT_TRIAL_FAIL;
    
    /* Test u32 */
    u32 test_u32 = (u32)(uval);
    if ((uint64_t)test_u32 != (uint32_t)uval) return THEFT_TRIAL_FAIL;
    
    /* Test u64 */
    u64 test_u64 = (u64)uval;
    if (test_u64 != uval) return THEFT_TRIAL_FAIL;
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 62: Pointer-sized integer alias correctness
 * sizeof(isize) and sizeof(usize) equal sizeof(void*), pointer round-trip works
 *============================================================================*/

static enum theft_trial_res prop_pointer_sized_integers(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    
    /* Size checks */
    if (sizeof(isize) != sizeof(void*)) return THEFT_TRIAL_FAIL;
    if (sizeof(usize) != sizeof(void*)) return THEFT_TRIAL_FAIL;
    if (sizeof(usize) != sizeof(size_t)) return THEFT_TRIAL_FAIL;
    if (sizeof(isize) != sizeof(intptr_t)) return THEFT_TRIAL_FAIL;
    if (sizeof(usize) != sizeof(uintptr_t)) return THEFT_TRIAL_FAIL;
    
    /* Pointer round-trip test */
    void *ptr = (void *)val_ptr;
    usize ptr_as_usize = (usize)ptr;
    void *ptr_back = (void *)ptr_as_usize;
    if (ptr_back != ptr) return THEFT_TRIAL_FAIL;
    
    isize ptr_as_isize = (isize)ptr;
    void *ptr_back2 = (void *)ptr_as_isize;
    if (ptr_back2 != ptr) return THEFT_TRIAL_FAIL;
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 63: Floating-point type alias size correctness
 * sizeof(f32) equals sizeof(float), sizeof(f64) equals sizeof(double)
 *============================================================================*/

static enum theft_trial_res prop_float_type_sizes(struct theft *t, void *arg1) {
    (void)t;
    (void)arg1;
    
    /* f32 must equal float */
    if (sizeof(f32) != sizeof(float)) return THEFT_TRIAL_FAIL;
    
    /* f64 must equal double */
    if (sizeof(f64) != sizeof(double)) return THEFT_TRIAL_FAIL;
    
    /* Verify exact byte sizes */
    if (sizeof(f32) != 4) return THEFT_TRIAL_FAIL;
    if (sizeof(f64) != 8) return THEFT_TRIAL_FAIL;
    
#if CYAN_HAS_FLOAT80
    /* f80 should be at least 10 bytes (80 bits), but may be padded to 12 or 16 */
    if (sizeof(f80) < 10) return THEFT_TRIAL_FAIL;
    if (sizeof(f80) != sizeof(long double)) return THEFT_TRIAL_FAIL;
#endif

#if CYAN_HAS_FLOAT128
    if (sizeof(f128) != 16) return THEFT_TRIAL_FAIL;
#endif

#if CYAN_HAS_FLOAT16
    if (sizeof(f16) != 2) return THEFT_TRIAL_FAIL;
#endif
    
    return THEFT_TRIAL_PASS;
}


/*============================================================================
 * Property 64: Floating-point type alias value preservation
 * For any finite float value, assigning and reading back preserves value
 *============================================================================*/

static enum theft_trial_res prop_float_value_preservation(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    
    /* Generate a finite float value from the random input */
    double dval = (double)(*val_ptr) / 1000.0;
    
    /* Skip if not finite */
    if (!isfinite(dval)) {
        return THEFT_TRIAL_SKIP;
    }
    
    /* Test f64 round-trip */
    f64 test_f64 = (f64)dval;
    if (test_f64 != dval) return THEFT_TRIAL_FAIL;
    
    /* Test f32 round-trip (with precision loss expected) */
    float fval = (float)dval;
    f32 test_f32 = (f32)fval;
    if (test_f32 != fval) return THEFT_TRIAL_FAIL;
    
    /* Verify the alias types work correctly in arithmetic */
    f64 a = 1.5;
    f64 b = 2.5;
    f64 c = a + b;
    if (c != 4.0) return THEFT_TRIAL_FAIL;
    
    f32 x = 1.5f;
    f32 y = 2.5f;
    f32 z = x + y;
    if (z != 4.0f) return THEFT_TRIAL_FAIL;
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 65: Bool alias correctness
 * bool alias behaves identically to C11 _Bool
 *============================================================================*/

static enum theft_trial_res prop_bool_alias(struct theft *t, void *arg1) {
    (void)t;
    (void)arg1;
    
    /* Size check */
    if (sizeof(bool) != sizeof(_Bool)) return THEFT_TRIAL_FAIL;
    
    /* Value checks */
    bool b_true = true;
    bool b_false = false;
    
    _Bool _b_true = 1;
    _Bool _b_false = 0;
    
    if (b_true != _b_true) return THEFT_TRIAL_FAIL;
    if (b_false != _b_false) return THEFT_TRIAL_FAIL;
    
    /* Verify true and false constants */
    if (true != 1) return THEFT_TRIAL_FAIL;
    if (false != 0) return THEFT_TRIAL_FAIL;
    
    /* Verify boolean operations */
    bool and_result = true && true;
    if (!and_result) return THEFT_TRIAL_FAIL;
    
    bool or_result = false || true;
    if (!or_result) return THEFT_TRIAL_FAIL;
    
    bool not_result = !false;
    if (!not_result) return THEFT_TRIAL_FAIL;
    
    /* Verify conversion from integers */
    bool from_zero = (bool)0;
    bool from_one = (bool)1;
    bool from_nonzero = (bool)42;
    
    if (from_zero != false) return THEFT_TRIAL_FAIL;
    if (from_one != true) return THEFT_TRIAL_FAIL;
    if (from_nonzero != true) return THEFT_TRIAL_FAIL;
    
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
} TypesTest;

static TypesTest types_tests[] = {
    {
        "Property 60: Integer type alias size correctness",
        prop_integer_type_sizes,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 61: Integer type alias value preservation",
        prop_integer_value_preservation,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 62: Pointer-sized integer alias correctness",
        prop_pointer_sized_integers,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 63: Floating-point type alias size correctness",
        prop_float_type_sizes,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 64: Floating-point type alias value preservation",
        prop_float_value_preservation,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 65: Bool alias correctness",
        prop_bool_alias,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_TYPES_TESTS (sizeof(types_tests) / sizeof(types_tests[0]))

int run_types_tests(theft_seed seed) {
    int failures = 0;
    
    printf("\nPrimitive Type Alias Tests:\n");
    
    for (size_t i = 0; i < NUM_TYPES_TESTS; i++) {
        TypesTest *test = &types_tests[i];
        
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

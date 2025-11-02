/**
 * @file test_serialize.c
 * @brief Property-based tests for serialization
 * 
 * Tests validate correctness properties:
 * - Property 28: Serialization round-trip
 * - Property 29: Invalid input returns error
 * - Property 30: Pretty-print preserves parseability
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "theft.h"
#include <cyan/serialize.h>

/*============================================================================
 * Property 28: Serialization round-trip
 * For any valid data structure, serializing then deserializing SHALL produce
 * an equivalent structure.
 *============================================================================*/

/**
 * Property 28a: Integer round-trip
 * For any integer, serialize_int then parse_int returns the original value
 */
static enum theft_trial_res prop_int_roundtrip(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    
    /* Constrain to int range */
    int val = (int)(*val_ptr % INT_MAX);
    
    /* Serialize */
    char *serialized = serialize_int(val);
    if (!serialized) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Parse back */
    Result_int_ParseError result = parse_int(serialized, NULL);
    free(serialized);
    
    if (!is_ok(result)) {
        return THEFT_TRIAL_FAIL;
    }
    
    int parsed = unwrap_ok(result);
    if (parsed != val) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/**
 * Property 28b: Double round-trip
 * For any finite double, serialize_double then parse_double returns the original value
 */
static enum theft_trial_res prop_double_roundtrip(struct theft *t, void *arg1) {
    (void)t;
    double *val_ptr = (double *)arg1;
    double val = *val_ptr;
    
    /* Skip NaN (NaN != NaN by definition) and infinity for basic round-trip */
    if (isnan(val) || isinf(val)) {
        return THEFT_TRIAL_SKIP;
    }
    
    /* Serialize */
    char *serialized = serialize_double(val);
    if (!serialized) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Parse back */
    Result_double_ParseError result = parse_double(serialized, NULL);
    free(serialized);
    
    if (!is_ok(result)) {
        return THEFT_TRIAL_FAIL;
    }
    
    double parsed = unwrap_ok(result);
    
    /* Check equality with tolerance for floating point */
    double diff = fabs(parsed - val);
    double tolerance = fabs(val) * 1e-15;
    if (tolerance < 1e-15) tolerance = 1e-15;
    
    if (diff > tolerance) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * String Generator for Property Testing
 *============================================================================*/

/* Generate printable ASCII strings for testing */
static enum theft_alloc_res string_alloc(struct theft *t, void *env, void **output) {
    (void)env;
    
    /* Generate length 0-100 */
    size_t len = theft_random_choice(t, 101);
    
    char *str = malloc(len + 1);
    if (!str) return THEFT_ALLOC_ERROR;
    
    for (size_t i = 0; i < len; i++) {
        /* Generate printable ASCII (32-126) plus some special chars */
        uint64_t choice = theft_random_choice(t, 100);
        if (choice < 90) {
            /* Regular printable ASCII */
            str[i] = (char)(32 + theft_random_choice(t, 95));
        } else if (choice < 93) {
            str[i] = '\n';
        } else if (choice < 96) {
            str[i] = '\t';
        } else if (choice < 98) {
            str[i] = '\\';
        } else {
            str[i] = '"';
        }
    }
    str[len] = '\0';
    
    *output = str;
    return THEFT_ALLOC_OK;
}

static void string_free(void *instance, void *env) {
    (void)env;
    free(instance);
}

static void string_print(FILE *f, const void *instance, void *env) {
    (void)env;
    const char *str = (const char *)instance;
    fprintf(f, "\"%s\" (len=%zu)", str, strlen(str));
}

static struct theft_type_info string_type_info = {
    .alloc = string_alloc,
    .free = string_free,
    .print = string_print,
};

/**
 * Property 28c: String round-trip
 * For any string, serialize_string then parse_string returns the original value
 */
static enum theft_trial_res prop_string_roundtrip(struct theft *t, void *arg1) {
    (void)t;
    const char *val = (const char *)arg1;
    
    /* Serialize */
    char *serialized = serialize_string(val);
    if (!serialized) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Parse back */
    Result_ParsedString_ParseError result = parse_string(serialized, NULL);
    free(serialized);
    
    if (!is_ok(result)) {
        return THEFT_TRIAL_FAIL;
    }
    
    char *parsed = unwrap_ok(result);
    int cmp = strcmp(parsed, val);
    free(parsed);
    
    if (cmp != 0) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 29: Invalid input returns error
 * For any malformed input string, deserialization SHALL return an Err Result.
 *============================================================================*/

static enum theft_trial_res prop_invalid_int_returns_error(struct theft *t, void *arg1) {
    (void)t;
    const char *input = (const char *)arg1;
    
    /* Skip inputs that might actually be valid integers */
    const char *p = input;
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n')) p++;
    if (*p == '-' || *p == '+') p++;
    if (*p >= '0' && *p <= '9') {
        return THEFT_TRIAL_SKIP;
    }
    
    /* Empty or whitespace-only strings should error */
    if (*p == '\0') {
        Result_int_ParseError result = parse_int(input, NULL);
        if (is_err(result)) {
            return THEFT_TRIAL_PASS;
        }
        return THEFT_TRIAL_FAIL;
    }
    
    /* Non-numeric strings should error */
    Result_int_ParseError result = parse_int(input, NULL);
    if (is_err(result)) {
        return THEFT_TRIAL_PASS;
    }
    
    return THEFT_TRIAL_FAIL;
}

static enum theft_trial_res prop_invalid_string_returns_error(struct theft *t, void *arg1) {
    (void)t;
    const char *input = (const char *)arg1;
    
    /* Skip inputs that start with a quote (might be valid) */
    const char *p = input;
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n')) p++;
    if (*p == '"') {
        return THEFT_TRIAL_SKIP;
    }
    
    /* Non-quoted strings should error */
    Result_ParsedString_ParseError result = parse_string(input, NULL);
    if (is_err(result)) {
        return THEFT_TRIAL_PASS;
    }
    
    return THEFT_TRIAL_FAIL;
}

/*============================================================================
 * Property 30: Pretty-print preserves parseability
 * For any serialized string, pretty-printing and then parsing SHALL produce
 * the same structure as parsing the original.
 *============================================================================*/

static enum theft_trial_res prop_pretty_print_int(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr % INT_MAX);
    
    /* Serialize */
    char *serialized = serialize_int(val);
    if (!serialized) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Pretty print */
    char *pretty = pretty_print(serialized, 2);
    if (!pretty) {
        free(serialized);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Parse original */
    Result_int_ParseError result1 = parse_int(serialized, NULL);
    free(serialized);
    
    /* Parse pretty-printed */
    Result_int_ParseError result2 = parse_int(pretty, NULL);
    free(pretty);
    
    if (!is_ok(result1) || !is_ok(result2)) {
        return THEFT_TRIAL_FAIL;
    }
    
    if (unwrap_ok(result1) != unwrap_ok(result2)) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

static enum theft_trial_res prop_pretty_print_double(struct theft *t, void *arg1) {
    (void)t;
    double *val_ptr = (double *)arg1;
    double val = *val_ptr;
    
    /* Skip special values */
    if (isnan(val) || isinf(val)) {
        return THEFT_TRIAL_SKIP;
    }
    
    /* Serialize */
    char *serialized = serialize_double(val);
    if (!serialized) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Pretty print */
    char *pretty = pretty_print(serialized, 2);
    if (!pretty) {
        free(serialized);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Parse original */
    Result_double_ParseError result1 = parse_double(serialized, NULL);
    free(serialized);
    
    /* Parse pretty-printed */
    Result_double_ParseError result2 = parse_double(pretty, NULL);
    free(pretty);
    
    if (!is_ok(result1) || !is_ok(result2)) {
        return THEFT_TRIAL_FAIL;
    }
    
    double v1 = unwrap_ok(result1);
    double v2 = unwrap_ok(result2);
    double diff = fabs(v1 - v2);
    double tolerance = fabs(v1) * 1e-15;
    if (tolerance < 1e-15) tolerance = 1e-15;
    
    if (diff > tolerance) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Test Registration
 *============================================================================*/

#define MIN_TEST_TRIALS 100

typedef struct {
    const char *name;
    theft_propfun1 *prop;
    const struct theft_type_info *type_info;
    enum theft_builtin_type_info builtin_type;
    bool use_builtin;
} SerializeTest;

static SerializeTest serialize_tests[] = {
    /* Property 28: Round-trip tests */
    {
        "Property 28a: Int round-trip",
        prop_int_roundtrip,
        NULL,
        THEFT_BUILTIN_int64_t,
        true
    },
    {
        "Property 28b: Double round-trip",
        prop_double_roundtrip,
        NULL,
        THEFT_BUILTIN_double,
        true
    },
    {
        "Property 28c: String round-trip",
        prop_string_roundtrip,
        &string_type_info,
        0,
        false
    },
    /* Property 29: Invalid input tests */
    {
        "Property 29a: Invalid int input returns error",
        prop_invalid_int_returns_error,
        &string_type_info,
        0,
        false
    },
    {
        "Property 29b: Invalid string input returns error",
        prop_invalid_string_returns_error,
        &string_type_info,
        0,
        false
    },
    /* Property 30: Pretty-print tests */
    {
        "Property 30a: Pretty-print int preserves parseability",
        prop_pretty_print_int,
        NULL,
        THEFT_BUILTIN_int64_t,
        true
    },
    {
        "Property 30b: Pretty-print double preserves parseability",
        prop_pretty_print_double,
        NULL,
        THEFT_BUILTIN_double,
        true
    },
};

#define NUM_SERIALIZE_TESTS (sizeof(serialize_tests) / sizeof(serialize_tests[0]))

int run_serialize_tests(theft_seed seed) {
    int failures = 0;
    
    printf("\nSerialization Tests:\n");
    
    for (size_t i = 0; i < NUM_SERIALIZE_TESTS; i++) {
        SerializeTest *test = &serialize_tests[i];
        
        const struct theft_type_info *type_info;
        if (test->use_builtin) {
            type_info = theft_get_builtin_type_info(test->builtin_type);
        } else {
            type_info = test->type_info;
        }
        
        struct theft_run_config config = {
            .name = test->name,
            .prop1 = test->prop,
            .type_info = { type_info },
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

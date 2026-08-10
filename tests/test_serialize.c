/**
 * @file test_serialize.c
 * @brief Property-based tests for serialization
 * 
 * Tests validate correctness properties:
 * - Property 28: Serialization round-trip
 * - Property 29: Invalid input returns error
 * - Property 30: Pretty-print preserves parseability
 * - Property 31: SExp serialize/parse round-trip
 * - Property 32: SExp parse structure and parse errors
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
 * Property 31: SExp serialize/parse round-trip
 * For any randomly built SExp tree (bounded depth and fanout, atoms covering
 * negative ints, non-integral doubles, nan/inf, strings with escapes, and
 * symbols), parse_sexp(serialize_sexp(tree)) is structurally equal to tree.
 *============================================================================*/

/* Build a random SExp tree; at max depth only atoms are generated */
static SExp *gen_random_sexp(struct theft *t, int depth) {
    uint64_t kind = theft_random_choice(t, depth >= 4 ? 4 : 5);
    switch (kind) {
        case 0:
            /* Integer atom, including negatives */
            return sexp_int((long)theft_random_choice(t, 2000001) - 1000000);
        case 1: {
            /* Double atom: non-integral so "%.17g" keeps a '.' and the text
             * parses back as SEXP_DOUBLE, plus the nan/inf specials */
            uint64_t pick = theft_random_choice(t, 20);
            if (pick == 0) return sexp_double(NAN);
            if (pick == 1) return sexp_double(INFINITY);
            if (pick == 2) return sexp_double(-INFINITY);
            long k = (long)theft_random_choice(t, 20001) - 10000;
            return sexp_double((double)k + 0.5);
        }
        case 2: {
            /* String atom with escape-worthy characters */
            static const char charset[] = "ab\"\\\n\tz 9";
            char buf[16];
            size_t n = theft_random_choice(t, 12);
            for (size_t i = 0; i < n; i++) {
                buf[i] = charset[theft_random_choice(t, sizeof(charset) - 1)];
            }
            buf[n] = '\0';
            return sexp_string(buf);
        }
        case 3: {
            /* Symbol atom ('s' prefix avoids the nan/inf keywords) */
            char buf[16];
            buf[0] = 's';
            size_t n = 1 + theft_random_choice(t, 8);
            for (size_t i = 1; i < n; i++) {
                buf[i] = (char)('a' + theft_random_choice(t, 26));
            }
            buf[n] = '\0';
            return sexp_symbol(buf);
        }
        default: {
            /* List with bounded fanout */
            SExp *list = sexp_list_new();
            size_t fanout = theft_random_choice(t, 5);
            for (size_t i = 0; i < fanout; i++) {
                sexp_list_push(list, gen_random_sexp(t, depth + 1));
            }
            return list;
        }
    }
}

static enum theft_trial_res prop_sexp_roundtrip(struct theft *t, void *arg1) {
    (void)arg1;

    SExp *tree = gen_random_sexp(t, 0);

    /* Serialize */
    char *text = serialize_sexp(tree);
    if (!text) {
        sexp_free(tree);
        return THEFT_TRIAL_FAIL;
    }

    /* Parse back */
    Result_SExpPtr_ParseError result = parse_sexp(text, NULL);
    free(text);

    if (!is_ok(result)) {
        sexp_free(tree);
        return THEFT_TRIAL_FAIL;
    }

    /* Structural equality (NaN compares equal to NaN in sexp_eq) */
    SExp *parsed = unwrap_ok(result);
    bool equal = sexp_eq(tree, parsed);
    sexp_free(parsed);
    sexp_free(tree);

    return equal ? THEFT_TRIAL_PASS : THEFT_TRIAL_FAIL;
}

/*============================================================================
 * Property 32: SExp parse structure and parse errors
 * Parsing "(1 2 (3 4) 5)" produces the documented tree shape, and malformed
 * inputs ("(1 2", ")", empty, NULL) produce Err results.
 *============================================================================*/

static enum theft_trial_res prop_sexp_parse_structure(struct theft *t, void *arg1) {
    (void)t;
    (void)arg1;

    /* Well-formed nested list */
    Result_SExpPtr_ParseError r = parse_sexp("(1 2 (3 4) 5)", NULL);
    if (!is_ok(r)) {
        return THEFT_TRIAL_FAIL;
    }
    SExp *e = unwrap_ok(r);

    /* Top level: list of 4 items */
    if (e->type != SEXP_LIST || e->list.len != 4) {
        sexp_free(e);
        return THEFT_TRIAL_FAIL;
    }

    /* Items 0, 1, 3 are the integers 1, 2, 5 */
    if (e->list.items[0]->type != SEXP_INT || e->list.items[0]->i != 1 ||
        e->list.items[1]->type != SEXP_INT || e->list.items[1]->i != 2 ||
        e->list.items[3]->type != SEXP_INT || e->list.items[3]->i != 5) {
        sexp_free(e);
        return THEFT_TRIAL_FAIL;
    }

    /* Item 2 is the nested list (3 4) */
    SExp *nested = e->list.items[2];
    if (nested->type != SEXP_LIST || nested->list.len != 2 ||
        nested->list.items[0]->type != SEXP_INT || nested->list.items[0]->i != 3 ||
        nested->list.items[1]->type != SEXP_INT || nested->list.items[1]->i != 4) {
        sexp_free(e);
        return THEFT_TRIAL_FAIL;
    }

    sexp_free(e);

    /* Unterminated list is an error */
    Result_SExpPtr_ParseError unterminated = parse_sexp("(1 2", NULL);
    if (!is_err(unterminated)) {
        return THEFT_TRIAL_FAIL;
    }

    /* A stray closing paren is an error */
    Result_SExpPtr_ParseError stray = parse_sexp(")", NULL);
    if (!is_err(stray)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Empty and NULL inputs are errors */
    Result_SExpPtr_ParseError empty = parse_sexp("", NULL);
    if (!is_err(empty)) {
        return THEFT_TRIAL_FAIL;
    }
    Result_SExpPtr_ParseError null_input = parse_sexp(NULL, NULL);
    if (!is_err(null_input)) {
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
    /* Property 31/32: SExp tests */
    {
        "Property 31: SExp serialize/parse round-trip",
        prop_sexp_roundtrip,
        NULL,
        THEFT_BUILTIN_int64_t,
        true
    },
    {
        "Property 32: SExp parse structure and parse errors",
        prop_sexp_parse_structure,
        NULL,
        THEFT_BUILTIN_int64_t,
        true
    },
};

#define NUM_SERIALIZE_TESTS (sizeof(serialize_tests) / sizeof(serialize_tests[0]))

int run_serialize_tests(theft_seed seed, int *num_tests) {
    int failures = 0;
    
    *num_tests = (int)NUM_SERIALIZE_TESTS;
    
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

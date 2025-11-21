/**
 * @file test_string.c
 * @brief Property-based tests for dynamic String type
 * 
 * Tests validate correctness properties:
 * - Property 46: String append preserves content
 * - Property 47: String format produces correct output
 * - Property 48: String slice matches substring
 * - Property 49: String cstr is null-terminated
 * - Property 50: String concat combines content
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theft.h"
#include <cyan/string.h>

/*============================================================================
 * String Generator for Property Testing
 *============================================================================*/

/* Generate printable ASCII strings for testing */
static enum theft_alloc_res string_gen_alloc(struct theft *t, void *env, void **output) {
    (void)env;
    
    /* Generate length 0-50 */
    size_t len = theft_random_choice(t, 51);
    
    char *str = malloc(len + 1);
    if (!str) return THEFT_ALLOC_ERROR;
    
    for (size_t i = 0; i < len; i++) {
        /* Generate printable ASCII (32-126) */
        str[i] = (char)(32 + theft_random_choice(t, 95));
    }
    str[len] = '\0';
    
    *output = str;
    return THEFT_ALLOC_OK;
}

static void string_gen_free(void *instance, void *env) {
    (void)env;
    free(instance);
}

static void string_gen_print(FILE *f, const void *instance, void *env) {
    (void)env;
    const char *str = (const char *)instance;
    fprintf(f, "\"%s\" (len=%zu)", str, strlen(str));
}

static struct theft_type_info string_gen_type_info = {
    .alloc = string_gen_alloc,
    .free = string_gen_free,
    .print = string_gen_print,
};

/*============================================================================
 * Property 46: String append preserves content
 * For any string and sequence of appends, the resulting string SHALL contain
 * all appended content in order, and length SHALL equal the sum of appended lengths.
 *============================================================================*/

static enum theft_trial_res prop_append_preserves_content(struct theft *t, void *arg1) {
    (void)t;
    const char *input = (const char *)arg1;
    
    /* Create a string and append the input */
    String s = string_new();
    string_append(&s, input);
    
    /* Verify length matches */
    size_t input_len = strlen(input);
    if (string_len(&s) != input_len) {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify content matches */
    if (strcmp(string_cstr(&s), input) != 0) {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test multiple appends */
    String s2 = string_from("Hello");
    size_t initial_len = string_len(&s2);
    string_append(&s2, input);
    
    /* Verify length is sum */
    if (string_len(&s2) != initial_len + input_len) {
        string_free(&s);
        string_free(&s2);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify content: should start with "Hello" and end with input */
    const char *cstr = string_cstr(&s2);
    if (strncmp(cstr, "Hello", 5) != 0) {
        string_free(&s);
        string_free(&s2);
        return THEFT_TRIAL_FAIL;
    }
    if (strcmp(cstr + 5, input) != 0) {
        string_free(&s);
        string_free(&s2);
        return THEFT_TRIAL_FAIL;
    }
    
    string_free(&s);
    string_free(&s2);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 47: String format produces correct output
 * For any format string and arguments, string_format SHALL produce output
 * equivalent to sprintf with automatic buffer management.
 *============================================================================*/

static enum theft_trial_res prop_format_correct_output(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr % 1000000);  /* Keep reasonable range */
    
    /* Format using string_format */
    String s = string_new();
    string_format(&s, "Value: %d", val);
    
    /* Format using sprintf for comparison */
    char expected[64];
    snprintf(expected, sizeof(expected), "Value: %d", val);
    
    /* Compare results */
    if (strcmp(string_cstr(&s), expected) != 0) {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test string_formatted */
    String s2 = string_formatted("Number %d is %s", val, val >= 0 ? "non-negative" : "negative");
    
    char expected2[128];
    snprintf(expected2, sizeof(expected2), "Number %d is %s", val, val >= 0 ? "non-negative" : "negative");
    
    if (strcmp(string_cstr(&s2), expected2) != 0) {
        string_free(&s);
        string_free(&s2);
        return THEFT_TRIAL_FAIL;
    }
    
    string_free(&s);
    string_free(&s2);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 48: String slice matches substring
 * For any string and valid range [start, end), the slice SHALL contain
 * characters from index start to end-1 of the original string.
 *============================================================================*/

static enum theft_trial_res prop_slice_matches_substring(struct theft *t, void *arg1) {
    (void)t;
    const char *input = (const char *)arg1;
    
    String s = string_from(input);
    size_t len = string_len(&s);
    
    /* Skip empty strings */
    if (len == 0) {
        string_free(&s);
        return THEFT_TRIAL_SKIP;
    }
    
    /* Generate random start and end within bounds */
    size_t start = theft_random_choice(t, len + 1);
    size_t end = start + theft_random_choice(t, len - start + 1);
    
    Slice_char slice = string_slice(&s, start, end);
    
    /* Verify slice length */
    if (slice.len != end - start) {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify slice content matches original substring */
    for (size_t i = 0; i < slice.len; i++) {
        if (slice.data[i] != input[start + i]) {
            string_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    string_free(&s);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 49: String cstr is null-terminated
 * For any string, string_cstr() SHALL return a pointer to a null-terminated
 * character array matching the string's content.
 *============================================================================*/

static enum theft_trial_res prop_cstr_null_terminated(struct theft *t, void *arg1) {
    (void)t;
    const char *input = (const char *)arg1;
    
    String s = string_from(input);
    const char *cstr = string_cstr(&s);
    
    /* Verify null termination at correct position */
    size_t len = string_len(&s);
    if (cstr[len] != '\0') {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify strlen matches string_len */
    if (strlen(cstr) != len) {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify content matches input */
    if (strcmp(cstr, input) != 0) {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test after modifications */
    string_push(&s, 'X');
    cstr = string_cstr(&s);
    len = string_len(&s);
    
    if (cstr[len] != '\0') {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    if (strlen(cstr) != len) {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    
    string_free(&s);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 50: String concat combines content
 * For any two strings A and B, string_concat(A, B) SHALL produce a string
 * containing A's content followed by B's content.
 *============================================================================*/

/* We need two string arguments for concat test */
static enum theft_trial_res prop_concat_combines_content(struct theft *t, void *arg1) {
    (void)t;
    const char *input_a = (const char *)arg1;
    
    /* Generate a second string for testing */
    size_t len_b = theft_random_choice(t, 31);
    char *input_b = malloc(len_b + 1);
    if (!input_b) return THEFT_TRIAL_ERROR;
    
    for (size_t i = 0; i < len_b; i++) {
        input_b[i] = (char)(32 + theft_random_choice(t, 95));
    }
    input_b[len_b] = '\0';
    
    String a = string_from(input_a);
    String b = string_from(input_b);
    
    String result = string_concat(&a, &b);
    
    /* Verify length is sum of both */
    size_t expected_len = string_len(&a) + string_len(&b);
    if (string_len(&result) != expected_len) {
        string_free(&a);
        string_free(&b);
        string_free(&result);
        free(input_b);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify content: first part matches a */
    const char *result_cstr = string_cstr(&result);
    if (strncmp(result_cstr, input_a, strlen(input_a)) != 0) {
        string_free(&a);
        string_free(&b);
        string_free(&result);
        free(input_b);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify content: second part matches b */
    if (strcmp(result_cstr + strlen(input_a), input_b) != 0) {
        string_free(&a);
        string_free(&b);
        string_free(&result);
        free(input_b);
        return THEFT_TRIAL_FAIL;
    }
    
    string_free(&a);
    string_free(&b);
    string_free(&result);
    free(input_b);
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
} StringTest;

static StringTest string_tests[] = {
    /* Property 46: Append preserves content */
    {
        "Property 46: String append preserves content",
        prop_append_preserves_content,
        &string_gen_type_info,
        0,
        false
    },
    /* Property 47: Format produces correct output */
    {
        "Property 47: String format produces correct output",
        prop_format_correct_output,
        NULL,
        THEFT_BUILTIN_int64_t,
        true
    },
    /* Property 48: Slice matches substring */
    {
        "Property 48: String slice matches substring",
        prop_slice_matches_substring,
        &string_gen_type_info,
        0,
        false
    },
    /* Property 49: cstr is null-terminated */
    {
        "Property 49: String cstr is null-terminated",
        prop_cstr_null_terminated,
        &string_gen_type_info,
        0,
        false
    },
    /* Property 50: Concat combines content */
    {
        "Property 50: String concat combines content",
        prop_concat_combines_content,
        &string_gen_type_info,
        0,
        false
    },
};

#define NUM_STRING_TESTS (sizeof(string_tests) / sizeof(string_tests[0]))

int run_string_tests(theft_seed seed) {
    int failures = 0;
    
    printf("\nString Tests:\n");
    
    for (size_t i = 0; i < NUM_STRING_TESTS; i++) {
        StringTest *test = &string_tests[i];
        
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

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
    
    /* Verify vt pointer is set after creation */
    if (s.vt == NULL) {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    
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
    
    /* Verify vt pointer is set after creation */
    if (s.vt == NULL) {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    
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
    
    /* Verify vt pointer is set after creation */
    if (s.vt == NULL) {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    
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
    
    /* Verify vt pointer is set after creation */
    if (s.vt == NULL) {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    
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
    
    /* Verify vt pointer is set after creation */
    if (a.vt == NULL || b.vt == NULL) {
        string_free(&a);
        string_free(&b);
        free(input_b);
        return THEFT_TRIAL_FAIL;
    }
    
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
 * Property 1 (vtable): Shared vtable instances (String)
 * For any two instances of String, their vtable pointers shall be equal
 * (point to the same address).
 *============================================================================*/

static enum theft_trial_res prop_shared_string_vtable(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    /* Create multiple String instances using different constructors */
    String s1 = string_new();
    String s2 = string_new();
    String s3 = string_from("Hello");
    String s4 = string_from("World");
    String s5 = string_with_capacity(10);
    String s6 = string_with_capacity((size_t)(val > 0 ? val % 100 : (-val) % 100 + 1));
    
    /* All vtable pointers should be non-null */
    if (s1.vt == NULL || s2.vt == NULL || s3.vt == NULL || 
        s4.vt == NULL || s5.vt == NULL || s6.vt == NULL) {
        string_free(&s1);
        string_free(&s2);
        string_free(&s3);
        string_free(&s4);
        string_free(&s5);
        string_free(&s6);
        return THEFT_TRIAL_FAIL;
    }
    
    /* All vtable pointers should point to the same address */
    if (s1.vt != s2.vt || s2.vt != s3.vt || s3.vt != s4.vt || 
        s4.vt != s5.vt || s5.vt != s6.vt) {
        string_free(&s1);
        string_free(&s2);
        string_free(&s3);
        string_free(&s4);
        string_free(&s5);
        string_free(&s6);
        return THEFT_TRIAL_FAIL;
    }
    
    string_free(&s1);
    string_free(&s2);
    string_free(&s3);
    string_free(&s4);
    string_free(&s5);
    string_free(&s6);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 5 (vtable): String vtable behavioral equivalence
 * For any String instance, any valid character, any valid C string, and any
 * valid index, calling operations through the vtable shall produce identical
 * results to calling the standalone functions.
 *============================================================================*/

static enum theft_trial_res prop_string_vtable_behavioral_equivalence(struct theft *t, void *arg1) {
    (void)t;
    const char *input = (const char *)arg1;
    
    /* Create two identical strings - one for vtable ops, one for standalone ops */
    String s_vtable = string_from("Initial");
    String s_standalone = string_from("Initial");
    
    /* Test push equivalence: vtable vs standalone */
    char c = 'X';
    s_vtable.vt->push(&s_vtable, c);
    string_push(&s_standalone, c);
    
    /* Test len equivalence */
    size_t len_vtable = s_vtable.vt->len(&s_vtable);
    size_t len_standalone = string_len(&s_standalone);
    
    if (len_vtable != len_standalone) {
        string_free(&s_vtable);
        string_free(&s_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test cstr equivalence */
    const char *cstr_vtable = s_vtable.vt->cstr(&s_vtable);
    const char *cstr_standalone = string_cstr(&s_standalone);
    
    if (strcmp(cstr_vtable, cstr_standalone) != 0) {
        string_free(&s_vtable);
        string_free(&s_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test append equivalence */
    s_vtable.vt->append(&s_vtable, input);
    string_append(&s_standalone, input);
    
    if (s_vtable.vt->len(&s_vtable) != string_len(&s_standalone)) {
        string_free(&s_vtable);
        string_free(&s_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    if (strcmp(s_vtable.vt->cstr(&s_vtable), string_cstr(&s_standalone)) != 0) {
        string_free(&s_vtable);
        string_free(&s_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test get equivalence for all indices */
    for (size_t i = 0; i < s_vtable.vt->len(&s_vtable); i++) {
        Option_char opt_vtable = s_vtable.vt->get(&s_vtable, i);
        Option_char opt_standalone = string_get(&s_standalone, i);
        
        if (is_some(opt_vtable) != is_some(opt_standalone)) {
            string_free(&s_vtable);
            string_free(&s_standalone);
            return THEFT_TRIAL_FAIL;
        }
        
        if (is_some(opt_vtable) && unwrap(opt_vtable) != unwrap(opt_standalone)) {
            string_free(&s_vtable);
            string_free(&s_standalone);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test get out-of-bounds equivalence */
    Option_char oob_vtable = s_vtable.vt->get(&s_vtable, len_vtable + 100);
    Option_char oob_standalone = string_get(&s_standalone, len_standalone + 100);
    
    if (is_none(oob_vtable) != is_none(oob_standalone)) {
        string_free(&s_vtable);
        string_free(&s_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test slice equivalence */
    size_t slice_start = 0;
    size_t slice_end = s_vtable.vt->len(&s_vtable) / 2;
    
    Slice_char slice_vtable = s_vtable.vt->slice(&s_vtable, slice_start, slice_end);
    Slice_char slice_standalone = string_slice(&s_standalone, slice_start, slice_end);
    
    if (slice_vtable.len != slice_standalone.len) {
        string_free(&s_vtable);
        string_free(&s_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    for (size_t i = 0; i < slice_vtable.len; i++) {
        if (slice_vtable.data[i] != slice_standalone.data[i]) {
            string_free(&s_vtable);
            string_free(&s_standalone);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test clear equivalence */
    s_vtable.vt->clear(&s_vtable);
    string_clear(&s_standalone);
    
    if (s_vtable.vt->len(&s_vtable) != string_len(&s_standalone)) {
        string_free(&s_vtable);
        string_free(&s_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    if (s_vtable.vt->len(&s_vtable) != 0) {
        string_free(&s_vtable);
        string_free(&s_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Cleanup using vtable free for one, standalone for other */
    s_vtable.vt->free(&s_vtable);
    string_free(&s_standalone);
    
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
    /* Property 1 (vtable): Shared vtable instances (String) */
    {
        "Property 1 (vtable): Shared vtable instances (String)",
        prop_shared_string_vtable,
        NULL,
        THEFT_BUILTIN_int64_t,
        true
    },
    /* Property 5 (vtable): String vtable behavioral equivalence */
    {
        "Property 5 (vtable): String vtable behavioral equivalence",
        prop_string_vtable_behavioral_equivalence,
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

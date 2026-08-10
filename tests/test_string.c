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
 * - Property 51: find/contains/starts_with/ends_with match naive search
 * - Property 52: trim is idempotent and strips edge whitespace
 * - Property 53: string_eq is reflexive and matches strcmp semantics
 * - Property 54: string_split_next reconstructs the input
 * - Property 55: String macro == function behavioral equivalence
 * - Property 56: string_from_slice materializes split pieces correctly
 * - Property 57: string_slice_eq agrees with materialize + strcmp
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
 * Property 51: find/contains/starts_with/ends_with match naive search
 * For any string and any substring of it (or a needle guaranteed absent),
 * string_find/string_contains/string_starts_with/string_ends_with agree
 * with naive reference implementations.
 *============================================================================*/

/* Naive reference: first index of needle in haystack, or -1 */
static long _naive_find(const char *haystack, const char *needle) {
    size_t h_len = strlen(haystack);
    size_t n_len = strlen(needle);
    if (n_len == 0) return 0;
    if (n_len > h_len) return -1;
    for (size_t i = 0; i + n_len <= h_len; i++) {
        if (memcmp(haystack + i, needle, n_len) == 0) {
            return (long)i;
        }
    }
    return -1;
}

static enum theft_trial_res prop_search_matches_naive(struct theft *t, void *arg1) {
    const char *input = (const char *)arg1;
    size_t len = strlen(input);

    String s = string_from(input);

    /* Pick a random substring of the input as the needle */
    char needle[64];
    size_t start = len > 0 ? theft_random_choice(t, len) : 0;
    size_t max_n = len - start;
    if (max_n > 63) max_n = 63;
    size_t n_len = max_n > 0 ? theft_random_choice(t, max_n + 1) : 0;
    memcpy(needle, input + start, n_len);
    needle[n_len] = '\0';

    /* find agrees with the naive reference */
    long expected = _naive_find(input, needle);
    Option_size_t found = string_find(&s, needle);
    if (expected < 0) {
        if (is_some(found)) {
            string_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    } else {
        if (!is_some(found) || unwrap(found) != (size_t)expected) {
            string_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* contains agrees with find */
    if (string_contains(&s, needle) != (expected >= 0)) {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    /* A needle containing a character absent from the input is never found */
    char absent[4] = { '\x01', 'z', 'q', '\0' };
    if (_naive_find(input, absent) < 0 && string_contains(&s, absent)) {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    /* starts_with agrees with strncmp on every prefix length */
    for (size_t p = 0; p <= len && p <= 8; p++) {
        char prefix[16];
        memcpy(prefix, input, p);
        prefix[p] = '\0';
        if (!string_starts_with(&s, prefix)) {
            string_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* ends_with agrees with the tail of the input */
    for (size_t p = 0; p <= len && p <= 8; p++) {
        const char *suffix = input + (len - p);
        if (!string_ends_with(&s, suffix)) {
            string_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* A strictly longer needle can be neither prefix nor suffix */
    char longer[128];
    snprintf(longer, sizeof(longer), "%.100sX", input);
    if (string_starts_with(&s, longer) || string_ends_with(&s, longer)) {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    string_free(&s);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 52: trim is idempotent and strips edge whitespace
 * After string_trim, the string has no leading or trailing whitespace and
 * trimming again is a no-op.
 *============================================================================*/

static enum theft_trial_res prop_trim(struct theft *t, void *arg1) {
    const char *input = (const char *)arg1;

    /* Surround the input with random whitespace */
    char padded[128];
    size_t lead = theft_random_choice(t, 4);
    size_t trail = theft_random_choice(t, 4);
    size_t pos = 0;
    for (size_t i = 0; i < lead; i++) padded[pos++] = (i % 2) ? ' ' : '\t';
    size_t in_len = strlen(input);
    if (in_len > 100) in_len = 100;
    memcpy(padded + pos, input, in_len);
    pos += in_len;
    for (size_t i = 0; i < trail; i++) padded[pos++] = (i % 2) ? ' ' : '\n';
    padded[pos] = '\0';

    String s = string_from(padded);
    string_trim(&s);

    /* No leading or trailing whitespace remains */
    size_t len = string_len(&s);
    if (len > 0) {
        const char *cstr = string_cstr(&s);
        if (isspace((unsigned char)cstr[0]) || isspace((unsigned char)cstr[len - 1])) {
            string_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Trimming is idempotent */
    String before = string_from(string_cstr(&s));
    string_trim(&s);
    if (!string_eq(&s, &before)) {
        string_free(&before);
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    string_free(&before);
    string_free(&s);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 53: string_eq is reflexive and matches strcmp semantics
 * A string equals itself and a copy of itself; equality agrees with strcmp
 * on the underlying C strings; appending a character breaks equality.
 *============================================================================*/

static enum theft_trial_res prop_string_eq(struct theft *t, void *arg1) {
    (void)t;
    const char *input = (const char *)arg1;

    String a = string_from(input);
    String b = string_from(input);

    /* Reflexive */
    if (!string_eq(&a, &a)) {
        string_free(&a);
        string_free(&b);
        return THEFT_TRIAL_FAIL;
    }

    /* Content-equal copies compare equal, matching strcmp */
    bool cmp_equal = strcmp(string_cstr(&a), string_cstr(&b)) == 0;
    if (string_eq(&a, &b) != cmp_equal) {
        string_free(&a);
        string_free(&b);
        return THEFT_TRIAL_FAIL;
    }

    /* Appending a character breaks equality */
    string_push(&b, '!');
    if (string_eq(&a, &b)) {
        string_free(&a);
        string_free(&b);
        return THEFT_TRIAL_FAIL;
    }
    if (string_eq(&a, &b) != (strcmp(string_cstr(&a), string_cstr(&b)) == 0)) {
        string_free(&a);
        string_free(&b);
        return THEFT_TRIAL_FAIL;
    }

    /* NULL is tolerated and treated as empty */
    String empty = string_new();
    if (!string_eq(NULL, NULL) || !string_eq(&empty, NULL)) {
        string_free(&a);
        string_free(&b);
        string_free(&empty);
        return THEFT_TRIAL_FAIL;
    }

    string_free(&a);
    string_free(&b);
    string_free(&empty);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 54: string_split_next reconstructs the input
 * Joining the pieces produced by string_split_next with the delimiter
 * yields the original string; "a,b," yields the 3 pieces "a", "b", "".
 *============================================================================*/

static enum theft_trial_res prop_split_reconstructs(struct theft *t, void *arg1) {
    (void)t;
    const char *input = (const char *)arg1;
    char delim = ',';

    String s = string_from(input);

    /* Collect pieces and rebuild the input */
    String rebuilt = string_new();
    Slice_char rest = string_as_slice(&s);
    Slice_char part;
    bool first = true;
    size_t piece_count = 0;

    while (string_split_next(&rest, delim, &part)) {
        if (!first) {
            string_push(&rebuilt, delim);
        }
        for (size_t i = 0; i < part.len; i++) {
            /* Pieces must never contain the delimiter */
            if (part.data[i] == delim) {
                string_free(&rebuilt);
                string_free(&s);
                return THEFT_TRIAL_FAIL;
            }
            string_push(&rebuilt, part.data[i]);
        }
        first = false;
        piece_count++;
    }

    /* Joining the pieces with the delimiter reconstructs the input */
    if (!string_eq(&rebuilt, &s)) {
        string_free(&rebuilt);
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    /* Any input (even empty) yields delimiter-count + 1 pieces, because
     * string_from always provides a non-NULL buffer */
    size_t delim_count = 0;
    for (const char *p = input; *p; p++) {
        if (*p == delim) delim_count++;
    }
    size_t expected_pieces = delim_count + 1;
    if (piece_count != expected_pieces) {
        string_free(&rebuilt);
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    string_free(&rebuilt);
    string_free(&s);

    /* Deterministic check: "a,b," yields "a", "b", "" */
    String abc = string_from("a,b,");
    Slice_char abc_rest = string_as_slice(&abc);
    Slice_char abc_part;
    const char *expected[3] = { "a", "b", "" };
    size_t idx = 0;
    while (string_split_next(&abc_rest, ',', &abc_part)) {
        if (idx >= 3) {
            string_free(&abc);
            return THEFT_TRIAL_FAIL;
        }
        if (abc_part.len != strlen(expected[idx]) ||
            (abc_part.len > 0 && memcmp(abc_part.data, expected[idx], abc_part.len) != 0)) {
            string_free(&abc);
            return THEFT_TRIAL_FAIL;
        }
        idx++;
    }
    if (idx != 3) {
        string_free(&abc);
        return THEFT_TRIAL_FAIL;
    }

    string_free(&abc);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 55: String macro == function behavioral equivalence
 * For any String instance, the STR_* macros (String is monomorphic, so no
 * type argument) produce identical results to calling the standalone
 * string_* functions.
 *============================================================================*/

static enum theft_trial_res prop_string_macro_fn_equivalence(struct theft *t, void *arg1) {
    (void)t;
    const char *input = (const char *)arg1;

    /* Create two identical strings - one for macro ops, one for standalone ops */
    String s_macro = string_from("Initial");
    String s_fn = string_from("Initial");

    /* Test push equivalence: macro vs standalone */
    char c = 'X';
    STR_PUSH(s_macro, c);
    string_push(&s_fn, c);

    /* Test len equivalence */
    if (STR_LEN(s_macro) != string_len(&s_fn)) {
        string_free(&s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test cstr equivalence */
    if (strcmp(STR_CSTR(s_macro), string_cstr(&s_fn)) != 0) {
        string_free(&s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test append equivalence */
    STR_APPEND(s_macro, input);
    string_append(&s_fn, input);

    if (STR_LEN(s_macro) != string_len(&s_fn)) {
        string_free(&s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    if (strcmp(STR_CSTR(s_macro), string_cstr(&s_fn)) != 0) {
        string_free(&s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test get equivalence for all indices (plus out-of-bounds) */
    for (size_t i = 0; i <= STR_LEN(s_macro); i++) {
        Option_char opt_macro = STR_GET(s_macro, i);
        Option_char opt_fn = string_get(&s_fn, i);

        if (is_some(opt_macro) != is_some(opt_fn)) {
            string_free(&s_macro);
            string_free(&s_fn);
            return THEFT_TRIAL_FAIL;
        }

        if (is_some(opt_macro) && unwrap(opt_macro) != unwrap(opt_fn)) {
            string_free(&s_macro);
            string_free(&s_fn);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Test find/contains equivalence */
    Option_size_t find_macro = STR_FIND(s_macro, "Init");
    Option_size_t find_fn = string_find(&s_fn, "Init");

    if (is_some(find_macro) != is_some(find_fn)) {
        string_free(&s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }
    if (is_some(find_macro) && unwrap(find_macro) != unwrap(find_fn)) {
        string_free(&s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    if (STR_CONTAINS(s_macro, "tial") != string_contains(&s_fn, "tial")) {
        string_free(&s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test slice equivalence */
    Slice_char slice_macro = STR_SLICE(s_macro, 0, STR_LEN(s_macro) / 2);
    Slice_char slice_fn = string_slice(&s_fn, 0, string_len(&s_fn) / 2);

    if (slice_macro.len != slice_fn.len) {
        string_free(&s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    for (size_t i = 0; i < slice_macro.len; i++) {
        if (slice_macro.data[i] != slice_fn.data[i]) {
            string_free(&s_macro);
            string_free(&s_fn);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Test clear equivalence */
    STR_CLEAR(s_macro);
    string_clear(&s_fn);

    if (STR_LEN(s_macro) != string_len(&s_fn) || STR_LEN(s_macro) != 0) {
        string_free(&s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Cleanup using STR_FREE for macro, standalone for other */
    STR_FREE(s_macro);
    string_free(&s_fn);

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 56: string_from_slice materializes split pieces correctly
 * For any string split on a random delimiter, each piece materialized via
 * string_from_slice has the piece's exact content and length and is
 * null-terminated. An empty or NULL-data slice yields an empty String.
 *============================================================================*/

static enum theft_trial_res prop_from_slice_materializes_pieces(struct theft *t, void *arg1) {
    const char *input = (const char *)arg1;

    /* Pick a random printable delimiter */
    char delim = (char)(32 + theft_random_choice(t, 95));

    String s = string_from(input);
    size_t total_len = string_len(&s);

    Slice_char rest = string_as_slice(&s);
    Slice_char part;
    size_t pieces_len_sum = 0;
    size_t piece_count = 0;

    while (string_split_next(&rest, delim, &part)) {
        String piece = string_from_slice(part);

        /* Length matches the slice */
        if (string_len(&piece) != part.len) {
            string_free(&piece);
            string_free(&s);
            return THEFT_TRIAL_FAIL;
        }

        /* Content matches the slice bytes */
        const char *cstr = string_cstr(&piece);
        if (part.len > 0 && memcmp(cstr, part.data, part.len) != 0) {
            string_free(&piece);
            string_free(&s);
            return THEFT_TRIAL_FAIL;
        }

        /* Null-terminated at exactly len (pieces contain no delimiter and
         * the generator produces no embedded NUL, so strlen must agree) */
        if (cstr[part.len] != '\0' || strlen(cstr) != part.len) {
            string_free(&piece);
            string_free(&s);
            return THEFT_TRIAL_FAIL;
        }

        pieces_len_sum += part.len;
        piece_count++;
        string_free(&piece);
    }

    /* Pieces plus delimiters account for the whole input */
    if (total_len > 0 || piece_count > 0) {
        if (pieces_len_sum + (piece_count - 1) != total_len) {
            string_free(&s);
            return THEFT_TRIAL_FAIL;
        }
    }

    string_free(&s);

    /* An empty slice yields an empty String */
    Slice_char empty_slice = { .data = NULL, .len = 0 };
    String e1 = string_from_slice(empty_slice);
    if (string_len(&e1) != 0 || strcmp(string_cstr(&e1), "") != 0) {
        string_free(&e1);
        return THEFT_TRIAL_FAIL;
    }
    string_free(&e1);

    /* A zero-length slice with non-NULL data also yields an empty String */
    char dummy = 'x';
    Slice_char zero_slice = { .data = &dummy, .len = 0 };
    String e2 = string_from_slice(zero_slice);
    if (string_len(&e2) != 0 || strcmp(string_cstr(&e2), "") != 0) {
        string_free(&e2);
        return THEFT_TRIAL_FAIL;
    }
    string_free(&e2);

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 57: string_slice_eq agrees with materialize + strcmp
 * For any random slice of a random string compared against a random C
 * string, string_slice_eq agrees with materializing the slice and using
 * strcmp. NULL cstr never matches; a length mismatch never matches.
 *============================================================================*/

static enum theft_trial_res prop_slice_eq_matches_strcmp(struct theft *t, void *arg1) {
    const char *input = (const char *)arg1;

    String s = string_from(input);
    size_t len = string_len(&s);

    /* Take a random sub-slice */
    size_t start = theft_random_choice(t, len + 1);
    size_t end = start + theft_random_choice(t, len - start + 1);
    Slice_char slice = string_slice(&s, start, end);

    /* The slice always equals its own materialization */
    String mat = string_from_slice(slice);
    if (!string_slice_eq(slice, string_cstr(&mat))) {
        string_free(&mat);
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    /* Compare against a random C string: must agree with strcmp on the
     * materialized copy */
    char other[64];
    size_t other_len = theft_random_choice(t, 8);
    for (size_t i = 0; i < other_len; i++) {
        /* Small alphabet so equal and unequal cases both occur */
        other[i] = (char)('a' + theft_random_choice(t, 4));
    }
    other[other_len] = '\0';

    bool expected = strcmp(string_cstr(&mat), other) == 0;
    if (string_slice_eq(slice, other) != expected) {
        string_free(&mat);
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    /* NULL cstr never matches, even for an empty slice */
    if (string_slice_eq(slice, NULL)) {
        string_free(&mat);
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    Slice_char empty_slice = { .data = NULL, .len = 0 };
    if (string_slice_eq(empty_slice, NULL)) {
        string_free(&mat);
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    /* Empty-vs-empty matches */
    if (!string_slice_eq(empty_slice, "")) {
        string_free(&mat);
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    /* A length mismatch never matches: extend the materialized copy */
    String longer = string_from(string_cstr(&mat));
    string_push(&longer, '!');
    if (string_slice_eq(slice, string_cstr(&longer))) {
        string_free(&longer);
        string_free(&mat);
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }
    /* And a truncated copy (when possible) never matches either */
    if (slice.len > 0) {
        String shorter = string_from_slice(slice);
        shorter.len--;
        shorter.data[shorter.len] = '\0';
        if (string_slice_eq(slice, string_cstr(&shorter))) {
            string_free(&shorter);
            string_free(&longer);
            string_free(&mat);
            string_free(&s);
            return THEFT_TRIAL_FAIL;
        }
        string_free(&shorter);
    }

    string_free(&longer);
    string_free(&mat);
    string_free(&s);
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
    /* Property 51: Search matches naive reference */
    {
        "Property 51: find/contains/starts_with/ends_with match naive search",
        prop_search_matches_naive,
        &string_gen_type_info,
        0,
        false
    },
    /* Property 52: Trim is idempotent */
    {
        "Property 52: trim is idempotent and strips edge whitespace",
        prop_trim,
        &string_gen_type_info,
        0,
        false
    },
    /* Property 53: string_eq semantics */
    {
        "Property 53: string_eq is reflexive and matches strcmp semantics",
        prop_string_eq,
        &string_gen_type_info,
        0,
        false
    },
    /* Property 54: split_next reconstruction */
    {
        "Property 54: string_split_next reconstructs the input",
        prop_split_reconstructs,
        &string_gen_type_info,
        0,
        false
    },
    /* Property 55: Macro == function equivalence */
    {
        "Property 55: String macro == function behavioral equivalence",
        prop_string_macro_fn_equivalence,
        &string_gen_type_info,
        0,
        false
    },
    /* Property 56: string_from_slice materializes pieces */
    {
        "Property 56: string_from_slice materializes split pieces correctly",
        prop_from_slice_materializes_pieces,
        &string_gen_type_info,
        0,
        false
    },
    /* Property 57: string_slice_eq matches strcmp */
    {
        "Property 57: string_slice_eq agrees with materialize + strcmp",
        prop_slice_eq_matches_strcmp,
        &string_gen_type_info,
        0,
        false
    },
};

#define NUM_STRING_TESTS (sizeof(string_tests) / sizeof(string_tests[0]))

int run_string_tests(theft_seed seed, int *num_tests) {
    int failures = 0;
    
    *num_tests = (int)NUM_STRING_TESTS;
    
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

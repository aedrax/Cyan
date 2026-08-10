/**
 * @file test_regressions.c
 * @brief Deterministic regression tests for previously observed bugs
 *
 * Tests validate correctness properties:
 * - Property 1: match_option break exits the user's loop
 * - Property 2: String self-append operations are safe
 * - Property 3: HashMap custom hash_fn/equal_fn survive the first insert
 * - Property 4: pretty_print survives deeply nested input
 * - Property 5: Non-threadsafe channel never blocks
 * - Property 6: Coroutine yield with and without values
 * - Property 7: Option macros evaluate their argument exactly once
 *
 * Each check is deterministic (or seeded by the generated value) and wrapped
 * as a theft property so failures reproduce with the reported seed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theft.h"
#include <cyan/channel.h>   /* CHANNEL_DEFINE(int) below provides Option_int */
#include <cyan/match.h>
#include <cyan/string.h>
#include <cyan/hashmap.h>
#include <cyan/serialize.h>
#include <cyan/coro.h>

/* Define test types (CHANNEL_DEFINE also defines Option_int) */
CHANNEL_DEFINE(int);
HASHMAP_DEFINE(int, int);

/*============================================================================
 * Property 1: match_option break exits the user's loop
 * A `break` inside a match_option branch targets the USER's enclosing for
 * loop (the macro must not swallow it with a do-while(0) wrapper). Looping
 * 0..4 and breaking at val == 2 must process exactly 3 iterations.
 *============================================================================*/

static enum theft_trial_res prop_match_break_exits_loop(struct theft *t, void *arg1) {
    (void)t;
    (void)arg1;

    int processed = 0;

    for (int i = 0; i < 5; i++) {
        Option_int opt = Some(int, i);
        match_option(opt, int, val,
            {
                processed++;
                if (val == 2) {
                    break;  /* Must exit the for loop, not just the branch */
                }
            },
            {
                /* Unreachable: every option is Some */
            }
        );
    }

    /* Iterations 0, 1, and 2 are processed; 3 and 4 are skipped */
    if (processed != 3) {
        return THEFT_TRIAL_FAIL;
    }

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 2: String self-append operations are safe
 * string_append_str(&s, &s) doubles the content, string_append with the
 * string's own cstr works, and string_format with a self-referencing "%s"
 * argument works (no stale pointers after reallocation).
 *============================================================================*/

static enum theft_trial_res prop_string_self_append(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr % 10000);

    char base[24];
    snprintf(base, sizeof(base), "v%d", val);

    String s = string_from(base);

    /* string_append_str(&s, &s) doubles the content */
    string_append_str(&s, &s);

    char expect_doubled[64];
    snprintf(expect_doubled, sizeof(expect_doubled), "%s%s", base, base);
    if (strcmp(string_cstr(&s), expect_doubled) != 0 ||
        string_len(&s) != strlen(expect_doubled)) {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    /* string_append with the string's own buffer doubles it again */
    string_append(&s, string_cstr(&s));

    char expect_quadrupled[128];
    snprintf(expect_quadrupled, sizeof(expect_quadrupled), "%s%s",
             expect_doubled, expect_doubled);
    if (strcmp(string_cstr(&s), expect_quadrupled) != 0 ||
        string_len(&s) != strlen(expect_quadrupled)) {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    /* string_format with a self-referencing argument appends a copy */
    string_format(&s, " %s", string_cstr(&s));

    char expect_formatted[280];
    snprintf(expect_formatted, sizeof(expect_formatted), "%s %s",
             expect_quadrupled, expect_quadrupled);
    if (strcmp(string_cstr(&s), expect_formatted) != 0 ||
        string_len(&s) != strlen(expect_formatted)) {
        string_free(&s);
        return THEFT_TRIAL_FAIL;
    }

    string_free(&s);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 3: HashMap custom hash_fn/equal_fn survive the first insert
 * Functions assigned before the first insert must still be assigned (and
 * actually used) afterwards, and inserting the same key 100 times keeps
 * len == 1 without triggering a resize.
 *============================================================================*/

static size_t g_custom_hash_calls = 0;
static size_t g_custom_equal_calls = 0;

static size_t custom_hash(const void *key, size_t key_size) {
    g_custom_hash_calls++;
    return _cyan_fnv1a_hash(key, key_size);
}

static bool custom_equal(const void *a, const void *b, size_t size) {
    g_custom_equal_calls++;
    return memcmp(a, b, size) == 0;
}

static enum theft_trial_res prop_hashmap_custom_fns(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int key = (int)(*val_ptr % 100000);

    g_custom_hash_calls = 0;
    g_custom_equal_calls = 0;

    HashMap_int_int m = hashmap_int_int_new();

    /* Assign the custom functions BEFORE the first insert */
    m.hash_fn = custom_hash;
    m.equal_fn = custom_equal;

    hashmap_int_int_insert(&m, key, 1);

    /* The custom functions must survive the lazy bucket initialization */
    if (m.hash_fn != custom_hash || m.equal_fn != custom_equal) {
        hashmap_int_int_free(&m);
        return THEFT_TRIAL_FAIL;
    }

    /* And they must actually have been used */
    if (g_custom_hash_calls == 0) {
        hashmap_int_int_free(&m);
        return THEFT_TRIAL_FAIL;
    }

    /* Inserting the same key 100 times keeps len == 1 and never resizes */
    size_t capacity_before = m.capacity;
    for (int i = 0; i < 100; i++) {
        hashmap_int_int_insert(&m, key, i);
    }

    if (hashmap_int_int_len(&m) != 1) {
        hashmap_int_int_free(&m);
        return THEFT_TRIAL_FAIL;
    }
    if (m.capacity != capacity_before) {
        hashmap_int_int_free(&m);
        return THEFT_TRIAL_FAIL;
    }

    /* The last inserted value wins */
    Option_int got = hashmap_int_int_get(&m, key);
    if (!is_some(got) || unwrap(got) != 99) {
        hashmap_int_int_free(&m);
        return THEFT_TRIAL_FAIL;
    }

    hashmap_int_int_free(&m);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 4: pretty_print survives deeply nested input
 * A 2000-deep nested "((((...))))" input must not crash or overflow the
 * output buffer, and must return a non-NULL result.
 *============================================================================*/

static enum theft_trial_res prop_pretty_print_deep_nesting(struct theft *t, void *arg1) {
    (void)t;
    (void)arg1;

    const size_t depth = 2000;
    char *input = (char *)malloc(2 * depth + 1);
    if (!input) {
        return THEFT_TRIAL_SKIP;
    }
    memset(input, '(', depth);
    memset(input + depth, ')', depth);
    input[2 * depth] = '\0';

    char *pretty = pretty_print(input, 2);
    free(input);

    if (!pretty) {
        return THEFT_TRIAL_FAIL;
    }
    free(pretty);

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 5: Non-threadsafe channel never blocks
 * Without CYAN_CHANNEL_THREADSAFE, a send to a full buffered channel must
 * return CHAN_WOULD_BLOCK and a recv from an empty channel must return
 * None immediately (a single-threaded wait could never be satisfied).
 *============================================================================*/

static enum theft_trial_res prop_channel_never_blocks(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);

    /* Fill a small buffered channel */
    Channel_int *ch = chan_int_new(2);
    if (!ch) {
        return THEFT_TRIAL_ERROR;
    }

    if (chan_int_send(ch, val) != CHAN_OK || chan_int_send(ch, val + 1) != CHAN_OK) {
        chan_int_free(ch);
        return THEFT_TRIAL_FAIL;
    }

    /* Send to the full channel returns CHAN_WOULD_BLOCK instead of hanging */
    if (chan_int_send(ch, val + 2) != CHAN_WOULD_BLOCK) {
        chan_int_free(ch);
        return THEFT_TRIAL_FAIL;
    }
    chan_int_free(ch);

    /* Recv from an empty channel returns None instead of hanging */
    Channel_int *empty = chan_int_new(2);
    if (!empty) {
        return THEFT_TRIAL_ERROR;
    }
    Option_int got = chan_int_recv(empty);
    if (is_some(got)) {
        chan_int_free(empty);
        return THEFT_TRIAL_FAIL;
    }
    chan_int_free(empty);

    /* An unbuffered channel cannot rendezvous single-threaded either */
    Channel_int *unbuffered = chan_int_new(0);
    if (!unbuffered) {
        return THEFT_TRIAL_ERROR;
    }
    if (chan_int_send(unbuffered, val) != CHAN_WOULD_BLOCK) {
        chan_int_free(unbuffered);
        return THEFT_TRIAL_FAIL;
    }
    chan_int_free(unbuffered);

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6: Coroutine yield with and without values
 * After coro_yield() (no value), coro_has_yield() must be false; a yielded
 * struct larger than the 64-byte internal buffer must round-trip intact
 * through coro_get_yield.
 *============================================================================*/

/* Larger than the Coro's 64-byte inline yield buffer */
typedef struct {
    char data[80];
    int tail;
} BigYield;

static void yielding_coro_fn(Coro *self, void *arg) {
    int seed = *(int *)arg;

    /* First suspension carries no value */
    coro_yield(self);

    /* Second suspension yields a > 64-byte struct */
    BigYield big;
    for (int i = 0; i < (int)sizeof(big.data); i++) {
        big.data[i] = (char)('a' + ((seed + i) % 26));
    }
    big.tail = seed;
    coro_yield_value(self, big);
}

static enum theft_trial_res prop_coro_yield_values(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int seed = (int)(*val_ptr % 10000);
    if (seed < 0) seed = -seed;

    Coro *c = coro_new(yielding_coro_fn, &seed, 0);
    if (!c) {
        return THEFT_TRIAL_ERROR;
    }

    /* First resume: coroutine yields without a value */
    if (!coro_resume(c)) {
        coro_free(c);
        return THEFT_TRIAL_FAIL;
    }
    if (coro_has_yield(c)) {
        coro_free(c);
        return THEFT_TRIAL_FAIL;
    }

    /* Second resume: coroutine yields the big struct */
    if (!coro_resume(c)) {
        coro_free(c);
        return THEFT_TRIAL_FAIL;
    }
    if (!coro_has_yield(c)) {
        coro_free(c);
        return THEFT_TRIAL_FAIL;
    }

    BigYield got = coro_get_yield(c, BigYield);
    for (int i = 0; i < (int)sizeof(got.data); i++) {
        if (got.data[i] != (char)('a' + ((seed + i) % 26))) {
            coro_free(c);
            return THEFT_TRIAL_FAIL;
        }
    }
    if (got.tail != seed) {
        coro_free(c);
        return THEFT_TRIAL_FAIL;
    }

    /* Third resume: coroutine runs to completion */
    if (coro_resume(c) || !coro_is_finished(c)) {
        coro_free(c);
        return THEFT_TRIAL_FAIL;
    }

    coro_free(c);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 7: Option macros evaluate their argument exactly once
 * unwrap/unwrap_or/expect on a function call with a side effect must call
 * the function exactly once (statement-expression single evaluation).
 *============================================================================*/

static int g_side_effect_calls = 0;

static Option_int counted_some(int v) {
    g_side_effect_calls++;
    return Some(int, v);
}

static enum theft_trial_res prop_single_evaluation(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr % 100000);

    /* unwrap evaluates its argument exactly once */
    g_side_effect_calls = 0;
    int got = unwrap(counted_some(val));
    if (g_side_effect_calls != 1 || got != val) {
        return THEFT_TRIAL_FAIL;
    }

    /* unwrap_or evaluates its argument exactly once */
    g_side_effect_calls = 0;
    got = unwrap_or(counted_some(val), val + 1);
    if (g_side_effect_calls != 1 || got != val) {
        return THEFT_TRIAL_FAIL;
    }

    /* expect evaluates its argument exactly once */
    g_side_effect_calls = 0;
    got = expect(counted_some(val), "expect on Some must not panic");
    if (g_side_effect_calls != 1 || got != val) {
        return THEFT_TRIAL_FAIL;
    }

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
} RegressionTest;

static RegressionTest regression_tests[] = {
    {
        "Property 1: match_option break exits the user's loop",
        prop_match_break_exits_loop,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 2: String self-append operations are safe",
        prop_string_self_append,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 3: HashMap custom hash_fn/equal_fn survive the first insert",
        prop_hashmap_custom_fns,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 4: pretty_print survives deeply nested input",
        prop_pretty_print_deep_nesting,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 5: Non-threadsafe channel never blocks",
        prop_channel_never_blocks,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 6: Coroutine yield with and without values",
        prop_coro_yield_values,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 7: Option macros evaluate their argument exactly once",
        prop_single_evaluation,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_REGRESSION_TESTS (sizeof(regression_tests) / sizeof(regression_tests[0]))

int run_regression_tests(theft_seed seed, int *num_tests) {
    int failures = 0;

    *num_tests = (int)NUM_REGRESSION_TESTS;

    printf("\nRegression Tests:\n");

    for (size_t i = 0; i < NUM_REGRESSION_TESTS; i++) {
        RegressionTest *test = &regression_tests[i];

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

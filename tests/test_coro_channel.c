/**
 * @file test_coro_channel.c
 * @brief Property-based tests for coroutine-aware channels (coro.h + channel.h)
 *
 * Tests validate correctness properties:
 * - Property 71: unbuffered producer/consumer delivers all values in order
 * - Property 72: buffered producer/consumer delivers all values in order
 * - Property 73: two producers, one consumer: every value arrives, sum matches
 * - Property 74: coro_run detects deadlock; close unblocks and finishes
 * - Property 75: outside coroutines single-threaded behavior is unchanged
 *
 * All coroutine machinery (coros, channels, coro_run) lives in this one
 * translation unit, as required by the header-only statics. coro.h is
 * included before channel.h so channel operations cooperate with coro_run.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theft.h"
#include <cyan/coro.h>
#include <cyan/channel.h>

CHANNEL_DEFINE(int);

/*============================================================================
 * Shared state for coroutine bodies
 *============================================================================
 * Channels and buffers must be reachable from the coroutine functions, so
 * they live in file-scope statics that each trial resets.
 */

#define CC_MAX_VALUES 128

static Channel_int *g_chan;

/* Values each producer sends (two slots so two producers can coexist) */
static int g_send_vals[2][CC_MAX_VALUES];
static size_t g_send_n[2];
static bool g_send_failed;
static bool g_producer_closes; /* whether the producer closes the channel */

/* What the consumer received */
static int g_recv_vals[2 * CC_MAX_VALUES];
static size_t g_recv_n;
static size_t g_recv_expected; /* used by the counting consumer */
static bool g_recv_failed;

/* Deadlock-scenario observations */
static bool g_blocked_recv_got_value;
static ChanStatus g_blocked_send_status;
static bool g_saw_current_self;

static void _reset_shared_state(void) {
    g_chan = NULL;
    g_send_n[0] = g_send_n[1] = 0;
    g_send_failed = false;
    g_producer_closes = false;
    g_recv_n = 0;
    g_recv_expected = 0;
    g_recv_failed = false;
    g_blocked_recv_got_value = false;
    g_blocked_send_status = CHAN_OK;
    g_saw_current_self = false;
}

/* Producer: sends its slot's values; optionally closes the channel after */
static void _producer_fn(Coro *self, void *arg) {
    size_t slot = (size_t)(uintptr_t)arg;
    if (coro_current() != self) {
        g_saw_current_self = true; /* misuse marker (checked as failure) */
    }
    for (size_t i = 0; i < g_send_n[slot]; i++) {
        if (chan_int_send(g_chan, g_send_vals[slot][i]) != CHAN_OK) {
            g_send_failed = true;
            return;
        }
    }
    if (g_producer_closes) {
        chan_int_close(g_chan);
    }
}

/* Consumer: receives until the channel is closed and drained */
static void _consumer_drain_fn(Coro *self, void *arg) {
    (void)self;
    (void)arg;
    for (;;) {
        Option_int v = chan_int_recv(g_chan);
        if (!v.has_value) break;
        if (g_recv_n >= sizeof(g_recv_vals) / sizeof(g_recv_vals[0])) {
            g_recv_failed = true;
            return;
        }
        g_recv_vals[g_recv_n++] = v.value;
    }
}

/* Consumer: receives exactly g_recv_expected values */
static void _consumer_count_fn(Coro *self, void *arg) {
    (void)self;
    (void)arg;
    for (size_t i = 0; i < g_recv_expected; i++) {
        Option_int v = chan_int_recv(g_chan);
        if (!v.has_value) {
            g_recv_failed = true;
            return;
        }
        g_recv_vals[g_recv_n++] = v.value;
    }
}

/* Coroutine that blocks on recv from a channel nobody sends to */
static void _blocked_recv_fn(Coro *self, void *arg) {
    (void)self;
    (void)arg;
    Option_int v = chan_int_recv(g_chan);
    g_blocked_recv_got_value = v.has_value;
}

/* Coroutine that blocks on an unbuffered send nobody receives */
static void _blocked_send_fn(Coro *self, void *arg) {
    (void)self;
    (void)arg;
    g_blocked_send_status = chan_int_send(g_chan, 42);
}

/*============================================================================
 * Common driver for the single-producer/single-consumer properties
 *============================================================================*/

static enum theft_trial_res _run_producer_consumer(struct theft *t, int seed,
                                                   size_t capacity) {
    _reset_shared_state();

    size_t n = theft_random_choice(t, CC_MAX_VALUES + 1);
    g_send_n[0] = n;
    for (size_t i = 0; i < n; i++) {
        g_send_vals[0][i] = seed + (int)i * 3 + (int)theft_random_choice(t, 7);
    }
    g_producer_closes = true;

    g_chan = chan_int_new(capacity);
    if (!g_chan) return THEFT_TRIAL_ERROR;

    /* A NULL entry in the array must be skipped by coro_run */
    Coro *coros[3] = {
        coro_new(_producer_fn, (void *)(uintptr_t)0, 0),
        NULL,
        coro_new(_consumer_drain_fn, NULL, 0),
    };
    if (!coros[0] || !coros[2]) return THEFT_TRIAL_ERROR;

    bool ok = coro_run(coros, 3);

    enum theft_trial_res res = THEFT_TRIAL_PASS;

    /* The whole group must finish without deadlock or send errors */
    if (!ok || g_send_failed || g_recv_failed || g_saw_current_self) {
        res = THEFT_TRIAL_FAIL;
    }
    if (!coro_is_finished(coros[0]) || !coro_is_finished(coros[2])) {
        res = THEFT_TRIAL_FAIL;
    }

    /* Every value was received, in send order */
    if (g_recv_n != n) {
        res = THEFT_TRIAL_FAIL;
    } else {
        for (size_t i = 0; i < n; i++) {
            if (g_recv_vals[i] != g_send_vals[0][i]) {
                res = THEFT_TRIAL_FAIL;
                break;
            }
        }
    }

    /* Channel ends up closed and drained */
    if (!chan_int_is_closed(g_chan) || chan_int_recv(g_chan).has_value) {
        res = THEFT_TRIAL_FAIL;
    }

    coro_free(coros[0]);
    coro_free(coros[2]);
    chan_int_free(g_chan);
    g_chan = NULL;
    return res;
}

/*============================================================================
 * Property 71: unbuffered producer/consumer delivers all values in order
 * N random values sent over an unbuffered (rendezvous) channel by a
 * producer coroutine are all received in order by a consumer coroutine,
 * and coro_run returns true.
 *============================================================================*/

static enum theft_trial_res prop_unbuffered_producer_consumer(struct theft *t, void *arg1) {
    int64_t *val_ptr = (int64_t *)arg1;
    int seed = (int)(*val_ptr % 100000);
    return _run_producer_consumer(t, seed, 0);
}

/*============================================================================
 * Property 72: buffered producer/consumer delivers all values in order
 * Same as Property 71 over a buffered channel with random capacity 1..8.
 *============================================================================*/

static enum theft_trial_res prop_buffered_producer_consumer(struct theft *t, void *arg1) {
    int64_t *val_ptr = (int64_t *)arg1;
    int seed = (int)(*val_ptr % 100000);
    size_t capacity = 1 + theft_random_choice(t, 8);
    return _run_producer_consumer(t, seed, capacity);
}

/*============================================================================
 * Property 73: two producers, one consumer: every value arrives, sum matches
 * With two producer coroutines and one consumer receiving exactly the
 * total count, coro_run finishes the whole group and the received values
 * are exactly the union of both producers' values (checked via sum and
 * per-producer subsequence order).
 *============================================================================*/

static enum theft_trial_res prop_two_producers_one_consumer(struct theft *t, void *arg1) {
    int64_t *val_ptr = (int64_t *)arg1;
    int seed = (int)(*val_ptr % 100000);

    _reset_shared_state();

    size_t n0 = theft_random_choice(t, 40);
    size_t n1 = theft_random_choice(t, 40);
    g_send_n[0] = n0;
    g_send_n[1] = n1;

    long expected_sum = 0;
    for (size_t i = 0; i < n0; i++) {
        /* Producer 0 sends even values, producer 1 odd values, so every
         * received value is attributable to exactly one producer */
        g_send_vals[0][i] = (seed + (int)i) * 2;
        expected_sum += g_send_vals[0][i];
    }
    for (size_t i = 0; i < n1; i++) {
        g_send_vals[1][i] = (seed + (int)i) * 2 + 1;
        expected_sum += g_send_vals[1][i];
    }
    g_recv_expected = n0 + n1;
    g_producer_closes = false; /* consumer counts instead of draining */

    size_t capacity = theft_random_choice(t, 5); /* 0..4, incl. unbuffered */
    g_chan = chan_int_new(capacity);
    if (!g_chan) return THEFT_TRIAL_ERROR;

    Coro *coros[3] = {
        coro_new(_producer_fn, (void *)(uintptr_t)0, 0),
        coro_new(_producer_fn, (void *)(uintptr_t)1, 0),
        coro_new(_consumer_count_fn, NULL, 0),
    };
    if (!coros[0] || !coros[1] || !coros[2]) return THEFT_TRIAL_ERROR;

    bool ok = coro_run(coros, 3);

    enum theft_trial_res res = THEFT_TRIAL_PASS;

    if (!ok || g_send_failed || g_recv_failed || g_saw_current_self) {
        res = THEFT_TRIAL_FAIL;
    }
    for (size_t i = 0; i < 3; i++) {
        if (!coro_is_finished(coros[i])) res = THEFT_TRIAL_FAIL;
    }

    /* All values received (any interleaving) and the sum matches */
    if (g_recv_n != n0 + n1) {
        res = THEFT_TRIAL_FAIL;
    } else {
        long sum = 0;
        size_t next0 = 0;
        size_t next1 = 0;
        bool order_ok = true;
        for (size_t i = 0; i < g_recv_n; i++) {
            sum += g_recv_vals[i];
            /* Each producer's values must arrive as an in-order
             * subsequence of the interleaving */
            if (g_recv_vals[i] % 2 == 0) {
                if (next0 >= n0 || g_recv_vals[i] != g_send_vals[0][next0]) {
                    order_ok = false;
                    break;
                }
                next0++;
            } else {
                if (next1 >= n1 || g_recv_vals[i] != g_send_vals[1][next1]) {
                    order_ok = false;
                    break;
                }
                next1++;
            }
        }
        if (!order_ok || sum != expected_sum || next0 != n0 || next1 != n1) {
            res = THEFT_TRIAL_FAIL;
        }
    }

    coro_free(coros[0]);
    coro_free(coros[1]);
    coro_free(coros[2]);
    chan_int_free(g_chan);
    g_chan = NULL;
    return res;
}

/*============================================================================
 * Property 74: coro_run detects deadlock; close unblocks and finishes
 * A single coroutine blocked on recv from a channel nobody sends to makes
 * coro_run return false; after chan close, rerunning coro_run returns
 * true and the coroutine finishes (recv observes None). The same holds
 * for a coroutine blocked on an unbuffered send (send observes
 * CHAN_CLOSED).
 *============================================================================*/

static enum theft_trial_res prop_deadlock_detection_and_recovery(struct theft *t, void *arg1) {
    (void)arg1;

    /* Scenario A: blocked receiver */
    _reset_shared_state();
    size_t capacity = theft_random_choice(t, 3); /* 0..2: recv blocks anyway */
    g_chan = chan_int_new(capacity);
    if (!g_chan) return THEFT_TRIAL_ERROR;

    Coro *recv_coro = coro_new(_blocked_recv_fn, NULL, 0);
    if (!recv_coro) return THEFT_TRIAL_ERROR;
    Coro *recv_group[1] = { recv_coro };

    /* Nobody ever sends: this must be reported as a deadlock */
    if (coro_run(recv_group, 1)) {
        coro_free(recv_coro);
        chan_int_free(g_chan);
        return THEFT_TRIAL_FAIL;
    }
    if (coro_is_finished(recv_coro)) {
        coro_free(recv_coro);
        chan_int_free(g_chan);
        return THEFT_TRIAL_FAIL;
    }

    /* Closing the channel unblocks the receiver */
    chan_int_close(g_chan);
    if (!coro_run(recv_group, 1) || !coro_is_finished(recv_coro)) {
        coro_free(recv_coro);
        chan_int_free(g_chan);
        return THEFT_TRIAL_FAIL;
    }
    if (g_blocked_recv_got_value) {
        /* recv on a closed, empty channel must yield None */
        coro_free(recv_coro);
        chan_int_free(g_chan);
        return THEFT_TRIAL_FAIL;
    }
    coro_free(recv_coro);
    chan_int_free(g_chan);

    /* Scenario B: blocked unbuffered sender */
    _reset_shared_state();
    g_chan = chan_int_new(0);
    if (!g_chan) return THEFT_TRIAL_ERROR;

    Coro *send_coro = coro_new(_blocked_send_fn, NULL, 0);
    if (!send_coro) return THEFT_TRIAL_ERROR;
    Coro *send_group[1] = { send_coro };

    /* Nobody ever receives: rendezvous can't complete -> deadlock */
    if (coro_run(send_group, 1)) {
        coro_free(send_coro);
        chan_int_free(g_chan);
        return THEFT_TRIAL_FAIL;
    }
    if (coro_is_finished(send_coro)) {
        coro_free(send_coro);
        chan_int_free(g_chan);
        return THEFT_TRIAL_FAIL;
    }

    chan_int_close(g_chan);
    if (!coro_run(send_group, 1) || !coro_is_finished(send_coro)) {
        coro_free(send_coro);
        chan_int_free(g_chan);
        return THEFT_TRIAL_FAIL;
    }
    if (g_blocked_send_status != CHAN_CLOSED) {
        coro_free(send_coro);
        chan_int_free(g_chan);
        return THEFT_TRIAL_FAIL;
    }
    coro_free(send_coro);
    chan_int_free(g_chan);
    g_chan = NULL;

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 75: outside coroutines single-threaded behavior is unchanged
 * At top level coro_current() is NULL; send to a full (or unbuffered)
 * channel returns CHAN_WOULD_BLOCK instead of yielding; recv from an
 * empty channel returns None; try_send/try_recv behave the same way; a
 * closed channel reports CHAN_CLOSED.
 *============================================================================*/

static enum theft_trial_res prop_toplevel_behavior_unchanged(struct theft *t, void *arg1) {
    int64_t *val_ptr = (int64_t *)arg1;
    int seed = (int)(*val_ptr % 100000);

    /* Not inside any coroutine at top level */
    if (coro_current() != NULL) {
        return THEFT_TRIAL_FAIL;
    }

    /* Unbuffered channel: send and recv would block forever, so they
     * return immediately */
    Channel_int *unbuf = chan_int_new(0);
    if (!unbuf) return THEFT_TRIAL_ERROR;
    if (chan_int_send(unbuf, seed) != CHAN_WOULD_BLOCK) {
        chan_int_free(unbuf);
        return THEFT_TRIAL_FAIL;
    }
    if (chan_int_recv(unbuf).has_value) {
        chan_int_free(unbuf);
        return THEFT_TRIAL_FAIL;
    }
    if (chan_int_try_send(unbuf, seed) != CHAN_WOULD_BLOCK ||
        chan_int_try_recv(unbuf).has_value) {
        chan_int_free(unbuf);
        return THEFT_TRIAL_FAIL;
    }
    chan_int_free(unbuf);

    /* Buffered channel: fills to capacity, then WOULD_BLOCK; drains in
     * FIFO order, then None */
    size_t capacity = 1 + theft_random_choice(t, 8);
    Channel_int *ch = chan_int_new(capacity);
    if (!ch) return THEFT_TRIAL_ERROR;

    for (size_t i = 0; i < capacity; i++) {
        if (chan_int_send(ch, seed + (int)i) != CHAN_OK) {
            chan_int_free(ch);
            return THEFT_TRIAL_FAIL;
        }
    }
    if (chan_int_send(ch, seed - 1) != CHAN_WOULD_BLOCK ||
        chan_int_try_send(ch, seed - 1) != CHAN_WOULD_BLOCK) {
        chan_int_free(ch);
        return THEFT_TRIAL_FAIL;
    }

    for (size_t i = 0; i < capacity; i++) {
        Option_int v = chan_int_recv(ch);
        if (!v.has_value || v.value != seed + (int)i) {
            chan_int_free(ch);
            return THEFT_TRIAL_FAIL;
        }
    }
    if (chan_int_recv(ch).has_value || chan_int_try_recv(ch).has_value) {
        chan_int_free(ch);
        return THEFT_TRIAL_FAIL;
    }

    /* Closed channel reports CHAN_CLOSED for sends, None for recv */
    chan_int_close(ch);
    if (chan_int_send(ch, seed) != CHAN_CLOSED ||
        chan_int_try_send(ch, seed) != CHAN_CLOSED ||
        chan_int_recv(ch).has_value) {
        chan_int_free(ch);
        return THEFT_TRIAL_FAIL;
    }
    chan_int_free(ch);

    /* Still not inside a coroutine */
    if (coro_current() != NULL) {
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
} CoroChannelTest;

static CoroChannelTest coro_channel_tests[] = {
    {
        "Property 71: unbuffered producer/consumer delivers all values in order",
        prop_unbuffered_producer_consumer,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 72: buffered producer/consumer delivers all values in order",
        prop_buffered_producer_consumer,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 73: two producers, one consumer: every value arrives, sum matches",
        prop_two_producers_one_consumer,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 74: coro_run detects deadlock; close unblocks and finishes",
        prop_deadlock_detection_and_recovery,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 75: outside coroutines single-threaded behavior is unchanged",
        prop_toplevel_behavior_unchanged,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_CORO_CHANNEL_TESTS \
    (sizeof(coro_channel_tests) / sizeof(coro_channel_tests[0]))

int run_coro_channel_tests(theft_seed seed, int *num_tests) {
    int failures = 0;

    *num_tests = (int)NUM_CORO_CHANNEL_TESTS;

    printf("\nCoroutine Channel Tests:\n");

    for (size_t i = 0; i < NUM_CORO_CHANNEL_TESTS; i++) {
        CoroChannelTest *test = &coro_channel_tests[i];

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

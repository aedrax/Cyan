/**
 * @file test_channel.c
 * @brief Property-based tests for Channel type
 * 
 * Tests validate correctness properties:
 * - Property 54: Channel send-recv round-trip
 * - Property 55: FIFO ordering
 * - Property 56: Closed channel recv returns None
 * - Property 57: Closed channel drains buffer first
 * - Property 58: Send to closed channel returns error
 * - Property 59: try_send to closed channel returns error
 * - Property 60: Channel macro == function behavioral equivalence
 * - Property 61: NULL channel operations are safe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theft.h"
#include <cyan/channel.h>

/* Define Channel type for testing */
CHANNEL_DEFINE(int);

/*============================================================================
 * Property 54: Channel send-recv round-trip
 * For any channel and value, sending then receiving SHALL return the sent value in FIFO order
 *============================================================================*/

static enum theft_trial_res prop_send_recv_roundtrip(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    /* Create a buffered channel */
    Channel_int *ch = chan_int_new(10);
    if (!ch) {
        return THEFT_TRIAL_ERROR;
    }
    
    /* Send the value */
    ChanStatus status = chan_int_send(ch, val);
    if (status != CHAN_OK) {
        chan_int_free(ch);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Receive the value */
    Option_int result = chan_int_recv(ch);
    if (!is_some(result)) {
        chan_int_free(ch);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify round-trip */
    int received = unwrap(result);
    if (received != val) {
        chan_int_free(ch);
        return THEFT_TRIAL_FAIL;
    }
    
    chan_int_free(ch);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 55: FIFO ordering
 * Multiple values sent should be received in FIFO order
 *============================================================================*/

static enum theft_trial_res prop_fifo_ordering(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int base_val = (int)(*val_ptr);
    
    /* Create a buffered channel */
    Channel_int *ch = chan_int_new(10);
    if (!ch) {
        return THEFT_TRIAL_ERROR;
    }
    
    /* Send multiple values */
    int values[5];
    for (int i = 0; i < 5; i++) {
        values[i] = base_val + i;
        ChanStatus status = chan_int_send(ch, values[i]);
        if (status != CHAN_OK) {
            chan_int_free(ch);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Receive and verify FIFO order */
    for (int i = 0; i < 5; i++) {
        Option_int result = chan_int_recv(ch);
        if (!is_some(result)) {
            chan_int_free(ch);
            return THEFT_TRIAL_FAIL;
        }
        
        int received = unwrap(result);
        if (received != values[i]) {
            chan_int_free(ch);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    chan_int_free(ch);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 56: Closed channel recv returns None
 * For any closed channel with an empty buffer, recv SHALL return None
 *============================================================================*/

static enum theft_trial_res prop_closed_recv_none(struct theft *t, void *arg1) {
    (void)t;
    (void)arg1;
    
    /* Create a buffered channel */
    Channel_int *ch = chan_int_new(10);
    if (!ch) {
        return THEFT_TRIAL_ERROR;
    }
    
    /* Close the channel without sending anything */
    chan_int_close(ch);
    
    /* Receive should return None */
    Option_int result = chan_int_recv(ch);
    if (is_some(result)) {
        chan_int_free(ch);
        return THEFT_TRIAL_FAIL;
    }
    
    chan_int_free(ch);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 57: Closed channel drains buffer first
 * Closed channel should return buffered values before returning None
 *============================================================================*/

static enum theft_trial_res prop_closed_drains_buffer(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    /* Create a buffered channel */
    Channel_int *ch = chan_int_new(10);
    if (!ch) {
        return THEFT_TRIAL_ERROR;
    }
    
    /* Send a value */
    ChanStatus status = chan_int_send(ch, val);
    if (status != CHAN_OK) {
        chan_int_free(ch);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Close the channel */
    chan_int_close(ch);
    
    /* First recv should return the buffered value */
    Option_int result1 = chan_int_recv(ch);
    if (!is_some(result1) || unwrap(result1) != val) {
        chan_int_free(ch);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Second recv should return None (buffer drained) */
    Option_int result2 = chan_int_recv(ch);
    if (is_some(result2)) {
        chan_int_free(ch);
        return THEFT_TRIAL_FAIL;
    }
    
    chan_int_free(ch);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 58: Send to closed channel returns error
 * For any closed channel, send SHALL return CHAN_CLOSED status
 *============================================================================*/

static enum theft_trial_res prop_send_closed_error(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    /* Create a buffered channel */
    Channel_int *ch = chan_int_new(10);
    if (!ch) {
        return THEFT_TRIAL_ERROR;
    }
    
    /* Close the channel */
    chan_int_close(ch);
    
    /* Send should return CHAN_CLOSED */
    ChanStatus status = chan_int_send(ch, val);
    if (status != CHAN_CLOSED) {
        chan_int_free(ch);
        return THEFT_TRIAL_FAIL;
    }
    
    chan_int_free(ch);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 59: try_send to closed channel returns error
 * For any closed channel, try_send SHALL also return CHAN_CLOSED status
 *============================================================================*/

static enum theft_trial_res prop_try_send_closed_error(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    /* Create a buffered channel */
    Channel_int *ch = chan_int_new(10);
    if (!ch) {
        return THEFT_TRIAL_ERROR;
    }
    
    /* Close the channel */
    chan_int_close(ch);
    
    /* try_send should return CHAN_CLOSED */
    ChanStatus status = chan_int_try_send(ch, val);
    if (status != CHAN_CLOSED) {
        chan_int_free(ch);
        return THEFT_TRIAL_FAIL;
    }
    
    chan_int_free(ch);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 60: Channel macro == function behavioral equivalence
 * For any Channel_T instance, the type-first CHAN_* macros produce identical
 * results to calling the standalone chan_T_* functions.
 *============================================================================*/

static enum theft_trial_res prop_channel_macro_fn_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);

    /* Create two channels - one for macro ops, one for standalone ops */
    Channel_int *ch_macro = chan_int_new(10);
    Channel_int *ch_fn = chan_int_new(10);

    if (!ch_macro || !ch_fn) {
        if (ch_macro) chan_int_free(ch_macro);
        if (ch_fn) chan_int_free(ch_fn);
        return THEFT_TRIAL_ERROR;
    }

    /* Test send via macro vs standalone */
    ChanStatus status_macro = CHAN_SEND(int, ch_macro, val);
    ChanStatus status_fn = chan_int_send(ch_fn, val);

    if (status_macro != status_fn) {
        chan_int_free(ch_macro);
        chan_int_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Send more values */
    CHAN_SEND(int, ch_macro, val + 1);
    chan_int_send(ch_fn, val + 1);

    CHAN_SEND(int, ch_macro, val + 2);
    chan_int_send(ch_fn, val + 2);

    /* Test recv via macro vs standalone */
    Option_int recv_macro = CHAN_RECV(int, ch_macro);
    Option_int recv_fn = chan_int_recv(ch_fn);

    if (is_some(recv_macro) != is_some(recv_fn)) {
        chan_int_free(ch_macro);
        chan_int_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    if (is_some(recv_macro) && unwrap(recv_macro) != unwrap(recv_fn)) {
        chan_int_free(ch_macro);
        chan_int_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test try_send via macro vs standalone */
    ChanStatus try_send_macro = CHAN_TRY_SEND(int, ch_macro, val + 10);
    ChanStatus try_send_fn = chan_int_try_send(ch_fn, val + 10);

    if (try_send_macro != try_send_fn) {
        chan_int_free(ch_macro);
        chan_int_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test try_recv via macro vs standalone */
    Option_int try_recv_macro = CHAN_TRY_RECV(int, ch_macro);
    Option_int try_recv_fn = chan_int_try_recv(ch_fn);

    if (is_some(try_recv_macro) != is_some(try_recv_fn)) {
        chan_int_free(ch_macro);
        chan_int_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    if (is_some(try_recv_macro) && unwrap(try_recv_macro) != unwrap(try_recv_fn)) {
        chan_int_free(ch_macro);
        chan_int_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test is_closed via macro vs standalone */
    if (CHAN_IS_CLOSED(int, ch_macro) != chan_int_is_closed(ch_fn)) {
        chan_int_free(ch_macro);
        chan_int_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test close via macro vs standalone */
    CHAN_CLOSE(int, ch_macro);
    chan_int_close(ch_fn);

    /* Verify both are now closed */
    if (CHAN_IS_CLOSED(int, ch_macro) != chan_int_is_closed(ch_fn)) {
        chan_int_free(ch_macro);
        chan_int_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test send to closed channel via macro vs standalone */
    ChanStatus closed_send_macro = CHAN_SEND(int, ch_macro, val);
    ChanStatus closed_send_fn = chan_int_send(ch_fn, val);

    if (closed_send_macro != closed_send_fn) {
        chan_int_free(ch_macro);
        chan_int_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Cleanup using CHAN_FREE for macro, standalone for other */
    CHAN_FREE(int, ch_macro);
    chan_int_free(ch_fn);

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 61: NULL channel operations are safe
 * The channel functions (and therefore the CHAN_* macros) tolerate NULL:
 * send reports CHAN_CLOSED, recv yields None, is_closed reports true, and
 * close/free are no-ops.
 *============================================================================*/

static enum theft_trial_res prop_null_channel_safety(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);

    Channel_int *null_ch = NULL;

    /* Send to a NULL channel reports CHAN_CLOSED */
    if (CHAN_SEND(int, null_ch, val) != CHAN_CLOSED) {
        return THEFT_TRIAL_FAIL;
    }
    if (CHAN_TRY_SEND(int, null_ch, val) != CHAN_CLOSED) {
        return THEFT_TRIAL_FAIL;
    }

    /* Recv from a NULL channel yields None */
    Option_int recv_res = CHAN_RECV(int, null_ch);
    if (is_some(recv_res)) {
        return THEFT_TRIAL_FAIL;
    }
    Option_int try_recv_res = CHAN_TRY_RECV(int, null_ch);
    if (is_some(try_recv_res)) {
        return THEFT_TRIAL_FAIL;
    }

    /* A NULL channel reports closed */
    if (!CHAN_IS_CLOSED(int, null_ch)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Close and free are no-ops on NULL */
    CHAN_CLOSE(int, null_ch);
    CHAN_FREE(int, null_ch);

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
} ChannelTest;

static ChannelTest channel_tests[] = {
    {
        "Property 54: Channel send-recv round-trip",
        prop_send_recv_roundtrip,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 55: FIFO ordering",
        prop_fifo_ordering,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 56: Closed channel recv returns None",
        prop_closed_recv_none,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 57: Closed channel drains buffer first",
        prop_closed_drains_buffer,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 58: Send to closed channel returns error",
        prop_send_closed_error,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 59: try_send to closed channel returns error",
        prop_try_send_closed_error,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 60: Channel macro == function behavioral equivalence",
        prop_channel_macro_fn_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 61: NULL channel operations are safe",
        prop_null_channel_safety,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_CHANNEL_TESTS (sizeof(channel_tests) / sizeof(channel_tests[0]))

int run_channel_tests(theft_seed seed, int *num_tests) {
    int failures = 0;
    
    *num_tests = (int)NUM_CHANNEL_TESTS;
    
    printf("\nChannel Type Tests:\n");
    
    for (size_t i = 0; i < NUM_CHANNEL_TESTS; i++) {
        ChannelTest *test = &channel_tests[i];
        
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

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
 * - Property 1 (vtable): Shared vtable instances (Channel)
 * - Property 11 (vtable): Channel vtable behavioral equivalence
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
 * Property 1 (vtable): Shared vtable instances (Channel)
 * For any two Channel_T instances, their vtable pointers shall be equal
 * (point to the same address).
 *============================================================================*/

static enum theft_trial_res prop_channel_shared_vtable(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    /* Use absolute value and constrain to reasonable capacity 1-10 */
    int64_t abs_val = (*val_ptr < 0) ? -(*val_ptr) : *val_ptr;
    size_t capacity = (size_t)((abs_val % 10) + 1);
    
    /* Create two channels with potentially different capacities */
    Channel_int *ch1 = chan_int_new(capacity);
    Channel_int *ch2 = chan_int_new(capacity + 5);
    
    if (!ch1 || !ch2) {
        if (ch1) chan_int_free(ch1);
        if (ch2) chan_int_free(ch2);
        return THEFT_TRIAL_ERROR;
    }
    
    /* Verify both channels share the same vtable */
    if (ch1->vt != ch2->vt) {
        chan_int_free(ch1);
        chan_int_free(ch2);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify vtable is not NULL */
    if (ch1->vt == NULL) {
        chan_int_free(ch1);
        chan_int_free(ch2);
        return THEFT_TRIAL_FAIL;
    }
    
    chan_int_free(ch1);
    chan_int_free(ch2);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 11 (vtable): Channel vtable behavioral equivalence
 * For any Channel_T instance, calling operations through the vtable
 * (ch->vt->send, ch->vt->recv, ch->vt->close) shall produce identical
 * results to calling the standalone functions (chan_T_send, chan_T_recv,
 * chan_T_close).
 *============================================================================*/

static enum theft_trial_res prop_channel_vtable_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    /* Create two channels - one for vtable ops, one for standalone ops */
    Channel_int *ch_vtable = chan_int_new(10);
    Channel_int *ch_standalone = chan_int_new(10);
    
    if (!ch_vtable || !ch_standalone) {
        if (ch_vtable) chan_int_free(ch_vtable);
        if (ch_standalone) chan_int_free(ch_standalone);
        return THEFT_TRIAL_ERROR;
    }
    
    /* Test send via vtable vs standalone */
    ChanStatus status_vtable = ch_vtable->vt->chan_send(ch_vtable, val);
    ChanStatus status_standalone = chan_int_send(ch_standalone, val);
    
    if (status_vtable != status_standalone) {
        chan_int_free(ch_vtable);
        chan_int_free(ch_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Send more values */
    ch_vtable->vt->chan_send(ch_vtable, val + 1);
    chan_int_send(ch_standalone, val + 1);
    
    ch_vtable->vt->chan_send(ch_vtable, val + 2);
    chan_int_send(ch_standalone, val + 2);
    
    /* Test recv via vtable vs standalone */
    Option_int recv_vtable = ch_vtable->vt->chan_recv(ch_vtable);
    Option_int recv_standalone = chan_int_recv(ch_standalone);
    
    if (is_some(recv_vtable) != is_some(recv_standalone)) {
        chan_int_free(ch_vtable);
        chan_int_free(ch_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    if (is_some(recv_vtable) && unwrap(recv_vtable) != unwrap(recv_standalone)) {
        chan_int_free(ch_vtable);
        chan_int_free(ch_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test try_send via vtable vs standalone */
    ChanStatus try_send_vtable = ch_vtable->vt->chan_try_send(ch_vtable, val + 10);
    ChanStatus try_send_standalone = chan_int_try_send(ch_standalone, val + 10);
    
    if (try_send_vtable != try_send_standalone) {
        chan_int_free(ch_vtable);
        chan_int_free(ch_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test try_recv via vtable vs standalone */
    Option_int try_recv_vtable = ch_vtable->vt->chan_try_recv(ch_vtable);
    Option_int try_recv_standalone = chan_int_try_recv(ch_standalone);
    
    if (is_some(try_recv_vtable) != is_some(try_recv_standalone)) {
        chan_int_free(ch_vtable);
        chan_int_free(ch_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    if (is_some(try_recv_vtable) && unwrap(try_recv_vtable) != unwrap(try_recv_standalone)) {
        chan_int_free(ch_vtable);
        chan_int_free(ch_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test is_closed via vtable vs standalone */
    bool is_closed_vtable = ch_vtable->vt->chan_is_closed(ch_vtable);
    bool is_closed_standalone = chan_int_is_closed(ch_standalone);
    
    if (is_closed_vtable != is_closed_standalone) {
        chan_int_free(ch_vtable);
        chan_int_free(ch_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test close via vtable vs standalone */
    ch_vtable->vt->chan_close(ch_vtable);
    chan_int_close(ch_standalone);
    
    /* Verify both are now closed */
    if (ch_vtable->vt->chan_is_closed(ch_vtable) != chan_int_is_closed(ch_standalone)) {
        chan_int_free(ch_vtable);
        chan_int_free(ch_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test send to closed channel via vtable vs standalone */
    ChanStatus closed_send_vtable = ch_vtable->vt->chan_send(ch_vtable, val);
    ChanStatus closed_send_standalone = chan_int_send(ch_standalone, val);
    
    if (closed_send_vtable != closed_send_standalone) {
        chan_int_free(ch_vtable);
        chan_int_free(ch_standalone);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Cleanup using vtable free for one, standalone for other */
    ch_vtable->vt->chan_free(ch_vtable);
    chan_int_free(ch_standalone);
    
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
        "Property 1 (vtable): Shared vtable instances (Channel)",
        prop_channel_shared_vtable,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 11 (vtable): Channel vtable behavioral equivalence",
        prop_channel_vtable_equivalence,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_CHANNEL_TESTS (sizeof(channel_tests) / sizeof(channel_tests[0]))

int run_channel_tests(theft_seed seed) {
    int failures = 0;
    
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

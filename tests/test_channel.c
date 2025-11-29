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

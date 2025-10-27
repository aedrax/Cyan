/**
 * @file test_coro.c
 * @brief Property-based tests for Coroutine mechanism
 * 
 * Tests validate correctness properties:
 * - Property 24: Resume continues from yield point
 * - Property 25: Yield value is retrievable
 * - Property 26: Completed coroutine is marked finished
 * - Property 27: Coroutine status reflects actual state
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theft.h"
#include <cyan/coro.h>

/* Minimum iterations for property tests */
#define MIN_TEST_TRIALS 100

/*============================================================================
 * Test Helpers
 *============================================================================*/

/* Global to track status during execution */
static CoroStatus g_status_during_run = CORO_CREATED;

/*============================================================================
 * Property 24: Resume continues from yield point
 * For any suspended coroutine, resume SHALL continue execution from
 * immediately after the last yield call.
 *============================================================================*/

/* Coroutine that yields at specific points and tracks progress */
static void coro_yield_sequence(Coro *self, void *arg) {
    int *sequence = (int *)arg;
    
    sequence[0] = 1;  /* Before first yield */
    coro_yield(self);
    
    sequence[1] = 2;  /* After first yield, before second */
    coro_yield(self);
    
    sequence[2] = 3;  /* After second yield, before third */
    coro_yield(self);
    
    sequence[3] = 4;  /* After third yield, at end */
}

static enum theft_trial_res prop_resume_continues_from_yield(struct theft *t, void *arg1) {
    (void)t;
    (void)arg1;
    
    int sequence[4] = {0, 0, 0, 0};
    
    Coro *c = coro_new(coro_yield_sequence, sequence, 0);
    if (!c) return THEFT_TRIAL_FAIL;
    
    /* First resume: should execute until first yield */
    coro_resume(c);
    if (sequence[0] != 1 || sequence[1] != 0) {
        coro_free(c);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Second resume: should continue from after first yield */
    coro_resume(c);
    if (sequence[1] != 2 || sequence[2] != 0) {
        coro_free(c);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Third resume: should continue from after second yield */
    coro_resume(c);
    if (sequence[2] != 3 || sequence[3] != 0) {
        coro_free(c);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Fourth resume: should complete */
    coro_resume(c);
    if (sequence[3] != 4) {
        coro_free(c);
        return THEFT_TRIAL_FAIL;
    }
    
    if (!coro_is_finished(c)) {
        coro_free(c);
        return THEFT_TRIAL_FAIL;
    }
    
    coro_free(c);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 25: Yield value is retrievable
 * For any coroutine that yields with a value, the caller SHALL be able to
 * retrieve that exact value via coro_get_yield.
 *============================================================================*/

/* Coroutine that yields a sequence of values */
static void coro_yield_values(Coro *self, void *arg) {
    int count = *(int *)arg;
    
    for (int i = 0; i < count; i++) {
        coro_yield_value(self, i * 10);
    }
}

static enum theft_trial_res prop_yield_value_retrievable(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int count = (int)((*val_ptr % 10) + 1);  /* 1-10 iterations */
    
    Coro *c = coro_new(coro_yield_values, &count, 0);
    if (!c) return THEFT_TRIAL_FAIL;
    
    for (int i = 0; i < count; i++) {
        bool yielded = coro_resume(c);
        if (!yielded && i < count - 1) {
            /* Should have yielded but didn't */
            coro_free(c);
            return THEFT_TRIAL_FAIL;
        }
        
        if (!coro_is_finished(c)) {
            int expected = i * 10;
            int actual = coro_get_yield(c, int);
            if (actual != expected) {
                coro_free(c);
                return THEFT_TRIAL_FAIL;
            }
        }
    }
    
    /* One more resume to complete */
    coro_resume(c);
    
    if (!coro_is_finished(c)) {
        coro_free(c);
        return THEFT_TRIAL_FAIL;
    }
    
    coro_free(c);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 26: Completed coroutine is marked finished
 * For any coroutine that runs to completion, its status SHALL be CORO_FINISHED
 * and further resume calls SHALL fail.
 *============================================================================*/

/* Simple coroutine that completes immediately */
static void coro_immediate_complete(Coro *self, void *arg) {
    (void)self;
    int *flag = (int *)arg;
    *flag = 1;
}

/* Coroutine that yields N times then completes */
static void coro_yield_n_times(Coro *self, void *arg) {
    int n = *(int *)arg;
    for (int i = 0; i < n; i++) {
        coro_yield(self);
    }
}

static enum theft_trial_res prop_completed_marked_finished(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int n = (int)((*val_ptr % 5) + 1);  /* 1-5 yields */
    
    /* Test 1: Coroutine that completes immediately */
    int flag = 0;
    Coro *c1 = coro_new(coro_immediate_complete, &flag, 0);
    if (!c1) return THEFT_TRIAL_FAIL;
    
    if (coro_status(c1) != CORO_CREATED) {
        coro_free(c1);
        return THEFT_TRIAL_FAIL;
    }
    
    coro_resume(c1);
    
    if (!coro_is_finished(c1)) {
        coro_free(c1);
        return THEFT_TRIAL_FAIL;
    }
    
    if (coro_status(c1) != CORO_FINISHED) {
        coro_free(c1);
        return THEFT_TRIAL_FAIL;
    }
    
    if (flag != 1) {
        coro_free(c1);
        return THEFT_TRIAL_FAIL;
    }
    
    coro_free(c1);
    
    /* Test 2: Coroutine that yields then completes */
    Coro *c2 = coro_new(coro_yield_n_times, &n, 0);
    if (!c2) return THEFT_TRIAL_FAIL;
    
    /* Resume n times (each yield) */
    for (int i = 0; i < n; i++) {
        coro_resume(c2);
        if (coro_is_finished(c2)) {
            coro_free(c2);
            return THEFT_TRIAL_FAIL;  /* Should not be finished yet */
        }
    }
    
    /* One more resume to complete */
    coro_resume(c2);
    
    if (!coro_is_finished(c2)) {
        coro_free(c2);
        return THEFT_TRIAL_FAIL;
    }
    
    if (coro_status(c2) != CORO_FINISHED) {
        coro_free(c2);
        return THEFT_TRIAL_FAIL;
    }
    
    coro_free(c2);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 27: Coroutine status reflects actual state
 * For any coroutine, the status SHALL accurately reflect whether it is
 * CREATED, RUNNING, SUSPENDED, or FINISHED.
 *============================================================================*/

static void coro_check_status(Coro *self, void *arg) {
    (void)arg;
    /* Capture status while running */
    g_status_during_run = coro_status(self);
    coro_yield(self);
    /* After yield and resume, should be RUNNING again */
    g_status_during_run = coro_status(self);
}

static enum theft_trial_res prop_status_reflects_state(struct theft *t, void *arg1) {
    (void)t;
    (void)arg1;
    
    g_status_during_run = CORO_CREATED;
    
    Coro *c = coro_new(coro_check_status, NULL, 0);
    if (!c) return THEFT_TRIAL_FAIL;
    
    /* Initial state: CREATED */
    if (coro_status(c) != CORO_CREATED) {
        coro_free(c);
        return THEFT_TRIAL_FAIL;
    }
    
    /* First resume: coroutine runs then yields */
    coro_resume(c);
    
    /* Status during run should have been RUNNING */
    if (g_status_during_run != CORO_RUNNING) {
        coro_free(c);
        return THEFT_TRIAL_FAIL;
    }
    
    /* After yield: SUSPENDED */
    if (coro_status(c) != CORO_SUSPENDED) {
        coro_free(c);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Second resume: coroutine completes */
    coro_resume(c);
    
    /* Status during second run should have been RUNNING */
    if (g_status_during_run != CORO_RUNNING) {
        coro_free(c);
        return THEFT_TRIAL_FAIL;
    }
    
    /* After completion: FINISHED */
    if (coro_status(c) != CORO_FINISHED) {
        coro_free(c);
        return THEFT_TRIAL_FAIL;
    }
    
    coro_free(c);
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Test Registration
 *============================================================================*/

typedef struct {
    const char *name;
    theft_propfun1 *prop;
    enum theft_builtin_type_info type;
} CoroTest;

static CoroTest coro_tests[] = {
    {
        "Property 24: Resume continues from yield point",
        prop_resume_continues_from_yield,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 25: Yield value is retrievable",
        prop_yield_value_retrievable,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 26: Completed coroutine is marked finished",
        prop_completed_marked_finished,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 27: Coroutine status reflects actual state",
        prop_status_reflects_state,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_CORO_TESTS (sizeof(coro_tests) / sizeof(coro_tests[0]))

int run_coro_tests(theft_seed seed) {
    int failures = 0;
    
    printf("\nCoroutine Tests:\n");
    
    for (size_t i = 0; i < NUM_CORO_TESTS; i++) {
        CoroTest *test = &coro_tests[i];
        
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

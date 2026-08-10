/**
 * @file 17_coro_channels.c
 * @brief Example demonstrating Go-style CSP: coroutines + channels + coro_run
 *
 * In single-threaded mode, a blocking channel operation performed INSIDE a
 * coroutine no longer fails with WOULD_BLOCK/None: it yields back to the
 * scheduler and retries when resumed. Driving the coroutines with coro_run
 * gives cooperative producer/consumer pipelines without threads.
 *
 * Everything (coroutines, channels, coro_run) must live in one translation
 * unit, and coro.h must be included before channel.h - including
 * <cyan/cyan.h> guarantees the order.
 *
 * NOTE: coroutines are EXPERIMENTAL (POSIX ucontext). On macOS the system
 * headers mark the ucontext functions deprecated; those warnings are
 * expected and harmless here.
 */

#include <cyan/cyan.h>  /* must come first: sets up _XOPEN_SOURCE for ucontext */
#include <stdio.h>

CHANNEL_DEFINE(int);

/* The channel shared by producers and consumers. A file-scope static (or a
 * pointer passed via the coroutine arg) is the canonical way to share it. */
static Channel_int *ch;

/*============================================================================
 * Coroutine bodies
 *============================================================================*/

/* Section 1: a plain generator - yields values directly, no channel */
static void countdown(Coro *self, void *arg) {
    int from = *(int *)arg;
    /* coro_current() identifies the running coroutine from anywhere */
    printf("   (inside the coroutine: coro_current() == self: %s)\n",
           coro_current() == self ? "yes" : "no");
    for (int i = from; i >= 1; i--) {
        coro_yield_value(self, i);
    }
}

/* Section 3: an incremental computation with NO channel traffic. It marks
 * its own progress so coro_run's deadlock detector does not mistake its
 * quiet yields for a stuck coroutine. */
static void ticker(Coro *self, void *arg) {
    (void)arg;
    for (int step = 1; step <= 3; step++) {
        coro_mark_progress();
        coro_yield(self);
    }
    printf("      ticker: finished 3 background steps\n");
}

/* Sends `base+1 .. base+3` into the shared channel; send yields while full */
static void producer(Coro *self, void *arg) {
    (void)self;
    int base = arg ? *(int *)arg : 0;
    for (int i = 1; i <= 3; i++) {
        printf("      producer(%d): sending %d\n", base, base + i);
        chan_int_send(ch, base + i);   /* rendezvous/waits inside a coroutine */
    }
}

/* Producer variant that closes the channel when done (single-producer case) */
static void closing_producer(Coro *self, void *arg) {
    producer(self, arg);
    chan_int_close(ch);
    printf("      producer(0): closed the channel\n");
}

/* Receives until the channel is closed and drained; recv yields while empty */
static void consumer(Coro *self, void *arg) {
    (void)self;
    (void)arg;
    for (;;) {
        Option_int v = chan_int_recv(ch);
        if (is_none(v)) break;   /* closed and drained */
        printf("      consumer: received %d\n", unwrap(v));
    }
    printf("      consumer: channel closed, done\n");
}

/* Receives a fixed number of values (multi-producer case: nobody closes) */
static void fixed_count_consumer(Coro *self, void *arg) {
    (void)self;
    int expected = *(int *)arg;
    for (int i = 0; i < expected; i++) {
        Option_int v = chan_int_recv(ch);
        if (is_none(v)) break;
        printf("      consumer: received %d\n", unwrap(v));
    }
}

/* Waits on a channel nobody ever sends to - used for the deadlock demo */
static void lonely_consumer(Coro *self, void *arg) {
    (void)self;
    (void)arg;
    Option_int v = chan_int_recv(ch);
    printf("      lonely consumer woke up: %s\n",
           is_some(v) ? "Some (unexpected)" : "None (channel was closed)");
}

/*============================================================================
 * Main
 *============================================================================*/

int main(void) {
    printf("=== Coroutines + Channels (CSP) Example ===\n\n");

    /* --------------------------------------------------------
     * 1. A coroutine as a generator (no channel yet)
     * -------------------------------------------------------- */
    printf("1. A coroutine as a generator\n");
    int from = 3;
    Coro *gen = coro_new(countdown, &from, 0);
    while (coro_resume(gen)) {
        if (coro_has_yield(gen)) {
            printf("   generator yielded %d\n", coro_get_yield(gen, int));
        }
    }
    printf("   generator finished (status == CORO_FINISHED: %s)\n\n",
           coro_status(gen) == CORO_FINISHED ? "yes" : "no");
    coro_free(gen);

    /* --------------------------------------------------------
     * 2. Unbuffered rendezvous driven by coro_run
     * -------------------------------------------------------- */
    printf("2. Unbuffered channel: producer + consumer under coro_run\n");
    ch = chan_int_new(0);   /* capacity 0: every send waits for its receiver */
    Coro *pair[] = {
        coro_new(closing_producer, NULL, 0),
        coro_new(consumer, NULL, 0),
    };
    bool all_finished = coro_run(pair, 2);
    printf("   coro_run -> %s\n\n",
           all_finished ? "true (all coroutines finished)" : "false (deadlock)");
    coro_free(pair[0]);
    coro_free(pair[1]);
    chan_int_free(ch);

    /* --------------------------------------------------------
     * 3. Buffered channel with two producers
     * -------------------------------------------------------- */
    printf("3. Buffered channel (cap 2) with two producers\n");
    ch = chan_int_new(2);   /* small buffer: producers occasionally wait */
    int base_a = 100;
    int base_b = 200;
    int expected = 6;       /* 3 values from each producer */
    Coro *group[] = {
        coro_new(producer, &base_a, 0),
        coro_new(producer, &base_b, 0),
        coro_new(fixed_count_consumer, &expected, 0),
        coro_new(ticker, NULL, 0),   /* non-channel work, marks progress itself */
    };
    all_finished = coro_run(group, 4);
    printf("   coro_run -> %s\n\n",
           all_finished ? "true (all coroutines finished)" : "false (deadlock)");
    for (int i = 0; i < 4; i++) {
        coro_free(group[i]);
    }
    chan_int_close(ch);
    chan_int_free(ch);

    /* --------------------------------------------------------
     * 4. Deadlock detection, then recovery by closing the channel
     * -------------------------------------------------------- */
    printf("4. Deadlock detection and recovery\n");
    ch = chan_int_new(0);
    Coro *stuck[] = { coro_new(lonely_consumer, NULL, 0) };

    printf("   running a receiver with no sender...\n");
    bool ok = coro_run(stuck, 1);
    printf("   coro_run -> %s\n", ok ? "true (unexpected)" : "false (deadlock detected)");

    /* Recovery: close the channel; the blocked recv sees None and finishes */
    printf("   closing the channel to release the blocked receiver...\n");
    chan_int_close(ch);
    ok = coro_run(stuck, 1);
    printf("   coro_run -> %s\n", ok ? "true (all coroutines finished)" : "false");

    coro_free(stuck[0]);
    chan_int_free(ch);

    printf("\n=== CSP example complete ===\n");
    return 0;
}

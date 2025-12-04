/**
 * @file 10_channel_communication.c
 * @brief Example demonstrating CSP-style channels for communication
 * 
 * This example shows how to use channels for passing messages between
 * different parts of your program. Channels provide a safe way to
 * communicate values with buffering support.
 */

#include <cyan/cyan.h>
#include <stdio.h>

/* Define channel type for integers */
CHANNEL_DEFINE(int);

int main(void) {
    printf("=== Channel Communication Example ===\n\n");
    
    /* --------------------------------------------------------
     * 1. Creating a buffered channel
     * -------------------------------------------------------- */
    printf("1. Creating a buffered channel with capacity 5\n");
    Channel_int *ch = chan_int_new(5);
    printf("   Channel created successfully\n\n");
    
    /* --------------------------------------------------------
     * 2. Sending values to the channel
     * -------------------------------------------------------- */
    printf("2. Sending values to the channel\n");
    
    ChanStatus status;
    status = chan_int_send(ch, 10);
    printf("   Sent 10: %s\n", status == CHAN_OK ? "OK" : "Failed");
    
    status = chan_int_send(ch, 20);
    printf("   Sent 20: %s\n", status == CHAN_OK ? "OK" : "Failed");
    
    status = chan_int_send(ch, 30);
    printf("   Sent 30: %s\n\n", status == CHAN_OK ? "OK" : "Failed");
    
    /* --------------------------------------------------------
     * 3. Receiving values from the channel
     * -------------------------------------------------------- */
    printf("3. Receiving values from the channel\n");
    
    Option_int val1 = chan_int_recv(ch);
    if (is_some(val1)) {
        printf("   Received: %d\n", unwrap(val1));
    }
    
    Option_int val2 = chan_int_recv(ch);
    if (is_some(val2)) {
        printf("   Received: %d\n", unwrap(val2));
    }
    printf("\n");
    
    /* --------------------------------------------------------
     * 4. Non-blocking operations with try_send/try_recv
     * -------------------------------------------------------- */
    printf("4. Non-blocking operations\n");
    
    /* Drain remaining value from section 3 first */
    chan_int_try_recv(ch);
    
    /* Fill the channel using try_send (non-blocking) */
    int sent_count = 0;
    for (int i = 0; i < 6; i++) {  /* Try to send 6, but only 5 will fit */
        status = chan_int_try_send(ch, i * 100);
        if (status == CHAN_OK) {
            sent_count++;
        }
    }
    printf("   Sent %d values (channel capacity is 5)\n", sent_count);
    
    /* Try to send when full - won't block */
    status = chan_int_try_send(ch, 999);
    printf("   try_send when full: %s\n", 
           status == CHAN_WOULD_BLOCK ? "Would block (expected)" : "Unexpected");
    
    /* Try to receive - won't block */
    Option_int try_val = chan_int_try_recv(ch);
    if (is_some(try_val)) {
        printf("   try_recv got: %d\n\n", unwrap(try_val));
    }
    
    /* --------------------------------------------------------
     * 5. Checking channel status
     * -------------------------------------------------------- */
    printf("5. Channel status\n");
    printf("   Is closed: %s\n\n", chan_int_is_closed(ch) ? "yes" : "no");
    
    /* --------------------------------------------------------
     * 6. Closing the channel
     * -------------------------------------------------------- */
    printf("6. Closing the channel\n");
    chan_int_close(ch);
    printf("   Channel closed\n");
    printf("   Is closed: %s\n", chan_int_is_closed(ch) ? "yes" : "no");
    
    /* Drain remaining values after close */
    printf("   Draining remaining values:\n");
    Option_int remaining;
    while (is_some(remaining = chan_int_recv(ch))) {
        printf("     Got: %d\n", unwrap(remaining));
    }
    printf("   Channel empty, recv returns None\n\n");
    
    /* --------------------------------------------------------
     * 7. Using vtable macros
     * -------------------------------------------------------- */
    printf("7. Using vtable convenience macros\n");
    Channel_int *ch2 = chan_int_new(3);
    
    CHAN_SEND(ch2, 42);
    CHAN_SEND(ch2, 84);
    
    Option_int v = CHAN_RECV(ch2);
    printf("   CHAN_RECV: %d\n", unwrap(v));
    
    printf("   CHAN_IS_CLOSED: %s\n", CHAN_IS_CLOSED(ch2) ? "yes" : "no");
    
    CHAN_CLOSE(ch2);
    CHAN_FREE(ch2);
    printf("   Channel freed via CHAN_FREE\n\n");
    
    /* --------------------------------------------------------
     * Cleanup
     * -------------------------------------------------------- */
    chan_int_free(ch);
    
    printf("=== Channel example complete ===\n");
    return 0;
}

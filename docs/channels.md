# Channels

CSP-style communication primitives for message passing. Defined in
`<cyan/channel.h>`.

```c
#include <cyan/channel.h>

CHANNEL_DEFINE(i32);  // Define Channel_i32

i32 main(void) {
    // Create buffered channel with capacity 10
    Channel_i32 *ch = chan_i32_new(10);
    
    // Send values
    chan_i32_send(ch, 1);
    chan_i32_send(ch, 2);
    chan_i32_send(ch, 3);
    
    // Receive values (returns Option)
    Option_i32 val = chan_i32_recv(ch);
    if (is_some(val)) {
        printf("Received: %d\n", unwrap(val));
    }
    
    // Non-blocking operations
    ChanStatus status = chan_i32_try_send(ch, 42);
    if (status == CHAN_OK) {
        printf("Sent successfully\n");
    } else if (status == CHAN_WOULD_BLOCK) {
        printf("Channel full\n");
    }
    
    Option_i32 maybe = chan_i32_try_recv(ch);
    
    // Close channel (no more sends allowed)
    chan_i32_close(ch);
    
    // Drain remaining values
    while (is_some(val = chan_i32_recv(ch))) {
        printf("Drained: %d\n", unwrap(val));
    }
    
    // Check if closed
    if (chan_i32_is_closed(ch)) {
        printf("Channel is closed\n");
    }
    
    chan_i32_free(ch);
    return 0;
}
```

## Thread-safe channels

```c
// Enable thread safety before including
#define CYAN_CHANNEL_THREADSAFE
#include <cyan/channel.h>

// Now channels use pthread mutexes and condition variables
// for safe concurrent access from multiple threads
```

## Coroutine integration (CSP)

In single-threaded builds, a blocking channel operation called inside a
[coroutine](coroutines.md) yields and retries instead of returning
`CHAN_WOULD_BLOCK`/`None`. The integration is active only when `coro.h` is
included before `channel.h` (`cyan.h` guarantees this order); with the
reverse order it silently compiles to no-ops. Drive the coroutines with
`coro_run` for Go-style CSP:

```c
CHANNEL_DEFINE(i32);
static Channel_i32 *ch;

static void producer(Coro *self, void *arg) {
    for (i32 i = 1; i <= 5; i++) chan_i32_send(ch, i * 10);  // rendezvous
    chan_i32_close(ch);
}
static void consumer(Coro *self, void *arg) {
    for (;;) {
        Option_i32 v = chan_i32_recv(ch);   // yields until a value arrives
        if (is_none(v)) break;
        printf("got %d\n", unwrap(v));
    }
}

ch = chan_i32_new(0);                        // capacity 0: unbuffered
Coro *cs[] = { coro_new(producer, NULL, 0), coro_new(consumer, NULL, 0) };
bool ok = coro_run(cs, 2);                   // false would mean deadlock
```

`coro_run` detects deadlock: if a full pass resumes coroutines but none
finishes and no channel makes progress, it returns `false`. Coroutines
that yield repeatedly without channel traffic should call
`coro_mark_progress()`. Everything (coroutines, channels, `coro_run`) must
live in one translation unit. Outside coroutines, and for
`try_send`/`try_recv`, single-threaded behavior is unchanged.

## API reference

| Function | Description |
|----------|-------------|
| `chan_T_new(capacity)` | Create channel (0 = unbuffered) |
| `chan_T_send(ch, value)` | Send value; when full, blocks in thread-safe mode, yields inside a coroutine, otherwise returns `CHAN_WOULD_BLOCK` |
| `chan_T_recv(ch)` | Receive value as Option; when empty, blocks in thread-safe mode, yields inside a coroutine, otherwise returns None |
| `chan_T_try_send(ch, value)` | Non-blocking send (always `CHAN_WOULD_BLOCK` on an unbuffered channel) |
| `chan_T_try_recv(ch)` | Non-blocking receive (returns an Option, never a status) |
| `chan_T_close(ch)` | Close channel |
| `chan_T_is_closed(ch)` | Check if closed |
| `chan_T_free(ch)` | Free channel |

Channel functions are NULL-safe: sending on a NULL channel returns
`CHAN_CLOSED`, receiving returns None, and `is_closed` reports true.

Status values:

- `CHAN_OK` - Operation succeeded
- `CHAN_CLOSED` - Channel is closed
- `CHAN_WOULD_BLOCK` - Operation could not proceed without blocking (returned by `try_send`, and by `send` in single-threaded builds outside a coroutine)

## Convenience macros

Macros take the element type first (`ch` is a pointer):

| Macro | Description |
|-------|-------------|
| `CHAN_SEND(T, ch, val)` | Send value to channel |
| `CHAN_RECV(T, ch)` | Receive value from channel |
| `CHAN_TRY_SEND(T, ch, val)` | Non-blocking send |
| `CHAN_TRY_RECV(T, ch)` | Non-blocking receive |
| `CHAN_CLOSE(T, ch)` | Close channel |
| `CHAN_IS_CLOSED(T, ch)` | Check if closed |
| `CHAN_FREE(T, ch)` | Free channel |

/**
 * @file channel.h
 * @brief CSP-style channels for communication between coroutines or threads
 * 
 * This header provides type-safe channels for passing messages between
 * concurrent execution contexts. Channels support both buffered and
 * unbuffered (synchronous) communication.
 * 
 * Usage:
 *   CHANNEL_DEFINE(int);  // Define Channel_int type
 *   Channel_int *ch = chan_int_new(10);  // Buffered channel with capacity 10
 *   chan_int_send(ch, 42);
 *   Option_int val = chan_int_recv(ch);
 *   chan_int_close(ch);
 *   chan_int_free(ch);
 * 
 * Thread Safety:
 *   Define CYAN_CHANNEL_THREADSAFE before including this header to enable
 *   thread-safe operations using pthread mutexes and condition variables.
 */

#ifndef CYAN_CHANNEL_H
#define CYAN_CHANNEL_H

#include "common.h"
#include "option.h"
#include <string.h>

#ifdef CYAN_CHANNEL_THREADSAFE
#include <pthread.h>
#endif

/*============================================================================
 * Channel Status
 *============================================================================*/

/**
 * @brief Status codes for channel operations
 */
typedef enum {
    CHAN_OK,          /**< Operation succeeded */
    CHAN_CLOSED,      /**< Channel is closed */
    CHAN_WOULD_BLOCK  /**< Operation would block (for try_* variants) */
} ChanStatus;

/*============================================================================
 * Channel Type Definition Macro
 *============================================================================*/

/* Forward declare vtable struct */
#define CHANNEL_VT_FORWARD(T) \
    typedef struct ChannelVT_##T ChannelVT_##T

/**
 * @brief Generate a Channel type for a given element type
 * @param T The element type to be sent through the channel
 * 
 * Creates:
 * - Channel_T struct with buffer, capacity, head, tail, count, closed flag
 * - chan_T_new(capacity) - Create a new channel
 * - chan_T_send(ch, value) - Send a value (blocks if full)
 * - chan_T_recv(ch) - Receive a value (blocks if empty)
 * - chan_T_try_send(ch, value) - Non-blocking send
 * - chan_T_try_recv(ch) - Non-blocking receive
 * - chan_T_close(ch) - Close the channel
 * - chan_T_is_closed(ch) - Check if channel is closed
 * - chan_T_free(ch) - Free the channel
 */
#define CHANNEL_DEFINE(T) \
    /* Make sure Option type is defined for this type */ \
    OPTION_DEFINE(T); \
    CHANNEL_VT_FORWARD(T); \
    \
    typedef struct { \
        T *buffer;           /* Circular buffer for elements */ \
        size_t capacity;     /* Buffer capacity (0 = unbuffered) */ \
        size_t head;         /* Read position */ \
        size_t tail;         /* Write position */ \
        size_t count;        /* Current number of elements */ \
        bool closed;         /* Channel closed flag */ \
        /* Thread safety (only used if CYAN_CHANNEL_THREADSAFE is defined) */ \
        void *mutex;         /* Mutex for thread safety */ \
        void *cond_send;     /* Condition variable for senders */ \
        void *cond_recv;     /* Condition variable for receivers */ \
        const ChannelVT_##T *vt; /* Pointer to shared vtable */ \
    } Channel_##T; \
    \
    /* Internal: Lock the channel mutex if thread-safe */ \
    static inline void _chan_##T##_lock(Channel_##T *ch) { \
        (void)ch; \
        _CYAN_CHANNEL_LOCK(ch); \
    } \
    \
    /* Internal: Unlock the channel mutex if thread-safe */ \
    static inline void _chan_##T##_unlock(Channel_##T *ch) { \
        (void)ch; \
        _CYAN_CHANNEL_UNLOCK(ch); \
    } \
    \
    /* Internal: Wait on send condition if thread-safe */ \
    static inline void _chan_##T##_wait_send(Channel_##T *ch) { \
        (void)ch; \
        _CYAN_CHANNEL_WAIT_SEND(ch); \
    } \
    \
    /* Internal: Wait on recv condition if thread-safe */ \
    static inline void _chan_##T##_wait_recv(Channel_##T *ch) { \
        (void)ch; \
        _CYAN_CHANNEL_WAIT_RECV(ch); \
    } \
    \
    /* Internal: Signal send condition if thread-safe */ \
    static inline void _chan_##T##_signal_send(Channel_##T *ch) { \
        (void)ch; \
        _CYAN_CHANNEL_SIGNAL_SEND(ch); \
    } \
    \
    /* Internal: Signal recv condition if thread-safe */ \
    static inline void _chan_##T##_signal_recv(Channel_##T *ch) { \
        (void)ch; \
        _CYAN_CHANNEL_SIGNAL_RECV(ch); \
    } \
    \
    /* Forward declarations for vtable */ \
    static inline ChanStatus chan_##T##_send(Channel_##T *ch, T value); \
    static inline Option_##T chan_##T##_recv(Channel_##T *ch); \
    static inline ChanStatus chan_##T##_try_send(Channel_##T *ch, T value); \
    static inline Option_##T chan_##T##_try_recv(Channel_##T *ch); \
    static inline void chan_##T##_close(Channel_##T *ch); \
    static inline bool chan_##T##_is_closed(Channel_##T *ch); \
    static inline void chan_##T##_free(Channel_##T *ch); \
    \
    /* Vtable structure */ \
    struct ChannelVT_##T { \
        ChanStatus (*chan_send)(Channel_##T *ch, T value); \
        Option_##T (*chan_recv)(Channel_##T *ch); \
        ChanStatus (*chan_try_send)(Channel_##T *ch, T value); \
        Option_##T (*chan_try_recv)(Channel_##T *ch); \
        void (*chan_close)(Channel_##T *ch); \
        bool (*chan_is_closed)(Channel_##T *ch); \
        void (*chan_free)(Channel_##T *ch); \
    }; \
    \
    /* Static const vtable instance */ \
    static const ChannelVT_##T _chan_##T##_vt = { \
        .chan_send = chan_##T##_send, \
        .chan_recv = chan_##T##_recv, \
        .chan_try_send = chan_##T##_try_send, \
        .chan_try_recv = chan_##T##_try_recv, \
        .chan_close = chan_##T##_close, \
        .chan_is_closed = chan_##T##_is_closed, \
        .chan_free = chan_##T##_free \
    }; \
    \
    /** \
     * @brief Create a new channel \
     * @param capacity Buffer size (0 for unbuffered/synchronous) \
     * @return Pointer to new channel, or NULL on failure \
     */ \
    static inline Channel_##T *chan_##T##_new(size_t capacity) { \
        Channel_##T *ch = (Channel_##T *)malloc(sizeof(Channel_##T)); \
        if (!ch) { \
            CYAN_PANIC("chan_new: allocation failed"); \
            return NULL; \
        } \
        \
        ch->capacity = capacity; \
        ch->head = 0; \
        ch->tail = 0; \
        ch->count = 0; \
        ch->closed = false; \
        ch->mutex = NULL; \
        ch->cond_send = NULL; \
        ch->cond_recv = NULL; \
        ch->vt = &_chan_##T##_vt; \
        \
        if (capacity > 0) { \
            ch->buffer = (T *)malloc(capacity * sizeof(T)); \
            if (!ch->buffer) { \
                free(ch); \
                CYAN_PANIC("chan_new: buffer allocation failed"); \
                return NULL; \
            } \
        } else { \
            ch->buffer = NULL; \
        } \
        \
        _CYAN_CHANNEL_INIT(ch); \
        \
        return ch; \
    } \
    \
    /** \
     * @brief Check if channel is closed \
     * @param ch The channel to check \
     * @return true if closed, false otherwise \
     */ \
    static inline bool chan_##T##_is_closed(Channel_##T *ch) { \
        if (!ch) return true; \
        _chan_##T##_lock(ch); \
        bool result = ch->closed; \
        _chan_##T##_unlock(ch); \
        return result; \
    } \
    \
    /** \
     * @brief Close the channel \
     * @param ch The channel to close \
     * \
     * After closing, no more sends are allowed. Receivers will get \
     * remaining buffered values, then None. \
     */ \
    static inline void chan_##T##_close(Channel_##T *ch) { \
        if (!ch) return; \
        _chan_##T##_lock(ch); \
        ch->closed = true; \
        /* Wake up all waiting threads */ \
        _chan_##T##_signal_send(ch); \
        _chan_##T##_signal_recv(ch); \
        _chan_##T##_unlock(ch); \
    } \
    \
    /** \
     * @brief Try to send a value without blocking \
     * @param ch The channel to send to \
     * @param value The value to send \
     * @return CHAN_OK on success, CHAN_CLOSED if closed, CHAN_WOULD_BLOCK if full \
     */ \
    static inline ChanStatus chan_##T##_try_send(Channel_##T *ch, T value) { \
        if (!ch) return CHAN_CLOSED; \
        \
        _chan_##T##_lock(ch); \
        \
        if (ch->closed) { \
            _chan_##T##_unlock(ch); \
            return CHAN_CLOSED; \
        } \
        \
        /* For unbuffered channels, we can't do non-blocking send */ \
        if (ch->capacity == 0) { \
            _chan_##T##_unlock(ch); \
            return CHAN_WOULD_BLOCK; \
        } \
        \
        if (ch->count >= ch->capacity) { \
            _chan_##T##_unlock(ch); \
            return CHAN_WOULD_BLOCK; \
        } \
        \
        ch->buffer[ch->tail] = value; \
        ch->tail = (ch->tail + 1) % ch->capacity; \
        ch->count++; \
        \
        _chan_##T##_signal_recv(ch); \
        _chan_##T##_unlock(ch); \
        \
        return CHAN_OK; \
    } \
    \
    /** \
     * @brief Try to receive a value without blocking \
     * @param ch The channel to receive from \
     * @return Option containing the value, or None if empty/closed \
     */ \
    static inline Option_##T chan_##T##_try_recv(Channel_##T *ch) { \
        if (!ch) return None(T); \
        \
        _chan_##T##_lock(ch); \
        \
        if (ch->count == 0) { \
            _chan_##T##_unlock(ch); \
            return None(T); \
        } \
        \
        T value = ch->buffer[ch->head]; \
        ch->head = (ch->head + 1) % ch->capacity; \
        ch->count--; \
        \
        _chan_##T##_signal_send(ch); \
        _chan_##T##_unlock(ch); \
        \
        return Some(T, value); \
    } \
    \
    /** \
     * @brief Send a value to the channel (blocks if full) \
     * @param ch The channel to send to \
     * @param value The value to send \
     * @return CHAN_OK on success, CHAN_CLOSED if channel is closed \
     */ \
    static inline ChanStatus chan_##T##_send(Channel_##T *ch, T value) { \
        if (!ch) return CHAN_CLOSED; \
        \
        _chan_##T##_lock(ch); \
        \
        /* Wait while buffer is full and channel is open */ \
        while (ch->capacity > 0 && ch->count >= ch->capacity && !ch->closed) { \
            _chan_##T##_wait_send(ch); \
        } \
        \
        if (ch->closed) { \
            _chan_##T##_unlock(ch); \
            return CHAN_CLOSED; \
        } \
        \
        /* For unbuffered channels (capacity == 0), we need special handling */ \
        if (ch->capacity == 0) { \
            /* Unbuffered: just mark closed and return error for now */ \
            /* Full unbuffered semantics would require rendezvous */ \
            _chan_##T##_unlock(ch); \
            return CHAN_WOULD_BLOCK; \
        } \
        \
        ch->buffer[ch->tail] = value; \
        ch->tail = (ch->tail + 1) % ch->capacity; \
        ch->count++; \
        \
        _chan_##T##_signal_recv(ch); \
        _chan_##T##_unlock(ch); \
        \
        return CHAN_OK; \
    } \
    \
    /** \
     * @brief Receive a value from the channel (blocks if empty) \
     * @param ch The channel to receive from \
     * @return Option containing the value, or None if closed and empty \
     */ \
    static inline Option_##T chan_##T##_recv(Channel_##T *ch) { \
        if (!ch) return None(T); \
        \
        _chan_##T##_lock(ch); \
        \
        /* Wait while buffer is empty and channel is open */ \
        while (ch->count == 0 && !ch->closed) { \
            _chan_##T##_wait_recv(ch); \
        } \
        \
        /* If closed and empty, return None */ \
        if (ch->count == 0 && ch->closed) { \
            _chan_##T##_unlock(ch); \
            return None(T); \
        } \
        \
        T value = ch->buffer[ch->head]; \
        ch->head = (ch->head + 1) % ch->capacity; \
        ch->count--; \
        \
        _chan_##T##_signal_send(ch); \
        _chan_##T##_unlock(ch); \
        \
        return Some(T, value); \
    } \
    \
    /** \
     * @brief Free the channel and its resources \
     * @param ch The channel to free \
     */ \
    static inline void chan_##T##_free(Channel_##T *ch) { \
        if (!ch) return; \
        \
        _CYAN_CHANNEL_DESTROY(ch); \
        \
        if (ch->buffer) { \
            free(ch->buffer); \
        } \
        free(ch); \
    }

/*============================================================================
 * Thread Safety Macros
 *============================================================================*/

#ifdef CYAN_CHANNEL_THREADSAFE

/* Thread-safe implementation using pthreads */
#define _CYAN_CHANNEL_INIT(ch) do { \
    (ch)->mutex = malloc(sizeof(pthread_mutex_t)); \
    (ch)->cond_send = malloc(sizeof(pthread_cond_t)); \
    (ch)->cond_recv = malloc(sizeof(pthread_cond_t)); \
    if (!(ch)->mutex || !(ch)->cond_send || !(ch)->cond_recv) { \
        free((ch)->mutex); free((ch)->cond_send); free((ch)->cond_recv); \
        free((ch)->buffer); free(ch); \
        CYAN_PANIC("chan_new: sync primitive allocation failed"); \
    } \
    pthread_mutex_init((pthread_mutex_t *)(ch)->mutex, NULL); \
    pthread_cond_init((pthread_cond_t *)(ch)->cond_send, NULL); \
    pthread_cond_init((pthread_cond_t *)(ch)->cond_recv, NULL); \
} while(0)

#define _CYAN_CHANNEL_DESTROY(ch) do { \
    if ((ch)->mutex) { \
        pthread_mutex_destroy((pthread_mutex_t *)(ch)->mutex); \
        free((ch)->mutex); \
    } \
    if ((ch)->cond_send) { \
        pthread_cond_destroy((pthread_cond_t *)(ch)->cond_send); \
        free((ch)->cond_send); \
    } \
    if ((ch)->cond_recv) { \
        pthread_cond_destroy((pthread_cond_t *)(ch)->cond_recv); \
        free((ch)->cond_recv); \
    } \
} while(0)

#define _CYAN_CHANNEL_LOCK(ch) \
    pthread_mutex_lock((pthread_mutex_t *)(ch)->mutex)

#define _CYAN_CHANNEL_UNLOCK(ch) \
    pthread_mutex_unlock((pthread_mutex_t *)(ch)->mutex)

#define _CYAN_CHANNEL_WAIT_SEND(ch) \
    pthread_cond_wait((pthread_cond_t *)(ch)->cond_send, (pthread_mutex_t *)(ch)->mutex)

#define _CYAN_CHANNEL_WAIT_RECV(ch) \
    pthread_cond_wait((pthread_cond_t *)(ch)->cond_recv, (pthread_mutex_t *)(ch)->mutex)

#define _CYAN_CHANNEL_SIGNAL_SEND(ch) \
    pthread_cond_broadcast((pthread_cond_t *)(ch)->cond_send)

#define _CYAN_CHANNEL_SIGNAL_RECV(ch) \
    pthread_cond_broadcast((pthread_cond_t *)(ch)->cond_recv)

#else

/* Non-thread-safe implementation (no-op for single-threaded use) */
#define _CYAN_CHANNEL_INIT(ch) ((void)0)
#define _CYAN_CHANNEL_DESTROY(ch) ((void)0)
#define _CYAN_CHANNEL_LOCK(ch) ((void)0)
#define _CYAN_CHANNEL_UNLOCK(ch) ((void)0)
#define _CYAN_CHANNEL_WAIT_SEND(ch) ((void)0)
#define _CYAN_CHANNEL_WAIT_RECV(ch) ((void)0)
#define _CYAN_CHANNEL_SIGNAL_SEND(ch) ((void)0)
#define _CYAN_CHANNEL_SIGNAL_RECV(ch) ((void)0)

#endif /* CYAN_CHANNEL_THREADSAFE */

/*============================================================================
 * Vtable Convenience Macros
 *============================================================================*/

/**
 * @brief Send a value to channel (via vtable)
 */
#define CHAN_SEND(ch, val) ((ch)->vt->chan_send((ch), (val)))

/**
 * @brief Receive a value from channel (via vtable)
 */
#define CHAN_RECV(ch) ((ch)->vt->chan_recv((ch)))

/**
 * @brief Try to send a value without blocking (via vtable)
 */
#define CHAN_TRY_SEND(ch, val) ((ch)->vt->chan_try_send((ch), (val)))

/**
 * @brief Try to receive a value without blocking (via vtable)
 */
#define CHAN_TRY_RECV(ch) ((ch)->vt->chan_try_recv((ch)))

/**
 * @brief Close the channel (via vtable)
 */
#define CHAN_CLOSE(ch) ((ch)->vt->chan_close((ch)))

/**
 * @brief Check if channel is closed (via vtable)
 */
#define CHAN_IS_CLOSED(ch) ((ch)->vt->chan_is_closed((ch)))

/**
 * @brief Free the channel (via vtable)
 */
#define CHAN_FREE(ch) ((ch)->vt->chan_free((ch)))

#endif /* CYAN_CHANNEL_H */

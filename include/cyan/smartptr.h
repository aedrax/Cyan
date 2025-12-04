/**
 * @file smartptr.h
 * @brief Smart pointer types for automatic memory management
 * 
 * This header provides smart pointer types that automatically manage memory
 * lifetime through RAII-style cleanup using __attribute__((cleanup)).
 * 
 * Features:
 * - UniquePtr: Exclusive ownership, memory freed when pointer goes out of scope
 * - SharedPtr: Reference-counted shared ownership
 * - WeakPtr: Non-owning reference that doesn't prevent deallocation
 * 
 * Usage:
 *   UNIQUE_PTR_DEFINE(int);  // Define UniquePtr_int type
 *   SHARED_PTR_DEFINE(int);  // Define SharedPtr_int and WeakPtr_int types
 *   
 *   // Unique pointer with auto-cleanup
 *   unique_ptr(int, p, 42);  // Auto-freed on scope exit
 *   int val = unique_int_deref(&p);
 *   
 *   // Shared pointer
 *   shared_ptr(int, s, 100);
 *   SharedPtr_int s2 = shared_int_clone(&s);  // ref count = 2
 */

#ifndef CYAN_SMARTPTR_H
#define CYAN_SMARTPTR_H

#include "common.h"
#include "option.h"
#include <string.h>

/*============================================================================
 * Destructor Function Type
 *============================================================================*/

/**
 * @brief Destructor function type for custom cleanup
 * @param ptr Pointer to the object to destroy
 * 
 * Custom destructors are called before freeing memory, allowing cleanup
 * of nested resources.
 */
typedef void (*Destructor)(void *);

/*============================================================================
 * Shared Control Block
 *============================================================================
 * The control block is shared between SharedPtr and WeakPtr instances.
 * It tracks both strong (SharedPtr) and weak (WeakPtr) reference counts.
 * 
 * Memory is freed when strong_count reaches 0.
 * Control block is freed when both strong_count and weak_count reach 0.
 */

typedef struct {
    size_t strong_count;  /**< Number of SharedPtr references */
    size_t weak_count;    /**< Number of WeakPtr references (+ 1 if strong_count > 0) */
    Destructor dtor;      /**< Optional custom destructor */
} _SharedCtrlBlock;

/*============================================================================
 * Unique Pointer Definition Macro
 *============================================================================*/

/* Forward declare vtable struct */
#define UNIQUE_PTR_VT_FORWARD(T) \
    typedef struct UniquePtrVT_##T UniquePtrVT_##T

/**
 * @brief Generate a UniquePtr type for a given base type
 * @param T The base type to wrap
 * 
 * Creates UniquePtr_T with exclusive ownership semantics.
 * Memory is automatically freed when the pointer goes out of scope.
 * 
 * Example:
 *   UNIQUE_PTR_DEFINE(int);
 *   UniquePtr_int p = unique_int_new(42);
 */
#define UNIQUE_PTR_DEFINE(T) \
    UNIQUE_PTR_VT_FORWARD(T); \
    \
    typedef struct { \
        T *ptr; \
        Destructor dtor; \
        const UniquePtrVT_##T *vt; \
    } UniquePtr_##T; \
    \
    /* Forward declarations for vtable */ \
    static inline T *unique_##T##_get(UniquePtr_##T *u); \
    static inline T unique_##T##_deref(UniquePtr_##T *u); \
    static inline UniquePtr_##T unique_##T##_move(UniquePtr_##T *u); \
    static inline void unique_##T##_free(UniquePtr_##T *u); \
    \
    /* Vtable structure */ \
    struct UniquePtrVT_##T { \
        T *(*uptr_get)(UniquePtr_##T *u); \
        T (*uptr_deref)(UniquePtr_##T *u); \
        UniquePtr_##T (*uptr_move)(UniquePtr_##T *u); \
        void (*uptr_free)(UniquePtr_##T *u); \
    }; \
    \
    /* Static const vtable instance */ \
    static const UniquePtrVT_##T _unique_##T##_vt = { \
        .uptr_get = unique_##T##_get, \
        .uptr_deref = unique_##T##_deref, \
        .uptr_move = unique_##T##_move, \
        .uptr_free = unique_##T##_free \
    }; \
    \
    /** @brief Create a new unique pointer with a value */ \
    static inline UniquePtr_##T unique_##T##_new(T value) { \
        T *p = (T *)malloc(sizeof(T)); \
        if (!p) CYAN_PANIC("allocation failed"); \
        *p = value; \
        return (UniquePtr_##T){ .ptr = p, .dtor = NULL, .vt = &_unique_##T##_vt }; \
    } \
    \
    /** @brief Create a new unique pointer with a value and custom destructor */ \
    static inline UniquePtr_##T unique_##T##_new_with_dtor(T value, Destructor dtor) { \
        T *p = (T *)malloc(sizeof(T)); \
        if (!p) CYAN_PANIC("allocation failed"); \
        *p = value; \
        return (UniquePtr_##T){ .ptr = p, .dtor = dtor, .vt = &_unique_##T##_vt }; \
    } \
    \
    /** @brief Get raw pointer (does not transfer ownership) */ \
    static inline T *unique_##T##_get(UniquePtr_##T *u) { \
        return u->ptr; \
    } \
    \
    /** @brief Dereference the unique pointer */ \
    static inline T unique_##T##_deref(UniquePtr_##T *u) { \
        if (!u->ptr) CYAN_PANIC("deref null unique_ptr"); \
        return *u->ptr; \
    } \
    \
    /** @brief Move ownership to a new unique pointer, nullifying source */ \
    static inline UniquePtr_##T unique_##T##_move(UniquePtr_##T *u) { \
        UniquePtr_##T moved = *u; \
        u->ptr = NULL; \
        u->dtor = NULL; \
        return moved; \
    } \
    \
    /** @brief Free the unique pointer and call destructor if set */ \
    static inline void unique_##T##_free(UniquePtr_##T *u) { \
        if (u->ptr) { \
            if (u->dtor) u->dtor(u->ptr); \
            free(u->ptr); \
            u->ptr = NULL; \
            u->dtor = NULL; \
        } \
    }

/**
 * @brief Declare a unique pointer with automatic cleanup on scope exit
 * @param T The type of the pointed-to value
 * @param name Variable name for the unique pointer
 * @param value Initial value
 * 
 * Example:
 *   unique_ptr(int, p, 42);
 *   // p is automatically freed when scope exits
 */
#define unique_ptr(T, name, value) \
    __attribute__((cleanup(unique_##T##_free))) \
    UniquePtr_##T name = unique_##T##_new(value)

/**
 * @brief Declare a unique pointer with custom destructor and automatic cleanup
 * @param T The type of the pointed-to value
 * @param name Variable name for the unique pointer
 * @param value Initial value
 * @param dtor Custom destructor function
 */
#define unique_ptr_with_dtor(T, name, value, dtor) \
    __attribute__((cleanup(unique_##T##_free))) \
    UniquePtr_##T name = unique_##T##_new_with_dtor(value, dtor)

/*============================================================================
 * Shared Pointer Definition Macro
 *============================================================================*/

/* Forward declare vtable structs */
#define SHARED_PTR_VT_FORWARD(T) \
    typedef struct SharedPtrVT_##T SharedPtrVT_##T

#define WEAK_PTR_VT_FORWARD(T) \
    typedef struct WeakPtrVT_##T WeakPtrVT_##T

/**
 * @brief Generate SharedPtr and WeakPtr types for a given base type
 * @param T The base type to wrap
 * 
 * Creates:
 * - SharedPtr_T: Reference-counted shared ownership
 * - WeakPtr_T: Non-owning weak reference
 * - Option_SharedPtr_T: For weak pointer upgrade results
 * 
 * Example:
 *   SHARED_PTR_DEFINE(int);
 *   SharedPtr_int s = shared_int_new(42);
 *   SharedPtr_int s2 = shared_int_clone(&s);  // ref count = 2
 */
#define SHARED_PTR_DEFINE(T) \
    SHARED_PTR_VT_FORWARD(T); \
    WEAK_PTR_VT_FORWARD(T); \
    typedef struct SharedPtr_##T SharedPtr_##T; \
    typedef struct WeakPtr_##T WeakPtr_##T; \
    \
    struct SharedPtr_##T { \
        T *ptr; \
        _SharedCtrlBlock *ctrl; \
        const SharedPtrVT_##T *vt; \
    }; \
    \
    struct WeakPtr_##T { \
        T *ptr; \
        _SharedCtrlBlock *ctrl; \
        const WeakPtrVT_##T *vt; \
    }; \
    \
    /* Define Option type for SharedPtr to support weak_upgrade */ \
    typedef struct { \
        bool has_value; \
        SharedPtr_##T value; \
    } Option_SharedPtr_##T; \
    \
    /* Forward declarations for vtable */ \
    static inline T *shared_##T##_get(SharedPtr_##T *s); \
    static inline T shared_##T##_deref(SharedPtr_##T *s); \
    static inline SharedPtr_##T shared_##T##_clone(SharedPtr_##T *s); \
    static inline size_t shared_##T##_count(SharedPtr_##T *s); \
    static inline void shared_##T##_release(SharedPtr_##T *s); \
    \
    /* Vtable structure */ \
    struct SharedPtrVT_##T { \
        T *(*sptr_get)(SharedPtr_##T *s); \
        T (*sptr_deref)(SharedPtr_##T *s); \
        SharedPtr_##T (*sptr_clone)(SharedPtr_##T *s); \
        size_t (*sptr_count)(SharedPtr_##T *s); \
        void (*sptr_release)(SharedPtr_##T *s); \
    }; \
    \
    /* Static const vtable instance */ \
    static const SharedPtrVT_##T _shared_##T##_vt = { \
        .sptr_get = shared_##T##_get, \
        .sptr_deref = shared_##T##_deref, \
        .sptr_clone = shared_##T##_clone, \
        .sptr_count = shared_##T##_count, \
        .sptr_release = shared_##T##_release \
    }; \
    \
    /* Forward declarations for WeakPtr vtable */ \
    static inline bool weak_##T##_is_expired(WeakPtr_##T *w); \
    static inline Option_SharedPtr_##T weak_##T##_upgrade(WeakPtr_##T *w); \
    static inline void weak_##T##_release(WeakPtr_##T *w); \
    \
    /* WeakPtr vtable structure */ \
    struct WeakPtrVT_##T { \
        bool (*wptr_is_expired)(WeakPtr_##T *w); \
        Option_SharedPtr_##T (*wptr_upgrade)(WeakPtr_##T *w); \
        void (*wptr_release)(WeakPtr_##T *w); \
    }; \
    \
    /* Static const WeakPtr vtable instance */ \
    static const WeakPtrVT_##T _weak_##T##_vt = { \
        .wptr_is_expired = weak_##T##_is_expired, \
        .wptr_upgrade = weak_##T##_upgrade, \
        .wptr_release = weak_##T##_release \
    }; \
    \
    /** @brief Create a new shared pointer with a value */ \
    static inline SharedPtr_##T shared_##T##_new(T value) { \
        T *p = (T *)malloc(sizeof(T)); \
        _SharedCtrlBlock *ctrl = (_SharedCtrlBlock *)malloc(sizeof(_SharedCtrlBlock)); \
        if (!p || !ctrl) { \
            free(p); \
            free(ctrl); \
            CYAN_PANIC("allocation failed"); \
        } \
        *p = value; \
        *ctrl = (_SharedCtrlBlock){ .strong_count = 1, .weak_count = 1, .dtor = NULL }; \
        return (SharedPtr_##T){ .ptr = p, .ctrl = ctrl, .vt = &_shared_##T##_vt }; \
    } \
    \
    /** @brief Create a new shared pointer with a value and custom destructor */ \
    static inline SharedPtr_##T shared_##T##_new_with_dtor(T value, Destructor dtor) { \
        T *p = (T *)malloc(sizeof(T)); \
        _SharedCtrlBlock *ctrl = (_SharedCtrlBlock *)malloc(sizeof(_SharedCtrlBlock)); \
        if (!p || !ctrl) { \
            free(p); \
            free(ctrl); \
            CYAN_PANIC("allocation failed"); \
        } \
        *p = value; \
        *ctrl = (_SharedCtrlBlock){ .strong_count = 1, .weak_count = 1, .dtor = dtor }; \
        return (SharedPtr_##T){ .ptr = p, .ctrl = ctrl, .vt = &_shared_##T##_vt }; \
    } \
    \
    /** @brief Clone a shared pointer (increment reference count) */ \
    static inline SharedPtr_##T shared_##T##_clone(SharedPtr_##T *s) { \
        if (s->ctrl) s->ctrl->strong_count++; \
        return (SharedPtr_##T){ .ptr = s->ptr, .ctrl = s->ctrl, .vt = &_shared_##T##_vt }; \
    } \
    \
    /** @brief Get raw pointer (does not affect reference count) */ \
    static inline T *shared_##T##_get(SharedPtr_##T *s) { \
        return s->ptr; \
    } \
    \
    /** @brief Dereference the shared pointer */ \
    static inline T shared_##T##_deref(SharedPtr_##T *s) { \
        if (!s->ptr) CYAN_PANIC("deref null shared_ptr"); \
        return *s->ptr; \
    } \
    \
    /** @brief Get the current reference count */ \
    static inline size_t shared_##T##_count(SharedPtr_##T *s) { \
        return s->ctrl ? s->ctrl->strong_count : 0; \
    } \
    \
    /** @brief Release a shared pointer (decrement reference count) */ \
    static inline void shared_##T##_release(SharedPtr_##T *s) { \
        if (!s->ctrl) return; \
        if (--s->ctrl->strong_count == 0) { \
            if (s->ctrl->dtor) s->ctrl->dtor(s->ptr); \
            free(s->ptr); \
            s->ptr = NULL; \
            if (--s->ctrl->weak_count == 0) { \
                free(s->ctrl); \
            } \
        } \
        s->ctrl = NULL; \
        s->ptr = NULL; \
    } \
    \
    /* ============== Weak Pointer Functions ============== */ \
    \
    /** @brief Create a weak pointer from a shared pointer */ \
    static inline WeakPtr_##T weak_##T##_from_shared(SharedPtr_##T *s) { \
        if (s->ctrl) s->ctrl->weak_count++; \
        return (WeakPtr_##T){ .ptr = s->ptr, .ctrl = s->ctrl, .vt = &_weak_##T##_vt }; \
    } \
    \
    /** @brief Check if the weak pointer's target has been freed */ \
    static inline bool weak_##T##_is_expired(WeakPtr_##T *w) { \
        return !w->ctrl || w->ctrl->strong_count == 0; \
    } \
    \
    /** @brief Upgrade a weak pointer to a shared pointer if still valid */ \
    static inline Option_SharedPtr_##T weak_##T##_upgrade(WeakPtr_##T *w) { \
        if (weak_##T##_is_expired(w)) { \
            return (Option_SharedPtr_##T){ .has_value = false }; \
        } \
        w->ctrl->strong_count++; \
        return (Option_SharedPtr_##T){ \
            .has_value = true, \
            .value = (SharedPtr_##T){ .ptr = w->ptr, .ctrl = w->ctrl, .vt = &_shared_##T##_vt } \
        }; \
    } \
    \
    /** @brief Release a weak pointer */ \
    static inline void weak_##T##_release(WeakPtr_##T *w) { \
        if (!w->ctrl) return; \
        if (--w->ctrl->weak_count == 0 && w->ctrl->strong_count == 0) { \
            free(w->ctrl); \
        } \
        w->ctrl = NULL; \
        w->ptr = NULL; \
    }

/**
 * @brief Declare a shared pointer with automatic cleanup on scope exit
 * @param T The type of the pointed-to value
 * @param name Variable name for the shared pointer
 * @param value Initial value
 * 
 * Example:
 *   shared_ptr(int, s, 42);
 *   // s is automatically released when scope exits
 */
#define shared_ptr(T, name, value) \
    __attribute__((cleanup(shared_##T##_release))) \
    SharedPtr_##T name = shared_##T##_new(value)

/**
 * @brief Declare a shared pointer with custom destructor and automatic cleanup
 * @param T The type of the pointed-to value
 * @param name Variable name for the shared pointer
 * @param value Initial value
 * @param dtor Custom destructor function
 */
#define shared_ptr_with_dtor(T, name, value, dtor) \
    __attribute__((cleanup(shared_##T##_release))) \
    SharedPtr_##T name = shared_##T##_new_with_dtor(value, dtor)

/**
 * @brief Declare a weak pointer with automatic cleanup on scope exit
 * @param T The type of the pointed-to value
 * @param name Variable name for the weak pointer
 * @param shared The shared pointer to create a weak reference from
 * 
 * Example:
 *   shared_ptr(int, s, 42);
 *   weak_ptr(int, w, s);
 */
#define weak_ptr(T, name, shared) \
    __attribute__((cleanup(weak_##T##_release))) \
    WeakPtr_##T name = weak_##T##_from_shared(&(shared))

/*============================================================================
 * Vtable Convenience Macros
 *============================================================================*/

/**
 * @brief Get raw pointer from UniquePtr (via vtable)
 */
#define UPTR_GET(u) ((u).vt->uptr_get(&(u)))

/**
 * @brief Dereference UniquePtr (via vtable)
 */
#define UPTR_DEREF(u) ((u).vt->uptr_deref(&(u)))

/**
 * @brief Move ownership from UniquePtr (via vtable)
 */
#define UPTR_MOVE(u) ((u).vt->uptr_move(&(u)))

/**
 * @brief Free UniquePtr (via vtable)
 */
#define UPTR_FREE(u) ((u).vt->uptr_free(&(u)))

/**
 * @brief Get raw pointer from SharedPtr (via vtable)
 */
#define SPTR_GET(s) ((s).vt->sptr_get(&(s)))

/**
 * @brief Dereference SharedPtr (via vtable)
 */
#define SPTR_DEREF(s) ((s).vt->sptr_deref(&(s)))

/**
 * @brief Clone SharedPtr (via vtable)
 */
#define SPTR_CLONE(s) ((s).vt->sptr_clone(&(s)))

/**
 * @brief Get reference count from SharedPtr (via vtable)
 */
#define SPTR_COUNT(s) ((s).vt->sptr_count(&(s)))

/**
 * @brief Release SharedPtr (via vtable)
 */
#define SPTR_RELEASE(s) ((s).vt->sptr_release(&(s)))

/**
 * @brief Check if WeakPtr target has been freed (via vtable)
 */
#define WPTR_IS_EXPIRED(w) ((w).vt->wptr_is_expired(&(w)))

/**
 * @brief Upgrade WeakPtr to SharedPtr if still valid (via vtable)
 */
#define WPTR_UPGRADE(w) ((w).vt->wptr_upgrade(&(w)))

/**
 * @brief Release WeakPtr (via vtable)
 */
#define WPTR_RELEASE(w) ((w).vt->wptr_release(&(w)))

#endif /* CYAN_SMARTPTR_H */

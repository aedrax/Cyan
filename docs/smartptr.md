# Smart pointers

Automatic memory management with unique and shared ownership semantics.
Defined in `<cyan/smartptr.h>`.

```c
#include <cyan/smartptr.h>

OPTION_DEFINE(i32);
UNIQUE_PTR_DEFINE(i32);
SHARED_PTR_DEFINE(i32);

// Custom destructor
void cleanup_resource(void *ptr) {
    printf("Cleaning up: %d\n", *(i32*)ptr);
}

i32 main(void) {
    // === Unique Pointer (exclusive ownership) ===
    {
        unique_ptr(i32, p, 42);  // Auto-cleanup on scope exit
        printf("Value: %d\n", unique_i32_deref(&p));
        
        // Get raw pointer (doesn't transfer ownership)
        i32 *raw = unique_i32_get(&p);
        
        // Move ownership
        UniquePtr_i32 moved = unique_i32_move(&p);
        // p is now NULL, moved owns the memory
        
        unique_i32_free(&moved);
    }  // p would be freed here if not moved
    
    // With custom destructor
    unique_ptr_with_dtor(i32, p2, 100, cleanup_resource);
    
    // === Shared Pointer (reference counted) ===
    SharedPtr_i32 s1 = shared_i32_new(100);
    printf("Count: %zu\n", shared_i32_count(&s1));  // 1
    
    SharedPtr_i32 s2 = shared_i32_clone(&s1);
    printf("Count: %zu\n", shared_i32_count(&s1));  // 2
    
    printf("Value: %d\n", shared_i32_deref(&s1));
    
    shared_i32_release(&s1);  // Count = 1
    shared_i32_release(&s2);  // Count = 0, memory freed

    // === Weak Pointer (non-owning reference) ===
    SharedPtr_i32 owner = shared_i32_new(200);
    WeakPtr_i32 weak = weak_i32_from_shared(&owner);
    
    // Check if target still exists (standalone function)
    if (!weak_i32_is_expired(&weak)) {
        // Upgrade to shared pointer
        Option_SharedPtr_i32 upgraded = weak_i32_upgrade(&weak);
        if (upgraded.has_value) {
            printf("Upgraded: %d\n", shared_i32_deref(&upgraded.value));
            shared_i32_release(&upgraded.value);
        }
    }
    
    // Convenience macros (equivalent to the standalone functions)
    if (!WPTR_IS_EXPIRED(i32, weak)) {
        Option_SharedPtr_i32 upgraded = WPTR_UPGRADE(i32, weak);
        if (upgraded.has_value) {
            printf("Upgraded via macro: %d\n", shared_i32_deref(&upgraded.value));
            shared_i32_release(&upgraded.value);
        }
    }
    
    // Clone a weak reference
    WeakPtr_i32 weak2 = WPTR_CLONE(i32, weak);
    
    shared_i32_release(&owner);  // Memory freed
    // weak_i32_is_expired(&weak) now returns true
    
    WPTR_RELEASE(i32, weak);   // Release weak references
    WPTR_RELEASE(i32, weak2);
    return 0;
}
```

## API reference

| Function | Description |
|----------|-------------|
| `unique_ptr(T, name, value)` | Declare unique pointer with auto-cleanup |
| `unique_ptr_with_dtor(T, name, value, dtor)` | Declare unique pointer with auto-cleanup and custom destructor |
| `unique_T_new(value)` | Create unique pointer |
| `unique_T_new_with_dtor(value, dtor)` | Create unique pointer with custom destructor |
| `unique_T_deref(u)` | Dereference |
| `unique_T_get(u)` | Get raw pointer |
| `unique_T_move(u)` | Transfer ownership |
| `unique_T_free(u)` | Free memory and null the pointer |
| `shared_ptr(T, name, value)` | Declare shared pointer with auto-cleanup |
| `shared_ptr_with_dtor(T, name, value, dtor)` | Declare shared pointer with auto-cleanup and custom destructor |
| `shared_T_new(value)` | Create shared pointer |
| `shared_T_new_with_dtor(value, dtor)` | Create shared pointer with custom destructor |
| `shared_T_clone(s)` | Clone (increment ref count) |
| `shared_T_deref(s)` | Dereference |
| `shared_T_get(s)` | Get raw pointer |
| `shared_T_count(s)` | Get reference count |
| `shared_T_release(s)` | Release (decrement ref count) and null the pointer |
| `weak_ptr(T, name, shared)` | Declare weak reference with auto-cleanup |
| `weak_T_from_shared(s)` | Create weak reference |
| `weak_T_is_expired(w)` | Check if target freed |
| `weak_T_upgrade(w)` | Upgrade to shared pointer |
| `weak_T_clone(w)` | Clone weak reference |
| `weak_T_release(w)` | Release weak reference and null it |

`unique_T_free`, `shared_T_release`, and `weak_T_release` all null out the
pointer struct they are given, so releasing the same variable twice is a
no-op.

## Convenience macros

Macros take the pointee type first:

| Macro | Description |
|-------|-------------|
| `UPTR_GET(T, u)` | Get raw pointer from UniquePtr |
| `UPTR_DEREF(T, u)` | Dereference UniquePtr |
| `UPTR_MOVE(T, u)` | Transfer ownership from UniquePtr |
| `UPTR_FREE(T, u)` | Free UniquePtr |
| `SPTR_GET(T, s)` | Get raw pointer from SharedPtr |
| `SPTR_DEREF(T, s)` | Dereference SharedPtr |
| `SPTR_CLONE(T, s)` | Clone SharedPtr (increment ref count) |
| `SPTR_COUNT(T, s)` | Get reference count |
| `SPTR_RELEASE(T, s)` | Release SharedPtr (decrement ref count) |
| `WPTR_IS_EXPIRED(T, w)` | Check if WeakPtr target freed |
| `WPTR_UPGRADE(T, w)` | Upgrade WeakPtr to SharedPtr |
| `WPTR_CLONE(T, w)` | Clone WeakPtr |
| `WPTR_RELEASE(T, w)` | Release WeakPtr |

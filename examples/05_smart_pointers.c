/**
 * @file 05_smart_pointers.c
 * @brief Example demonstrating smart pointer usage
 * 
 * Compile: gcc -std=c11 -I../include -o smartptr_demo 05_smart_pointers.c
 * Run: ./smartptr_demo
 */

#include <stdio.h>
#include <cyan/common.h>
#include <cyan/option.h>
#include <cyan/smartptr.h>

// Define types - SHARED_PTR_DEFINE creates Option_SharedPtr_T internally
OPTION_DEFINE(i32);
UNIQUE_PTR_DEFINE(i32);
SHARED_PTR_DEFINE(i32);

// Custom destructor example
void i32_destructor(void *ptr) {
    i32 *p = (i32 *)ptr;
    printf("   [Custom destructor called for value: %d]\n", *p);
}

i32 main(void) {
    printf("=== Smart Pointer Examples ===\n\n");
    
    // Example 1: Unique pointer basics
    printf("1. Unique Pointer Basics:\n");
    {
        UniquePtr_i32 p = unique_i32_new(42);
        printf("   Created unique_ptr with value: %d\n", unique_i32_deref(&p));
        printf("   Pointer address: %p\n", (void *)unique_i32_get(&p));
        unique_i32_free(&p);
        printf("   After free, pointer is: %p\n", (void *)unique_i32_get(&p));
    }
    
    // Example 2: Auto-cleanup unique pointer
    printf("\n2. Auto-cleanup Unique Pointer:\n");
    {
        unique_ptr(i32, auto_p, 100);
        printf("   Value: %d\n", unique_i32_deref(&auto_p));
        printf("   Exiting scope...\n");
    }
    printf("   Pointer automatically freed!\n");
    
    // Example 3: Moving unique pointers
    printf("\n3. Moving Unique Pointers:\n");
    {
        UniquePtr_i32 source = unique_i32_new(999);
        printf("   Source value: %d\n", unique_i32_deref(&source));
        
        UniquePtr_i32 dest = unique_i32_move(&source);
        printf("   After move:\n");
        printf("   - Source pointer: %p (should be NULL)\n", (void *)unique_i32_get(&source));
        printf("   - Dest value: %d\n", unique_i32_deref(&dest));
        
        unique_i32_free(&dest);
    }
    
    // Example 4: Custom destructor
    printf("\n4. Custom Destructor:\n");
    {
        UniquePtr_i32 p = unique_i32_new_with_dtor(777, i32_destructor);
        printf("   Created with custom destructor, value: %d\n", unique_i32_deref(&p));
        printf("   Freeing...\n");
        unique_i32_free(&p);
    }
    
    // Example 5: Shared pointer basics
    printf("\n5. Shared Pointer Basics:\n");
    {
        SharedPtr_i32 s1 = shared_i32_new(50);
        printf("   Created shared_ptr, value: %d, count: %zu\n", 
               shared_i32_deref(&s1), shared_i32_count(&s1));
        
        SharedPtr_i32 s2 = shared_i32_clone(&s1);
        printf("   After clone, count: %zu\n", shared_i32_count(&s1));
        
        SharedPtr_i32 s3 = shared_i32_clone(&s2);
        printf("   After another clone, count: %zu\n", shared_i32_count(&s1));
        
        shared_i32_release(&s3);
        printf("   After releasing s3, count: %zu\n", shared_i32_count(&s1));
        
        shared_i32_release(&s2);
        printf("   After releasing s2, count: %zu\n", shared_i32_count(&s1));
        
        shared_i32_release(&s1);
        printf("   After releasing s1, memory freed\n");
    }
    
    // Example 6: Weak pointers
    printf("\n6. Weak Pointers:\n");
    {
        SharedPtr_i32 shared = shared_i32_new(123);
        printf("   Created shared_ptr, value: %d\n", shared_i32_deref(&shared));
        
        WeakPtr_i32 weak = weak_i32_from_shared(&shared);
        printf("   Created weak_ptr, is_expired: %s\n", 
               weak_i32_is_expired(&weak) ? "yes" : "no");
        printf("   Strong count still: %zu\n", shared_i32_count(&shared));
        
        // Upgrade weak to shared
        Option_SharedPtr_i32 upgraded = weak_i32_upgrade(&weak);
        if (upgraded.has_value) {
            SharedPtr_i32 s = upgraded.value;
            printf("   Upgraded weak_ptr, value: %d, count: %zu\n",
                   shared_i32_deref(&s), shared_i32_count(&s));
            shared_i32_release(&s);
        }
        
        // Release the original shared pointer
        shared_i32_release(&shared);
        printf("   After releasing shared, weak is_expired: %s\n",
               weak_i32_is_expired(&weak) ? "yes" : "no");
        
        // Try to upgrade expired weak pointer
        Option_SharedPtr_i32 failed = weak_i32_upgrade(&weak);
        printf("   Upgrade expired weak: %s\n", failed.has_value ? "Some" : "None");
        
        weak_i32_release(&weak);
    }
    
    // Example 7: Vtable Method-Style API for UniquePtr
    printf("\n7. UniquePtr Vtable API:\n");
    {
        UniquePtr_i32 u = unique_i32_new(555);
        
        // Direct vtable access
        printf("   Direct vtable access:\n");
        printf("      u.vt->uptr_deref(&u) = %d\n", u.vt->uptr_deref(&u));
        printf("      u.vt->uptr_get(&u) = %p\n", (void *)u.vt->uptr_get(&u));
        
        // Convenience macros
        printf("   Convenience macros:\n");
        printf("      UPTR_DEREF(u) = %d\n", UPTR_DEREF(u));
        printf("      UPTR_GET(u) = %p\n", (void *)UPTR_GET(u));
        
        // Move using vtable macro
        UniquePtr_i32 u2 = UPTR_MOVE(u);
        printf("   After UPTR_MOVE:\n");
        printf("      Original ptr: %p (should be NULL)\n", (void *)UPTR_GET(u));
        printf("      New ptr value: %d\n", UPTR_DEREF(u2));
        
        UPTR_FREE(u2);
    }
    
    // Example 8: Vtable Method-Style API for SharedPtr
    printf("\n8. SharedPtr Vtable API:\n");
    {
        SharedPtr_i32 s = shared_i32_new(777);
        
        // Direct vtable access
        printf("   Direct vtable access:\n");
        printf("      s.vt->sptr_deref(&s) = %d\n", s.vt->sptr_deref(&s));
        printf("      s.vt->sptr_count(&s) = %zu\n", s.vt->sptr_count(&s));
        
        // Convenience macros
        printf("   Convenience macros:\n");
        printf("      SPTR_DEREF(s) = %d\n", SPTR_DEREF(s));
        printf("      SPTR_COUNT(s) = %zu\n", SPTR_COUNT(s));
        
        // Clone using vtable macro
        SharedPtr_i32 s2 = SPTR_CLONE(s);
        printf("   After SPTR_CLONE:\n");
        printf("      SPTR_COUNT(s) = %zu\n", SPTR_COUNT(s));
        printf("      SPTR_COUNT(s2) = %zu\n", SPTR_COUNT(s2));
        
        // Shared vtable verification
        printf("   Shared vtable (memory efficient):\n");
        printf("      s.vt == s2.vt: %s\n", s.vt == s2.vt ? "yes" : "no");
        
        SPTR_RELEASE(s2);
        SPTR_RELEASE(s);
    }
    
    // Example 9: Vtable Method-Style API for WeakPtr
    printf("\n9. WeakPtr Vtable API:\n");
    {
        SharedPtr_i32 shared = shared_i32_new(888);
        WeakPtr_i32 w = weak_i32_from_shared(&shared);
        
        // Direct vtable access
        printf("   Direct vtable access:\n");
        printf("      w.vt->wptr_is_expired(&w) = %s\n", 
               w.vt->wptr_is_expired(&w) ? "yes" : "no");
        
        // Upgrade via vtable - returns Option_SharedPtr_i32
        Option_SharedPtr_i32 upgraded = w.vt->wptr_upgrade(&w);
        if (upgraded.has_value) {
            printf("      w.vt->wptr_upgrade(&w) = Some(%d)\n", 
                   shared_i32_deref(&upgraded.value));
            shared_i32_release(&upgraded.value);
        }
        
        // Convenience macros
        printf("   Convenience macros:\n");
        printf("      WPTR_IS_EXPIRED(w) = %s\n", 
               WPTR_IS_EXPIRED(w) ? "yes" : "no");
        
        // Upgrade via macro - returns Option_SharedPtr_i32
        Option_SharedPtr_i32 upgraded2 = WPTR_UPGRADE(w);
        if (upgraded2.has_value) {
            printf("      WPTR_UPGRADE(w) = Some(%d)\n", 
                   shared_i32_deref(&upgraded2.value));
            shared_i32_release(&upgraded2.value);
        }
        
        // Shared vtable verification
        WeakPtr_i32 w2 = weak_i32_from_shared(&shared);
        printf("   Shared vtable (memory efficient):\n");
        printf("      w.vt == w2.vt: %s\n", w.vt == w2.vt ? "yes" : "no");
        
        // Release shared pointer to demonstrate expired weak pointer
        shared_i32_release(&shared);
        printf("   After releasing shared pointer:\n");
        printf("      WPTR_IS_EXPIRED(w) = %s\n", 
               WPTR_IS_EXPIRED(w) ? "yes" : "no");
        
        // Upgrade expired weak pointer - returns None
        Option_SharedPtr_i32 failed = WPTR_UPGRADE(w);
        printf("      WPTR_UPGRADE(w) = %s\n", 
               failed.has_value ? "Some" : "None");
        
        // Release weak pointers via vtable macro
        WPTR_RELEASE(w);
        WPTR_RELEASE(w2);
    }
    
    printf("\n=== Done ===\n");
    return 0;
}

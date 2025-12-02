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
    
    printf("\n=== Done ===\n");
    return 0;
}

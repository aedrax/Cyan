/**
 * @file 05_smart_pointers.c
 * @brief Example demonstrating smart pointer usage
 * 
 * Compile: gcc -std=c11 -I../include -o smartptr_demo 05_smart_pointers.c
 * Run: ./smartptr_demo
 */

#include <stdio.h>
#include <cyan/option.h>
#include <cyan/smartptr.h>

// Define types - SHARED_PTR_DEFINE creates Option_SharedPtr_T internally
OPTION_DEFINE(int);
UNIQUE_PTR_DEFINE(int);
SHARED_PTR_DEFINE(int);

// Custom destructor example
void int_destructor(void *ptr) {
    int *p = (int *)ptr;
    printf("   [Custom destructor called for value: %d]\n", *p);
}

int main(void) {
    printf("=== Smart Pointer Examples ===\n\n");
    
    // Example 1: Unique pointer basics
    printf("1. Unique Pointer Basics:\n");
    {
        UniquePtr_int p = unique_int_new(42);
        printf("   Created unique_ptr with value: %d\n", unique_int_deref(&p));
        printf("   Pointer address: %p\n", (void *)unique_int_get(&p));
        unique_int_free(&p);
        printf("   After free, pointer is: %p\n", (void *)unique_int_get(&p));
    }
    
    // Example 2: Auto-cleanup unique pointer
    printf("\n2. Auto-cleanup Unique Pointer:\n");
    {
        unique_ptr(int, auto_p, 100);
        printf("   Value: %d\n", unique_int_deref(&auto_p));
        printf("   Exiting scope...\n");
    }
    printf("   Pointer automatically freed!\n");
    
    // Example 3: Moving unique pointers
    printf("\n3. Moving Unique Pointers:\n");
    {
        UniquePtr_int source = unique_int_new(999);
        printf("   Source value: %d\n", unique_int_deref(&source));
        
        UniquePtr_int dest = unique_int_move(&source);
        printf("   After move:\n");
        printf("   - Source pointer: %p (should be NULL)\n", (void *)unique_int_get(&source));
        printf("   - Dest value: %d\n", unique_int_deref(&dest));
        
        unique_int_free(&dest);
    }
    
    // Example 4: Custom destructor
    printf("\n4. Custom Destructor:\n");
    {
        UniquePtr_int p = unique_int_new_with_dtor(777, int_destructor);
        printf("   Created with custom destructor, value: %d\n", unique_int_deref(&p));
        printf("   Freeing...\n");
        unique_int_free(&p);
    }
    
    // Example 5: Shared pointer basics
    printf("\n5. Shared Pointer Basics:\n");
    {
        SharedPtr_int s1 = shared_int_new(50);
        printf("   Created shared_ptr, value: %d, count: %zu\n", 
               shared_int_deref(&s1), shared_int_count(&s1));
        
        SharedPtr_int s2 = shared_int_clone(&s1);
        printf("   After clone, count: %zu\n", shared_int_count(&s1));
        
        SharedPtr_int s3 = shared_int_clone(&s2);
        printf("   After another clone, count: %zu\n", shared_int_count(&s1));
        
        shared_int_release(&s3);
        printf("   After releasing s3, count: %zu\n", shared_int_count(&s1));
        
        shared_int_release(&s2);
        printf("   After releasing s2, count: %zu\n", shared_int_count(&s1));
        
        shared_int_release(&s1);
        printf("   After releasing s1, memory freed\n");
    }
    
    // Example 6: Weak pointers
    printf("\n6. Weak Pointers:\n");
    {
        SharedPtr_int shared = shared_int_new(123);
        printf("   Created shared_ptr, value: %d\n", shared_int_deref(&shared));
        
        WeakPtr_int weak = weak_int_from_shared(&shared);
        printf("   Created weak_ptr, is_expired: %s\n", 
               weak_int_is_expired(&weak) ? "yes" : "no");
        printf("   Strong count still: %zu\n", shared_int_count(&shared));
        
        // Upgrade weak to shared
        Option_SharedPtr_int upgraded = weak_int_upgrade(&weak);
        if (upgraded.has_value) {
            SharedPtr_int s = upgraded.value;
            printf("   Upgraded weak_ptr, value: %d, count: %zu\n",
                   shared_int_deref(&s), shared_int_count(&s));
            shared_int_release(&s);
        }
        
        // Release the original shared pointer
        shared_int_release(&shared);
        printf("   After releasing shared, weak is_expired: %s\n",
               weak_int_is_expired(&weak) ? "yes" : "no");
        
        // Try to upgrade expired weak pointer
        Option_SharedPtr_int failed = weak_int_upgrade(&weak);
        printf("   Upgrade expired weak: %s\n", failed.has_value ? "Some" : "None");
        
        weak_int_release(&weak);
    }
    
    printf("\n=== Done ===\n");
    return 0;
}

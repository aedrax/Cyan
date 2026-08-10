/**
 * @file 03_vector_collections.c
 * @brief Example demonstrating Vector (dynamic array) usage
 * 
 * Compile: gcc -std=c11 -I../include -o vector_demo 03_vector_collections.c
 * Run: ./vector_demo
 */

#include <stdio.h>
#include <cyan/common.h>
#include <cyan/option.h>
#include <cyan/vector.h>

// Define types using primitive aliases
OPTION_DEFINE(i32);
OPTION_DEFINE(f64);
VECTOR_DEFINE(i32);
VECTOR_DEFINE(f64);

// A simple struct to demonstrate custom types
typedef struct {
    i32 x;
    i32 y;
} Point;

OPTION_DEFINE(Point);
VECTOR_DEFINE(Point);

i32 main(void) {
    printf("=== Vector Examples ===\n\n");
    
    // Example 1: Basic vector operations
    printf("1. Basic Operations:\n");
    Vec_i32 numbers = vec_i32_new();
    
    // Push elements
    for (i32 i = 1; i <= 5; i++) {
        vec_i32_push(&numbers, i * 10);
    }
    printf("   After pushing 5 elements, length: %zu\n", vec_i32_len(&numbers));
    
    // Access elements
    printf("   Elements: ");
    for (usize i = 0; i < vec_i32_len(&numbers); i++) {
        Option_i32 elem = vec_i32_get(&numbers, i);
        if (is_some(elem)) {
            printf("%d ", unwrap(elem));
        }
    }
    printf("\n");
    
    // Example 2: Bounds checking
    printf("\n2. Bounds Checking:\n");
    Option_i32 valid = vec_i32_get(&numbers, 2);
    Option_i32 invalid = vec_i32_get(&numbers, 100);
    
    printf("   get(2): %s\n", is_some(valid) ? "Some" : "None");
    printf("   get(100): %s\n", is_some(invalid) ? "Some" : "None");
    
    // Example 3: Pop elements
    printf("\n3. Popping Elements:\n");
    printf("   Popping: ");
    Option_i32 popped;
    while (is_some(popped = vec_i32_pop(&numbers))) {
        printf("%d ", unwrap(popped));
    }
    printf("\n");
    printf("   After popping all, length: %zu\n", vec_i32_len(&numbers));
    
    // Example 4: Pre-allocated capacity
    printf("\n4. Pre-allocated Vector:\n");
    Vec_f64 values = vec_f64_with_capacity(100);
    printf("   Created with capacity 100, length: %zu\n", vec_f64_len(&values));
    
    for (i32 i = 0; i < 10; i++) {
        vec_f64_push(&values, i * 1.5);
    }
    printf("   After 10 pushes, length: %zu\n", vec_f64_len(&values));
    
    // Example 5: Custom struct vector
    printf("\n5. Vector of Structs:\n");
    Vec_Point points = vec_Point_new();
    
    vec_Point_push(&points, (Point){.x = 0, .y = 0});
    vec_Point_push(&points, (Point){.x = 10, .y = 20});
    vec_Point_push(&points, (Point){.x = -5, .y = 15});
    
    printf("   Points: ");
    for (usize i = 0; i < vec_Point_len(&points); i++) {
        Option_Point p = vec_Point_get(&points, i);
        if (is_some(p)) {
            Point pt = unwrap(p);
            printf("(%d,%d) ", pt.x, pt.y);
        }
    }
    printf("\n");
    
    // Example 6: Insert and Remove at Arbitrary Indices
    printf("\n6. Insert and Remove:\n");
    Vec_i32 seq = vec_i32_new();
    vec_i32_push(&seq, 10);
    vec_i32_push(&seq, 30);
    vec_i32_push(&seq, 40);

    // Insert 20 between 10 and 30
    vec_i32_insert(&seq, 1, 20);
    printf("   After insert(1, 20): ");
    for (usize i = 0; i < vec_i32_len(&seq); i++) {
        printf("%d ", unwrap(vec_i32_get(&seq, i)));
    }
    printf("\n");

    // Remove the element at index 2 (returns the removed value)
    Option_i32 taken = vec_i32_remove(&seq, 2);
    printf("   remove(2) returned: %d\n", is_some(taken) ? unwrap(taken) : -1);
    printf("   After remove(2): ");
    for (usize i = 0; i < vec_i32_len(&seq); i++) {
        printf("%d ", unwrap(vec_i32_get(&seq, i)));
    }
    printf("\n");

    vec_i32_free(&seq);

    // Example 7: Type-First Convenience Macros
    printf("\n7. Type-First Convenience Macros:\n");
    Vec_i32 demo = vec_i32_new();

    // Macros take the element type first, then the vector
    VEC_PUSH(i32, demo, 100);
    VEC_PUSH(i32, demo, 200);
    VEC_PUSH(i32, demo, 300);
    printf("      VEC_PUSH(i32, demo, 100/200/300)\n");
    printf("      VEC_LEN(i32, demo) = %zu\n", VEC_LEN(i32, demo));
    Option_i32 macro_elem = VEC_GET(i32, demo, 2);
    printf("      VEC_GET(i32, demo, 2) = %d\n", is_some(macro_elem) ? unwrap(macro_elem) : -1);

    // Insert and remove via macros
    VEC_INSERT(i32, demo, 0, 50);
    printf("      VEC_INSERT(i32, demo, 0, 50): len = %zu\n", VEC_LEN(i32, demo));
    Option_i32 removed = VEC_REMOVE(i32, demo, 0);
    printf("      VEC_REMOVE(i32, demo, 0) = %d\n", is_some(removed) ? unwrap(removed) : -1);

    // Pop using convenience macro
    printf("   Popping with VEC_POP: ");
    Option_i32 pop_val;
    while (is_some(pop_val = VEC_POP(i32, demo))) {
        printf("%d ", unwrap(pop_val));
    }
    printf("\n");

    VEC_FREE(i32, demo);
    
    // Cleanup
    vec_i32_free(&numbers);
    vec_f64_free(&values);
    vec_Point_free(&points);
    
    printf("\n=== Done ===\n");
    return 0;
}

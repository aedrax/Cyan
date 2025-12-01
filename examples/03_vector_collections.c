/**
 * @file 03_vector_collections.c
 * @brief Example demonstrating Vector (dynamic array) usage
 * 
 * Compile: gcc -std=c11 -I../include -o vector_demo 03_vector_collections.c
 * Run: ./vector_demo
 */

#include <stdio.h>
#include <cyan/option.h>
#include <cyan/vector.h>

// Define types
OPTION_DEFINE(int);
OPTION_DEFINE(double);
VECTOR_DEFINE(int);
VECTOR_DEFINE(double);

// A simple struct to demonstrate custom types
typedef struct {
    int x;
    int y;
} Point;

OPTION_DEFINE(Point);
VECTOR_DEFINE(Point);

int main(void) {
    printf("=== Vector Examples ===\n\n");
    
    // Example 1: Basic vector operations
    printf("1. Basic Operations:\n");
    Vec_int numbers = vec_int_new();
    
    // Push elements
    for (int i = 1; i <= 5; i++) {
        vec_int_push(&numbers, i * 10);
    }
    printf("   After pushing 5 elements, length: %zu\n", vec_int_len(&numbers));
    
    // Access elements
    printf("   Elements: ");
    for (size_t i = 0; i < vec_int_len(&numbers); i++) {
        Option_int elem = vec_int_get(&numbers, i);
        if (is_some(elem)) {
            printf("%d ", unwrap(elem));
        }
    }
    printf("\n");
    
    // Example 2: Bounds checking
    printf("\n2. Bounds Checking:\n");
    Option_int valid = vec_int_get(&numbers, 2);
    Option_int invalid = vec_int_get(&numbers, 100);
    
    printf("   get(2): %s\n", is_some(valid) ? "Some" : "None");
    printf("   get(100): %s\n", is_some(invalid) ? "Some" : "None");
    
    // Example 3: Pop elements
    printf("\n3. Popping Elements:\n");
    printf("   Popping: ");
    Option_int popped;
    while (is_some(popped = vec_int_pop(&numbers))) {
        printf("%d ", unwrap(popped));
    }
    printf("\n");
    printf("   After popping all, length: %zu\n", vec_int_len(&numbers));
    
    // Example 4: Pre-allocated capacity
    printf("\n4. Pre-allocated Vector:\n");
    Vec_double values = vec_double_with_capacity(100);
    printf("   Created with capacity 100, length: %zu\n", vec_double_len(&values));
    
    for (int i = 0; i < 10; i++) {
        vec_double_push(&values, i * 1.5);
    }
    printf("   After 10 pushes, length: %zu\n", vec_double_len(&values));
    
    // Example 5: Custom struct vector
    printf("\n5. Vector of Structs:\n");
    Vec_Point points = vec_Point_new();
    
    vec_Point_push(&points, (Point){.x = 0, .y = 0});
    vec_Point_push(&points, (Point){.x = 10, .y = 20});
    vec_Point_push(&points, (Point){.x = -5, .y = 15});
    
    printf("   Points: ");
    for (size_t i = 0; i < vec_Point_len(&points); i++) {
        Option_Point p = vec_Point_get(&points, i);
        if (is_some(p)) {
            Point pt = unwrap(p);
            printf("(%d,%d) ", pt.x, pt.y);
        }
    }
    printf("\n");
    
    // Cleanup
    vec_int_free(&numbers);
    vec_double_free(&values);
    vec_Point_free(&points);
    
    printf("\n=== Done ===\n");
    return 0;
}

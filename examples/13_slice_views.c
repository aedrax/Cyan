/**
 * @file 13_slice_views.c
 * @brief Example demonstrating slices for safe array views
 * 
 * This example shows how to use slices as non-owning views into
 * arrays and vectors, providing bounds-checked access without copying.
 */

#include <cyan/cyan.h>
#include <stdio.h>

/* Define required types first */
OPTION_DEFINE(int);                 /* Required for Option_int */
VECTOR_DEFINE(int);                 /* Required for Vec_int */
SLICE_DEFINE(int);                  /* Define Slice_int */

int main(void) {
    printf("=== Slice Views Example ===\n\n");
    
    /* --------------------------------------------------------
     * 1. Creating a slice from a C array
     * -------------------------------------------------------- */
    printf("1. Creating a slice from a C array\n");
    
    int numbers[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    Slice_int s = slice_int_from_array(numbers, 10);
    
    printf("   Array: ");
    for (size_t i = 0; i < slice_int_len(s); i++) {
        Option_int val = slice_int_get(s, i);
        if (is_some(val)) {
            printf("%d ", unwrap(val));
        }
    }
    printf("\n");
    printf("   Slice length: %zu\n\n", slice_int_len(s));
    
    /* --------------------------------------------------------
     * 2. Bounds-checked access
     * -------------------------------------------------------- */
    printf("2. Bounds-checked access with Option\n");
    
    Option_int elem = slice_int_get(s, 3);
    if (is_some(elem)) {
        printf("   Element at index 3: %d\n", unwrap(elem));
    }
    
    elem = slice_int_get(s, 0);
    if (is_some(elem)) {
        printf("   First element: %d\n", unwrap(elem));
    }
    
    elem = slice_int_get(s, 9);
    if (is_some(elem)) {
        printf("   Last element: %d\n", unwrap(elem));
    }
    
    /* Out of bounds access returns None */
    elem = slice_int_get(s, 100);
    printf("   Index 100 exists: %s\n\n", is_some(elem) ? "yes" : "no");
    
    /* --------------------------------------------------------
     * 3. Creating subslices
     * -------------------------------------------------------- */
    printf("3. Creating subslices (views into views)\n");
    
    /* Get elements 2-5 (indices 2, 3, 4) */
    Slice_int sub = slice_int_subslice(s, 2, 5);
    printf("   Subslice [2:5]: ");
    for (size_t i = 0; i < slice_int_len(sub); i++) {
        printf("%d ", unwrap(slice_int_get(sub, i)));
    }
    printf("\n");
    printf("   Subslice length: %zu\n", slice_int_len(sub));
    
    /* First half */
    Slice_int first_half = slice_int_subslice(s, 0, 5);
    printf("   First half [0:5]: ");
    for (size_t i = 0; i < slice_int_len(first_half); i++) {
        printf("%d ", unwrap(slice_int_get(first_half, i)));
    }
    printf("\n");
    
    /* Second half */
    Slice_int second_half = slice_int_subslice(s, 5, 10);
    printf("   Second half [5:10]: ");
    for (size_t i = 0; i < slice_int_len(second_half); i++) {
        printf("%d ", unwrap(slice_int_get(second_half, i)));
    }
    printf("\n\n");
    
    /* --------------------------------------------------------
     * 4. Subslice bounds clamping
     * -------------------------------------------------------- */
    printf("4. Subslice bounds are automatically clamped\n");
    
    /* Out of bounds end is clamped */
    Slice_int clamped = slice_int_subslice(s, 8, 100);
    printf("   subslice(s, 8, 100) length: %zu (clamped to valid range)\n", 
           slice_int_len(clamped));
    
    /* Start > end gives empty slice */
    Slice_int empty = slice_int_subslice(s, 5, 3);
    printf("   subslice(s, 5, 3) length: %zu (start > end = empty)\n\n", 
           slice_int_len(empty));
    
    /* --------------------------------------------------------
     * 5. Creating slice from vector
     * -------------------------------------------------------- */
    printf("5. Creating a slice from a vector\n");
    
    Vec_int v = vec_int_new();
    vec_int_push(&v, 100);
    vec_int_push(&v, 200);
    vec_int_push(&v, 300);
    vec_int_push(&v, 400);
    
    Slice_int vec_slice = slice_int_from_vec(&v);
    printf("   Vector contents via slice: ");
    for (size_t i = 0; i < slice_int_len(vec_slice); i++) {
        printf("%d ", unwrap(slice_int_get(vec_slice, i)));
    }
    printf("\n");
    printf("   Slice length: %zu\n\n", slice_int_len(vec_slice));
    
    vec_int_free(&v);
    
    /* --------------------------------------------------------
     * 6. Using vtable convenience macros
     * -------------------------------------------------------- */
    printf("6. Using vtable convenience macros\n");
    
    int data[] = {1, 2, 3, 4, 5};
    Slice_int sl = slice_int_from_array(data, 5);
    
    printf("   SLICE_LEN: %zu\n", SLICE_LEN(sl));
    printf("   SLICE_GET(sl, 2): %d\n", unwrap(SLICE_GET(sl, 2)));
    
    Slice_int sub2 = SLICE_SUBSLICE(sl, 1, 4);
    printf("   SLICE_SUBSLICE(sl, 1, 4): ");
    for (size_t i = 0; i < SLICE_LEN(sub2); i++) {
        printf("%d ", unwrap(SLICE_GET(sub2, i)));
    }
    printf("\n\n");
    
    /* --------------------------------------------------------
     * 7. Slices are non-owning views
     * -------------------------------------------------------- */
    printf("7. Slices are non-owning (zero-copy) views\n");
    
    int original[] = {1, 2, 3};
    Slice_int view = slice_int_from_array(original, 3);
    
    printf("   Original array: %d, %d, %d\n", original[0], original[1], original[2]);
    printf("   Slice view: %d, %d, %d\n", 
           unwrap(slice_int_get(view, 0)),
           unwrap(slice_int_get(view, 1)),
           unwrap(slice_int_get(view, 2)));
    
    /* Modify original - slice sees the change */
    original[1] = 999;
    printf("   After modifying original[1] = 999:\n");
    printf("   Slice view: %d, %d, %d\n",
           unwrap(slice_int_get(view, 0)),
           unwrap(slice_int_get(view, 1)),
           unwrap(slice_int_get(view, 2)));
    printf("   (Slice reflects the change - it's a view, not a copy)\n\n");
    
    printf("=== Slice example complete ===\n");
    return 0;
}

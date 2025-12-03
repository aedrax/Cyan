/**
 * @file 01_option_basics.c
 * @brief Example demonstrating Option type usage
 * 
 * Compile: gcc -std=c11 -I../include -o option_basics 01_option_basics.c
 * Run: ./option_basics
 */

#include <stdio.h>
#include <cyan/common.h>
#include <cyan/option.h>

// Define Option types for the types we'll use
OPTION_DEFINE(i32);
OPTION_DEFINE(f64);

// A function that may or may not find a value
Option_i32 find_first_even(i32 arr[], usize len) {
    for (usize i = 0; i < len; i++) {
        if (arr[i] % 2 == 0) {
            return Some(i32, arr[i]);
        }
    }
    return None(i32);
}

// Transform i32 to f64
f64 i32_to_f64(i32 x) {
    return (f64)x;
}

i32 main(void) {
    printf("=== Option Type Examples ===\n\n");
    
    // Example 1: Creating Options
    printf("1. Creating Options:\n");
    Option_i32 some_val = Some(i32, 42);
    Option_i32 no_val = None(i32);
    
    printf("   some_val has value: %s\n", is_some(some_val) ? "yes" : "no");
    printf("   no_val has value: %s\n", is_some(no_val) ? "yes" : "no");
    
    // Example 2: Checking and unwrapping
    printf("\n2. Checking and Unwrapping:\n");
    if (is_some(some_val)) {
        printf("   Unwrapped value: %d\n", unwrap(some_val));
    }
    
    // Example 3: Using unwrap_or for safe defaults
    printf("\n3. Using unwrap_or:\n");
    i32 val1 = unwrap_or(some_val, -1);
    i32 val2 = unwrap_or(no_val, -1);
    printf("   some_val unwrap_or(-1): %d\n", val1);
    printf("   no_val unwrap_or(-1): %d\n", val2);
    
    // Example 4: Practical usage - finding values
    printf("\n4. Finding Values:\n");
    i32 numbers[] = {1, 3, 5, 8, 9, 11};
    Option_i32 found = find_first_even(numbers, 6);
    
    if (is_some(found)) {
        printf("   First even number: %d\n", unwrap(found));
    } else {
        printf("   No even number found\n");
    }
    
    i32 odd_numbers[] = {1, 3, 5, 7, 9};
    Option_i32 not_found = find_first_even(odd_numbers, 5);
    printf("   In odd array: %s\n", is_none(not_found) ? "None" : "Some");
    
    // Example 5: Transforming Options with map_option
    printf("\n5. Transforming with map_option:\n");
    Option_i32 x = Some(i32, 10);
    Option_f64 doubled = map_option(x, f64, i32_to_f64);
    
    if (is_some(doubled)) {
        printf("   Transformed value: %.1f\n", unwrap(doubled));
    }
    
    // Mapping None produces None
    Option_i32 empty = None(i32);
    Option_f64 mapped_empty = map_option(empty, f64, i32_to_f64);
    printf("   Mapping None: %s\n", is_none(mapped_empty) ? "None" : "Some");
    
    // Example 6: Vtable Method-Style API
    printf("\n6. Vtable Method-Style API:\n");
    Option_i32 opt_vt = Some(i32, 99);
    Option_i32 none_vt = None(i32);
    
    // Direct vtable access
    printf("   Direct vtable access:\n");
    printf("      opt_vt.vt->opt_is_some(&opt_vt) = %s\n", opt_vt.vt->opt_is_some(&opt_vt) ? "yes" : "no");
    printf("      opt_vt.vt->opt_unwrap(&opt_vt) = %d\n", opt_vt.vt->opt_unwrap(&opt_vt));
    printf("      none_vt.vt->opt_unwrap_or(&none_vt, -1) = %d\n", none_vt.vt->opt_unwrap_or(&none_vt, -1));
    
    // Convenience macros (cleaner syntax)
    printf("   Convenience macros:\n");
    printf("      OPT_IS_SOME(opt_vt) = %s\n", OPT_IS_SOME(opt_vt) ? "yes" : "no");
    printf("      OPT_IS_NONE(none_vt) = %s\n", OPT_IS_NONE(none_vt) ? "yes" : "no");
    printf("      OPT_UNWRAP(opt_vt) = %d\n", OPT_UNWRAP(opt_vt));
    printf("      OPT_UNWRAP_OR(none_vt, -1) = %d\n", OPT_UNWRAP_OR(none_vt, -1));
    
    // All Option_i32 instances share the same vtable
    printf("   Shared vtable (memory efficient):\n");
    printf("      some_val.vt == opt_vt.vt: %s\n", some_val.vt == opt_vt.vt ? "yes" : "no");
    
    printf("\n=== Done ===\n");
    return 0;
}

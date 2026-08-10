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

// Chainable fallible transformation: reciprocal fails for zero
Option_f64 safe_reciprocal(i32 x) {
    if (x == 0) {
        return None(f64);
    }
    return Some(f64, 1.0 / (f64)x);
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
    
    // Example 6: Two Equivalent Call Styles
    printf("\n6. Two Equivalent Call Styles:\n");
    Option_i32 opt = Some(i32, 99);
    Option_i32 none_opt = None(i32);

    // Style 1: Standalone per-type functions
    printf("   Standalone functions:\n");
    printf("      option_i32_is_some(&opt) = %s\n", option_i32_is_some(&opt) ? "yes" : "no");
    printf("      option_i32_unwrap(&opt) = %d\n", option_i32_unwrap(&opt));
    printf("      option_i32_unwrap_or(&none_opt, -1) = %d\n", option_i32_unwrap_or(&none_opt, -1));

    // Style 2: Convenience macros (cleaner syntax)
    printf("   Convenience macros:\n");
    printf("      OPT_IS_SOME(opt) = %s\n", OPT_IS_SOME(opt) ? "yes" : "no");
    printf("      OPT_IS_NONE(none_opt) = %s\n", OPT_IS_NONE(none_opt) ? "yes" : "no");
    printf("      OPT_UNWRAP(opt) = %d\n", OPT_UNWRAP(opt));
    printf("      OPT_UNWRAP_OR(none_opt, -1) = %d\n", OPT_UNWRAP_OR(none_opt, -1));

    // Example 7: Chaining with and_then
    printf("\n7. Chaining with and_then:\n");
    Option_i32 ten = Some(i32, 10);
    Option_f64 recip = and_then(ten, f64, safe_reciprocal);
    printf("   and_then(Some(10), f64, safe_reciprocal) = Some(%.2f)\n", unwrap(recip));

    Option_i32 zero = Some(i32, 0);
    Option_f64 no_recip = and_then(zero, f64, safe_reciprocal);
    printf("   and_then(Some(0), f64, safe_reciprocal) = %s\n",
           is_none(no_recip) ? "None" : "Some");

    printf("\n=== Done ===\n");
    return 0;
}

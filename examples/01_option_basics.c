/**
 * @file 01_option_basics.c
 * @brief Example demonstrating Option type usage
 * 
 * Compile: gcc -std=c11 -I../include -o option_basics 01_option_basics.c
 * Run: ./option_basics
 */

#include <stdio.h>
#include <cyan/option.h>

// Define Option types for the types we'll use
OPTION_DEFINE(int);
OPTION_DEFINE(double);

// A function that may or may not find a value
Option_int find_first_even(int arr[], size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (arr[i] % 2 == 0) {
            return Some(int, arr[i]);
        }
    }
    return None(int);
}

// Transform int to double
double int_to_double(int x) {
    return (double)x;
}

int main(void) {
    printf("=== Option Type Examples ===\n\n");
    
    // Example 1: Creating Options
    printf("1. Creating Options:\n");
    Option_int some_val = Some(int, 42);
    Option_int no_val = None(int);
    
    printf("   some_val has value: %s\n", is_some(some_val) ? "yes" : "no");
    printf("   no_val has value: %s\n", is_some(no_val) ? "yes" : "no");
    
    // Example 2: Checking and unwrapping
    printf("\n2. Checking and Unwrapping:\n");
    if (is_some(some_val)) {
        printf("   Unwrapped value: %d\n", unwrap(some_val));
    }
    
    // Example 3: Using unwrap_or for safe defaults
    printf("\n3. Using unwrap_or:\n");
    int val1 = unwrap_or(some_val, -1);
    int val2 = unwrap_or(no_val, -1);
    printf("   some_val unwrap_or(-1): %d\n", val1);
    printf("   no_val unwrap_or(-1): %d\n", val2);
    
    // Example 4: Practical usage - finding values
    printf("\n4. Finding Values:\n");
    int numbers[] = {1, 3, 5, 8, 9, 11};
    Option_int found = find_first_even(numbers, 6);
    
    if (is_some(found)) {
        printf("   First even number: %d\n", unwrap(found));
    } else {
        printf("   No even number found\n");
    }
    
    int odd_numbers[] = {1, 3, 5, 7, 9};
    Option_int not_found = find_first_even(odd_numbers, 5);
    printf("   In odd array: %s\n", is_none(not_found) ? "None" : "Some");
    
    // Example 5: Transforming Options with map_option
    printf("\n5. Transforming with map_option:\n");
    Option_int x = Some(int, 10);
    Option_double doubled = map_option(x, double, int_to_double);
    
    if (is_some(doubled)) {
        printf("   Transformed value: %.1f\n", unwrap(doubled));
    }
    
    // Mapping None produces None
    Option_int empty = None(int);
    Option_double mapped_empty = map_option(empty, double, int_to_double);
    printf("   Mapping None: %s\n", is_none(mapped_empty) ? "None" : "Some");
    
    printf("\n=== Done ===\n");
    return 0;
}

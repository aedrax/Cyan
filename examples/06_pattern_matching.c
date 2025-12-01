/**
 * @file 06_pattern_matching.c
 * @brief Example demonstrating pattern matching macros
 * 
 * Compile: gcc -std=c11 -I../include -o match_demo 06_pattern_matching.c
 * Run: ./match_demo
 */

#include <stdio.h>
#include <cyan/option.h>
#include <cyan/result.h>
#include <cyan/match.h>

// Define types
OPTION_DEFINE(int);
OPTION_DEFINE(double);

typedef const char* const_charp;
RESULT_DEFINE(int, const_charp);
RESULT_DEFINE(double, const_charp);

// Helper function
Option_int find_index(int arr[], size_t len, int target) {
    for (size_t i = 0; i < len; i++) {
        if (arr[i] == target) return Some(int, (int)i);
    }
    return None(int);
}

Result_double_const_charp safe_sqrt(double x) {
    if (x < 0) return Err(double, const_charp, "cannot take sqrt of negative");
    // Simple approximation for demo
    double guess = x / 2.0;
    for (int i = 0; i < 10; i++) {
        guess = (guess + x / guess) / 2.0;
    }
    return Ok(double, const_charp, guess);
}

int main(void) {
    printf("=== Pattern Matching Examples ===\n\n");
    
    // Example 1: Basic Option matching
    printf("1. Option Matching:\n");
    Option_int some_val = Some(int, 42);
    Option_int no_val = None(int);
    
    printf("   Matching Some(42): ");
    match_option(some_val, int, val,
        { printf("Got value %d\n", val); },
        { printf("No value\n"); }
    );
    
    printf("   Matching None: ");
    match_option(no_val, int, val,
        { printf("Got value %d\n", val); },
        { printf("No value\n"); }
    );
    
    // Example 2: Result matching
    printf("\n2. Result Matching:\n");
    Result_int_const_charp ok_res = Ok(int, const_charp, 100);
    Result_int_const_charp err_res = Err(int, const_charp, "something failed");
    
    printf("   Matching Ok(100): ");
    match_result(ok_res, int, const_charp, val, e,
        { printf("Success with %d\n", val); },
        { printf("Error: %s\n", e); }
    );
    
    printf("   Matching Err: ");
    match_result(err_res, int, const_charp, val, e,
        { printf("Success with %d\n", val); },
        { printf("Error: %s\n", e); }
    );
    
    // Example 3: Practical usage - array search
    printf("\n3. Array Search with Matching:\n");
    int numbers[] = {10, 20, 30, 40, 50};
    
    int targets[] = {30, 99};
    for (int i = 0; i < 2; i++) {
        Option_int idx = find_index(numbers, 5, targets[i]);
        printf("   Finding %d: ", targets[i]);
        match_option(idx, int, index,
            { printf("found at index %d\n", index); },
            { printf("not found\n"); }
        );
    }
    
    // Example 4: Safe math operations
    printf("\n4. Safe Math with Result Matching:\n");
    double inputs[] = {16.0, -4.0, 25.0};
    
    for (int i = 0; i < 3; i++) {
        Result_double_const_charp res = safe_sqrt(inputs[i]);
        printf("   sqrt(%.1f) = ", inputs[i]);
        match_result(res, double, const_charp, val, e,
            { printf("%.4f\n", val); },
            { printf("Error: %s\n", e); }
        );
    }
    
    // Example 5: Expression-based matching
    printf("\n5. Expression-based Matching:\n");
    Option_int opt = Some(int, 5);
    
    // Get doubled value or 0
    int doubled = match_option_expr(opt, int, int, v, v * 2, 0);
    printf("   Some(5) doubled: %d\n", doubled);
    
    Option_int empty = None(int);
    int default_val = match_option_expr(empty, int, int, v, v * 2, -1);
    printf("   None doubled with default -1: %d\n", default_val);
    
    // Result expression matching
    Result_int_const_charp res = Ok(int, const_charp, 10);
    int squared = match_result_expr(res, int, const_charp, int, v, err, v * v, 0);
    printf("   Ok(10) squared: %d\n", squared);
    
    printf("\n=== Done ===\n");
    return 0;
}

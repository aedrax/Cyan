/**
 * @file 06_pattern_matching.c
 * @brief Example demonstrating pattern matching macros
 * 
 * Compile: gcc -std=c11 -I../include -o match_demo 06_pattern_matching.c
 * Run: ./match_demo
 */

#include <stdio.h>
#include <cyan/common.h>
#include <cyan/option.h>
#include <cyan/result.h>
#include <cyan/match.h>

// Define types using primitive aliases
OPTION_DEFINE(i32);
OPTION_DEFINE(f64);

typedef const char* const_charp;
RESULT_DEFINE(i32, const_charp);
RESULT_DEFINE(f64, const_charp);

// Helper function
Option_i32 find_index(i32 arr[], usize len, i32 target) {
    for (usize i = 0; i < len; i++) {
        if (arr[i] == target) return Some(i32, (i32)i);
    }
    return None(i32);
}

Result_f64_const_charp safe_sqrt(f64 x) {
    if (x < 0) return Err(f64, const_charp, "cannot take sqrt of negative");
    // Simple approximation for demo
    f64 guess = x / 2.0;
    for (i32 i = 0; i < 10; i++) {
        guess = (guess + x / guess) / 2.0;
    }
    return Ok(f64, const_charp, guess);
}

i32 main(void) {
    printf("=== Pattern Matching Examples ===\n\n");
    
    // Example 1: Basic Option matching
    printf("1. Option Matching:\n");
    Option_i32 some_val = Some(i32, 42);
    Option_i32 no_val = None(i32);
    
    printf("   Matching Some(42): ");
    match_option(some_val, i32, val,
        { printf("Got value %d\n", val); },
        { printf("No value\n"); }
    );
    
    printf("   Matching None: ");
    match_option(no_val, i32, val,
        { printf("Got value %d\n", val); },
        { printf("No value\n"); }
    );
    
    // Example 2: Result matching
    printf("\n2. Result Matching:\n");
    Result_i32_const_charp ok_res = Ok(i32, const_charp, 100);
    Result_i32_const_charp err_res = Err(i32, const_charp, "something failed");
    
    printf("   Matching Ok(100): ");
    match_result(ok_res, i32, const_charp, val, e,
        { printf("Success with %d\n", val); },
        { printf("Error: %s\n", e); }
    );
    
    printf("   Matching Err: ");
    match_result(err_res, i32, const_charp, val, e,
        { printf("Success with %d\n", val); },
        { printf("Error: %s\n", e); }
    );
    
    // Example 3: Practical usage - array search
    printf("\n3. Array Search with Matching:\n");
    i32 numbers[] = {10, 20, 30, 40, 50};
    
    i32 targets[] = {30, 99};
    for (i32 i = 0; i < 2; i++) {
        Option_i32 idx = find_index(numbers, 5, targets[i]);
        printf("   Finding %d: ", targets[i]);
        match_option(idx, i32, index,
            { printf("found at index %d\n", index); },
            { printf("not found\n"); }
        );
    }
    
    // Example 4: Safe math operations
    printf("\n4. Safe Math with Result Matching:\n");
    f64 inputs[] = {16.0, -4.0, 25.0};
    
    for (i32 i = 0; i < 3; i++) {
        Result_f64_const_charp res = safe_sqrt(inputs[i]);
        printf("   sqrt(%.1f) = ", inputs[i]);
        match_result(res, f64, const_charp, val, e,
            { printf("%.4f\n", val); },
            { printf("Error: %s\n", e); }
        );
    }
    
    // Example 5: Expression-based matching
    printf("\n5. Expression-based Matching:\n");
    Option_i32 opt = Some(i32, 5);
    
    // Get doubled value or 0
    i32 doubled = match_option_expr(opt, i32, i32, v, v * 2, 0);
    printf("   Some(5) doubled: %d\n", doubled);
    
    Option_i32 empty = None(i32);
    i32 default_val = match_option_expr(empty, i32, i32, v, v * 2, -1);
    printf("   None doubled with default -1: %d\n", default_val);
    
    // Result expression matching
    Result_i32_const_charp res = Ok(i32, const_charp, 10);
    i32 squared = match_result_expr(res, i32, const_charp, i32, v, err, v * v, 0);
    printf("   Ok(10) squared: %d\n", squared);
    
    printf("\n=== Done ===\n");
    return 0;
}

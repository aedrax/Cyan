/**
 * @file 02_result_error_handling.c
 * @brief Example demonstrating Result type for error handling
 * 
 * Compile: gcc -std=c11 -I../include -o result_errors 02_result_error_handling.c
 * Run: ./result_errors
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <cyan/result.h>

// Define a type alias for cleaner code
typedef const char* const_charp;

// Define Result types
RESULT_DEFINE(int, const_charp);
RESULT_DEFINE(double, const_charp);

// Parse a positive integer from string
Result_int_const_charp parse_positive_int(const char *str) {
    if (str == NULL || *str == '\0') {
        return Err(int, const_charp, "empty input");
    }
    
    char *endptr;
    long val = strtol(str, &endptr, 10);
    
    if (*endptr != '\0') {
        return Err(int, const_charp, "invalid number format");
    }
    
    if (val <= 0) {
        return Err(int, const_charp, "number must be positive");
    }
    
    if (val > INT_MAX) {
        return Err(int, const_charp, "number too large");
    }
    
    return Ok(int, const_charp, (int)val);
}

// Divide two numbers safely
Result_double_const_charp safe_divide(double a, double b) {
    if (b == 0.0) {
        return Err(double, const_charp, "division by zero");
    }
    return Ok(double, const_charp, a / b);
}

int main(void) {
    printf("=== Result Type Examples ===\n\n");
    
    // Example 1: Basic Result creation
    printf("1. Creating Results:\n");
    Result_int_const_charp success = Ok(int, const_charp, 42);
    Result_int_const_charp failure = Err(int, const_charp, "something went wrong");
    
    printf("   success is_ok: %s\n", is_ok(success) ? "yes" : "no");
    printf("   failure is_ok: %s\n", is_ok(failure) ? "yes" : "no");
    
    // Example 2: Unwrapping Results
    printf("\n2. Unwrapping Results:\n");
    if (is_ok(success)) {
        printf("   Success value: %d\n", unwrap_ok(success));
    }
    if (is_err(failure)) {
        printf("   Error message: %s\n", unwrap_err(failure));
    }
    
    // Example 3: Using unwrap_ok_or
    printf("\n3. Using unwrap_ok_or:\n");
    int val1 = unwrap_ok_or(success, -1);
    int val2 = unwrap_ok_or(failure, -1);
    printf("   success unwrap_ok_or(-1): %d\n", val1);
    printf("   failure unwrap_ok_or(-1): %d\n", val2);
    
    // Example 4: Parsing with error handling
    printf("\n4. Parsing Examples:\n");
    const char *inputs[] = {"123", "-5", "abc", "", "999999999999"};
    
    for (int i = 0; i < 5; i++) {
        Result_int_const_charp res = parse_positive_int(inputs[i]);
        printf("   parse(\"%s\"): ", inputs[i]);
        if (is_ok(res)) {
            printf("Ok(%d)\n", unwrap_ok(res));
        } else {
            printf("Err(\"%s\")\n", unwrap_err(res));
        }
    }
    
    // Example 5: Chaining operations
    printf("\n5. Safe Division:\n");
    double numerators[] = {10.0, 5.0, 0.0};
    double denominators[] = {2.0, 0.0, 3.0};
    
    for (int i = 0; i < 3; i++) {
        Result_double_const_charp res = safe_divide(numerators[i], denominators[i]);
        printf("   %.1f / %.1f = ", numerators[i], denominators[i]);
        if (is_ok(res)) {
            printf("%.2f\n", unwrap_ok(res));
        } else {
            printf("Error: %s\n", unwrap_err(res));
        }
    }
    
    printf("\n=== Done ===\n");
    return 0;
}

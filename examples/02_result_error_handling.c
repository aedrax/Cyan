/**
 * @file 02_result_error_handling.c
 * @brief Example demonstrating Result type for error handling
 * 
 * Compile: gcc -std=c11 -I../include -o result_errors 02_result_error_handling.c
 * Run: ./result_errors
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <cyan/common.h>
#include <cyan/result.h>

// Define a type alias for cleaner code
typedef const char* const_charp;

// Define Result types
RESULT_DEFINE(i32, const_charp);
RESULT_DEFINE(f64, const_charp);

// Parse a positive integer from string
Result_i32_const_charp parse_positive_int(const char *str) {
    if (str == NULL || *str == '\0') {
        return Err(i32, const_charp, "empty input");
    }
    
    char *endptr;
    long val = strtol(str, &endptr, 10);
    
    if (*endptr != '\0') {
        return Err(i32, const_charp, "invalid number format");
    }
    
    if (val <= 0) {
        return Err(i32, const_charp, "number must be positive");
    }
    
    if (val > INT_MAX) {
        return Err(i32, const_charp, "number too large");
    }
    
    return Ok(i32, const_charp, (i32)val);
}

// Divide two numbers safely
Result_f64_const_charp safe_divide(f64 a, f64 b) {
    if (b == 0.0) {
        return Err(f64, const_charp, "division by zero");
    }
    return Ok(f64, const_charp, a / b);
}

i32 main(void) {
    printf("=== Result Type Examples ===\n\n");
    
    // Example 1: Basic Result creation
    printf("1. Creating Results:\n");
    Result_i32_const_charp success = Ok(i32, const_charp, 42);
    Result_i32_const_charp failure = Err(i32, const_charp, "something went wrong");
    
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
    i32 val1 = unwrap_ok_or(success, -1);
    i32 val2 = unwrap_ok_or(failure, -1);
    printf("   success unwrap_ok_or(-1): %d\n", val1);
    printf("   failure unwrap_ok_or(-1): %d\n", val2);
    
    // Example 4: Parsing with error handling
    printf("\n4. Parsing Examples:\n");
    const char *inputs[] = {"123", "-5", "abc", "", "999999999999"};
    
    for (i32 i = 0; i < 5; i++) {
        Result_i32_const_charp res = parse_positive_int(inputs[i]);
        printf("   parse(\"%s\"): ", inputs[i]);
        if (is_ok(res)) {
            printf("Ok(%d)\n", unwrap_ok(res));
        } else {
            printf("Err(\"%s\")\n", unwrap_err(res));
        }
    }
    
    // Example 5: Chaining operations
    printf("\n5. Safe Division:\n");
    f64 numerators[] = {10.0, 5.0, 0.0};
    f64 denominators[] = {2.0, 0.0, 3.0};
    
    for (i32 i = 0; i < 3; i++) {
        Result_f64_const_charp res = safe_divide(numerators[i], denominators[i]);
        printf("   %.1f / %.1f = ", numerators[i], denominators[i]);
        if (is_ok(res)) {
            printf("%.2f\n", unwrap_ok(res));
        } else {
            printf("Error: %s\n", unwrap_err(res));
        }
    }
    
    // Example 6: Vtable Method-Style API
    printf("\n6. Vtable Method-Style API:\n");
    Result_i32_const_charp ok_vt = Ok(i32, const_charp, 200);
    Result_i32_const_charp err_vt = Err(i32, const_charp, "vtable error");
    
    // Direct vtable access
    printf("   Direct vtable access:\n");
    printf("      ok_vt.vt->res_is_ok(&ok_vt) = %s\n", ok_vt.vt->res_is_ok(&ok_vt) ? "yes" : "no");
    printf("      ok_vt.vt->res_unwrap_ok(&ok_vt) = %d\n", ok_vt.vt->res_unwrap_ok(&ok_vt));
    printf("      err_vt.vt->res_unwrap_err(&err_vt) = \"%s\"\n", err_vt.vt->res_unwrap_err(&err_vt));
    
    // Convenience macros (cleaner syntax)
    printf("   Convenience macros:\n");
    printf("      RES_IS_OK(ok_vt) = %s\n", RES_IS_OK(ok_vt) ? "yes" : "no");
    printf("      RES_IS_ERR(err_vt) = %s\n", RES_IS_ERR(err_vt) ? "yes" : "no");
    printf("      RES_UNWRAP_OK(ok_vt) = %d\n", RES_UNWRAP_OK(ok_vt));
    printf("      RES_UNWRAP_OK_OR(err_vt, -1) = %d\n", RES_UNWRAP_OK_OR(err_vt, -1));
    
    // All Result_i32_const_charp instances share the same vtable
    printf("   Shared vtable (memory efficient):\n");
    printf("      success.vt == ok_vt.vt: %s\n", success.vt == ok_vt.vt ? "yes" : "no");
    
    printf("\n=== Done ===\n");
    return 0;
}

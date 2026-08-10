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

// Parse two integers and add them; try_ok propagates the first Err
// (Rust-`?`-style: the enclosing function must return the same Result type)
Result_i32_const_charp sum_parsed(const char *a, const char *b) {
    i32 x = try_ok(parse_positive_int(a));
    i32 y = try_ok(parse_positive_int(b));
    return Ok(i32, const_charp, x + y);
}

// --- Ticket-order validation pipeline (used in example 8) ---
typedef struct {
    i32 age;
    i32 tickets;
} Order;

RESULT_DEFINE(Order, const_charp);

Result_i32_const_charp check_adult(i32 age) {
    if (age < 18) return Err(i32, const_charp, "must be 18 or older");
    if (age > 120) return Err(i32, const_charp, "age not plausible");
    return Ok(i32, const_charp, age);
}

// and_then_result chains a second fallible step onto a Result
Result_i32_const_charp validate_age(const char *age_str) {
    return and_then_result(parse_positive_int(age_str), i32, const_charp, check_adult);
}

// try_ok keeps the happy path linear inside the helper
Result_i32_const_charp validate_tickets(const char *tickets_str) {
    i32 t = try_ok(parse_positive_int(tickets_str));
    if (t > 8) return Err(i32, const_charp, "max 8 tickets per order");
    return Ok(i32, const_charp, t);
}

// Used with map_result: a promo doubles the ticket count of a valid order
i32 double_tickets(i32 t) { return t * 2; }

// End-to-end: the first failing field rejects the whole order
Result_Order_const_charp validate_order(const char *age_str, const char *tickets_str) {
    Result_i32_const_charp age = validate_age(age_str);
    if (is_err(age)) return Err(Order, const_charp, unwrap_err(age));

    Result_i32_const_charp tickets = validate_tickets(tickets_str);
    if (is_err(tickets)) return Err(Order, const_charp, unwrap_err(tickets));

    return Ok(Order, const_charp,
              ((Order){ .age = unwrap_ok(age), .tickets = unwrap_ok(tickets) }));
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
    
    // Example 6: Two Equivalent Call Styles
    printf("\n6. Two Equivalent Call Styles:\n");
    Result_i32_const_charp ok_res = Ok(i32, const_charp, 200);
    Result_i32_const_charp err_res = Err(i32, const_charp, "example error");

    // Style 1: Standalone per-type functions
    printf("   Standalone functions:\n");
    printf("      result_i32_const_charp_is_ok(&ok_res) = %s\n",
           result_i32_const_charp_is_ok(&ok_res) ? "yes" : "no");
    printf("      result_i32_const_charp_unwrap_ok(&ok_res) = %d\n",
           result_i32_const_charp_unwrap_ok(&ok_res));
    printf("      result_i32_const_charp_unwrap_err(&err_res) = \"%s\"\n",
           result_i32_const_charp_unwrap_err(&err_res));

    // Style 2: Convenience macros (cleaner syntax)
    printf("   Convenience macros:\n");
    printf("      RES_IS_OK(ok_res) = %s\n", RES_IS_OK(ok_res) ? "yes" : "no");
    printf("      RES_IS_ERR(err_res) = %s\n", RES_IS_ERR(err_res) ? "yes" : "no");
    printf("      RES_UNWRAP_OK(ok_res) = %d\n", RES_UNWRAP_OK(ok_res));
    printf("      RES_UNWRAP_OK_OR(err_res, -1) = %d\n", RES_UNWRAP_OK_OR(err_res, -1));

    // Example 7: Error propagation with try_ok (like Rust's `?`)
    printf("\n7. Error Propagation with try_ok:\n");
    Result_i32_const_charp summed = sum_parsed("40", "2");
    printf("   sum_parsed(\"40\", \"2\") = Ok(%d)\n", unwrap_ok(summed));

    Result_i32_const_charp propagated = sum_parsed("40", "oops");
    printf("   sum_parsed(\"40\", \"oops\") = Err(\"%s\")\n", unwrap_err(propagated));

    // Example 8: Putting it together - validating user input end-to-end
    printf("\n8. Putting It Together - Validating a Ticket Order:\n");
    const char *orders[][2] = {
        {"34", "4"},       // valid
        {"16", "2"},       // too young
        {"42", "twelve"},  // not a number
        {"29", "9"},       // too many tickets
    };

    for (usize i = 0; i < 4; i++) {
        Result_Order_const_charp res = validate_order(orders[i][0], orders[i][1]);
        printf("   order(age=%s, tickets=%s): ", orders[i][0], orders[i][1]);
        if (is_ok(res)) {
            Order o = unwrap_ok(res);
            printf("accepted (age %d, %d tickets)\n", o.age, o.tickets);
        } else {
            printf("rejected: %s\n", unwrap_err(res));
        }
    }

    // map_result transforms the Ok value and passes an Err through untouched
    Result_i32_const_charp promo = map_result(validate_tickets("3"),
                                              i32, const_charp, double_tickets);
    printf("   promo doubles tickets: 3 -> %d\n", unwrap_ok(promo));
    Result_i32_const_charp promo_err = map_result(validate_tickets("9"),
                                                  i32, const_charp, double_tickets);
    printf("   promo on invalid order: Err(\"%s\") passes through\n",
           unwrap_err(promo_err));

    // expect_ok documents an invariant: failure here is a programming error
    Order known_good = expect_ok(validate_order("30", "1"),
                                 "literal order must validate");
    printf("   expect_ok on known-good order: age %d, %d ticket(s)\n",
           known_good.age, known_good.tickets);

    printf("\n=== Done ===\n");
    return 0;
}

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
#include <cyan/result.h>

// Define Option types for the types we'll use
OPTION_DEFINE(i32);
OPTION_DEFINE(f64);

// Result type used by ok_or in example 8
typedef const char* const_charp;
RESULT_DEFINE(i32, const_charp);

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

// --- Config lookup chain helpers (used in example 8) ---
// Simulated configuration sources: each returns Some(port) when it defines one.
Option_i32 env_port(void)  { return None(i32); }        // env var not set
Option_i32 file_port(void) { return Some(i32, 8080); }  // config file has it

// Only non-privileged ports are acceptable for this app
Option_i32 validate_port(i32 p) {
    if (p < 1024 || p > 65535) return None(i32);
    return Some(i32, p);
}

// try_some propagates None like Rust's `?`
Option_i32 checked_port_from_file(void) {
    i32 p = try_some(file_port());
    return validate_port(p);
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

    // Example 8: Putting it together - a config lookup chain
    printf("\n8. Putting It Together - Config Lookup Chain:\n");

    // Look up a port: environment first, config file as fallback, then validate
    Option_i32 from_env = env_port();
    Option_i32 raw_port = or_else(from_env, file_port);       // env -> file
    printf("   env unset, or_else fell back to file: %s\n",
           is_some(raw_port) ? "Some" : "None");

    Option_i32 port = and_then(raw_port, i32, validate_port); // then validate
    printf("   validated port (unwrap_or default 9000): %d\n", unwrap_or(port, 9000));

    // expect() documents an invariant when a value must be present
    // (expect is the short name for OPT_EXPECT; ok_or below is OPT_OK_OR)
    i32 must_have = expect(port, "config file guarantees a port");
    printf("   expect() on validated port: %d\n", must_have);
    printf("   OPT_EXPECT (uppercase spelling): %d\n",
           OPT_EXPECT(port, "config file guarantees a port"));

    // try_some inside a helper propagates None automatically
    Option_i32 helper = checked_port_from_file();
    printf("   checked_port_from_file(): Some(%d)\n", unwrap(helper));

    // ok_or converts the Option into a Result for error-reporting layers
    Result_i32_const_charp as_result = ok_or(port, i32, const_charp, "no usable port");
    printf("   ok_or(Some) -> %s(%d)\n",
           is_ok(as_result) ? "Ok" : "Err", unwrap_ok_or(as_result, -1));

    Option_i32 bad = validate_port(80);                       // privileged port -> None
    Result_i32_const_charp bad_result = OPT_OK_OR(bad, i32, const_charp, "no usable port");
    printf("   OPT_OK_OR(None) -> Err(\"%s\")\n", unwrap_err(bad_result));

    printf("\n=== Done ===\n");
    return 0;
}

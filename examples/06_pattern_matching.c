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

// --- Tiny connection state machine (used in example 7) ---
typedef enum { ST_IDLE, ST_CONNECTING, ST_ONLINE } ConnState;

const char *state_name(ConnState s) {
    switch (s) {
        case ST_IDLE:       return "IDLE";
        case ST_CONNECTING: return "CONNECTING";
        case ST_ONLINE:     return "ONLINE";
    }
    return "?";
}

// Recognize an event name; unknown events produce None
Option_i32 parse_event(const char *ev) {
    if (ev[0] == 'd' && ev[1] == 'i') return Some(i32, 0);  // dial
    if (ev[0] == 'a') return Some(i32, 1);                  // ack
    if (ev[0] == 'd' && ev[1] == 'r') return Some(i32, 2);  // drop
    return None(i32);
}

// A transition may be invalid for the current state
Result_i32_const_charp next_state(ConnState s, i32 event) {
    if (s == ST_IDLE && event == 0)       return Ok(i32, const_charp, ST_CONNECTING);
    if (s == ST_CONNECTING && event == 1) return Ok(i32, const_charp, ST_ONLINE);
    if (event == 2)                       return Ok(i32, const_charp, ST_IDLE);
    return Err(i32, const_charp, "invalid transition");
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
    
    // Example 6: Convenience Macros with Pattern Matching
    printf("\n6. Convenience Macros with Pattern Matching:\n");
    
    // Using convenience macros before pattern matching
    Option_i32 vt_opt = Some(i32, 100);
    printf("   Using convenience macros for checks:\n");
    if (OPT_IS_SOME(vt_opt)) {
        printf("      OPT_IS_SOME(vt_opt) = true, value = %d\n", OPT_UNWRAP(vt_opt));
    }
    
    Result_i32_const_charp vt_res = Ok(i32, const_charp, 50);
    if (RES_IS_OK(vt_res)) {
        printf("      RES_IS_OK(vt_res) = true, value = %d\n", RES_UNWRAP_OK(vt_res));
    }
    
    // Combining convenience macros with pattern matching
    printf("   Combining macro checks with match:\n");
    Option_i32 maybe = Some(i32, 25);
    if (OPT_IS_SOME(maybe)) {
        match_option(maybe, i32, val,
            { printf("      Macro confirmed Some, match got: %d\n", val); },
            { printf("      Unexpected None\n"); }
        );
    }
    
    // Using OPT_UNWRAP_OR for safe defaults
    Option_i32 empty_opt = None(i32);
    i32 safe_val = OPT_UNWRAP_OR(empty_opt, 999);
    printf("   OPT_UNWRAP_OR(None, 999) = %d\n", safe_val);
    
    Result_i32_const_charp err_result = Err(i32, const_charp, "failed");
    i32 safe_res = RES_UNWRAP_OK_OR(err_result, -1);
    printf("   RES_UNWRAP_OK_OR(Err, -1) = %d\n", safe_res);
    
    // Example 7: Putting it together - state machine dispatch
    printf("\n7. Putting It Together - State Machine Dispatch:\n");
    ConnState state = ST_IDLE;
    const char *events[] = {"dial", "ack", "ping", "ack", "drop"};

    for (usize i = 0; i < 5; i++) {
        printf("   [%-10s] event \"%s\": ", state_name(state), events[i]);
        Option_i32 ev = parse_event(events[i]);
        match_option(ev, i32, code, {
            // Known event: attempt the transition, which itself may fail
            Result_i32_const_charp t = next_state(state, code);
            match_result(t, i32, const_charp, ns, e,
                { state = (ConnState)ns; printf("-> %s\n", state_name(state)); },
                { printf("ignored (%s)\n", e); }
            );
        }, {
            printf("unknown event, state unchanged\n");
        });
    }
    printf("   final state: %s\n", state_name(state));

    printf("\n=== Done ===\n");
    return 0;
}

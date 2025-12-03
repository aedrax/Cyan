/**
 * @file 09_panic_handler.c
 * @brief Example demonstrating panic handler functionality
 * 
 * This example shows how the Cyan library handles unrecoverable errors
 * through the panic mechanism, including:
 * - Default panic behavior (prints file, line, message to stderr)
 * - Panic triggers from Option unwrap on None
 * - Panic triggers from Result unwrap_ok on Err
 * - Panic triggers from Result unwrap_err on Ok
 * - How to define a custom panic handler
 * 
 * Compile: gcc -std=c11 -I../include -o panic_handler 09_panic_handler.c
 * Run: ./panic_handler
 * 
 * NOTE: This example will abort the program when a panic is triggered.
 *       Use the interactive menu to select which panic scenario to demonstrate.
 */

/*============================================================================
 * Custom Panic Handler Example (Commented Out)
 *============================================================================
 * To use a custom panic handler, you MUST define CYAN_PANIC before including
 * any Cyan headers. The custom handler receives a message string and should
 * handle the error appropriately (typically by logging and aborting).
 * 
 * Uncomment the following block to try a custom panic handler:
 */

/*
#define CYAN_PANIC(msg) do { \
    fprintf(stderr, "\n"); \
    fprintf(stderr, "╔══════════════════════════════════════════════════════════╗\n"); \
    fprintf(stderr, "║  CUSTOM PANIC HANDLER                                    ║\n"); \
    fprintf(stderr, "╠══════════════════════════════════════════════════════════╣\n"); \
    fprintf(stderr, "║  File: %-50s ║\n", __FILE__); \
    fprintf(stderr, "║  Line: %-50d ║\n", __LINE__); \
    fprintf(stderr, "║  Message: %-47s ║\n", msg); \
    fprintf(stderr, "╚══════════════════════════════════════════════════════════╝\n"); \
    abort(); \
} while(0)
*/

/*============================================================================
 * Includes
 *============================================================================*/

#include <stdio.h>
#include <cyan/common.h>
#include <cyan/option.h>
#include <cyan/result.h>

/*============================================================================
 * Type Definitions
 *============================================================================*/

/* Define Option and Result types for demonstration */
OPTION_DEFINE(i32);

typedef const char* const_charp;
RESULT_DEFINE(i32, const_charp);

/*============================================================================
 * Panic Trigger Functions
 *============================================================================
 * Each function demonstrates a different scenario that triggers a panic.
 * These are isolated in separate functions for clarity and to show the
 * exact line where the panic occurs.
 */

/**
 * @brief Trigger panic by calling unwrap() on a None Option
 * 
 * When you call unwrap() on an Option that contains no value (None),
 * the library panics because there is no valid value to return.
 * 
 * Default panic output:
 *   PANIC at 09_panic_handler.c:XX: unwrap called on None
 */
void trigger_panic_unwrap_none(void) {
    printf("\n--- Triggering panic: unwrap() on None Option ---\n");
    printf("Creating None Option and calling unwrap()...\n\n");
    
    Option_i32 empty = None(i32);
    
    /* This line will panic - there's no value to unwrap */
    i32 value = unwrap(empty);
    
    /* This line is never reached */
    printf("Value: %d\n", value);
}

/**
 * @brief Trigger panic by calling unwrap_ok() on an Err Result
 * 
 * When you call unwrap_ok() on a Result that contains an error (Err),
 * the library panics because there is no success value to return.
 * 
 * Default panic output:
 *   PANIC at 09_panic_handler.c:XX: unwrap_ok called on Err
 */
void trigger_panic_unwrap_ok_on_err(void) {
    printf("\n--- Triggering panic: unwrap_ok() on Err Result ---\n");
    printf("Creating Err Result and calling unwrap_ok()...\n\n");
    
    Result_i32_const_charp error_result = Err(i32, const_charp, "something went wrong");
    
    /* This line will panic - there's no Ok value to unwrap */
    i32 value = unwrap_ok(error_result);
    
    /* This line is never reached */
    printf("Value: %d\n", value);
}

/**
 * @brief Trigger panic by calling unwrap_err() on an Ok Result
 * 
 * When you call unwrap_err() on a Result that contains a success value (Ok),
 * the library panics because there is no error value to return.
 * 
 * Default panic output:
 *   PANIC at 09_panic_handler.c:XX: unwrap_err called on Ok
 */
void trigger_panic_unwrap_err_on_ok(void) {
    printf("\n--- Triggering panic: unwrap_err() on Ok Result ---\n");
    printf("Creating Ok Result and calling unwrap_err()...\n\n");
    
    Result_i32_const_charp success_result = Ok(i32, const_charp, 42);
    
    /* This line will panic - there's no Err value to unwrap */
    const char* error = unwrap_err(success_result);
    
    /* This line is never reached */
    printf("Error: %s\n", error);
}

/*============================================================================
 * Default Panic Behavior Demonstration
 *============================================================================*/

/**
 * @brief Display information about the default panic handler
 * 
 * The default CYAN_PANIC macro:
 * 1. Prints to stderr: "PANIC at <file>:<line>: <message>"
 * 2. Calls abort() to terminate the program
 * 
 * This behavior can be customized by defining CYAN_PANIC before including
 * any Cyan headers (see the commented example at the top of this file).
 */
void show_default_panic_info(void) {
    printf("=== Default Panic Handler Behavior ===\n\n");
    
    printf("The default CYAN_PANIC macro does the following:\n");
    printf("  1. Prints to stderr: \"PANIC at <file>:<line>: <message>\"\n");
    printf("  2. Calls abort() to terminate the program immediately\n\n");
    
    printf("Panic is triggered in these scenarios:\n");
    printf("  - unwrap() on a None Option\n");
    printf("  - unwrap_ok() on an Err Result\n");
    printf("  - unwrap_err() on an Ok Result\n");
    printf("  - Memory allocation failures in Cyan collections\n");
    printf("  - Resuming a finished coroutine\n\n");
    
    printf("To customize panic behavior, define CYAN_PANIC before including\n");
    printf("any Cyan headers. See the commented example at the top of this file.\n\n");
}

/*============================================================================
 * Interactive Menu
 *============================================================================*/

/**
 * @brief Display the interactive menu and handle user selection
 * 
 * This menu allows users to choose which panic scenario to trigger,
 * preventing accidental panics when running the example.
 */
void show_menu(void) {
    printf("=== Panic Scenario Selection ===\n\n");
    printf("Select a panic scenario to demonstrate:\n\n");
    printf("  [1] unwrap() on None Option\n");
    printf("      - Creates a None Option and calls unwrap()\n\n");
    printf("  [2] unwrap_ok() on Err Result\n");
    printf("      - Creates an Err Result and calls unwrap_ok()\n\n");
    printf("  [3] unwrap_err() on Ok Result\n");
    printf("      - Creates an Ok Result and calls unwrap_err()\n\n");
    printf("  [0] Exit without triggering panic\n\n");
    printf("Enter your choice (0-3): ");
}

i32 main(void) {
    printf("=== Panic Handler Example ===\n\n");
    
    /* First, show information about the default panic behavior */
    show_default_panic_info();
    
    /* Show the interactive menu */
    show_menu();
    
    /* Read user selection */
    i32 choice = 0;
    if (scanf("%d", &choice) != 1) {
        printf("Invalid input. Exiting.\n");
        return 1;
    }
    
    /* Execute the selected panic scenario */
    switch (choice) {
        case 0:
            printf("\nExiting without triggering panic.\n");
            printf("=== Done ===\n");
            return 0;
            
        case 1:
            trigger_panic_unwrap_none();
            break;
            
        case 2:
            trigger_panic_unwrap_ok_on_err();
            break;
            
        case 3:
            trigger_panic_unwrap_err_on_ok();
            break;
            
        default:
            printf("\nInvalid choice. Please enter 0-3.\n");
            return 1;
    }
    
    /* This line is never reached if a panic was triggered */
    printf("\n=== Done ===\n");
    return 0;
}

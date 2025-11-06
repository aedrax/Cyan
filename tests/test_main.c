/**
 * @file test_main.c
 * @brief Main test runner for Cyan library property-based tests
 * 
 * This file provides the entry point for running all property-based tests
 * using the theft library. Tests are organized by component and each test
 * validates specific correctness properties.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "theft.h"
#include <cyan/common.h>

/* External test functions from component test files */
extern int run_option_tests(theft_seed seed);
extern int run_result_tests(theft_seed seed);
extern int run_vector_tests(theft_seed seed);
extern int run_slice_tests(theft_seed seed);
extern int run_functional_tests(theft_seed seed);
extern int run_defer_tests(theft_seed seed);
extern int run_coro_tests(theft_seed seed);
extern int run_serialize_tests(theft_seed seed);
extern int run_smartptr_tests(theft_seed seed);

/*============================================================================
 * Test Configuration
 *============================================================================*/

/* Minimum iterations for property tests */
#define MIN_TEST_TRIALS 100

/* Default seed for reproducibility (0 = use time-based seed) */
static theft_seed g_seed = 0;

/* Filter for running specific tests */
static const char *g_filter = NULL;

/*============================================================================
 * Test Result Tracking
 *============================================================================*/

typedef struct {
    int total;
    int passed;
    int failed;
    int skipped;
} TestResults;

static TestResults g_results = {0};

/*============================================================================
 * Test Utilities
 *============================================================================*/

/**
 * @brief Check if a test name matches the filter
 */
static int test_matches_filter(const char *name) {
    if (g_filter == NULL) return 1;
    return strstr(name, g_filter) != NULL;
}

/**
 * @brief Print test result
 */
static void print_result(const char *name, enum theft_run_res result) {
    const char *status;
    switch (result) {
        case THEFT_RUN_PASS:
            status = "\033[32mPASS\033[0m";
            g_results.passed++;
            break;
        case THEFT_RUN_FAIL:
            status = "\033[31mFAIL\033[0m";
            g_results.failed++;
            break;
        case THEFT_RUN_SKIP:
            status = "\033[33mSKIP\033[0m";
            g_results.skipped++;
            break;
        default:
            status = "\033[31mERROR\033[0m";
            g_results.failed++;
            break;
    }
    printf("  [%s] %s\n", status, name);
    g_results.total++;
}

/*============================================================================
 * Placeholder Test (validates test infrastructure)
 *============================================================================*/

/**
 * Feature: modern-c11-header-library, Property 0: Infrastructure test
 * Validates that the test framework is working correctly
 */
static enum theft_trial_res prop_infrastructure(struct theft *t, void *arg) {
    (void)t;
    (void)arg;
    /* Simple sanity check - always passes */
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Test Runner
 *============================================================================*/

/**
 * @brief Run a single property test with 1 argument
 */
static enum theft_run_res run_test1(
    const char *name,
    theft_propfun1 *prop,
    const struct theft_type_info *type_info
) {
    if (!test_matches_filter(name)) {
        g_results.skipped++;
        g_results.total++;
        printf("  [\033[33mSKIP\033[0m] %s\n", name);
        return THEFT_RUN_SKIP;
    }

    theft_seed seed = g_seed ? g_seed : theft_seed_of_time();
    
    struct theft_run_config config = {
        .name = name,
        .prop1 = prop,
        .type_info = { type_info },
        .trials = MIN_TEST_TRIALS,
        .seed = seed,
        .hooks = {
            .trial_post = theft_hook_trial_post_print_result,
        },
    };

    enum theft_run_res res = theft_run(&config);
    print_result(name, res);
    return res;
}

/**
 * @brief Run all tests
 */
static void run_all_tests(void) {
    printf("\n=== Cyan Library Property-Based Tests ===\n\n");
    
    if (g_seed) {
        printf("Using seed: %lu\n\n", (unsigned long)g_seed);
    }

    /* Infrastructure test - uses built-in bool type */
    printf("Infrastructure:\n");
    run_test1(
        "Property 0: Test framework operational",
        prop_infrastructure,
        theft_get_builtin_type_info(THEFT_BUILTIN_bool)
    );

    /* Option type tests */
    theft_seed seed = g_seed ? g_seed : theft_seed_of_time();
    int option_failures = run_option_tests(seed);
    g_results.failed += option_failures;
    g_results.passed += (4 - option_failures);  /* 4 option tests */
    g_results.total += 4;

    /* Result type tests */
    int result_failures = run_result_tests(seed);
    g_results.failed += result_failures;
    g_results.passed += (4 - result_failures);  /* 4 result tests */
    g_results.total += 4;

    /* Vector type tests */
    int vector_failures = run_vector_tests(seed);
    g_results.failed += vector_failures;
    g_results.passed += (4 - vector_failures);  /* 4 vector tests */
    g_results.total += 4;

    /* Slice type tests */
    int slice_failures = run_slice_tests(seed);
    g_results.failed += slice_failures;
    g_results.passed += (3 - slice_failures);  /* 3 slice tests */
    g_results.total += 3;

    /* Functional primitives tests */
    int functional_failures = run_functional_tests(seed);
    g_results.failed += functional_failures;
    g_results.passed += (4 - functional_failures);  /* 4 functional tests */
    g_results.total += 4;

    /* Defer mechanism tests */
    int defer_failures = run_defer_tests(seed);
    g_results.failed += defer_failures;
    g_results.passed += (4 - defer_failures);  /* 4 defer tests */
    g_results.total += 4;

    /* Coroutine tests */
    int coro_failures = run_coro_tests(seed);
    g_results.failed += coro_failures;
    g_results.passed += (4 - coro_failures);  /* 4 coro tests */
    g_results.total += 4;

    /* Serialization tests */
    int serialize_failures = run_serialize_tests(seed);
    g_results.failed += serialize_failures;
    g_results.passed += (7 - serialize_failures);  /* 7 serialize tests */
    g_results.total += 7;

    /* Smart pointer tests */
    int smartptr_failures = run_smartptr_tests(seed);
    g_results.failed += smartptr_failures;
    g_results.passed += (11 - smartptr_failures);  /* 11 smartptr tests */
    g_results.total += 11;

    printf("\n");
}

/*============================================================================
 * Main Entry Point
 *============================================================================*/

static void print_usage(const char *prog) {
    printf("Usage: %s [options]\n", prog);
    printf("\nOptions:\n");
    printf("  --seed <n>     Use specific seed for reproducibility\n");
    printf("  --filter <s>   Run only tests matching <s>\n");
    printf("  --help         Show this help\n");
}

int main(int argc, char *argv[]) {
    /* Parse command line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            g_seed = (theft_seed)strtoul(argv[++i], NULL, 10);
        } else if (strcmp(argv[i], "--filter") == 0 && i + 1 < argc) {
            g_filter = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    /* Run tests */
    run_all_tests();

    /* Print summary */
    printf("=== Summary ===\n");
    printf("Total: %d, Passed: %d, Failed: %d, Skipped: %d\n",
           g_results.total, g_results.passed, g_results.failed, g_results.skipped);

    return g_results.failed > 0 ? 1 : 0;
}

/**
 * @file 07_functional.c
 * @brief Example demonstrating functional programming primitives
 * 
 * Compile: gcc -std=c11 -I../include -o functional_demo 07_functional.c
 * Run: ./functional_demo
 */

#include <stdio.h>
#include <cyan/common.h>
#include <cyan/option.h>
#include <cyan/vector.h>
#include <cyan/functional.h>

// Define types using primitive aliases
OPTION_DEFINE(i32);
OPTION_DEFINE(f64);
VECTOR_DEFINE(i32);
VECTOR_DEFINE(f64);

// Define vector map function for i32 -> f64
VEC_MAP_DEFINE(i32, f64);

// Transformation functions
i32 square(i32 x) { return x * x; }
i32 double_it(i32 x) { return x * 2; }
f64 to_f64(i32 x) { return (f64)x; }

// Predicate functions
bool is_even(i32 x) { return x % 2 == 0; }
bool is_positive(i32 x) { return x > 0; }
bool greater_than_10(i32 x) { return x > 10; }

// Accumulator functions
i32 add(i32 a, i32 b) { return a + b; }
i32 multiply(i32 a, i32 b) { return a * b; }
i32 max_val(i32 a, i32 b) { return a > b ? a : b; }

// Side-effect function
void print_i32(i32 x) { printf("%d ", x); }

// Pipeline helper for example 8: readings are stored in tenths of a degree
i32 tenths_to_degrees(i32 t) { return t / 10; }

i32 main(void) {
    printf("=== Functional Programming Examples ===\n\n");
    
    i32 numbers[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    usize len = 10;
    
    // Example 1: Map
    printf("1. Map - Transform Elements:\n");
    printf("   Original: ");
    foreach(numbers, len, print_i32);
    printf("\n");
    
    i32 squared[10];
    map(numbers, len, squared, square);
    printf("   Squared:  ");
    foreach(squared, len, print_i32);
    printf("\n");
    
    i32 doubled[10];
    map(numbers, len, doubled, double_it);
    printf("   Doubled:  ");
    foreach(doubled, len, print_i32);
    printf("\n");
    
    // Example 2: Filter
    printf("\n2. Filter - Select Elements:\n");
    i32 evens[10];
    usize evens_len;
    filter(numbers, len, evens, &evens_len, is_even);
    printf("   Even numbers: ");
    foreach(evens, evens_len, print_i32);
    printf("(count: %zu)\n", evens_len);
    
    i32 big[10];
    usize big_len;
    filter(numbers, len, big, &big_len, greater_than_10);
    printf("   Greater than 10: ");
    if (big_len == 0) {
        printf("(none)");
    } else {
        foreach(big, big_len, print_i32);
    }
    printf(" (count: %zu)\n", big_len);
    
    // Example 3: Reduce
    printf("\n3. Reduce - Combine Elements:\n");
    i32 sum;
    reduce(sum, numbers, len, 0, add);
    printf("   Sum: %d\n", sum);
    
    i32 product;
    reduce(product, numbers, len, 1, multiply);
    printf("   Product: %d\n", product);
    
    i32 maximum;
    reduce(maximum, numbers, len, numbers[0], max_val);
    printf("   Maximum: %d\n", maximum);
    
    // Example 4: Foreach
    printf("\n4. Foreach - Side Effects:\n");
    printf("   Printing each: ");
    foreach(numbers, len, print_i32);
    printf("\n");
    
    // Example 5: Chaining operations
    printf("\n5. Chaining Operations:\n");
    // Filter evens, then square them, then sum
    i32 temp[10];
    usize temp_len;
    filter(numbers, len, temp, &temp_len, is_even);
    
    i32 temp_squared[10];
    map(temp, temp_len, temp_squared, square);
    
    i32 sum_of_squared_evens;
    reduce(sum_of_squared_evens, temp_squared, temp_len, 0, add);
    printf("   Sum of squared evens: %d\n", sum_of_squared_evens);
    printf("   (2² + 4² + 6² + 8² + 10² = 4 + 16 + 36 + 64 + 100 = 220)\n");
    
    // Example 6: Working with vectors
    printf("\n6. Vector Map:\n");
    Vec_i32 v = vec_i32_new();
    for (i32 i = 1; i <= 5; i++) {
        vec_i32_push(&v, i);
    }
    
    printf("   Original vector: ");
    for (usize i = 0; i < vec_i32_len(&v); i++) {
        printf("%d ", unwrap(vec_i32_get(&v, i)));
    }
    printf("\n");
    
    Vec_f64 v_doubled = vec_map_i32_to_f64(&v, to_f64);
    printf("   As doubles: ");
    for (usize i = 0; i < vec_f64_len(&v_doubled); i++) {
        printf("%.1f ", unwrap(vec_f64_get(&v_doubled, i)));
    }
    printf("\n");
    
    vec_i32_free(&v);
    vec_f64_free(&v_doubled);
    
    // Example 7: Convenience Macros with Functional Operations
    printf("\n7. Convenience Macros with Functional Operations:\n");

    // Create vector using type-first convenience macros
    Vec_i32 mv = vec_i32_new();
    for (i32 i = 1; i <= 5; i++) {
        VEC_PUSH(i32, mv, i * 10);
    }

    printf("   Vector created with VEC_PUSH: ");
    for (usize i = 0; i < VEC_LEN(i32, mv); i++) {
        Option_i32 elem = VEC_GET(i32, mv, i);
        if (OPT_IS_SOME(elem)) {
            printf("%d ", OPT_UNWRAP(elem));
        }
    }
    printf("\n");

    // Copy elements out, then apply functional operations
    printf("   Applying map (square):\n");
    i32 mv_data[5];
    for (usize i = 0; i < VEC_LEN(i32, mv); i++) {
        Option_i32 elem = VEC_GET(i32, mv, i);
        if (OPT_IS_SOME(elem)) {
            mv_data[i] = OPT_UNWRAP(elem);
        }
    }

    i32 mv_squared[5];
    map(mv_data, 5, mv_squared, square);
    printf("      Squared: ");
    foreach(mv_squared, 5, print_i32);
    printf("\n");

    // Filter and reduce over the copied data
    i32 mv_evens[5];
    usize mv_evens_len;
    filter(mv_data, 5, mv_evens, &mv_evens_len, is_even);
    printf("   Filtered evens: ");
    foreach(mv_evens, mv_evens_len, print_i32);
    printf("(count: %zu)\n", mv_evens_len);

    i32 mv_sum;
    reduce(mv_sum, mv_data, 5, 0, add);
    printf("   Sum via reduce: %d\n", mv_sum);

    VEC_FREE(i32, mv);

    // Example 8: Putting it together - a sensor data pipeline
    printf("\n8. Putting It Together - Sensor Data Pipeline:\n");
    // Raw temperature readings in tenths of a degree; -999 marks a bad sample
    i32 readings[] = {215, -999, 187, 240, -999, 198, 305, 172};
    usize n_read = 8;

    printf("   raw readings:      ");
    foreach(readings, n_read, print_i32);
    printf("\n");

    // Stage 1: filter out sensor errors
    i32 valid[8];
    usize n_valid;
    filter(readings, n_read, valid, &n_valid, is_positive);
    printf("   valid samples (%zu): ", n_valid);
    foreach(valid, n_valid, print_i32);
    printf("\n");

    // Stage 2: map tenths -> whole degrees
    i32 degrees[8];
    map(valid, n_valid, degrees, tenths_to_degrees);
    printf("   in degrees:        ");
    foreach(degrees, n_valid, print_i32);
    printf("\n");

    // Stage 3: reduce to summary statistics
    i32 total;
    reduce(total, degrees, n_valid, 0, add);
    i32 hottest;
    reduce(hottest, degrees, n_valid, degrees[0], max_val);
    printf("   average: %d degrees, hottest: %d degrees\n",
           total / (i32)n_valid, hottest);

    printf("\n=== Done ===\n");
    return 0;
}

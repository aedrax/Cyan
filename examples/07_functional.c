/**
 * @file 07_functional.c
 * @brief Example demonstrating functional programming primitives
 * 
 * Compile: gcc -std=c11 -I../include -o functional_demo 07_functional.c
 * Run: ./functional_demo
 */

#include <stdio.h>
#include <cyan/option.h>
#include <cyan/vector.h>
#include <cyan/functional.h>

// Define types
OPTION_DEFINE(int);
OPTION_DEFINE(double);
VECTOR_DEFINE(int);
VECTOR_DEFINE(double);

// Define vector map function for int -> double
VEC_MAP_DEFINE(int, double);

// Transformation functions
int square(int x) { return x * x; }
int double_it(int x) { return x * 2; }
double to_double(int x) { return (double)x; }

// Predicate functions
bool is_even(int x) { return x % 2 == 0; }
bool is_positive(int x) { return x > 0; }
bool greater_than_10(int x) { return x > 10; }

// Accumulator functions
int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }
int max_val(int a, int b) { return a > b ? a : b; }

// Side-effect function
void print_int(int x) { printf("%d ", x); }

int main(void) {
    printf("=== Functional Programming Examples ===\n\n");
    
    int numbers[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    size_t len = 10;
    
    // Example 1: Map
    printf("1. Map - Transform Elements:\n");
    printf("   Original: ");
    foreach(numbers, len, print_int);
    printf("\n");
    
    int squared[10];
    map(numbers, len, squared, square);
    printf("   Squared:  ");
    foreach(squared, len, print_int);
    printf("\n");
    
    int doubled[10];
    map(numbers, len, doubled, double_it);
    printf("   Doubled:  ");
    foreach(doubled, len, print_int);
    printf("\n");
    
    // Example 2: Filter
    printf("\n2. Filter - Select Elements:\n");
    int evens[10];
    size_t evens_len;
    filter(numbers, len, evens, &evens_len, is_even);
    printf("   Even numbers: ");
    foreach(evens, evens_len, print_int);
    printf("(count: %zu)\n", evens_len);
    
    int big[10];
    size_t big_len;
    filter(numbers, len, big, &big_len, greater_than_10);
    printf("   Greater than 10: ");
    if (big_len == 0) {
        printf("(none)");
    } else {
        foreach(big, big_len, print_int);
    }
    printf(" (count: %zu)\n", big_len);
    
    // Example 3: Reduce
    printf("\n3. Reduce - Combine Elements:\n");
    int sum;
    reduce(sum, numbers, len, 0, add);
    printf("   Sum: %d\n", sum);
    
    int product;
    reduce(product, numbers, len, 1, multiply);
    printf("   Product: %d\n", product);
    
    int maximum;
    reduce(maximum, numbers, len, numbers[0], max_val);
    printf("   Maximum: %d\n", maximum);
    
    // Example 4: Foreach
    printf("\n4. Foreach - Side Effects:\n");
    printf("   Printing each: ");
    foreach(numbers, len, print_int);
    printf("\n");
    
    // Example 5: Chaining operations
    printf("\n5. Chaining Operations:\n");
    // Filter evens, then square them, then sum
    int temp[10];
    size_t temp_len;
    filter(numbers, len, temp, &temp_len, is_even);
    
    int temp_squared[10];
    map(temp, temp_len, temp_squared, square);
    
    int sum_of_squared_evens;
    reduce(sum_of_squared_evens, temp_squared, temp_len, 0, add);
    printf("   Sum of squared evens: %d\n", sum_of_squared_evens);
    printf("   (2² + 4² + 6² + 8² + 10² = 4 + 16 + 36 + 64 + 100 = 220)\n");
    
    // Example 6: Working with vectors
    printf("\n6. Vector Map:\n");
    Vec_int v = vec_int_new();
    for (int i = 1; i <= 5; i++) {
        vec_int_push(&v, i);
    }
    
    printf("   Original vector: ");
    for (size_t i = 0; i < vec_int_len(&v); i++) {
        printf("%d ", unwrap(vec_int_get(&v, i)));
    }
    printf("\n");
    
    Vec_double v_doubled = vec_map_int_to_double(&v, to_double);
    printf("   As doubles: ");
    for (size_t i = 0; i < vec_double_len(&v_doubled); i++) {
        printf("%.1f ", unwrap(vec_double_get(&v_doubled, i)));
    }
    printf("\n");
    
    vec_int_free(&v);
    vec_double_free(&v_doubled);
    
    printf("\n=== Done ===\n");
    return 0;
}

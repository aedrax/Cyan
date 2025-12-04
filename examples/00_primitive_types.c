/**
 * @file 00_primitive_types.c
 * @brief Example demonstrating primitive type aliases
 * 
 * Compile: gcc -std=c11 -I../include -o primitive_types 08_primitive_types.c
 * Run: ./primitive_types
 */

#include <stdio.h>
#include <cyan/common.h>

int main(void) {
    printf("=== Primitive Type Aliases ===\n\n");
    
    // Example 1: Fixed-width signed integers
    printf("1. Signed Integer Types:\n");
    i8  a = 127;           // 8-bit signed (-128 to 127)
    i16 b = 32767;         // 16-bit signed
    i32 c = 2147483647;    // 32-bit signed
    i64 d = 9223372036854775807LL;  // 64-bit signed
    
    printf("   i8  max:  %d (size: %zu bytes)\n", a, sizeof(i8));
    printf("   i16 max:  %d (size: %zu bytes)\n", b, sizeof(i16));
    printf("   i32 max:  %d (size: %zu bytes)\n", c, sizeof(i32));
    printf("   i64 max:  %lld (size: %zu bytes)\n", (long long)d, sizeof(i64));
    
#if CYAN_HAS_INT128
    printf("   i128 available: yes (size: %zu bytes)\n", sizeof(i128));
#else
    printf("   i128 available: no (platform limitation)\n");
#endif
    
    // Example 2: Fixed-width unsigned integers
    printf("\n2. Unsigned Integer Types:\n");
    u8  e = 255;           // 8-bit unsigned (0 to 255)
    u16 f = 65535;         // 16-bit unsigned
    u32 g = 4294967295U;   // 32-bit unsigned
    u64 h = 18446744073709551615ULL;  // 64-bit unsigned
    
    printf("   u8  max:  %u (size: %zu bytes)\n", e, sizeof(u8));
    printf("   u16 max:  %u (size: %zu bytes)\n", f, sizeof(u16));
    printf("   u32 max:  %u (size: %zu bytes)\n", g, sizeof(u32));
    printf("   u64 max:  %llu (size: %zu bytes)\n", (unsigned long long)h, sizeof(u64));
    
    // Example 3: Pointer-sized integers
    printf("\n3. Pointer-Sized Types:\n");
    i32 value = 42;
    usize ptr_val = (usize)&value;
    isize offset = -100;
    
    printf("   usize (pointer as int): 0x%lx (size: %zu bytes)\n", 
           (unsigned long)ptr_val, sizeof(usize));
    printf("   isize (signed offset):  %ld (size: %zu bytes)\n", 
           (long)offset, sizeof(isize));
    printf("   sizeof(void*) = %zu bytes\n", sizeof(void*));
    
    // Example 4: Floating-point types
    printf("\n4. Floating-Point Types:\n");
    f32 pi_f32 = 3.14159265f;
    f64 pi_f64 = 3.141592653589793;
    
    printf("   f32 (float):  %.7f (size: %zu bytes)\n", pi_f32, sizeof(f32));
    printf("   f64 (double): %.15f (size: %zu bytes)\n", pi_f64, sizeof(f64));
    
#if CYAN_HAS_FLOAT80
    f80 pi_f80 = 3.14159265358979323846L;
    printf("   f80 (long double): %.18Lf (size: %zu bytes)\n", pi_f80, sizeof(f80));
#else
    printf("   f80 available: no (platform limitation)\n");
#endif

#if CYAN_HAS_FLOAT16
    printf("   f16 available: yes (size: %zu bytes)\n", sizeof(f16));
#else
    printf("   f16 available: no (platform limitation)\n");
#endif

#if CYAN_HAS_FLOAT128
    printf("   f128 available: yes (size: %zu bytes)\n", sizeof(f128));
#else
    printf("   f128 available: no (platform limitation)\n");
#endif
    
    // Example 5: Type-erased pointers
    printf("\n5. Type-Erased Pointers (any):\n");
    i32 int_val = 100;
    f64 float_val = 3.14;
    
    any* generic_ptr;
    
    generic_ptr = &int_val;
    printf("   Stored i32: %d\n", *(i32*)generic_ptr);
    
    generic_ptr = &float_val;
    printf("   Stored f64: %.2f\n", *(f64*)generic_ptr);
    
    // Example 6: Practical usage - byte manipulation
    printf("\n6. Byte Manipulation:\n");
    u32 color = 0xFF5733FF;  // RGBA color
    
    u8 red = (color >> 24) & 0xFF;
    u8 green = (color >> 16) & 0xFF;
    u8 blue = (color >> 8) & 0xFF;
    u8 alpha = color & 0xFF;
    
    printf("   Color 0x%08X -> R:%u G:%u B:%u A:%u\n", color, red, green, blue, alpha);
    
    // Example 7: Array indexing with usize
    printf("\n7. Array Indexing with usize:\n");
    i32 numbers[] = {10, 20, 30, 40, 50};
    usize len = sizeof(numbers) / sizeof(numbers[0]);
    
    printf("   Array (len=%zu): ", len);
    for (usize i = 0; i < len; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\n");
    
    printf("\n=== Done ===\n");
    return 0;
}

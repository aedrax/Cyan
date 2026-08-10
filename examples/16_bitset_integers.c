/**
 * @file 16_bitset_integers.c
 * @brief Example demonstrating bitsets and custom bit-width integers
 * 
 * Compile: gcc -std=gnu11 -I../include -o 16_bitset_integers 16_bitset_integers.c
 * Run: ./16_bitset_integers
 */

#include <stdio.h>
#include <cyan/bitint.h>
#include <cyan/bitset.h>

// Define custom bit-width integers
UINT_DEFINE(6);   // u6: 6-bit unsigned (0-63)
UINT_DEFINE(12);  // u12: 12-bit unsigned (0-4095)
INT_DEFINE(6);    // i6: 6-bit signed (-32 to 31)
INT_DEFINE(12);   // i12: 12-bit signed (-2048 to 2047)

// Define bitsets
BITSET_DEFINE(8);  // Bitset_8: 8-bit bitset
BITSET_DEFINE(16); // Bitset_16: 16-bit bitset
BITSET_DEFINE(4);  // Bitset_4: 4-bit bitset for permissions

// Define named flags for file permissions
FLAGS_DEFINE(FilePerms, READ, WRITE, EXECUTE, HIDDEN);

// Saturating add built on the public u6 API: clamps at u6_max() (63)
// instead of wrapping around to 0 (used in part 8)
u6 u6_add_sat(u6 a, u6 b) {
    u6 headroom = u6_sub(u6_max(), a);   // room left before 63
    if (u6_lt(headroom, b)) return u6_max();
    return u6_add(a, b);
}

int main(void) {
    printf("=== Bitsets and Custom Bit-Width Integers ===\n\n");

    // =========================================================================
    // Part 1: Custom Unsigned Integers
    // =========================================================================
    printf("1. Custom Unsigned Integers (u6, u12):\n");
    
    // u6: 6-bit unsigned integer (0-63)
    u6 small = u6_new(42);
    printf("   u6_new(42) = %u\n", u6_get(&small));
    
    // Values are automatically masked to fit
    u6 overflow = u6_new(100);  // 100 & 0x3F = 36
    printf("   u6_new(100) = %u (masked to 6 bits)\n", u6_get(&overflow));
    
    // Arithmetic operations
    u6 a = u6_new(30);
    u6 b = u6_new(40);
    u6 sum = u6_add(a, b);  // (30 + 40) & 0x3F = 70 & 63 = 6
    printf("   u6: 30 + 40 = %u (wraps at 64)\n", u6_get(&sum));
    
    // u12: 12-bit unsigned integer (0-4095)
    u12 medium = u12_new(2048);
    printf("   u12_new(2048) = %u\n", u12_get(&medium));
    
    // Min/max values
    u6 u6_min_val = u6_min();
    u6 u6_max_val = u6_max();
    u12 u12_min_val = u12_min();
    u12 u12_max_val = u12_max();
    printf("   u6 range: %u to %u\n", u6_get(&u6_min_val), u6_get(&u6_max_val));
    printf("   u12 range: %u to %u\n", u12_get(&u12_min_val), u12_get(&u12_max_val));

    // =========================================================================
    // Part 2: Custom Signed Integers
    // =========================================================================
    printf("\n2. Custom Signed Integers (i6, i12):\n");
    
    // i6: 6-bit signed integer (-32 to 31)
    i6 pos = i6_new(20);
    i6 neg = i6_new(-15);
    printf("   i6_new(20) = %d\n", i6_get(&pos));
    printf("   i6_new(-15) = %d\n", i6_get(&neg));
    
    // Arithmetic with sign extension
    i6 diff = i6_sub(pos, neg);  // 20 - (-15) = 35, wraps to -29
    printf("   i6: 20 - (-15) = %d (wraps in 6-bit signed)\n", i6_get(&diff));
    
    // Negation
    i6 negated = i6_neg(pos);
    printf("   i6: -20 = %d\n", i6_get(&negated));
    
    // i12: 12-bit signed integer (-2048 to 2047)
    i12 large_neg = i12_new(-1000);
    printf("   i12_new(-1000) = %d\n", i12_get(&large_neg));
    
    // Min/max values
    i6 i6_min_val = i6_min();
    i6 i6_max_val = i6_max();
    i12 i12_min_val = i12_min();
    i12 i12_max_val = i12_max();
    printf("   i6 range: %d to %d\n", i6_get(&i6_min_val), i6_get(&i6_max_val));
    printf("   i12 range: %d to %d\n", i12_get(&i12_min_val), i12_get(&i12_max_val));

    // =========================================================================
    // Part 3: Basic Bitset Operations
    // =========================================================================
    printf("\n3. Basic Bitset Operations:\n");
    
    Bitset_8 bs = bitset_8_new();
    printf("   New bitset: count=%u, none=%s\n", 
           bitset_8_count(&bs), bitset_8_none(&bs) ? "true" : "false");
    
    // Set individual bits
    bitset_8_set(&bs, 0);  // Set bit 0
    bitset_8_set(&bs, 3);  // Set bit 3
    bitset_8_set(&bs, 7);  // Set bit 7
    printf("   After setting bits 0, 3, 7: count=%u\n", bitset_8_count(&bs));
    
    // Check bits
    printf("   Bit 0: %s, Bit 1: %s, Bit 3: %s\n",
           bitset_8_get(&bs, 0) ? "set" : "clear",
           bitset_8_get(&bs, 1) ? "set" : "clear",
           bitset_8_get(&bs, 3) ? "set" : "clear");
    
    // Toggle bits
    bitset_8_toggle(&bs, 3);  // Clear bit 3
    bitset_8_toggle(&bs, 4);  // Set bit 4
    printf("   After toggling bits 3, 4: count=%u\n", bitset_8_count(&bs));
    
    // Clear bits
    bitset_8_clear(&bs, 0);
    printf("   After clearing bit 0: count=%u\n", bitset_8_count(&bs));

    // =========================================================================
    // Part 4: Set Operations
    // =========================================================================
    printf("\n4. Set Operations:\n");
    
    Bitset_8 set1 = bitset_8_from_raw(0b00001111);  // Bits 0-3
    Bitset_8 set2 = bitset_8_from_raw(0b00111100);  // Bits 2-5
    
    printf("   Set1 (bits 0-3): 0x%02X\n", set1.bits);
    printf("   Set2 (bits 2-5): 0x%02X\n", set2.bits);
    
    // Union (OR)
    Bitset_8 union_set = bitset_8_union(&set1, &set2);
    printf("   Union:      0x%02X (bits 0-5)\n", union_set.bits);
    
    // Intersection (AND)
    Bitset_8 intersect_set = bitset_8_intersect(&set1, &set2);
    printf("   Intersect:  0x%02X (bits 2-3)\n", intersect_set.bits);
    
    // Difference (set1 - set2)
    Bitset_8 diff_set = bitset_8_diff(&set1, &set2);
    printf("   Difference: 0x%02X (bits 0-1)\n", diff_set.bits);
    
    // Complement
    Bitset_8 comp_set = bitset_8_complement(&set1);
    printf("   Complement: 0x%02X (bits 4-7)\n", comp_set.bits);

    // =========================================================================
    // Part 5: Utility Functions
    // =========================================================================
    printf("\n5. Utility Functions:\n");
    
    Bitset_8 full = bitset_8_from_raw(0xFF);
    Bitset_8 empty = bitset_8_new();
    Bitset_8 partial = bitset_8_from_raw(0b10101010);
    
    printf("   Full bitset:    all=%s, any=%s, none=%s, count=%u\n",
           bitset_8_all(&full) ? "true" : "false",
           bitset_8_any(&full) ? "true" : "false",
           bitset_8_none(&full) ? "true" : "false",
           bitset_8_count(&full));
    
    printf("   Empty bitset:   all=%s, any=%s, none=%s, count=%u\n",
           bitset_8_all(&empty) ? "true" : "false",
           bitset_8_any(&empty) ? "true" : "false",
           bitset_8_none(&empty) ? "true" : "false",
           bitset_8_count(&empty));
    
    printf("   Partial bitset: all=%s, any=%s, none=%s, count=%u\n",
           bitset_8_all(&partial) ? "true" : "false",
           bitset_8_any(&partial) ? "true" : "false",
           bitset_8_none(&partial) ? "true" : "false",
           bitset_8_count(&partial));

    // =========================================================================
    // Part 6: Named Flags
    // =========================================================================
    printf("\n6. Named Flags (File Permissions):\n");
    
    // FilePerms_COUNT is 4, so we use Bitset_4
    Bitset_4 perms = bitset_4_new();
    
    // Set permissions using named flags
    FLAGS_SET(4, perms, FilePerms_READ);
    FLAGS_SET(4, perms, FilePerms_WRITE);
    
    printf("   Permissions set: READ, WRITE\n");
    printf("   Has READ:    %s\n", FLAGS_HAS(4, perms, FilePerms_READ) ? "yes" : "no");
    printf("   Has WRITE:   %s\n", FLAGS_HAS(4, perms, FilePerms_WRITE) ? "yes" : "no");
    printf("   Has EXECUTE: %s\n", FLAGS_HAS(4, perms, FilePerms_EXECUTE) ? "yes" : "no");
    printf("   Has HIDDEN:  %s\n", FLAGS_HAS(4, perms, FilePerms_HIDDEN) ? "yes" : "no");
    
    // Clear a permission
    FLAGS_CLEAR(4, perms, FilePerms_WRITE);
    printf("   After clearing WRITE:\n");
    printf("   Has WRITE:   %s\n", FLAGS_HAS(4, perms, FilePerms_WRITE) ? "yes" : "no");

    // =========================================================================
    // Part 7: Convenience Macros
    // =========================================================================
    printf("\n7. Type-First Convenience Macros:\n");
    
    Bitset_8 vt_bs = bitset_8_new();
    
    // BS_* macros take the bit width first, then the bitset
    BS_SET(8, vt_bs, 1);
    BS_SET(8, vt_bs, 5);
    printf("   After BS_SET(1, 5): count=%u\n", BS_COUNT(8, vt_bs));
    printf("   BS_GET(1)=%s, BS_GET(2)=%s\n",
           BS_GET(8, vt_bs, 1) ? "true" : "false",
           BS_GET(8, vt_bs, 2) ? "true" : "false");
    
    BS_TOGGLE(8, vt_bs, 1);
    printf("   After BS_TOGGLE(1): BS_GET(1)=%s\n",
           BS_GET(8, vt_bs, 1) ? "true" : "false");
    
    printf("   BS_ANY=%s, BS_NONE=%s\n",
           BS_ANY(8, vt_bs) ? "true" : "false",
           BS_NONE(8, vt_bs) ? "true" : "false");

    // =========================================================================
    // Part 8: Putting It Together - Permission Gate + Saturating Counter
    // =========================================================================
    printf("\n8. Putting It Together - Permission Gate + Saturating Counter:\n");

    // An editor session needs READ and WRITE, and the file must not be HIDDEN
    Bitset_4 session = bitset_4_new();
    FLAGS_SET(4, session, FilePerms_READ);
    FLAGS_SET(4, session, FilePerms_WRITE);

    Bitset_4 required = bitset_4_new();
    FLAGS_SET(4, required, FilePerms_READ);
    FLAGS_SET(4, required, FilePerms_WRITE);

    // The gate: (session AND required) == required, and HIDDEN clear
    Bitset_4 granted = BS_INTERSECT(4, session, required);
    bool can_edit = BS_EQ(4, granted, required)
                 && !FLAGS_HAS(4, session, FilePerms_HIDDEN);
    printf("   session may edit: %s\n", can_edit ? "yes" : "no");

    FLAGS_CLEAR(4, session, FilePerms_WRITE);
    granted = BS_INTERSECT(4, session, required);
    can_edit = BS_EQ(4, granted, required)
            && !FLAGS_HAS(4, session, FilePerms_HIDDEN);
    printf("   after WRITE revoked, may edit: %s\n", can_edit ? "yes" : "no");

    // A retry counter that saturates at u6_max() (63) instead of wrapping:
    // compare with part 1, where 30 + 40 silently wrapped to 6
    u6 retries = u6_new(58);
    printf("   retry counter starts at %u (u6, max 63)\n", u6_get(&retries));
    for (int i = 0; i < 3; i++) {
        retries = u6_add_sat(retries, u6_new(4));
        printf("   after +4: %u%s\n", u6_get(&retries),
               u6_eq(retries, u6_max()) ? " (saturated, no wraparound)" : "");
    }

    printf("\n=== Done ===\n");
    return 0;
}

/**
 * @file bitset.h
 * @brief Fixed-size bitset types for the Cyan library
 * 
 * This header provides bitset types with arbitrary bit widths (1-64).
 * Use BITSET_DEFINE(N) to create a Bitset_N type for managing N bits.
 * 
 * Usage:
 *   BITSET_DEFINE(8);  // Creates Bitset_8 type
 *   Bitset_8 bs = bitset_8_new();
 *   bitset_8_set(&bs, 3);      // Set bit 3
 *   bool is_set = bitset_8_get(&bs, 3);  // Returns true
 *   bitset_8_toggle(&bs, 3);   // Toggle bit 3
 *   bitset_8_clear(&bs, 3);    // Clear bit 3
 * 
 * Named flags:
 *   FLAGS_DEFINE(Permissions, READ, WRITE, EXECUTE);
 *   BITSET_DEFINE(Permissions_COUNT);
 *   Bitset_3 perms = bitset_3_new();
 *   FLAGS_SET(perms, Permissions_READ);
 */

#ifndef CYAN_BITSET_H
#define CYAN_BITSET_H

#include "common.h"
#include "bitint.h"


/*============================================================================
 * Bitset Type Definition (BITSET_DEFINE)
 *============================================================================*/

/**
 * @brief Generate a bitset type with N bits
 * @param N Number of bits (1-64)
 * 
 * Creates:
 * - Bitset_N struct with bits and vt fields
 * - BitsetVT_N vtable struct
 * - bitset_N_new() constructor (all bits cleared)
 * - bitset_N_from_raw(value) constructor from raw value
 * - Bit manipulation: set, clear, get, toggle
 * - Set operations: union, intersect, diff, complement
 * - Utilities: eq, count, all, any, none
 * 
 * Example:
 *   BITSET_DEFINE(8);
 *   Bitset_8 bs = bitset_8_new();
 *   bitset_8_set(&bs, 0);  // Set bit 0
 */
#define BITSET_DEFINE(N) \
    _Static_assert((N) >= 1 && (N) <= 64, "BITSET_DEFINE: N must be 1-64"); \
    \
    /* Forward declare vtable */ \
    typedef struct BitsetVT_##N BitsetVT_##N; \
    \
    /* Type definition */ \
    typedef struct { \
        _CYAN_BACKING_UINT(N) bits; \
        const BitsetVT_##N *vt; \
    } Bitset_##N; \
    \
    /* Function prototypes for vtable */ \
    static inline void _bitset_##N##_set(Bitset_##N *bs, u8 index); \
    static inline void _bitset_##N##_clear(Bitset_##N *bs, u8 index); \
    static inline bool _bitset_##N##_get(const Bitset_##N *bs, u8 index); \
    static inline void _bitset_##N##_toggle(Bitset_##N *bs, u8 index); \
    static inline Bitset_##N _bitset_##N##_union(const Bitset_##N *a, const Bitset_##N *b); \
    static inline Bitset_##N _bitset_##N##_intersect(const Bitset_##N *a, const Bitset_##N *b); \
    static inline Bitset_##N _bitset_##N##_diff(const Bitset_##N *a, const Bitset_##N *b); \
    static inline Bitset_##N _bitset_##N##_complement(const Bitset_##N *bs); \
    static inline bool _bitset_##N##_eq(const Bitset_##N *a, const Bitset_##N *b); \
    static inline u8 _bitset_##N##_count(const Bitset_##N *bs); \
    static inline bool _bitset_##N##_all(const Bitset_##N *bs); \
    static inline bool _bitset_##N##_any(const Bitset_##N *bs); \
    static inline bool _bitset_##N##_none(const Bitset_##N *bs); \
    \
    /* Vtable structure */ \
    struct BitsetVT_##N { \
        void (*bs_set)(Bitset_##N *bs, u8 index); \
        void (*bs_clear)(Bitset_##N *bs, u8 index); \
        bool (*bs_get)(const Bitset_##N *bs, u8 index); \
        void (*bs_toggle)(Bitset_##N *bs, u8 index); \
        Bitset_##N (*bs_union)(const Bitset_##N *a, const Bitset_##N *b); \
        Bitset_##N (*bs_intersect)(const Bitset_##N *a, const Bitset_##N *b); \
        Bitset_##N (*bs_diff)(const Bitset_##N *a, const Bitset_##N *b); \
        Bitset_##N (*bs_complement)(const Bitset_##N *bs); \
        bool (*bs_eq)(const Bitset_##N *a, const Bitset_##N *b); \
        u8 (*bs_count)(const Bitset_##N *bs); \
        bool (*bs_all)(const Bitset_##N *bs); \
        bool (*bs_any)(const Bitset_##N *bs); \
        bool (*bs_none)(const Bitset_##N *bs); \
    }; \
    \
    /* Static vtable instance - forward declaration */ \
    static const BitsetVT_##N _bitset_##N##_vt; \
    \
    /* Constructor - all bits cleared */ \
    static inline Bitset_##N bitset_##N##_new(void) { \
        return (Bitset_##N){ \
            .bits = 0, \
            .vt = &_bitset_##N##_vt \
        }; \
    } \
    \
    /* Constructor from raw value - masks to N bits */ \
    static inline Bitset_##N bitset_##N##_from_raw(_CYAN_BACKING_UINT(N) value) { \
        return (Bitset_##N){ \
            .bits = (_CYAN_BACKING_UINT(N))(value & _CYAN_MASK(N)), \
            .vt = &_bitset_##N##_vt \
        }; \
    } \
    \
    /* Bit manipulation - set bit at index */ \
    static inline void _bitset_##N##_set(Bitset_##N *bs, u8 index) { \
        if (index >= (N)) { \
            CYAN_PANIC("bit index out of bounds"); \
        } \
        bs->bits |= ((_CYAN_BACKING_UINT(N))1 << index); \
    } \
    static inline void bitset_##N##_set(Bitset_##N *bs, u8 index) { \
        _bitset_##N##_set(bs, index); \
    } \
    \
    /* Bit manipulation - clear bit at index */ \
    static inline void _bitset_##N##_clear(Bitset_##N *bs, u8 index) { \
        if (index >= (N)) { \
            CYAN_PANIC("bit index out of bounds"); \
        } \
        bs->bits &= ~((_CYAN_BACKING_UINT(N))1 << index); \
    } \
    static inline void bitset_##N##_clear(Bitset_##N *bs, u8 index) { \
        _bitset_##N##_clear(bs, index); \
    } \
    \
    /* Bit manipulation - get bit at index */ \
    static inline bool _bitset_##N##_get(const Bitset_##N *bs, u8 index) { \
        if (index >= (N)) { \
            CYAN_PANIC("bit index out of bounds"); \
        } \
        return (bs->bits >> index) & 1; \
    } \
    static inline bool bitset_##N##_get(const Bitset_##N *bs, u8 index) { \
        return _bitset_##N##_get(bs, index); \
    } \
    \
    /* Bit manipulation - toggle bit at index */ \
    static inline void _bitset_##N##_toggle(Bitset_##N *bs, u8 index) { \
        if (index >= (N)) { \
            CYAN_PANIC("bit index out of bounds"); \
        } \
        bs->bits ^= ((_CYAN_BACKING_UINT(N))1 << index); \
    } \
    static inline void bitset_##N##_toggle(Bitset_##N *bs, u8 index) { \
        _bitset_##N##_toggle(bs, index); \
    } \
    \
    /* Set operations - union (OR) */ \
    static inline Bitset_##N _bitset_##N##_union(const Bitset_##N *a, const Bitset_##N *b) { \
        return bitset_##N##_from_raw(a->bits | b->bits); \
    } \
    static inline Bitset_##N bitset_##N##_union(const Bitset_##N *a, const Bitset_##N *b) { \
        return _bitset_##N##_union(a, b); \
    } \
    \
    /* Set operations - intersect (AND) */ \
    static inline Bitset_##N _bitset_##N##_intersect(const Bitset_##N *a, const Bitset_##N *b) { \
        return bitset_##N##_from_raw(a->bits & b->bits); \
    } \
    static inline Bitset_##N bitset_##N##_intersect(const Bitset_##N *a, const Bitset_##N *b) { \
        return _bitset_##N##_intersect(a, b); \
    } \
    \
    /* Set operations - difference (a AND NOT b) */ \
    static inline Bitset_##N _bitset_##N##_diff(const Bitset_##N *a, const Bitset_##N *b) { \
        return bitset_##N##_from_raw(a->bits & ~b->bits); \
    } \
    static inline Bitset_##N bitset_##N##_diff(const Bitset_##N *a, const Bitset_##N *b) { \
        return _bitset_##N##_diff(a, b); \
    } \
    \
    /* Set operations - complement (NOT, masked to N bits) */ \
    static inline Bitset_##N _bitset_##N##_complement(const Bitset_##N *bs) { \
        return bitset_##N##_from_raw(~bs->bits & _CYAN_MASK(N)); \
    } \
    static inline Bitset_##N bitset_##N##_complement(const Bitset_##N *bs) { \
        return _bitset_##N##_complement(bs); \
    } \
    \
    /* Equality check */ \
    static inline bool _bitset_##N##_eq(const Bitset_##N *a, const Bitset_##N *b) { \
        return (a->bits & _CYAN_MASK(N)) == (b->bits & _CYAN_MASK(N)); \
    } \
    static inline bool bitset_##N##_eq(const Bitset_##N *a, const Bitset_##N *b) { \
        return _bitset_##N##_eq(a, b); \
    } \
    \
    /* Utility - count set bits (popcount) */ \
    static inline u8 _bitset_##N##_count(const Bitset_##N *bs) { \
        _CYAN_BACKING_UINT(N) v = bs->bits & _CYAN_MASK(N); \
        u8 count = 0; \
        while (v) { \
            count += v & 1; \
            v >>= 1; \
        } \
        return count; \
    } \
    static inline u8 bitset_##N##_count(const Bitset_##N *bs) { \
        return _bitset_##N##_count(bs); \
    } \
    \
    /* Utility - check if all bits are set */ \
    static inline bool _bitset_##N##_all(const Bitset_##N *bs) { \
        return (bs->bits & _CYAN_MASK(N)) == _CYAN_MASK(N); \
    } \
    static inline bool bitset_##N##_all(const Bitset_##N *bs) { \
        return _bitset_##N##_all(bs); \
    } \
    \
    /* Utility - check if any bit is set */ \
    static inline bool _bitset_##N##_any(const Bitset_##N *bs) { \
        return (bs->bits & _CYAN_MASK(N)) != 0; \
    } \
    static inline bool bitset_##N##_any(const Bitset_##N *bs) { \
        return _bitset_##N##_any(bs); \
    } \
    \
    /* Utility - check if no bits are set */ \
    static inline bool _bitset_##N##_none(const Bitset_##N *bs) { \
        return (bs->bits & _CYAN_MASK(N)) == 0; \
    } \
    static inline bool bitset_##N##_none(const Bitset_##N *bs) { \
        return _bitset_##N##_none(bs); \
    } \
    \
    /* Static vtable instance */ \
    static const BitsetVT_##N _bitset_##N##_vt __attribute__((unused)) = { \
        .bs_set = _bitset_##N##_set, \
        .bs_clear = _bitset_##N##_clear, \
        .bs_get = _bitset_##N##_get, \
        .bs_toggle = _bitset_##N##_toggle, \
        .bs_union = _bitset_##N##_union, \
        .bs_intersect = _bitset_##N##_intersect, \
        .bs_diff = _bitset_##N##_diff, \
        .bs_complement = _bitset_##N##_complement, \
        .bs_eq = _bitset_##N##_eq, \
        .bs_count = _bitset_##N##_count, \
        .bs_all = _bitset_##N##_all, \
        .bs_any = _bitset_##N##_any, \
        .bs_none = _bitset_##N##_none \
    }


/*============================================================================
 * Vtable Convenience Macros
 *============================================================================*/

#if defined(__GNUC__) || defined(__clang__)

/**
 * @brief Set bit at index via vtable
 * @param bs Bitset instance (an lvalue)
 * @param i Bit index
 */
#define BS_SET(bs, i) \
    ({ __typeof__(bs) *_cyan_bsp = &(bs); _cyan_bsp->vt->bs_set(_cyan_bsp, (i)); })

/**
 * @brief Clear bit at index via vtable
 * @param bs Bitset instance (an lvalue)
 * @param i Bit index
 */
#define BS_CLEAR(bs, i) \
    ({ __typeof__(bs) *_cyan_bsp = &(bs); _cyan_bsp->vt->bs_clear(_cyan_bsp, (i)); })

/**
 * @brief Get bit at index via vtable
 * @param bs Bitset instance (an lvalue)
 * @param i Bit index
 * @return true if bit is set, false otherwise
 */
#define BS_GET(bs, i) \
    ({ __typeof__(bs) *_cyan_bsp = &(bs); _cyan_bsp->vt->bs_get(_cyan_bsp, (i)); })

/**
 * @brief Toggle bit at index via vtable
 * @param bs Bitset instance (an lvalue)
 * @param i Bit index
 */
#define BS_TOGGLE(bs, i) \
    ({ __typeof__(bs) *_cyan_bsp = &(bs); _cyan_bsp->vt->bs_toggle(_cyan_bsp, (i)); })

/**
 * @brief Union of two bitsets via vtable
 * @param a First bitset (an lvalue)
 * @param b Second bitset (an lvalue)
 * @return New bitset with bits set where either input has bits set
 */
#define BS_UNION(a, b) \
    ({ __typeof__(a) *_cyan_bsa = &(a); __typeof__(b) *_cyan_bsb = &(b); \
       _cyan_bsa->vt->bs_union(_cyan_bsa, _cyan_bsb); })

/**
 * @brief Intersection of two bitsets via vtable
 * @param a First bitset (an lvalue)
 * @param b Second bitset (an lvalue)
 * @return New bitset with bits set where both inputs have bits set
 */
#define BS_INTERSECT(a, b) \
    ({ __typeof__(a) *_cyan_bsa = &(a); __typeof__(b) *_cyan_bsb = &(b); \
       _cyan_bsa->vt->bs_intersect(_cyan_bsa, _cyan_bsb); })

/**
 * @brief Difference of two bitsets via vtable
 * @param a First bitset (an lvalue)
 * @param b Second bitset (an lvalue)
 * @return New bitset with bits set in a but not in b
 */
#define BS_DIFF(a, b) \
    ({ __typeof__(a) *_cyan_bsa = &(a); __typeof__(b) *_cyan_bsb = &(b); \
       _cyan_bsa->vt->bs_diff(_cyan_bsa, _cyan_bsb); })

/**
 * @brief Complement of a bitset via vtable
 * @param bs Bitset instance (an lvalue)
 * @return New bitset with all bits flipped within the N-bit range
 */
#define BS_COMPLEMENT(bs) \
    ({ __typeof__(bs) *_cyan_bsp = &(bs); _cyan_bsp->vt->bs_complement(_cyan_bsp); })

/**
 * @brief Check equality of two bitsets via vtable
 * @param a First bitset (an lvalue)
 * @param b Second bitset (an lvalue)
 * @return true if all bits match, false otherwise
 */
#define BS_EQ(a, b) \
    ({ __typeof__(a) *_cyan_bsa = &(a); __typeof__(b) *_cyan_bsb = &(b); \
       _cyan_bsa->vt->bs_eq(_cyan_bsa, _cyan_bsb); })

/**
 * @brief Count set bits via vtable
 * @param bs Bitset instance (an lvalue)
 * @return Number of bits set to 1
 */
#define BS_COUNT(bs) \
    ({ __typeof__(bs) *_cyan_bsp = &(bs); _cyan_bsp->vt->bs_count(_cyan_bsp); })

/**
 * @brief Check if all bits are set via vtable
 * @param bs Bitset instance (an lvalue)
 * @return true if all N bits are set
 */
#define BS_ALL(bs) \
    ({ __typeof__(bs) *_cyan_bsp = &(bs); _cyan_bsp->vt->bs_all(_cyan_bsp); })

/**
 * @brief Check if any bit is set via vtable
 * @param bs Bitset instance (an lvalue)
 * @return true if at least one bit is set
 */
#define BS_ANY(bs) \
    ({ __typeof__(bs) *_cyan_bsp = &(bs); _cyan_bsp->vt->bs_any(_cyan_bsp); })

/**
 * @brief Check if no bits are set via vtable
 * @param bs Bitset instance (an lvalue)
 * @return true if no bits are set
 */
#define BS_NONE(bs) \
    ({ __typeof__(bs) *_cyan_bsp = &(bs); _cyan_bsp->vt->bs_none(_cyan_bsp); })

#else /* Fallbacks: evaluate their bitset arguments more than once */

#define BS_SET(bs, i)       ((bs).vt->bs_set(&(bs), (i)))
#define BS_CLEAR(bs, i)     ((bs).vt->bs_clear(&(bs), (i)))
#define BS_GET(bs, i)       ((bs).vt->bs_get(&(bs), (i)))
#define BS_TOGGLE(bs, i)    ((bs).vt->bs_toggle(&(bs), (i)))
#define BS_UNION(a, b)      ((a).vt->bs_union(&(a), &(b)))
#define BS_INTERSECT(a, b)  ((a).vt->bs_intersect(&(a), &(b)))
#define BS_DIFF(a, b)       ((a).vt->bs_diff(&(a), &(b)))
#define BS_COMPLEMENT(bs)   ((bs).vt->bs_complement(&(bs)))
#define BS_EQ(a, b)         ((a).vt->bs_eq(&(a), &(b)))
#define BS_COUNT(bs)        ((bs).vt->bs_count(&(bs)))
#define BS_ALL(bs)          ((bs).vt->bs_all(&(bs)))
#define BS_ANY(bs)          ((bs).vt->bs_any(&(bs)))
#define BS_NONE(bs)         ((bs).vt->bs_none(&(bs)))

#endif


/*============================================================================
 * Named Flags (FLAGS_DEFINE)
 *============================================================================*/

/**
 * @brief Helper macro to count variadic arguments (up to 64)
 */
#define _CYAN_ARG_COUNT(...) \
    _CYAN_ARG_COUNT_IMPL(__VA_ARGS__, \
        64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, \
        48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, \
        32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, \
        16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define _CYAN_ARG_COUNT_IMPL( \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
    _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, \
    _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, \
    _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, \
    N, ...) N

/**
 * @brief Helper macros for generating enum values
 */
#define _CYAN_ENUM_VAL_1(Name, a) Name##_##a = 0
#define _CYAN_ENUM_VAL_2(Name, a, b) _CYAN_ENUM_VAL_1(Name, a), Name##_##b = 1
#define _CYAN_ENUM_VAL_3(Name, a, b, c) _CYAN_ENUM_VAL_2(Name, a, b), Name##_##c = 2
#define _CYAN_ENUM_VAL_4(Name, a, b, c, d) _CYAN_ENUM_VAL_3(Name, a, b, c), Name##_##d = 3
#define _CYAN_ENUM_VAL_5(Name, a, b, c, d, e) _CYAN_ENUM_VAL_4(Name, a, b, c, d), Name##_##e = 4
#define _CYAN_ENUM_VAL_6(Name, a, b, c, d, e, f) _CYAN_ENUM_VAL_5(Name, a, b, c, d, e), Name##_##f = 5
#define _CYAN_ENUM_VAL_7(Name, a, b, c, d, e, f, g) _CYAN_ENUM_VAL_6(Name, a, b, c, d, e, f), Name##_##g = 6
#define _CYAN_ENUM_VAL_8(Name, a, b, c, d, e, f, g, h) _CYAN_ENUM_VAL_7(Name, a, b, c, d, e, f, g), Name##_##h = 7
#define _CYAN_ENUM_VAL_9(Name, a, b, c, d, e, f, g, h, i) _CYAN_ENUM_VAL_8(Name, a, b, c, d, e, f, g, h), Name##_##i = 8
#define _CYAN_ENUM_VAL_10(Name, a, b, c, d, e, f, g, h, i, j) _CYAN_ENUM_VAL_9(Name, a, b, c, d, e, f, g, h, i), Name##_##j = 9
#define _CYAN_ENUM_VAL_11(Name, a, b, c, d, e, f, g, h, i, j, k) _CYAN_ENUM_VAL_10(Name, a, b, c, d, e, f, g, h, i, j), Name##_##k = 10
#define _CYAN_ENUM_VAL_12(Name, a, b, c, d, e, f, g, h, i, j, k, l) _CYAN_ENUM_VAL_11(Name, a, b, c, d, e, f, g, h, i, j, k), Name##_##l = 11
#define _CYAN_ENUM_VAL_13(Name, a, b, c, d, e, f, g, h, i, j, k, l, m) _CYAN_ENUM_VAL_12(Name, a, b, c, d, e, f, g, h, i, j, k, l), Name##_##m = 12
#define _CYAN_ENUM_VAL_14(Name, a, b, c, d, e, f, g, h, i, j, k, l, m, n) _CYAN_ENUM_VAL_13(Name, a, b, c, d, e, f, g, h, i, j, k, l, m), Name##_##n = 13
#define _CYAN_ENUM_VAL_15(Name, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) _CYAN_ENUM_VAL_14(Name, a, b, c, d, e, f, g, h, i, j, k, l, m, n), Name##_##o = 14
#define _CYAN_ENUM_VAL_16(Name, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) _CYAN_ENUM_VAL_15(Name, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o), Name##_##p = 15

/**
 * @brief Helper to select the right enum generator based on argument count
 */
#define _CYAN_ENUM_VALS_N(N) _CYAN_ENUM_VAL_##N
#define _CYAN_ENUM_VALS(N, Name, ...) _CYAN_ENUM_VALS_N(N)(Name, __VA_ARGS__)

/**
 * @brief Define named flags with sequential bit positions
 * @param Name The name prefix for the flags
 * @param ... Flag names (up to 16)
 * 
 * Creates:
 * - Enum constants Name_FLAG1 = 0, Name_FLAG2 = 1, etc.
 * - Name_COUNT constant with the total number of flags
 * 
 * Example:
 *   FLAGS_DEFINE(Permissions, READ, WRITE, EXECUTE);
 *   // Creates: Permissions_READ = 0, Permissions_WRITE = 1, Permissions_EXECUTE = 2
 *   // Creates: Permissions_COUNT = 3
 */
#define FLAGS_DEFINE(Name, ...) \
    enum { \
        _CYAN_ENUM_VALS(_CYAN_ARG_COUNT(__VA_ARGS__), Name, __VA_ARGS__), \
        Name##_COUNT \
    }


/*============================================================================
 * Flag Manipulation Macros
 *============================================================================*/

/**
 * @brief Set a flag in a bitset
 * @param bs Bitset instance (must be an lvalue)
 * @param flag Flag constant (bit position)
 * 
 * Example:
 *   FLAGS_SET(perms, Permissions_READ);
 */
#define FLAGS_SET(bs, flag)   BS_SET(bs, flag)

/**
 * @brief Clear a flag in a bitset
 * @param bs Bitset instance (must be an lvalue)
 * @param flag Flag constant (bit position)
 * 
 * Example:
 *   FLAGS_CLEAR(perms, Permissions_READ);
 */
#define FLAGS_CLEAR(bs, flag) BS_CLEAR(bs, flag)

/**
 * @brief Check if a flag is set in a bitset
 * @param bs Bitset instance
 * @param flag Flag constant (bit position)
 * @return true if the flag is set, false otherwise
 * 
 * Example:
 *   if (FLAGS_HAS(perms, Permissions_READ)) { ... }
 */
#define FLAGS_HAS(bs, flag)   BS_GET(bs, flag)


#endif /* CYAN_BITSET_H */

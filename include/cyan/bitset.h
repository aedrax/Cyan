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

#endif /* CYAN_BITSET_H */

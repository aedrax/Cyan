/**
 * @file bitint.h
 * @brief Custom bit-width integer types for the Cyan library
 * 
 * This header provides Zig-inspired integer types with arbitrary bit widths.
 * Use UINT_DEFINE(N) to create unsigned N-bit integers (u1-u64) and
 * INT_DEFINE(N) to create signed N-bit integers (i2-i64).
 * 
 * Usage:
 *   UINT_DEFINE(6);  // Creates u6 type (6-bit unsigned, 0-63)
 *   u6 val = u6_new(42);
 *   u8 raw = u6_get(&val);  // Returns 42
 *   
 *   INT_DEFINE(6);   // Creates i6 type (6-bit signed, -32 to 31)
 *   i6 sval = i6_new(-10);
 *   i8 sraw = i6_get(&sval);  // Returns -10
 */

#ifndef CYAN_BITINT_H
#define CYAN_BITINT_H

#include "common.h"

/*============================================================================
 * Backing Type Selection Macros
 *============================================================================
 * These macros select the smallest standard integer type that can hold N bits.
 * Uses compile-time selection based on bit width ranges.
 */

/**
 * @brief Select smallest unsigned backing type for N bits
 * @param N Number of bits (1-64)
 * @return Type name: u8, u16, u32, or u64
 */
#define _CYAN_BACKING_UINT(N) \
    __typeof__( \
        _Generic((char(*)[((N) <= 8) ? 1 : ((N) <= 16) ? 2 : ((N) <= 32) ? 3 : 4])0, \
            char(*)[1]: (u8)0, \
            char(*)[2]: (u16)0, \
            char(*)[3]: (u32)0, \
            char(*)[4]: (u64)0 \
        ) \
    )

/**
 * @brief Select smallest signed backing type for N bits
 * @param N Number of bits (2-64)
 * @return Type name: i8, i16, i32, or i64
 */
#define _CYAN_BACKING_INT(N) \
    __typeof__( \
        _Generic((char(*)[((N) <= 8) ? 1 : ((N) <= 16) ? 2 : ((N) <= 32) ? 3 : 4])0, \
            char(*)[1]: (i8)0, \
            char(*)[2]: (i16)0, \
            char(*)[3]: (i32)0, \
            char(*)[4]: (i64)0 \
        ) \
    )


/**
 * @brief Generate N-bit mask (all 1s in lower N bits)
 * @param N Number of bits (1-64)
 * @return Mask value with N lower bits set to 1
 * 
 * Special case for N=64 to avoid undefined behavior from shifting.
 */
#define _CYAN_MASK(N) \
    (((N) >= 64) ? (~(u64)0) : (((u64)1 << (N)) - 1))

/**
 * @brief Sign-extend an N-bit value to full width
 * @param v The value to sign-extend
 * @param N The bit width of the value
 * @return Sign-extended value
 * 
 * Uses the XOR-subtract trick for sign extension:
 * If bit N-1 is set (negative), extends 1s; otherwise extends 0s.
 */
#define _CYAN_SIGN_EXTEND(v, N) \
    ((i64)(((u64)(v) ^ ((u64)1 << ((N) - 1))) - ((u64)1 << ((N) - 1))))

/*============================================================================
 * Unsigned Integer Type Definition (UINT_DEFINE)
 *============================================================================*/

/**
 * @brief Generate an unsigned N-bit integer type
 * @param N Number of bits (1-64)
 * 
 * Creates:
 * - uN struct with _value and vt fields
 * - uN_VT vtable struct
 * - uN_new(value) constructor
 * - uN_get(ptr) accessor
 * - uN_raw(ptr) raw value accessor
 * - Arithmetic operations: add, sub, mul, and, or, xor, not, shl, shr
 * - Comparison operations: eq, lt, le
 * - Constants: max, min
 * 
 * Example:
 *   UINT_DEFINE(6);  // Creates u6 type
 *   u6 val = u6_new(100);  // Masked to 36 (100 & 0x3F)
 */
#define UINT_DEFINE(N) \
    _Static_assert((N) >= 1 && (N) <= 64, "UINT_DEFINE: N must be 1-64"); \
    \
    /* Forward declare vtable */ \
    typedef struct u##N##_VT u##N##_VT; \
    \
    /* Type definition */ \
    typedef struct { \
        _CYAN_BACKING_UINT(N) _value; \
        const u##N##_VT *vt; \
    } u##N; \
    \
    /* Function prototypes for vtable */ \
    static inline u##N _u##N##_add(const u##N *a, const u##N *b); \
    static inline u##N _u##N##_sub(const u##N *a, const u##N *b); \
    static inline u##N _u##N##_mul(const u##N *a, const u##N *b); \
    static inline u##N _u##N##_and(const u##N *a, const u##N *b); \
    static inline u##N _u##N##_or(const u##N *a, const u##N *b); \
    static inline u##N _u##N##_xor(const u##N *a, const u##N *b); \
    static inline u##N _u##N##_not(const u##N *a); \
    static inline u##N _u##N##_shl(const u##N *a, u8 shift); \
    static inline u##N _u##N##_shr(const u##N *a, u8 shift); \
    static inline bool _u##N##_eq(const u##N *a, const u##N *b); \
    static inline bool _u##N##_lt(const u##N *a, const u##N *b); \
    static inline bool _u##N##_le(const u##N *a, const u##N *b); \
    \
    /* Vtable structure */ \
    struct u##N##_VT { \
        u##N (*add)(const u##N *a, const u##N *b); \
        u##N (*sub)(const u##N *a, const u##N *b); \
        u##N (*mul)(const u##N *a, const u##N *b); \
        u##N (*bit_and)(const u##N *a, const u##N *b); \
        u##N (*bit_or)(const u##N *a, const u##N *b); \
        u##N (*bit_xor)(const u##N *a, const u##N *b); \
        u##N (*bit_not)(const u##N *a); \
        u##N (*shl)(const u##N *a, u8 shift); \
        u##N (*shr)(const u##N *a, u8 shift); \
        bool (*eq)(const u##N *a, const u##N *b); \
        bool (*lt)(const u##N *a, const u##N *b); \
        bool (*le)(const u##N *a, const u##N *b); \
    }; \
    \
    /* Static vtable instance - forward declaration */ \
    static const u##N##_VT _u##N##_vt; \
    \
    /* Constructor - masks value to N bits */ \
    static inline u##N u##N##_new(_CYAN_BACKING_UINT(N) value) { \
        return (u##N){ \
            ._value = (_CYAN_BACKING_UINT(N))(value & _CYAN_MASK(N)), \
            .vt = &_u##N##_vt \
        }; \
    } \
    \
    /* Accessor - returns masked value */ \
    static inline _CYAN_BACKING_UINT(N) u##N##_get(const u##N *n) { \
        return n->_value & _CYAN_MASK(N); \
    } \
    \
    /* Raw accessor - returns backing value */ \
    static inline _CYAN_BACKING_UINT(N) u##N##_raw(const u##N *n) { \
        return n->_value; \
    } \
    \
    /* Arithmetic operations */ \
    static inline u##N _u##N##_add(const u##N *a, const u##N *b) { \
        return u##N##_new((_CYAN_BACKING_UINT(N))(a->_value + b->_value)); \
    } \
    static inline u##N u##N##_add(u##N a, u##N b) { \
        return _u##N##_add(&a, &b); \
    } \
    \
    static inline u##N _u##N##_sub(const u##N *a, const u##N *b) { \
        return u##N##_new((_CYAN_BACKING_UINT(N))(a->_value - b->_value)); \
    } \
    static inline u##N u##N##_sub(u##N a, u##N b) { \
        return _u##N##_sub(&a, &b); \
    } \
    \
    static inline u##N _u##N##_mul(const u##N *a, const u##N *b) { \
        return u##N##_new((_CYAN_BACKING_UINT(N))(a->_value * b->_value)); \
    } \
    static inline u##N u##N##_mul(u##N a, u##N b) { \
        return _u##N##_mul(&a, &b); \
    } \
    \
    /* Bitwise operations */ \
    static inline u##N _u##N##_and(const u##N *a, const u##N *b) { \
        return u##N##_new((_CYAN_BACKING_UINT(N))(a->_value & b->_value)); \
    } \
    static inline u##N u##N##_and(u##N a, u##N b) { \
        return _u##N##_and(&a, &b); \
    } \
    \
    static inline u##N _u##N##_or(const u##N *a, const u##N *b) { \
        return u##N##_new((_CYAN_BACKING_UINT(N))(a->_value | b->_value)); \
    } \
    static inline u##N u##N##_or(u##N a, u##N b) { \
        return _u##N##_or(&a, &b); \
    } \
    \
    static inline u##N _u##N##_xor(const u##N *a, const u##N *b) { \
        return u##N##_new((_CYAN_BACKING_UINT(N))(a->_value ^ b->_value)); \
    } \
    static inline u##N u##N##_xor(u##N a, u##N b) { \
        return _u##N##_xor(&a, &b); \
    } \
    \
    static inline u##N _u##N##_not(const u##N *a) { \
        return u##N##_new((_CYAN_BACKING_UINT(N))(~a->_value)); \
    } \
    static inline u##N u##N##_not(u##N a) { \
        return _u##N##_not(&a); \
    } \
    \
    /* Shift operations */ \
    static inline u##N _u##N##_shl(const u##N *a, u8 shift) { \
        if (shift >= (N)) return u##N##_new(0); \
        return u##N##_new((_CYAN_BACKING_UINT(N))(a->_value << shift)); \
    } \
    static inline u##N u##N##_shl(u##N a, u8 shift) { \
        return _u##N##_shl(&a, shift); \
    } \
    \
    static inline u##N _u##N##_shr(const u##N *a, u8 shift) { \
        if (shift >= (N)) return u##N##_new(0); \
        return u##N##_new((_CYAN_BACKING_UINT(N))(a->_value >> shift)); \
    } \
    static inline u##N u##N##_shr(u##N a, u8 shift) { \
        return _u##N##_shr(&a, shift); \
    } \
    \
    /* Comparison operations */ \
    static inline bool _u##N##_eq(const u##N *a, const u##N *b) { \
        return (a->_value & _CYAN_MASK(N)) == (b->_value & _CYAN_MASK(N)); \
    } \
    static inline bool u##N##_eq(u##N a, u##N b) { \
        return _u##N##_eq(&a, &b); \
    } \
    \
    static inline bool _u##N##_lt(const u##N *a, const u##N *b) { \
        return (a->_value & _CYAN_MASK(N)) < (b->_value & _CYAN_MASK(N)); \
    } \
    static inline bool u##N##_lt(u##N a, u##N b) { \
        return _u##N##_lt(&a, &b); \
    } \
    \
    static inline bool _u##N##_le(const u##N *a, const u##N *b) { \
        return (a->_value & _CYAN_MASK(N)) <= (b->_value & _CYAN_MASK(N)); \
    } \
    static inline bool u##N##_le(u##N a, u##N b) { \
        return _u##N##_le(&a, &b); \
    } \
    \
    /* Constants */ \
    static inline u##N u##N##_max(void) { \
        return u##N##_new((_CYAN_BACKING_UINT(N))_CYAN_MASK(N)); \
    } \
    static inline u##N u##N##_min(void) { \
        return u##N##_new(0); \
    } \
    \
    /* Static vtable instance */ \
    static const u##N##_VT _u##N##_vt __attribute__((unused)) = { \
        .add = _u##N##_add, \
        .sub = _u##N##_sub, \
        .mul = _u##N##_mul, \
        .bit_and = _u##N##_and, \
        .bit_or = _u##N##_or, \
        .bit_xor = _u##N##_xor, \
        .bit_not = _u##N##_not, \
        .shl = _u##N##_shl, \
        .shr = _u##N##_shr, \
        .eq = _u##N##_eq, \
        .lt = _u##N##_lt, \
        .le = _u##N##_le \
    }

/*============================================================================
 * Signed Integer Type Definition (INT_DEFINE)
 *============================================================================*/

/**
 * @brief Generate a signed N-bit integer type
 * @param N Number of bits (2-64)
 * 
 * Creates:
 * - iN struct with _value and vt fields
 * - iN_VT vtable struct
 * - iN_new(value) constructor with sign extension
 * - iN_get(ptr) accessor returning sign-extended value
 * - Arithmetic operations: add, sub, mul, neg
 * - Comparison operations: eq, lt, le
 * - Constants: max, min
 * 
 * Example:
 *   INT_DEFINE(6);   // Creates i6 type (-32 to 31)
 *   i6 val = i6_new(-10);
 *   i8 raw = i6_get(&val);  // Returns -10
 */
#define INT_DEFINE(N) \
    _Static_assert((N) >= 2 && (N) <= 64, "INT_DEFINE: N must be 2-64"); \
    \
    /* Forward declare vtable */ \
    typedef struct i##N##_VT i##N##_VT; \
    \
    /* Type definition */ \
    typedef struct { \
        _CYAN_BACKING_INT(N) _value; \
        const i##N##_VT *vt; \
    } i##N; \
    \
    /* Function prototypes for vtable */ \
    static inline i##N _i##N##_add(const i##N *a, const i##N *b); \
    static inline i##N _i##N##_sub(const i##N *a, const i##N *b); \
    static inline i##N _i##N##_mul(const i##N *a, const i##N *b); \
    static inline i##N _i##N##_neg(const i##N *a); \
    static inline bool _i##N##_eq(const i##N *a, const i##N *b); \
    static inline bool _i##N##_lt(const i##N *a, const i##N *b); \
    static inline bool _i##N##_le(const i##N *a, const i##N *b); \
    \
    /* Vtable structure */ \
    struct i##N##_VT { \
        i##N (*add)(const i##N *a, const i##N *b); \
        i##N (*sub)(const i##N *a, const i##N *b); \
        i##N (*mul)(const i##N *a, const i##N *b); \
        i##N (*neg)(const i##N *a); \
        bool (*eq)(const i##N *a, const i##N *b); \
        bool (*lt)(const i##N *a, const i##N *b); \
        bool (*le)(const i##N *a, const i##N *b); \
    }; \
    \
    /* Static vtable instance - forward declaration */ \
    static const i##N##_VT _i##N##_vt; \
    \
    /* Helper: truncate to N bits (keep lower N bits) */ \
    static inline _CYAN_BACKING_INT(N) _i##N##_truncate(i64 value) { \
        return (_CYAN_BACKING_INT(N))((u64)value & _CYAN_MASK(N)); \
    } \
    \
    /* Helper: sign-extend N-bit value to backing type */ \
    static inline _CYAN_BACKING_INT(N) _i##N##_sign_extend(_CYAN_BACKING_INT(N) value) { \
        u64 masked = (u64)value & _CYAN_MASK(N); \
        return (_CYAN_BACKING_INT(N))_CYAN_SIGN_EXTEND(masked, N); \
    } \
    \
    /* Constructor - truncates and stores N-bit value */ \
    static inline i##N i##N##_new(_CYAN_BACKING_INT(N) value) { \
        return (i##N){ \
            ._value = _i##N##_truncate(value), \
            .vt = &_i##N##_vt \
        }; \
    } \
    \
    /* Accessor - returns sign-extended value */ \
    static inline _CYAN_BACKING_INT(N) i##N##_get(const i##N *n) { \
        return _i##N##_sign_extend(n->_value); \
    } \
    \
    /* Arithmetic operations */ \
    static inline i##N _i##N##_add(const i##N *a, const i##N *b) { \
        i64 result = (i64)i##N##_get(a) + (i64)i##N##_get(b); \
        return i##N##_new((_CYAN_BACKING_INT(N))result); \
    } \
    static inline i##N i##N##_add(i##N a, i##N b) { \
        return _i##N##_add(&a, &b); \
    } \
    \
    static inline i##N _i##N##_sub(const i##N *a, const i##N *b) { \
        i64 result = (i64)i##N##_get(a) - (i64)i##N##_get(b); \
        return i##N##_new((_CYAN_BACKING_INT(N))result); \
    } \
    static inline i##N i##N##_sub(i##N a, i##N b) { \
        return _i##N##_sub(&a, &b); \
    } \
    \
    static inline i##N _i##N##_mul(const i##N *a, const i##N *b) { \
        i64 result = (i64)i##N##_get(a) * (i64)i##N##_get(b); \
        return i##N##_new((_CYAN_BACKING_INT(N))result); \
    } \
    static inline i##N i##N##_mul(i##N a, i##N b) { \
        return _i##N##_mul(&a, &b); \
    } \
    \
    static inline i##N _i##N##_neg(const i##N *a) { \
        i64 result = -(i64)i##N##_get(a); \
        return i##N##_new((_CYAN_BACKING_INT(N))result); \
    } \
    static inline i##N i##N##_neg(i##N a) { \
        return _i##N##_neg(&a); \
    } \
    \
    /* Comparison operations */ \
    static inline bool _i##N##_eq(const i##N *a, const i##N *b) { \
        return i##N##_get(a) == i##N##_get(b); \
    } \
    static inline bool i##N##_eq(i##N a, i##N b) { \
        return _i##N##_eq(&a, &b); \
    } \
    \
    static inline bool _i##N##_lt(const i##N *a, const i##N *b) { \
        return i##N##_get(a) < i##N##_get(b); \
    } \
    static inline bool i##N##_lt(i##N a, i##N b) { \
        return _i##N##_lt(&a, &b); \
    } \
    \
    static inline bool _i##N##_le(const i##N *a, const i##N *b) { \
        return i##N##_get(a) <= i##N##_get(b); \
    } \
    static inline bool i##N##_le(i##N a, i##N b) { \
        return _i##N##_le(&a, &b); \
    } \
    \
    /* Constants */ \
    static inline i##N i##N##_max(void) { \
        /* Max value for N-bit signed: 2^(N-1) - 1 */ \
        return i##N##_new((_CYAN_BACKING_INT(N))(((u64)1 << ((N) - 1)) - 1)); \
    } \
    static inline i##N i##N##_min(void) { \
        /* Min value for N-bit signed: -2^(N-1) */ \
        return i##N##_new((_CYAN_BACKING_INT(N))(-(i64)((u64)1 << ((N) - 1)))); \
    } \
    \
    /* Static vtable instance */ \
    static const i##N##_VT _i##N##_vt __attribute__((unused)) = { \
        .add = _i##N##_add, \
        .sub = _i##N##_sub, \
        .mul = _i##N##_mul, \
        .neg = _i##N##_neg, \
        .eq = _i##N##_eq, \
        .lt = _i##N##_lt, \
        .le = _i##N##_le \
    }

#endif /* CYAN_BITINT_H */

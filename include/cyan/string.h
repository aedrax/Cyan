/**
 * @file string.h
 * @brief Dynamic string type for safe text manipulation
 * 
 * This header provides a heap-allocated, growable string type with safe
 * operations that handle buffer sizing automatically. It avoids common
 * pitfalls of C strings like buffer overflows and null-terminator issues.
 * 
 * Usage:
 *   String s = string_from("Hello");
 *   string_append(&s, " World");
 *   printf("%s\n", string_cstr(&s));  // "Hello World"
 *   string_free(&s);
 * 
 * Or with auto-cleanup:
 *   string_auto(s, string_from("Hello"));
 *   // s is automatically freed when scope exits
 */

#ifndef CYAN_STRING_H
#define CYAN_STRING_H

#include "common.h"
#include "option.h"
#include "vector.h"
#include "slice.h"
#include <string.h>
#include <stdarg.h>

/*============================================================================
 * Type Definitions
 *============================================================================*/

/* Define Option_char for string_get return type */
OPTION_DEFINE(char);

/* Define Vec_char for slice_from_vec compatibility */
VECTOR_DEFINE(char);

/* Define Slice_char for string slicing */
SLICE_DEFINE(char);

/* Forward declare String for use in vtable */
typedef struct String String;

/**
 * @brief Vtable structure for String containing function pointers
 */
typedef struct {
    void (*push)(String *s, char c);
    void (*append)(String *s, const char *cstr);
    void (*clear)(String *s);
    Option_char (*get)(const String *s, size_t idx);
    size_t (*len)(const String *s);
    const char *(*cstr)(const String *s);
    Slice_char (*slice)(const String *s, size_t start, size_t end);
    void (*free)(String *s);
} StringVT;

/**
 * @brief Dynamic string type
 * 
 * A heap-allocated, growable string with:
 * - data: null-terminated character buffer
 * - len: length excluding null terminator
 * - cap: capacity including null terminator
 * - vt: pointer to shared vtable
 */
struct String {
    char *data;      /* Null-terminated buffer */
    size_t len;      /* Length excluding null terminator */
    size_t cap;      /* Capacity including null terminator */
    const StringVT *vt;  /* Pointer to shared vtable */
};

/* Forward declare vtable instance */
static const StringVT _string_vt;

/*============================================================================
 * Constructors
 *============================================================================*/

/**
 * @brief Create an empty string
 * @return A new empty String
 */
static inline String string_new(void) {
    return (String){ .data = NULL, .len = 0, .cap = 0, .vt = &_string_vt };
}

/**
 * @brief Create a string from a C string
 * @param cstr The source C string (null-terminated)
 * @return A new String containing a copy of cstr
 * @note Panics if allocation fails
 */
static inline String string_from(const char *cstr) {
    if (!cstr) {
        return string_new();
    }
    size_t len = strlen(cstr);
    size_t cap = len + 1;
    char *data = (char *)malloc(cap);
    if (!data) CYAN_PANIC("allocation failed");
    memcpy(data, cstr, cap);  /* Includes null terminator */
    return (String){ .data = data, .len = len, .cap = cap, .vt = &_string_vt };
}

/**
 * @brief Create a string with pre-allocated capacity
 * @param cap Initial capacity (excluding null terminator)
 * @return A new empty String with allocated storage
 * @note Panics if allocation fails
 */
static inline String string_with_capacity(size_t cap) {
    if (cap == 0) {
        return string_new();
    }
    size_t actual_cap = cap + 1;  /* +1 for null terminator */
    char *data = (char *)malloc(actual_cap);
    if (!data) CYAN_PANIC("allocation failed");
    data[0] = '\0';
    return (String){ .data = data, .len = 0, .cap = actual_cap, .vt = &_string_vt };
}

/*============================================================================
 * Internal Helpers
 *============================================================================*/

/**
 * @brief Check that string has enough capacity for additional bytes
 * @param s Pointer to the string
 * @param additional Number of additional bytes needed
 */
static inline void _string_check_capacity(String *s, size_t additional) {
    size_t required = s->len + additional + 1;  /* +1 for null terminator */
    if (required <= s->cap) return;
    
    size_t new_cap = s->cap == 0 ? CYAN_DEFAULT_CAPACITY : s->cap;
    while (new_cap < required) {
        new_cap *= CYAN_GROWTH_FACTOR;
    }
    
    char *new_data = (char *)realloc(s->data, new_cap);
    if (!new_data) CYAN_PANIC("allocation failed");
    s->data = new_data;
    s->cap = new_cap;
}

/*============================================================================
 * Modification
 *============================================================================*/

/**
 * @brief Push a single character to the string
 * @param s Pointer to the string
 * @param c Character to append
 */
static inline void string_push(String *s, char c) {
    _string_check_capacity(s, 1);
    s->data[s->len++] = c;
    s->data[s->len] = '\0';
}

/**
 * @brief Append a C string to the string
 * @param s Pointer to the string
 * @param cstr C string to append
 */
static inline void string_append(String *s, const char *cstr) {
    if (!cstr) return;
    size_t add_len = strlen(cstr);
    if (add_len == 0) return;
    
    _string_check_capacity(s, add_len);
    memcpy(s->data + s->len, cstr, add_len + 1);  /* Includes null terminator */
    s->len += add_len;
}

/**
 * @brief Append another String to this string
 * @param s Pointer to the destination string
 * @param other Pointer to the source string
 */
static inline void string_append_str(String *s, const String *other) {
    if (!other || other->len == 0) return;
    
    _string_check_capacity(s, other->len);
    memcpy(s->data + s->len, other->data, other->len + 1);
    s->len += other->len;
}

/**
 * @brief Clear the string content (keeps capacity)
 * @param s Pointer to the string
 */
static inline void string_clear(String *s) {
    s->len = 0;
    if (s->data) {
        s->data[0] = '\0';
    }
}

/*============================================================================
 * Formatting
 *============================================================================*/

/**
 * @brief Format and append to string (like sprintf)
 * @param s Pointer to the string
 * @param fmt Format string
 * @param ... Format arguments
 * @note Automatically grows buffer as needed
 */
static inline void string_format(String *s, const char *fmt, ...) {
    va_list args, args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);
    
    /* First, determine required size */
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    
    if (needed < 0) {
        va_end(args_copy);
        return;  /* Format error */
    }
    
    _string_check_capacity(s, (size_t)needed);
    
    /* Now format into the buffer */
    vsnprintf(s->data + s->len, (size_t)needed + 1, fmt, args_copy);
    va_end(args_copy);
    
    s->len += (size_t)needed;
}

/**
 * @brief Create a new formatted string
 * @param fmt Format string
 * @param ... Format arguments
 * @return A new String containing the formatted output
 */
static inline String string_formatted(const char *fmt, ...) {
    va_list args, args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);
    
    /* First, determine required size */
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    
    if (needed < 0) {
        va_end(args_copy);
        return string_new();  /* Format error */
    }
    
    String s = string_with_capacity((size_t)needed);
    vsnprintf(s.data, (size_t)needed + 1, fmt, args_copy);
    va_end(args_copy);
    
    s.len = (size_t)needed;
    return s;
}

/*============================================================================
 * Access
 *============================================================================*/

/**
 * @brief Get the null-terminated C string
 * @param s Pointer to the string
 * @return Pointer to null-terminated character array
 * @note Returns empty string "" for uninitialized strings
 */
static inline const char *string_cstr(const String *s) {
    return s->data ? s->data : "";
}

/**
 * @brief Get the length of the string
 * @param s Pointer to the string
 * @return Number of characters (excluding null terminator)
 */
static inline size_t string_len(const String *s) {
    return s->len;
}

/**
 * @brief Get character at index with bounds checking
 * @param s Pointer to the string
 * @param idx Index to access
 * @return Option_char containing the character, or None if out of bounds
 */
static inline Option_char string_get(const String *s, size_t idx) {
    if (idx >= s->len) return None(char);
    return Some(char, s->data[idx]);
}

/*============================================================================
 * Slicing
 *============================================================================*/

/**
 * @brief Create a slice view of a portion of the string
 * @param s Pointer to the string
 * @param start Start index (inclusive)
 * @param end End index (exclusive)
 * @return Slice_char viewing the specified range
 * @note Indices are clamped to valid bounds
 * @note The slice becomes invalid if the string is modified or freed
 */
static inline Slice_char string_slice(const String *s, size_t start, size_t end) {
    if (!s->data || s->len == 0) {
        return (Slice_char){ .data = NULL, .len = 0, .vt = &_slice_char_vt };
    }
    if (start > s->len) start = s->len;
    if (end > s->len) end = s->len;
    if (start > end) start = end;
    return (Slice_char){ .data = s->data + start, .len = end - start, .vt = &_slice_char_vt };
}

/**
 * @brief Create a slice view of the entire string
 * @param s Pointer to the string
 * @return Slice_char viewing the entire string
 */
static inline Slice_char string_as_slice(const String *s) {
    if (!s->data) {
        return (Slice_char){ .data = NULL, .len = 0, .vt = &_slice_char_vt };
    }
    return (Slice_char){ .data = s->data, .len = s->len, .vt = &_slice_char_vt };
}

/*============================================================================
 * Concatenation
 *============================================================================*/

/**
 * @brief Concatenate two strings into a new string
 * @param a Pointer to the first string
 * @param b Pointer to the second string
 * @return A new String containing a's content followed by b's content
 */
static inline String string_concat(const String *a, const String *b) {
    size_t total_len = a->len + b->len;
    
    /* Handle empty result case */
    if (total_len == 0) {
        return string_new();
    }
    
    String result = string_with_capacity(total_len);
    
    if (a->data && a->len > 0) {
        memcpy(result.data, a->data, a->len);
    }
    if (b->data && b->len > 0) {
        memcpy(result.data + a->len, b->data, b->len);
    }
    result.data[total_len] = '\0';
    result.len = total_len;
    
    return result;
}

/*============================================================================
 * Cleanup
 *============================================================================*/

/**
 * @brief Free all memory associated with the string
 * @param s Pointer to the string
 * @note Resets the string to empty state
 */
static inline void string_free(String *s) {
    free(s->data);
    s->data = NULL;
    s->len = 0;
    s->cap = 0;
}

/*============================================================================
 * Auto-Cleanup Macro
 *============================================================================*/

/**
 * @brief Declare a string with automatic cleanup on scope exit
 * @param name Variable name
 * @param init Initializer expression (e.g., string_from("hello"))
 * 
 * Example:
 *   string_auto(s, string_from("Hello"));
 *   // s is automatically freed when scope exits
 */
#define string_auto(name, init) \
    __attribute__((cleanup(string_free))) String name = (init)

/*============================================================================
 * Vtable Instance
 *============================================================================*/

/**
 * @brief Static const vtable instance shared by all String instances
 */
static const StringVT _string_vt = {
    .push = string_push,
    .append = string_append,
    .clear = string_clear,
    .get = string_get,
    .len = string_len,
    .cstr = string_cstr,
    .slice = string_slice,
    .free = string_free
};

/*============================================================================
 * String Convenience Macros
 *============================================================================*/

/**
 * @brief Push a character to the string via vtable
 * @param s The string (not a pointer)
 * @param c Character to append
 */
#define STR_PUSH(s, c) ((s).vt->push(&(s), (c)))

/**
 * @brief Append a C string to the string via vtable
 * @param s The string (not a pointer)
 * @param cstr C string to append
 */
#define STR_APPEND(s, cstr) ((s).vt->append(&(s), (cstr)))

/**
 * @brief Clear the string content via vtable
 * @param s The string (not a pointer)
 */
#define STR_CLEAR(s) ((s).vt->clear(&(s)))

/**
 * @brief Get character at index via vtable
 * @param s The string (not a pointer)
 * @param idx Index to access
 * @return Option_char containing the character, or None if out of bounds
 */
#define STR_GET(s, idx) ((s).vt->get(&(s), (idx)))

/**
 * @brief Get the length of the string via vtable
 * @param s The string (not a pointer)
 * @return Number of characters (excluding null terminator)
 */
#define STR_LEN(s) ((s).vt->len(&(s)))

/**
 * @brief Get the null-terminated C string via vtable
 * @param s The string (not a pointer)
 * @return Pointer to null-terminated character array
 */
#define STR_CSTR(s) ((s).vt->cstr(&(s)))

/**
 * @brief Create a slice view of a portion of the string via vtable
 * @param s The string (not a pointer)
 * @param start Start index (inclusive)
 * @param end End index (exclusive)
 * @return Slice_char viewing the specified range
 */
#define STR_SLICE(s, start, end) ((s).vt->slice(&(s), (start), (end)))

/**
 * @brief Free all memory associated with the string via vtable
 * @param s The string (not a pointer)
 */
#define STR_FREE(s) ((s).vt->free(&(s)))

#endif /* CYAN_STRING_H */

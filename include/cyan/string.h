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
#include <ctype.h>

/*============================================================================
 * Type Definitions
 *============================================================================*/

/* Define Option_char for string_get return type */
OPTION_DEFINE(char);

/* Option_size_t (for string_find) is provided by option.h */

/* Define Vec_char for slice_from_vec compatibility */
VECTOR_DEFINE(char);

/* Define Slice_char for string slicing */
SLICE_DEFINE(char);

/**
 * @brief Dynamic string type
 *
 * A heap-allocated, growable string with:
 * - data: null-terminated character buffer
 * - len: length excluding null terminator
 * - cap: capacity including null terminator
 */
typedef struct {
    char *data;      /* Null-terminated buffer */
    size_t len;      /* Length excluding null terminator */
    size_t cap;      /* Capacity including null terminator */
} String;

/*============================================================================
 * Constructors
 *============================================================================*/

/**
 * @brief Create an empty string
 * @return A new empty String
 */
static inline String string_new(void) {
    return (String){ .data = NULL, .len = 0, .cap = 0 };
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
    char *data = (char *)CYAN_MALLOC(cap);
    if (!data) CYAN_PANIC("allocation failed");
    memcpy(data, cstr, cap);  /* Includes null terminator */
    return (String){ .data = data, .len = len, .cap = cap };
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
    if (cap == SIZE_MAX) CYAN_PANIC("string capacity overflow");
    size_t actual_cap = cap + 1;  /* +1 for null terminator */
    char *data = (char *)CYAN_MALLOC(actual_cap);
    if (!data) CYAN_PANIC("allocation failed");
    data[0] = '\0';
    return (String){ .data = data, .len = 0, .cap = actual_cap };
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
    /* Guard len + additional + 1 against wrapping around SIZE_MAX */
    if (additional > SIZE_MAX - s->len - 1) CYAN_PANIC("string capacity overflow");
    size_t required = s->len + additional + 1;  /* +1 for null terminator */
    if (required <= s->cap) return;

    size_t new_cap = s->cap == 0 ? CYAN_DEFAULT_CAPACITY : s->cap;
    while (new_cap < required) {
        if (new_cap > SIZE_MAX / CYAN_GROWTH_FACTOR) {
            new_cap = required;
            break;
        }
        new_cap *= CYAN_GROWTH_FACTOR;
    }

    char *new_data = (char *)CYAN_REALLOC(s->data, new_cap);
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
 * @param cstr C string to append (NULL is a no-op)
 * @note Safe even when cstr points into s's own buffer (e.g. string_cstr(s)):
 *       the source is re-derived after any reallocation
 */
static inline void string_append(String *s, const char *cstr) {
    if (!cstr) return;
    size_t add_len = strlen(cstr);
    if (add_len == 0) return;

    /* Detect a source that aliases our own buffer before realloc can move it */
    bool self_alias = s->data && cstr >= s->data && cstr < s->data + s->cap;
    size_t src_offset = self_alias ? (size_t)(cstr - s->data) : 0;

    _string_check_capacity(s, add_len);
    const char *src = self_alias ? s->data + src_offset : cstr;
    memmove(s->data + s->len, src, add_len);
    s->len += add_len;
    s->data[s->len] = '\0';
}

/**
 * @brief Append another String to this string
 * @param s Pointer to the destination string
 * @param other Pointer to the source string (NULL is a no-op)
 * @note Safe for self-append (string_append_str(&s, &s))
 */
static inline void string_append_str(String *s, const String *other) {
    if (!other || other->len == 0) return;

    size_t add_len = other->len;
    _string_check_capacity(s, add_len);
    /* For self-append, other->data is s->data and already reflects any
     * reallocation; copy the character range and terminate explicitly so the
     * source null terminator is never part of an overlapping copy. */
    memmove(s->data + s->len, other->data, add_len);
    s->len += add_len;
    s->data[s->len] = '\0';
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

    /* Render into a temporary buffer first: format arguments may point into
     * s->data (e.g. string_format(&s, "%s", string_cstr(&s))), which growing
     * the buffer would invalidate, and vsnprintf must not read its output
     * region. */
    char *tmp = (char *)CYAN_MALLOC((size_t)needed + 1);
    if (!tmp) CYAN_PANIC("allocation failed");
    vsnprintf(tmp, (size_t)needed + 1, fmt, args_copy);
    va_end(args_copy);

    _string_check_capacity(s, (size_t)needed);
    memcpy(s->data + s->len, tmp, (size_t)needed + 1);
    CYAN_FREE(tmp);

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
 * Search and Comparison
 *============================================================================*/

/**
 * @brief Find the first occurrence of a substring
 * @param s Pointer to the string
 * @param needle Null-terminated substring to search for
 * @return Option_size_t containing the byte index of the first match,
 *         Some(0) for an empty needle, or None if not found / needle NULL
 */
static inline Option_size_t string_find(const String *s, const char *needle) {
    if (!needle) return None(size_t);
    if (*needle == '\0') return Some(size_t, 0);
    if (!s->data || s->len == 0) return None(size_t);
    const char *hit = strstr(s->data, needle);
    if (!hit) return None(size_t);
    return Some(size_t, (size_t)(hit - s->data));
}

/**
 * @brief Check whether the string contains a substring
 * @param s Pointer to the string
 * @param needle Null-terminated substring to search for
 * @return true if found (an empty needle always matches)
 */
static inline bool string_contains(const String *s, const char *needle) {
    return string_find(s, needle).has_value;
}

/**
 * @brief Check whether the string starts with a prefix
 * @param s Pointer to the string
 * @param prefix Null-terminated prefix (empty prefix always matches)
 * @return true if s begins with prefix
 */
static inline bool string_starts_with(const String *s, const char *prefix) {
    if (!prefix) return false;
    size_t plen = strlen(prefix);
    if (plen == 0) return true;
    if (plen > s->len) return false;
    return memcmp(s->data, prefix, plen) == 0;
}

/**
 * @brief Check whether the string ends with a suffix
 * @param s Pointer to the string
 * @param suffix Null-terminated suffix (empty suffix always matches)
 * @return true if s ends with suffix
 */
static inline bool string_ends_with(const String *s, const char *suffix) {
    if (!suffix) return false;
    size_t slen = strlen(suffix);
    if (slen == 0) return true;
    if (slen > s->len) return false;
    return memcmp(s->data + (s->len - slen), suffix, slen) == 0;
}

/**
 * @brief Compare two strings for content equality
 * @param a Pointer to the first string (NULL is treated as empty)
 * @param b Pointer to the second string (NULL is treated as empty)
 * @return true if both contain the same bytes
 */
static inline bool string_eq(const String *a, const String *b) {
    size_t a_len = a ? a->len : 0;
    size_t b_len = b ? b->len : 0;
    if (a_len != b_len) return false;
    if (a_len == 0) return true;
    return memcmp(a->data, b->data, a_len) == 0;
}

/**
 * @brief Trim leading and trailing whitespace in place
 * @param s Pointer to the string
 */
static inline void string_trim(String *s) {
    if (!s->data || s->len == 0) return;
    size_t start = 0;
    while (start < s->len && isspace((unsigned char)s->data[start])) start++;
    size_t end = s->len;
    while (end > start && isspace((unsigned char)s->data[end - 1])) end--;
    size_t new_len = end - start;
    if (start > 0 && new_len > 0) {
        memmove(s->data, s->data + start, new_len);
    }
    s->len = new_len;
    s->data[new_len] = '\0';
}

/*============================================================================
 * Splitting
 *============================================================================*/

/**
 * @brief Iterate delimiter-separated pieces of a character slice
 * @param rest In/out cursor over the remaining input; initialize with
 *             string_as_slice()/string_slice() (or any Slice_char) and pass
 *             the same variable on each call
 * @param delim Delimiter character
 * @param out Receives the next piece (excluding the delimiter); may be empty
 *            for consecutive delimiters
 * @return true if a piece was produced, false when input is exhausted
 *
 * Example:
 *   Slice_char rest = string_as_slice(&s);
 *   Slice_char part;
 *   while (string_split_next(&rest, ',', &part)) {
 *       // use part.data / part.len (not null-terminated)
 *   }
 *
 * @note Pieces view the original buffer and are not null-terminated.
 * @note "a,b," yields "a", "b", "" (a trailing delimiter yields a final
 *       empty piece); an empty slice (NULL data) yields no pieces.
 */
static inline bool string_split_next(Slice_char *rest, char delim, Slice_char *out) {
    if (!rest || !out) return false;
    /* Exhausted: signalled by a NULL cursor after the final piece */
    if (!rest->data) return false;

    size_t i = 0;
    while (i < rest->len && rest->data[i] != delim) i++;

    *out = (Slice_char){ .data = rest->data, .len = i };

    if (i < rest->len) {
        /* Skip the delimiter; remaining piece may be empty */
        rest->data = rest->data + i + 1;
        rest->len = rest->len - i - 1;
    } else {
        /* Consumed the final piece */
        rest->data = NULL;
        rest->len = 0;
    }
    return true;
}

/*============================================================================
 * Slice Materialization
 *============================================================================*/

/**
 * @brief Create an owned, null-terminated String from a character slice
 * @param s The slice to copy (e.g. a piece produced by string_split_next)
 * @return A new String containing a copy of the slice's bytes
 *
 * Example:
 *   Slice_char rest = string_as_slice(&csv), part;
 *   while (string_split_next(&rest, ',', &part)) {
 *       String field = string_from_slice(part);   // usable as a C string
 *       ...
 *       string_free(&field);
 *   }
 */
static inline String string_from_slice(Slice_char s) {
    if (!s.data || s.len == 0) {
        return string_new();
    }
    String out = string_with_capacity(s.len);
    memcpy(out.data, s.data, s.len);
    out.data[s.len] = '\0';
    out.len = s.len;
    return out;
}

/**
 * @brief Compare a character slice against a C string without materializing
 * @param s The slice (need not be null-terminated)
 * @param cstr The null-terminated string to compare with (NULL never matches)
 * @return true if the slice's bytes equal cstr exactly
 *
 * Example:
 *   if (string_slice_eq(part, "verbose")) { ... }
 */
static inline bool string_slice_eq(Slice_char s, const char *cstr) {
    if (!cstr) return false;
    size_t n = strlen(cstr);
    if (n != s.len) return false;
    if (n == 0) return true;
    return memcmp(s.data, cstr, n) == 0;
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
        return (Slice_char){ .data = NULL, .len = 0 };
    }
    if (start > s->len) start = s->len;
    if (end > s->len) end = s->len;
    if (start > end) start = end;
    return (Slice_char){ .data = s->data + start, .len = end - start };
}

/**
 * @brief Create a slice view of the entire string
 * @param s Pointer to the string
 * @return Slice_char viewing the entire string
 */
static inline Slice_char string_as_slice(const String *s) {
    if (!s->data) {
        return (Slice_char){ .data = NULL, .len = 0 };
    }
    return (Slice_char){ .data = s->data, .len = s->len };
}

/*============================================================================
 * Concatenation
 *============================================================================*/

/**
 * @brief Concatenate two strings into a new string
 * @param a Pointer to the first string (NULL is treated as an empty string)
 * @param b Pointer to the second string (NULL is treated as an empty string)
 * @return A new String containing a's content followed by b's content
 */
static inline String string_concat(const String *a, const String *b) {
    size_t a_len = a ? a->len : 0;
    size_t b_len = b ? b->len : 0;
    if (b_len > SIZE_MAX - a_len) CYAN_PANIC("string capacity overflow");
    size_t total_len = a_len + b_len;

    /* Handle empty result case */
    if (total_len == 0) {
        return string_new();
    }

    String result = string_with_capacity(total_len);

    if (a_len > 0 && a->data) {
        memcpy(result.data, a->data, a_len);
    }
    if (b_len > 0 && b->data) {
        memcpy(result.data + a_len, b->data, b_len);
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
    CYAN_FREE(s->data);
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
 * String Convenience Macros
 *============================================================================
 * String is monomorphic, so these need no type argument. Each argument is
 * evaluated exactly once.
 */

/**
 * @brief Push a character to the string
 * @param s The string (an lvalue, not a pointer)
 * @param c Character to append
 */
#define STR_PUSH(s, c) string_push(&(s), (c))

/**
 * @brief Append a C string to the string
 * @param s The string (an lvalue, not a pointer)
 * @param cstr C string to append
 */
#define STR_APPEND(s, cstr) string_append(&(s), (cstr))

/**
 * @brief Clear the string content
 * @param s The string (an lvalue, not a pointer)
 */
#define STR_CLEAR(s) string_clear(&(s))

/**
 * @brief Get character at index
 * @param s The string (an lvalue, not a pointer)
 * @param idx Index to access
 * @return Option_char containing the character, or None if out of bounds
 */
#define STR_GET(s, idx) string_get(&(s), (idx))

/**
 * @brief Get the length of the string
 * @param s The string (an lvalue, not a pointer)
 * @return Number of characters (excluding null terminator)
 */
#define STR_LEN(s) string_len(&(s))

/**
 * @brief Get the null-terminated C string
 * @param s The string (an lvalue, not a pointer)
 * @return Pointer to null-terminated character array
 */
#define STR_CSTR(s) string_cstr(&(s))

/**
 * @brief Create a slice view of a portion of the string
 * @param s The string (an lvalue, not a pointer)
 * @param start Start index (inclusive)
 * @param end End index (exclusive)
 * @return Slice_char viewing the specified range
 */
#define STR_SLICE(s, start, end) string_slice(&(s), (start), (end))

/**
 * @brief Find the first occurrence of a substring
 * @param s The string (an lvalue, not a pointer)
 * @param needle Substring to search for
 * @return Option_size_t index of first match, or None
 */
#define STR_FIND(s, needle) string_find(&(s), (needle))

/**
 * @brief Check whether the string contains a substring
 * @param s The string (an lvalue, not a pointer)
 * @param needle Substring to search for
 * @return true if found
 */
#define STR_CONTAINS(s, needle) string_contains(&(s), (needle))

/**
 * @brief Free all memory associated with the string
 * @param s The string (an lvalue, not a pointer)
 */
#define STR_FREE(s) string_free(&(s))

#endif /* CYAN_STRING_H */

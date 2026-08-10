/**
 * @file serialize.h
 * @brief Text serialization and parsing for primitive types
 * 
 * This header provides serialization and deserialization functions for
 * primitive types using a simple S-expression-like text format.
 * 
 * Grammar:
 *   value    := atom | list
 *   atom     := number | string | symbol
 *   number   := ['-'] digit+ ['.' digit+]
 *   string   := '"' char* '"'
 *   symbol   := alpha (alpha | digit | '_')*
 *   list     := '(' value* ')'
 * 
 * Usage:
 *   char *s = serialize_int(42);
 *   Result_int_const_charp r = parse_int(s, NULL);
 *   if (is_ok(r)) {
 *       int val = unwrap_ok(r);
 *   }
 *   free(s);
 */

#ifndef CYAN_SERIALIZE_H
#define CYAN_SERIALIZE_H

#include "common.h"
#include "result.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <math.h>

/*============================================================================
 * Type Aliases for Result Types
 *============================================================================*/

/* Type aliases needed for RESULT_DEFINE macro (which concatenates type names) */
typedef const char *ParseError;
typedef char *ParsedString;

/*============================================================================
 * Result Types for Parsing
 *============================================================================*/

/* Result type for int parsing */
RESULT_DEFINE(int, ParseError);

/* Result type for double parsing */
RESULT_DEFINE(double, ParseError);

/* Result type for string parsing (returns allocated string) */
RESULT_DEFINE(ParsedString, ParseError);

/*============================================================================
 * Serialization Functions
 *============================================================================*/

/**
 * @brief Serialize an integer to a string
 * @param val The integer value to serialize
 * @return Newly allocated string (caller must free)
 * 
 * Example:
 *   char *s = serialize_int(42);  // Returns "42"
 *   free(s);
 */
static inline char *serialize_int(int val) {
    /* Max int string: "-2147483648" = 11 chars + null */
    char *buf = (char *)CYAN_MALLOC(16);
    if (!buf) CYAN_PANIC("allocation failed");
    snprintf(buf, 16, "%d", val);
    return buf;
}

/**
 * @brief Serialize a double to a string
 * @param val The double value to serialize
 * @return Newly allocated string (caller must free)
 * 
 * Uses enough precision to round-trip the value.
 * 
 * Example:
 *   char *s = serialize_double(3.14);  // Returns "3.14"
 *   free(s);
 */
static inline char *serialize_double(double val) {
    /* Use enough buffer for full precision double */
    char *buf = (char *)CYAN_MALLOC(32);
    if (!buf) CYAN_PANIC("allocation failed");
    
    /* Handle special cases */
    if (isnan(val)) {
        strcpy(buf, "nan");
    } else if (isinf(val)) {
        strcpy(buf, val > 0 ? "inf" : "-inf");
    } else {
        /* Use %.17g for full double precision round-trip */
        snprintf(buf, 32, "%.17g", val);
    }
    return buf;
}

/**
 * @brief Serialize a string with proper escaping
 * @param str The string to serialize
 * @return Newly allocated quoted string (caller must free)
 * 
 * Escapes special characters: \n, \t, \r, \\, \"
 * 
 * Example:
 *   char *s = serialize_string("hello");  // Returns "\"hello\""
 *   free(s);
 */
static inline char *serialize_string(const char *str) {
    if (!str) {
        char *buf = (char *)CYAN_MALLOC(3);
        if (!buf) CYAN_PANIC("allocation failed");
        strcpy(buf, "\"\"");
        return buf;
    }
    
    /* Calculate required size with escaping */
    size_t len = 2;  /* Opening and closing quotes */
    for (const char *p = str; *p; p++) {
        switch (*p) {
            case '\n': case '\t': case '\r': case '\\': case '"':
                len += 2;  /* Escape sequence */
                break;
            default:
                len += 1;
                break;
        }
    }
    len += 1;  /* Null terminator */
    
    char *buf = (char *)CYAN_MALLOC(len);
    if (!buf) CYAN_PANIC("allocation failed");
    
    char *out = buf;
    *out++ = '"';
    for (const char *p = str; *p; p++) {
        switch (*p) {
            case '\n': *out++ = '\\'; *out++ = 'n'; break;
            case '\t': *out++ = '\\'; *out++ = 't'; break;
            case '\r': *out++ = '\\'; *out++ = 'r'; break;
            case '\\': *out++ = '\\'; *out++ = '\\'; break;
            case '"':  *out++ = '\\'; *out++ = '"'; break;
            default:   *out++ = *p; break;
        }
    }
    *out++ = '"';
    *out = '\0';
    
    return buf;
}

/*============================================================================
 * Parsing Helper Functions
 *============================================================================*/

/**
 * @brief Skip whitespace in input
 * @param input Pointer to current position
 * @return Pointer to first non-whitespace character
 */
static inline const char *skip_whitespace(const char *input) {
    while (*input && isspace((unsigned char)*input)) {
        input++;
    }
    return input;
}

/*============================================================================
 * Parsing Functions
 *============================================================================*/

/**
 * @brief Parse an integer from a string
 * @param input The input string to parse
 * @param end If non-NULL, set to point after parsed value
 * @return Result containing parsed int or error message
 * 
 * Example:
 *   const char *end;
 *   Result_int_ParseError r = parse_int("42 rest", &end);
 *   // unwrap_ok(r) == 42, end points to " rest"
 */
static inline Result_int_ParseError parse_int(const char *input, const char **end) {
    if (!input) {
        return Err(int, ParseError, "null input");
    }
    
    input = skip_whitespace(input);
    
    if (*input == '\0') {
        return Err(int, ParseError, "empty input");
    }
    
    /* Check for valid start of integer */
    if (!isdigit((unsigned char)*input) && *input != '-' && *input != '+') {
        return Err(int, ParseError, "invalid integer format");
    }
    
    char *parse_end;
    errno = 0;
    long val = strtol(input, &parse_end, 10);
    
    /* Check if any characters were consumed */
    if (parse_end == input) {
        return Err(int, ParseError, "no digits found");
    }
    
    /* Check for overflow */
    if (errno == ERANGE || val > INT_MAX || val < INT_MIN) {
        return Err(int, ParseError, "integer overflow");
    }
    
    if (end) {
        *end = parse_end;
    }
    
    return Ok(int, ParseError, (int)val);
}

/**
 * @brief Parse a double from a string
 * @param input The input string to parse
 * @param end If non-NULL, set to point after parsed value
 * @return Result containing parsed double or error message
 * 
 * Handles special values: nan, inf, -inf
 * 
 * Example:
 *   const char *end;
 *   Result_double_ParseError r = parse_double("3.14 rest", &end);
 *   // unwrap_ok(r) == 3.14, end points to " rest"
 */
static inline Result_double_ParseError parse_double(const char *input, const char **end) {
    if (!input) {
        return Err(double, ParseError, "null input");
    }
    
    input = skip_whitespace(input);
    
    if (*input == '\0') {
        return Err(double, ParseError, "empty input");
    }
    
    /* Handle special values */
    if (strncmp(input, "nan", 3) == 0) {
        if (end) *end = input + 3;
        return Ok(double, ParseError, NAN);
    }
    if (strncmp(input, "inf", 3) == 0) {
        if (end) *end = input + 3;
        return Ok(double, ParseError, INFINITY);
    }
    if (strncmp(input, "-inf", 4) == 0) {
        if (end) *end = input + 4;
        return Ok(double, ParseError, -INFINITY);
    }
    
    /* Check for valid start of number */
    if (!isdigit((unsigned char)*input) && *input != '-' && *input != '+' && *input != '.') {
        return Err(double, ParseError, "invalid double format");
    }
    
    char *parse_end;
    errno = 0;
    double val = strtod(input, &parse_end);
    
    /* Check if any characters were consumed */
    if (parse_end == input) {
        return Err(double, ParseError, "no digits found");
    }
    
    /* Check for overflow (underflow to zero is acceptable) */
    if (errno == ERANGE && (val == HUGE_VAL || val == -HUGE_VAL)) {
        return Err(double, ParseError, "double overflow");
    }
    
    if (end) {
        *end = parse_end;
    }
    
    return Ok(double, ParseError, val);
}

/**
 * @brief Parse a quoted string from input
 * @param input The input string to parse (must start with ")
 * @param end If non-NULL, set to point after closing quote
 * @return Result containing newly allocated string or error message
 * 
 * Handles escape sequences: \n, \t, \r, \\, \"
 * 
 * Example:
 *   const char *end;
 *   Result_ParsedString_ParseError r = parse_string("\"hello\" rest", &end);
 *   // unwrap_ok(r) == "hello", end points to " rest"
 */
static inline Result_ParsedString_ParseError parse_string(const char *input, const char **end) {
    if (!input) {
        return Err(ParsedString, ParseError, "null input");
    }
    
    input = skip_whitespace(input);
    
    if (*input == '\0') {
        return Err(ParsedString, ParseError, "empty input");
    }
    
    if (*input != '"') {
        return Err(ParsedString, ParseError, "string must start with quote");
    }
    
    input++;  /* Skip opening quote */
    
    /* First pass: calculate required size */
    size_t len = 0;
    const char *p = input;
    while (*p && *p != '"') {
        if (*p == '\\') {
            p++;
            if (*p == '\0') {
                return Err(ParsedString, ParseError, "unterminated escape sequence");
            }
        }
        len++;
        p++;
    }
    
    if (*p != '"') {
        return Err(ParsedString, ParseError, "unterminated string");
    }
    
    /* Allocate and fill buffer */
    char *buf = (char *)CYAN_MALLOC(len + 1);
    if (!buf) CYAN_PANIC("allocation failed");
    
    char *out = buf;
    p = input;
    while (*p && *p != '"') {
        if (*p == '\\') {
            p++;
            switch (*p) {
                case 'n':  *out++ = '\n'; break;
                case 't':  *out++ = '\t'; break;
                case 'r':  *out++ = '\r'; break;
                case '\\': *out++ = '\\'; break;
                case '"':  *out++ = '"'; break;
                default:   *out++ = *p; break;  /* Unknown escape, keep as-is */
            }
        } else {
            *out++ = *p;
        }
        p++;
    }
    *out = '\0';
    
    if (end) {
        *end = p + 1;  /* Skip closing quote */
    }
    
    return Ok(ParsedString, ParseError, buf);
}

/*============================================================================
 * Generic Serialization Macro
 *============================================================================*/

/**
 * @brief Generic serialization macro using _Generic
 * @param val The value to serialize
 * @return Newly allocated string (caller must free)
 * 
 * Automatically selects the appropriate serialization function based on type.
 * 
 * Example:
 *   char *s1 = serialize(42);       // Uses serialize_int
 *   char *s2 = serialize(3.14);     // Uses serialize_double
 *   char *s3 = serialize("hello");  // Uses serialize_string
 */
#define serialize(val) _Generic((val), \
    int: serialize_int, \
    long: serialize_long, \
    double: serialize_double, \
    float: serialize_float, \
    char*: serialize_string, \
    const char*: serialize_string \
)(val)

/**
 * @brief Serialize a long to a string
 * @param val The long value to serialize
 * @return Newly allocated string (caller must free)
 */
static inline char *serialize_long(long val) {
    char *buf = (char *)CYAN_MALLOC(24);
    if (!buf) CYAN_PANIC("allocation failed");
    snprintf(buf, 24, "%ld", val);
    return buf;
}

/**
 * @brief Serialize a float to a string
 * @param val The float value to serialize
 * @return Newly allocated string (caller must free)
 */
static inline char *serialize_float(float val) {
    char *buf = (char *)CYAN_MALLOC(32);
    if (!buf) CYAN_PANIC("allocation failed");
    
    if (isnan(val)) {
        strcpy(buf, "nan");
    } else if (isinf(val)) {
        strcpy(buf, val > 0 ? "inf" : "-inf");
    } else {
        snprintf(buf, 32, "%.9g", (double)val);
    }
    return buf;
}

/*============================================================================
 * Pretty Print Function
 *============================================================================*/

/**
 * @brief Pretty print a serialized value with indentation
 * @param serialized The serialized string to format
 * @param indent_width Number of spaces per indentation level
 * @return Newly allocated formatted string (caller must free)
 * 
 * For simple values (numbers, strings), returns a copy.
 * For lists, adds newlines and indentation.
 * 
 * Example:
 *   char *pretty = pretty_print("(1 2 3)", 2);
 *   // Returns:
 *   // (
 *   //   1
 *   //   2
 *   //   3
 *   // )
 */
/**
 * @brief Ensure the pretty_print output buffer can hold `extra` more bytes
 * @param buf Pointer to the buffer pointer (updated on realloc)
 * @param cap Pointer to the current capacity (updated on realloc)
 * @param len Bytes currently used
 * @param extra Additional bytes needed (excluding null terminator)
 */
static inline void _cyan_pp_reserve(char **buf, size_t *cap, size_t len, size_t extra) {
    if (extra > SIZE_MAX - len - 1) CYAN_PANIC("pretty_print: output too large");
    size_t needed = len + extra + 1;
    if (needed <= *cap) return;
    size_t new_cap = *cap;
    while (new_cap < needed) {
        if (new_cap > SIZE_MAX / 2) {
            new_cap = needed;
            break;
        }
        new_cap *= 2;
    }
    char *new_buf = (char *)CYAN_REALLOC(*buf, new_cap);
    if (!new_buf) CYAN_PANIC("allocation failed");
    *buf = new_buf;
    *cap = new_cap;
}

static inline char *pretty_print(const char *serialized, int indent_width) {
    if (!serialized) {
        char *empty = (char *)CYAN_MALLOC(1);
        if (!empty) CYAN_PANIC("allocation failed");
        empty[0] = '\0';
        return empty;
    }
    if (indent_width < 0) indent_width = 0;

    /* The output grows dynamically: deeply nested input needs
     * depth * indent_width bytes of indentation per line, which a fixed
     * per-character estimate cannot bound (it previously overflowed the
     * heap on inputs like "((((((...") */
    size_t input_len = strlen(serialized);
    size_t cap = input_len + 16;
    char *buf = (char *)CYAN_MALLOC(cap);
    if (!buf) CYAN_PANIC("allocation failed");

    size_t len = 0;
    size_t depth = 0;
    bool in_string = false;
    bool escape_next = false;
    bool prev_was_open = false;
    size_t indent = (size_t)indent_width;

    for (const char *p = serialized; *p; p++) {
        if (in_string) {
            _cyan_pp_reserve(&buf, &cap, len, 1);
            buf[len++] = *p;
            /* Track escapes as a state machine so a string ending in an
             * escaped backslash (\\") still closes on its real quote */
            if (escape_next) {
                escape_next = false;
            } else if (*p == '\\') {
                escape_next = true;
            } else if (*p == '"') {
                in_string = false;
            }
            continue;
        }

        switch (*p) {
            case '"':
                in_string = true;
                escape_next = false;
                _cyan_pp_reserve(&buf, &cap, len, 1);
                buf[len++] = *p;
                prev_was_open = false;
                break;

            case '(':
                depth++;
                _cyan_pp_reserve(&buf, &cap, len, 2 + depth * indent);
                buf[len++] = '(';
                buf[len++] = '\n';
                for (size_t i = 0; i < depth * indent; i++) {
                    buf[len++] = ' ';
                }
                prev_was_open = true;
                break;

            case ')':
                if (!prev_was_open) {
                    if (depth > 0) depth--;
                    _cyan_pp_reserve(&buf, &cap, len, 2 + depth * indent);
                    buf[len++] = '\n';
                    for (size_t i = 0; i < depth * indent; i++) {
                        buf[len++] = ' ';
                    }
                } else {
                    /* Remove the newline and indent we just added */
                    len -= depth * indent + 1;
                    if (depth > 0) depth--;
                    _cyan_pp_reserve(&buf, &cap, len, 1);
                }
                buf[len++] = ')';
                prev_was_open = false;
                break;

            case ' ':
            case '\t':
            case '\n':
            case '\r':
                /* In a list, whitespace separates elements */
                if (depth > 0 && !prev_was_open) {
                    /* Skip consecutive whitespace */
                    while (*(p+1) && isspace((unsigned char)*(p+1))) p++;
                    if (*(p+1) && *(p+1) != ')') {
                        _cyan_pp_reserve(&buf, &cap, len, 1 + depth * indent);
                        buf[len++] = '\n';
                        for (size_t i = 0; i < depth * indent; i++) {
                            buf[len++] = ' ';
                        }
                    }
                } else if (depth == 0) {
                    _cyan_pp_reserve(&buf, &cap, len, 1);
                    buf[len++] = *p;
                }
                prev_was_open = false;
                break;

            default:
                _cyan_pp_reserve(&buf, &cap, len, 1);
                buf[len++] = *p;
                prev_was_open = false;
                break;
        }
    }

    buf[len] = '\0';

    /* Shrink buffer to actual size */
    char *result = (char *)CYAN_REALLOC(buf, len + 1);
    return result ? result : buf;
}

/*============================================================================
 * S-Expression Values (SExp)
 *============================================================================
 * A tagged tree type implementing the full documented grammar, including
 * nested lists and symbols. parse_sexp/serialize_sexp round-trip:
 *   parse(serialize(x)) is structurally equal to x (see sexp_eq).
 */

/**
 * @brief Maximum nesting depth accepted by parse_sexp
 * Override by defining CYAN_SEXP_MAX_DEPTH before including headers.
 */
#ifndef CYAN_SEXP_MAX_DEPTH
#define CYAN_SEXP_MAX_DEPTH 1000
#endif

/**
 * @brief Kind of an S-expression node
 */
typedef enum {
    SEXP_INT,     /**< Integer atom (long) */
    SEXP_DOUBLE,  /**< Floating-point atom (includes nan/inf) */
    SEXP_STRING,  /**< Quoted string atom */
    SEXP_SYMBOL,  /**< Bare symbol atom */
    SEXP_LIST     /**< List of child S-expressions */
} SExpType;

/**
 * @brief Heap-allocated S-expression node
 */
typedef struct SExp {
    SExpType type;
    union {
        long i;                   /* SEXP_INT */
        double d;                 /* SEXP_DOUBLE */
        char *str;                /* SEXP_STRING / SEXP_SYMBOL (owned) */
        struct {                  /* SEXP_LIST */
            struct SExp **items;  /* owned child pointers */
            size_t len;
            size_t cap;
        } list;
    };
} SExp;

/* Result type for S-expression parsing */
typedef SExp *SExpPtr;
RESULT_DEFINE(SExpPtr, ParseError);

/*----------------------------------------------------------------------------
 * Constructors and destructor
 *----------------------------------------------------------------------------*/

static inline SExp *_sexp_alloc(SExpType type) {
    SExp *e = (SExp *)CYAN_MALLOC(sizeof(SExp));
    if (!e) CYAN_PANIC("allocation failed");
    memset(e, 0, sizeof(SExp));
    e->type = type;
    return e;
}

/** @brief Create an integer atom */
static inline SExp *sexp_int(long value) {
    SExp *e = _sexp_alloc(SEXP_INT);
    e->i = value;
    return e;
}

/** @brief Create a double atom */
static inline SExp *sexp_double(double value) {
    SExp *e = _sexp_alloc(SEXP_DOUBLE);
    e->d = value;
    return e;
}

/** @brief Create a string atom (copies the input) */
static inline SExp *sexp_string(const char *value) {
    SExp *e = _sexp_alloc(SEXP_STRING);
    size_t n = value ? strlen(value) + 1 : 1;
    e->str = (char *)CYAN_MALLOC(n);
    if (!e->str) CYAN_PANIC("allocation failed");
    memcpy(e->str, value ? value : "", n);
    return e;
}

/** @brief Create a symbol atom (copies the input) */
static inline SExp *sexp_symbol(const char *name) {
    SExp *e = sexp_string(name);
    e->type = SEXP_SYMBOL;
    return e;
}

/** @brief Create an empty list */
static inline SExp *sexp_list_new(void) {
    return _sexp_alloc(SEXP_LIST);
}

/** @brief Free an S-expression tree recursively (NULL is a no-op) */
static inline void sexp_free(SExp *e) {
    if (!e) return;
    switch (e->type) {
        case SEXP_STRING:
        case SEXP_SYMBOL:
            CYAN_FREE(e->str);
            break;
        case SEXP_LIST:
            for (size_t i = 0; i < e->list.len; i++) {
                sexp_free(e->list.items[i]);
            }
            CYAN_FREE(e->list.items);
            break;
        default:
            break;
    }
    CYAN_FREE(e);
}

/**
 * @brief Append a child to a list (takes ownership of child)
 * @note Panics if list is not SEXP_LIST or on allocation failure
 */
static inline void sexp_list_push(SExp *list, SExp *child) {
    if (!list || list->type != SEXP_LIST) CYAN_PANIC("sexp_list_push: not a list");
    if (list->list.len >= list->list.cap) {
        size_t new_cap = list->list.cap == 0 ? CYAN_DEFAULT_CAPACITY
                                             : list->list.cap * CYAN_GROWTH_FACTOR;
        if (new_cap > SIZE_MAX / sizeof(SExp *)) CYAN_PANIC("sexp list overflow");
        SExp **items = (SExp **)CYAN_REALLOC(list->list.items, new_cap * sizeof(SExp *));
        if (!items) CYAN_PANIC("allocation failed");
        list->list.items = items;
        list->list.cap = new_cap;
    }
    list->list.items[list->list.len++] = child;
}

/**
 * @brief Structural equality of two S-expression trees
 * @note NaN doubles compare equal to each other (so round-trips verify)
 */
static inline bool sexp_eq(const SExp *a, const SExp *b) {
    if (a == b) return true;
    if (!a || !b || a->type != b->type) return false;
    switch (a->type) {
        case SEXP_INT:    return a->i == b->i;
        case SEXP_DOUBLE:
            if (isnan(a->d) && isnan(b->d)) return true;
            return a->d == b->d;
        case SEXP_STRING:
        case SEXP_SYMBOL: return strcmp(a->str, b->str) == 0;
        case SEXP_LIST:
            if (a->list.len != b->list.len) return false;
            for (size_t i = 0; i < a->list.len; i++) {
                if (!sexp_eq(a->list.items[i], b->list.items[i])) return false;
            }
            return true;
    }
    return false;
}

/*----------------------------------------------------------------------------
 * Parsing
 *----------------------------------------------------------------------------*/

/* Internal: parse one value at *input; on success advances *end */
static inline Result_SExpPtr_ParseError _parse_sexp_value(
    const char *input, const char **end, int depth
) {
    if (depth > CYAN_SEXP_MAX_DEPTH) {
        return Err(SExpPtr, ParseError, "maximum nesting depth exceeded");
    }

    input = skip_whitespace(input);
    if (*input == '\0') {
        return Err(SExpPtr, ParseError, "empty input");
    }

    /* List */
    if (*input == '(') {
        input++;
        SExp *list = sexp_list_new();
        for (;;) {
            input = skip_whitespace(input);
            if (*input == ')') {
                input++;
                break;
            }
            if (*input == '\0') {
                sexp_free(list);
                return Err(SExpPtr, ParseError, "unterminated list");
            }
            const char *child_end;
            Result_SExpPtr_ParseError child = _parse_sexp_value(input, &child_end, depth + 1);
            if (!child.is_ok_flag) {
                sexp_free(list);
                return child;
            }
            sexp_list_push(list, child.ok_value);
            input = child_end;
        }
        *end = input;
        return Ok(SExpPtr, ParseError, list);
    }

    /* Quoted string */
    if (*input == '"') {
        const char *str_end;
        Result_ParsedString_ParseError s = parse_string(input, &str_end);
        if (!s.is_ok_flag) {
            return Err(SExpPtr, ParseError, s.err_value);
        }
        SExp *e = _sexp_alloc(SEXP_STRING);
        e->str = s.ok_value; /* take ownership of the parsed buffer */
        *end = str_end;
        return Ok(SExpPtr, ParseError, e);
    }

    /* Special doubles (match serialize_double output) */
    if (strncmp(input, "nan", 3) == 0 && !isalnum((unsigned char)input[3]) && input[3] != '_') {
        *end = input + 3;
        return Ok(SExpPtr, ParseError, sexp_double(NAN));
    }
    if (strncmp(input, "inf", 3) == 0 && !isalnum((unsigned char)input[3]) && input[3] != '_') {
        *end = input + 3;
        return Ok(SExpPtr, ParseError, sexp_double(INFINITY));
    }
    if (strncmp(input, "-inf", 4) == 0 && !isalnum((unsigned char)input[4]) && input[4] != '_') {
        *end = input + 4;
        return Ok(SExpPtr, ParseError, sexp_double(-INFINITY));
    }

    /* Number */
    if (isdigit((unsigned char)*input) ||
        ((*input == '-' || *input == '+') && (isdigit((unsigned char)input[1]) || input[1] == '.')) ||
        (*input == '.' && isdigit((unsigned char)input[1]))) {
        char *int_end;
        char *dbl_end;
        errno = 0;
        long ival = strtol(input, &int_end, 10);
        bool int_overflow = (errno == ERANGE);
        errno = 0;
        double dval = strtod(input, &dbl_end);
        if (dbl_end == input) {
            return Err(SExpPtr, ParseError, "invalid number");
        }
        if (errno == ERANGE && (dval == HUGE_VAL || dval == -HUGE_VAL)) {
            return Err(SExpPtr, ParseError, "double overflow");
        }
        if (int_end == dbl_end && !int_overflow) {
            *end = int_end;
            return Ok(SExpPtr, ParseError, sexp_int(ival));
        }
        *end = dbl_end;
        return Ok(SExpPtr, ParseError, sexp_double(dval));
    }

    /* Symbol */
    if (isalpha((unsigned char)*input) || *input == '_') {
        const char *p = input + 1;
        while (isalnum((unsigned char)*p) || *p == '_') p++;
        size_t n = (size_t)(p - input);
        char *name = (char *)CYAN_MALLOC(n + 1);
        if (!name) CYAN_PANIC("allocation failed");
        memcpy(name, input, n);
        name[n] = '\0';
        SExp *e = _sexp_alloc(SEXP_SYMBOL);
        e->str = name;
        *end = p;
        return Ok(SExpPtr, ParseError, e);
    }

    return Err(SExpPtr, ParseError, "unexpected character");
}

/**
 * @brief Parse a complete S-expression value (atom or nested list)
 * @param input The input text
 * @param end If non-NULL, set to point after the parsed value
 * @return Result containing a heap-allocated SExp tree (free with sexp_free)
 *         or an error message
 *
 * Example:
 *   Result_SExpPtr_ParseError r = parse_sexp("(1 2 (3 4) 5)", NULL);
 *   if (is_ok(r)) {
 *       SExp *e = unwrap_ok(r);
 *       // e->list.len == 4
 *       sexp_free(e);
 *   }
 */
static inline Result_SExpPtr_ParseError parse_sexp(const char *input, const char **end) {
    if (!input) {
        return Err(SExpPtr, ParseError, "null input");
    }
    const char *local_end;
    Result_SExpPtr_ParseError r = _parse_sexp_value(input, &local_end, 0);
    if (r.is_ok_flag && end) {
        *end = local_end;
    }
    return r;
}

/*----------------------------------------------------------------------------
 * Serialization
 *----------------------------------------------------------------------------*/

/* Internal: append bytes to a growing buffer (reuses _cyan_pp_reserve) */
static inline void _sexp_write(char **buf, size_t *cap, size_t *len,
                               const char *src, size_t n) {
    _cyan_pp_reserve(buf, cap, *len, n);
    memcpy(*buf + *len, src, n);
    *len += n;
}

/* Internal: recursively render a tree into the buffer */
static inline void _sexp_render(const SExp *e, char **buf, size_t *cap, size_t *len) {
    switch (e->type) {
        case SEXP_INT: {
            char tmp[32];
            int n = snprintf(tmp, sizeof(tmp), "%ld", e->i);
            _sexp_write(buf, cap, len, tmp, (size_t)n);
            break;
        }
        case SEXP_DOUBLE: {
            char *s = serialize_double(e->d);
            _sexp_write(buf, cap, len, s, strlen(s));
            CYAN_FREE(s);
            break;
        }
        case SEXP_STRING: {
            char *s = serialize_string(e->str);
            _sexp_write(buf, cap, len, s, strlen(s));
            CYAN_FREE(s);
            break;
        }
        case SEXP_SYMBOL:
            _sexp_write(buf, cap, len, e->str, strlen(e->str));
            break;
        case SEXP_LIST:
            _sexp_write(buf, cap, len, "(", 1);
            for (size_t i = 0; i < e->list.len; i++) {
                if (i > 0) _sexp_write(buf, cap, len, " ", 1);
                _sexp_render(e->list.items[i], buf, cap, len);
            }
            _sexp_write(buf, cap, len, ")", 1);
            break;
    }
}

/**
 * @brief Serialize an S-expression tree to text
 * @param e The tree to serialize (NULL yields an empty string)
 * @return Newly allocated string (caller must free)
 *
 * Output parses back to a structurally equal tree:
 *   sexp_eq(unwrap_ok(parse_sexp(serialize_sexp(e), NULL)), e)
 */
static inline char *serialize_sexp(const SExp *e) {
    size_t cap = 64;
    size_t len = 0;
    char *buf = (char *)CYAN_MALLOC(cap);
    if (!buf) CYAN_PANIC("allocation failed");
    if (e) {
        _sexp_render(e, &buf, &cap, &len);
    }
    buf[len] = '\0';
    return buf;
}

#endif /* CYAN_SERIALIZE_H */

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
    char *buf = (char *)malloc(16);
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
    char *buf = (char *)malloc(32);
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
        char *buf = (char *)malloc(3);
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
    
    char *buf = (char *)malloc(len);
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
    char *buf = (char *)malloc(len + 1);
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
    char *buf = (char *)malloc(24);
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
    char *buf = (char *)malloc(32);
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
static inline char *pretty_print(const char *serialized, int indent_width) {
    if (!serialized) {
        char *buf = (char *)malloc(1);
        if (!buf) CYAN_PANIC("allocation failed");
        buf[0] = '\0';
        return buf;
    }
    
    /* Calculate output size (worst case: each char gets newline + indent) */
    size_t input_len = strlen(serialized);
    size_t max_output = input_len * (1 + indent_width + 1) + 1;
    
    char *buf = (char *)malloc(max_output);
    if (!buf) CYAN_PANIC("allocation failed");
    
    char *out = buf;
    int depth = 0;
    bool in_string = false;
    bool prev_was_open = false;
    
    for (const char *p = serialized; *p; p++) {
        if (in_string) {
            *out++ = *p;
            if (*p == '"' && (p == serialized || *(p-1) != '\\')) {
                in_string = false;
            }
            continue;
        }
        
        switch (*p) {
            case '"':
                in_string = true;
                *out++ = *p;
                prev_was_open = false;
                break;
                
            case '(':
                *out++ = '(';
                *out++ = '\n';
                depth++;
                for (int i = 0; i < depth * indent_width; i++) {
                    *out++ = ' ';
                }
                prev_was_open = true;
                break;
                
            case ')':
                if (!prev_was_open) {
                    *out++ = '\n';
                    depth--;
                    for (int i = 0; i < depth * indent_width; i++) {
                        *out++ = ' ';
                    }
                } else {
                    /* Remove the newline and indent we just added */
                    out -= (depth * indent_width + 1);
                    depth--;
                }
                *out++ = ')';
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
                        *out++ = '\n';
                        for (int i = 0; i < depth * indent_width; i++) {
                            *out++ = ' ';
                        }
                    }
                } else if (depth == 0) {
                    *out++ = *p;
                }
                prev_was_open = false;
                break;
                
            default:
                *out++ = *p;
                prev_was_open = false;
                break;
        }
    }
    
    *out = '\0';
    
    /* Shrink buffer to actual size */
    size_t actual_len = (size_t)(out - buf) + 1;
    char *result = (char *)realloc(buf, actual_len);
    return result ? result : buf;
}

#endif /* CYAN_SERIALIZE_H */

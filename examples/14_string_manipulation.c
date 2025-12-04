/**
 * @file 14_string_manipulation.c
 * @brief Example demonstrating dynamic string manipulation
 * 
 * This example shows how to use the String type for safe, growable
 * text manipulation with automatic memory management.
 */

#include <cyan/cyan.h>
#include <stdio.h>

int main(void) {
    printf("=== String Manipulation Example ===\n\n");
    
    /* --------------------------------------------------------
     * 1. Creating strings
     * -------------------------------------------------------- */
    printf("1. Creating strings\n");
    
    /* Empty string */
    String empty = string_new();
    printf("   Empty string length: %zu\n", string_len(&empty));
    
    /* From C string */
    String hello = string_from("Hello, World!");
    printf("   From C string: \"%s\" (len=%zu)\n", string_cstr(&hello), string_len(&hello));
    
    /* With pre-allocated capacity */
    String preallocated = string_with_capacity(100);
    printf("   Pre-allocated capacity: created (len=%zu)\n\n", string_len(&preallocated));
    
    string_free(&empty);
    string_free(&preallocated);
    
    /* --------------------------------------------------------
     * 2. Appending content
     * -------------------------------------------------------- */
    printf("2. Appending content\n");
    
    String s = string_from("Hello");
    printf("   Initial: \"%s\"\n", string_cstr(&s));
    
    string_append(&s, ", ");
    printf("   After append \", \": \"%s\"\n", string_cstr(&s));
    
    string_append(&s, "World!");
    printf("   After append \"World!\": \"%s\"\n\n", string_cstr(&s));
    
    string_free(&s);
    
    /* --------------------------------------------------------
     * 3. Pushing characters
     * -------------------------------------------------------- */
    printf("3. Pushing individual characters\n");
    
    String chars = string_new();
    string_push(&chars, 'A');
    string_push(&chars, 'B');
    string_push(&chars, 'C');
    printf("   After pushing A, B, C: \"%s\"\n\n", string_cstr(&chars));
    
    string_free(&chars);
    
    /* --------------------------------------------------------
     * 4. Formatted strings
     * -------------------------------------------------------- */
    printf("4. Formatted strings (like sprintf)\n");
    
    String formatted = string_formatted("Value: %d, Pi: %.2f", 42, 3.14159);
    printf("   string_formatted: \"%s\"\n", string_cstr(&formatted));
    
    /* Append formatted content */
    String base = string_from("Results: ");
    string_format(&base, "[%d, %d, %d]", 1, 2, 3);
    printf("   string_format append: \"%s\"\n\n", string_cstr(&base));
    
    string_free(&formatted);
    string_free(&base);
    
    /* --------------------------------------------------------
     * 5. Accessing characters
     * -------------------------------------------------------- */
    printf("5. Accessing characters with bounds checking\n");
    
    String text = string_from("Hello");
    
    Option_char c = string_get(&text, 0);
    if (is_some(c)) {
        printf("   Character at 0: '%c'\n", unwrap(c));
    }
    
    c = string_get(&text, 4);
    if (is_some(c)) {
        printf("   Character at 4: '%c'\n", unwrap(c));
    }
    
    c = string_get(&text, 100);
    printf("   Character at 100 exists: %s\n\n", is_some(c) ? "yes" : "no");
    
    string_free(&text);
    
    /* --------------------------------------------------------
     * 6. String slicing
     * -------------------------------------------------------- */
    printf("6. String slicing (non-owning views)\n");
    
    String sentence = string_from("The quick brown fox");
    
    Slice_char slice1 = string_slice(&sentence, 0, 3);
    printf("   slice(0, 3): \"");
    for (size_t i = 0; i < slice_char_len(slice1); i++) {
        printf("%c", unwrap(slice_char_get(slice1, i)));
    }
    printf("\"\n");
    
    Slice_char slice2 = string_slice(&sentence, 4, 9);
    printf("   slice(4, 9): \"");
    for (size_t i = 0; i < slice_char_len(slice2); i++) {
        printf("%c", unwrap(slice_char_get(slice2, i)));
    }
    printf("\"\n");
    
    Slice_char full = string_as_slice(&sentence);
    printf("   Full slice length: %zu\n\n", slice_char_len(full));
    
    string_free(&sentence);
    
    /* --------------------------------------------------------
     * 7. String concatenation
     * -------------------------------------------------------- */
    printf("7. String concatenation\n");
    
    String a = string_from("Hello, ");
    String b = string_from("World!");
    String combined = string_concat(&a, &b);
    
    printf("   \"%s\" + \"%s\" = \"%s\"\n\n", 
           string_cstr(&a), string_cstr(&b), string_cstr(&combined));
    
    string_free(&a);
    string_free(&b);
    string_free(&combined);
    
    /* --------------------------------------------------------
     * 8. Clearing strings
     * -------------------------------------------------------- */
    printf("8. Clearing strings (keeps capacity)\n");
    
    String clearable = string_from("Some content here");
    printf("   Before clear: \"%s\" (len=%zu)\n", 
           string_cstr(&clearable), string_len(&clearable));
    
    string_clear(&clearable);
    printf("   After clear: \"%s\" (len=%zu)\n", 
           string_cstr(&clearable), string_len(&clearable));
    
    string_append(&clearable, "New content");
    printf("   After new append: \"%s\"\n\n", string_cstr(&clearable));
    
    string_free(&clearable);
    
    /* --------------------------------------------------------
     * 9. Using vtable convenience macros
     * -------------------------------------------------------- */
    printf("9. Using vtable convenience macros\n");
    
    String str = string_from("Test");
    
    STR_PUSH(str, '!');
    printf("   After STR_PUSH('!'): \"%s\"\n", STR_CSTR(str));
    
    STR_APPEND(str, " More");
    printf("   After STR_APPEND(\" More\"): \"%s\"\n", STR_CSTR(str));
    
    printf("   STR_LEN: %zu\n", STR_LEN(str));
    printf("   STR_GET(0): '%c'\n", unwrap(STR_GET(str, 0)));
    
    STR_CLEAR(str);
    printf("   After STR_CLEAR: \"%s\" (len=%zu)\n\n", STR_CSTR(str), STR_LEN(str));
    
    STR_FREE(str);
    
    /* --------------------------------------------------------
     * 10. Auto-cleanup with string_auto
     * -------------------------------------------------------- */
    printf("10. Auto-cleanup with string_auto\n");
    {
        string_auto(auto_str, string_from("I will be freed automatically"));
        printf("   Inside scope: \"%s\"\n", string_cstr(&auto_str));
        /* auto_str is automatically freed when scope exits */
    }
    printf("   Scope exited - string was auto-freed\n\n");
    
    /* --------------------------------------------------------
     * 11. Appending another String
     * -------------------------------------------------------- */
    printf("11. Appending another String object\n");
    
    String dest = string_from("Start: ");
    String src = string_from("appended content");
    
    string_append_str(&dest, &src);
    printf("   Result: \"%s\"\n\n", string_cstr(&dest));
    
    string_free(&dest);
    string_free(&src);
    
    /* --------------------------------------------------------
     * Cleanup
     * -------------------------------------------------------- */
    string_free(&hello);
    
    printf("=== String example complete ===\n");
    return 0;
}

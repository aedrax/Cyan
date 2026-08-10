/**
 * @file 08_method_macros.c
 * @brief Example demonstrating the method-style macro API
 *
 * Every Cyan type can be used through two equivalent call styles:
 * 1. Standalone functions: vec_i32_push(&v, 42)
 * 2. Type-first method-style macros: VEC_PUSH(i32, v, 42)
 *
 * The macros expand directly to the standalone functions - there is no
 * indirection and no hidden state. All Cyan types are plain structs.
 *
 * Compile: gcc -std=c11 -I../include -o method_macros 08_method_macros.c
 * Run: ./method_macros
 */

#include <stdio.h>
#include <cyan/common.h>
#include <cyan/option.h>
#include <cyan/result.h>
#include <cyan/vector.h>
#include <cyan/hashmap.h>
#include <cyan/slice.h>
#include <cyan/string.h>
#include <cyan/smartptr.h>
#include <cyan/channel.h>

// Define types
// Note: CHANNEL_DEFINE internally calls OPTION_DEFINE, so we define Channel first
// to avoid duplicate definitions. Other types that need Option_i32 will use
// the one already defined by CHANNEL_DEFINE.
CHANNEL_DEFINE(i32);
VECTOR_DEFINE(i32);
HASHMAP_DEFINE(i32, i32);
SLICE_DEFINE(i32);
RESULT_DEFINE(i32, i32);
UNIQUE_PTR_DEFINE(i32);
SHARED_PTR_DEFINE(i32);

i32 main(void) {
    printf("=== Method-Style Macro API Examples ===\n\n");

    /*========================================================================
     * Section 1: Vector - Two API Styles
     *========================================================================*/
    printf("1. Vector - Two API Styles:\n");

    Vec_i32 v = vec_i32_new();

    // Style 1: Standalone functions
    printf("   Style 1 - Standalone functions:\n");
    vec_i32_push(&v, 10);
    vec_i32_push(&v, 20);
    printf("      vec_i32_push(&v, 10), vec_i32_push(&v, 20)\n");
    printf("      Length: %zu\n", vec_i32_len(&v));

    // Style 2: Type-first convenience macros
    printf("   Style 2 - Convenience macros:\n");
    VEC_PUSH(i32, v, 30);
    VEC_PUSH(i32, v, 40);
    printf("      VEC_PUSH(i32, v, 30), VEC_PUSH(i32, v, 40)\n");
    printf("      Length: %zu\n", VEC_LEN(i32, v));

    // Show all elements
    printf("   All elements: ");
    for (usize i = 0; i < VEC_LEN(i32, v); i++) {
        Option_i32 elem = VEC_GET(i32, v, i);
        if (is_some(elem)) {
            printf("%d ", unwrap(elem));
        }
    }
    printf("\n");

    // Pop using macros
    printf("   Popping via macros: ");
    Option_i32 popped;
    while (is_some(popped = VEC_POP(i32, v))) {
        printf("%d ", unwrap(popped));
    }
    printf("\n");

    VEC_FREE(i32, v);

    /*========================================================================
     * Section 2: HashMap - Two API Styles
     *========================================================================*/
    printf("\n2. HashMap - Two API Styles:\n");

    HashMap_i32_i32 m = hashmap_i32_i32_new();

    // Style 1: Standalone functions
    printf("   Style 1 - Standalone functions:\n");
    hashmap_i32_i32_insert(&m, 1, 100);
    hashmap_i32_i32_insert(&m, 2, 200);
    printf("      hashmap_i32_i32_insert(&m, 1, 100)\n");
    printf("      hashmap_i32_i32_insert(&m, 2, 200)\n");
    Option_i32 val = hashmap_i32_i32_get(&m, 2);
    printf("      hashmap_i32_i32_get(&m, 2) = %d\n", is_some(val) ? unwrap(val) : -1);

    // Style 2: Type-first convenience macros
    printf("   Style 2 - Convenience macros:\n");
    MAP_INSERT(i32, i32, m, 3, 300);
    MAP_INSERT(i32, i32, m, 4, 400);
    printf("      MAP_INSERT(i32, i32, m, 3, 300), MAP_INSERT(i32, i32, m, 4, 400)\n");
    printf("      MAP_LEN(i32, i32, m) = %zu\n", MAP_LEN(i32, i32, m));
    printf("      MAP_CONTAINS(i32, i32, m, 3) = %s\n", MAP_CONTAINS(i32, i32, m, 3) ? "true" : "false");
    printf("      MAP_CONTAINS(i32, i32, m, 99) = %s\n", MAP_CONTAINS(i32, i32, m, 99) ? "true" : "false");

    // Remove using the macro
    Option_i32 removed = MAP_REMOVE(i32, i32, m, 1);
    printf("      MAP_REMOVE(i32, i32, m, 1) = %d\n", is_some(removed) ? unwrap(removed) : -1);
    printf("      MAP_LEN(i32, i32, m) after remove = %zu\n", MAP_LEN(i32, i32, m));

    MAP_FREE(i32, i32, m);

    /*========================================================================
     * Section 3: Slice - Two API Styles
     *========================================================================*/
    printf("\n3. Slice - Two API Styles:\n");

    i32 arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    Slice_i32 s = slice_i32_from_array(arr, 10);

    // Style 1: Standalone functions
    printf("   Style 1 - Standalone functions:\n");
    printf("      slice_i32_len(s) = %zu\n", slice_i32_len(s));
    Option_i32 elem = slice_i32_get(s, 3);
    printf("      slice_i32_get(s, 3) = %d\n", is_some(elem) ? unwrap(elem) : -1);
    Slice_i32 sub = slice_i32_subslice(s, 2, 6);
    printf("      slice_i32_subslice(s, 2, 6): ");
    for (usize i = 0; i < slice_i32_len(sub); i++) {
        Option_i32 e = slice_i32_get(sub, i);
        if (is_some(e)) printf("%d ", unwrap(e));
    }
    printf("\n");

    // Style 2: Type-first convenience macros
    printf("   Style 2 - Convenience macros:\n");
    printf("      SLICE_LEN(i32, s) = %zu\n", SLICE_LEN(i32, s));
    Slice_i32 sub2 = SLICE_SUBSLICE(i32, s, 5, 10);
    printf("      SLICE_SUBSLICE(i32, s, 5, 10): ");
    for (usize i = 0; i < SLICE_LEN(i32, sub2); i++) {
        Option_i32 e = SLICE_GET(i32, sub2, i);
        if (is_some(e)) printf("%d ", unwrap(e));
    }
    printf("\n");

    /*========================================================================
     * Section 4: String - Two API Styles
     *========================================================================*/
    printf("\n4. String - Two API Styles:\n");

    String str = string_from("Hello");

    // Style 1: Standalone functions
    printf("   Style 1 - Standalone functions:\n");
    string_append(&str, " World");
    printf("      string_append(&str, \" World\")\n");
    printf("      string_cstr(&str) = \"%s\"\n", string_cstr(&str));
    printf("      string_len(&str) = %zu\n", string_len(&str));

    // Style 2: Convenience macros (String is monomorphic - no type argument)
    printf("   Style 2 - Convenience macros:\n");
    STR_APPEND(str, "!!");
    printf("      STR_APPEND(str, \"!!\")\n");
    printf("      STR_CSTR(str) = \"%s\"\n", STR_CSTR(str));
    printf("      STR_LEN(str) = %zu\n", STR_LEN(str));

    // Get character at index
    Option_char ch = STR_GET(str, 0);
    printf("      STR_GET(str, 0) = '%c'\n", is_some(ch) ? unwrap(ch) : '?');

    // Search within the string (new in 0.2.0)
    printf("      STR_CONTAINS(str, \"World\") = %s\n",
           STR_CONTAINS(str, "World") ? "true" : "false");

    // Slice the string
    Slice_char slice = STR_SLICE(str, 0, 5);
    printf("      STR_SLICE(str, 0, 5): \"");
    for (usize i = 0; i < slice_char_len(slice); i++) {
        Option_char c = slice_char_get(slice, i);
        if (is_some(c)) printf("%c", unwrap(c));
    }
    printf("\"\n");

    STR_FREE(str);

    /*========================================================================
     * Section 5: Option - Two API Styles
     *========================================================================*/
    printf("\n5. Option - Two API Styles:\n");

    Option_i32 some_val = Some(i32, 42);
    Option_i32 none_val = None(i32);

    // Style 1: Standalone functions
    printf("   Style 1 - Standalone functions:\n");
    printf("      option_i32_is_some(&some_val) = %s\n", option_i32_is_some(&some_val) ? "true" : "false");
    printf("      option_i32_is_none(&none_val) = %s\n", option_i32_is_none(&none_val) ? "true" : "false");
    printf("      option_i32_unwrap(&some_val) = %d\n", option_i32_unwrap(&some_val));
    printf("      option_i32_unwrap_or(&none_val, 99) = %d\n", option_i32_unwrap_or(&none_val, 99));

    // Style 2: Convenience macros (typeless member access - no type argument)
    printf("   Style 2 - Convenience macros:\n");
    printf("      OPT_IS_SOME(some_val) = %s\n", OPT_IS_SOME(some_val) ? "true" : "false");
    printf("      OPT_IS_NONE(none_val) = %s\n", OPT_IS_NONE(none_val) ? "true" : "false");
    printf("      OPT_UNWRAP(some_val) = %d\n", OPT_UNWRAP(some_val));
    printf("      OPT_UNWRAP_OR(none_val, 99) = %d\n", OPT_UNWRAP_OR(none_val, 99));

    /*========================================================================
     * Section 6: Result - Two API Styles
     *========================================================================*/
    printf("\n6. Result - Two API Styles:\n");

    Result_i32_i32 ok_res = Ok(i32, i32, 100);
    Result_i32_i32 err_res = Err(i32, i32, -1);

    // Style 1: Standalone functions
    printf("   Style 1 - Standalone functions:\n");
    printf("      result_i32_i32_is_ok(&ok_res) = %s\n", result_i32_i32_is_ok(&ok_res) ? "true" : "false");
    printf("      result_i32_i32_is_err(&err_res) = %s\n", result_i32_i32_is_err(&err_res) ? "true" : "false");
    printf("      result_i32_i32_unwrap_ok(&ok_res) = %d\n", result_i32_i32_unwrap_ok(&ok_res));
    printf("      result_i32_i32_unwrap_err(&err_res) = %d\n", result_i32_i32_unwrap_err(&err_res));

    // Style 2: Convenience macros (typeless member access - no type argument)
    printf("   Style 2 - Convenience macros:\n");
    printf("      RES_IS_OK(ok_res) = %s\n", RES_IS_OK(ok_res) ? "true" : "false");
    printf("      RES_IS_ERR(err_res) = %s\n", RES_IS_ERR(err_res) ? "true" : "false");
    printf("      RES_UNWRAP_OK(ok_res) = %d\n", RES_UNWRAP_OK(ok_res));
    printf("      RES_UNWRAP_ERR(err_res) = %d\n", RES_UNWRAP_ERR(err_res));
    printf("      RES_UNWRAP_OK_OR(err_res, 0) = %d\n", RES_UNWRAP_OK_OR(err_res, 0));

    /*========================================================================
     * Section 7: UniquePtr - Two API Styles
     *========================================================================*/
    printf("\n7. UniquePtr - Two API Styles:\n");

    UniquePtr_i32 uptr = unique_i32_new(42);

    // Style 1: Standalone functions
    printf("   Style 1 - Standalone functions:\n");
    printf("      unique_i32_deref(&uptr) = %d\n", unique_i32_deref(&uptr));
    printf("      unique_i32_get(&uptr) = %p\n", (void*)unique_i32_get(&uptr));

    // Style 2: Type-first convenience macros
    printf("   Style 2 - Convenience macros:\n");
    printf("      UPTR_DEREF(i32, uptr) = %d\n", UPTR_DEREF(i32, uptr));
    printf("      UPTR_GET(i32, uptr) = %p\n", (void*)UPTR_GET(i32, uptr));

    // Move ownership
    UniquePtr_i32 uptr2 = UPTR_MOVE(i32, uptr);
    printf("      After UPTR_MOVE: original ptr = %p, new ptr = %p\n",
           (void*)UPTR_GET(i32, uptr), (void*)UPTR_GET(i32, uptr2));

    UPTR_FREE(i32, uptr2);

    /*========================================================================
     * Section 8: SharedPtr - Two API Styles
     *========================================================================*/
    printf("\n8. SharedPtr - Two API Styles:\n");

    SharedPtr_i32 sptr = shared_i32_new(100);

    // Style 1: Standalone functions
    printf("   Style 1 - Standalone functions:\n");
    printf("      shared_i32_deref(&sptr) = %d\n", shared_i32_deref(&sptr));
    printf("      shared_i32_count(&sptr) = %zu\n", shared_i32_count(&sptr));

    // Style 2: Type-first convenience macros
    SharedPtr_i32 sptr2 = SPTR_CLONE(i32, sptr);
    printf("   Style 2 - After SPTR_CLONE(i32, sptr):\n");
    printf("      SPTR_COUNT(i32, sptr) = %zu\n", SPTR_COUNT(i32, sptr));
    printf("      SPTR_COUNT(i32, sptr2) = %zu\n", SPTR_COUNT(i32, sptr2));
    printf("      SPTR_DEREF(i32, sptr2) = %d\n", SPTR_DEREF(i32, sptr2));

    // Release one reference
    SPTR_RELEASE(i32, sptr2);
    printf("   After SPTR_RELEASE(i32, sptr2):\n");
    printf("      SPTR_COUNT(i32, sptr) = %zu\n", SPTR_COUNT(i32, sptr));

    SPTR_RELEASE(i32, sptr);

    /*========================================================================
     * Section 9: Channel - Two API Styles
     *========================================================================*/
    printf("\n9. Channel - Two API Styles:\n");

    Channel_i32 *chan = chan_i32_new(5);  // Buffered channel with capacity 5

    // Style 1: Standalone functions
    printf("   Style 1 - Standalone functions:\n");
    chan_i32_send(chan, 10);
    chan_i32_send(chan, 20);
    printf("      chan_i32_send(chan, 10), chan_i32_send(chan, 20)\n");

    // Style 2: Type-first convenience macros
    printf("   Style 2 - Convenience macros:\n");
    CHAN_SEND(i32, chan, 30);
    printf("      CHAN_SEND(i32, chan, 30)\n");
    printf("      CHAN_IS_CLOSED(i32, chan) = %s\n", CHAN_IS_CLOSED(i32, chan) ? "true" : "false");

    // Receive values
    printf("   Receiving values:\n");
    Option_i32 recv1 = CHAN_RECV(i32, chan);
    Option_i32 recv2 = CHAN_RECV(i32, chan);
    Option_i32 recv3 = CHAN_RECV(i32, chan);
    printf("      CHAN_RECV: %d, %d, %d\n",
           is_some(recv1) ? unwrap(recv1) : -1,
           is_some(recv2) ? unwrap(recv2) : -1,
           is_some(recv3) ? unwrap(recv3) : -1);

    // Try non-blocking operations
    printf("   Non-blocking operations:\n");
    ChanStatus status = CHAN_TRY_SEND(i32, chan, 40);
    printf("      CHAN_TRY_SEND(i32, chan, 40) = %s\n",
           status == CHAN_OK ? "OK" : (status == CHAN_CLOSED ? "CLOSED" : "WOULD_BLOCK"));

    Option_i32 try_recv = CHAN_TRY_RECV(i32, chan);
    printf("      CHAN_TRY_RECV(i32, chan) = %s\n",
           is_some(try_recv) ? "Some" : "None");
    if (is_some(try_recv)) {
        printf("         value = %d\n", unwrap(try_recv));
    }

    // Close and free
    CHAN_CLOSE(i32, chan);
    printf("   After CHAN_CLOSE:\n");
    printf("      CHAN_IS_CLOSED(i32, chan) = %s\n", CHAN_IS_CLOSED(i32, chan) ? "true" : "false");

    CHAN_FREE(i32, chan);

    /*========================================================================
     * Section 10: Plain Structs - No Hidden Pointers
     *========================================================================*/
    printf("\n10. Plain Structs - No Hidden Pointers:\n");

    printf("    Cyan types are plain structs; the macros expand directly to\n");
    printf("    function calls, so a value carries no vtable pointer:\n");
    printf("      sizeof(Option_i32)     = %zu\n", sizeof(Option_i32));
    printf("      sizeof(Result_i32_i32) = %zu\n", sizeof(Result_i32_i32));
    printf("      sizeof(Vec_i32)        = %zu\n", sizeof(Vec_i32));
    printf("      sizeof(Slice_i32)      = %zu\n", sizeof(Slice_i32));
    printf("      sizeof(String)         = %zu\n", sizeof(String));
    printf("      sizeof(UniquePtr_i32)  = %zu\n", sizeof(UniquePtr_i32));
    printf("      sizeof(SharedPtr_i32)  = %zu\n", sizeof(SharedPtr_i32));

    printf("\n=== Done ===\n");
    return 0;
}

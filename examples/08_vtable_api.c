/**
 * @file 08_vtable_api.c
 * @brief Example demonstrating the vtable method-style API
 * 
 * This example shows how to use the vtable pattern in Cyan for a more
 * object-oriented, method-style API. Each collection type has a vtable
 * pointer (vt) that provides function pointers for operations.
 * 
 * Three API styles are demonstrated:
 * 1. Standalone functions: vec_i32_push(&v, 42)
 * 2. Direct vtable access: v.vt->push(&v, 42)
 * 3. Convenience macros: VEC_PUSH(v, 42)
 * 
 * Compile: gcc -std=c11 -I../include -o vtable_demo 08_vtable_api.c
 * Run: ./vtable_demo
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
    printf("=== Vtable Method-Style API Examples ===\n\n");
    
    /*========================================================================
     * Section 1: Vector - Three API Styles
     *========================================================================*/
    printf("1. Vector - Three API Styles:\n");
    
    Vec_i32 v = vec_i32_new();
    
    // Style 1: Standalone functions
    printf("   Style 1 - Standalone functions:\n");
    vec_i32_push(&v, 10);
    vec_i32_push(&v, 20);
    printf("      vec_i32_push(&v, 10), vec_i32_push(&v, 20)\n");
    printf("      Length: %zu\n", vec_i32_len(&v));
    
    // Style 2: Direct vtable access
    printf("   Style 2 - Direct vtable access:\n");
    v.vt->push(&v, 30);
    v.vt->push(&v, 40);
    printf("      v.vt->push(&v, 30), v.vt->push(&v, 40)\n");
    printf("      Length: %zu\n", v.vt->len(&v));
    
    // Style 3: Convenience macros
    printf("   Style 3 - Convenience macros:\n");
    VEC_PUSH(v, 50);
    printf("      VEC_PUSH(v, 50)\n");
    printf("      Length: %zu\n", VEC_LEN(v));
    
    // Show all elements
    printf("   All elements: ");
    for (usize i = 0; i < VEC_LEN(v); i++) {
        Option_i32 elem = VEC_GET(v, i);
        if (is_some(elem)) {
            printf("%d ", unwrap(elem));
        }
    }
    printf("\n");
    
    // Pop using vtable
    printf("   Popping via vtable: ");
    Option_i32 popped;
    while (is_some(popped = VEC_POP(v))) {
        printf("%d ", unwrap(popped));
    }
    printf("\n");
    
    VEC_FREE(v);
    
    /*========================================================================
     * Section 2: HashMap - Three API Styles
     *========================================================================*/
    printf("\n2. HashMap - Three API Styles:\n");
    
    HashMap_i32_i32 m = hashmap_i32_i32_new();
    
    // Style 1: Standalone functions
    printf("   Style 1 - Standalone functions:\n");
    hashmap_i32_i32_insert(&m, 1, 100);
    hashmap_i32_i32_insert(&m, 2, 200);
    printf("      hashmap_i32_i32_insert(&m, 1, 100)\n");
    printf("      hashmap_i32_i32_insert(&m, 2, 200)\n");
    
    // Style 2: Direct vtable access
    printf("   Style 2 - Direct vtable access:\n");
    m.vt->insert(&m, 3, 300);
    printf("      m.vt->insert(&m, 3, 300)\n");
    Option_i32 val = m.vt->get(&m, 2);
    printf("      m.vt->get(&m, 2) = %d\n", is_some(val) ? unwrap(val) : -1);
    
    // Style 3: Convenience macros
    printf("   Style 3 - Convenience macros:\n");
    MAP_INSERT(m, 4, 400);
    printf("      MAP_INSERT(m, 4, 400)\n");
    printf("      MAP_LEN(m) = %zu\n", MAP_LEN(m));
    printf("      MAP_CONTAINS(m, 3) = %s\n", MAP_CONTAINS(m, 3) ? "true" : "false");
    printf("      MAP_CONTAINS(m, 99) = %s\n", MAP_CONTAINS(m, 99) ? "true" : "false");
    
    // Remove using vtable
    Option_i32 removed = MAP_REMOVE(m, 1);
    printf("      MAP_REMOVE(m, 1) = %d\n", is_some(removed) ? unwrap(removed) : -1);
    printf("      MAP_LEN(m) after remove = %zu\n", MAP_LEN(m));
    
    MAP_FREE(m);
    
    /*========================================================================
     * Section 3: Slice - Three API Styles
     *========================================================================*/
    printf("\n3. Slice - Three API Styles:\n");
    
    i32 arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    Slice_i32 s = slice_i32_from_array(arr, 10);
    
    // Style 1: Standalone functions
    printf("   Style 1 - Standalone functions:\n");
    printf("      slice_i32_len(s) = %zu\n", slice_i32_len(s));
    Option_i32 elem = slice_i32_get(s, 3);
    printf("      slice_i32_get(s, 3) = %d\n", is_some(elem) ? unwrap(elem) : -1);
    
    // Style 2: Direct vtable access
    printf("   Style 2 - Direct vtable access:\n");
    printf("      s.vt->len(s) = %zu\n", s.vt->len(s));
    Slice_i32 sub = s.vt->subslice(s, 2, 6);
    printf("      s.vt->subslice(s, 2, 6): ");
    for (usize i = 0; i < sub.vt->len(sub); i++) {
        Option_i32 e = sub.vt->get(sub, i);
        if (is_some(e)) printf("%d ", unwrap(e));
    }
    printf("\n");
    
    // Style 3: Convenience macros
    printf("   Style 3 - Convenience macros:\n");
    printf("      SLICE_LEN(s) = %zu\n", SLICE_LEN(s));
    Slice_i32 sub2 = SLICE_SUBSLICE(s, 5, 10);
    printf("      SLICE_SUBSLICE(s, 5, 10): ");
    for (usize i = 0; i < SLICE_LEN(sub2); i++) {
        Option_i32 e = SLICE_GET(sub2, i);
        if (is_some(e)) printf("%d ", unwrap(e));
    }
    printf("\n");
    
    /*========================================================================
     * Section 4: String - Three API Styles
     *========================================================================*/
    printf("\n4. String - Three API Styles:\n");
    
    String str = string_from("Hello");
    
    // Style 1: Standalone functions
    printf("   Style 1 - Standalone functions:\n");
    string_append(&str, " World");
    printf("      string_append(&str, \" World\")\n");
    printf("      string_cstr(&str) = \"%s\"\n", string_cstr(&str));
    printf("      string_len(&str) = %zu\n", string_len(&str));
    
    // Style 2: Direct vtable access
    printf("   Style 2 - Direct vtable access:\n");
    str.vt->push(&str, '!');
    printf("      str.vt->push(&str, '!')\n");
    printf("      str.vt->cstr(&str) = \"%s\"\n", str.vt->cstr(&str));
    printf("      str.vt->len(&str) = %zu\n", str.vt->len(&str));
    
    // Style 3: Convenience macros
    printf("   Style 3 - Convenience macros:\n");
    STR_APPEND(str, "!!");
    printf("      STR_APPEND(str, \"!!\")\n");
    printf("      STR_CSTR(str) = \"%s\"\n", STR_CSTR(str));
    printf("      STR_LEN(str) = %zu\n", STR_LEN(str));
    
    // Get character at index
    Option_char ch = STR_GET(str, 0);
    printf("      STR_GET(str, 0) = '%c'\n", is_some(ch) ? unwrap(ch) : '?');
    
    // Slice the string
    Slice_char slice = STR_SLICE(str, 0, 5);
    printf("      STR_SLICE(str, 0, 5): \"");
    for (usize i = 0; i < SLICE_LEN(slice); i++) {
        Option_char c = SLICE_GET(slice, i);
        if (is_some(c)) printf("%c", unwrap(c));
    }
    printf("\"\n");
    
    STR_FREE(str);
    
    /*========================================================================
     * Section 5: Option - Vtable API
     *========================================================================*/
    printf("\n5. Option - Vtable API:\n");
    
    Option_i32 some_val = Some(i32, 42);
    Option_i32 none_val = None(i32);
    
    // Traditional macros
    printf("   Traditional macros:\n");
    printf("      is_some(some_val) = %s\n", is_some(some_val) ? "true" : "false");
    printf("      is_none(none_val) = %s\n", is_none(none_val) ? "true" : "false");
    printf("      unwrap(some_val) = %d\n", unwrap(some_val));
    printf("      unwrap_or(none_val, 99) = %d\n", unwrap_or(none_val, 99));
    
    // Vtable convenience macros
    printf("   Vtable convenience macros:\n");
    printf("      OPT_IS_SOME(some_val) = %s\n", OPT_IS_SOME(some_val) ? "true" : "false");
    printf("      OPT_IS_NONE(none_val) = %s\n", OPT_IS_NONE(none_val) ? "true" : "false");
    printf("      OPT_UNWRAP(some_val) = %d\n", OPT_UNWRAP(some_val));
    printf("      OPT_UNWRAP_OR(none_val, 99) = %d\n", OPT_UNWRAP_OR(none_val, 99));
    
    /*========================================================================
     * Section 6: Result - Vtable API
     *========================================================================*/
    printf("\n6. Result - Vtable API:\n");
    
    Result_i32_i32 ok_res = Ok(i32, i32, 100);
    Result_i32_i32 err_res = Err(i32, i32, -1);
    
    // Traditional macros
    printf("   Traditional macros:\n");
    printf("      is_ok(ok_res) = %s\n", is_ok(ok_res) ? "true" : "false");
    printf("      is_err(err_res) = %s\n", is_err(err_res) ? "true" : "false");
    printf("      unwrap_ok(ok_res) = %d\n", unwrap_ok(ok_res));
    printf("      unwrap_err(err_res) = %d\n", unwrap_err(err_res));
    
    // Vtable convenience macros
    printf("   Vtable convenience macros:\n");
    printf("      RES_IS_OK(ok_res) = %s\n", RES_IS_OK(ok_res) ? "true" : "false");
    printf("      RES_IS_ERR(err_res) = %s\n", RES_IS_ERR(err_res) ? "true" : "false");
    printf("      RES_UNWRAP_OK(ok_res) = %d\n", RES_UNWRAP_OK(ok_res));
    printf("      RES_UNWRAP_ERR(err_res) = %d\n", RES_UNWRAP_ERR(err_res));
    printf("      RES_UNWRAP_OK_OR(err_res, 0) = %d\n", RES_UNWRAP_OK_OR(err_res, 0));
    
    /*========================================================================
     * Section 7: UniquePtr - Vtable API
     *========================================================================*/
    printf("\n7. UniquePtr - Vtable API:\n");
    
    UniquePtr_i32 uptr = unique_i32_new(42);
    
    // Standalone functions
    printf("   Standalone functions:\n");
    printf("      unique_i32_deref(&uptr) = %d\n", unique_i32_deref(&uptr));
    printf("      unique_i32_get(&uptr) = %p\n", (void*)unique_i32_get(&uptr));
    
    // Vtable convenience macros
    printf("   Vtable convenience macros:\n");
    printf("      UPTR_DEREF(uptr) = %d\n", UPTR_DEREF(uptr));
    printf("      UPTR_GET(uptr) = %p\n", (void*)UPTR_GET(uptr));
    
    // Move ownership
    UniquePtr_i32 uptr2 = UPTR_MOVE(uptr);
    printf("      After UPTR_MOVE: original ptr = %p, new ptr = %p\n",
           (void*)UPTR_GET(uptr), (void*)UPTR_GET(uptr2));
    
    UPTR_FREE(uptr2);
    
    /*========================================================================
     * Section 8: SharedPtr - Vtable API
     *========================================================================*/
    printf("\n8. SharedPtr - Vtable API:\n");
    
    SharedPtr_i32 sptr = shared_i32_new(100);
    
    // Standalone functions
    printf("   Standalone functions:\n");
    printf("      shared_i32_deref(&sptr) = %d\n", shared_i32_deref(&sptr));
    printf("      shared_i32_count(&sptr) = %zu\n", shared_i32_count(&sptr));
    
    // Clone using vtable
    SharedPtr_i32 sptr2 = SPTR_CLONE(sptr);
    printf("   After SPTR_CLONE:\n");
    printf("      SPTR_COUNT(sptr) = %zu\n", SPTR_COUNT(sptr));
    printf("      SPTR_COUNT(sptr2) = %zu\n", SPTR_COUNT(sptr2));
    printf("      SPTR_DEREF(sptr2) = %d\n", SPTR_DEREF(sptr2));
    
    // Release one reference
    SPTR_RELEASE(sptr2);
    printf("   After SPTR_RELEASE(sptr2):\n");
    printf("      SPTR_COUNT(sptr) = %zu\n", SPTR_COUNT(sptr));
    
    SPTR_RELEASE(sptr);
    
    /*========================================================================
     * Section 9: Channel - Vtable API
     *========================================================================*/
    printf("\n9. Channel - Vtable API:\n");
    
    Channel_i32 *chan = chan_i32_new(5);  // Buffered channel with capacity 5
    
    // Standalone functions
    printf("   Standalone functions:\n");
    chan_i32_send(chan, 10);
    chan_i32_send(chan, 20);
    printf("      chan_i32_send(chan, 10), chan_i32_send(chan, 20)\n");
    
    // Vtable convenience macros
    printf("   Vtable convenience macros:\n");
    CHAN_SEND(chan, 30);
    printf("      CHAN_SEND(chan, 30)\n");
    printf("      CHAN_IS_CLOSED(chan) = %s\n", CHAN_IS_CLOSED(chan) ? "true" : "false");
    
    // Receive values
    printf("   Receiving values:\n");
    Option_i32 recv1 = CHAN_RECV(chan);
    Option_i32 recv2 = CHAN_RECV(chan);
    Option_i32 recv3 = CHAN_RECV(chan);
    printf("      CHAN_RECV: %d, %d, %d\n",
           is_some(recv1) ? unwrap(recv1) : -1,
           is_some(recv2) ? unwrap(recv2) : -1,
           is_some(recv3) ? unwrap(recv3) : -1);
    
    // Try non-blocking operations
    printf("   Non-blocking operations:\n");
    ChanStatus status = CHAN_TRY_SEND(chan, 40);
    printf("      CHAN_TRY_SEND(chan, 40) = %s\n",
           status == CHAN_OK ? "OK" : (status == CHAN_CLOSED ? "CLOSED" : "WOULD_BLOCK"));
    
    Option_i32 try_recv = CHAN_TRY_RECV(chan);
    printf("      CHAN_TRY_RECV(chan) = %s\n",
           is_some(try_recv) ? "Some" : "None");
    if (is_some(try_recv)) {
        printf("         value = %d\n", unwrap(try_recv));
    }
    
    // Close and free
    CHAN_CLOSE(chan);
    printf("   After CHAN_CLOSE:\n");
    printf("      CHAN_IS_CLOSED(chan) = %s\n", CHAN_IS_CLOSED(chan) ? "true" : "false");
    
    CHAN_FREE(chan);
    
    /*========================================================================
     * Section 10: Memory Efficiency - Shared Vtables
     *========================================================================*/
    printf("\n10. Memory Efficiency - Shared Vtables:\n");
    
    Vec_i32 v1 = vec_i32_new();
    Vec_i32 v2 = vec_i32_new();
    Vec_i32 v3 = vec_i32_new();
    
    printf("    All Vec_i32 instances share the same vtable:\n");
    printf("      v1.vt = %p\n", (void*)v1.vt);
    printf("      v2.vt = %p\n", (void*)v2.vt);
    printf("      v3.vt = %p\n", (void*)v3.vt);
    printf("      v1.vt == v2.vt == v3.vt: %s\n",
           (v1.vt == v2.vt && v2.vt == v3.vt) ? "true" : "false");
    
    VEC_FREE(v1);
    VEC_FREE(v2);
    VEC_FREE(v3);
    
    printf("\n=== Done ===\n");
    return 0;
}

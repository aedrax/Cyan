/**
 * @file test_vtable_macros.c
 * @brief Property-based tests for convenience macro equivalence
 * 
 * Tests validate correctness property:
 * - Property 6: Convenience macro equivalence
 *   For any collection instance and valid operation arguments, calling a
 *   convenience macro (VEC_PUSH, MAP_GET, etc.) shall produce identical
 *   results to calling the vtable function pointer directly.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theft.h"
#include <cyan/option.h>
#include <cyan/result.h>
#include <cyan/smartptr.h>
#include <cyan/channel.h>
#include <cyan/vector.h>
#include <cyan/hashmap.h>
#include <cyan/slice.h>
#include <cyan/string.h>

/* Define Option type first (required by Vector, HashMap) */
OPTION_DEFINE(int);

/* Define collection types for testing */
VECTOR_DEFINE(int);
HASHMAP_DEFINE(int, int);
SLICE_DEFINE(int);

/* Define Result, SmartPtr types for extended testing */
RESULT_DEFINE(int, int);
UNIQUE_PTR_DEFINE(int);
SHARED_PTR_DEFINE(int);

/* Channel needs its own Option type - use long to avoid conflict with Option_int */
CHANNEL_DEFINE(long);

/*============================================================================
 * Property 6: Convenience macro equivalence (Vector)
 * For any Vec_T instance and valid operations, calling convenience macros
 * (VEC_PUSH, VEC_POP, VEC_GET, VEC_LEN, VEC_FREE) shall produce identical
 * results to calling the vtable function pointers directly.
 *============================================================================*/

static enum theft_trial_res prop_vec_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    /* Create two identical vectors - one for macro ops, one for direct vtable ops */
    Vec_int v_macro = vec_int_new();
    Vec_int v_vtable = vec_int_new();
    
    /* Test VEC_PUSH vs v.vt->push */
    VEC_PUSH(v_macro, val);
    v_vtable.vt->push(&v_vtable, val);
    
    VEC_PUSH(v_macro, val + 1);
    v_vtable.vt->push(&v_vtable, val + 1);
    
    VEC_PUSH(v_macro, val + 2);
    v_vtable.vt->push(&v_vtable, val + 2);
    
    /* Test VEC_LEN vs v.vt->len */
    size_t len_macro = VEC_LEN(v_macro);
    size_t len_vtable = v_vtable.vt->len(&v_vtable);
    
    if (len_macro != len_vtable) {
        VEC_FREE(v_macro);
        v_vtable.vt->free(&v_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test VEC_GET vs v.vt->get for all indices */
    for (size_t i = 0; i < len_macro; i++) {
        Option_int opt_macro = VEC_GET(v_macro, i);
        Option_int opt_vtable = v_vtable.vt->get(&v_vtable, i);
        
        if (is_some(opt_macro) != is_some(opt_vtable)) {
            VEC_FREE(v_macro);
            v_vtable.vt->free(&v_vtable);
            return THEFT_TRIAL_FAIL;
        }
        
        if (is_some(opt_macro) && unwrap(opt_macro) != unwrap(opt_vtable)) {
            VEC_FREE(v_macro);
            v_vtable.vt->free(&v_vtable);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test VEC_GET out-of-bounds vs v.vt->get out-of-bounds */
    Option_int oob_macro = VEC_GET(v_macro, len_macro + 10);
    Option_int oob_vtable = v_vtable.vt->get(&v_vtable, len_vtable + 10);
    
    if (is_none(oob_macro) != is_none(oob_vtable)) {
        VEC_FREE(v_macro);
        v_vtable.vt->free(&v_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test VEC_POP vs v.vt->pop */
    Option_int pop_macro = VEC_POP(v_macro);
    Option_int pop_vtable = v_vtable.vt->pop(&v_vtable);
    
    if (is_some(pop_macro) != is_some(pop_vtable)) {
        VEC_FREE(v_macro);
        v_vtable.vt->free(&v_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    if (is_some(pop_macro) && unwrap(pop_macro) != unwrap(pop_vtable)) {
        VEC_FREE(v_macro);
        v_vtable.vt->free(&v_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify lengths are still equal after pop */
    if (VEC_LEN(v_macro) != v_vtable.vt->len(&v_vtable)) {
        VEC_FREE(v_macro);
        v_vtable.vt->free(&v_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test VEC_POP on empty vector */
    Vec_int empty_macro = vec_int_new();
    Vec_int empty_vtable = vec_int_new();
    
    Option_int empty_pop_macro = VEC_POP(empty_macro);
    Option_int empty_pop_vtable = empty_vtable.vt->pop(&empty_vtable);
    
    if (is_none(empty_pop_macro) != is_none(empty_pop_vtable)) {
        VEC_FREE(v_macro);
        v_vtable.vt->free(&v_vtable);
        VEC_FREE(empty_macro);
        empty_vtable.vt->free(&empty_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Cleanup using VEC_FREE for macro, direct vtable for other */
    VEC_FREE(v_macro);
    v_vtable.vt->free(&v_vtable);
    VEC_FREE(empty_macro);
    empty_vtable.vt->free(&empty_vtable);
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6: Convenience macro equivalence (HashMap)
 * For any HashMap_K_V instance and valid operations, calling convenience macros
 * (MAP_INSERT, MAP_GET, MAP_CONTAINS, MAP_REMOVE, MAP_LEN, MAP_FREE) shall
 * produce identical results to calling the vtable function pointers directly.
 *============================================================================*/

static enum theft_trial_res prop_map_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int seed = (int)(*val_ptr);
    
    /* Create two identical hashmaps - one for macro ops, one for direct vtable ops */
    HashMap_int_int m_macro = hashmap_int_int_new();
    HashMap_int_int m_vtable = hashmap_int_int_new();
    
    /* Test MAP_INSERT vs m.vt->insert */
    int keys[5];
    int values[5];
    for (int i = 0; i < 5; i++) {
        keys[i] = seed + i * 7;
        values[i] = seed * 3 + i;
        
        MAP_INSERT(m_macro, keys[i], values[i]);
        m_vtable.vt->insert(&m_vtable, keys[i], values[i]);
    }
    
    /* Test MAP_LEN vs m.vt->len */
    size_t len_macro = MAP_LEN(m_macro);
    size_t len_vtable = m_vtable.vt->len(&m_vtable);
    
    if (len_macro != len_vtable) {
        MAP_FREE(m_macro);
        m_vtable.vt->free(&m_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test MAP_GET vs m.vt->get for all inserted keys */
    for (int i = 0; i < 5; i++) {
        Option_int opt_macro = MAP_GET(m_macro, keys[i]);
        Option_int opt_vtable = m_vtable.vt->get(&m_vtable, keys[i]);
        
        if (is_some(opt_macro) != is_some(opt_vtable)) {
            MAP_FREE(m_macro);
            m_vtable.vt->free(&m_vtable);
            return THEFT_TRIAL_FAIL;
        }
        
        if (is_some(opt_macro) && unwrap(opt_macro) != unwrap(opt_vtable)) {
            MAP_FREE(m_macro);
            m_vtable.vt->free(&m_vtable);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test MAP_GET for missing key vs m.vt->get for missing key */
    int missing_key = seed + 1000;
    Option_int missing_macro = MAP_GET(m_macro, missing_key);
    Option_int missing_vtable = m_vtable.vt->get(&m_vtable, missing_key);
    
    if (is_none(missing_macro) != is_none(missing_vtable)) {
        MAP_FREE(m_macro);
        m_vtable.vt->free(&m_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test MAP_CONTAINS vs m.vt->contains */
    bool contains_macro = MAP_CONTAINS(m_macro, keys[0]);
    bool contains_vtable = m_vtable.vt->contains(&m_vtable, keys[0]);
    
    if (contains_macro != contains_vtable) {
        MAP_FREE(m_macro);
        m_vtable.vt->free(&m_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test MAP_CONTAINS for missing key */
    bool missing_contains_macro = MAP_CONTAINS(m_macro, missing_key);
    bool missing_contains_vtable = m_vtable.vt->contains(&m_vtable, missing_key);
    
    if (missing_contains_macro != missing_contains_vtable) {
        MAP_FREE(m_macro);
        m_vtable.vt->free(&m_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test MAP_REMOVE vs m.vt->remove */
    Option_int remove_macro = MAP_REMOVE(m_macro, keys[0]);
    Option_int remove_vtable = m_vtable.vt->remove(&m_vtable, keys[0]);
    
    if (is_some(remove_macro) != is_some(remove_vtable)) {
        MAP_FREE(m_macro);
        m_vtable.vt->free(&m_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    if (is_some(remove_macro) && unwrap(remove_macro) != unwrap(remove_vtable)) {
        MAP_FREE(m_macro);
        m_vtable.vt->free(&m_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify lengths are still equal after remove */
    if (MAP_LEN(m_macro) != m_vtable.vt->len(&m_vtable)) {
        MAP_FREE(m_macro);
        m_vtable.vt->free(&m_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test MAP_REMOVE for missing key */
    Option_int remove_missing_macro = MAP_REMOVE(m_macro, missing_key);
    Option_int remove_missing_vtable = m_vtable.vt->remove(&m_vtable, missing_key);
    
    if (is_none(remove_missing_macro) != is_none(remove_missing_vtable)) {
        MAP_FREE(m_macro);
        m_vtable.vt->free(&m_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Cleanup using MAP_FREE for macro, direct vtable for other */
    MAP_FREE(m_macro);
    m_vtable.vt->free(&m_vtable);
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6: Convenience macro equivalence (Slice)
 * For any Slice_T instance and valid operations, calling convenience macros
 * (SLICE_GET, SLICE_SUBSLICE, SLICE_LEN) shall produce identical results
 * to calling the vtable function pointers directly.
 *============================================================================*/

static enum theft_trial_res prop_slice_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int base_val = (int)(*val_ptr);
    
    /* Create an array with known values */
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = base_val + i * 10;
    }
    
    Slice_int s = slice_int_from_array(arr, 10);
    
    /* Test SLICE_LEN vs s.vt->len */
    size_t len_macro = SLICE_LEN(s);
    size_t len_vtable = s.vt->len(s);
    
    if (len_macro != len_vtable) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test SLICE_GET vs s.vt->get for all valid indices */
    for (size_t i = 0; i < len_macro; i++) {
        Option_int opt_macro = SLICE_GET(s, i);
        Option_int opt_vtable = s.vt->get(s, i);
        
        if (is_some(opt_macro) != is_some(opt_vtable)) {
            return THEFT_TRIAL_FAIL;
        }
        
        if (is_some(opt_macro) && unwrap(opt_macro) != unwrap(opt_vtable)) {
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test SLICE_GET out-of-bounds vs s.vt->get out-of-bounds */
    Option_int oob_macro = SLICE_GET(s, len_macro + 10);
    Option_int oob_vtable = s.vt->get(s, len_vtable + 10);
    
    if (is_none(oob_macro) != is_none(oob_vtable)) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test SLICE_SUBSLICE vs s.vt->subslice */
    size_t start = 2;
    size_t end = 7;
    Slice_int sub_macro = SLICE_SUBSLICE(s, start, end);
    Slice_int sub_vtable = s.vt->subslice(s, start, end);
    
    /* Verify subslice lengths are equal */
    if (SLICE_LEN(sub_macro) != sub_vtable.vt->len(sub_vtable)) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Verify subslice elements are equal */
    for (size_t i = 0; i < SLICE_LEN(sub_macro); i++) {
        Option_int elem_macro = SLICE_GET(sub_macro, i);
        Option_int elem_vtable = sub_vtable.vt->get(sub_vtable, i);
        
        if (is_some(elem_macro) != is_some(elem_vtable)) {
            return THEFT_TRIAL_FAIL;
        }
        
        if (is_some(elem_macro) && unwrap(elem_macro) != unwrap(elem_vtable)) {
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test SLICE_SUBSLICE with clamped bounds (start > end) */
    Slice_int clamped_macro = SLICE_SUBSLICE(s, 5, 3);
    Slice_int clamped_vtable = s.vt->subslice(s, 5, 3);
    
    if (SLICE_LEN(clamped_macro) != clamped_vtable.vt->len(clamped_vtable)) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test SLICE_SUBSLICE with bounds beyond length */
    Slice_int beyond_macro = SLICE_SUBSLICE(s, 8, 15);
    Slice_int beyond_vtable = s.vt->subslice(s, 8, 15);
    
    if (SLICE_LEN(beyond_macro) != beyond_vtable.vt->len(beyond_vtable)) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6: Convenience macro equivalence (String)
 * For any String instance and valid operations, calling convenience macros
 * (STR_PUSH, STR_APPEND, STR_CLEAR, STR_GET, STR_LEN, STR_CSTR, STR_SLICE,
 * STR_FREE) shall produce identical results to calling the vtable function
 * pointers directly.
 *============================================================================*/

static enum theft_trial_res prop_str_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int seed = (int)(*val_ptr);
    
    /* Create two identical strings - one for macro ops, one for direct vtable ops */
    String s_macro = string_from("Initial");
    String s_vtable = string_from("Initial");
    
    /* Test STR_PUSH vs s.vt->push */
    char c = 'X';
    STR_PUSH(s_macro, c);
    s_vtable.vt->push(&s_vtable, c);
    
    /* Test STR_LEN vs s.vt->len */
    size_t len_macro = STR_LEN(s_macro);
    size_t len_vtable = s_vtable.vt->len(&s_vtable);
    
    if (len_macro != len_vtable) {
        STR_FREE(s_macro);
        s_vtable.vt->free(&s_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test STR_CSTR vs s.vt->cstr */
    const char *cstr_macro = STR_CSTR(s_macro);
    const char *cstr_vtable = s_vtable.vt->cstr(&s_vtable);
    
    if (strcmp(cstr_macro, cstr_vtable) != 0) {
        STR_FREE(s_macro);
        s_vtable.vt->free(&s_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test STR_APPEND vs s.vt->append */
    char append_str[32];
    snprintf(append_str, sizeof(append_str), "_%d_", seed);
    STR_APPEND(s_macro, append_str);
    s_vtable.vt->append(&s_vtable, append_str);
    
    if (STR_LEN(s_macro) != s_vtable.vt->len(&s_vtable)) {
        STR_FREE(s_macro);
        s_vtable.vt->free(&s_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    if (strcmp(STR_CSTR(s_macro), s_vtable.vt->cstr(&s_vtable)) != 0) {
        STR_FREE(s_macro);
        s_vtable.vt->free(&s_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test STR_GET vs s.vt->get for all indices */
    for (size_t i = 0; i < STR_LEN(s_macro); i++) {
        Option_char opt_macro = STR_GET(s_macro, i);
        Option_char opt_vtable = s_vtable.vt->get(&s_vtable, i);
        
        if (is_some(opt_macro) != is_some(opt_vtable)) {
            STR_FREE(s_macro);
            s_vtable.vt->free(&s_vtable);
            return THEFT_TRIAL_FAIL;
        }
        
        if (is_some(opt_macro) && unwrap(opt_macro) != unwrap(opt_vtable)) {
            STR_FREE(s_macro);
            s_vtable.vt->free(&s_vtable);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test STR_GET out-of-bounds vs s.vt->get out-of-bounds */
    Option_char oob_macro = STR_GET(s_macro, len_macro + 100);
    Option_char oob_vtable = s_vtable.vt->get(&s_vtable, len_vtable + 100);
    
    if (is_none(oob_macro) != is_none(oob_vtable)) {
        STR_FREE(s_macro);
        s_vtable.vt->free(&s_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test STR_SLICE vs s.vt->slice */
    size_t slice_start = 0;
    size_t slice_end = STR_LEN(s_macro) / 2;
    
    Slice_char slice_macro = STR_SLICE(s_macro, slice_start, slice_end);
    Slice_char slice_vtable = s_vtable.vt->slice(&s_vtable, slice_start, slice_end);
    
    if (slice_macro.len != slice_vtable.len) {
        STR_FREE(s_macro);
        s_vtable.vt->free(&s_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    for (size_t i = 0; i < slice_macro.len; i++) {
        if (slice_macro.data[i] != slice_vtable.data[i]) {
            STR_FREE(s_macro);
            s_vtable.vt->free(&s_vtable);
            return THEFT_TRIAL_FAIL;
        }
    }
    
    /* Test STR_CLEAR vs s.vt->clear */
    STR_CLEAR(s_macro);
    s_vtable.vt->clear(&s_vtable);
    
    if (STR_LEN(s_macro) != s_vtable.vt->len(&s_vtable)) {
        STR_FREE(s_macro);
        s_vtable.vt->free(&s_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    if (STR_LEN(s_macro) != 0) {
        STR_FREE(s_macro);
        s_vtable.vt->free(&s_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Cleanup using STR_FREE for macro, direct vtable for other */
    STR_FREE(s_macro);
    s_vtable.vt->free(&s_vtable);
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6 (extended): Convenience macro equivalence (Option)
 * For any Option_T instance, calling convenience macros (OPT_IS_SOME,
 * OPT_IS_NONE, OPT_UNWRAP, OPT_UNWRAP_OR) shall produce identical results
 * to calling the vtable function pointers directly.
 *============================================================================*/

static enum theft_trial_res prop_opt_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    int default_val = val + 100;
    
    /* Test with Some value */
    Option_int opt_some = Some(int, val);
    
    /* Test OPT_IS_SOME vs opt.vt->opt_is_some */
    bool is_some_macro = OPT_IS_SOME(opt_some);
    bool is_some_vtable = opt_some.vt->opt_is_some(&opt_some);
    
    if (is_some_macro != is_some_vtable) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test OPT_IS_NONE vs opt.vt->opt_is_none */
    bool is_none_macro = OPT_IS_NONE(opt_some);
    bool is_none_vtable = opt_some.vt->opt_is_none(&opt_some);
    
    if (is_none_macro != is_none_vtable) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test OPT_UNWRAP vs opt.vt->opt_unwrap */
    int unwrap_macro = OPT_UNWRAP(opt_some);
    int unwrap_vtable = opt_some.vt->opt_unwrap(&opt_some);
    
    if (unwrap_macro != unwrap_vtable) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test OPT_UNWRAP_OR vs opt.vt->opt_unwrap_or */
    int unwrap_or_macro = OPT_UNWRAP_OR(opt_some, default_val);
    int unwrap_or_vtable = opt_some.vt->opt_unwrap_or(&opt_some, default_val);
    
    if (unwrap_or_macro != unwrap_or_vtable) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test with None value */
    Option_int opt_none = None(int);
    
    /* Test OPT_IS_SOME vs opt.vt->opt_is_some for None */
    is_some_macro = OPT_IS_SOME(opt_none);
    is_some_vtable = opt_none.vt->opt_is_some(&opt_none);
    
    if (is_some_macro != is_some_vtable) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test OPT_IS_NONE vs opt.vt->opt_is_none for None */
    is_none_macro = OPT_IS_NONE(opt_none);
    is_none_vtable = opt_none.vt->opt_is_none(&opt_none);
    
    if (is_none_macro != is_none_vtable) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test OPT_UNWRAP_OR vs opt.vt->opt_unwrap_or for None */
    unwrap_or_macro = OPT_UNWRAP_OR(opt_none, default_val);
    unwrap_or_vtable = opt_none.vt->opt_unwrap_or(&opt_none, default_val);
    
    if (unwrap_or_macro != unwrap_or_vtable) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6 (extended): Convenience macro equivalence (Result)
 * For any Result_T_E instance, calling convenience macros (RES_IS_OK,
 * RES_IS_ERR, RES_UNWRAP_OK, RES_UNWRAP_ERR, RES_UNWRAP_OK_OR) shall produce
 * identical results to calling the vtable function pointers directly.
 *============================================================================*/

static enum theft_trial_res prop_res_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    int err_val = val + 50;
    int default_val = val + 100;
    
    /* Test with Ok value */
    Result_int_int res_ok = Ok(int, int, val);
    
    /* Test RES_IS_OK vs res.vt->res_is_ok */
    bool is_ok_macro = RES_IS_OK(res_ok);
    bool is_ok_vtable = res_ok.vt->res_is_ok(&res_ok);
    
    if (is_ok_macro != is_ok_vtable) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test RES_IS_ERR vs res.vt->res_is_err */
    bool is_err_macro = RES_IS_ERR(res_ok);
    bool is_err_vtable = res_ok.vt->res_is_err(&res_ok);
    
    if (is_err_macro != is_err_vtable) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test RES_UNWRAP_OK vs res.vt->res_unwrap_ok */
    int unwrap_ok_macro = RES_UNWRAP_OK(res_ok);
    int unwrap_ok_vtable = res_ok.vt->res_unwrap_ok(&res_ok);
    
    if (unwrap_ok_macro != unwrap_ok_vtable) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test RES_UNWRAP_OK_OR vs res.vt->res_unwrap_ok_or */
    int unwrap_ok_or_macro = RES_UNWRAP_OK_OR(res_ok, default_val);
    int unwrap_ok_or_vtable = res_ok.vt->res_unwrap_ok_or(&res_ok, default_val);
    
    if (unwrap_ok_or_macro != unwrap_ok_or_vtable) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test with Err value */
    Result_int_int res_err = Err(int, int, err_val);
    
    /* Test RES_IS_OK vs res.vt->res_is_ok for Err */
    is_ok_macro = RES_IS_OK(res_err);
    is_ok_vtable = res_err.vt->res_is_ok(&res_err);
    
    if (is_ok_macro != is_ok_vtable) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test RES_IS_ERR vs res.vt->res_is_err for Err */
    is_err_macro = RES_IS_ERR(res_err);
    is_err_vtable = res_err.vt->res_is_err(&res_err);
    
    if (is_err_macro != is_err_vtable) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test RES_UNWRAP_ERR vs res.vt->res_unwrap_err */
    int unwrap_err_macro = RES_UNWRAP_ERR(res_err);
    int unwrap_err_vtable = res_err.vt->res_unwrap_err(&res_err);
    
    if (unwrap_err_macro != unwrap_err_vtable) {
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test RES_UNWRAP_OK_OR vs res.vt->res_unwrap_ok_or for Err */
    unwrap_ok_or_macro = RES_UNWRAP_OK_OR(res_err, default_val);
    unwrap_ok_or_vtable = res_err.vt->res_unwrap_ok_or(&res_err, default_val);
    
    if (unwrap_ok_or_macro != unwrap_ok_or_vtable) {
        return THEFT_TRIAL_FAIL;
    }
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6 (extended): Convenience macro equivalence (UniquePtr)
 * For any UniquePtr_T instance, calling convenience macros (UPTR_GET,
 * UPTR_DEREF, UPTR_MOVE, UPTR_FREE) shall produce identical results
 * to calling the vtable function pointers directly.
 *============================================================================*/

static enum theft_trial_res prop_uptr_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    /* Create two identical unique pointers - one for macro ops, one for direct vtable ops */
    UniquePtr_int u_macro = unique_int_new(val);
    UniquePtr_int u_vtable = unique_int_new(val);
    
    /* Test UPTR_GET vs u.vt->uptr_get */
    int *get_macro = UPTR_GET(u_macro);
    int *get_vtable = u_vtable.vt->uptr_get(&u_vtable);
    
    if (*get_macro != *get_vtable) {
        UPTR_FREE(u_macro);
        u_vtable.vt->uptr_free(&u_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test UPTR_DEREF vs u.vt->uptr_deref */
    int deref_macro = UPTR_DEREF(u_macro);
    int deref_vtable = u_vtable.vt->uptr_deref(&u_vtable);
    
    if (deref_macro != deref_vtable) {
        UPTR_FREE(u_macro);
        u_vtable.vt->uptr_free(&u_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test UPTR_MOVE vs u.vt->uptr_move */
    UniquePtr_int moved_macro = UPTR_MOVE(u_macro);
    UniquePtr_int moved_vtable = u_vtable.vt->uptr_move(&u_vtable);
    
    /* After move, original should be null */
    if (u_macro.ptr != NULL || u_vtable.ptr != NULL) {
        UPTR_FREE(moved_macro);
        moved_vtable.vt->uptr_free(&moved_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Moved pointers should have same value */
    if (UPTR_DEREF(moved_macro) != moved_vtable.vt->uptr_deref(&moved_vtable)) {
        UPTR_FREE(moved_macro);
        moved_vtable.vt->uptr_free(&moved_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Cleanup using UPTR_FREE for macro, direct vtable for other */
    UPTR_FREE(moved_macro);
    moved_vtable.vt->uptr_free(&moved_vtable);
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6 (extended): Convenience macro equivalence (SharedPtr)
 * For any SharedPtr_T instance, calling convenience macros (SPTR_GET,
 * SPTR_DEREF, SPTR_CLONE, SPTR_COUNT, SPTR_RELEASE) shall produce identical
 * results to calling the vtable function pointers directly.
 *============================================================================*/

static enum theft_trial_res prop_sptr_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    
    /* Create two identical shared pointers - one for macro ops, one for direct vtable ops */
    SharedPtr_int s_macro = shared_int_new(val);
    SharedPtr_int s_vtable = shared_int_new(val);
    
    /* Test SPTR_GET vs s.vt->sptr_get */
    int *get_macro = SPTR_GET(s_macro);
    int *get_vtable = s_vtable.vt->sptr_get(&s_vtable);
    
    if (*get_macro != *get_vtable) {
        SPTR_RELEASE(s_macro);
        s_vtable.vt->sptr_release(&s_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test SPTR_DEREF vs s.vt->sptr_deref */
    int deref_macro = SPTR_DEREF(s_macro);
    int deref_vtable = s_vtable.vt->sptr_deref(&s_vtable);
    
    if (deref_macro != deref_vtable) {
        SPTR_RELEASE(s_macro);
        s_vtable.vt->sptr_release(&s_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test SPTR_COUNT vs s.vt->sptr_count */
    size_t count_macro = SPTR_COUNT(s_macro);
    size_t count_vtable = s_vtable.vt->sptr_count(&s_vtable);
    
    if (count_macro != count_vtable) {
        SPTR_RELEASE(s_macro);
        s_vtable.vt->sptr_release(&s_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test SPTR_CLONE vs s.vt->sptr_clone */
    SharedPtr_int clone_macro = SPTR_CLONE(s_macro);
    SharedPtr_int clone_vtable = s_vtable.vt->sptr_clone(&s_vtable);
    
    /* After clone, count should be 2 for both */
    if (SPTR_COUNT(s_macro) != s_vtable.vt->sptr_count(&s_vtable)) {
        SPTR_RELEASE(clone_macro);
        clone_vtable.vt->sptr_release(&clone_vtable);
        SPTR_RELEASE(s_macro);
        s_vtable.vt->sptr_release(&s_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Cloned pointers should have same value */
    if (SPTR_DEREF(clone_macro) != clone_vtable.vt->sptr_deref(&clone_vtable)) {
        SPTR_RELEASE(clone_macro);
        clone_vtable.vt->sptr_release(&clone_vtable);
        SPTR_RELEASE(s_macro);
        s_vtable.vt->sptr_release(&s_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Cleanup using SPTR_RELEASE for macro, direct vtable for other */
    SPTR_RELEASE(clone_macro);
    clone_vtable.vt->sptr_release(&clone_vtable);
    SPTR_RELEASE(s_macro);
    s_vtable.vt->sptr_release(&s_vtable);
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6 (extended): Convenience macro equivalence (Channel)
 * For any Channel_T instance, calling convenience macros (CHAN_SEND,
 * CHAN_RECV, CHAN_TRY_SEND, CHAN_TRY_RECV, CHAN_CLOSE, CHAN_IS_CLOSED,
 * CHAN_FREE) shall produce identical results to calling the vtable
 * function pointers directly.
 *============================================================================*/

static enum theft_trial_res prop_chan_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    long val = (long)(*val_ptr);
    
    /* Create two identical channels - one for macro ops, one for direct vtable ops */
    Channel_long *ch_macro = chan_long_new(10);
    Channel_long *ch_vtable = chan_long_new(10);
    
    if (!ch_macro || !ch_vtable) {
        if (ch_macro) CHAN_FREE(ch_macro);
        if (ch_vtable) ch_vtable->vt->chan_free(ch_vtable);
        return THEFT_TRIAL_SKIP;
    }
    
    /* Test CHAN_IS_CLOSED vs ch->vt->chan_is_closed */
    bool is_closed_macro = CHAN_IS_CLOSED(ch_macro);
    bool is_closed_vtable = ch_vtable->vt->chan_is_closed(ch_vtable);
    
    if (is_closed_macro != is_closed_vtable) {
        CHAN_FREE(ch_macro);
        ch_vtable->vt->chan_free(ch_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test CHAN_SEND vs ch->vt->chan_send */
    ChanStatus send_macro = CHAN_SEND(ch_macro, val);
    ChanStatus send_vtable = ch_vtable->vt->chan_send(ch_vtable, val);
    
    if (send_macro != send_vtable) {
        CHAN_FREE(ch_macro);
        ch_vtable->vt->chan_free(ch_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Send more values */
    CHAN_SEND(ch_macro, val + 1);
    ch_vtable->vt->chan_send(ch_vtable, val + 1);
    
    CHAN_SEND(ch_macro, val + 2);
    ch_vtable->vt->chan_send(ch_vtable, val + 2);
    
    /* Test CHAN_TRY_RECV vs ch->vt->chan_try_recv */
    Option_long recv_macro = CHAN_TRY_RECV(ch_macro);
    Option_long recv_vtable = ch_vtable->vt->chan_try_recv(ch_vtable);
    
    if (is_some(recv_macro) != is_some(recv_vtable)) {
        CHAN_FREE(ch_macro);
        ch_vtable->vt->chan_free(ch_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    if (is_some(recv_macro) && unwrap(recv_macro) != unwrap(recv_vtable)) {
        CHAN_FREE(ch_macro);
        ch_vtable->vt->chan_free(ch_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test CHAN_RECV vs ch->vt->chan_recv */
    recv_macro = CHAN_RECV(ch_macro);
    recv_vtable = ch_vtable->vt->chan_recv(ch_vtable);
    
    if (is_some(recv_macro) != is_some(recv_vtable)) {
        CHAN_FREE(ch_macro);
        ch_vtable->vt->chan_free(ch_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    if (is_some(recv_macro) && unwrap(recv_macro) != unwrap(recv_vtable)) {
        CHAN_FREE(ch_macro);
        ch_vtable->vt->chan_free(ch_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test CHAN_TRY_SEND vs ch->vt->chan_try_send */
    ChanStatus try_send_macro = CHAN_TRY_SEND(ch_macro, val + 10);
    ChanStatus try_send_vtable = ch_vtable->vt->chan_try_send(ch_vtable, val + 10);
    
    if (try_send_macro != try_send_vtable) {
        CHAN_FREE(ch_macro);
        ch_vtable->vt->chan_free(ch_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Test CHAN_CLOSE vs ch->vt->chan_close */
    CHAN_CLOSE(ch_macro);
    ch_vtable->vt->chan_close(ch_vtable);
    
    /* Test CHAN_IS_CLOSED after close */
    is_closed_macro = CHAN_IS_CLOSED(ch_macro);
    is_closed_vtable = ch_vtable->vt->chan_is_closed(ch_vtable);
    
    if (is_closed_macro != is_closed_vtable) {
        CHAN_FREE(ch_macro);
        ch_vtable->vt->chan_free(ch_vtable);
        return THEFT_TRIAL_FAIL;
    }
    
    /* Cleanup using CHAN_FREE for macro, direct vtable for other */
    CHAN_FREE(ch_macro);
    ch_vtable->vt->chan_free(ch_vtable);
    
    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Test Registration
 *============================================================================*/

/* Minimum iterations for property tests */
#define MIN_TEST_TRIALS 100

typedef struct {
    const char *name;
    theft_propfun1 *prop;
    enum theft_builtin_type_info type;
} VtableMacroTest;

static VtableMacroTest vtable_macro_tests[] = {
    {
        "Property 6 (vtable): Vector convenience macro equivalence",
        prop_vec_macro_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 6 (vtable): HashMap convenience macro equivalence",
        prop_map_macro_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 6 (vtable): Slice convenience macro equivalence",
        prop_slice_macro_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 6 (vtable): String convenience macro equivalence",
        prop_str_macro_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 6 (extended): Option convenience macro equivalence",
        prop_opt_macro_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 6 (extended): Result convenience macro equivalence",
        prop_res_macro_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 6 (extended): UniquePtr convenience macro equivalence",
        prop_uptr_macro_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 6 (extended): SharedPtr convenience macro equivalence",
        prop_sptr_macro_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 6 (extended): Channel convenience macro equivalence",
        prop_chan_macro_equivalence,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_VTABLE_MACRO_TESTS (sizeof(vtable_macro_tests) / sizeof(vtable_macro_tests[0]))

int run_vtable_macro_tests(theft_seed seed) {
    int failures = 0;
    
    printf("\nVtable Convenience Macro Tests:\n");
    
    for (size_t i = 0; i < NUM_VTABLE_MACRO_TESTS; i++) {
        VtableMacroTest *test = &vtable_macro_tests[i];
        
        struct theft_run_config config = {
            .name = test->name,
            .prop1 = test->prop,
            .type_info = { theft_get_builtin_type_info(test->type) },
            .trials = MIN_TEST_TRIALS,
            .seed = seed ? seed : theft_seed_of_time(),
        };
        
        enum theft_run_res res = theft_run(&config);
        
        const char *status;
        switch (res) {
            case THEFT_RUN_PASS:
                status = "\033[32mPASS\033[0m";
                break;
            case THEFT_RUN_FAIL:
                status = "\033[31mFAIL\033[0m";
                failures++;
                break;
            default:
                status = "\033[31mERROR\033[0m";
                failures++;
                break;
        }
        printf("  [%s] %s\n", status, test->name);
    }
    
    return failures;
}

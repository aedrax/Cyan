/**
 * @file test_macros.c
 * @brief Property-based tests for type-first convenience macro equivalence
 *
 * Tests validate correctness property:
 * - Property 6: Convenience macro equivalence
 *   For any collection instance and valid operation arguments, calling a
 *   type-first convenience macro (VEC_PUSH(T, ...), MAP_GET(K, V, ...),
 *   etc.) shall produce identical results to calling the corresponding
 *   standalone function directly.
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
#include <cyan/bitset.h>

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

/* Bitset types for BS_ and FLAGS_ macro testing */
BITSET_DEFINE(8);
BITSET_DEFINE(3);
FLAGS_DEFINE(TestPerms, READ, WRITE, EXECUTE);

/*============================================================================
 * Property 6: Convenience macro equivalence (Vector)
 * For any Vec_T instance and valid operations, calling type-first macros
 * (VEC_PUSH, VEC_POP, VEC_GET, VEC_LEN, VEC_INSERT, VEC_REMOVE, VEC_EXTEND,
 * VEC_RESERVE, VEC_CLEAR, VEC_FREE) shall produce identical results to
 * calling the standalone vec_T_* functions directly.
 *============================================================================*/

static enum theft_trial_res prop_vec_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr % 100000);

    /* Create two identical vectors - one for macro ops, one for function ops */
    Vec_int v_macro = vec_int_new();
    Vec_int v_fn = vec_int_new();

    /* Test VEC_PUSH vs vec_int_push */
    VEC_PUSH(int, v_macro, val);
    vec_int_push(&v_fn, val);

    VEC_PUSH(int, v_macro, val + 1);
    vec_int_push(&v_fn, val + 1);

    VEC_PUSH(int, v_macro, val + 2);
    vec_int_push(&v_fn, val + 2);

    /* Test VEC_LEN vs vec_int_len */
    if (VEC_LEN(int, v_macro) != vec_int_len(&v_fn)) {
        VEC_FREE(int, v_macro);
        vec_int_free(&v_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test VEC_INSERT vs vec_int_insert */
    VEC_INSERT(int, v_macro, 1, val + 10);
    vec_int_insert(&v_fn, 1, val + 10);

    /* Test VEC_GET vs vec_int_get for all indices */
    for (size_t i = 0; i < VEC_LEN(int, v_macro); i++) {
        Option_int opt_macro = VEC_GET(int, v_macro, i);
        Option_int opt_fn = vec_int_get(&v_fn, i);

        if (is_some(opt_macro) != is_some(opt_fn)) {
            VEC_FREE(int, v_macro);
            vec_int_free(&v_fn);
            return THEFT_TRIAL_FAIL;
        }

        if (is_some(opt_macro) && unwrap(opt_macro) != unwrap(opt_fn)) {
            VEC_FREE(int, v_macro);
            vec_int_free(&v_fn);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Test VEC_GET out-of-bounds vs vec_int_get out-of-bounds */
    Option_int oob_macro = VEC_GET(int, v_macro, VEC_LEN(int, v_macro) + 10);
    Option_int oob_fn = vec_int_get(&v_fn, vec_int_len(&v_fn) + 10);

    if (is_none(oob_macro) != is_none(oob_fn)) {
        VEC_FREE(int, v_macro);
        vec_int_free(&v_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test VEC_REMOVE vs vec_int_remove */
    Option_int rem_macro = VEC_REMOVE(int, v_macro, 1);
    Option_int rem_fn = vec_int_remove(&v_fn, 1);

    if (is_some(rem_macro) != is_some(rem_fn)) {
        VEC_FREE(int, v_macro);
        vec_int_free(&v_fn);
        return THEFT_TRIAL_FAIL;
    }

    if (is_some(rem_macro) && unwrap(rem_macro) != unwrap(rem_fn)) {
        VEC_FREE(int, v_macro);
        vec_int_free(&v_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test VEC_EXTEND vs vec_int_extend */
    int extra[3] = { val + 20, val + 21, val + 22 };
    VEC_EXTEND(int, v_macro, extra, 3);
    vec_int_extend(&v_fn, extra, 3);

    if (VEC_LEN(int, v_macro) != vec_int_len(&v_fn)) {
        VEC_FREE(int, v_macro);
        vec_int_free(&v_fn);
        return THEFT_TRIAL_FAIL;
    }

    for (size_t i = 0; i < VEC_LEN(int, v_macro); i++) {
        if (unwrap(VEC_GET(int, v_macro, i)) != unwrap(vec_int_get(&v_fn, i))) {
            VEC_FREE(int, v_macro);
            vec_int_free(&v_fn);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Test VEC_RESERVE vs vec_int_reserve */
    VEC_RESERVE(int, v_macro, 128);
    vec_int_reserve(&v_fn, 128);

    if (v_macro.cap != v_fn.cap || VEC_LEN(int, v_macro) != vec_int_len(&v_fn)) {
        VEC_FREE(int, v_macro);
        vec_int_free(&v_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test VEC_POP vs vec_int_pop */
    Option_int pop_macro = VEC_POP(int, v_macro);
    Option_int pop_fn = vec_int_pop(&v_fn);

    if (is_some(pop_macro) != is_some(pop_fn)) {
        VEC_FREE(int, v_macro);
        vec_int_free(&v_fn);
        return THEFT_TRIAL_FAIL;
    }

    if (is_some(pop_macro) && unwrap(pop_macro) != unwrap(pop_fn)) {
        VEC_FREE(int, v_macro);
        vec_int_free(&v_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test VEC_CLEAR vs vec_int_clear */
    VEC_CLEAR(int, v_macro);
    vec_int_clear(&v_fn);

    if (VEC_LEN(int, v_macro) != vec_int_len(&v_fn) || VEC_LEN(int, v_macro) != 0) {
        VEC_FREE(int, v_macro);
        vec_int_free(&v_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test VEC_POP on empty vector */
    Option_int empty_pop_macro = VEC_POP(int, v_macro);
    Option_int empty_pop_fn = vec_int_pop(&v_fn);

    if (is_none(empty_pop_macro) != is_none(empty_pop_fn)) {
        VEC_FREE(int, v_macro);
        vec_int_free(&v_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Cleanup using VEC_FREE for macro, standalone function for other */
    VEC_FREE(int, v_macro);
    vec_int_free(&v_fn);

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6: Convenience macro equivalence (HashMap)
 * For any HashMap_K_V instance and valid operations, calling type-first
 * macros (MAP_INSERT, MAP_GET, MAP_CONTAINS, MAP_REMOVE, MAP_LEN, MAP_FREE)
 * shall produce identical results to calling the standalone functions.
 *============================================================================*/

static enum theft_trial_res prop_map_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int seed = (int)(*val_ptr);

    /* Create two identical hashmaps - one for macro ops, one for function ops */
    HashMap_int_int m_macro = hashmap_int_int_new();
    HashMap_int_int m_fn = hashmap_int_int_new();

    /* Test MAP_INSERT vs hashmap_int_int_insert */
    int keys[5];
    int values[5];
    for (int i = 0; i < 5; i++) {
        keys[i] = seed + i * 7;
        values[i] = seed * 3 + i;

        MAP_INSERT(int, int, m_macro, keys[i], values[i]);
        hashmap_int_int_insert(&m_fn, keys[i], values[i]);
    }

    /* Test MAP_LEN vs hashmap_int_int_len */
    if (MAP_LEN(int, int, m_macro) != hashmap_int_int_len(&m_fn)) {
        MAP_FREE(int, int, m_macro);
        hashmap_int_int_free(&m_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test MAP_GET vs hashmap_int_int_get for all inserted keys */
    for (int i = 0; i < 5; i++) {
        Option_int opt_macro = MAP_GET(int, int, m_macro, keys[i]);
        Option_int opt_fn = hashmap_int_int_get(&m_fn, keys[i]);

        if (is_some(opt_macro) != is_some(opt_fn)) {
            MAP_FREE(int, int, m_macro);
            hashmap_int_int_free(&m_fn);
            return THEFT_TRIAL_FAIL;
        }

        if (is_some(opt_macro) && unwrap(opt_macro) != unwrap(opt_fn)) {
            MAP_FREE(int, int, m_macro);
            hashmap_int_int_free(&m_fn);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Test MAP_GET for missing key vs hashmap_int_int_get for missing key */
    int missing_key = seed + 1000;
    Option_int missing_macro = MAP_GET(int, int, m_macro, missing_key);
    Option_int missing_fn = hashmap_int_int_get(&m_fn, missing_key);

    if (is_none(missing_macro) != is_none(missing_fn)) {
        MAP_FREE(int, int, m_macro);
        hashmap_int_int_free(&m_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test MAP_CONTAINS vs hashmap_int_int_contains */
    if (MAP_CONTAINS(int, int, m_macro, keys[0]) != hashmap_int_int_contains(&m_fn, keys[0])) {
        MAP_FREE(int, int, m_macro);
        hashmap_int_int_free(&m_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test MAP_CONTAINS for missing key */
    if (MAP_CONTAINS(int, int, m_macro, missing_key) != hashmap_int_int_contains(&m_fn, missing_key)) {
        MAP_FREE(int, int, m_macro);
        hashmap_int_int_free(&m_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test MAP_REMOVE vs hashmap_int_int_remove */
    Option_int remove_macro = MAP_REMOVE(int, int, m_macro, keys[0]);
    Option_int remove_fn = hashmap_int_int_remove(&m_fn, keys[0]);

    if (is_some(remove_macro) != is_some(remove_fn)) {
        MAP_FREE(int, int, m_macro);
        hashmap_int_int_free(&m_fn);
        return THEFT_TRIAL_FAIL;
    }

    if (is_some(remove_macro) && unwrap(remove_macro) != unwrap(remove_fn)) {
        MAP_FREE(int, int, m_macro);
        hashmap_int_int_free(&m_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Verify lengths are still equal after remove */
    if (MAP_LEN(int, int, m_macro) != hashmap_int_int_len(&m_fn)) {
        MAP_FREE(int, int, m_macro);
        hashmap_int_int_free(&m_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test MAP_REMOVE for missing key */
    Option_int remove_missing_macro = MAP_REMOVE(int, int, m_macro, missing_key);
    Option_int remove_missing_fn = hashmap_int_int_remove(&m_fn, missing_key);

    if (is_none(remove_missing_macro) != is_none(remove_missing_fn)) {
        MAP_FREE(int, int, m_macro);
        hashmap_int_int_free(&m_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Cleanup using MAP_FREE for macro, standalone function for other */
    MAP_FREE(int, int, m_macro);
    hashmap_int_int_free(&m_fn);

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6: Convenience macro equivalence (Slice)
 * For any Slice_T instance and valid operations, calling type-first macros
 * (SLICE_GET, SLICE_SUBSLICE, SLICE_LEN) shall produce identical results
 * to calling the standalone functions directly.
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

    /* Test SLICE_LEN vs slice_int_len */
    if (SLICE_LEN(int, s) != slice_int_len(s)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test SLICE_GET vs slice_int_get for all valid indices */
    for (size_t i = 0; i < slice_int_len(s); i++) {
        Option_int opt_macro = SLICE_GET(int, s, i);
        Option_int opt_fn = slice_int_get(s, i);

        if (is_some(opt_macro) != is_some(opt_fn)) {
            return THEFT_TRIAL_FAIL;
        }

        if (is_some(opt_macro) && unwrap(opt_macro) != unwrap(opt_fn)) {
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Test SLICE_GET out-of-bounds vs slice_int_get out-of-bounds */
    Option_int oob_macro = SLICE_GET(int, s, slice_int_len(s) + 10);
    Option_int oob_fn = slice_int_get(s, slice_int_len(s) + 10);

    if (is_none(oob_macro) != is_none(oob_fn)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test SLICE_SUBSLICE vs slice_int_subslice */
    Slice_int sub_macro = SLICE_SUBSLICE(int, s, 2, 7);
    Slice_int sub_fn = slice_int_subslice(s, 2, 7);

    /* Verify subslice lengths are equal */
    if (SLICE_LEN(int, sub_macro) != slice_int_len(sub_fn)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Verify subslice elements are equal */
    for (size_t i = 0; i < SLICE_LEN(int, sub_macro); i++) {
        Option_int elem_macro = SLICE_GET(int, sub_macro, i);
        Option_int elem_fn = slice_int_get(sub_fn, i);

        if (is_some(elem_macro) != is_some(elem_fn)) {
            return THEFT_TRIAL_FAIL;
        }

        if (is_some(elem_macro) && unwrap(elem_macro) != unwrap(elem_fn)) {
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Test SLICE_SUBSLICE with clamped bounds (start > end) */
    Slice_int clamped_macro = SLICE_SUBSLICE(int, s, 5, 3);
    Slice_int clamped_fn = slice_int_subslice(s, 5, 3);

    if (SLICE_LEN(int, clamped_macro) != slice_int_len(clamped_fn)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test SLICE_SUBSLICE with bounds beyond length */
    Slice_int beyond_macro = SLICE_SUBSLICE(int, s, 8, 15);
    Slice_int beyond_fn = slice_int_subslice(s, 8, 15);

    if (SLICE_LEN(int, beyond_macro) != slice_int_len(beyond_fn)) {
        return THEFT_TRIAL_FAIL;
    }

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6: Convenience macro equivalence (String)
 * For any String instance and valid operations, calling macros (STR_PUSH,
 * STR_APPEND, STR_CLEAR, STR_GET, STR_LEN, STR_CSTR, STR_SLICE, STR_FIND,
 * STR_CONTAINS, STR_FREE) shall produce identical results to calling the
 * standalone string_* functions directly.
 *============================================================================*/

static enum theft_trial_res prop_str_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int seed = (int)(*val_ptr);

    /* Create two identical strings - one for macro ops, one for function ops */
    String s_macro = string_from("Initial");
    String s_fn = string_from("Initial");

    /* Test STR_PUSH vs string_push */
    char c = 'X';
    STR_PUSH(s_macro, c);
    string_push(&s_fn, c);

    /* Test STR_LEN vs string_len */
    if (STR_LEN(s_macro) != string_len(&s_fn)) {
        STR_FREE(s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test STR_CSTR vs string_cstr */
    if (strcmp(STR_CSTR(s_macro), string_cstr(&s_fn)) != 0) {
        STR_FREE(s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test STR_APPEND vs string_append */
    char append_str[32];
    snprintf(append_str, sizeof(append_str), "_%d_", seed);
    STR_APPEND(s_macro, append_str);
    string_append(&s_fn, append_str);

    if (STR_LEN(s_macro) != string_len(&s_fn)) {
        STR_FREE(s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    if (strcmp(STR_CSTR(s_macro), string_cstr(&s_fn)) != 0) {
        STR_FREE(s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test STR_GET vs string_get for all indices */
    for (size_t i = 0; i < STR_LEN(s_macro); i++) {
        Option_char opt_macro = STR_GET(s_macro, i);
        Option_char opt_fn = string_get(&s_fn, i);

        if (is_some(opt_macro) != is_some(opt_fn)) {
            STR_FREE(s_macro);
            string_free(&s_fn);
            return THEFT_TRIAL_FAIL;
        }

        if (is_some(opt_macro) && unwrap(opt_macro) != unwrap(opt_fn)) {
            STR_FREE(s_macro);
            string_free(&s_fn);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Test STR_GET out-of-bounds vs string_get out-of-bounds */
    Option_char oob_macro = STR_GET(s_macro, STR_LEN(s_macro) + 100);
    Option_char oob_fn = string_get(&s_fn, string_len(&s_fn) + 100);

    if (is_none(oob_macro) != is_none(oob_fn)) {
        STR_FREE(s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test STR_FIND vs string_find (both present and absent needles) */
    Option_size_t find_macro = STR_FIND(s_macro, "nitia");
    Option_size_t find_fn = string_find(&s_fn, "nitia");

    if (is_some(find_macro) != is_some(find_fn)) {
        STR_FREE(s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    if (is_some(find_macro) && unwrap(find_macro) != unwrap(find_fn)) {
        STR_FREE(s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    Option_size_t absent_macro = STR_FIND(s_macro, "\x01missing");
    Option_size_t absent_fn = string_find(&s_fn, "\x01missing");

    if (is_none(absent_macro) != is_none(absent_fn)) {
        STR_FREE(s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test STR_CONTAINS vs string_contains */
    if (STR_CONTAINS(s_macro, "Init") != string_contains(&s_fn, "Init")) {
        STR_FREE(s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    if (STR_CONTAINS(s_macro, "\x01missing") != string_contains(&s_fn, "\x01missing")) {
        STR_FREE(s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test STR_SLICE vs string_slice */
    Slice_char slice_macro = STR_SLICE(s_macro, 0, STR_LEN(s_macro) / 2);
    Slice_char slice_fn = string_slice(&s_fn, 0, string_len(&s_fn) / 2);

    if (slice_macro.len != slice_fn.len) {
        STR_FREE(s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    for (size_t i = 0; i < slice_macro.len; i++) {
        if (slice_macro.data[i] != slice_fn.data[i]) {
            STR_FREE(s_macro);
            string_free(&s_fn);
            return THEFT_TRIAL_FAIL;
        }
    }

    /* Test STR_CLEAR vs string_clear */
    STR_CLEAR(s_macro);
    string_clear(&s_fn);

    if (STR_LEN(s_macro) != string_len(&s_fn) || STR_LEN(s_macro) != 0) {
        STR_FREE(s_macro);
        string_free(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Cleanup using STR_FREE for macro, standalone function for other */
    STR_FREE(s_macro);
    string_free(&s_fn);

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6 (extended): Convenience macro equivalence (Option)
 * For any Option_T instance, calling macros (OPT_IS_SOME, OPT_IS_NONE,
 * OPT_UNWRAP, OPT_UNWRAP_OR) shall produce identical results to calling
 * the generated option_T_* functions directly.
 *============================================================================*/

static enum theft_trial_res prop_opt_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    int default_val = val + 100;

    /* Test with Some value */
    Option_int opt_some = Some(int, val);

    /* Test OPT_IS_SOME vs option_int_is_some */
    if (OPT_IS_SOME(opt_some) != option_int_is_some(&opt_some)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test OPT_IS_NONE vs option_int_is_none */
    if (OPT_IS_NONE(opt_some) != option_int_is_none(&opt_some)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test OPT_UNWRAP vs option_int_unwrap */
    if (OPT_UNWRAP(opt_some) != option_int_unwrap(&opt_some)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test OPT_UNWRAP_OR vs option_int_unwrap_or */
    if (OPT_UNWRAP_OR(opt_some, default_val) != option_int_unwrap_or(&opt_some, default_val)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test with None value */
    Option_int opt_none = None(int);

    /* Test OPT_IS_SOME vs option_int_is_some for None */
    if (OPT_IS_SOME(opt_none) != option_int_is_some(&opt_none)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test OPT_IS_NONE vs option_int_is_none for None */
    if (OPT_IS_NONE(opt_none) != option_int_is_none(&opt_none)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test OPT_UNWRAP_OR vs option_int_unwrap_or for None */
    if (OPT_UNWRAP_OR(opt_none, default_val) != option_int_unwrap_or(&opt_none, default_val)) {
        return THEFT_TRIAL_FAIL;
    }

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6 (extended): Convenience macro equivalence (Result)
 * For any Result_T_E instance, calling macros (RES_IS_OK, RES_IS_ERR,
 * RES_UNWRAP_OK, RES_UNWRAP_ERR, RES_UNWRAP_OK_OR) shall produce identical
 * results to calling the generated result_T_E_* functions directly.
 *============================================================================*/

static enum theft_trial_res prop_res_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);
    int err_val = val + 50;
    int default_val = val + 100;

    /* Test with Ok value */
    Result_int_int res_ok = Ok(int, int, val);

    /* Test RES_IS_OK vs result_int_int_is_ok */
    if (RES_IS_OK(res_ok) != result_int_int_is_ok(&res_ok)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test RES_IS_ERR vs result_int_int_is_err */
    if (RES_IS_ERR(res_ok) != result_int_int_is_err(&res_ok)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test RES_UNWRAP_OK vs result_int_int_unwrap_ok */
    if (RES_UNWRAP_OK(res_ok) != result_int_int_unwrap_ok(&res_ok)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test RES_UNWRAP_OK_OR vs result_int_int_unwrap_ok_or */
    if (RES_UNWRAP_OK_OR(res_ok, default_val) != result_int_int_unwrap_ok_or(&res_ok, default_val)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test with Err value */
    Result_int_int res_err = Err(int, int, err_val);

    /* Test RES_IS_OK vs result_int_int_is_ok for Err */
    if (RES_IS_OK(res_err) != result_int_int_is_ok(&res_err)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test RES_IS_ERR vs result_int_int_is_err for Err */
    if (RES_IS_ERR(res_err) != result_int_int_is_err(&res_err)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test RES_UNWRAP_ERR vs result_int_int_unwrap_err */
    if (RES_UNWRAP_ERR(res_err) != result_int_int_unwrap_err(&res_err)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test RES_UNWRAP_OK_OR vs result_int_int_unwrap_ok_or for Err */
    if (RES_UNWRAP_OK_OR(res_err, default_val) != result_int_int_unwrap_ok_or(&res_err, default_val)) {
        return THEFT_TRIAL_FAIL;
    }

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6 (extended): Convenience macro equivalence (UniquePtr)
 * For any UniquePtr_T instance, calling type-first macros (UPTR_GET,
 * UPTR_DEREF, UPTR_MOVE, UPTR_FREE) shall produce identical results
 * to calling the standalone unique_T_* functions directly.
 *============================================================================*/

static enum theft_trial_res prop_uptr_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);

    /* Create two identical unique pointers - one for macro ops, one for function ops */
    UniquePtr_int u_macro = unique_int_new(val);
    UniquePtr_int u_fn = unique_int_new(val);

    /* Test UPTR_GET vs unique_int_get */
    int *get_macro = UPTR_GET(int, u_macro);
    int *get_fn = unique_int_get(&u_fn);

    if (*get_macro != *get_fn) {
        UPTR_FREE(int, u_macro);
        unique_int_free(&u_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test UPTR_DEREF vs unique_int_deref */
    if (UPTR_DEREF(int, u_macro) != unique_int_deref(&u_fn)) {
        UPTR_FREE(int, u_macro);
        unique_int_free(&u_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test UPTR_MOVE vs unique_int_move */
    UniquePtr_int moved_macro = UPTR_MOVE(int, u_macro);
    UniquePtr_int moved_fn = unique_int_move(&u_fn);

    /* After move, original should be null */
    if (u_macro.ptr != NULL || u_fn.ptr != NULL) {
        UPTR_FREE(int, moved_macro);
        unique_int_free(&moved_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Moved pointers should have same value */
    if (UPTR_DEREF(int, moved_macro) != unique_int_deref(&moved_fn)) {
        UPTR_FREE(int, moved_macro);
        unique_int_free(&moved_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Cleanup using UPTR_FREE for macro, standalone function for other */
    UPTR_FREE(int, moved_macro);
    unique_int_free(&moved_fn);

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6 (extended): Convenience macro equivalence (SharedPtr)
 * For any SharedPtr_T instance, calling type-first macros (SPTR_GET,
 * SPTR_DEREF, SPTR_CLONE, SPTR_COUNT, SPTR_RELEASE) shall produce identical
 * results to calling the standalone shared_T_* functions directly.
 *============================================================================*/

static enum theft_trial_res prop_sptr_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);

    /* Create two identical shared pointers - one for macro ops, one for function ops */
    SharedPtr_int s_macro = shared_int_new(val);
    SharedPtr_int s_fn = shared_int_new(val);

    /* Test SPTR_GET vs shared_int_get */
    int *get_macro = SPTR_GET(int, s_macro);
    int *get_fn = shared_int_get(&s_fn);

    if (*get_macro != *get_fn) {
        SPTR_RELEASE(int, s_macro);
        shared_int_release(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test SPTR_DEREF vs shared_int_deref */
    if (SPTR_DEREF(int, s_macro) != shared_int_deref(&s_fn)) {
        SPTR_RELEASE(int, s_macro);
        shared_int_release(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test SPTR_COUNT vs shared_int_count */
    if (SPTR_COUNT(int, s_macro) != shared_int_count(&s_fn)) {
        SPTR_RELEASE(int, s_macro);
        shared_int_release(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test SPTR_CLONE vs shared_int_clone */
    SharedPtr_int clone_macro = SPTR_CLONE(int, s_macro);
    SharedPtr_int clone_fn = shared_int_clone(&s_fn);

    /* After clone, count should be 2 for both */
    if (SPTR_COUNT(int, s_macro) != shared_int_count(&s_fn)) {
        SPTR_RELEASE(int, clone_macro);
        shared_int_release(&clone_fn);
        SPTR_RELEASE(int, s_macro);
        shared_int_release(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Cloned pointers should have same value */
    if (SPTR_DEREF(int, clone_macro) != shared_int_deref(&clone_fn)) {
        SPTR_RELEASE(int, clone_macro);
        shared_int_release(&clone_fn);
        SPTR_RELEASE(int, s_macro);
        shared_int_release(&s_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Cleanup using SPTR_RELEASE for macro, standalone function for other */
    SPTR_RELEASE(int, clone_macro);
    shared_int_release(&clone_fn);
    SPTR_RELEASE(int, s_macro);
    shared_int_release(&s_fn);

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6 (extended): Convenience macro equivalence (WeakPtr)
 * For any WeakPtr_T instance, calling type-first macros (WPTR_IS_EXPIRED,
 * WPTR_UPGRADE, WPTR_CLONE, WPTR_RELEASE) shall produce identical results
 * to calling the standalone weak_T_* functions, and WPTR_CLONE shall
 * maintain the shared control block's weak reference count.
 *============================================================================*/

static enum theft_trial_res prop_wptr_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    int val = (int)(*val_ptr);

    SharedPtr_int s = shared_int_new(val);
    WeakPtr_int w_macro = weak_int_from_shared(&s);
    WeakPtr_int w_fn = weak_int_from_shared(&s);

    /* Test WPTR_IS_EXPIRED vs weak_int_is_expired (valid case) */
    if (WPTR_IS_EXPIRED(int, w_macro) != weak_int_is_expired(&w_fn)) {
        weak_int_release(&w_macro);
        weak_int_release(&w_fn);
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }

    /* Test WPTR_CLONE vs weak_int_clone: weak count advances in lockstep */
    size_t weak_before = s.ctrl->weak_count;
    WeakPtr_int clone_macro = WPTR_CLONE(int, w_macro);
    WeakPtr_int clone_fn = weak_int_clone(&w_fn);

    if (s.ctrl->weak_count != weak_before + 2) {
        weak_int_release(&clone_macro);
        weak_int_release(&clone_fn);
        weak_int_release(&w_macro);
        weak_int_release(&w_fn);
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }

    /* Clones share the control block with their sources */
    if (clone_macro.ctrl != w_macro.ctrl || clone_fn.ctrl != w_fn.ctrl) {
        weak_int_release(&clone_macro);
        weak_int_release(&clone_fn);
        weak_int_release(&w_macro);
        weak_int_release(&w_fn);
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }

    /* Test WPTR_RELEASE vs weak_int_release: weak count drops back */
    WPTR_RELEASE(int, clone_macro);
    weak_int_release(&clone_fn);

    if (s.ctrl->weak_count != weak_before) {
        weak_int_release(&w_macro);
        weak_int_release(&w_fn);
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }

    /* Test WPTR_UPGRADE vs weak_int_upgrade (valid case) */
    Option_SharedPtr_int up_macro = WPTR_UPGRADE(int, w_macro);
    Option_SharedPtr_int up_fn = weak_int_upgrade(&w_fn);

    if (up_macro.has_value != up_fn.has_value) {
        if (up_macro.has_value) shared_int_release(&up_macro.value);
        if (up_fn.has_value) shared_int_release(&up_fn.value);
        weak_int_release(&w_macro);
        weak_int_release(&w_fn);
        shared_int_release(&s);
        return THEFT_TRIAL_FAIL;
    }

    if (up_macro.has_value) {
        /* Both should have same value */
        if (shared_int_deref(&up_macro.value) != shared_int_deref(&up_fn.value)) {
            shared_int_release(&up_macro.value);
            shared_int_release(&up_fn.value);
            weak_int_release(&w_macro);
            weak_int_release(&w_fn);
            shared_int_release(&s);
            return THEFT_TRIAL_FAIL;
        }
        shared_int_release(&up_macro.value);
        shared_int_release(&up_fn.value);
    }

    /* Release the shared pointer to make both weak pointers expired */
    shared_int_release(&s);

    /* Test WPTR_IS_EXPIRED vs weak_int_is_expired (expired case) */
    if (WPTR_IS_EXPIRED(int, w_macro) != weak_int_is_expired(&w_fn)) {
        weak_int_release(&w_macro);
        weak_int_release(&w_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test WPTR_UPGRADE vs weak_int_upgrade (expired case) */
    up_macro = WPTR_UPGRADE(int, w_macro);
    up_fn = weak_int_upgrade(&w_fn);

    if (up_macro.has_value != up_fn.has_value) {
        if (up_macro.has_value) shared_int_release(&up_macro.value);
        if (up_fn.has_value) shared_int_release(&up_fn.value);
        weak_int_release(&w_macro);
        weak_int_release(&w_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Cleanup using WPTR_RELEASE for macro, standalone function for other */
    WPTR_RELEASE(int, w_macro);
    weak_int_release(&w_fn);

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6 (extended): Convenience macro equivalence (Channel)
 * For any Channel_T instance, calling type-first macros (CHAN_SEND,
 * CHAN_RECV, CHAN_TRY_SEND, CHAN_TRY_RECV, CHAN_CLOSE, CHAN_IS_CLOSED,
 * CHAN_FREE) shall produce identical results to calling the standalone
 * chan_T_* functions directly.
 *============================================================================*/

static enum theft_trial_res prop_chan_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    long val = (long)(*val_ptr);

    /* Create two identical channels - one for macro ops, one for function ops */
    Channel_long *ch_macro = chan_long_new(10);
    Channel_long *ch_fn = chan_long_new(10);

    if (!ch_macro || !ch_fn) {
        if (ch_macro) CHAN_FREE(long, ch_macro);
        if (ch_fn) chan_long_free(ch_fn);
        return THEFT_TRIAL_SKIP;
    }

    /* Test CHAN_IS_CLOSED vs chan_long_is_closed */
    if (CHAN_IS_CLOSED(long, ch_macro) != chan_long_is_closed(ch_fn)) {
        CHAN_FREE(long, ch_macro);
        chan_long_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test CHAN_SEND vs chan_long_send */
    ChanStatus send_macro = CHAN_SEND(long, ch_macro, val);
    ChanStatus send_fn = chan_long_send(ch_fn, val);

    if (send_macro != send_fn) {
        CHAN_FREE(long, ch_macro);
        chan_long_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Send more values */
    CHAN_SEND(long, ch_macro, val + 1);
    chan_long_send(ch_fn, val + 1);

    CHAN_SEND(long, ch_macro, val + 2);
    chan_long_send(ch_fn, val + 2);

    /* Test CHAN_TRY_RECV vs chan_long_try_recv */
    Option_long recv_macro = CHAN_TRY_RECV(long, ch_macro);
    Option_long recv_fn = chan_long_try_recv(ch_fn);

    if (is_some(recv_macro) != is_some(recv_fn)) {
        CHAN_FREE(long, ch_macro);
        chan_long_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    if (is_some(recv_macro) && unwrap(recv_macro) != unwrap(recv_fn)) {
        CHAN_FREE(long, ch_macro);
        chan_long_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test CHAN_RECV vs chan_long_recv */
    recv_macro = CHAN_RECV(long, ch_macro);
    recv_fn = chan_long_recv(ch_fn);

    if (is_some(recv_macro) != is_some(recv_fn)) {
        CHAN_FREE(long, ch_macro);
        chan_long_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    if (is_some(recv_macro) && unwrap(recv_macro) != unwrap(recv_fn)) {
        CHAN_FREE(long, ch_macro);
        chan_long_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test CHAN_TRY_SEND vs chan_long_try_send */
    ChanStatus try_send_macro = CHAN_TRY_SEND(long, ch_macro, val + 10);
    ChanStatus try_send_fn = chan_long_try_send(ch_fn, val + 10);

    if (try_send_macro != try_send_fn) {
        CHAN_FREE(long, ch_macro);
        chan_long_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Test CHAN_CLOSE vs chan_long_close */
    CHAN_CLOSE(long, ch_macro);
    chan_long_close(ch_fn);

    /* Test CHAN_IS_CLOSED after close */
    if (CHAN_IS_CLOSED(long, ch_macro) != chan_long_is_closed(ch_fn)) {
        CHAN_FREE(long, ch_macro);
        chan_long_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* NULL-channel behavior: the macros are NULL-safe like the functions */
    Channel_long *null_ch = NULL;

    if (CHAN_SEND(long, null_ch, val) != CHAN_CLOSED) {
        CHAN_FREE(long, ch_macro);
        chan_long_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }
    if (chan_long_send(NULL, val) != CHAN_CLOSED) {
        CHAN_FREE(long, ch_macro);
        chan_long_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    Option_long null_recv = CHAN_RECV(long, null_ch);
    if (is_some(null_recv) || is_some(chan_long_recv(NULL))) {
        CHAN_FREE(long, ch_macro);
        chan_long_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    if (!CHAN_IS_CLOSED(long, null_ch) || !chan_long_is_closed(NULL)) {
        CHAN_FREE(long, ch_macro);
        chan_long_free(ch_fn);
        return THEFT_TRIAL_FAIL;
    }

    /* Close and free on NULL are no-ops */
    CHAN_CLOSE(long, null_ch);
    CHAN_FREE(long, null_ch);

    /* Cleanup using CHAN_FREE for macro, standalone function for other */
    CHAN_FREE(long, ch_macro);
    chan_long_free(ch_fn);

    return THEFT_TRIAL_PASS;
}

/*============================================================================
 * Property 6 (extended): Convenience macro equivalence (Bitset/Flags)
 * For any Bitset_N instance, calling width-first macros (BS_SET, BS_CLEAR,
 * BS_GET, BS_TOGGLE, BS_UNION, BS_INTERSECT, BS_DIFF, BS_COMPLEMENT, BS_EQ,
 * BS_COUNT, BS_ALL, BS_ANY, BS_NONE, FLAGS_SET, FLAGS_CLEAR, FLAGS_HAS)
 * shall produce identical results to calling the standalone bitset_N_*
 * functions directly.
 *============================================================================*/

static enum theft_trial_res prop_bs_macro_equivalence(struct theft *t, void *arg1) {
    (void)t;
    int64_t *val_ptr = (int64_t *)arg1;
    u8 raw = (u8)(*val_ptr & 0xFF);
    u8 index = (u8)((*val_ptr >> 8) & 0x07);

    /* Create two identical bitsets - one for macro ops, one for function ops */
    Bitset_8 bs_macro = bitset_8_from_raw(raw);
    Bitset_8 bs_fn = bitset_8_from_raw(raw);
    Bitset_8 other = bitset_8_from_raw((u8)~raw);

    /* Test BS_GET vs bitset_8_get */
    if (BS_GET(8, bs_macro, index) != bitset_8_get(&bs_fn, index)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test BS_SET vs bitset_8_set */
    BS_SET(8, bs_macro, index);
    bitset_8_set(&bs_fn, index);

    if (bs_macro.bits != bs_fn.bits) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test BS_CLEAR vs bitset_8_clear */
    BS_CLEAR(8, bs_macro, index);
    bitset_8_clear(&bs_fn, index);

    if (bs_macro.bits != bs_fn.bits) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test BS_TOGGLE vs bitset_8_toggle */
    BS_TOGGLE(8, bs_macro, index);
    bitset_8_toggle(&bs_fn, index);

    if (bs_macro.bits != bs_fn.bits) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test BS_UNION vs bitset_8_union */
    Bitset_8 union_macro = BS_UNION(8, bs_macro, other);
    Bitset_8 union_fn = bitset_8_union(&bs_fn, &other);

    if (union_macro.bits != union_fn.bits) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test BS_INTERSECT vs bitset_8_intersect */
    Bitset_8 inter_macro = BS_INTERSECT(8, bs_macro, other);
    Bitset_8 inter_fn = bitset_8_intersect(&bs_fn, &other);

    if (inter_macro.bits != inter_fn.bits) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test BS_DIFF vs bitset_8_diff */
    Bitset_8 diff_macro = BS_DIFF(8, bs_macro, other);
    Bitset_8 diff_fn = bitset_8_diff(&bs_fn, &other);

    if (diff_macro.bits != diff_fn.bits) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test BS_COMPLEMENT vs bitset_8_complement */
    Bitset_8 comp_macro = BS_COMPLEMENT(8, bs_macro);
    Bitset_8 comp_fn = bitset_8_complement(&bs_fn);

    if (comp_macro.bits != comp_fn.bits) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test BS_EQ vs bitset_8_eq */
    if (BS_EQ(8, bs_macro, bs_fn) != bitset_8_eq(&bs_macro, &bs_fn)) {
        return THEFT_TRIAL_FAIL;
    }
    if (BS_EQ(8, bs_macro, other) != bitset_8_eq(&bs_macro, &other)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test BS_COUNT vs bitset_8_count */
    if (BS_COUNT(8, bs_macro) != bitset_8_count(&bs_fn)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test BS_ALL/BS_ANY/BS_NONE vs standalone functions */
    if (BS_ALL(8, bs_macro) != bitset_8_all(&bs_fn) ||
        BS_ANY(8, bs_macro) != bitset_8_any(&bs_fn) ||
        BS_NONE(8, bs_macro) != bitset_8_none(&bs_fn)) {
        return THEFT_TRIAL_FAIL;
    }

    /* Test FLAGS_SET/FLAGS_HAS/FLAGS_CLEAR vs standalone functions */
    Bitset_3 perms_macro = bitset_3_new();
    Bitset_3 perms_fn = bitset_3_new();

    FLAGS_SET(3, perms_macro, TestPerms_READ);
    bitset_3_set(&perms_fn, TestPerms_READ);

    FLAGS_SET(3, perms_macro, TestPerms_EXECUTE);
    bitset_3_set(&perms_fn, TestPerms_EXECUTE);

    if (perms_macro.bits != perms_fn.bits) {
        return THEFT_TRIAL_FAIL;
    }

    if (FLAGS_HAS(3, perms_macro, TestPerms_READ) != bitset_3_get(&perms_fn, TestPerms_READ) ||
        FLAGS_HAS(3, perms_macro, TestPerms_WRITE) != bitset_3_get(&perms_fn, TestPerms_WRITE)) {
        return THEFT_TRIAL_FAIL;
    }

    FLAGS_CLEAR(3, perms_macro, TestPerms_READ);
    bitset_3_clear(&perms_fn, TestPerms_READ);

    if (perms_macro.bits != perms_fn.bits) {
        return THEFT_TRIAL_FAIL;
    }

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
} MacroTest;

static MacroTest macro_tests[] = {
    {
        "Property 6 (macros): Vector convenience macro equivalence",
        prop_vec_macro_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 6 (macros): HashMap convenience macro equivalence",
        prop_map_macro_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 6 (macros): Slice convenience macro equivalence",
        prop_slice_macro_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 6 (macros): String convenience macro equivalence",
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
        "Property 6 (extended): WeakPtr convenience macro equivalence",
        prop_wptr_macro_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 6 (extended): Channel convenience macro equivalence",
        prop_chan_macro_equivalence,
        THEFT_BUILTIN_int64_t
    },
    {
        "Property 6 (extended): Bitset/Flags convenience macro equivalence",
        prop_bs_macro_equivalence,
        THEFT_BUILTIN_int64_t
    },
};

#define NUM_MACRO_TESTS (sizeof(macro_tests) / sizeof(macro_tests[0]))

int run_macro_tests(theft_seed seed, int *num_tests) {
    int failures = 0;

    *num_tests = (int)NUM_MACRO_TESTS;

    printf("\nConvenience Macro Tests:\n");

    for (size_t i = 0; i < NUM_MACRO_TESTS; i++) {
        MacroTest *test = &macro_tests[i];

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

# Serialization

Text-based serialization using an S-expression format: scalar helpers for
ints, doubles, and strings, plus a full tree API (`SExp`) that can parse,
build, compare, and serialize arbitrarily nested lists. Defined in
`<cyan/serialize.h>`.

```c
#include <cyan/serialize.h>

i32 main(void) {
    // === Serialization ===
    char *int_str = serialize_int(42);        // "42"
    char *dbl_str = serialize_double(3.14);   // "%.17g" format, so
                                              // "3.1400000000000001"
    char *str_str = serialize_string("hello\nworld");  // "\"hello\\nworld\""
    
    // Generic serialize macro (uses _Generic)
    char *s1 = serialize(42);       // Uses serialize_int
    char *s2 = serialize(3.14);     // Uses serialize_double
    char *s3 = serialize("hello");  // Uses serialize_string
    
    // === Parsing ===
    const char *end;
    
    // Parse integer
    Result_int_ParseError int_res = parse_int("42 rest", &end);
    if (is_ok(int_res)) {
        printf("Parsed: %d\n", unwrap_ok(int_res));
        // end points to " rest"
    }
    
    // Parse double (handles nan, inf, -inf)
    Result_double_ParseError dbl_res = parse_double("3.14", NULL);
    
    // Parse quoted string (handles escape sequences)
    Result_ParsedString_ParseError str_res = parse_string("\"hello\\nworld\"", &end);
    if (is_ok(str_res)) {
        char *parsed = unwrap_ok(str_res);
        printf("Parsed: %s\n", parsed);  // "hello\nworld"
        free(parsed);  // Caller must free
    }
    
    // === Pretty Printing ===
    char *pretty = pretty_print("(1 2 (3 4) 5)", 2);
    printf("%s\n", pretty);
    // Output:
    // (
    //   1
    //   2
    //   (
    //     3
    //     4
    //   )
    //   5
    // )
    
    free(int_str);
    free(dbl_str);
    free(str_str);
    free(s1);
    free(s2);
    free(s3);
    free(pretty);
    return 0;
}
```

## S-expression trees

```c
#include <cyan/serialize.h>

i32 main(void) {
    // === Parsing ===
    Result_SExpPtr_ParseError r = parse_sexp("(add 1 2.5 \"three\" (nested list))", NULL);
    if (is_ok(r)) {
        SExp *e = unwrap_ok(r);
        // e->type is one of SEXP_INT, SEXP_DOUBLE, SEXP_STRING,
        // SEXP_SYMBOL, SEXP_LIST
        // Access: e->i (long), e->d (double), e->str (string/symbol),
        //         e->list.items / e->list.len (list)
        printf("List with %zu items\n", e->list.len);
        sexp_free(e);  // Recursively frees the whole tree
    }
    
    // === Building trees programmatically ===
    SExp *list = sexp_list_new();
    sexp_list_push(list, sexp_symbol("point"));  // push takes ownership
    sexp_list_push(list, sexp_int(3));
    sexp_list_push(list, sexp_double(1.5));
    sexp_list_push(list, sexp_string("label"));
    
    // === Serializing ===
    char *text = serialize_sexp(list);  // "(point 3 1.5 \"label\")"
    printf("%s\n", text);
    
    // === Round-trip guarantee ===
    Result_SExpPtr_ParseError back = parse_sexp(text, NULL);
    // sexp_eq is structural equality (NaN == NaN is true here)
    assert(sexp_eq(unwrap_ok(back), list));
    
    sexp_free(unwrap_ok(back));
    sexp_free(list);
    free(text);
    return 0;
}
```

`parse_sexp` rejects input nested deeper than `CYAN_SEXP_MAX_DEPTH`
(default 1000, overridable before including headers).

## Grammar

```
value    := atom | list
atom     := number | string | symbol
number   := ['-'] digit+ ['.' digit+]
string   := '"' char* '"'
symbol   := alpha (alpha | digit | '_')*
list     := '(' value* ')'
```

`parse_sexp` handles every production, including symbols and arbitrarily
nested lists.

## API reference

| Function | Description |
|----------|-------------|
| `serialize_int(val)` | Serialize integer to string |
| `serialize_long(val)` | Serialize long to string |
| `serialize_float(val)` | Serialize float to string |
| `serialize_double(val)` | Serialize double to string |
| `serialize_string(str)` | Serialize string with escaping |
| `serialize(val)` | Generic serialize (auto-selects type) |
| `parse_int(input, end)` | Parse integer, return Result |
| `parse_double(input, end)` | Parse double, return Result |
| `parse_string(input, end)` | Parse quoted string, return Result |
| `pretty_print(str, indent)` | Format with indentation |
| `skip_whitespace(input)` | Advance past leading whitespace, return the new position |
| `parse_sexp(input, end)` | Parse full S-expression tree, return `Result_SExpPtr_ParseError` |
| `serialize_sexp(e)` | Serialize tree back to text (caller frees) |
| `sexp_int(v)` / `sexp_double(v)` / `sexp_string(s)` / `sexp_symbol(s)` | Construct atom nodes |
| `sexp_list_new()` | Construct empty list node |
| `sexp_list_push(list, child)` | Append child to list (takes ownership) |
| `sexp_eq(a, b)` | Structural equality (NaN == NaN is true) |
| `sexp_free(e)` | Recursively free a tree |

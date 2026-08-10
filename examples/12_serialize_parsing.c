/**
 * @file 12_serialize_parsing.c
 * @brief Example demonstrating serialization and parsing of primitive types
 * 
 * This example shows how to serialize values to text format and parse
 * them back, with proper error handling using Result types.
 */

#include <cyan/cyan.h>
#include <stdio.h>

int main(void) {
    printf("=== Serialization and Parsing Example ===\n\n");
    
    /* --------------------------------------------------------
     * 1. Serializing integers
     * -------------------------------------------------------- */
    printf("1. Serializing integers\n");
    
    char *s1 = serialize_int(42);
    char *s2 = serialize_int(-1234);
    char *s3 = serialize_int(0);
    
    printf("   42 -> \"%s\"\n", s1);
    printf("   -1234 -> \"%s\"\n", s2);
    printf("   0 -> \"%s\"\n\n", s3);
    
    free(s1);
    free(s2);
    free(s3);
    
    /* --------------------------------------------------------
     * 2. Serializing doubles
     * -------------------------------------------------------- */
    printf("2. Serializing doubles\n");
    
    char *d1 = serialize_double(3.14159);
    char *d2 = serialize_double(-0.001);
    char *d3 = serialize_double(1.0e10);
    
    printf("   3.14159 -> \"%s\"\n", d1);
    printf("   -0.001 -> \"%s\"\n", d2);
    printf("   1.0e10 -> \"%s\"\n\n", d3);
    
    free(d1);
    free(d2);
    free(d3);
    
    /* --------------------------------------------------------
     * 3. Serializing strings with escaping
     * -------------------------------------------------------- */
    printf("3. Serializing strings (with escaping)\n");
    
    char *str1 = serialize_string("Hello, World!");
    char *str2 = serialize_string("Line1\nLine2");
    char *str3 = serialize_string("Tab\there");
    char *str4 = serialize_string("Quote: \"test\"");
    
    printf("   Hello, World! -> %s\n", str1);
    printf("   Line1\\nLine2 -> %s\n", str2);
    printf("   Tab\\there -> %s\n", str3);
    printf("   Quote: \"test\" -> %s\n\n", str4);
    
    free(str1);
    free(str2);
    free(str3);
    free(str4);
    
    /* --------------------------------------------------------
     * 4. Parsing integers
     * -------------------------------------------------------- */
    printf("4. Parsing integers\n");
    
    const char *end;
    Result_int_ParseError r1 = parse_int("42", NULL);
    if (is_ok(r1)) {
        printf("   \"42\" -> %d\n", unwrap_ok(r1));
    }
    
    Result_int_ParseError r2 = parse_int("  -100 extra", &end);
    if (is_ok(r2)) {
        printf("   \"  -100 extra\" -> %d (remaining: \"%s\")\n", unwrap_ok(r2), end);
    }
    
    Result_int_ParseError r3 = parse_int("not a number", NULL);
    if (is_err(r3)) {
        printf("   \"not a number\" -> Error: %s\n\n", unwrap_err(r3));
    }
    
    /* --------------------------------------------------------
     * 5. Parsing doubles
     * -------------------------------------------------------- */
    printf("5. Parsing doubles\n");
    
    Result_double_ParseError rd1 = parse_double("3.14159", NULL);
    if (is_ok(rd1)) {
        printf("   \"3.14159\" -> %f\n", unwrap_ok(rd1));
    }
    
    Result_double_ParseError rd2 = parse_double("-2.5e-3", NULL);
    if (is_ok(rd2)) {
        printf("   \"-2.5e-3\" -> %g\n", unwrap_ok(rd2));
    }
    
    /* Special values */
    Result_double_ParseError rd3 = parse_double("inf", NULL);
    Result_double_ParseError rd4 = parse_double("-inf", NULL);
    Result_double_ParseError rd5 = parse_double("nan", NULL);
    
    printf("   \"inf\" -> %f\n", unwrap_ok(rd3));
    printf("   \"-inf\" -> %f\n", unwrap_ok(rd4));
    printf("   \"nan\" -> %f\n\n", unwrap_ok(rd5));
    
    /* --------------------------------------------------------
     * 6. Parsing quoted strings
     * -------------------------------------------------------- */
    printf("6. Parsing quoted strings\n");
    
    Result_ParsedString_ParseError rs1 = parse_string("\"Hello\"", NULL);
    if (is_ok(rs1)) {
        char *parsed = unwrap_ok(rs1);
        printf("   '\"Hello\"' -> \"%s\"\n", parsed);
        free(parsed);
    }
    
    Result_ParsedString_ParseError rs2 = parse_string("\"Line1\\nLine2\"", NULL);
    if (is_ok(rs2)) {
        char *parsed = unwrap_ok(rs2);
        printf("   '\"Line1\\\\nLine2\"' -> \"%s\" (with actual newline)\n", parsed);
        free(parsed);
    }
    
    Result_ParsedString_ParseError rs3 = parse_string("\"Tab\\there\"", NULL);
    if (is_ok(rs3)) {
        char *parsed = unwrap_ok(rs3);
        printf("   '\"Tab\\\\there\"' -> \"%s\"\n", parsed);
        free(parsed);
    }
    
    /* Error case */
    Result_ParsedString_ParseError rs4 = parse_string("no quotes", NULL);
    if (is_err(rs4)) {
        printf("   \"no quotes\" -> Error: %s\n\n", unwrap_err(rs4));
    }
    
    /* --------------------------------------------------------
     * 7. Round-trip serialization
     * -------------------------------------------------------- */
    printf("7. Round-trip serialization (serialize then parse)\n");
    
    int original_int = 12345;
    char *serialized = serialize_int(original_int);
    Result_int_ParseError parsed_back = parse_int(serialized, NULL);
    
    printf("   Original: %d\n", original_int);
    printf("   Serialized: \"%s\"\n", serialized);
    printf("   Parsed back: %d\n", unwrap_ok(parsed_back));
    printf("   Round-trip success: %s\n\n", 
           original_int == unwrap_ok(parsed_back) ? "yes" : "no");
    
    free(serialized);
    
    /* --------------------------------------------------------
     * 8. Using generic serialize macro
     * -------------------------------------------------------- */
    printf("8. Using generic serialize() macro\n");
    
    char *g1 = serialize(42);
    char *g2 = serialize(3.14);
    char *g3 = serialize("hello");
    
    printf("   serialize(42) -> \"%s\"\n", g1);
    printf("   serialize(3.14) -> \"%s\"\n", g2);
    printf("   serialize(\"hello\") -> %s\n\n", g3);
    
    free(g1);
    free(g2);
    free(g3);
    
    /* --------------------------------------------------------
     * 9. S-expression trees (parse, walk, serialize, pretty print)
     * -------------------------------------------------------- */
    printf("9. S-expression trees\n");

    const char *sexpr = "(1 2 (3 4) 5)";
    printf("   Input: %s\n", sexpr);

    /* Parse the literal into an SExp tree */
    Result_SExpPtr_ParseError parsed_sexp = parse_sexp(sexpr, NULL);
    if (is_ok(parsed_sexp)) {
        SExp *tree = unwrap_ok(parsed_sexp);

        /* Walk the tree: the top level is a list of 4 items */
        printf("   Top-level list length: %zu\n", tree->list.len);
        printf("   First atom: %ld\n", tree->list.items[0]->i);
        SExp *nested = tree->list.items[2];
        printf("   Nested list length: %zu (first nested atom: %ld)\n",
               nested->list.len, nested->list.items[0]->i);

        /* Serialize the tree back to text */
        char *round = serialize_sexp(tree);
        printf("   Serialized back: %s\n", round);

        /* Round-trip: parse(serialize(x)) is structurally equal to x */
        Result_SExpPtr_ParseError reparsed = parse_sexp(round, NULL);
        if (is_ok(reparsed)) {
            SExp *tree2 = unwrap_ok(reparsed);
            printf("   Round-trip sexp_eq: %s\n",
                   sexp_eq(tree, tree2) ? "true" : "false");
            sexp_free(tree2);
        }

        /* Pretty print the serialized form */
        char *pretty = pretty_print(round, 2);
        printf("   Pretty printed:\n%s\n\n", pretty);

        free(pretty);
        free(round);
        sexp_free(tree);
    } else {
        printf("   Parse error: %s\n\n", unwrap_err(parsed_sexp));
    }
    
    /* --------------------------------------------------------
     * 10. Putting it together: a config round-trip as an SExp tree
     * -------------------------------------------------------- */
    printf("10. Putting it together: config round-trip via SExp\n");

    /* Build (config (name "editor") (width 1280) (scale 1.5)) in code */
    SExp *config = sexp_list_new();
    sexp_list_push(config, sexp_symbol("config"));

    SExp *name_pair = sexp_list_new();
    sexp_list_push(name_pair, sexp_symbol("name"));
    sexp_list_push(name_pair, sexp_string("editor"));
    sexp_list_push(config, name_pair);

    SExp *width_pair = sexp_list_new();
    sexp_list_push(width_pair, sexp_symbol("width"));
    sexp_list_push(width_pair, sexp_int(1280));
    sexp_list_push(config, width_pair);

    SExp *scale_pair = sexp_list_new();
    sexp_list_push(scale_pair, sexp_symbol("scale"));
    sexp_list_push(scale_pair, sexp_double(1.5));
    sexp_list_push(config, scale_pair);

    /* Serialize: this is the text you would write to a config file */
    char *saved = serialize_sexp(config);
    printf("   saved form: %s\n", saved);

    /* Load it back and verify nothing was lost */
    Result_SExpPtr_ParseError loaded = parse_sexp(saved, NULL);
    if (is_ok(loaded)) {
        SExp *cfg = unwrap_ok(loaded);
        printf("   round-trip sexp_eq: %s\n", sexp_eq(config, cfg) ? "yes" : "no");

        /* Read settings back out of the tree: (key value) pairs after the tag */
        for (size_t i = 1; i < cfg->list.len; i++) {
            SExp *pair = cfg->list.items[i];
            SExp *key = pair->list.items[0];
            SExp *val = pair->list.items[1];
            printf("   %-6s = ", key->str);
            switch (val->type) {
                case SEXP_INT:    printf("%ld\n", val->i); break;
                case SEXP_DOUBLE: printf("%g\n", val->d); break;
                case SEXP_STRING: printf("\"%s\"\n", val->str); break;
                default:          printf("(other)\n"); break;
            }
        }
        sexp_free(cfg);
    } else {
        printf("   reload failed: %s\n", unwrap_err(loaded));
    }

    free(saved);
    sexp_free(config);
    printf("\n");

    printf("=== Serialization example complete ===\n");
    return 0;
}

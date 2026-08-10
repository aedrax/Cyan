/**
 * @file 18_result_pipeline.c
 * @brief Example: a realistic Result-based pipeline that parses a config file
 *
 * Pipeline: raw text -> lines (string_split_next) -> key/value pairs ->
 * typed values (parse_int via try_ok helpers) -> a validated struct.
 * Errors are reported at the top level with map_err + match_result.
 */

#include <cyan/cyan.h>
#include <stdio.h>
#include <string.h>

/* The typed configuration we want out of the text */
typedef struct {
    int width;
    int height;
    int fullscreen;
} DisplayConfig;

/* ParseError (a const char * alias) and Result_int_ParseError come from
 * serialize.h; we only add the Result for our own struct */
RESULT_DEFINE(DisplayConfig, ParseError);

/*============================================================================
 * 1. Field helpers - each step returns a Result and composes with try_ok
 *============================================================================*/

/* Materialize a slice into a stack buffer so parse_int can read it */
static Result_int_ParseError parse_int_field(Slice_char value) {
    char buf[32];
    if (value.len == 0) return Err(int, ParseError, "empty value");
    if (value.len >= sizeof(buf)) return Err(int, ParseError, "value too long");
    memcpy(buf, value.data, value.len);
    buf[value.len] = '\0';

    const char *end;
    int v = try_ok(parse_int(buf, &end));   /* propagates parse errors */
    if (*end != '\0') return Err(int, ParseError, "trailing junk after number");
    return Ok(int, ParseError, v);
}

static Result_int_ParseError require_positive(int v) {
    if (v <= 0) return Err(int, ParseError, "must be positive");
    return Ok(int, ParseError, v);
}

/* Parse + validate in one chained step */
static Result_int_ParseError parse_dimension(Slice_char value) {
    return and_then_result(parse_int_field(value), int, ParseError, require_positive);
}

static Result_int_ParseError parse_bool_field(Slice_char value) {
    if (string_slice_eq(value, "on"))  return Ok(int, ParseError, 1);
    if (string_slice_eq(value, "off")) return Ok(int, ParseError, 0);
    return Err(int, ParseError, "expected 'on' or 'off'");
}

/*============================================================================
 * 2. The pipeline - text to typed config, first error wins
 *============================================================================*/

static Result_DisplayConfig_ParseError parse_config(const char *text) {
    DisplayConfig cfg = { .width = 0, .height = 0, .fullscreen = 0 };

    String source = string_from(text);
    Slice_char rest = string_as_slice(&source);
    Slice_char line;
    while (string_split_next(&rest, '\n', &line)) {
        if (line.len == 0) continue;   /* skip blank lines */

        /* Split "key=value": the first piece is the key, the un-consumed
         * remainder (which may itself contain '=') is the value */
        Slice_char kv = line;
        Slice_char key;
        string_split_next(&kv, '=', &key);
        if (!kv.data) {
            string_free(&source);
            return Err(DisplayConfig, ParseError, "line is missing '='");
        }
        Slice_char value = kv;

        Result_int_ParseError field;
        int *target;
        if (string_slice_eq(key, "width")) {
            field = parse_dimension(value);
            target = &cfg.width;
        } else if (string_slice_eq(key, "height")) {
            field = parse_dimension(value);
            target = &cfg.height;
        } else if (string_slice_eq(key, "fullscreen")) {
            field = parse_bool_field(value);
            target = &cfg.fullscreen;
        } else {
            string_free(&source);
            return Err(DisplayConfig, ParseError, "unknown key");
        }

        if (is_err(field)) {
            string_free(&source);
            return Err(DisplayConfig, ParseError, unwrap_err(field));
        }
        *target = unwrap_ok(field);
    }
    string_free(&source);

    if (cfg.width == 0 || cfg.height == 0) {
        return Err(DisplayConfig, ParseError, "width and height are required");
    }
    return Ok(DisplayConfig, ParseError, cfg);
}

/*============================================================================
 * 3. Error decoration - map_err rewrites terse errors for end users
 *============================================================================*/

static ParseError friendly_error(ParseError e) {
    static const struct {
        const char *terse;
        const char *friendly;
    } table[] = {
        { "expected 'on' or 'off'",
          "config error: fullscreen must be 'on' or 'off'" },
        { "must be positive",
          "config error: dimensions must be positive numbers" },
        { "width and height are required",
          "config error: both width= and height= must be set" },
        { "unknown key",
          "config error: unrecognized setting name" },
    };
    for (size_t i = 0; i < sizeof(table) / sizeof(table[0]); i++) {
        if (strcmp(e, table[i].terse) == 0) return table[i].friendly;
    }
    return e;   /* already descriptive (e.g. parse_int's own messages) */
}

/*============================================================================
 * Main
 *============================================================================*/

int main(void) {
    printf("=== Result Pipeline Example: Parsing a Config File ===\n\n");

    /* --------------------------------------------------------
     * 1. A well-formed config parses into the typed struct
     * -------------------------------------------------------- */
    printf("1. Parsing a valid config\n");
    const char *good =
        "width=1280\n"
        "height=720\n"
        "fullscreen=on\n";
    printf("   input: width=1280 | height=720 | fullscreen=on\n");

    Result_DisplayConfig_ParseError parsed = parse_config(good);
    match_result(parsed, DisplayConfig, ParseError, cfg, err,
        {
            printf("   parsed: %dx%d, fullscreen=%s\n\n",
                   cfg.width, cfg.height, cfg.fullscreen ? "on" : "off");
        },
        { printf("   failed: %s\n\n", err); }
    );

    /* --------------------------------------------------------
     * 2. Broken configs: the first error propagates to the top
     * -------------------------------------------------------- */
    printf("2. Parsing broken configs (first error wins)\n");
    const char *broken[] = {
        "width=1280\nheight=abc\n",                    /* not a number */
        "width=-5\nheight=720\n",                      /* fails validation */
        "width=1280\nheight=720\nfullscreen=maybe\n",  /* bad boolean */
        "width=1280\n",                                /* missing field */
        "resolution=big\n",                            /* unknown key */
    };
    for (size_t i = 0; i < 5; i++) {
        Result_DisplayConfig_ParseError r = parse_config(broken[i]);
        /* map_err rewrites the terse internal error for end users */
        Result_DisplayConfig_ParseError reported =
            map_err(r, DisplayConfig, ParseError, friendly_error);
        match_result(reported, DisplayConfig, ParseError, cfg, err,
            { printf("   config %zu: unexpectedly ok (%dx%d)\n", i, cfg.width, cfg.height); },
            { printf("   config %zu: %s\n", i, err); }
        );
    }
    printf("\n");

    /* --------------------------------------------------------
     * 3. expect_ok for configs that must parse (built-in defaults)
     * -------------------------------------------------------- */
    printf("3. Built-in defaults with expect_ok\n");
    DisplayConfig defaults = expect_ok(parse_config("width=800\nheight=600\n"),
                                       "built-in default config must parse");
    printf("   defaults: %dx%d, fullscreen=%s\n",
           defaults.width, defaults.height, defaults.fullscreen ? "on" : "off");

    printf("\n=== Pipeline example complete ===\n");
    return 0;
}

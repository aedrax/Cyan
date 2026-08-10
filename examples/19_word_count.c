/**
 * @file 19_word_count.c
 * @brief Capstone example: word frequency count across several Cyan types
 *
 * Pipeline: raw text -> normalize into a String (lowercase, strip
 * punctuation) -> split into word slices (string_split_next) -> count with
 * a content-hashed HashMap_str_int -> collect (word, count) pairs into a
 * Vec -> sort by count descending -> print the top 5.
 *
 * HashMap_str_int has no public iterator, so distinct words are collected
 * into the Vec while counting (on first sighting) using only public API.
 */

#include <cyan/cyan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* A word paired with its final count, for sorting and display */
typedef struct {
    char *word;   /* owned copy, freed at the end */
    int count;
} WordCount;

OPTION_DEFINE(int);            /* required by HASHMAP_STR_DEFINE(int) */
OPTION_DEFINE(WordCount);      /* required by VECTOR_DEFINE(WordCount) */
VECTOR_DEFINE(WordCount);
HASHMAP_STR_DEFINE(int);

/* qsort-style comparator: count descending, ties alphabetical */
static int cmp_wordcount(const void *a, const void *b) {
    const WordCount *x = (const WordCount *)a;
    const WordCount *y = (const WordCount *)b;
    if (x->count != y->count) return y->count - x->count;
    return strcmp(x->word, y->word);
}

static char *copy_word(const char *s) {
    size_t n = strlen(s) + 1;
    char *out = (char *)malloc(n);
    if (out) memcpy(out, s, n);
    return out;
}

int main(void) {
    printf("=== Word Count Capstone ===\n\n");

    const char *text =
        "The quick brown fox jumps over the lazy dog. "
        "The dog barks, and the fox runs. A quick fox is a happy fox.";

    /* --------------------------------------------------------
     * 1. Normalize: lowercase, punctuation -> spaces
     * -------------------------------------------------------- */
    printf("1. Normalizing the text\n");
    String normalized = string_new();
    for (const char *p = text; *p; p++) {
        unsigned char c = (unsigned char)*p;
        string_push(&normalized, isalpha(c) ? (char)tolower(c) : ' ');
    }
    printf("   \"%.48s...\"\n\n", string_cstr(&normalized));

    /* --------------------------------------------------------
     * 2. Split into words and count with a string-keyed map
     * -------------------------------------------------------- */
    printf("2. Counting words with HashMap_str_int\n");
    HashMap_str_int counts = hashmap_str_int_new();
    Vec_WordCount stats = vec_WordCount_new();   /* distinct words */

    Slice_char rest = string_as_slice(&normalized);
    Slice_char token;
    size_t total_words = 0;
    while (string_split_next(&rest, ' ', &token)) {
        if (token.len == 0) continue;   /* collapsed spaces/punctuation */
        String word = string_from_slice(token);
        total_words++;

        int seen = unwrap_or(hashmap_str_int_get(&counts, string_cstr(&word)), 0);
        hashmap_str_int_insert(&counts, string_cstr(&word), seen + 1);
        if (seen == 0) {
            /* First sighting: remember the word for the ranking phase */
            WordCount wc = { .word = copy_word(string_cstr(&word)), .count = 0 };
            if (wc.word) VEC_PUSH(WordCount, stats, wc);
        }
        string_free(&word);
    }
    printf("   %zu words total, %zu distinct\n\n",
           total_words, hashmap_str_int_len(&counts));

    /* --------------------------------------------------------
     * 3. Copy the final counts into the vector
     * -------------------------------------------------------- */
    printf("3. Collecting (word, count) pairs into a Vec\n");
    VEC_FOREACH(WordCount, stats, it) {
        it->count = unwrap_or(hashmap_str_int_get(&counts, it->word), 0);
    }
    printf("   %zu pairs collected\n\n", VEC_LEN(WordCount, stats));

    /* --------------------------------------------------------
     * 4. Sort by count (descending) and print the top 5
     * -------------------------------------------------------- */
    printf("4. Top 5 words\n");
    VEC_SORT(WordCount, stats, cmp_wordcount);
    size_t top = VEC_LEN(WordCount, stats) < 5 ? VEC_LEN(WordCount, stats) : 5;
    for (size_t i = 0; i < top; i++) {
        WordCount wc = unwrap(VEC_GET(WordCount, stats, i));
        printf("   %zu. %-8s %d\n", i + 1, wc.word, wc.count);
    }

    /* --------------------------------------------------------
     * Cleanup: the map frees its own key copies; we free ours
     * -------------------------------------------------------- */
    VEC_FOREACH(WordCount, stats, it) {
        free(it->word);
    }
    VEC_FREE(WordCount, stats);
    hashmap_str_int_free(&counts);
    string_free(&normalized);

    printf("\n=== Capstone complete ===\n");
    return 0;
}

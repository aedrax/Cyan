/**
 * @file 11_hashmap_dictionary.c
 * @brief Example demonstrating type-safe hash maps (dictionaries)
 * 
 * This example shows how to use hash maps for key-value storage with
 * automatic resizing, collision handling, and iteration support.
 */

#include <cyan/cyan.h>
#include <stdio.h>

/* Define required types first */
OPTION_DEFINE(int);                 /* Required for Option_int */
HASHMAP_DEFINE(int, int);           /* int -> int mapping */
HASHMAP_ITER_DEFINE(int, int);      /* Iterator for int -> int */

int main(void) {
    printf("=== HashMap Dictionary Example ===\n\n");
    
    /* --------------------------------------------------------
     * 1. Creating a hash map
     * -------------------------------------------------------- */
    printf("1. Creating a hash map\n");
    HashMap_int_int scores = hashmap_int_int_new();
    printf("   Empty map created, length: %zu\n\n", hashmap_int_int_len(&scores));
    
    /* --------------------------------------------------------
     * 2. Inserting key-value pairs
     * -------------------------------------------------------- */
    printf("2. Inserting key-value pairs\n");
    
    /* Player IDs -> Scores */
    hashmap_int_int_insert(&scores, 101, 2500);
    hashmap_int_int_insert(&scores, 102, 1800);
    hashmap_int_int_insert(&scores, 103, 3200);
    hashmap_int_int_insert(&scores, 104, 2100);
    
    printf("   Inserted 4 player scores\n");
    printf("   Map length: %zu\n\n", hashmap_int_int_len(&scores));
    
    /* --------------------------------------------------------
     * 3. Looking up values
     * -------------------------------------------------------- */
    printf("3. Looking up values\n");
    
    Option_int score = hashmap_int_int_get(&scores, 101);
    if (is_some(score)) {
        printf("   Player 101 score: %d\n", unwrap(score));
    }
    
    score = hashmap_int_int_get(&scores, 103);
    if (is_some(score)) {
        printf("   Player 103 score: %d\n", unwrap(score));
    }
    
    /* Non-existent key */
    score = hashmap_int_int_get(&scores, 999);
    printf("   Player 999 exists: %s\n\n", is_some(score) ? "yes" : "no");
    
    /* --------------------------------------------------------
     * 4. Checking key existence
     * -------------------------------------------------------- */
    printf("4. Checking key existence\n");
    printf("   Contains key 102: %s\n", 
           hashmap_int_int_contains(&scores, 102) ? "yes" : "no");
    printf("   Contains key 999: %s\n\n", 
           hashmap_int_int_contains(&scores, 999) ? "yes" : "no");
    
    /* --------------------------------------------------------
     * 5. Updating values
     * -------------------------------------------------------- */
    printf("5. Updating values (insert overwrites)\n");
    printf("   Player 101 old score: %d\n", unwrap(hashmap_int_int_get(&scores, 101)));
    
    hashmap_int_int_insert(&scores, 101, 2750);  /* Update score */
    printf("   Player 101 new score: %d\n\n", unwrap(hashmap_int_int_get(&scores, 101)));
    
    /* --------------------------------------------------------
     * 6. Removing entries
     * -------------------------------------------------------- */
    printf("6. Removing entries\n");
    printf("   Map length before: %zu\n", hashmap_int_int_len(&scores));
    
    Option_int removed = hashmap_int_int_remove(&scores, 104);
    if (is_some(removed)) {
        printf("   Removed player 104 with score: %d\n", unwrap(removed));
    }
    
    printf("   Map length after: %zu\n", hashmap_int_int_len(&scores));
    printf("   Contains 104 now: %s\n\n", 
           hashmap_int_int_contains(&scores, 104) ? "yes" : "no");
    
    /* --------------------------------------------------------
     * 7. Iterating over entries
     * -------------------------------------------------------- */
    printf("7. Iterating over all entries\n");
    
    HashMapIter_int_int iter = hashmap_int_int_iter(&scores);
    Option_MapPair_int_int pair;
    
    while ((pair = hashmap_int_int_iter_next(&iter)).has_value) {
        printf("   Player %d -> Score %d\n", pair.value.key, pair.value.value);
    }
    printf("\n");
    
    /* --------------------------------------------------------
     * 8. Using vtable convenience macros
     * -------------------------------------------------------- */
    printf("8. Using vtable convenience macros\n");
    
    HashMap_int_int inventory = hashmap_int_int_new();
    
    MAP_INSERT(inventory, 1, 50);   /* Item 1: quantity 50 */
    MAP_INSERT(inventory, 2, 25);   /* Item 2: quantity 25 */
    MAP_INSERT(inventory, 3, 100);  /* Item 3: quantity 100 */
    
    printf("   Inventory length: %zu\n", MAP_LEN(inventory));
    printf("   Item 2 quantity: %d\n", unwrap(MAP_GET(inventory, 2)));
    printf("   Has item 3: %s\n", MAP_CONTAINS(inventory, 3) ? "yes" : "no");
    
    MAP_REMOVE(inventory, 1);
    printf("   After removing item 1, length: %zu\n\n", MAP_LEN(inventory));
    
    MAP_FREE(inventory);
    
    /* --------------------------------------------------------
     * 9. Pre-allocated capacity
     * -------------------------------------------------------- */
    printf("9. Creating map with pre-allocated capacity\n");
    HashMap_int_int large_map = hashmap_int_int_with_capacity(1000);
    printf("   Created map with capacity for ~1000 entries\n");
    
    /* Add many entries efficiently */
    for (int i = 0; i < 100; i++) {
        hashmap_int_int_insert(&large_map, i, i * i);
    }
    printf("   Added 100 entries, length: %zu\n\n", hashmap_int_int_len(&large_map));
    
    hashmap_int_int_free(&large_map);
    
    /* --------------------------------------------------------
     * Cleanup
     * -------------------------------------------------------- */
    hashmap_int_int_free(&scores);
    
    printf("=== HashMap example complete ===\n");
    return 0;
}

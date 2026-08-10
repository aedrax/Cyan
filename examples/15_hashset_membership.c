/**
 * @file 15_hashset_membership.c
 * @brief Example demonstrating hash sets for fast membership testing
 *
 * This example shows how to use HashSet for deduplication, membership
 * queries, iteration with SET_FOREACH, tombstone reuse after removals,
 * and manual set algebra (intersection).
 */

#include <cyan/cyan.h>
#include <stdio.h>

/* Define the set type and its iterator (SET_FOREACH requires the iterator) */
HASHSET_DEFINE(int);
HASHSET_ITER_DEFINE(int);

int main(void) {
    printf("=== HashSet Membership Example ===\n\n");

    /* --------------------------------------------------------
     * 1. Creating a set and adding elements
     * -------------------------------------------------------- */
    printf("1. Creating a set and adding elements\n");
    HashSet_int seen = hashset_int_new();
    printf("   Empty set, len: %zu\n", hashset_int_len(&seen));

    printf("   add(7)  newly added: %s\n", hashset_int_add(&seen, 7) ? "yes" : "no");
    printf("   add(11) newly added: %s\n", hashset_int_add(&seen, 11) ? "yes" : "no");
    printf("   add(7)  again:       %s (duplicates are rejected)\n",
           hashset_int_add(&seen, 7) ? "yes" : "no");
    printf("   len: %zu\n\n", hashset_int_len(&seen));

    /* --------------------------------------------------------
     * 2. Membership tests
     * -------------------------------------------------------- */
    printf("2. Membership tests\n");
    printf("   contains(7):  %s\n", hashset_int_contains(&seen, 7) ? "yes" : "no");
    printf("   contains(42): %s\n\n", hashset_int_contains(&seen, 42) ? "yes" : "no");

    /* --------------------------------------------------------
     * 3. Removing elements (tombstones)
     * -------------------------------------------------------- */
    printf("3. Removing elements\n");
    printf("   remove(7) was present: %s\n", hashset_int_remove(&seen, 7) ? "yes" : "no");
    printf("   remove(7) again:       %s\n", hashset_int_remove(&seen, 7) ? "yes" : "no");
    printf("   contains(7) now:       %s\n", hashset_int_contains(&seen, 7) ? "yes" : "no");

    /* Removal leaves a tombstone in the bucket so probing still works;
     * a later add can reuse that slot - the table does not need to grow. */
    printf("   add(7) back (reuses the tombstone slot): %s\n",
           hashset_int_add(&seen, 7) ? "yes" : "no");
    printf("   len: %zu\n\n", hashset_int_len(&seen));

    hashset_int_free(&seen);

    /* --------------------------------------------------------
     * 4. Deduplicating a stream of values
     * -------------------------------------------------------- */
    printf("4. Deduplicating an int stream\n");
    int stream[] = {4, 8, 15, 16, 23, 42, 8, 15, 4, 42, 108};
    size_t stream_len = sizeof(stream) / sizeof(stream[0]);

    HashSet_int uniques = hashset_int_new();
    printf("   stream: ");
    for (size_t i = 0; i < stream_len; i++) {
        /* SET_ADD returns true only for first sightings */
        bool fresh = SET_ADD(int, uniques, stream[i]);
        printf("%d%s ", stream[i], fresh ? "" : "(dup)");
    }
    printf("\n");
    printf("   %zu values in, %zu unique\n\n", stream_len, SET_LEN(int, uniques));

    /* --------------------------------------------------------
     * 5. Listing elements with SET_FOREACH
     * -------------------------------------------------------- */
    printf("5. Listing with SET_FOREACH (order is unspecified)\n");
    int item;
    printf("   uniques: ");
    SET_FOREACH(int, uniques, item) {
        printf("%d ", item);
    }
    printf("\n");

    /* The same traversal via the explicit iterator API */
    HashSetIter_int it = hashset_int_iter(&uniques);
    Option_SetItem_int next;
    size_t visited = 0;
    while ((next = hashset_int_iter_next(&it)).has_value) {
        visited++;
    }
    printf("   explicit iterator visited %zu elements\n\n", visited);

    /* --------------------------------------------------------
     * 6. Putting it together: intersection of two sets
     * -------------------------------------------------------- */
    printf("6. Putting it together: who is in BOTH groups?\n");

    /* Badge numbers seen at the front door and at the lab door */
    int front_door[] = {101, 102, 103, 105, 108};
    int lab_door[]   = {102, 104, 105, 109};

    HashSet_int front = hashset_int_new();
    HashSet_int lab = hashset_int_new();
    for (size_t i = 0; i < 5; i++) SET_ADD(int, front, front_door[i]);
    for (size_t i = 0; i < 4; i++) SET_ADD(int, lab, lab_door[i]);

    /* Manual intersection: iterate one set, test membership in the other */
    HashSet_int both = hashset_int_new();
    SET_FOREACH(int, front, item) {
        if (SET_CONTAINS(int, lab, item)) {
            SET_ADD(int, both, item);
        }
    }

    printf("   front door: %zu badges, lab door: %zu badges\n",
           SET_LEN(int, front), SET_LEN(int, lab));
    printf("   badges seen at both doors (%zu): ", SET_LEN(int, both));
    SET_FOREACH(int, both, item) {
        printf("%d ", item);
    }
    printf("\n");

    /* Badge 105 is revoked: drop it from the intersection via the macro */
    printf("   SET_REMOVE(105) was present: %s, remaining in both: %zu\n",
           SET_REMOVE(int, both, 105) ? "yes" : "no", SET_LEN(int, both));

    /* Cleanup */
    SET_FREE(int, both);
    SET_FREE(int, lab);
    SET_FREE(int, front);
    SET_FREE(int, uniques);

    printf("\n=== HashSet example complete ===\n");
    return 0;
}

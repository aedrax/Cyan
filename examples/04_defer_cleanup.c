/**
 * @file 04_defer_cleanup.c
 * @brief Example demonstrating defer for automatic resource cleanup
 * 
 * Compile: gcc -std=c11 -I../include -o defer_demo 04_defer_cleanup.c
 * Run: ./defer_demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cyan/common.h>
#include <cyan/defer.h>

// Simulated resource that needs cleanup
typedef struct {
    i32 id;
    char *name;
} Resource;

Resource *resource_create(i32 id, const char *name) {
    Resource *r = malloc(sizeof(Resource));
    if (!r) return NULL;
    r->id = id;
    r->name = malloc(strlen(name) + 1);
    if (!r->name) {
        free(r);
        return NULL;
    }
    strcpy(r->name, name);
    printf("   [Created resource %d: %s]\n", id, name);
    return r;
}

void resource_destroy(Resource *r) {
    if (r) {
        printf("   [Destroyed resource %d: %s]\n", r->id, r->name);
        free(r->name);
        free(r);
    }
}

// --- Resource pyramid with early returns (used in example 6) ---
// Acquire three resources in order; any failure returns early, and defer
// unwinds exactly the resources acquired so far, in LIFO order.
i32 start_service(bool fail_network) {
    Resource *db = resource_create(10, "Database");
    if (!db) return -1;
    defer({ resource_destroy(db); });

    Resource *net = fail_network ? NULL : resource_create(11, "Network");
    if (!net) {
        printf("   [Network unavailable - aborting startup]\n");
        return -1;  // db is still destroyed by its defer
    }
    defer({ resource_destroy(net); });

    Resource *cache = resource_create(12, "Cache");
    if (!cache) return -1;  // net and db unwound automatically
    defer({ resource_destroy(cache); });

    printf("   Service running with all three resources\n");
    return 0;  // cache, net, db destroyed in that order
}

i32 main(void) {
    printf("=== Defer Examples ===\n\n");
    
    // Example 1: Basic defer
    printf("1. Basic Defer:\n");
    {
        printf("   Entering scope\n");
        defer({ printf("   Deferred: cleanup executed!\n"); });
        printf("   Doing work...\n");
        printf("   Exiting scope\n");
    }
    printf("   After scope\n");
    
    // Example 2: Multiple defers (LIFO order)
    printf("\n2. Multiple Defers (LIFO order):\n");
    {
        defer({ printf("   Deferred 1 (first declared)\n"); });
        defer({ printf("   Deferred 2 (second declared)\n"); });
        defer({ printf("   Deferred 3 (third declared)\n"); });
        printf("   About to exit scope...\n");
    }
    
    // Example 3: Resource cleanup
    printf("\n3. Resource Cleanup:\n");
    {
        Resource *r1 = resource_create(1, "Database");
        if (!r1) return 1;
        defer({ resource_destroy(r1); });
        
        Resource *r2 = resource_create(2, "Network");
        if (!r2) return 1;  // r1 still cleaned up!
        defer({ resource_destroy(r2); });
        
        printf("   Using resources...\n");
        // Resources automatically cleaned up in reverse order
    }
    
    // Example 4: defer_free for simple allocations
    printf("\n4. defer_free for Memory:\n");
    {
        i32 *data = malloc(sizeof(i32) * 10);
        defer_free(data);
        
        for (i32 i = 0; i < 10; i++) {
            data[i] = i * i;
        }
        
        printf("   Squares: ");
        for (i32 i = 0; i < 10; i++) {
            printf("%d ", data[i]);
        }
        printf("\n");
        // data automatically freed
    }
    printf("   Memory freed automatically\n");
    
    // Example 5: Nested scopes
    printf("\n5. Nested Scopes:\n");
    {
        defer({ printf("   Outer scope cleanup\n"); });
        printf("   In outer scope\n");
        
        {
            defer({ printf("   Inner scope cleanup\n"); });
            printf("   In inner scope\n");
        }
        
        printf("   Back in outer scope\n");
    }
    
    // Example 6: Putting it together - resource pyramid with early returns
    printf("\n6. Putting It Together - Resource Pyramid:\n");
    printf("   -- successful startup --\n");
    i32 rc_ok = start_service(false);
    printf("   start_service returned %d\n", rc_ok);

    printf("   -- startup with network failure --\n");
    i32 rc_fail = start_service(true);
    printf("   start_service returned %d (only acquired resources were cleaned up)\n",
           rc_fail);

    printf("\n=== Done ===\n");
    return 0;
}

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include "group.h"


void 
print_grp(struct grp *G) {
    printf("Group(id=%u, p=%lu)\n", G->id, G->p);
}

bool 
grp_equal(struct grp *G1, struct grp *G2) {
    bool is_equal = (G1 -> p == G2 -> p) && (G1 -> id == G2 -> id);
    return is_equal;
}

struct elem 
elem_mult(struct elem *e1, struct elem *e2) {
    struct grp G = e1 -> group;
    assert(grp_equal(&(e1 -> group), &(e2 -> group)));
    struct elem new = { e1 -> group, ((e1 -> val) * (e2 -> val)) % (G.p)};
    return new;
}


/* testing */

int main(void) {
    struct grp G = {7, 1};
    print_grp(&G);
    struct grp H = {7, 1};
    print_grp(&H);
    struct elem e1 = {G, 4};
    struct elem e2 = {G, 3};
    printf("Group multiplication test: %lu \n", elem_mult(&e1,&e2).val);
}

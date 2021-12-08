#pragma once

struct grp {
    size_t p;           /* grp order */
    unsigned int id;    /* unique grp id a.k.a index */ 
};

struct elem {
    struct grp group;       /* which group this element is part of */
    size_t val;         /* generator power */
};

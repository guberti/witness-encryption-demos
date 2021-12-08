#pragma once

#include <gmp.h>

struct clt_elem_t {
    mpz_t elem;
    size_t level;
};

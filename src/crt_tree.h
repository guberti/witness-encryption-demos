#pragma once

#include "clt13.h"

#pragma GCC visibility push(hidden)

typedef struct crt_tree {
    size_t n;
    mpz_t mod;
    mpz_t crt_left;
    mpz_t crt_right;
    struct crt_tree *left;
    struct crt_tree *right;
} crt_tree;

crt_tree * crt_tree_new(mpz_t *const ps, size_t n);
void crt_tree_free(crt_tree *crt);

void crt_tree_do_crt(mpz_t rop, const crt_tree *crt, mpz_t *cs);
crt_tree * crt_tree_fread(FILE *const fp, size_t n);
void crt_tree_write(const char *fname, const crt_tree *const crt, size_t n);
int crt_tree_fwrite(FILE *const fp, const crt_tree *const crt, size_t n);

#pragma GCC visibility pop

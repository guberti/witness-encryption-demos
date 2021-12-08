#include "crt_tree.h"
#include "utils.h"

#include <assert.h>
#include <stdbool.h>

static crt_tree *
_crt_tree_new(mpz_t *ps, size_t n)
{
    crt_tree *crt;

    if (ps == NULL || n == 0)
        return NULL;

    crt = calloc(1, sizeof(crt_tree));
    crt->n = n;
    mpz_inits(crt->mod, crt->crt_left, crt->crt_right, NULL);
    if (crt->n == 1) {
        crt->left = NULL;
        crt->right = NULL;
        mpz_set(crt->mod, ps[0]);
    } else {
        mpz_t g;

        {
#pragma omp task
        {
            crt->left = _crt_tree_new(ps, crt->n / 2);
        }
#pragma omp task
        {
            crt->right = _crt_tree_new(ps + crt->n / 2, crt->n  - crt->n / 2);
        }
#pragma omp taskwait
        }

        if (!crt->left || !crt->right) {
            crt_tree_free(crt);
            return NULL;
        }

        mpz_init_set_ui(g, 0);
        mpz_gcdext(g, crt->crt_right, crt->crt_left, crt->left->mod, crt->right->mod);
        if (mpz_cmp_ui(g, 1) != 0) {
            crt_tree_free(crt);
            mpz_clear(g);
            return NULL;
        }
        mpz_clear(g);

        mpz_mul(crt->crt_left,  crt->crt_left,  crt->right->mod);
        mpz_mul(crt->crt_right, crt->crt_right, crt->left->mod);
        mpz_mul(crt->mod, crt->left->mod, crt->right->mod);
    }
    return crt;
}

crt_tree *
crt_tree_new(mpz_t *ps, size_t n)
{
    crt_tree *crt;
#pragma omp parallel default(shared)
#pragma omp single
    crt = _crt_tree_new(ps, n);
    return crt;
}

void
crt_tree_free(crt_tree *crt)
{
    if (crt->left)
        crt_tree_free(crt->left);
    if (crt->right)
        crt_tree_free(crt->right);
    mpz_clears(crt->mod, crt->crt_left, crt->crt_right, NULL);
    free(crt);
}

void
crt_tree_do_crt(mpz_t rop, const crt_tree *crt, mpz_t *cs)
{
    if (crt->n == 1) {
        mpz_set(rop, cs[0]);
    } else {
        mpz_t left, right, tmp;
        mpz_inits(left, right, tmp, NULL);

        crt_tree_do_crt(left,  crt->left,  cs);
        crt_tree_do_crt(right, crt->right, cs + crt->n / 2);

        mpz_mul(rop, left,  crt->crt_left);
        mpz_mul(tmp, right, crt->crt_right);
        mpz_add(rop, rop, tmp);
        mpz_mod(rop, rop, crt->mod);

        mpz_clears(left, right, tmp, NULL);
    }
}

static void
_crt_tree_get_leafs(mpz_t *leafs, int *i, const crt_tree *crt)
{
    if (crt->n == 1) {
        mpz_set(leafs[(*i)++], crt->mod);
    } else {
        _crt_tree_get_leafs(leafs, i, crt->left);
        _crt_tree_get_leafs(leafs, i, crt->right);
    }
}

crt_tree *
crt_tree_fread(FILE *fp, size_t n)
{
    crt_tree *crt = NULL;
    mpz_t *ps;

    ps = mpz_vector_new(n);
    if (mpz_vector_fread(ps, n, fp) == CLT_ERR)
        goto cleanup;
    if ((crt = crt_tree_new(ps, n)) == NULL)
        goto cleanup;

cleanup:
    mpz_vector_free(ps, n);
    return crt;
}

int
crt_tree_fwrite(FILE *fp, const crt_tree *crt, size_t n)
{
    int ret = CLT_ERR, ctr = 0;
    mpz_t *ps;

    ps = mpz_vector_new(n);
    _crt_tree_get_leafs(ps, &ctr, crt);
    if (mpz_vector_fwrite(ps, n, fp) == CLT_ERR)
        goto cleanup;
    ret = CLT_OK;
cleanup:
    mpz_vector_free(ps, n);
    return ret;
}

#include "clt13.h"
#include "clt_elem.h"
#include "utils.h"

#include <omp.h>

clt_elem_t *
clt_elem_new(void)
{
    clt_elem_t *e = calloc(1, sizeof e[0]);
    mpz_init(e->elem);
    e->level = 0;
    return e;
}

void
clt_elem_free(clt_elem_t *e)
{
    mpz_clear(e->elem);
    free(e);
}

clt_elem_t *
clt_elem_copy(clt_elem_t *a)
{
    clt_elem_t *e;
    e = calloc(1, sizeof e[0]);
    clt_elem_set(e, a);
    return e;
}

void
clt_elem_set(clt_elem_t *a, const clt_elem_t *b)
{
    mpz_set(a->elem, b->elem);
    a->level = b->level;
}

void
clt_elem_print(const clt_elem_t *a)
{
    gmp_printf("%lu | %Zd", a->level, a->elem);
}

int
clt_elem_fread(clt_elem_t *x, FILE *fp)
{
    if (mpz_fread(x->elem, fp) == CLT_ERR) return CLT_ERR;
    if (size_t_fread(fp, &x->level) == CLT_ERR) return CLT_ERR;
    return CLT_OK;
}

int
clt_elem_fwrite(clt_elem_t *x, FILE *fp)
{
    if (mpz_fwrite(x->elem, fp) == CLT_ERR) return CLT_ERR;
    if (size_t_fwrite(fp, x->level) == CLT_ERR) return CLT_ERR;
    return CLT_OK;
}

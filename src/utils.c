#include "clt13.h"
#include "utils.h"

#include <sys/time.h>

int
mpz_fread(mpz_t x, FILE *fp)
{
    if (mpz_inp_raw(x, fp) == 0)
        return CLT_ERR;
    return CLT_OK;
}

int
mpz_fwrite(mpz_t x, FILE *fp)
{
    if (mpz_out_raw(fp, x) == 0)
        return CLT_ERR;
    return CLT_OK;
}

mpz_t *
mpz_vector_new(size_t n)
{
    mpz_t *v;
    if ((v = calloc(n, sizeof v[0])) == NULL)
        return NULL;
    for (size_t i = 0; i < n; ++i)
        mpz_init(v[i]);
    return v;
}

void
mpz_vector_free(mpz_t *v, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        mpz_clear(v[i]);
    free(v);
}

int
mpz_vector_fread(mpz_t *m, size_t len, FILE *fp)
{
    for (size_t i = 0; i < len; ++i) {
        if (mpz_fread(m[i], fp) == CLT_ERR)
            return CLT_ERR;
    }
    return CLT_OK;
}

int
mpz_vector_fwrite(mpz_t *m, size_t len, FILE *fp)
{
    for (size_t i = 0; i < len; ++i) {
        if (mpz_fwrite(m[i], fp) == CLT_ERR)
            return CLT_ERR;
    }
    return CLT_OK;
}

double
current_time(void)
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + (double) (t.tv_usec / 1000000.0);
}

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

void
print_progress(size_t cur, size_t total)
{
    static int last_val = 0;
    double percentage = (double) cur / total;
    int val  = percentage * 100;
    int lpad = percentage * PBWIDTH;
    int rpad = PBWIDTH - lpad;
    if (val != last_val) {
        fprintf(stderr, "\r\t%3d%% [%.*s%*s] %lu/%lu", val, lpad, PBSTR, rpad, "", cur, total);
        fflush(stderr);
        last_val = val;
    }
}

int
size_t_fread(FILE *fp, size_t *x)
{
    if (fread(x, sizeof x[0], 1, fp) != 1)
        return CLT_ERR;
    return CLT_OK;
}

int
size_t_fwrite(FILE *fp, size_t x)
{
    if (fwrite(&x, sizeof x, 1, fp) != 1)
        return CLT_ERR;
    return CLT_OK;
}

int
bool_fread(FILE *fp, bool *x)
{
    return (fread(x, sizeof x[0], 1, fp) == 1) ? CLT_OK : CLT_ERR;
}
int
bool_fwrite(FILE *fp, bool x)
{
    return (fwrite(&x, sizeof x, 1, fp) == 1) ? CLT_OK : CLT_ERR;
}

#pragma once

#include <aesrand.h>
#include <assert.h>
#include <gmp.h>
#include <stdbool.h>
#include <stdio.h>

#pragma GCC visibility push(hidden)

static inline size_t
max(size_t a, size_t b) {
    return a > b ? a : b;
}

static inline size_t
min(size_t a, size_t b) {
    return a > b ? b : a;
}

double current_time(void);
void print_progress(size_t cur, size_t total);

/* `rop` = `a` mod `p`, where -p/2 < rop < p/2 */
static inline void
mpz_mod_near(mpz_t rop, const mpz_t a, const mpz_t p)
{
    mpz_t p_;
    mpz_init(p_);
    mpz_mod(rop, a, p);
    mpz_tdiv_q_2exp(p_, p, 1);
    if (mpz_cmp(rop, p_) > 0)
        mpz_sub(rop, rop, p);
    mpz_clear(p_);
}

static inline void
mpz_mod_near_ui(mpz_t rop, const mpz_t a, size_t p)
{

    mpz_mod_ui(rop, a, p);
    if (mpz_cmp_ui(rop, p / 2) > 0)
        mpz_sub_ui(rop, rop, p);
}

static inline void
mpz_mul_mod_near(mpz_t rop, const mpz_t a, const mpz_t b, const mpz_t p)
{
    mpz_mul(rop, a, b);
    mpz_mod_near(rop, rop, p);
}

static inline void
mpz_quotient(mpz_t rop, const mpz_t a, const mpz_t b)
{
    mpz_t tmp;
    mpz_init(tmp);
    mpz_mod_near(tmp, a, b);
    mpz_sub(rop, a, tmp);
    mpz_tdiv_q(rop, rop, b);
    mpz_clear(tmp);
}

static inline void
mpz_quotient_2exp(mpz_t rop, const mpz_t a, const size_t b)
{
    mpz_t tmp;
    mpz_init(tmp);
    mpz_mod_near_ui(tmp, a, 1 << b);
    mpz_sub(rop, a, tmp);
    mpz_tdiv_q_2exp(rop, rop, b);
    mpz_clear(tmp);
}

static inline void
mpz_random_(mpz_t rop, aes_randstate_t rng, size_t len)
{
    mpz_urandomb_aes(rop, rng, len);
    mpz_setbit(rop, len-1);
}

static inline void
mpz_prime(mpz_t rop, aes_randstate_t rng, size_t len)
{
    mpz_t p_unif;
    mpz_init(p_unif);
    do {
        mpz_random_(p_unif, rng, len);
        mpz_nextprime(rop, p_unif);
    } while (mpz_tstbit(rop, len) == 1);
    assert(mpz_tstbit(rop, len-1) == 1);
    mpz_clear(p_unif);
}

static inline void
product(mpz_t rop, mpz_t *xs, size_t n, bool verbose)
{
    double start = current_time();
    /* TODO: could parallelize this if desired */
    mpz_set_ui(rop, 1);
    for (size_t i = 0; i < n; i++) {
        mpz_mul(rop, rop, xs[i]);
        if (verbose)
            print_progress(i, n-1);
    }
    if (verbose)
        fprintf(stderr, "\t[%.2fs]\n", current_time() - start);
}

static inline void
crt_coeffs(mpz_t *coeffs, mpz_t *ps, size_t n, mpz_t x0, bool verbose)
{
    const double start = current_time();
    int count = 0;
#pragma omp parallel for
    for (size_t i = 0; i < n; i++) {
        mpz_t q;
        mpz_init(q);
        mpz_div(q, x0, ps[i]);
        mpz_invert(coeffs[i], q, ps[i]);
        mpz_mul_mod_near(coeffs[i], coeffs[i], q, x0);
        if (verbose) {
#pragma omp critical
            print_progress(++count, n);
        }
        mpz_clear(q);
    }
    if (verbose)
        fprintf(stderr, "\t[%.2fs]\n", current_time() - start);
}

static inline void
generate_zs(mpz_t *zs, mpz_t *zinvs, aes_randstate_t *rngs, size_t nzs, const mpz_t x0, bool verbose)
{
    const double start = current_time();
    int count = 0;
#pragma omp parallel for
    for (size_t i = 0; i < nzs; ++i) {
        do {
            mpz_urandomm_aes(zs[i], rngs[i], x0);
        } while (mpz_invert(zinvs[i], zs[i], x0) == 0);
        if (verbose)
#pragma omp critical
            print_progress(++count, nzs);
    }
    if (verbose)
        fprintf(stderr, "\t[%.2fs]\n", current_time() - start);
}

static inline void
generate_pzt(mpz_t pzt, size_t rho, size_t n, mpz_t *ps, mpz_t *gs,
             size_t nzs, mpz_t *zs, const int *ix, const mpz_t x0,
             aes_randstate_t *rngs, bool verbose)
{
    mpz_t zk;
    int count = 0;
    double start = current_time();
    mpz_set_ui(pzt, 0);
    if (verbose)
        fprintf(stderr, "  Generating pzt:\n");
    mpz_init_set_ui(zk, 1);
    /* compute z_1^t_1 ... z_k^t_k mod x0 */
    if (ix) {
        for (size_t i = 0; i < nzs; ++i) {
            mpz_t tmp;
            mpz_init(tmp);
            mpz_powm_ui(tmp, zs[i], ix[i], x0);
            mpz_mul_mod_near(zk, zk, tmp, x0);
            mpz_clear(tmp);
            if (verbose)
                print_progress(++count, n + nzs);
        }
    }
#pragma omp parallel for
    for (size_t i = 0; i < n; ++i) {
        mpz_t tmp, qpi, rnd, test;
        mpz_inits(tmp, qpi, rnd, test, NULL);
        /* compute ((g_i^{-1} mod p_i) · z · r_i · (x0 / p_i) */
        mpz_invert(tmp, gs[i], ps[i]);
        mpz_mul(tmp, tmp, zk);
        do {
            mpz_urandomb_aes(rnd, rngs[i], rho);
            mpz_mod(test, rnd, gs[i]);
        } while (mpz_cmp_ui(test, 0) == 0);
        mpz_mul(tmp, tmp, rnd);
        mpz_divexact(qpi, x0, ps[i]);
        mpz_mul_mod_near(tmp, tmp, qpi, x0);
#pragma omp critical
        {
            mpz_add(pzt, pzt, tmp);
        }
        mpz_clears(tmp, qpi, rnd, test, NULL);
        if (verbose)
#pragma omp critical
        {
            print_progress(++count, n + nzs);
        }
    }
    mpz_mod_near(pzt, pzt, x0);
    mpz_clear(zk);
    if (verbose)
        fprintf(stderr, "\t[%.2fs]\n", current_time() - start);
}


/* Generates `n` primes each of bitlength `len` */
static inline int
generate_primes(mpz_t *v, aes_randstate_t *rngs, size_t n, size_t len, bool verbose)
{
    const double start = current_time();
    int count = 0;
    if (verbose)
        fprintf(stderr, "%lu", len);
    print_progress(count, n);
#pragma omp parallel for
    for (size_t i = 0; i < n; ++i) {
        mpz_prime(v[i], rngs[i], len);
        if (verbose) {
#pragma omp critical
            print_progress(++count, n);
        }
    }
    if (verbose)
        fprintf(stderr, "\t[%.2fs]\n", current_time() - start);
    return CLT_OK;
}

/* Returns the number of ones in `x` */
static inline size_t
nb_of_bits(size_t x)
{
    size_t nb = 0;
    while (x > 0) {
        x >>= 1;
        nb++;
    }
    return nb;
}

static inline size_t
slot(size_t i, size_t n, size_t maxslots)
{
    if (i == maxslots - 1)
        return n - 1;
    else
        return i / (maxslots / n);
}

int mpz_fread(mpz_t x, FILE *fp);
int mpz_fwrite(mpz_t x, FILE *fp);
mpz_t * mpz_vector_new(size_t n);
void mpz_vector_free(mpz_t *v, size_t n);
int mpz_vector_fread(mpz_t *m, size_t len, FILE *fp);
int mpz_vector_fwrite(mpz_t *m, size_t len, FILE *fp);

int size_t_fread(FILE *fp, size_t *x);
int size_t_fwrite(FILE *fp, size_t x);
int bool_fread(FILE *fp, bool *x);
int bool_fwrite(FILE *fp, bool x);

#pragma GCC visibility pop

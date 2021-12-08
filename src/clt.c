#include "clt13.h"
#include "clt_elem.h"
#include "crt_tree.h"
#include "estimates.h"
#include "utils.h"

#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <omp.h>

/* etap is the size of the factors in the p_i's when using the composite ps
 * optimization.  We default this to 420, as is done in
 * https://github.com/tlepoint/new-multilinear-maps/blob/master/generate_pp.cpp */
#define ETAP_DEFAULT 420

struct clt_state_t {
    size_t n;                   /* number of slots */
    size_t nzs;                 /* number of z's in the index set */
    size_t rho;                 /* bitsize of randomness */
    size_t nu;                  /* number of most-significant-bits to extract */
    mpz_t *gs;                  /* plaintext moduli */
    aes_randstate_t *rngs;      /* random number generators (one per slot) */
    mpz_t x0;
    mpz_t pzt;                  /* zero testing parameter */
    mpz_t *zinvs;               /* z inverses */
    union {
        crt_tree *crt;
        mpz_t *crt_coeffs;
    };
    size_t flags;
};

struct clt_pp_t {
    mpz_t x0;
    mpz_t pzt;                  /* zero testing parameter */
    size_t nu;                  /* number of most-significant-bits to extract */
};

int
clt_elem_add(clt_elem_t *rop, const clt_pp_t *pp, const clt_elem_t *a, const clt_elem_t *b)
{
    mpz_add(rop->elem, a->elem, b->elem);
    mpz_mod(rop->elem, rop->elem, pp->x0);
    return CLT_OK;
}

int
clt_elem_sub(clt_elem_t *rop, const clt_pp_t *pp, const clt_elem_t *a, const clt_elem_t *b)
{
    mpz_sub(rop->elem, a->elem, b->elem);
    mpz_mod(rop->elem, rop->elem, pp->x0);
    return CLT_OK;
}

int
clt_elem_mul(clt_elem_t *rop, const clt_pp_t *pp, const clt_elem_t *a, const clt_elem_t *b)
{
    mpz_mul(rop->elem, a->elem, b->elem);
    mpz_mod(rop->elem, rop->elem, pp->x0);
    return CLT_OK;
}

int
clt_elem_mul_ui(clt_elem_t *rop, const clt_pp_t *pp, const clt_elem_t *a, unsigned int b)
{
    mpz_mul_ui(rop->elem, a->elem, b);
    mpz_mod(rop->elem, rop->elem, pp->x0);
    return CLT_OK;
}

clt_state_t *
clt_state_fread(FILE *fp)
{
    clt_state_t *s;

    if ((s = calloc(1, sizeof s[0])) == NULL) return NULL;
    if (size_t_fread(fp, &s->flags) == CLT_ERR) goto error;
    if (size_t_fread(fp, &s->n) == CLT_ERR) goto error;
    if (size_t_fread(fp, &s->nzs) == CLT_ERR) goto error;
    if (size_t_fread(fp, &s->rho) == CLT_ERR) goto error;
    if (size_t_fread(fp, &s->nu) == CLT_ERR) goto error;
    mpz_init(s->x0);
    if (mpz_fread(s->x0, fp) == CLT_ERR) goto error;
    mpz_init(s->pzt);
    if (mpz_fread(s->pzt, fp) == CLT_ERR) goto error;
    s->gs = mpz_vector_new(s->n);
    if (mpz_vector_fread(s->gs, s->n, fp) == CLT_ERR) goto error;
    s->zinvs = mpz_vector_new(s->nzs);
    if (mpz_vector_fread(s->zinvs, s->nzs, fp) == CLT_ERR) goto error;
    if (s->flags & CLT_FLAG_OPT_CRT_TREE) {
        if ((s->crt = crt_tree_fread(fp, s->n)) == NULL) goto error;
    } else {
        s->crt_coeffs = mpz_vector_new(s->n);
        if (mpz_vector_fread(s->crt_coeffs, s->n, fp) == CLT_ERR) goto error;
    }
    s->rngs = calloc(max(s->n, s->nzs), sizeof s->rngs[0]);
    for (size_t i = 0; i < max(s->n, s->nzs); ++i)
        if (aes_randstate_fread(s->rngs[i], fp) == AESRAND_ERR) goto error;
    return s;
error:
    free(s);
    return NULL;
}

int
clt_state_fwrite(clt_state_t *const s, FILE *const fp)
{
    if (s == NULL || fp == NULL)
        return CLT_ERR;
    if (size_t_fwrite(fp, s->flags) == CLT_ERR) goto error;
    if (size_t_fwrite(fp, s->n) == CLT_ERR) goto error;
    if (size_t_fwrite(fp, s->nzs) == CLT_ERR) goto error;
    if (size_t_fwrite(fp, s->rho) == CLT_ERR) goto error;
    if (size_t_fwrite(fp, s->nu) == CLT_ERR) goto error;
    if (mpz_fwrite(s->x0, fp) == CLT_ERR) goto error;
    if (mpz_fwrite(s->pzt, fp) == CLT_ERR) goto error;
    if (mpz_vector_fwrite(s->gs, s->n, fp) == CLT_ERR) goto error;
    if (mpz_vector_fwrite(s->zinvs, s->nzs, fp) == CLT_ERR) goto error;
    if (s->flags & CLT_FLAG_OPT_CRT_TREE) {
        if (crt_tree_fwrite(fp, s->crt, s->n) == CLT_ERR) goto error;
    } else {
        if (mpz_vector_fwrite(s->crt_coeffs, s->n, fp) == CLT_ERR) goto error;
    }
    for (size_t i = 0; i < max(s->n, s->nzs); ++i)
        aes_randstate_fwrite(s->rngs[i], fp);
    return CLT_OK;
error:
    return CLT_ERR;
}

clt_pp_t *
clt_pp_new(const clt_state_t *mmap)
{
    clt_pp_t *pp;

    if ((pp = calloc(1, sizeof pp[0])) == NULL)
        return NULL;
    mpz_inits(pp->x0, pp->pzt, NULL);
    mpz_set(pp->x0, mmap->x0);
    mpz_set(pp->pzt, mmap->pzt);
    pp->nu = mmap->nu;
    return pp;
}

void
clt_pp_free(clt_pp_t *pp)
{
    mpz_clears(pp->x0, pp->pzt, NULL);
    free(pp);
}

clt_pp_t *
clt_pp_fread(FILE *fp)
{
    clt_pp_t *pp;

    if ((pp = calloc(1, sizeof pp[0])) == NULL)
        return NULL;
    mpz_inits(pp->x0, pp->pzt, NULL);
    if (size_t_fread(fp, &pp->nu) == CLT_ERR) goto cleanup;
    if (mpz_fread(pp->x0, fp) == CLT_ERR) goto cleanup;
    if (mpz_fread(pp->pzt, fp) == CLT_ERR) goto cleanup;
    return pp;
cleanup:
    clt_pp_free(pp);
    return NULL;
}

int
clt_pp_fwrite(clt_pp_t *pp, FILE *fp)
{
    if (size_t_fwrite(fp, pp->nu) == CLT_ERR) goto cleanup;
    if (mpz_fwrite(pp->x0, fp) == CLT_ERR) goto cleanup;
    if (mpz_fwrite(pp->pzt, fp) == CLT_ERR) goto cleanup;
    return CLT_OK;
cleanup:
    return CLT_ERR;
}

static int
generate_primes_composite_ps(mpz_t *v, aes_randstate_t *rngs, size_t n, size_t eta, bool verbose)
{
    int count = 0;
    double start = current_time();
    size_t etap = ETAP_DEFAULT;
    if (eta > 350)
        /* TODO: change how we set etap, should be resistant to factoring x_0 */
        for (/* */; eta % etap < 350; etap++)
            ;
    if (verbose) {
        fprintf(stderr, " [eta_p: %lu] ", etap);
    }
    size_t nchunks = eta / etap;
    size_t leftover = eta - nchunks * etap;
    if (verbose) {
        fprintf(stderr, "[nchunks=%lu leftover=%lu]\n", nchunks, leftover);
        print_progress(count, n);
    }
#pragma omp parallel for
    for (size_t i = 0; i < n; i++) {
        mpz_t p_unif;
        mpz_set_ui(v[i], 1);
        mpz_init(p_unif);
        /* generate a p_i */
        for (size_t j = 0; j < nchunks; j++) {
            mpz_prime(p_unif, rngs[i], etap);
            mpz_mul(v[i], v[i], p_unif);
        }
        mpz_prime(p_unif, rngs[i], leftover);
        mpz_mul(v[i], v[i], p_unif);
        mpz_clear(p_unif);

        if (verbose) {
#pragma omp critical
            print_progress(++count, n);
        }
    }
    if (verbose) {
        fprintf(stderr, "\t[%.2fs]\n", current_time() - start);
    }
    return CLT_OK;
}

static inline size_t
max3(size_t a, size_t b, size_t c)
{
    return a >= b && a >= c ? a : b >= a && b >= c ? b : c;
}

clt_state_t *
clt_state_new(const clt_params_t *params, const clt_opt_params_t *opts,
              size_t ncores, size_t flags, aes_randstate_t rng)
{
    clt_state_t *s;
    size_t alpha, beta, eta, rho_f;
    mpz_t *ps = NULL, *zs;
    double start_time = 0.0;
    const bool verbose = flags & CLT_FLAG_VERBOSE;
    const size_t slots = opts ? opts->slots : 0;

    if ((s = calloc(1, sizeof s[0])) == NULL)
        return NULL;

    if (ncores == 0)
        ncores = sysconf(_SC_NPROCESSORS_ONLN);
    (void) omp_set_num_threads(ncores);

    /* calculate CLT parameters */
    s->nzs = params->nzs;
    alpha  = params->lambda;           /* bitsize of g_i primes */
    beta   = params->lambda;           /* bitsize of h_i entries */
    s->rho = params->lambda;           /* bitsize of randomness */
    rho_f  = params->kappa * (s->rho + alpha); /* max bitsize of r_i's */
    eta    = rho_f + alpha + beta + 9; /* bitsize of primes p_i */
    s->n   = max(estimate_n(params->lambda, eta, flags), slots); /* number of primes */
    eta    = rho_f + alpha + beta + nb_of_bits(s->n) + 9; /* bitsize of primes p_i */
    s->nu  = eta - beta - rho_f - nb_of_bits(s->n) - 3; /* number of msbs to extract */
    s->flags = flags;

    /* Loop until a fixed point reached for choosing eta, n, and nu */
    {
        size_t old_eta = 0, old_n = 0, old_nu = 0;
        int i = 0;
        for (; i < 10 && (old_eta != eta || old_n != s->n || old_nu != s->nu);
             ++i) {
            old_eta = eta, old_n = s->n, old_nu = s->nu;
            eta = rho_f + alpha + beta + nb_of_bits(s->n) + 9;
            s->n = max(estimate_n(params->lambda, eta, flags), slots);
            s->nu = eta - beta - rho_f - nb_of_bits(s->n) - 3;
        }

        if (i == 10 && (old_eta != eta || old_n != s->n || old_nu != s->nu)) {
            fprintf(stderr, "error: unable to find valid η, n, and ν choices\n");
            free(s);
            return NULL;
        }
    }

    /* Make sure the proper bounds are hit [CLT13, Lemma 8] */
    assert(s->nu >= alpha + 6);
    assert(beta + alpha + rho_f + nb_of_bits(s->n) <= eta - 9);
    assert(s->n >= slots);

    if (verbose) {
        fprintf(stderr, "  λ: %ld\n", params->lambda);
        fprintf(stderr, "  κ: %ld\n", params->kappa);
        fprintf(stderr, "  α: %ld\n", alpha);
        fprintf(stderr, "  β: %ld\n", beta);
        fprintf(stderr, "  η: %ld\n", eta);
        fprintf(stderr, "  ν: %ld\n", s->nu);
        fprintf(stderr, "  ρ: %ld\n", s->rho);
        fprintf(stderr, "  ρ_f: %ld\n", rho_f);
        fprintf(stderr, "  n: %ld\n", s->n);
        fprintf(stderr, "  nzs: %ld\n", s->nzs);
        fprintf(stderr, "  ncores: %ld\n", ncores);
        fprintf(stderr, "  Flags: \n");
        if (s->flags & CLT_FLAG_OPT_CRT_TREE)
            fprintf(stderr, "    CRT TREE\n");
        if (s->flags & CLT_FLAG_OPT_PARALLEL_ENCODE)
            fprintf(stderr, "    PARALLEL ENCODE\n");
        if (s->flags & CLT_FLAG_OPT_COMPOSITE_PS)
            fprintf(stderr, "    COMPOSITE PS\n");
        if (s->flags & CLT_FLAG_SEC_IMPROVED_BKZ)
            fprintf(stderr, "    IMPROVED BKZ\n");
        if (s->flags & CLT_FLAG_SEC_CONSERVATIVE)
            fprintf(stderr, "    CONSERVATIVE\n");
    }

    /* Generate randomness for each core */
    s->rngs = calloc(max(s->n, s->nzs), sizeof s->rngs[0]);
    for (size_t i = 0; i < max(s->n, s->nzs); ++i) {
        unsigned char *buf;
        size_t nbytes;

        buf = random_aes(rng, 128, &nbytes);
        aes_randinit_seedn(s->rngs[i], (char *) buf, nbytes, NULL, 0);
        free(buf);
    }

    /* Generate "plaintext" moduli */
    s->gs = mpz_vector_new(s->n);
    if (verbose)
        fprintf(stderr, "  Generating g_i's:\n");
    if (opts && opts->moduli && opts->nmoduli) {
        for (size_t i = 0; i < opts->nmoduli; ++i)
            mpz_set(s->gs[i], opts->moduli[i]);
        generate_primes(s->gs + opts->nmoduli, s->rngs, s->n - opts->nmoduli, alpha, verbose);
    } else {
        generate_primes(s->gs, s->rngs, s->n, alpha, verbose);
    }

    mpz_init_set_ui(s->x0,  1);

    if (!(s->flags & CLT_FLAG_OPT_CRT_TREE)) {
        s->crt_coeffs = mpz_vector_new(s->n);
    }

    /* Generate "ciphertext" moduli */
    if (verbose)
        fprintf(stderr, "  Generating p_i's:\n");
    ps = mpz_vector_new(s->n);
generate_ps:
    if (s->flags & CLT_FLAG_OPT_COMPOSITE_PS) {
        generate_primes_composite_ps(ps, s->rngs, s->n, eta, verbose);
    } else {
        generate_primes(ps, s->rngs, s->n, eta, verbose);
    }

    /* Compute product if "ciphertext" moduli */
    if (s->flags & CLT_FLAG_OPT_CRT_TREE) {
        start_time = current_time();
        if (verbose)
            fprintf(stderr, "  Generating CRT tree: ");
        if ((s->crt = crt_tree_new(ps, s->n)) == NULL) {
            /* if crt_tree_new fails, regenerate with new p_i's */
            if (verbose)
                fprintf(stderr, "(restarting)\n");
            goto generate_ps;
        }
        mpz_set(s->x0, s->crt->mod);
        if (verbose)
            fprintf(stderr, "[%.2fs]\n", current_time() - start_time);
    } else {
        if (verbose)
            fprintf(stderr, "  Computing x0:\n");
        product(s->x0, ps, s->n, verbose);
        if (verbose)
            fprintf(stderr, "  Generating CRT coefficients:\n");
        crt_coeffs(s->crt_coeffs, ps, s->n, s->x0, verbose);
    }

    zs       = mpz_vector_new(s->nzs);
    s->zinvs = mpz_vector_new(s->nzs);
    if (verbose)
        fprintf(stderr, "  Generating z_i's:\n");
    generate_zs(zs, s->zinvs, s->rngs, s->nzs, s->x0, verbose);

    mpz_init(s->pzt);
    generate_pzt(s->pzt, beta, s->n, ps, s->gs, s->nzs, zs, params->pows,
                 s->x0, s->rngs, verbose);

    mpz_vector_free(ps, s->n);
    mpz_vector_free(zs, s->nzs);

    return s;
}

void
clt_state_free(clt_state_t *s)
{
    mpz_clear(s->x0);
    mpz_clear(s->pzt);
    mpz_vector_free(s->gs, s->n);
    mpz_vector_free(s->zinvs, s->nzs);
    if (s->flags & CLT_FLAG_OPT_CRT_TREE) {
        crt_tree_free(s->crt);
    } else {
        mpz_vector_free(s->crt_coeffs, s->n);
    }
    if (s->rngs) {
        for (size_t i = 0; i < max(s->n, s->nzs); ++i) {
            aes_randclear(s->rngs[i]);
        }
        free(s->rngs);
    }
    free(s);
}

mpz_t *
clt_state_moduli(const clt_state_t *s)
{
    return s->gs;
}

size_t
clt_state_nslots(const clt_state_t *s)
{
    return s->n;
}

size_t
clt_state_nzs(const clt_state_t *s)
{
    return s->nzs;
}

int
clt_encode(clt_elem_t *rop, const clt_state_t *s, size_t n, const mpz_t *xs,
           const int *ix)
{
    if (rop == NULL || s == NULL || n == 0 || xs == NULL)
        return CLT_ERR;

    if (!(s->flags & CLT_FLAG_OPT_PARALLEL_ENCODE))
        omp_set_num_threads(1);

    /* slots[i] = m[i] + r·g[i] */
    if (s->flags & CLT_FLAG_OPT_CRT_TREE) {
        mpz_t *slots = mpz_vector_new(s->n);
#pragma omp parallel for
        for (size_t i = 0; i < s->n; i++) {
            mpz_random_(slots[i], s->rngs[i], s->rho);
            mpz_mul(slots[i], slots[i], s->gs[i]);
            if (i < n)
                mpz_add(slots[i], slots[i], xs[i]);
            /* mpz_add(slots[i], slots[i], xs[slot(i, n, s->n)]); */
        }
        crt_tree_do_crt(rop->elem, s->crt, slots);
        mpz_vector_free(slots, s->n);
    } else {
        mpz_set_ui(rop->elem, 0);
#pragma omp parallel for
        for (size_t i = 0; i < s->n; ++i) {
            mpz_t tmp;
            mpz_init(tmp);
            mpz_random_(tmp, s->rngs[i], s->rho);
            mpz_mul(tmp, tmp, s->gs[i]);
            if (i < n)
                mpz_add(tmp, tmp, xs[i]);
            /* mpz_add(tmp, tmp, xs[slot(i, n, s->n)]); */
            mpz_mul(tmp, tmp, s->crt_coeffs[i]);
#pragma omp critical
            {
                mpz_add(rop->elem, rop->elem, tmp);
            }
            mpz_clear(tmp);
        }
    }
    if (ix) {
        mpz_t tmp;
        mpz_init(tmp);
        /* multiply by appropriate zinvs */
        for (size_t i = 0; i < s->nzs; ++i) {
            if (ix[i] <= 0) continue;
            mpz_powm_ui(tmp, s->zinvs[i], ix[i], s->x0);
            mpz_mul_mod_near(rop->elem, rop->elem, tmp, s->x0);
        }
        mpz_clear(tmp);
    }
    return CLT_OK;
}

int
clt_is_zero(const clt_elem_t *c, const clt_pp_t *pp)
{
    int ret;

    mpz_t tmp;
    mpz_init(tmp);

    mpz_mul(tmp, c->elem, pp->pzt);
    mpz_mod_near(tmp, tmp, pp->x0);

    ret = mpz_sizeinbase(tmp, 2) < mpz_sizeinbase(pp->x0, 2) - pp->nu;
    mpz_clear(tmp);
    return ret ? 1 : 0;
}

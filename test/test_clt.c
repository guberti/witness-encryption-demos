#include <clt13.h>
#include <aesrand.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

static size_t
ram_usage(void)
{
    FILE *fp;
    size_t rss = 0;
    if ((fp = fopen("/proc/self/statm", "r")) == NULL)
        return 0;
    if (fscanf(fp, "%*s%lu", &rss) != 1) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return rss * sysconf(_SC_PAGESIZE) / 1024;
}

static int
expect(char *desc, int expected, int recieved)
{
    if (expected != recieved) {
        printf("\033[1;41m");
    }
    printf("%s = %d", desc, recieved);
    if (expected != recieved) {
        printf("\033[0m");
    }
    puts("");
    return expected == recieved;
}

static int
test(size_t flags, size_t nzs, size_t lambda, size_t kappa)
{
    srand(time(NULL));

    clt_state_t *mmap;
    clt_pp_t *pp;
    mpz_t *moduli, modulus;
    aes_randstate_t rng;
    int pows[nzs];

    int ok = 1;

    aes_randinit(rng);
    for (size_t i = 0; i < nzs; i++) pows[i] = 1;
    mpz_init_set_ui(modulus, 2);

    clt_params_t params = {
        .lambda = lambda,
        .kappa = kappa,
        .nzs = nzs,
        .pows = pows,
    };
    clt_opt_params_t opts = {
        .slots = 0,
        .moduli = &modulus,
        .nmoduli = 1,
    };
    mmap = clt_state_new(&params, &opts, 0, flags, rng);
    pp = clt_pp_new(mmap);

    /* Test read/write */

    {
        FILE *mmap_f;

        mmap_f = tmpfile();
        if (clt_state_fwrite(mmap, mmap_f) == CLT_ERR) {
            fprintf(stderr, "clt_state_fwrite failed!\n");
            exit(1);
        }
        clt_state_free(mmap);
        rewind(mmap_f);
        if ((mmap = clt_state_fread(mmap_f)) == NULL) {
            fprintf(stderr, "clt_state_fread failed!\n");
            exit(1);
        }
        fclose(mmap_f);
    }

    {
        FILE *pp_f;

        pp_f = tmpfile();
        if (pp_f == NULL) {
            fprintf(stderr, "Couldn't open tmpfile!\n");
            exit(1);
        }
        if (clt_pp_fwrite(pp, pp_f) == CLT_ERR) {
            fprintf(stderr, "clt_pp_fwrite failed!\n");
            exit(1);
        }
        clt_pp_free(pp);
        rewind(pp_f);
        if ((pp = clt_pp_fread(pp_f)) == NULL) {
            fprintf(stderr, "clt_pp_fread failed!\n");
            exit(1);
        }
        fclose(pp_f);
    }

    moduli = clt_state_moduli(mmap);

    mpz_t x[1], zero[1], one[1], two[1], three[1];
    int top_level[nzs];

    mpz_init_set_ui(x[0], 0);
    while (mpz_cmp_ui(x[0], 0) <= 0) {
        mpz_set_ui(x[0], rand());
        mpz_mod(x[0], x[0], moduli[0]);
    }
    gmp_printf("x = %Zd\n", x[0]);

    mpz_init_set_ui(zero[0], 0);
    mpz_init_set_ui(one[0], 1);
    mpz_init_set_ui(two[0], 2);
    mpz_init_set_ui(three[0], 3);

    for (size_t i = 0; i < nzs; i++) {
        top_level[i] = 1;
    }

    clt_elem_t *x0, *x1, *xp;
    x0 = clt_elem_new();
    x1 = clt_elem_new();
    xp = clt_elem_new();

    clt_encode(x0, mmap, 1, (const mpz_t *) zero, top_level);
    clt_encode(x1, mmap, 1, (const mpz_t *) zero, top_level);
    clt_elem_add(xp, pp, x0, x1);
    ok &= expect("is_zero(0 + 0)", 1, clt_is_zero(xp, pp));

    clt_encode(x0, mmap, 1, (const mpz_t *) zero, top_level);
    clt_encode(x1, mmap, 1, (const mpz_t *) one,  top_level);
    ok &= expect("is_zero(0)", 1, clt_is_zero(x0, pp));
    ok &= expect("is_zero(1)", 0, clt_is_zero(x1, pp));
    clt_elem_add(xp, pp, x0, x1);
    ok &= expect("is_zero(0 + 1)", 0, clt_is_zero(xp, pp));

    clt_encode(x0, mmap, 1, (const mpz_t *) one, top_level);
    clt_encode(x1, mmap, 1, (const mpz_t *) two, top_level);
    clt_elem_mul_ui(x0, pp, x0, 2);
    clt_elem_sub(xp, pp, x1, x0);
    ok &= expect("is_zero(2 - 2[1])", 1, clt_is_zero(xp, pp));

    clt_encode(x0, mmap, 1, (const mpz_t *) zero, top_level);
    clt_encode(x1, mmap, 1, (const mpz_t *) x,    top_level);
    clt_elem_add(xp, pp, x0, x1);
    ok &= expect("is_zero(0 + x)", 0, clt_is_zero(xp, pp));

    clt_encode(x0, mmap, 1, (const mpz_t *) x, top_level);
    clt_encode(x1, mmap, 1, (const mpz_t *) x, top_level);
    clt_elem_sub(xp, pp, x0, x1);
    ok &= expect("is_zero(x - x)", 1, clt_is_zero(xp, pp));

    clt_encode(x0, mmap, 1, (const mpz_t *) zero, top_level);
    clt_encode(x1, mmap, 1, (const mpz_t *) x,    top_level);
    clt_elem_sub(xp, pp, x0, x1);
    ok &= expect("is_zero(0 - x)", 0, clt_is_zero(xp, pp));

    clt_encode(x0, mmap, 1, (const mpz_t *) one,  top_level);
    clt_encode(x1, mmap, 1, (const mpz_t *) zero, top_level);
    clt_elem_sub(xp, pp, x0, x1);
    ok &= expect("is_zero(1 - 0)", 0, clt_is_zero(xp, pp));

    clt_encode(x0, mmap, 1, (const mpz_t *) one,  top_level);
    clt_encode(x1, mmap, 1, (const mpz_t *) three, top_level);
    clt_elem_mul_ui(x0, pp, x0, 3);
    clt_elem_sub(xp, pp, x0, x1);
    ok &= expect("is_zero(3*[1] - [3])", 1, clt_is_zero(xp, pp));

    int ix0[nzs], ix1[nzs];
    for (size_t i = 0; i < nzs; i++) {
        if (i < nzs / 2) {
            ix0[i] = 1;
            ix1[i] = 0;
        } else {
            ix0[i] = 0;
            ix1[i] = 1;
        }
    }
    clt_encode(x0, mmap, 1, (const mpz_t *) x   , ix0);
    clt_encode(x1, mmap, 1, (const mpz_t *) zero, ix1);
    clt_elem_mul(xp, pp, x0, x1);
    ok &= expect("is_zero(x * 0)", 1, clt_is_zero(xp, pp));

    clt_encode(x0, mmap, 1, (const mpz_t *) x  , ix0);
    clt_encode(x1, mmap, 1, (const mpz_t *) one, ix1);
    clt_elem_mul(xp, pp, x0, x1);
    ok &= expect("is_zero(x * 1)", 0, clt_is_zero(xp, pp));

    clt_encode(x0, mmap, 1, (const mpz_t *) x, ix0);
    clt_encode(x1, mmap, 1, (const mpz_t *) x, ix1);
    clt_elem_mul(xp, pp, x0, x1);
    ok &= expect("is_zero(x * x)", 0, clt_is_zero(xp, pp));

    // zimmerman-like test

    mpz_t in0[2], in1[2], cin[2];
    clt_elem_t *c = clt_elem_new();

    mpz_inits(in0[0], in0[1], in1[0], in1[1], cin[0], cin[1], NULL);

    mpz_set_ui      (in0[0], 0);
    mpz_urandomb_aes(in0[1], rng, lambda);
    mpz_mod         (in0[1], in0[1], moduli[1]);

    mpz_urandomb_aes(in1[0], rng, lambda);
    mpz_mod         (in1[0], in1[0], moduli[0]);
    mpz_urandomb_aes(in1[1], rng, lambda);
    mpz_mod         (in1[1], in1[1], moduli[1]);

    mpz_set_ui(cin[0], 0);
    mpz_mul   (cin[1], in0[1], in1[1]);

    clt_encode(x0, mmap, 2, (const mpz_t *) in0, ix0);
    clt_encode(x1, mmap, 2, (const mpz_t *) in1, ix1);
    clt_encode(c,  mmap, 2, (const mpz_t *) cin, top_level);

    clt_elem_mul(xp, pp, x0, x1);
    clt_elem_sub(xp, pp, xp, c);

    ok &= expect("[Z] is_zero(0 * x)", 1, clt_is_zero(xp, pp));

    /* [ $, $ ] */
    mpz_urandomb_aes(in0[0], rng, lambda);
    mpz_mod         (in0[0], in0[0], moduli[0]);
    mpz_urandomb_aes(in0[1], rng, lambda);
    mpz_mod         (in0[1], in0[1], moduli[1]);
    /* [ $, $ ] */
    mpz_urandomb_aes(in1[0], rng, lambda);
    mpz_mod         (in1[0], in1[0], moduli[0]);
    mpz_urandomb_aes(in1[1], rng, lambda);
    mpz_mod         (in1[1], in1[1], moduli[1]);

    mpz_set_ui      (cin[0], 0);
    mpz_urandomb_aes(cin[1], rng, lambda);
    mpz_mod         (cin[1], cin[1], moduli[1]);
    /* mpz_mul   (cin[1], in0[1], in1[1]); */

    clt_encode(x0, mmap, 2, (const mpz_t *) in0, ix0);
    clt_encode(x1, mmap, 2, (const mpz_t *) in1, ix1);
    clt_encode(c,  mmap, 2, (const mpz_t *) cin, top_level);

    clt_elem_mul(xp, pp, x0, x1);
    clt_elem_sub(xp, pp, xp, c);

    ok &= expect("[Z] is_zero(x * y)", 0, clt_is_zero(xp, pp));

    mpz_clears(x[0], zero[0], one[0], two[0], three[0],
               in0[0], in0[1], in1[0], in1[1], cin[0], cin[1], NULL);

    clt_elem_free(c);
    clt_elem_free(x0);
    clt_elem_free(x1);
    clt_elem_free(xp);
    clt_pp_free(pp);
    clt_state_free(mmap);
    aes_randclear(rng);

    {
        size_t ram = ram_usage();
        printf("RAM: %lu Kb\n", ram);
    }

    return !ok;
}

static int
test_levels(size_t flags, size_t kappa, size_t lambda)
{
    int pows[kappa], top_level[kappa];
    clt_state_t *s;
    clt_pp_t *pp;
    aes_randstate_t rng;
    mpz_t zero, one;
    clt_elem_t *value, *result, *top_one, *top_zero;
    int ok = 1;

    clt_params_t params = {
        .lambda = lambda,
        .kappa = kappa,
        .nzs = kappa,
        .pows = top_level
    };

    value = clt_elem_new();
    result = clt_elem_new();
    top_one = clt_elem_new();
    top_zero = clt_elem_new();

    printf("Testing levels: λ = %lu, κ = %lu\n", lambda, kappa);

    aes_randinit(rng);
    mpz_init_set_ui(zero, 0);
    mpz_init_set_ui(one, 1);

    for (size_t i = 0; i < kappa; ++i)
        top_level[i] = 1;

    s = clt_state_new(&params, NULL, 0, flags, rng);
    pp = clt_pp_new(s);

    clt_encode(top_one, s, 1, (const mpz_t *) &one, top_level);
    clt_encode(top_zero, s, 1, (const mpz_t *) &zero, top_level);

    for (size_t i = 0; i < kappa; ++i) {
        for (size_t j = 0; j < kappa; ++j) {
            if (j != i)
                pows[j] = 0;
            else
                pows[j] = 1;
        }
        clt_encode(value, s, 1, (const mpz_t *) &one, pows);
        if (i == 0)
            clt_elem_set(result, value);
        else {
            clt_elem_mul(result, pp, result, value);
        }
    }

    ok &= expect("is_zero(1 * ... * 1)", 0, clt_is_zero(result, pp));

    clt_elem_sub(result, pp, result, top_one);

    ok &= expect("is_zero(1 * ... * 1 - 1)", 1, clt_is_zero(result, pp));

    for (size_t i = 0; i < kappa; ++i) {
        for (size_t j = 0; j < kappa; ++j) {
            if (j != i)
                pows[j] = 0;
            else
                pows[j] = 1;
        }
        if (i == 0) {
            clt_encode(value, s, 1, (const mpz_t *) &zero, pows);
            clt_elem_set(result, value);
        } else {
            clt_encode(value, s, 1, (const mpz_t *) &one, pows);
            clt_elem_mul(result, pp, result, value);
        }
    }

    ok &= expect("is_zero(0 * 1 *  ... * 1)", 1, clt_is_zero(result, pp));

    clt_elem_add(result, pp, result, top_one);

    ok &= expect("is_zero(0 * 1 *  ... * 1 + 1)", 0, clt_is_zero(result, pp));

    {
        size_t ram = ram_usage();
        printf("RAM: %lu Kb\n", ram);
    }

    clt_pp_free(pp);
    clt_state_free(s);

    mpz_clears(zero, one, NULL);
    clt_elem_free(value);
    clt_elem_free(result);
    clt_elem_free(top_one);
    clt_elem_free(top_zero);
    aes_randclear(rng);

    return !ok;
}

int
main(int argc, char *argv[])
{
    size_t default_flags = CLT_FLAG_NONE | CLT_FLAG_VERBOSE;
    size_t flags;
    size_t kappa;
    size_t lambda = 20;
    size_t nzs = 10;

    if (argc == 2) {
        lambda = atoi(argv[1]);
    }

    if (test_levels(default_flags, 32, 10))
        return 1;

    printf("* No optimizations\n");
    flags = default_flags;
    if (test(flags, nzs, 16, 2) == 1)
        return 1;

    kappa = 15;

    printf("* No optimizations\n");
    flags = default_flags;
    if (test(flags, nzs, lambda, kappa) == 1)
        return 1;

    printf("* CRT tree\n");
    flags = default_flags | CLT_FLAG_OPT_CRT_TREE;
    if (test(flags, nzs, lambda, kappa) == 1)
        return 1;

    printf("* CRT tree + parallel encode\n");
    flags = default_flags | CLT_FLAG_OPT_CRT_TREE | CLT_FLAG_OPT_PARALLEL_ENCODE;
    if (test(flags, nzs, lambda, kappa) == 1)
        return 1;

    printf("* CRT tree + composite ps\n");
    flags = default_flags | CLT_FLAG_OPT_CRT_TREE | CLT_FLAG_OPT_COMPOSITE_PS;
    if (test(flags, nzs, lambda, kappa) == 1)
        return 1;

    printf("* CRT tree + parallel encode + composite ps\n");
    flags = default_flags | CLT_FLAG_OPT_CRT_TREE | CLT_FLAG_OPT_PARALLEL_ENCODE | CLT_FLAG_OPT_COMPOSITE_PS;
    if (test(flags, nzs, lambda, kappa) == 1)
        return 1;

    kappa = 12;
    printf("* CRT tree + parallel encode + composite ps\n");
    flags = default_flags | CLT_FLAG_OPT_CRT_TREE | CLT_FLAG_OPT_PARALLEL_ENCODE | CLT_FLAG_OPT_COMPOSITE_PS;
    if (test(flags, nzs, lambda, kappa) == 1)
        return 1;

    return 0;
}

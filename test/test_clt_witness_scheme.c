#include <clt13.h>
#include <aesrand.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#include <puzzles/easy.c>

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
test_levels(size_t flags, size_t kappa, size_t lambda)
{
    int pows[kappa], top_level[kappa];
    clt_state_t *s;
    clt_pp_t *pp;
    aes_randstate_t rng;
    clt_elem_t *value, *result, *top_target;
    int ok = 1;

    clt_params_t params = {
        .lambda = lambda,
        .kappa = kappa,
        .nzs = kappa,
        .pows = top_level
    };

    mpz_t *a;
    mpz_t target_product;
    mpz_t test_product;
    a = malloc(kappa * sizeof(mpz_t));
    mpz_init_set_ui(target_product, 1);
    mpz_init_set_ui(test_product, 1);
    mpz_array_init(*a, kappa, 1);
    for (size_t i = 0; i < kappa; i++) {
        mpz_urandomb_aes(a[i], rng, 2 * lambda);
        mpz_mul(target_product, a[i], target_product);
    }

    value = clt_elem_new();
    result = clt_elem_new();
    top_target = clt_elem_new();

    printf("Testing levels: λ = %lu, κ = %lu\n", lambda, kappa);

    aes_randinit(rng);

    for (size_t i = 0; i < kappa; ++i)
        top_level[i] = 1;

    s = clt_state_new(&params, NULL, 0, flags, rng);
    pp = clt_pp_new(s);

    mpz_t intermediate;
    mpz_init(intermediate);
    for (size_t i = 0; i < 3; ++i) {
        mpz_set_ui(intermediate, 1);
        for (size_t j = 0; j < kappa; ++j) {
            pows[j] = 0;
        }

        for (size_t j = 0; j < 2; ++j) {
            uint16_t index = puzzle[i][j];
            pows[index] = 1;
            mpz_mul(intermediate, a[index], intermediate);
        }


        clt_encode(value, s, 1, (const mpz_t *) &intermediate, pows);
        // Our witness is {0, 2}
        if (i == 0 || i == 2) {
            if (i == 0)
                clt_elem_set(result, value);
            else {
                clt_elem_mul(result, pp, result, value);
            }
        }
    }
    clt_encode(top_target, s, 1, (const mpz_t *) &target_product, top_level);

    ok &= expect("is_zero(1 * ... * 1)", 0, clt_is_zero(result, pp));

    clt_elem_sub(result, pp, result, top_target);

    ok &= expect("is_zero(1 * ... * 1 - 1)", 1, clt_is_zero(result, pp));

    clt_pp_free(pp);
    clt_state_free(s);
    clt_elem_free(value);
    clt_elem_free(result);
    clt_elem_free(top_target);
    aes_randclear(rng);

    return !ok;
}

int
main()
{
    size_t default_flags = CLT_FLAG_NONE | CLT_FLAG_VERBOSE;
    if (test_levels(default_flags, 4, 20))
        return 1;
    return 0;
}

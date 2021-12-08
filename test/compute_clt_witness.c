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
#include <string.h>

#include <puzzles/sudoku1.c>
#include <puzzles/easywallet1.c>

const char folder[] = "sudoku1";
static int
test_levels(size_t flags, size_t kappa, size_t lambda)
{
    int pows[kappa], top_level[kappa];
    clt_state_t *s;
    clt_pp_t *pp;
    aes_randstate_t rng;
    clt_elem_t *value, *result;
    int ok = 1;

    clt_params_t params = {
        .lambda = lambda,
        .kappa = kappa,
        .nzs = kappa,
        .pows = top_level
    };

    mpz_t *a;
    mpz_t target_product, false_product;

    value = clt_elem_new();
    result = clt_elem_new();

    printf("Testing levels: λ = %lu, κ = %lu\n", lambda, kappa);

    aes_randinit(rng);

    for (size_t i = 0; i < kappa; ++i)
        top_level[i] = 1;

    s = clt_state_new(&params, NULL, 0, flags, rng);
    pp = clt_pp_new(s);

    // Helper variable for computing intermediate products
    mpz_t intermediate;
    mpz_init(intermediate);

    char filename_buff[80];
    FILE *f = NULL;


    mkdir(folder, 0777);
    sprintf(filename_buff, "%s/pp_l=%zu.enc", folder, lambda);
    f = fopen(filename_buff,"w");
    clt_pp_fwrite(pp, f);
    if (f) fclose(f);

    for (size_t b_i = 0; b_i < 256; ++b_i) {
        sprintf(filename_buff, "%s/message_bit_%zu", folder, b_i);
        mkdir(filename_buff, 0777);

        a = malloc(kappa * sizeof(mpz_t));
        mpz_init_set_ui(target_product, 1);
        mpz_init_set_ui(false_product, 1);
        mpz_array_init(*a, kappa, 1);
        for (size_t i = 0; i < kappa; i++) {
            mpz_urandomb_aes(a[i], rng, 2 * lambda);
            mpz_mul(target_product, a[i], target_product);
        }
        mpz_add_ui(false_product, target_product, 1);

        // Write set encodings to file
        for (size_t i = 0; i < 248; ++i) {
            // Reset from previous iterations
            mpz_set_ui(intermediate, 1);
            for (size_t j = 0; j < kappa; ++j) {
                pows[j] = 0;
            }

            for (size_t j = 0; j < 4; ++j) {
                uint16_t index = puzzle[i][j];
                pows[index] = 1;
                mpz_mul(intermediate, a[index], intermediate);
            }

            clt_encode(value, s, 1, (const mpz_t *) &intermediate, pows);

            // Write out file
            sprintf(filename_buff, "%s/message_bit_%zu/set_%zu.enc", folder, b_i, i);
            f = fopen(filename_buff,"w");
            clt_elem_fwrite(value, f);
            if (f) fclose(f);
        }

        // Bit encoding
        if (message[b_i] == 0) {
            clt_encode(value, s, 1, (const mpz_t *) &false_product, top_level);
        } else {
            clt_encode(value, s, 1, (const mpz_t *) &target_product, top_level);
        }

        sprintf(filename_buff, "%s/message_bit_%zu/message_bit.enc", folder, b_i);
        f = fopen(filename_buff,"w");
        clt_elem_fwrite(value, f);
        if (f) fclose(f);

        printf("Wrote message bit %zu/256\n", b_i);
    }

    clt_pp_free(pp);
    clt_state_free(s);
    clt_elem_free(value);
    clt_elem_free(result);
    aes_randclear(rng);
    return !ok;
}

int main() {
    size_t flags = CLT_FLAG_VERBOSE | CLT_FLAG_OPT_CRT_TREE | CLT_FLAG_OPT_PARALLEL_ENCODE | CLT_FLAG_OPT_COMPOSITE_PS;
    if (test_levels(flags, 64, 20))
        return 1;
    return 0;
}

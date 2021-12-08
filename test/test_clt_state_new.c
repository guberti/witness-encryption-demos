#include <clt13.h>
#include <sys/stat.h>
#include <sys/time.h>

static int lambda = 60;
static int kappa = 21;
static int nzs = 17;
static size_t flags = CLT_FLAG_VERBOSE | CLT_FLAG_OPT_CRT_TREE | CLT_FLAG_OPT_COMPOSITE_PS | CLT_FLAG_SEC_IMPROVED_BKZ;

static double
current_time(void)
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + (double) (t.tv_usec / 1000000.0);
}

int
main(void)
{
    clt_state_t *s;
    int pows[nzs];
    aes_randstate_t rng;
    int ret = 1;
    double start;

    clt_params_t params = {
        .lambda = lambda,
        .kappa = kappa,
        .nzs = nzs,
        .pows = pows
    };

    aes_randinit(rng);
    for (int i = 0; i < nzs; ++i)
        pows[i] = 1;


    start = current_time();
    s = clt_state_new(&params, NULL, 0, flags, rng);
    if (s == NULL)
        goto cleanup;
    fprintf(stderr, "Parameter Generation Time: %.2fs\n", current_time() - start);

    {
        clt_elem_t *x;
        mpz_t zero[1];

        x = clt_elem_new();
        mpz_init_set_ui(zero[0], 0);

        fprintf(stderr, "Encoding...\n");
        start = current_time();
        clt_encode(x, s, 1, (const mpz_t *) zero, pows);
        fprintf(stderr, "Encoding Time: %.2fs\n", current_time() - start);

        {
            FILE *f = NULL;
            struct stat st;

            f = tmpfile();
            if (clt_elem_fwrite(x, f) == CLT_ERR) {
                fprintf(stderr, "Failed writing element to disk\n");
                return 1;
            }
            fstat(fileno(f), &st);
            fprintf(stderr, "Encoding Size: %ld KB\n", st.st_size / 1024);
            if (f)
                fclose(f);
        }
        clt_elem_free(x);
        mpz_clear(zero[0]);
    }

    clt_state_free(s);
    ret = 0;
cleanup:
    aes_randclear(rng);
    return ret;
}

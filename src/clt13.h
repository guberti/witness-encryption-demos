#ifndef __CLT13_H__
#define __CLT13_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CLT_OK 0
#define CLT_ERR (-1)

#include <aesrand.h>
#include <gmp.h>
#include <stdbool.h>

typedef struct clt_elem_t clt_elem_t;
typedef struct clt_state_t clt_state_t;
typedef struct clt_pp_t clt_pp_t;

/* Required parameters to clt_state_new */
typedef struct {
    /* security parameter */
    size_t lambda;
    /* multilinearity */
    size_t kappa;
    /* number of values in the index set */
    size_t nzs;
    /* powers of the index set values at the top level */
    int *pows;
} clt_params_t;

/* Optional parameters to clt_state_new */
typedef struct {
    /* minimum number of slots needed */
    size_t slots;
    /* plaintext moduli */
    mpz_t *moduli;
    /* number of plaintext moduli given */
    size_t nmoduli;
} clt_opt_params_t;

#define CLT_FLAG_NONE 0x00
/* Be verbose */
#define CLT_FLAG_VERBOSE 0x01

/* CLT optimization flags */

/* Use CRT tree optimization */
#define CLT_FLAG_OPT_CRT_TREE 0x02
/* Parallelize the encoding procedure */
#define CLT_FLAG_OPT_PARALLEL_ENCODE 0x04
/* Use composite primes (each "real" prime is the product of smaller primes) */
#define CLT_FLAG_OPT_COMPOSITE_PS 0x08

/* CLT security parameterization flags */

/* Use improved BKZ algorithm when generating attack estimates */
#define CLT_FLAG_SEC_IMPROVED_BKZ 0x10
/* Be conservative when generating attack estimates */
#define CLT_FLAG_SEC_CONSERVATIVE 0x20

#define CLT_FLAG_DEFAULT                        \
    ( CLT_FLAG_OPT_CRT_TREE                     \
      | CLT_FLAG_OPT_COMPOSITE_PS               \
      | CLT_FLAG_SEC_IMPROVED_BKZ               \
        )

clt_state_t * clt_state_new(const clt_params_t *params,
                            const clt_opt_params_t *opts, size_t ncores,
                            size_t flags, aes_randstate_t rng);
void          clt_state_free(clt_state_t *s);
clt_state_t * clt_state_fread(FILE *fp);
int           clt_state_fwrite(clt_state_t *s, FILE *fp);
mpz_t *       clt_state_moduli(const clt_state_t *s);
size_t        clt_state_nslots(const clt_state_t *s);
size_t        clt_state_nzs(const clt_state_t *s);

clt_pp_t * clt_pp_new(const clt_state_t *mmap);
void       clt_pp_free(clt_pp_t *pp);
clt_pp_t * clt_pp_fread(FILE *fp);
int        clt_pp_fwrite(clt_pp_t *pp, FILE *fp);

/* Creates an encoding `rop` using CLT state `s` of integers `xs` of length `n`
 * and index set `ix` of length `clt_state_nzs(s)` */
int clt_encode(clt_elem_t *rop, const clt_state_t *s, size_t n, const mpz_t *xs,
               const int *ix);
int clt_is_zero(const clt_elem_t *a, const clt_pp_t *pp);

clt_elem_t * clt_elem_new(void);
void         clt_elem_free(clt_elem_t *a);
clt_elem_t * clt_elem_copy(clt_elem_t *a);
void         clt_elem_set(clt_elem_t *a, const clt_elem_t *b);
int          clt_elem_add(clt_elem_t *rop, const clt_pp_t *pp, const clt_elem_t *a, const clt_elem_t *b);
int          clt_elem_sub(clt_elem_t *rop, const clt_pp_t *pp, const clt_elem_t *a, const clt_elem_t *b);
int          clt_elem_mul(clt_elem_t *rop, const clt_pp_t *pp, const clt_elem_t *a, const clt_elem_t *b);
int          clt_elem_mul_ui(clt_elem_t *rop, const clt_pp_t *pp, const clt_elem_t *a, unsigned int b);
void         clt_elem_print(const clt_elem_t *a);
int          clt_elem_fread(clt_elem_t *x, FILE *fp);
int          clt_elem_fwrite(clt_elem_t *x, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif

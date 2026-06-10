/*
 * Minimal CHOLMOD smoke test:
 *   build a sparse symmetric positive-definite matrix,
 *   factorize it, solve A x = b, and print x.
 *
 * Compile with:
 *   target_link_libraries(... SuiteSparse::CHOLMOD)
 */
#include <stdio.h>
#include <stdlib.h>
#include <cholmod.h>

int main(void)
{
    cholmod_common c;
    cholmod_start(&c);

    /* A = [ 4 1 ; 1 3 ]  (symmetric, stored as triplet, upper) */
    cholmod_triplet *T = cholmod_allocate_triplet(
        2, 2, 3, 1 /* stype upper */, CHOLMOD_REAL, &c);
    if (!T) { fprintf(stderr, "allocate_triplet failed\n"); return 1; }

    int    *Ti = (int *)    T->i;
    int    *Tj = (int *)    T->j;
    double *Tx = (double *) T->x;
    Ti[0] = 0; Tj[0] = 0; Tx[0] = 4.0;
    Ti[1] = 1; Tj[1] = 1; Tx[1] = 3.0;
    Ti[2] = 0; Tj[2] = 1; Tx[2] = 1.0;
    T->nnz = 3;

    cholmod_sparse *A = cholmod_triplet_to_sparse(T, 0, &c);
    cholmod_free_triplet(&T, &c);

    /* b = [ 1 ; 2 ] */
    cholmod_dense *b = cholmod_zeros(2, 1, CHOLMOD_REAL, &c);
    ((double *) b->x)[0] = 1.0;
    ((double *) b->x)[1] = 2.0;

    cholmod_factor *L = cholmod_analyze(A, &c);
    cholmod_factorize(A, L, &c);
    cholmod_dense  *x = cholmod_solve(CHOLMOD_A, L, b, &c);

    if (!x) { fprintf(stderr, "solve failed\n"); return 2; }
    double *xv = (double *) x->x;
    printf("CHOLMOD solve OK: x = [%.6f, %.6f]\n", xv[0], xv[1]);

    /* Sanity: expected x = A^-1 * b = [1/11, 7/11] approx [0.0909, 0.6364] */
    int ok = (xv[0] > 0.09 && xv[0] < 0.092) && (xv[1] > 0.635 && xv[1] < 0.637);

    cholmod_free_dense(&x, &c);
    cholmod_free_dense(&b, &c);
    cholmod_free_factor(&L, &c);
    cholmod_free_sparse(&A, &c);
    cholmod_finish(&c);

    return ok ? 0 : 3;
}

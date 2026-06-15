/* ecallib.h
 *
 * converted to C by John Pavan
 *
 * Adapted from:
 * _Data Reduction and Error Analysis for the Physical Sciences_
 * by Bevington
 */

/* --- includes --- */
// #include "gnuscopefuncs.h"

/* --- structs --- */

struct my_matrix_struct {
  int size;     // size of the matrix
  double *data; // elements of the matrix
};

/* --- globals --- */

/* --- function declarations --- */

double MatVal(int x, int y, struct my_matrix_struct *matrix);
double MatDet(struct my_matrix_struct *matrix);
struct my_matrix_struct *MatNew(int size);
void MatSetVal(int x, int y, double value, struct my_matrix_struct *matrix);
void PolChiSqr(double *coefs, int num_coefs, double *xpts, double *ypts, int num_pts, double *cs);
void MatDestroy(struct my_matrix_struct *matrix);
void PolFit(int nterms, double *xpts, double *ypts, double *sigpts, int num_pts, int mode,
            double *a);

/* ecallib.c
 * converted to C by John Pavan
 */

//--ddc aug11 remove unnecessary stuff #include <gtk/gtk.h>
#include <stdio.h>
#include <unistd.h>
//--ddc 3jun08 add missing function prototypes (stdlib)
#include <stdlib.h>
#include "ecallib.h"
#include <math.h>

/* --- functions --- */

/* PolFit
 *
 * Makes least-squares fit to data with a polynimal curve
 * y = a[0] + a[1] * x + a[2] * x^2 +...
 * Adapted from bevington, Data Red. & anal. for physicisits
 *
 * nterms is the number of coefficients (polynomial order +1)
 */
void PolFit(int nterms, double *xpts, double *ypts, double *sigpts, int num_pts, int mode,
            double *a) {
  double xterm, yterm, chisqr;
  double sumx[19], sumy[10];
  double sume[10];
  int i, j, k, l, m, n;
  double z;
  int nmax;
  double xi, yi;
  double weight, delta;
  double fre;
  int npts;
  double zterm;
  struct my_matrix_struct *array;

  array = (struct my_matrix_struct *)MatNew(nterms);

  nmax = 2 * nterms - 1;
  for (n = 0; n < nmax; n++)
    sumx[n] = 0;
  for (j = 0; j < nterms; j++) {
    sumy[j] = 0;
    sume[j] = 0;
  }

  chisqr = 0;
  for (i = 0; i < num_pts; i++) {
    xi = (double)xpts[i];
    yi = (double)ypts[i];
    z = pow(10, -2 * xi);

    if (mode < 0) {
      if (yi != 0) {
        weight = 1 / fabs(yi);
      } else {
        weight = 1;
      }
    }
    if (mode == 0) {
      weight = 1;
    }
    if (mode > 0) {
      weight = 1 / pow(sigpts[i], 2);
    }

    xterm = weight;
    for (n = 0; n < nmax; n++) {
      sumx[n] += xterm;
      xterm *= xi;
    }
    yterm = weight * yi;
    zterm = weight * z;
    for (n = 0; n < nterms; n++) {
      sumy[n] += yterm;
      sume[n] += zterm;
      yterm *= xi;
      zterm *= xi;
    }
    sume[4] += weight * z * z;
    if (mode == 2)
      sumy[4] += weight * yi * z;
    chisqr = chisqr + weight * yi * yi;
  }

  for (j = 0; j < nterms; j++) {
    for (k = 0; k < nterms; k++) {
      n = j + k;
      MatSetVal(j, k, sumx[n], array);
    }
  }

  if (mode == 2) {
    for (j = 0; j < 4; j++) {
      MatSetVal(j, 3, sume[j], array);
      MatSetVal(3, j, sume[j], array);
    }
  }

  delta = MatDet(array);
  if (delta == 0) {
    chisqr = 0;
    for (j = 0; j < nterms; j++) {
      a[j] = 0;
      goto stop;
    }
  }

  for (l = 0; l < nterms; l++) {
    for (j = 0; j < nterms; j++) {
      for (k = 0; k < nterms; k++) {
        n = j + k;
        MatSetVal(j, k, sumx[n], array);
        if ((mode == 2) && (j == 3))
          MatSetVal(j, k, sume[k], array);
        if ((mode == 2) && (k == 3))
          MatSetVal(j, k, sume[j], array);
      }
      MatSetVal(j, l, sumy[j], array);
    }
    a[l] = MatDet(array) / delta;
  }

  for (j = 0; j < nterms; j++) {
    chisqr = chisqr - 2 * a[j] * sumy[j];
    for (k = 0; k < nterms; k++) {
      n = j + k;
      chisqr = chisqr + a[j] * a[k] * sumx[n];
    }
  }
  fre = npts - nterms;
  chisqr = chisqr / fre;

stop:

  MatDestroy(array);
}

/* Poly
 *
 * Returns the y value of a polynomial given X, the coefficients, and the number of coeeficients
 */
double Poly(double X, double *coefs, int num_coefs) {
  int i;
  double Y;

  Y = 0;
  for (i = 0; i < num_coefs; i++) {
    Y += coefs[i] * pow(X, i);
  }
  return (Y);
}

/* PolChiSqr
 *
 * Returns the value of chisqr for the appropriate information
 */
void PolChiSqr(double *coefs, int num_coefs, double *xpts, double *ypts, int num_pts, double *cs) {
  int i, j, k;

  *cs = 0;
  for (i = 0; i < num_pts; i++) {
    *cs += pow((Poly(xpts[i], coefs, num_coefs) - ypts[i]), 2);
  }
}

/* MatDet
 *
 * Determines the determinate of a Matrix
 *
 * if the size is greater than 3 Laplace's method is used via recursion
 */
double MatDet(struct my_matrix_struct *matrix) {
  double temp;
  struct my_matrix_struct *tempmatrix;
  int i, j, k;
  int counter;
  int size;
  double array[10][10];

  size = matrix->size;

  if (size < 1)
    return (0);

  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++) {
      array[i][j] = MatVal(i, j, matrix);
    }
  }

  switch (size) {
  case 1:
    return (array[0][0]);
    break;
  case 2:
    temp = array[0][0] * array[1][1] - array[0][1] * array[1][0];
    return (temp);
    break;
  case 3:
    temp = array[0][0] * array[1][1] * array[2][2] + array[1][0] * array[2][1] * array[0][2] +
           array[2][0] * array[0][1] * array[1][2] - array[0][2] * array[1][1] * array[2][0] -
           array[0][0] * array[1][2] * array[2][1] - array[0][1] * array[1][0] * array[2][2];
    return (temp);
    break;
  default:
    /* --- this should be the case where size is greater than 3 --- */
    /* --- here we use Laplace's fromula --- */
    temp = 0;
    for (i = 0; i < size; i++) {
      /* --- have to make an array missing the top row and 1 column (i) --- */
      tempmatrix = MatNew(size - 1);
      counter = 0;
      for (j = 1; j < size; j++) {
        for (k = 0; k < size; k++) {
          if (k == i)
            k++; // skip the appropriate column
          MatSetVal(j, k, array[j][k], tempmatrix);
          counter++;
        }
      }
      temp = pow(-1, (i + 1)) * MatDet(tempmatrix);
    }
    MatDestroy(tempmatrix);
    return (temp);
    break;
  }
}

/* MatVal
 *
 * Returns the value of a matrix at a x and y
 */
double MatVal(int x, int y, struct my_matrix_struct *matrix) {
  double temp;
  int index;
  index = x + y * matrix->size;
  return (matrix->data[index]);
}

/* MatSetVal
 *
 * Sets the value of a matrix at x and y
 */
void MatSetVal(int x, int y, double value, struct my_matrix_struct *matrix) {
  int index;

  index = x + y * matrix->size;
  matrix->data[index] = value;
}

/* MatNew
 *
 * creates a matrix of a particluar size
 */
struct my_matrix_struct *MatNew(int size) {
  int i;
  struct my_matrix_struct *matrix;

  matrix = (struct my_matrix_struct *)malloc(sizeof(struct my_matrix_struct));

  matrix->size = size;
  matrix->data = (double *)malloc(sizeof(double) * size * size);
  for (i = 0; i < (size * size); i++)
    matrix->data[i] = 0;
  return (matrix);
}

void MatDestroy(struct my_matrix_struct *matrix) {
  //--ddc  aug11 g_free(matrix->data);
  free(matrix->data);
  //--ddc aug11 g_free(matrix);
  free(matrix);
}

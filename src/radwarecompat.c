/* radwarecompat.c
 *
 * For exporting the floating point matricies from pgamsort
 * to radware
 *
 * by John Pavan
 */


//--ddc 10apr09 problems with the 'radware mat' files.  These were not
//--ddc being written out in the size that a radware mat file must be,
//--ddc for a triangle matrix.

/* --- includes --- */

//#include "pgam.h"
#include "gnuscopefuncs.h"
#include "gnuscopeglobals.h"
//#include <stdio.h>
//#include <string.h>
#include <sys/stat.h>

/* --- structs --- */

/* --- globals --- */

struct pgam_matrix_info pgammatrixdata;  // the pgammatrix data from pgammatrix.c
//int radwarematrixsize; // the size for radware matricies (no header)

/* --- function declarations --- */


/* --- functions --- */

/* WriteRadwareMatrix
 *
 * determine if we want to write a 
 * symmetric or an assymetric matrix (from ints or floats)
 */
void WriteRadwareMatrix(char *sFilename)
{
  switch(twoddata.filetype) {
  case 1:
    /* --- symmetric matrix --- */
    WriteRadwareIntTriangle(sFilename);
    break;
  case 2:
    /* --- square matrix --- */
    WriteRadwareIntSquare(sFilename);
    break;
  default:
    switch(pgammatrixdata.type) {
    case 0:
    WriteRadwareSquare(sFilename);
    break;
    case 3:
      WriteRadwareTriangle(sFilename);
      break;
    default:
      printf("Error determining type of matrix.\n");
    }
    break;
  }
}

/* WriteRadwareSquare
 *
 * Writes out a square for radware to read in
 */
void WriteRadwareSquare(char *sFilename)
{
  /* --- local variables --- */

  FILE *outfile;
  int i,j,k;
  short int *outbuffer;
  
  /* --- we'll start with the dumb way --- */

  if ((outfile = fopen(sFilename,"wb")) != NULL) {
    /* --- we have successfully opened the file --- */
    /* --- ok, we will want to do this export via buffered writting
       --- therefore we will need a output buffer --- */
    outbuffer = (short int *) malloc(sizeof(short int) * radwarematrixsize);
    for (i = 0; i < radwarematrixsize; i++) {
      outbuffer[i] = 0;
    }

    /* --- ok, now we need to enter the file writing loop --- */
    /* --- we are going to simply conver the floats in pgammatrixdata.data
       --- to short ints via the outbuffer.  At the end of each row they will be
       --- written out --- */
    for (i = 0; (i < radwarematrixsize); i++) {
      if (i < pgammatrixdata.size) {
	for (j = 0; (j < radwarematrixsize) && ( j < pgammatrixdata.size); j++) {
	  outbuffer[j] = (short int) *(pgammatrixdata.data + pgammatrixdata.size * i + j);
	}
      }
      fwrite(outbuffer,sizeof(short int),radwarematrixsize,outfile);
      for (j = 0; (j < radwarematrixsize); j++) 
	outbuffer[j] = 0;
    }

    /* --- we need to free the memory from the outbuffer before exiting the function --- */
    free(outbuffer);
    /* --- need to close the outfile before we exit the function --- */
    fclose(outfile);
  }

}

/* WriteRadwareTriangle
 *
 * outputs a symmetric matrix (triangle) from the floating point
 * pgamsort format to the symmetric file format for radware
 */
void WriteRadwareTriangle(char *sFilename)
{

  /* --- local variables --- */

  FILE *outfile;
  int i,j,k;
  int ix,iy;
  int isize;
  int rsize;
  int index;
  short int *outbuffer;
  
  isize = 2 * pgammatrixdata.size + 1;
  rsize = 2 * radwarematrixsize + 1;

  if ((outfile = fopen(sFilename,"wb")) != NULL) {
    /* --- we have successfully opened the file --- */
    /* --- ok, we will want to do this export via buffered writting
       --- therefore we will need a output buffer --- */
    outbuffer = (short int *) malloc(sizeof(short int) * radwarematrixsize);
    for (i = 0; i < radwarematrixsize; i++) {
      outbuffer[i] = 0;
    }

    /* --- ok, now we need to enter the file writing loop --- */
    /* --- we are going to simply conver the floats in pgammatrixdata.data
       --- to short ints via the outbuffer.  At the end of each row they will be
       --- written out --- */
    for (i = 0; (i < radwarematrixsize); i++) {
      if (i < pgammatrixdata.size) {
	for (j = 0; (j < radwarematrixsize) && ( j < pgammatrixdata.size); j++) {
	  ix = Max(i,j);
	  iy = Min(i,j);
	  index = ix - iy +  ( isize - iy) * ((float) iy / (float) 2);
	  outbuffer[j] = (short int) *(pgammatrixdata.data + index);
	}
      }
      //--ddc 10apr09 THIS->
      //fwrite(outbuffer,sizeof(short int),radwarematrixsize - i,outfile);
      //Does NOT write out a matrix the size of a radware matrix...
      fwrite(outbuffer,sizeof(short int),radwarematrixsize,outfile);
      for (j = 0; (j < radwarematrixsize); j++) 
	outbuffer[j] = 0;
    }

    /* --- we need to free the memory from the outbuffer before exiting the function --- */
    free(outbuffer);
    /* --- need to close the outfile before we exit the function --- */
    fclose(outfile);
  }
}

/* WriteRadwareIntTriangle
 *
 * Writes out an integer triangle to a radware mat file
 */
void WriteRadwareIntTriangle(char *sFilename)
{
 

  /* --- local variables --- */

  FILE *outfile;
  int i,j,k;
  int ix,iy;
  int isize;
  int rsize;
  int index;
  short int *outbuffer;
  
  isize = 2 * twoddata.size + 1;
  rsize = 2 * radwarematrixsize + 1;

  if ((outfile = fopen(sFilename,"wb")) != NULL) {
    /* --- we have successfully opened the file --- */
    /* --- ok, we will want to do this export via buffered writting
       --- therefore we will need a output buffer --- */
    outbuffer = (short int *) malloc(sizeof(short int) * radwarematrixsize);
    for (i = 0; i < radwarematrixsize; i++) {
      outbuffer[i] = 0;
    }

    /* --- ok, now we need to enter the file writing loop --- */
    /* --- we are going to simply conver the floats in twoddata.data
       --- to short ints via the outbuffer.  At the end of each row they will be
       --- written out --- */
    for (i = 0; (i < radwarematrixsize); i++) {
      if (i < twoddata.size) {
	for (j = 0; (j < radwarematrixsize) && ( j < twoddata.size); j++) {
	  ix = Max(i,j);
	  iy = Min(i,j);
	  index = ix - iy +  ( isize - iy) * ((float) iy / (float) 2);
	  outbuffer[j] = (short int) *(twoddata.data + index);
	}
      }
      //--ddc 10apr09 THIS->
      //fwrite(outbuffer,sizeof(short int),radwarematrixsize - i,outfile);
      //Does NOT write out a matrix the size of a radware matrix...
      fwrite(outbuffer,sizeof(short int),radwarematrixsize,outfile);
      for (j = 0; (j < radwarematrixsize); j++) 
	outbuffer[j] = 0;
    }

    /* --- we need to free the memory from the outbuffer before exiting the function --- */
    free(outbuffer);
    /* --- need to close the outfile before we exit the function --- */
    fclose(outfile);
  }
}

/* WriteRadwareIntSquare 
 *
 * Writes out the ints in memory to a square .mat file
 */
void WriteRadwareIntSquare(char *sFilename)
{

  /* --- local variables --- */

  FILE *outfile;
  int i,j,k;
  short int *outbuffer;
  
  /* --- we'll start with the dumb way --- */

  if ((outfile = fopen(sFilename,"wb")) != NULL) {
    /* --- we have successfully opened the file --- */
    /* --- ok, we will want to do this export via buffered writting
       --- therefore we will need a output buffer --- */
    outbuffer = (short int *) malloc(sizeof(short int) * radwarematrixsize);
    for (i = 0; i < radwarematrixsize; i++) {
      outbuffer[i] = 0;
    }

    /* --- ok, now we need to enter the file writing loop --- */
    /* --- we are going to simply conver the floats in twoddata.data
       --- to short ints via the outbuffer.  At the end of each row they will be
       --- written out --- */
    for (i = 0; (i < radwarematrixsize); i++) {
      if (i < twoddata.size) {
	for (j = 0; (j < radwarematrixsize) && ( j < twoddata.size); j++) {
	  outbuffer[j] = (short int) *(twoddata.data + twoddata.size * i + j);
	}
      }
      fwrite(outbuffer,sizeof(short int),radwarematrixsize,outfile);
      for (j = 0; (j < radwarematrixsize); j++) 
	outbuffer[j] = 0;
    }

    /* --- we need to free the memory from the outbuffer before exiting the function --- */
    free(outbuffer);
    /* --- need to close the outfile before we exit the function --- */
    fclose(outfile);
  }
}

/* ReadRadwareMatrix
 *
 * Determines the size of a radware matrix
 * assumes matrix is square, and made of short ints
 */
void ReadRadwareMatrix(char *sFilename)
{
  struct stat filebuffer;
  long int filelength = 0;
  int matsize;
  FILE *infile;

  /* --- determine the length of the file --- */
  stat(sFilename,&filebuffer);
  filelength = filebuffer.st_size;

  /* --- not figure out the dimensions of the matrix --- */
  matsize = sqrt(filelength / sizeof(short int));

  /* --- ok, now let's read stuff in --- */
  if ((infile = fopen(sFilename,"rb")) != NULL) {
    twoddata.filetype = 2;
    twoddata.headbuffer[0] = twoddata.headbuffer[1] = 12;
    twoddata.size = matsize;
    twoddata.type = 3;
    twoddata.df = 0;
    twoddata.databuffer[0] = twoddata.databuffer[1] = filelength;
    if (twoddata.data == NULL) {
      twoddata.data = (short int *) malloc(filelength);
    } else {
      twoddata.data = (short int *) realloc(twoddata.data,filelength);
    }
    fread(twoddata.data,1,filelength,infile);
    fclose(infile);
  }

}

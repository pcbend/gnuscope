#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>

/* everything assumes default size for int is 4bytes */

struct ornlheader {
  int id;
  char label[12];
  char timestamp[12];
  int bytes;          /* always -4, :) no longer used */
  int length;         /* header length, in 2byte units,64 */
  int datalength;     /* no longer used */
  int dimensions;     /* no longer used (but always 1!) */
  int channels;       /* number of channels in histograms */
  int raw;            /* length of raw parameter, power of 2 */
  int scaled;         /* length of scaled parameter, power of 2 */
  int record;         /* record length (0 if all data in one record)*/
  int minchan;        /* min non-zero channel number */
  int maxchan;        /* max non-zero channel number */
  int calibration[3]; /* calibration constants */
  char reserved[8];   /* reserved space... for future? */
  char title[40];     /* histogram title */
};
struct ornldir {
  char datatype[8]; /*HIRRF */
  int numspec;      /*the number of spectra in the file */
  int nextrec;      /*location next "part" of the file in halfwords*/
  struct {
    int id;       /*the spectrum id number */
    int location; /*location in file, in halfwords */
  } entry[254];   /*the identity and location of each spectra*/
};
int ornlfile;

/***/

/*
oops, in the fsu stuff, there is integer data that overlaps byte boundaries
(due to character strings which are indivisable by four).  The C compiler
tries to avoid this by skipping some bytes before assigning space
for the following integer... but of course this raises hell with the
fread/fwrite, which take the stream of data without regard to "boundaries".
The solution is to make special functions to read/write the data (yuck).
*/

struct fsudir {
  int buffsize;      /*fortran file, each record starts with record size */
  short int numrun;  /*the run number*/
  short int numspec; /*the number of spectra in the file*/
  int trutim;        /*the ACTUAL truetime */
  char dat[9];       /*date field*/
  char tim[8];       /*time field*/
  int buffcheck;     /*fortran file, record ends with record size :) */
};

struct fsuheader {
  int buffsize; /*fortran file, each record starts with record size */
  //--ddc apr12
  unsigned short int numchn; /*number channels in histogram*/
  short int itype;           /*type of histogram... must be 2! */
  char label[6];             /*histogram label*/
  int truet;                 /*truetime for ADC, or livetime*/
  int buffcheck;             /*fortran file, record ends with record size :) */
};

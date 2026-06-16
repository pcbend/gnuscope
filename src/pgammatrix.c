/* pgammatricies.c
 *
 * by John Pavan
 *
 * For use with gnuscope
 *
 * contains the algorythims and the functions and subroutines
 * for dealing with the two-dimensional matricies
 * such as a particle-gamma square or a gamma-gamma triangle
 *
 * these differ from the ones for use with gs92 because
 * they are composed of floats instead of short ints.
 * this means they take up much more memory
 */

/* --- includes --- */

//--ddc 3jun08 (this duplicated in gnuscopefuncs.h) #include "pgam.h"
#include <stdio.h>
#include <sys/types.h>
#include <math.h>
#include <gtk/gtk.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "gnuscopeglobals.h"
#include "gnuscopefuncs.h"
// #include <string.h>

/* --- structs --- */

// the struct for storing the data is in pgam.h

/* --- globals --- */

struct pgam_matrix_info pgammatrixdata;
// int histid[254],histsize[254],*histloc[254];
// char field1[254][40],field2[254][40],field3[254][40];
int spectra;
int currentrange[2];

/* --- function delcarations --- */

void ReadTwdAdd(char *sFilename);
void ReadSqrAdd(char *sFilename);
void ReadPgamAdd(char *sFilename);
void ReadPgamTwod(char *sfilename);
void WritePgamTwod(char *sfilename);
void PgamTwodProjectSqr(int addlower, int addupper, int subtractlower, int subtractupper,
                        int histdest, int add, int xory);
void PgamTwodProjectTwd(int addlower, int addupper, int subtractlower, int subtractupper,
                        int histdest, int add);
void ReadAutoMatrix(char *sFilename);
void WriteAutoMatrix(char *sFilename);

//--ddc 19jul07 add next
int GetLastSpectrum(void);
//--ddc 3jun08 add...(for gs92 functions)
void WriteTwd(FILE *);
void WriteSqr(FILE *);

/* --- functions --- */

/* AddAutoMatrix
 *
 * Adds to whatever matrix we have in memory
 */
void AddAutoMatrix(char *sFilename) {
  char dummystr[120];
  FILE *infile;
  int infiletype, existingfiletype;
  int tempbuffer[4];
  int buffersize[2];
  int databuffersize[2];
  int matrixtype;

  /* --- just like when we read in a matrix without adding to
     --- it we are going to need to use the data size to
     --- determine the type of matrix --- */
  /* --- for more information on how this is done look at the
     --- comments at the beginning of ReadAutoMatrix
     --- later in this file --- */

  /* --- guess the first question about this is, do we have
     --- another matrix in memory --- */
  /* --- if we don't, we should just call ReadAutoMatrix --- */
  if ((twoddata.data == NULL) && (pgammatrixdata.data == NULL))
    ReadAutoMatrix(sFilename);
  else {

    /* --- guess at this point we know that we have another
       --- matrix in memory, let's figure out what type --- */
    if (twoddata.data != NULL) {
      /* --- must be a short int matrix --- */
      if (twoddata.filetype == 1)
        existingfiletype = 1;
      if (twoddata.filetype == 2)
        existingfiletype = 2;
    } else {
      /* --- must be a floating point matrix --- */
      if (pgammatrixdata.type == 0)
        existingfiletype = 4;
      if (pgammatrixdata.type == 3)
        existingfiletype = 3;
    }
    /* --- ok, now let's start reading the file --- */
    if ((infile = fopen(sFilename, "rb")) != NULL) {
      /* --- now let's read the headbuffer --- */
      fread(&buffersize[0], 4, 1, infile);
      fread(tempbuffer, 1, buffersize[0], infile);
      fread(&buffersize[1], 4, 1, infile);
      if (buffersize[0] == buffersize[1]) {
        fread(&databuffersize[0], 4, 1, infile);
        if (databuffersize[0] == (sizeof(short int) * tempbuffer[0] * tempbuffer[0])) {
          /* --- must be short int square --- */
          matrixtype = 2;
        } else if (databuffersize[0] == (sizeof(float) * tempbuffer[0] * tempbuffer[0])) {
          /* --- must be a float square --- */
          matrixtype = 4;
        } else if (databuffersize[0] ==
                   (sizeof(short int) * tempbuffer[0] * (tempbuffer[0] + 1) / 2)) {
          /* --- must be a short int triangle --- */
          matrixtype = 1;
        } else if (databuffersize[0] == (sizeof(float) * tempbuffer[0] * (tempbuffer[0] + 1) / 2)) {
          /* --- must be a floating point triangle --- */
          matrixtype = 3;
        }
        /* --- ok, now we know the existing matrix type
           --- and the matrix type, they must be the same --- */
        fclose(infile);
        if (existingfiletype == matrixtype) {
          switch (matrixtype) {
          case 1:
            /* --- short int triangle --- */
            ReadTwdAdd(sFilename);
            break;
          case 2:
            /* --- short int square --- */
            ReadSqrAdd(sFilename);
            break;
          case 3:
            /* --- floating point triangle --- */
            ReadPgamAdd(sFilename);
            break;
          case 4:
            /* --- floating point square --- */
            ReadPgamAdd(sFilename);
            break;
          default:
            GetMessageDialog("Error determing the matrix types.\n");
            break;
          }
        } else
          GetMessageDialog("Error: Incompatable matrix types.\n");
      } else
        GetMessageDialog("Error reading file header.\n");
    }
  }
}

/*
 * WriteAutoMatrix
 *
 * Writes out whatever matrix we have in mememory
 */
void WriteAutoMatrix(char *sFilename) {
  // ddc 7jun06 sFilename never has enough memory to add an extension!!!
  // ddc 7jun06 so, we use (the previously unused) dummystr
  char dummystr[MAXPATH];
  FILE *outfile;

  // ddc 7jun06 sFilename never has enough memory to add an extension!!!
  dummystr[0] = '\0';
  if (strnlen(sFilename, MAXPATH - 5) > MAXPATH - 5) {
    WriteMainText("Output filename too long, abort write.");
  }
  strncat(dummystr, sFilename, MAXPATH - 5);

  if ((strstr(sFilename, ".mat") != NULL) || (strstr(sFilename, ".MAT") != NULL)) {
    WriteRadwareMatrix(sFilename);
  }
  switch (twoddata.filetype) {
  case 1:
    /* --- twod --- */
    if (strstr(sFilename, ".twd") == NULL)
      // ddc 7jun06 sFilename never has enough memory to add an extension!!!
      //      strcat(sFilename,".twd");
      strcat(dummystr, ".twd");
    break;
  case 2:
    /* --- sqr --- */
    if (strstr(sFilename, ".sqr") == NULL)
      // ddc 7jun06 sFilename never has enough memory to add an extension!!!
      //      strcat(sFilename,".sqr");
      strcat(dummystr, ".sqr");
    break;
  default:
    break;
  }

  //--ddc 07jun06 replace sFilename with 'dummystr' from here on.

  switch (twoddata.filetype) {
  case 1:
    /* --- short int twd --- */
    if ((outfile = fopen(dummystr, "w")) != NULL)
      WriteTwd(outfile);
    fclose(outfile);
    break;
  case 2:
    /* --- short int sqr --- */
    if ((outfile = fopen(dummystr, "w")) != NULL)
      WriteSqr(outfile);
    fclose(outfile);
    break;
  default:
    /* --- a float matrix --- */
    if ((pgammatrixdata.type == 0) && (strstr(dummystr, ".sqr") == NULL))
      strcat(dummystr, ".sqr");
    if ((pgammatrixdata.type == 3) && (strstr(dummystr, ".twd") == NULL))
      strcat(dummystr, ".twd");
    WritePgamTwod(dummystr);
    break;
  }
}

/* ReadAutoMatrix
 *
 * Automatically figures out the type of matrix
 * Accepts sFilename as the only input
 *
 * if buffersize is 16 then it is a short int twd
 * if what != 0 then it is a short in sqr
 * if type == 3 it is a float twd
 * otherwise it is a float sqr
 */
void ReadAutoMatrix(char *sFilename) {
  char dummystr[80];
  FILE *infile;
  int tempbuffer[4];
  int size;
  int buffersize[2];
  int databuffersize[2];
  int matrixtype;

  /* --- ok for determining the file type we should try to look
     --- at the size of the "data" compaired to the size defined
     --- in the header --- */
  /* --- for instance, a short int square would have size
     --- sizeof(short int) * size * size ---
     --- floating point square would be of size
     --- sizeof(float) * size * size ---
     --- short int triangle would be of size
     --- sizeof(short int) * 1/2 * size * (size + 1) ---
     --- and a floating point triangle would be
     --- sizeof (float) * 1/2 * size * (size + 1) --- */
  /* --- note that this will not work if anyone ever makes
     --- a matrix of ints --- */

  /* --- first we have to open the file --- */
  if ((infile = fopen(sFilename, "r")) != NULL) {
    /* --- first we get the buffersize --- */
    fread(&buffersize[0], 4, 1, infile);
    /* --- might as well read in the rest of the buffer now --- */
    fread(tempbuffer, 1, buffersize[0], infile);
    /* --- ok the length of the side of the matrix is defined as the first part of the tempbuffer
     * --- */
    /* --- let's read the end of the headbuffer
       --- and the size of the databuffer --- */
    fread(&buffersize[1], sizeof(int), 1, infile);
    if (buffersize[1] == buffersize[0]) {
      /* --- now we readin the first part of the databuffer --- */
      fread(&databuffersize[0], sizeof(int), 1, infile);
      /* --- now we can use this and the size
         --- from the header (tempbuffer[0])
         --- to determine the type of the file --- */
      if (databuffersize[0] == (sizeof(short int) * tempbuffer[0] * tempbuffer[0])) {
        /* --- it is a square matrix made of short ints --- */
        matrixtype = 2;
      } else if (databuffersize[0] == (sizeof(float) * tempbuffer[0] * tempbuffer[0])) {
        /* --- it is a square matrix made of floats --- */
        matrixtype = 4;
      } else if (databuffersize[0] ==
                 (sizeof(short int) * tempbuffer[0] * (tempbuffer[0] + 1) / 2)) {
        /* --- it is a triangle made of short ints --- */
        matrixtype = 1;
      } else if (databuffersize[0] == (sizeof(float) * tempbuffer[0] * (tempbuffer[0] + 1) / 2)) {
        /* --- it is a triangle made of floats --- */
        matrixtype = 3;
      }
      /* --- now let's deal with each of the matrix types --- */
      switch (matrixtype) {
      case 1:
        /* --- case of a short int triangle --- */
        twoddata.filetype = 1;
        if (twoddata.data != NULL) {
          free(twoddata.data);
          twoddata.data = NULL;
        }
        twoddata.headbuffer[0] = buffersize[0];
        twoddata.size = tempbuffer[0];
        twoddata.type = tempbuffer[2];
        twoddata.df = tempbuffer[3];
        twoddata.what = tempbuffer[1];
        twoddata.headbuffer[1] = buffersize[1];
        twoddata.databuffer[0] = databuffersize[0];
        twoddata.data = (short int *)malloc(twoddata.databuffer[0]);
        fread(twoddata.data, 1, twoddata.databuffer[0], infile);
        fread(&twoddata.databuffer[1], sizeof(int), 1, infile);
        if (twoddata.databuffer[1] != twoddata.databuffer[0]) {
          GetMessageDialog("Error reading matrix data from file.\n");
        }
        fclose(infile);
        break;
      case 2:
        /* --- case of a short int square --- */
        twoddata.filetype = 2;
        twoddata.headbuffer[0] = buffersize[0];
        twoddata.headbuffer[1] = buffersize[1];
        twoddata.size = tempbuffer[0];
        twoddata.type = tempbuffer[1];
        twoddata.df = tempbuffer[2];
        twoddata.databuffer[0] = databuffersize[0];
        if (twoddata.data == NULL)
          twoddata.data = (short int *)malloc(twoddata.databuffer[0]);
        else
          twoddata.data = (short int *)realloc(twoddata.data, twoddata.databuffer[0]);
        fread(twoddata.data, 1, twoddata.databuffer[0], infile);
        fread(&twoddata.databuffer[1], sizeof(int), 1, infile);
        if (twoddata.databuffer[0] != twoddata.databuffer[1]) {
          GetMessageDialog("Error Reading matrix data from file.\n");
        }
        fclose(infile);
        break;
      case 3:
        /* --- case of a float triangle --- */
        pgammatrixdata.headbuffer[0] = buffersize[0];
        pgammatrixdata.headbuffer[1] = buffersize[1];
        pgammatrixdata.size = tempbuffer[0];
        pgammatrixdata.type = 3;
        pgammatrixdata.what = tempbuffer[1];
        pgammatrixdata.databuffer[0] = databuffersize[0];
        if (pgammatrixdata.data == NULL)
          pgammatrixdata.data = (float *)malloc(pgammatrixdata.databuffer[0]);
        else
          pgammatrixdata.data = (float *)realloc(pgammatrixdata.data, pgammatrixdata.databuffer[0]);
        fread(pgammatrixdata.data, 1, pgammatrixdata.databuffer[0], infile);
        fread(&pgammatrixdata.databuffer[1], sizeof(int), 1, infile);
        if (pgammatrixdata.databuffer[0] != pgammatrixdata.databuffer[1])
          GetMessageDialog("Error reading matrix data from file.\n");
        fclose(infile);
        break;
      case 4:
        /* --- case of a float square --- */
        pgammatrixdata.headbuffer[0] = buffersize[0];
        pgammatrixdata.headbuffer[1] = buffersize[1];
        pgammatrixdata.size = tempbuffer[0];
        pgammatrixdata.type = 0;
        pgammatrixdata.what = tempbuffer[1];
        pgammatrixdata.databuffer[0] = databuffersize[0];
        if (pgammatrixdata.data == NULL)
          pgammatrixdata.data = (float *)malloc(pgammatrixdata.databuffer[0]);
        else
          pgammatrixdata.data = (float *)realloc(pgammatrixdata.data, pgammatrixdata.databuffer[0]);
        fread(pgammatrixdata.data, 1, pgammatrixdata.databuffer[0], infile);
        fread(&pgammatrixdata.databuffer[1], sizeof(int), 1, infile);
        if (pgammatrixdata.databuffer[0] != pgammatrixdata.databuffer[1])
          GetMessageDialog("Error reading matrix data from file.\n");
        fclose(infile);
        break;
      default:
        printf("Error determining Matrix Type.\n");
        GetMessageDialog("Error determining matrix type.\n");
        break;
      };

    } else {
      GetMessageDialog("Error Opening Matrix File.\n");
    }
  }
}

/* ReadPgamAdd
 *
 * Adds a pgam matrix from disk to the one currently in memory
 */
void ReadPgamAdd(char *sFilename) {
  char dummystr[120];
  FILE *infile;
  int headbuffer[2];
  int databuffer[2];
  int buffer1[4];
  float *buffer2 = NULL;
  int i;

  /* --- see if we already have something in memory --- */
  if (pgammatrixdata.data != NULL) {
    /* --- start by opening the file and reading the data in it --- */
    if ((infile = fopen(sFilename, "r")) != NULL) {
      fread(&headbuffer[0], 4, 1, infile);
      fread(buffer1, 12, 1, infile);
      fread(&headbuffer[1], 4, 1, infile);
      if (headbuffer[1] != headbuffer[0]) {
        sprintf(dummystr, "Error reading header for %s\n", sFilename);
        GetMessageDialog(dummystr);
      }
      fread(&databuffer[0], 4, 1, infile);
      if ((buffer2 = (float *)malloc(databuffer[0])) != NULL) {
        fread(buffer2, databuffer[0], 1, infile);
      }
      fread(&databuffer[1], 4, 1, infile);
      if (databuffer[0] != databuffer[1]) {
        sprintf(dummystr, "Error reading data from %s.\n", sFilename);
        GetMessageDialog(dummystr);
      }
      fclose(infile);

      /* --- now we need to add the data from temporary buffer, to that in the main matrix --- */
      for (i = 0; i < (databuffer[0] / sizeof(float)); i++) {
        pgammatrixdata.data[i] += buffer2[i];
      }
      if (buffer2 != NULL)
        free(buffer2);
    }
  } else {
    /* --- case where we don't already have something in memory --- */
    ReadPgamTwod(sFilename);
  }
}

//--ddc 13apr09 for background subtraction.

/* ReadPgamSub
 *
 * Subtract a pgam matrix from disk to the one currently in memory
 */
void ReadPgamSub(char *sFilename) {
  char dummystr[120];
  FILE *infile;
  int headbuffer[2];
  int databuffer[2];
  int buffer1[4];
  float *buffer2 = NULL;
  int i;

  /* --- see if we already have something in memory --- */
  if (pgammatrixdata.data != NULL) {
    /* --- start by opening the file and reading the data in it --- */
    if ((infile = fopen(sFilename, "r")) != NULL) {
      fread(&headbuffer[0], 4, 1, infile);
      fread(buffer1, 12, 1, infile);
      fread(&headbuffer[1], 4, 1, infile);
      if (headbuffer[1] != headbuffer[0]) {
        sprintf(dummystr, "Error reading header for %s\n", sFilename);
        GetMessageDialog(dummystr);
      }
      fread(&databuffer[0], 4, 1, infile);
      if ((buffer2 = (float *)malloc(databuffer[0])) != NULL) {
        fread(buffer2, databuffer[0], 1, infile);
      }
      fread(&databuffer[1], 4, 1, infile);
      if (databuffer[0] != databuffer[1]) {
        sprintf(dummystr, "Error reading data from %s.\n", sFilename);
        GetMessageDialog(dummystr);
      }
      fclose(infile);

      /* --- now we need to add the data from temporary buffer, to that in the main matrix --- */
      for (i = 0; i < (databuffer[0] / sizeof(float)); i++) {
        pgammatrixdata.data[i] -= buffer2[i];
      }
      if (buffer2 != NULL)
        free(buffer2);
    }
  } else {
    /* --- case where we don't already have something in memory --- */
    ReadPgamTwod(sFilename);
    for (i = 0; i < (databuffer[0] / sizeof(float)); i++) {
      pgammatrixdata.data[i] = -1 * pgammatrixdata.data[i];
    }
  }
}

/* ReadPgamTwod
 *
 * Read in the PgamTwoddata from the file sFilename
 */
void ReadPgamTwod(char *sFilename) {
  char dummystr[80];
  FILE *infile;

  /* --- open and read in the file --- */
  if ((infile = fopen(sFilename, "r")) != NULL) {
    fread(&pgammatrixdata.headbuffer[0], 4, 1, infile);
    fread(&pgammatrixdata.size, 4, 1, infile);
    fread(&pgammatrixdata.what, 4, 1, infile);
    fread(&pgammatrixdata.type, 4, 1, infile);
    fread(&pgammatrixdata.headbuffer[1], 4, 1, infile);
    /* --- make sure that the two head buffer's agree --- */
    if (pgammatrixdata.headbuffer[0] != pgammatrixdata.headbuffer[1]) {
      GetMessageDialog("Error reading file header.\n");
      goto closefile;
    }
    /* --- if the header was read in correctly, let's get the data --- */
    fread(&pgammatrixdata.databuffer[0], 4, 1, infile);
    /* --- allocate memory for the data --- */
    if (pgammatrixdata.data == NULL) {
      pgammatrixdata.data = (float *)malloc(pgammatrixdata.databuffer[0]);
    } else {
      pgammatrixdata.data = (float *)realloc(pgammatrixdata.data, pgammatrixdata.databuffer[0]);
    }
    /* --- read in the data --- */
    fread(pgammatrixdata.data, 1, pgammatrixdata.databuffer[0], infile);
    fread(&pgammatrixdata.databuffer[1], 4, 1, infile);
    /* --- make sure that the data buffer's agree --- */
    if (pgammatrixdata.databuffer[0] != pgammatrixdata.databuffer[1])
      GetMessageDialog("Error reading data from file.");

  closefile:
    fclose(infile);
  } // done
}

/* WritePgamTwod
 *
 * Writes the pgamtwoddata from memory to file sFilename
 */
void WritePgamTwod(char *sFilename) {
  char dummystr[80];
  FILE *outfile;

  g_return_if_fail(pgammatrixdata.data != NULL);

  /* --- open the file for writing --- */
  if ((outfile = fopen(sFilename, "w")) != NULL) {
    fwrite(&pgammatrixdata.headbuffer[0], 4, 1, outfile);
    fwrite(&pgammatrixdata.size, 4, 1, outfile);
    fwrite(&pgammatrixdata.what, 4, 1, outfile);
    fwrite(&pgammatrixdata.type, 4, 1, outfile);
    fwrite(&pgammatrixdata.headbuffer[1], 4, 1, outfile);
    fwrite(&pgammatrixdata.databuffer[0], 4, 1, outfile);
    fwrite(pgammatrixdata.data, 1, pgammatrixdata.databuffer[0], outfile);
    fwrite(&pgammatrixdata.databuffer[1], 4, 1, outfile);
    fclose(outfile);
  } else {
    //--ddc 23jul07 Add next, let's throw an error if we didn't open the file.
    g_return_if_fail(outfile == NULL);
  }
}

/* PgamTwodProjectFull
 *
 * projects to the histograms from the data based on the data type
 */
void PgamTwodProjectFull() {

  //--ddc 19jul07 annoying to have histograms overwritten by full projection.
  // i've add histdest here, and will make call to get the value of the
  // last histid to use in the projection calls.
  int histdest;

  histdest = GetLastSpectrum() + 1;

  /* --- only do this if we have data --- */
  g_return_if_fail(pgammatrixdata.data != NULL);

  switch (pgammatrixdata.type) {
  case 0:
    /* --- this is the case of the square --- */
    //    PgamTwodProjectSqr(0,(pgammatrixdata.size - 1),0,0,0,0,0);
    PgamTwodProjectSqr(0, (pgammatrixdata.size - 1), 0, 0, histdest, 0, 0);
    histdest++;
    //    PgamTwodProjectSqr(0,(pgammatrixdata.size - 1),0,0,1,0,1);
    PgamTwodProjectSqr(0, (pgammatrixdata.size - 1), 0, 0, histdest, 0, 1);
    break;
  case 3:
    /* --- this is the case of the triangel --- */
    //    PgamTwodProjectTwd(0,(pgammatrixdata.size - 1),0,0,0,0);
    PgamTwodProjectTwd(0, (pgammatrixdata.size - 1), 0, 0, histdest, 0);
    break;
  default:
    /* --- do nothing --- */
    break;
  }
}

/* PgamTwodProjectSqr
 *
 * projects a from a twod square
 */
void PgamTwodProjectSqr(int addlower, int addupper, int subtractlower, int subtractupper,
                        int histdest, int add, int xory) {
  /* --- local variables --- */
  //--ddc eliminate floats (to reduce rounding errors) and initialize frac
  //--ddc jun17 replace int with 'long long' to keep sums in spectra from
  //--ddc jun17 overlflowing for counts and subcounts

  int i, j, k;
  double temphist[16384];
  double subhist[16384];
  int newmemsize, lasthist;
  //--ddc jun17  long int subcounts;
  long long subcounts;
  double frac = 0;
  int counter, maxcounter;
  double asratio;

  /* --- tests --- */

  g_return_if_fail(pgammatrixdata.data != NULL);
  g_return_if_fail((addlower >= 0) && (addlower < pgammatrixdata.size));
  g_return_if_fail((addupper >= addlower) && (addupper < pgammatrixdata.size));
  //  g_return_if_fail((subtractlower >= 0) && (subtractlower < pgammatrixdata.size));
  g_return_if_fail((subtractupper >= subtractlower) && (subtractupper < pgammatrixdata.size));

  /* --- initialize locals --- */

  if (subtractlower != subtractupper)
    asratio = ((fabs(addlower - addupper) + 1) / (fabs(subtractlower - subtractupper) + 1));
  else
    asratio = 1;

  for (i = 0; i < 16384; i++) {
    temphist[i] = 0;
    subhist[i] = 0;
  }
  subcounts = 0;

  /* --- if subtract lower is negative we are going to want to know
     --- the number of counts in the reference histogram --- */
  if (subtractlower < 0) {
    for (i = 0; i < histsize[xory]; i++) {
      subcounts = subcounts + *(histloc[xory] + i);
    }
  }
  /* --- now we have that --- */

  /* --- let's do the projection --- */
  maxcounter = pgammatrixdata.size * (addupper - addlower + 1);
  counter = 0;
  StartProgress();
  for (i = addlower; i <= addupper; i++) {
    for (j = 0; j < pgammatrixdata.size; j++) {
      counter = pgammatrixdata.size * i + j;
      /*--ddc dec11 whew this is super-inefficient!!
      if (!(i % 10))
        UpdateProgress(counter,maxcounter);
      ********/
      if (xory == 0)
        temphist[j] = temphist[j] + *(pgammatrixdata.data + pgammatrixdata.size * i + j);
      if (xory == 1)
        temphist[j] = temphist[j] + *(pgammatrixdata.data + pgammatrixdata.size * j + i);
    }
    UpdateProgress(counter, maxcounter);
  }
  EndProgress();
  /* --- now for the subtract gate --- */
  if (subtractlower > 0) {
    maxcounter = pgammatrixdata.size * (subtractupper - subtractlower + 1);
    counter = 0;
    StartProgress();
    for (i = subtractlower; i <= subtractupper; i++) {
      for (j = 0; j < pgammatrixdata.size; j++) {
        counter = pgammatrixdata.size * i + j;
        /*--ddc dec11 more super-inefficiency!
        UpdateProgress(counter,maxcounter);
        */
        if (xory == 0)
          subhist[j] = subhist[j] + (float)*(pgammatrixdata.data + pgammatrixdata.size * i + j);
        if (xory == 1)
          subhist[j] = subhist[j] + (float)*(pgammatrixdata.data + pgammatrixdata.size * j + i);
      }
      UpdateProgress(counter, maxcounter);
    }
    EndProgress();
    for (j = 0; j < pgammatrixdata.size; j++) {
      temphist[j] -= subhist[j] * asratio;
    }
  } else {
    if (subtractlower < 0) {
      frac = (float)(addupper - addlower + 1) * abs(subtractlower) / (float)subcounts;
      //      frac = - (float) subtractlower / (float) subcounts;
      for (i = 0; i < histsize[xory]; i++) {
        temphist[i] = temphist[i] - *(histloc[xory] + i) * frac;
      }
    }
  }
  /* --- do the add --- */
  if (add != 0) {
    for (i = 0; i < pgammatrixdata.size; i++) {
      temphist[i] = temphist[i] + add;
    }
  }
  /* --- make sure there are no zeros --- */
  // for (i= 0; i < pgammatrixdata.size;i++) {
  //  if (temphist[i] < 0) temphist[i] = 0;
  // }
  /* --- write the darned thing to memory --- */
  ObliterateHistogram(histdest);
  histsize[histdest] = (int)pgammatrixdata.size;
  histid[histdest] = histdest;
  sprintf(field1[histdest], "");
  sprintf(field2[histdest], "Gamma in gate %d -> %d", addlower, addupper);
  sprintf(field3[histdest], "");
  histloc[histdest] = (int *)malloc(pgammatrixdata.size * sizeof(int));
  //--ddc reduce rounding errors using rint!
  for (i = 0; i < pgammatrixdata.size; i++) {
    *(histloc[histdest] + i) = rint(temphist[i]);
  }
  spectra = histdest;
  currentrange[0] = 0;
  currentrange[1] = histsize[histdest] - 1;
  spectradisplayed[0][0] = spectra;
  DisplayCurrentRange();
}

/* PgamTwodProjectTwd
 *
 * Projects from a .twd compsed of floats
 */
void PgamTwodProjectTwd(int addlower, int addupper, int subtractlower, int subtractupper,
                        int histdest, int add) {
  /* --- local variables --- */

  //--ddc eliminate floats (to reduce rounding errors) and initialize frac
  //--ddc jun17 replace int with 'long long' to keep sums in spectra from
  //--ddc jun17 overlflowing for counts and subcounts

  int i, j, k;
  double temphist[16384];
  double subhist[16384];
  long long counts;
  int ix, iy, ind;
  double temp, temporary;
  char dummystr[80];
  int isize;
  long long subcounts;
  double frac = 0;
  double asratio;

  /* --- tests --- */
  g_return_if_fail((histdest >= 0) && (histdest <= (GetLastSpectrum() + 1)));
  g_return_if_fail(pgammatrixdata.data != NULL);
  g_return_if_fail((addlower >= 0) && (addlower < pgammatrixdata.size));
  g_return_if_fail((addupper >= addlower) && (addupper < pgammatrixdata.size));
  //  g_return_if_fail((subtractlower >= 0) && (subtractlower < pgammatrixdata.size));
  g_return_if_fail((subtractupper >= subtractlower) && (subtractupper < pgammatrixdata.size));

  /* --- initializations --- */
  counts = 0;
  for (i = 0; i < 16384; i++)
    temphist[i] = 0;
  for (i = 0; i < 16384; i++)
    subhist[i] = 0;
  isize = 2 * pgammatrixdata.size + 1;
  subcounts = 0;

  /* --- if subtract lower is less than zero we are going to
     --- need the number of counts in histogram 0. --- */

  asratio = ((fabs(addlower - addupper) + 1) / (fabs(subtractlower - subtractupper) + 1));

  if (subtractlower < 0) {
    for (i = 0; i < histsize[0]; i++) {
      subcounts = subcounts + *(histloc[0] + i);
    }
    frac = (double)(addupper - addlower + 1) * abs(subtractlower) / (double)subcounts;
    //    frac = - (float) subtractlower / (float) subcounts;y
  }

  /* --- now we make the histogram and do the gating --- */
  StartProgress();
  for (i = 0; i < pgammatrixdata.size; i++) {
    UpdateProgress(i, pgammatrixdata.size);
    temporary = 0;
    for (j = addlower; j <= addupper; j++) {
      ix = Max(i, j);
      iy = Min(i, j);
      ind = ix - iy + (isize - iy) * ((double)iy / (double)2);
      // temphist[i] += *(pgammatrixdata.data + ind);
      // if (ix == iy) temphist[i] += *(pgammatrixdata.ind);
      temp = *(pgammatrixdata.data + ind);
      if (ix == iy)
        temporary = temporary + temp; // intentionally double count on x = y
      temporary = temporary + temp;
    }
    /* --- now for the subtract gate --- */
    if (subtractlower > 0) {
      for (j = subtractlower; j <= subtractupper; j++) {
        ix = Max(i, j);
        iy = Min(i, j);
        ind = ix - iy + (isize - iy) * ((double)iy / (double)2);
        subhist[i] += *(pgammatrixdata.data + ind);
        if (ix == iy)
          subhist[i] += *(pgammatrixdata.data + ind);
        //	temp = *(pgammatrixdata.data + ind);
        // if (ix == iy) temporary = temporary - temp * asratio; // intentionally double count on x
        // = y temporary = temporary - temp * asratio;
      }
    } else {
      if (subtractlower < 0) {
        temporary = temporary - frac * *(histloc[0] + i);
      }
    }
    /* --- now for the linear add --- */
    temporary = temporary + add;
    temphist[i] = temporary;
    //--ddc 19jul07 moved background subtraction from outside loop to here!

    if (subtractlower > 0)
      temphist[i] -= subhist[i] * asratio;

    //--ddc reduce rounding errors!    counts = counts + temporary;
    counts = counts + rint(temporary);
  }
  //--ddc 19jul07 oooh I'll bet jp wanted to put the background subtraction
  // in the loop, where it'll do some good!  Comment out here, and move up
  //  if (subtractlower > 0)
  //    temphist[i] -= subhist[i] * asratio;
  EndProgress();
  //--ddc jun17 overlflowing for counts, changed counts to long long, so print
  // must change.
  //  sprintf(dummystr,"Counts in spectrum %d.\n",counts);
  sprintf(dummystr, "Counts in spectrum %lld.\n", counts);
  WriteMainText(dummystr);

  /* --- allocate memory and assign meaningful numbers --- */
  ObliterateHistogram(histdest);
  histsize[histdest] = pgammatrixdata.size;
  histid[histdest] = histdest;
  sprintf(field1[histdest], "");
  sprintf(field2[histdest], "Gamma in gate %d -> %d", addlower, addupper);
  sprintf(field3[histdest], "");
  histloc[histdest] = (int *)malloc(sizeof(int) * pgammatrixdata.size);
  //--ddc reduce rounding errors!
  // for ( i = 0; i < pgammatrixdata.size; i++)  *(histloc[histdest] + i) = temphist[i];
  for (i = 0; i < pgammatrixdata.size; i++)
    *(histloc[histdest] + i) = rint(temphist[i]);

  spectra = histdest;
  currentrange[0] = 0;
  currentrange[1] = histsize[histdest];
  spectradisplayed[0][0] = spectra;
  DisplayCurrentRange();
}

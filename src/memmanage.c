/* memmanange.c
 *
 * by John Pavan
 *
 * Really just a function for clearing the memory for a histogram
 * for use with Gnuscope
 */

#include "gnuscopefuncs.h"
#include "gnuscopeglobals.h"

/* --- global variables --- */

int intype,spectra,plottype;
//GtkWidget *graph;
int markers[4],binsize,offset;
//int background;
int backgroundploydeg;
int oldscope;

/* --- function declarations --- */

void ObliterateHistogram(int delhist);
//void CreateHistogram();

/* --- functions --- */

/*
 * ObliterateHistogram
 *
 * Frees up the memory occupied by a histogram (if it exists)
 * and then sets all of the test fields to zero.
 */
void ObliterateHistogram (int delhist)
{
  if (histsize[delhist] > 0) {
    //    printf("Histogram %d exists.  Attempting to clear the memory allocated for it.\n",delhist);
    free(histloc[delhist]);
    histsize[delhist] = 0;
    histid[delhist] = 0;
    field1[delhist][0]='\0';
    field2[delhist][0]='\0';
    field3[delhist][0]='\0';
  }
}



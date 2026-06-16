/*
 * GS92.c
 *
 * by John Pavan
 * For use with gnuscope --- reads,writes,projects, and sorts gs92 data
 */

//--ddc nov10 gtk2 deprecations...

/* --- includes --- */

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <sys/stat.h>
#include "gnuscopefuncs.h"
#include "gnuscopeglobals.h"

/* --- Global Variables --- */

int intype, spectra, plottype;
int markers[4], binsize, offset, currentrange[2];

int compression;
struct axis_info xaxis_info, yaxis_info;
struct gamma_gate gammagates[128];
int pchannels[2], badness, sorttype;
int addarray[8192], subtractarray[8192], xgatearray[110], ygatearray[110];
int kstat[21][2];
GtkWidget *box1;
float beta;

/* --- Function Declarations --- */

void DopplerCorrect(int *energy, int detector);
void UpdateProgress(long pos, long len);
void StartProgress();
void EndProgress();
void TrackStats();
void ObliterateHistogram(int delhist);
void ReadGS(char *sFilename);
void WriteGS(char *sFilename);
void WriteSqr(FILE *outfile);
void WriteTwd(FILE *outfile);
void ReadSqr(FILE *infile);
void ReadTwd(FILE *infile);
void ReadDetectorGte(char *sFilename);
void ReadGammaGte(char *sFilename);
int Max(int a, int b);
int Min(int a, int b);
void TwdProject(int addlower, int addupper, int subtractlower, int subtractupper, int desthist,
                int add);
void SqrProject(int addlower, int addupper, int subtractlower, int subtractupper, int desthist,
                int add, int xory);
void MakeGateArrays();
void GetDialog(int type);
void SortTypePrompt(GtkWidget *text);
void ChannelsPrompt(GtkWidget *text);
void BadnessPrompt(GtkWidget *text);
void DopplerCorrectionPrompt(GtkWidget *text);
void TwdorSqrPrompt(GtkWidget *text);
void SortTypeEntry(GtkWidget *widget, GtkWidget *entry);
void ChannelsEntry(GtkWidget *widget, GtkWidget *entry);
void BadnessEntry(GtkWidget *widget, GtkWidget *entry);
void DopplerCorrectionEntry(GtkWidget *widget, GtkWidget *entry);
void TwdorSqrEntry(GtkWidget *widget, GtkWidget *entry);

/* --- functions --- */

/*
 * ReadGS
 *
 * From the file extention determine if it is a .sqr or .twd file and then read the data
 * in an appropriate manner
 */
void ReadGS(char *sFilename) {
  char dummystr[6];
  FILE *infile;

  twoddata.filetype = 0;

  sprintf(dummystr, ".twd");
  if (strstr(sFilename, dummystr) != 0)
    twoddata.filetype = 1;
  sprintf(dummystr, ".sqr");
  if (strstr(sFilename, dummystr) != 0)
    twoddata.filetype = 2;

  /* --- if there is already data in twoddata, we must free the memory --- */
  if (twoddata.data != NULL) {
    free(twoddata.data);
    twoddata.data = NULL;
  }

  /* --- open the file --- */
  if ((infile = fopen(sFilename, "r")) != NULL) {

    /* --- there should be two cases - .twd and .sqr --- */
    switch (twoddata.filetype) {
    case 1:
      /* --- this should be .twd --- */
      ReadTwd(infile);
      break;
    case 2:
      /* --- this should be .sqr --- */
      ReadSqr(infile);
      break;
    default:
      printf("Could not determine the file type from the extention.\n");
    }

    /* --- close the file --- */
    fclose(infile);
  } else {
    printf("Could not open file %s.\n", sFilename);
  }
}

/* ReadSqrAdd
 *
 * Adds a .sqr from a file to what is already in memory
 */
void ReadSqrAdd(char *sFilename) {
  FILE *infile;
  char dummystr[120];
  int headbuffer[2];
  int databuffer[2];
  int buffer1[4];
  short int *buffer2 = NULL;
  int i;

  /* --- open a file --- */
  if ((infile = fopen(sFilename, "r")) != NULL) {

    /* --- see if we already have a matrix in memory --- */
    if (twoddata.data != NULL) {
      /* --- read the data from the new file --- */
      fread(&headbuffer[0], 4, 1, infile);
      fread(buffer1, 12, 1, infile);
      fread(&headbuffer[1], 4, 1, infile);
      if (headbuffer[0] != headbuffer[1]) {
        sprintf(dummystr, "Error reading header for %s.\n", sFilename);
        GetMessageDialog(dummystr);
      }
      fread(&databuffer[0], 4, 1, infile);
      buffer2 = (short int *)malloc(databuffer[0]);
      if (buffer2 != NULL)
        fread(buffer2, 1, databuffer[0], infile);
      fread(&databuffer[1], 4, 1, infile);
      if (databuffer[0] != databuffer[1]) {
        sprintf(dummystr, "Error reading data from %s.\n", sFilename);
        GetMessageDialog(dummystr);
      }
      for (i = 0; i < (databuffer[0] / sizeof(short int)); i++) {
        twoddata.data[i] += buffer2[i];
      }
      if (buffer2 != NULL)
        free(buffer2);
    } else {
      ReadSqr(infile);
    }
    fclose(infile);
  }
}

/*
 * ReadSqr
 *
 * Reads in a .sqr file
 */
void ReadSqr(FILE *infile) {
  fread(&twoddata.headbuffer[0], 4, 1, infile);
  fread(&twoddata.size, 4, 1, infile);
  fread(&twoddata.type, 4, 1, infile);
  fread(&twoddata.df, 4, 1, infile);
  fread(&twoddata.headbuffer[1], 4, 1, infile);
  if (twoddata.headbuffer[1] == twoddata.headbuffer[0]) {
    fread(&twoddata.databuffer[0], 4, 1, infile);
    if (twoddata.data == NULL) {
      twoddata.data = (short int *)malloc(twoddata.databuffer[0]);
    } else {
      twoddata.data = (short int *)realloc(twoddata.data, twoddata.databuffer[0]);
    }
    fread(twoddata.data, 2, (twoddata.size * twoddata.size), infile);
    fread(&twoddata.databuffer[1], 4, 1, infile);
    if (twoddata.databuffer[0] != twoddata.databuffer[1])
      printf("Error reading data.\n");
  } else {
    printf("Error reading header.\n");
  }
}

/* ReadTwdAdd
 *
 * Adds a .sqr from a file to what is already in memory
 */
void ReadTwdAdd(char *sFilename) {
  FILE *infile;
  char dummystr[120];
  int headbuffer[2];
  int databuffer[2];
  int buffer1[4];
  short int *buffer2 = NULL;
  int i;

  /* --- open a file --- */
  if ((infile = fopen(sFilename, "r")) != NULL) {

    /* --- see if we already have a matrix in memory --- */
    if (twoddata.data != NULL) {
      /* --- read the data from the new file --- */
      fread(&headbuffer[0], 4, 1, infile);
      fread(buffer1, 16, 1, infile);
      fread(&headbuffer[1], 4, 1, infile);
      if (headbuffer[0] != headbuffer[1]) {
        sprintf(dummystr, "Error reading header for %s.\n", sFilename);
        GetMessageDialog(dummystr);
      }
      fread(&databuffer[0], 4, 1, infile);
      buffer2 = (short int *)malloc(databuffer[0]);
      if (buffer2 != NULL)
        fread(buffer2, 1, databuffer[0], infile);
      fread(&databuffer[1], 4, 1, infile);
      if (databuffer[0] != databuffer[1]) {
        sprintf(dummystr, "Error reading data from %s.\n", sFilename);
        GetMessageDialog(dummystr);
      }
      for (i = 0; i < (databuffer[0] / sizeof(short int)); i++) {
        twoddata.data[i] += buffer2[i];
      }
      if (buffer2 != NULL)
        free(buffer2);
    } else {
      ReadTwd(infile);
    }
    fclose(infile);
  }
}

/*
 * ReadTwd
 *
 * Reads in a .twd file
 */
void ReadTwd(FILE *infile) {
  /* --- this should be .twd --- */
  /* --- in this case the head buffer is 16 bytes long --- */
  fread(&twoddata.headbuffer[0], 4, 1, infile);
  fread(&twoddata.size, 4, 1, infile);
  fread(&twoddata.what, 4, 1, infile);
  fread(&twoddata.type, 4, 1, infile);
  fread(&twoddata.df, 4, 1, infile);
  fread(&twoddata.headbuffer[1], 4, 1, infile);
  if (twoddata.headbuffer[0] == twoddata.headbuffer[1]) {
    fread(&twoddata.databuffer[0], 4, 1, infile);
    if (twoddata.data == NULL) {
      twoddata.data = (short int *)malloc(twoddata.databuffer[0]); /* --- allocate memory --- */
    } else {
      twoddata.data = (short int *)realloc(twoddata.data, twoddata.databuffer[0]);
    }
    fread(twoddata.data, 1, twoddata.databuffer[0], infile);
    fread(&twoddata.databuffer[1], 4, 1, infile);
    if (twoddata.databuffer[0] != twoddata.databuffer[1])
      printf("There was an error trying to read the data.\n");
  } else {
    printf("There was an error trying to read the header.\n");
  }
}

/*
 * ReadDetectorGte
 *
 * reads in a .gte file which contains information about detectors
 */
void ReadDetectorGte(char *sFilename) {
  FILE *infile;
  int i;

  if ((infile = fopen(sFilename, "r")) != NULL) {
    /* --- initialize --- */
    for (i = 0; i < 254; i++) {
      xaxis_info.detectors[i] = 0;
      yaxis_info.detectors[i] = 0;
    }
    fscanf(infile, "%d\n", &compression);
    printf("Compression: %d\n", compression);
    fscanf(infile, "%d\n", &xaxis_info.num);
    printf("Number of xaxis detectors: %d\nDetectors: ", xaxis_info.num);
    for (i = 0; i < xaxis_info.num; i++) {
      fscanf(infile, "%d", &xaxis_info.detectors[i]);
      printf("%d ", xaxis_info.detectors[i]);
    }
    fscanf(infile, "\n%d\n", &yaxis_info.num);
    printf("\nNumber of yaxis detectors: %d\n", yaxis_info.num);
    for (i = 0; i < yaxis_info.num; i++) {
      fscanf(infile, "%d", &yaxis_info.detectors[i]);
      printf("%d ", yaxis_info.detectors[i]);
    }
    printf("\n");
    fclose(infile);
  } else {
    printf("Error opening file.\n");
  }
}

/*
 * ReadGammaGte
 *
 * Reads in a .gte file which contains information about gammas
 */
void ReadGammaGte(char *sFilename) {
  FILE *infile;
  int i, test;

  /* initialize */
  for (i = 0; i < 128; i++) {
    gammagates[i].addorsubtract = 0;
    gammagates[i].lower = -1;
    gammagates[i].upper = -1;
    gammagates[i].gamma_ray = -1;
  }
  i = 0;

  if ((infile = fopen(sFilename, "r")) != NULL) {
    while (test != EOF) {
      test = fscanf(infile, "%d %d %d %d\n", &gammagates[i].addorsubtract, &gammagates[i].lower,
                    &gammagates[i].upper, &gammagates[i].gamma_ray);
      printf("%d %d %d %d\n", gammagates[i].addorsubtract, gammagates[i].lower, gammagates[i].upper,
             gammagates[i].gamma_ray);
      i++;
    }
    fclose(infile);
  } else {
    printf("Error opening file.\n");
  }
}

/*
 * WriteGS
 *
 * Write out the file in an appropriate manner from the file type
 */
void WriteGS(char *sFilename) {
  char dummystr[6];
  FILE *outfile;

  switch (twoddata.filetype) {
  case 1:
    sprintf(dummystr, ".twd");
    if (strstr(sFilename, dummystr) == NULL)
      strcat(sFilename, dummystr);
    if ((outfile = fopen(sFilename, "w")) != NULL) {
      WriteTwd(outfile);
    } else
      printf("Couldn't open output file.\n");
    break;
  case 2:
    sprintf(dummystr, ".sqr");
    if (strstr(sFilename, dummystr) == NULL)
      strcat(sFilename, dummystr);
    if ((outfile = fopen(sFilename, "w")) != NULL) {
      WriteSqr(outfile);
    } else
      printf("Couldn't open output file.\n");
    break;
  }
}

/*
 * WriteSqr
 *
 * Writes a .sqr
 */
void WriteSqr(FILE *outfile) {
  fwrite(&twoddata.headbuffer[0], 4, 1, outfile);
  fwrite(&twoddata.size, 4, 1, outfile);
  fwrite(&twoddata.type, 4, 1, outfile);
  fwrite(&twoddata.df, 4, 1, outfile);
  fwrite(&twoddata.headbuffer[1], 4, 1, outfile);
  fwrite(&twoddata.databuffer[0], 4, 1, outfile);
  fwrite(twoddata.data, 2, (twoddata.size * twoddata.size), outfile);
  fwrite(&twoddata.databuffer[1], 4, 1, outfile);
  //  printf("Doesn't work yet.\n");
}

/*
 * WriteTwd
 *
 * Writes a .twd
 */
void WriteTwd(FILE *outfile) {
  fwrite(&twoddata.headbuffer[0], 4, 1, outfile);
  fwrite(&twoddata.size, 4, 1, outfile);
  fwrite(&twoddata.what, 4, 1, outfile);
  fwrite(&twoddata.type, 4, 1, outfile);
  fwrite(&twoddata.df, 4, 1, outfile);
  fwrite(&twoddata.headbuffer[1], 4, 1, outfile);
  fwrite(&twoddata.databuffer[0], 4, 1, outfile);
  fwrite(twoddata.data, 1, (twoddata.databuffer[0]), outfile);
  fwrite(&twoddata.databuffer[1], 4, 1, outfile);
  //  printf("Doesn't work yet.\n");
}

/*
 * SortTypePrompt
 * makes the text for the sort type dialog
 */
void SortTypePrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "What kind of Sort?\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "1 - singles     2 - doubles\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "3 - triples     4 - evaps  \n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * SortTypeEntry
 *
 * interprets the entry in the sort type dialog and goes to the next stage of set up
 */
void SortTypeEntry(GtkWidget *widget, GtkWidget *entry) {
  int test;

  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%d", &sorttype);
  if ((test == 1) && (sorttype >= 1) && (sorttype <= 4)) {
    GetDialog(20);
  }
}

/*
 * ChannelsPrompt
 * sets text for channel selection dialog
 */
void ChannelsPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Enter Particle channels to sort?\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "MIN MAX\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * ChannelsEntry
 * Interprets the entry in the channel selection dialog and goes to the next stage of setup
 */
void ChannelsEntry(GtkWidget *widget, GtkWidget *entry) {
  int test, i;
  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%d %d", &pchannels[0], &pchannels[1]);
  if ((test == 2) && (0 < pchannels[0] && pchannels[0] <= 20) &&
      (0 < pchannels[1] && pchannels[1] <= 20)) {
    // if ((test == 2) && (0 < pchannels[0] <=20) && (0 < pchannels[1] <=20)) {
    /* --- get pchannels in the right order --- */
    if (pchannels[0] > pchannels[1]) {
      i = pchannels[1];
      pchannels[1] = pchannels[0];
      pchannels[0] = i;
    }
    GetDialog(21);
  }
}

/*
 * BadnessPrompt
 * Asks for the maximum badness to accept
 */
void BadnessPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "What is the maximum badness you will accept?\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * BadnessEntry
 * Interprets the badness entry and proceeds to the net step of setup
 */
void BadnessEntry(GtkWidget *widget, GtkWidget *entry) {
  int test, i;
  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%d", &i);
  if (test == 1) {
    badness = i;
    GetDialog(22);
  }
}

/*
 * DopplerCorrectionPrompt
 * sets the text for the doppler correction prompt
 */
void DopplerCorrectionPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "If you wish to use Doppler Shift Correction,\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "please enter the Beta you wish to use.\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "All other resonces assume beta = 0.\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * DopplerCorrectionEntry
 * Interprets the DopplerCorrection Entry
 */
void DopplerCorrectionEntry(GtkWidget *widget, GtkWidget *entry) {
  int test;
  float temp;

  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%f", &temp);
  if ((test == 1) && (0 <= temp && temp <= 1)) {
    // if ((test == 1) && (0 <= temp <= 1)) {
    beta = temp;
  } else {
    beta = 0;
  }
  if ((sorttype == 2) || (sorttype == 3))
    GetDialog(23);
}

/*
 * TwdorSqrPrompt
 * asks if the user wants a twd or sqr to sort
 */
void TwdorSqrPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, ".twd or .sqr file\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/* TwdorSqrEntry
 * Determines if they wanted a twd or sqr
 */
void TwdorSqrEntry(GtkWidget *widget, GtkWidget *entry) {
  int test;
  char temp[20];
  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%s", &temp);
  if (test == 1) {
    if (strstr(temp, "twd") != 0)
      twoddata.filetype = 1;
    if (strstr(temp, "sqr") != 0)
      twoddata.filetype = 2;

    if ((twoddata.filetype == 1) || (twoddata.filetype == 2)) {
      GetFilename("Detector Gate", 0, ReadDetectorGte);
      if (sorttype == 3)
        GetFilename("Gamma Gate", 0, ReadGammaGte);
    }
  }
}

/*
 * Setup
 *
 * gets the appropriate information to do a sort
 */
void Setup() {
  char dummystr[6], type[6];
  int i;

  beta = 0;

  /* --- get the type of sort to do --- */

  printf("What kind of sort? (1 - singles, 2 - doubles, 3 - triples, 4 - evap)");
  scanf("%d", &sorttype);
  if ((1 <= sorttype) && (4 >= sorttype)) {
    printf("What reaction channels?");
    scanf("%d %d", &pchannels[0], &pchannels[1]);
    /* --- make sure that the pchannels are in the appropriate range --- */
    if ((pchannels[0] > 0) && (pchannels[0] <= 20) && (pchannels[1] > 0) && (pchannels[1] <= 20)) {
      /* --- get the particle channels in the normal range --- */
      if (pchannels[0] > pchannels[1]) {
        i = pchannels[1];
        pchannels[1] = pchannels[0];
        pchannels[0] = i;
      }
      printf("What maximum badness will you accept?");
      scanf("%d", &badness);

      printf("Do you wish to correct for doppler shifting?(yes, no) [no]");
      scanf("%s", dummystr);
      if (strstr(dummystr, "y")) {
        printf("What value of beta do you wish to use.");
        scanf("%f", &beta);
      }
      /* --- if we are doing doubles (or triples) we need
         --- to know if we are a square or a triangle --- */

      if ((sorttype == 2) || (sorttype == 3)) {

        /* --- triangle or square --- */

        if (twoddata.filetype == 1)
          sprintf(dummystr, ".twd");
        if (twoddata.filetype == 2)
          sprintf(dummystr, ".sqr");
        if ((twoddata.filetype == 1) || (twoddata.filetype == 2)) {
          printf("The current file type is %s.  ", dummystr);
        }
        printf("What would you like to change the file type to?\n");
        scanf("%s", dummystr);
        sprintf(type, "twd");
        if (strstr(dummystr, type) != NULL)
          twoddata.filetype = 1;
        sprintf(type, "sqr");
        if (strstr(dummystr, type) != NULL)
          twoddata.filetype = 2;
        while (!((twoddata.filetype == 1) || (twoddata.filetype == 2))) {
          printf("What would you like to change the file type to?\n");
          scanf("%s", dummystr);
          if (strstr(dummystr, ".twd") != NULL)
            twoddata.filetype = 1;
          if (strstr(dummystr, ".sqr") != NULL)
            twoddata.filetype = 2;
        }
        /* --- we also need to know which detectors to use --- */
        GetFilename("Detector Gate", 0, ReadDetectorGte);
        /* --- in the case of triples, we need that too --- */
        if (sorttype == 3) {
          GetFilename("Gamma Gate", 0, ReadGammaGte);
        }
      }
    } else {
      printf("Particle channels out of range.\n");
    }
  }
}

/*
 * MakeGateArrays
 *
 * Makes the boolean arrays for add and subtract gates
 */
void MakeGateArrays() {
  int i, j;

  /* --- initializations --- */
  for (i = 0; i < 110; i++) {
    xgatearray[i] = ygatearray[i] = 0;
  }
  for (i = 0; i < 8192; i++) {
    addarray[i] = subtractarray[i] = 0;
  }

  /* --- make the detector gates --- */

  printf("Using xdetectors: ");
  for (i = 0; i < xaxis_info.num; i++) {
    xgatearray[xaxis_info.detectors[i]] = 1;
    printf("%d ", xaxis_info.detectors[i]);
  }
  printf("\nUsing ydetectors: ");
  for (i = 0; i < yaxis_info.num; i++) {
    ygatearray[yaxis_info.detectors[i]] = 1;
    printf("%d ", yaxis_info.detectors[i]);
  }

  printf("\n");
  /* --- make gamma gate arrays --- */

  i = 0;
  while (gammagates[i].lower != -1) {
    for (j = 0; j < 8192; j++) {
      if ((j <= gammagates[i].upper) && (j >= gammagates[i].lower)) {
        if (gammagates[i].addorsubtract)
          addarray[j] = 1;
        else if (gammagates[i].addorsubtract == -1)
          subtractarray[j] = 1;
      }
    }
    i++;
  }
}

/*
 * TrackStats
 *
 * Tracks the gamma multipolarity
 */
void TrackStats() {
  int i;

  for (i = 2; i <= 5; i++) {
    kstat[i][2] = 0;
  }

  /* --- no gammas --- */
  kstat[1][2] = kstat[1][1];
  /* --- gamma singles --- */
  for (i = 2; i <= 10; i++) {
    kstat[2][2] = kstat[2][2] + (i - 1) * kstat[i][1];
  }
  /* --- gamma doubles --- */
  for (i = 3; i <= 10; i++) {
    kstat[3][2] = kstat[3][2] + (float)(kstat[i][1] * (i - 1) * (i - 2)) / (float)2;
  }
  /* --- gamma triples --- */
  for (i = 4; i <= 10; i++) {
    kstat[4][2] = kstat[4][2] + (float)(kstat[i][1] * (i - 1) * (i - 2) * (i - 3)) / (float)6;
  }
  /* --- gamma 4 fold --- */
  for (i = 5; i < 10; i++) {
    kstat[5][2] =
        kstat[5][2] + (float)(kstat[i][1] * (i - 1) * (i - 2) * (i - 3) * (i - 4)) / (float)24;
  }
  printf("No gammas: %d  Gamma Singles: %d  Gamma Doubles:  %d\n", kstat[1][2], kstat[2][2],
         kstat[3][2]);
  printf("Gamma Triples: %d  Gamma Quads: %d\n", kstat[4][2], kstat[5][2]);
}

/*
 * Sort
 *
 * Sorts GS92 data based on the parameters from setup
 */
void Sort() {
  int sortall, sortrecords, recordmin, recordmax, runmin, runmax;
  char dummystr[80];
  int i, j, k, l, m;
  int abort;
  char filename[80];
  FILE *infile;
  short int buffer[4092];
  int buffersize[2], records;
  int kval, igam, kprot, kalpha, ibad, ind, jhk, jh, jk, krecol, ithet, iphi, evap;
  int egam[2046], kid, idgam[2046], iand, ii;
  int kk, stillreading;
  int kchn, ener;
  int readthisrec;
  int irec;
  int statistics[4];
  int ix, iy;
  int ig1, ig2, ig3, ig4, n, ng[3], o, js, is, ks, mg;
  struct stat filebuffer;
  int evaph[101][101];
  long int fileLen = 0;
  int twdind, isize, ix1, iy1;

  for (i = 0; i < 4; i++)
    statistics[i] = 0;
  sortall = 0;
  sortrecords = 0;
  runmin = 0;
  runmax = 50;
  abort = 0;
  m = 0;
  recordmin = 0;
  recordmax = 100000000;

  /* --- make sure the user has run sort setup --- */
  if (((pchannels[0] >= 0) && (pchannels[0] < 20))) { // setup check

    /* --- Get the appropriate information from the user --- */
    printf("Would you like to sort all of the gs92 runs? (yes,no) [no]");
    scanf("%s", dummystr);
    if (strstr(dummystr, "y") != NULL) {
      abort = 0;
      sortall = 1;
    }
    if (sortall == 0) {
      printf("Enter the number of records to sort.\n (0 for all, -1 to abort or sort in the middle "
             "of the run)\n");
      scanf("%d", &sortrecords);
      if (sortrecords < -1) {
        printf("Do you want to abort the sort?.");
        scanf("%s", dummystr);
        if (strstr(dummystr, "y") == NULL)
          abort = 1;
        else {
          printf("Type first and last records to sort.");
          scanf("%d %d", &recordmin, &recordmax);
        }
      } else {
        printf("What are the first and last run numbers you wish to sort?");
        scanf("%d %d", &runmin, &runmax);
      }
    }

    /* --- must allocate memory and define nesacary stuff for data structures --- */
    if (!(abort)) {
      switch (sorttype) {                                      // data structure switch
      case 1:                                                  // singles, need histograms
        for (i = 0; i <= (pchannels[1] - pchannels[0]); i++) { // histogram loop
          ObliterateHistogram(i);
          histloc[i] = (int *)malloc((sizeof(int) * 8192));
          histsize[i] = 8192;
          histid[i] = i;
          sprintf(field1[i], "");
          sprintf(field2[i], "Pchan %d", (i + pchannels[0]));
          sprintf(field3[i], "");
          /* --- initialize histogram --- */
          for (j = 0; j < histsize[i]; j++) {
            *(histloc[i] + j) = 0;
          }
        } // end histogram loop
        break;
      case 2:
      case 3:
        /* --- initialize our gatting arrays --- */
        MakeGateArrays();
        /* --- in both these cases we will need to allocate memory for
           --- either a triangle or square based on what the twoddata.filetype is */
        switch (twoddata.filetype) {
          /* --- we will default the size to 3000 channels to a side --- */
        case 1:
          /* --- .twd --- */
          twoddata.size = 3000;
          isize = 2 * twoddata.size + 1;
          twoddata.what = 0;
          twoddata.type = 5;
          twoddata.df = 0;
          twoddata.headbuffer[0] = twoddata.headbuffer[1] = 16;
          twoddata.databuffer[0] = twoddata.databuffer[1] = (float)sizeof(short int) / (float)2 *
                                                            (float)twoddata.size *
                                                            (float)(twoddata.size + 1);
          if (twoddata.data == NULL) {
            twoddata.data = (short int *)malloc(twoddata.databuffer[0]);
          } else {
            twoddata.data = (short int *)realloc(twoddata.data, twoddata.databuffer[0]);
          }
          for (j = 0; j < (twoddata.databuffer[0] / sizeof(short int)); j++) {
            *(twoddata.data + j) = 0;
          }
          break;
        case 2:
          /* --- .sqr --- */
          twoddata.size = 3000;
          twoddata.type = 5;
          twoddata.df = 0;
          twoddata.headbuffer[0] = twoddata.headbuffer[1] = 12;
          if (twoddata.data == NULL) {
            twoddata.data = (short int *)malloc(3000 * 3000 * sizeof(short int));
          } else {
            twoddata.data = (short int *)realloc(twoddata.data, 3000 * 3000 * sizeof(short int));
          }
          twoddata.databuffer[0] = twoddata.databuffer[1] = (3000 * 3000 * sizeof(short int));
          for (j = 0; j < twoddata.size; j++) {
            for (k = 0; k < twoddata.size; k++) {
              *(twoddata.data + j * twoddata.size + k) = 0;
            }
          }
          break;
        default:
          printf("This doesn't work for the file type you selected.\n");
          abort = 1;
        } // end filetype switch
        break;
      case 4:
        for (i = 0; i <= (pchannels[1] - pchannels[0]); i++) { // histogram loop
          ObliterateHistogram(i);
          histloc[i] = (int *)malloc((sizeof(int) * 5000));
          histsize[i] = 5000;
          histid[i] = i;
          sprintf(field1[i], "");
          sprintf(field2[i], "Pchan %d", (i + pchannels[0]));
          sprintf(field3[i], "");
          /* --- initialize histogram --- */
          for (j = 0; j < histsize[i]; j++) {
            *(histloc[i] + j) = 0;
          }
        } // end histogram loop
        break;
      default:
        abort = 1;
      } // end data structure switch
    } // done defining data structures

    /* --- let's do this sort --- */
    if (!(abort)) {                                    // don't do it if aborting
      for (i = pchannels[0]; i <= pchannels[1]; i++) { // particle channel loop
        for (j = runmin; j <= runmax; j++) {           // run loop
          /* --- opening the file --- */
          /* --- generate the file name --- */
          sprintf(filename, "/gamma1/gs92c%dr%d.mul", i, j);
          /* --- open the file --- */
          /* --- generate a progress bar --- */
          stat(filename, &filebuffer);
          fileLen = filebuffer.st_size;
          StartSortProgress();
          if ((infile = fopen(filename, "r")) != NULL) { // file test
            printf("Sorting file %s.\n", filename);
            irec = 0;                            // keep track of the records sorted
            stillreading = 1;                    // boolean for our reading loop
            fread(&buffersize[0], 4, 1, infile); // read in the first peice of the fortran buffer
            while ((fread(&buffer, 1, buffersize[0], infile) == buffersize[0]) && (stillreading)) {
              /* --- read in other half of the fortran buffer and check for errors --- */
              fread(&buffersize[1], 4, 1, infile);
              /* --- error check --- */
              if (buffersize[0] != buffersize[1])
                stillreading = 0;
              fread(&buffersize[0], 4, 1, infile);
              /* --- increment the record counter, see if we are in the appropriate range --- */
              irec++;
              /* --- update the progress bar --- */
              if ((irec % 100) == 0)
                UpdateSortProgress(ftell(infile), fileLen);
              readthisrec = 1;
              if (irec < recordmin)
                readthisrec = 0;
              if ((stillreading) && (readthisrec)) {
                // printf("Trying to read the record.\n");
                /* --- do stuff if there isn't an error and supposed to read --- */
                /* --- now we have to interpret this thing --- */
                for (k = 0; k < 4092; k++) { // interpreting loop
                  // printf("Entering Interpreting loop.\n");
                  if (buffer[k] < 0) { // event test
                    igam = 0;
                    kval = -(buffer[k]);
                    igam = kval % 16;
                    kval = kval - igam;
                    kprot = kval % 128;
                    kval = kval - kprot;
                    kprot = (float)kprot / (float)16; // (float) makes it divide right
                    kalpha = kval % 1024;
                    kval = kval - kalpha;
                    kalpha = (float)kalpha / (float)128;
                    ibad = kval % 4096;
                    ibad = (float)ibad / (float)1024;
                    /* --- move onto the next word --- */
                    k++;
                    if (k >= 4092)
                      goto jumpend; // make sure we arn't going to dump
                    jhk = buffer[k];
                    jh = jhk % 1024;
                    jk = (float)jhk / (float)1024;
                    k++;
                    if (k >= 4092)
                      goto jumpend; // make sure we arn't going to dump
                    krecol = buffer[k];
                    k++;
                    if (k >= 4092)
                      goto jumpend; // make sure we don't dump
                    kval = buffer[k];
                    if (kval < 0)
                      kval = kval + 65536;
                    ithet = kval % 128;
                    kval = kval - ithet;
                    iphi = (float)kval / (float)128;
                    k++;
                    if (k >= 4092)
                      goto jumpend; // make sure we don't dump
                    evap = buffer[k];
                    kchn = 0;
                    if (kprot < 5)
                      kchn = kalpha * 5 + kprot;
                    /* --- get the gamma energies --- */
                    for (l = 0; l < igam; l++) { // gamma energy loop
                      k++;
                      if (k >= 4092)
                        goto recordgammas;
                      kval = buffer[k];
                      if (kval < 0)
                        kval = kval + 65536;
                      egam[l] = kval % 16384;
                    } // end gamma energy loop
                    /* --- get packed detector id numbers --- */
                    kid = (float)igam / (float)2 + igam % 2;
                    ii = 1;
                    for (l = 0; l < kid; l++) { // detector id loop
                      k++;
                      if (k >= 4092)
                        goto recordgammas;
                      idgam[ii] = buffer[k] & 255;
                      ii++;
                      if (ii <= igam) {
                        idgam[ii] = (float)buffer[k] / (float)256;
                        ii++;
                      }
                    } // end detector id loop
                  recordgammas:
                    /* --- monitor gamma ray statistics --- */
                    ii = igam + 1;
                    /* --- make sure we are within the pchannel range
                       --- and the badness range */
                    if ((kchn >= pchannels[0]) && (kchn <= pchannels[1]) && (ibad < badness)) {
                      kk = kchn - pchannels[0]; // histogram id of pchannel
                      if (ii <= 20)
                        kstat[ii][1] = kstat[ii][1] + 1;
                      switch (sorttype) { // what kind of sorting
                      case 1:             // singles
                        /* --- record multiplicity statistics --- */
                        /* --- write out the egam data --- */
                        for (l = 0; l < igam; l++) {
                          ener = egam[l];
                          if (beta != 0)
                            DopplerCorrect(&ener, idgam[l]);
                          if ((ener > 0) && (ener < 8192)) {
                            *(histloc[kk] + ener) = *(histloc[kk] + ener) + 1;
                          }
                        } // end egam reading

                        break;
                        GetDialog(1);
                      case 2:            // doubles
                        if (igam >= 2) { // double condition

                          /* --- adding --- */
                          for (l = 0; l < igam; l++) {   // unfold gammas
                            for (m = 0; m < igam; m++) { // unfold 2
                              if (l != m) {              // no double counting
                                ix = (float)egam[l] / (float)compression;
                                if ((ix > 0) && (ix < twoddata.size) &&
                                    (xgatearray[idgam[l]])) { // valid range & xdetector
                                  iy = (float)egam[m] / (float)compression;
                                  if ((iy > 0) && (iy < twoddata.size) &&
                                      (ygatearray[idgam[m]])) {  // valid range and ydetector
                                    switch (twoddata.filetype) { // filetype switch
                                    case 1:                      // .twd
                                      // in this case doppler correct both energies
                                      if (beta != 0) { // doppler correct condition
                                        DopplerCorrect(&ix, idgam[l]);
                                        DopplerCorrect(&iy, idgam[m]);
                                      } // end doppler correct condition
                                      ix1 = Max(ix, iy);
                                      iy1 = Min(ix, iy);
                                      twdind = ix1 - iy1 + (isize - iy1) * ((float)iy1 / (float)2);
                                      *(twoddata.data + twdind) = *(twoddata.data + twdind) + 1;
                                      break;
                                    case 2: // .sqr
                                      // in this case only doppler correct what is on y
                                      if (beta != 0)
                                        DopplerCorrect(&iy, idgam[m]);
                                      *(twoddata.data + ix * twoddata.size + iy) =
                                          *(twoddata.data + ix * twoddata.size + iy) + 1;
                                      break;
                                    default:
                                      printf("This doesn't work for this file type.\n");
                                    } // end filetype switch
                                    //  printf("Saw a valid count.\n");
                                  } // end valid range and ydetector condition
                                } // end valid range & xdetector condition
                              } // end double count avoidance
                            } // done unfold 2
                          } // done unfolding gammas
                        } // end doubles condition
                        break;
                      case 3:            // triples
                        if (igam >= 3) { // triples condition
                          ig1 = igam - 2;
                          for (l = 0; l < ig1; l++) { // main triples loop
                            ig2 = l + 1;
                            ig3 = igam - 1;
                            for (m = ig2; m <= ig3; m++) { // triples loop 2
                              ig4 = m + 1;
                              for (n = ig4; n <= igam; n++) { // triples loop 3
                                ng[0] = l;
                                ng[1] = m;
                                ng[2] = n;
                                for (o = 0; o < 3; o++) { // testing loop
                                  // printf("Made it to the testing loop.\n");
                                  mg = egam[ng[o]];
                                  if ((addarray[mg]) || (subtractarray[mg])) { // first con met
                                    // printf("Made it to the first condition.\n");
                                    js = (o + 1) % 3;
                                    ks = (o + 2) % 3;
                                    /* --- proceed if the other two detectors are allwed --- */
                                    if ((xgatearray[idgam[ng[js]]]) &&
                                        (ygatearray[idgam[ng[ks]]])) { // det con
                                      /* --- write the event --- */
                                      ix = (float)egam[ng[js]] / (float)compression;
                                      iy = (float)egam[ng[ks]] / (float)compression;
                                      if ((ix > 0) && (ix < twoddata.size) && (iy > 0) &&
                                          (iy < twoddata.size)) {      // write the event
                                        if (addarray[mg]) {            // adding
                                          switch (twoddata.filetype) { // twd or sqr
                                          case 1:
                                            // in this case doppler correct both energies
                                            if (beta != 0) { // doppler correct condition
                                              DopplerCorrect(&ix, idgam[ng[js]]);
                                              DopplerCorrect(&iy, idgam[ng[ks]]);
                                            } // end doppler correct condition
                                            ix1 = Max(ix, iy);
                                            iy1 = Min(ix, iy);
                                            twdind =
                                                ix1 - iy1 + (isize - iy1) * ((float)iy1 / (float)2);
                                            *(twoddata.data + twdind) =
                                                *(twoddata.data + twdind) + 1;
                                            break;
                                          case 2:
                                            // in this case only doppler correct y
                                            if (beta != 0)
                                              DopplerCorrect(&iy, idgam[ng[ks]]);
                                            *(twoddata.data + ix * twoddata.size + iy) =
                                                *(twoddata.data + ix * twoddata.size + iy) + 1;
                                            break;
                                          default:
                                            break;
                                          } // end twd or sqr
                                        } // done adding
                                        if (subtractarray[mg]) {       // subtracting
                                          switch (twoddata.filetype) { // twd or sqr
                                          case 1:
                                            ix1 = Max(ix, iy);
                                            iy1 = Min(ix, iy);
                                            twdind =
                                                ix1 - iy1 + (isize - iy1) * ((float)iy1 / (float)2);
                                            *(twoddata.data + twdind) =
                                                *(twoddata.data + twdind) + 1;
                                            break;
                                          case 2:
                                            *(twoddata.data + ix * twoddata.size + iy) =
                                                *(twoddata.data + ix * twoddata.size + iy) - 1;
                                            break;
                                          default:
                                            break;
                                          } // end twd or sqr
                                        } // done subtracting
                                      } // done writting the event
                                    } // end det con
                                  } // end first con met
                                } // end testing loop
                              } // end triples loop 3
                            } // end triples loop 2
                          } // end main triples loop
                        } // end triples condition
                        break;
                      case 4:                             // particles
                        if ((evap > 0) && (evap < 800)) { // evap condition
                          *(histloc[kk] + evap) = *(histloc[kk] + evap) + 1;
                        } // end evap condition
                        for (l = 0; l < igam; l++) {                  // igam loop
                          if ((idgam[l] >= 1) || (idgam[l] <= 110)) { // idgam condition
                            *(histloc[kk] + 799 + idgam[l]) = *(histloc[kk] + 799 + idgam[l]) + 1;
                          } // end idgam condtion
                        } // end igam loop
                        if ((jh > 0) && (jh < 1000)) { // jh condition
                          *(histloc[kk] + 1000 + jh) = *(histloc[kk] + 1000 + jh) + 1;
                        } // end jh condition
                        if ((jk > 0) && (jk < 1000)) { // jk condition
                          *(histloc[kk] + 2000 + jk) = *(histloc[kk] + 2000 + jk) + 1;
                        } // end jk condition
                        if ((ithet > 0) && (ithet < 1000)) { // ithet condition
                          *(histloc[kk] + 3000 + ithet) = *(histloc[kk] + 3000 + ithet) + 1;
                        } // end ithet condition
                        if ((iphi > 1) && (iphi < 1000)) { // iphi condition
                          *(histloc[kk] + 4000 + iphi) = *(histloc[kk] + 4000 + iphi) + 1;
                        } // end iphi condition
                        if ((evap > 1) && (evap < 100)) { // first stat condition
                          jh = (float)jh / (float)10;
                          if ((jh > 1) && (jh < 100)) { // second stat condition
                            evaph[evap][jh] = evaph[evap][jh] + 1;
                          } // end second stat condition
                        } // end first stat condition
                        break;
                      default:
                        break;
                      } // end switch
                    } // end pchannel and badness check
                    //--ddc gcc 3.4.4 doesn't like labels followed by ... nothing (added continue)
                  jumpend:
                    continue;

                  } // end event test
                } // end interpreting loop

                /* --- if we have sorted all the request records we are done --- */
                if (irec > recordmax)
                  stillreading = 0;
                // printf("Are we still reading? %d \n",stillreading);
              } // end of doing stuff if there is not an error, and we are supposed to read
            } // reading loop
            /* --- display the number of records sorted --- */
            printf("%s file closed, %d records sorted.\n", filename, irec);
            TrackStats();
            // if you want to monitor evaph, here is the place to do it
            /* --- if we success fully open a file, we should close it --- */
            fclose(infile);
            /* --- get rid of the progress bar --- */
            EndSortProgress();
          } else {
            EndSortProgress();
          } // end file test
        } // end run loop
      } // end particle channel loop
    } else { // abort check
      printf("Sort Aborted.\n");
    } // end abort check
  } else { // setup check
    printf("Please run Sort Setup first.\n");
  } // setup check
}

/*
 * Project
 *
 * Projects from either a .twd or a .sqr depending on what is in memory.
 */
void Project() {
  /* --- local variables --- */
  int addupper, addlower, subtractupper, subtractlower, histdest, add;
  int lasthist, xory;
  char dummystr[3];

  /* --- get the markers in the right order --- */
  if (markers[0] > markers[1]) {
    addupper = markers[0];
    addlower = markers[1];
  } else {
    addupper = markers[1];
    addlower = markers[0];
  }
  if (markers[2] > markers[3]) {
    subtractupper = markers[2];
    subtractlower = markers[3];
  } else {
    subtractupper = markers[3];
    subtractlower = markers[2];
  }
  /* --- the the destination histogram and the add --- */
  printf("Please enter the destination histogram and a constant to add.");
  scanf("%d %d", &histdest, &add);
  /* --- make sure that the destination histogram is at most one more than the
     --- last histogram in memory */
  lasthist = GetLastSpectrum();
  if (histdest > (lasthist + 1)) {
    histdest = lasthist + 1;
    printf("The destination histogram has been changed to %d.\n", histdest);
  }

  switch (twoddata.filetype) {
  case 1:
    /* --- get the nessasary information and pass it to the function --- */
    printf("Passing the following information to TwdProject: %d %d %d %d %d %d.\n", addlower,
           addupper, subtractlower, subtractupper, histdest, add);
    TwdProject(addlower, addupper, subtractlower, subtractupper, histdest, add);
    break;
  case 2:
    printf("Project from the x or y axis?");
    scanf("%s", dummystr);
    if (strstr(dummystr, "x") != NULL)
      xory = 0;
    if (strstr(dummystr, "y") != NULL)
      xory = 1;
    SqrProject(addlower, addupper, subtractlower, subtractupper, histdest, add, xory);
    break;
  default:
    printf("There does not seem to be a gs92 .twd or .sqr in memory.\n");
  }
}

/*
 * ProjectFull
 *
 * Projects from either a .twd or a .sqr depending on what is in memory.
 * the projection is a complete projection with no subtraction
 */
void ProjectFull() {

  switch (twoddata.filetype) {
  case 1:
    TwdProject(0, (twoddata.size - 1), 0, 0, 0, 0);
    break;
  case 2:
    SqrProject(0, (twoddata.size - 1), 0, 0, 0, 0, 0);
    SqrProject(0, (twoddata.size - 1), 0, 0, 1, 0, 1);
    break;
  default:
    printf("There does not seem to be a gs92 .twd or .sqr in memory.\n");
  }
}

/*
 * ManualProject
 *
 * Projects manually from a .twd or .sqr
 * Prompts the user for an add gate and
 * either a subtract gate or a subtract fraction (from the total projection)
 */
void ManualProject() {

  /* --- local variables --- */
  int addlower, addupper, subtractlower, subtractupper, histdest, add, xory;
  int lasthist;
  char dummystr[3];

  printf("What is the range of the add gate?");
  scanf("%d %d", &addlower, &addupper);
  printf("What is the range of the subtract gate?\n (or \"-<counts> 0\" to subtract a fraction of "
         "the total projection).");
  scanf("%d %d", &subtractlower, &subtractupper);
  printf("What is the destination histogram and a constant to add?");
  scanf("%d %d", &histdest, &add);

  /* --- make sure that the destination histogram is at most one more than the
     --- last histogram in memory */
  lasthist = GetLastSpectrum();
  if (histdest > (lasthist + 1)) {
    histdest = lasthist + 1;
    printf("Destination histogram changed to %d.\n", histdest);
  }

  /* --- prompt for nessasary information --- */

  switch (twoddata.filetype) {
  case 1:
    TwdProject(addlower, addupper, subtractlower, subtractupper, histdest, add);
    break;
  case 2:
    printf("Project from the x or y axis?");
    scanf("%s", dummystr);
    if (strstr(dummystr, "x") != NULL)
      xory = 0;
    if (strstr(dummystr, "y") != NULL)
      xory = 1;
    SqrProject(addlower, addupper, subtractlower, subtractupper, histdest, add, xory);
    break;
  default:
    printf("There does not seem to be a gs92 .twd or .sqr in memory.\n");
  }
}

/*
 * TwdProject
 *
 * projects a .twd given the add gate, subtract gate, and destination histogram
 * and add
 */
void TwdProject(int addlower, int addupper, int subtractlower, int subtractupper, int desthist,
                int add) {
  /* --- local variables --- */
  int i, j, k;
  int temphist[8192];
  int newmemsize;
  int counts;
  int ix, iy, ind;
  float temp, temporary;
  int isize;
  int lasthist;
  int subcounts;
  float frac;
  float asratio;

  g_return_if_fail(twoddata.data != NULL);
  g_return_if_fail((addlower >= 0) && (addlower < twoddata.size));
  g_return_if_fail((addupper >= addlower) && (addupper < twoddata.size));
  //  g_return_if_fail((subtractlower >= 0) && (subtractlower < twoddata.size));
  g_return_if_fail((subtractupper >= subtractlower) && (subtractupper < twoddata.size));

  /* --- initialize --- */
  counts = 0;
  for (i = 0; i < 8192; i++)
    temphist[i] = 0;
  isize = 2 * twoddata.size + 1;
  subcounts = 0;

  if (subtractlower != subtractupper)
    asratio = (fabs(addlower - addupper) / fabs(subtractlower - subtractupper));
  else
    asratio = 1;

  /* --- if subtractlower is less than zero we are going to need
     --- the number of counts in histogram 0 */
  if (subtractlower < 0) {
    for (i = 0; i < histsize[0]; i++) {
      subcounts = subcounts + *(histloc[0] + i);
    }
  }
  frac = (float)(addupper - addlower + 1) * abs(subtractlower) / (float)subcounts;
  // frac = (float)abs((double) subtractlower) / (float) subcounts;
  /* --- now we get to make the histogram --- */
  /* --- add gate first --- */
  StartProgress();
  for (i = 0; i < twoddata.size; i++) {
    UpdateProgress(i, twoddata.size);
    temporary = 0;
    for (j = addlower; j <= addupper; j++) {
      ix = Max(i, j);
      iy = Min(i, j);
      ind = ix - iy + (isize - iy) * ((float)iy / (float)2);
      temp = *(twoddata.data + ind);
      if ((temp < 0) && (twoddata.df > 0))
        temp = temp + 65536;
      if (ix == iy)
        temporary = temporary + temp; /* --- intentional double counting on hypotenus --- */
      temporary = temporary + temp;
    }
    /* --- and now the subtract gate --- */
    /* --- two ways, old fashioned way first --- */
    if (subtractlower > 0) {
      for (j = subtractlower; j <= subtractupper; j++) {
        ix = Max(i, j);
        iy = Min(i, j);
        ind = ix - isize + (isize - iy) * ((float)iy / (float)2);
        temp = *(twoddata.data + ind);
        if ((temp < 0) && (twoddata.df > 0))
          temp = temp + 65536;
        if (ix == iy)
          temporary = temporary - temp * asratio;
        temporary = temporary - temp * asratio;
      }
    } else {
      if (subtractlower < 0) {
        temporary = temporary - frac * *(histloc[0] + i);
      }
    }
    /* --- do the add --- */
    temporary = temporary + add;
    /* --- write it to the temporary histogram --- */
    temphist[i] = temporary;
    counts = counts + temporary;
  }
  EndProgress();
  printf("Counts in spectrum: %d\n", counts);

  /* --- Allocate memory and assign meaningful numbers --- */

  newmemsize = (int)twoddata.size * sizeof(int);
  ObliterateHistogram(desthist);
  histsize[desthist] = (int)twoddata.size;
  histid[desthist] = 0;
  sprintf(field1[desthist], "");
  sprintf(field2[desthist], "Gamma");
  sprintf(field3[desthist], "");
  histloc[desthist] = (int *)malloc(newmemsize);
  for (i = 0; i < twoddata.size; i++) {
    if (temphist[i] > 0) {
      *(histloc[desthist] + i) = (int)temphist[i];
    } else {
      *(histloc[desthist] + i) = 0;
    }
  }
  spectra = desthist;
  currentrange[0] = 0;
  currentrange[1] = histsize[desthist] - 1;
  spectradisplayed[0][0] = spectra;
  DisplayCurrentRange();

  //  printf("TwdManualProject doesn't do anything yet.\n");
}

/*
 * SqrProject
 *
 * Prompts for an add gate and either a
 * subtract gate or a subtract counts (goes to fraction from total projection)
 */
void SqrProject(int addlower, int addupper, int subtractlower, int subtractupper, int histdest,
                int add, int xory) {
  /* --- local variables --- */
  int i, j, k;
  int temphist[8192];
  int newmemsize, lasthist;
  long int subcounts;
  float frac;
  int counter, maxcounter;
  float asratio;

  g_return_if_fail(twoddata.data != NULL);
  g_return_if_fail((addlower >= 0) && (addlower < twoddata.size));
  g_return_if_fail((addupper >= addlower) && (addupper < twoddata.size));
  //  g_return_if_fail((subtractlower >= 0) && (subtractlower < twoddata.size));
  g_return_if_fail((subtractupper >= subtractlower) && (subtractupper < twoddata.size));

  /* --- initialization --- */
  for (i = 0; i < 8192; i++)
    temphist[i] = 0;
  subcounts = 0;
  if (subtractlower != subtractupper)
    asratio = (fabs(addlower - addupper) / fabs(subtractlower - subtractupper));
  else
    asratio = 1;

  /* --- if subtractlower is negative we are going to want to know the number
     --- of counts in or reference histogram */
  if (subtractlower < 0) {
    for (i = 0; i < histsize[xory]; i++) {
      subcounts = subcounts + *(histloc[xory] + i);
    }
  }
  /* --- there we should have the number of counts to subtract --- */

  /* --- Now to generate the appropriate histogram --- */
  /* --- add gate first --- */
  maxcounter = twoddata.size * (addupper - addlower + 1);
  StartProgress();
  for (i = addlower; i <= addupper; i++) {
    for (j = 0; j < twoddata.size; j++) {
      counter = twoddata.size * i + j;
      UpdateProgress(counter, maxcounter);
      if (xory == 0)
        temphist[j] = temphist[j] + (int)*(twoddata.data + twoddata.size * i + j);
      if (xory == 1)
        temphist[j] = temphist[j] + (int)*(twoddata.data + twoddata.size * j + i);
    }
  }
  EndProgress();
  /* --- subtract gate second --- */
  /* --- first the more traditional subtract gate --- */
  if (subtractlower > 0) {
    maxcounter = twoddata.size * (subtractupper - subtractlower + 1);
    StartProgress();
    for (i = subtractlower; i <= subtractupper; i++) {
      for (j = 0; j < twoddata.size; j++) {
        counter = twoddata.size * i + j;
        UpdateProgress(counter, maxcounter);
        if (xory == 0)
          temphist[j] = temphist[j] - (int)*(twoddata.data + twoddata.size * i + j) * asratio;
        if (xory == 1)
          temphist[j] = temphist[j] - (int)*(twoddata.data + twoddata.size * j + i) * asratio;
      }
    }
    EndProgress();
  } else {
    if (subtractlower < 0) {
      frac = (float)(addupper - addlower + 1) * abs(subtractlower) / (float)subcounts;
      //   frac = - (float)subtractlower / (float)subcounts;
      for (i = 0; i < histsize[xory]; i++) {
        temphist[i] = temphist[i] - *(histloc[xory] + i) * frac;
      }
    }
  }
  /* --- do the add --- */
  if (add != 0) {
    for (i = 0; i < twoddata.size; i++) {
      temphist[i] = temphist[i] + add;
    }
  }
  /* --- get rid of the zeros --- */
  for (i = 0; i < twoddata.size; i++) {
    if (temphist[i] < 0)
      temphist[i] = 0;
  }
  /* --- write out the darned thing --- */
  newmemsize = (int)twoddata.size * sizeof(int);
  ObliterateHistogram(histdest);
  /* --- Write identifying information --- */
  histsize[histdest] = (int)twoddata.size;
  histid[histdest] = histdest;
  sprintf(field1[histdest], "");
  sprintf(field2[histdest], "Gamma");
  sprintf(field3[histdest], "");

  //  printf("histsize[%d]: %d",his\tdest,histsize[histdest]);
  histloc[histdest] = (int *)malloc(newmemsize);
  for (i = 0; i < twoddata.size; i++) {
    if (temphist > 0) {
      *(histloc[histdest] + i) = (int)temphist[i];
    } else {
      *(histloc[histdest] + i) = 0;
    }
  }

  //  printf("Projection completed, attempting to display.\n");
  //  printf("Size of new spectra : %d.\n",histsize[a]);
  /* --- display our new histogram --- */
  spectra = histdest;
  currentrange[0] = 0;
  currentrange[1] = histsize[histdest] - 1;
  spectradisplayed[0][0] = spectra;
  DisplayCurrentRange();
}

/*
 * Max
 *
 * returns the larger of the two ints passed to it
 */
int Max(int a, int b) {
  if (a >= b) {
    return a;
  } else {
    if (b > a)
      return b;
  }
}

/*
 * Min
 *
 * returns the smaller of the two ints passed to it
 */
int Min(int a, int b) {
  if (a <= b) {
    return a;
  } else {
    if (b < a)
      return b;
  }
}

/*
 * DopplerCorrect
 *
 * for beta != 0 call to doppler shift correct the energy of the gamma ray.
 */
void DopplerCorrect(int *energy, int detector) {
  int i;
  float theta;
  float cosang;
  theta = 90;

  switch (detector) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 6:
    theta = 0.3014995;
    break;
  case 5:
  case 7:
  case 9:
  case 8:
  case 10:
    theta = 0.5535742;
    break;
  case 11:
  case 12:
  case 13:
  case 14:
  case 16:
    theta = 0.652358;
    break;
  case 15:
  case 17:
  case 18:
  case 19:
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
  case 26:
    theta = 0.8737997;
    break;
  case 25:
  case 27:
  case 28:
  case 30:
  case 32:
    theta = 1.017222;
    break;
  case 29:
  case 31:
  case 33:
  case 34:
  case 35:
  case 36:
  case 37:
  case 38:
  case 40:
  case 42:
    theta = 1.218594;
    break;
  case 39:
  case 41:
  case 44:
  case 46:
  case 48:
    theta = 1.3820857;
    break;
  case 43:
  case 45:
  case 47:
  case 50:
  case 52:
    theta = 1.408648;
    break;
  case 49:
  case 51:
  case 53:
  case 54:
  case 55:
  case 56:
  case 57:
  case 58:
  case 60:
  case 62:
    theta = 1.579796;
    break;
  case 59:
  case 61:
  case 64:
  case 66:
  case 68:
    theta = 1.732944;
    break;
  case 63:
  case 65:
  case 67:
  case 70:
  case 72:
    theta = 1.759506;
    break;
  case 69:
  case 71:
  case 73:
  case 74:
  case 75:
  case 76:
  case 77:
  case 78:
  case 80:
  case 82:
    theta = 1.922998;
    break;
  case 79:
  case 81:
  case 83:
  case 84:
  case 86:
    theta = 2.124371;
    break;
  case 85:
  case 87:
  case 88:
  case 89:
  case 90:
  case 91:
  case 92:
  case 93:
  case 94:
  case 96:
    theta = 2.267793;
    break;
  case 95:
  case 97:
  case 99:
  case 98:
  case 100:
    theta = 2.4892344;
    break;
  case 101:
  case 103:
  case 102:
  case 104:
  case 106:
    theta = 2.488018;
    break;
  case 105:
  case 107:
  case 109:
  case 108:
  case 110:
    theta = 2.8400931;
  default:
    /* --- unknown detector angle, do nothing --- */
    break;
  }
  cosang = 1 - beta * cos(theta);
  *energy = (int)*energy * cosang;
}

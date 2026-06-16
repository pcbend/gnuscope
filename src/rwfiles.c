/* rwfiles.c
 *
 * by John Pavan
 *
 * For reading and writing files for gnuscope
 */

#include "gnuscopefuncs.h"
#include "gnuscopeglobals.h"

//--ddc 3jun08 problems with function prototypes, and fixing calls like
//--ddc strnlen, strnlen(char *, sizet), so strnlen(&dummystr,80), IS
//--ddc incorrect.

//--ddc 10apr09 problems with the 'ascii spe radware' files.  As fixing that,
//--ddc I've decided to make the spe files output binary (the ascii
//--ddc representation is not as useful, and there are standalone conversion
//--ddc programs for this.

//--ddc may12 bug in READING binary spe files... fixed.
//--ddc jul12 correct implicit type conversion to prevent overflow for
//      large arrays.

/* --- Global Variables --- */

struct pgam_matrix_info pgammatrixdata;
//--ddc 10apr09 enum {ZERO,SPK,NSM,ASCII,PCNSM,GEN,CNF,ASC,SIG};
enum { ZERO, SPK, NSM, SPE, PCNSM, GEN, CNF, ASC, SIG };

int intype, spectra;
int currentrange[2];
int overwriteflag;
int writemin, writemax;
int binsize, yscaleforce, yscale;
long double displayused;

int lastfitnumpoints;
struct floatpoint *lastfitpoints;

GtkWidget *graph;
int ybinforce, ybin;

/* --- Function declarations --- */

void AutoDetectFileType(char *sFilename);
// void ReadGZAutoMatrix(char *sFilename);

/* --- functions --- */

/* AutoDetectFileType
 *
 * Takes the idiot's approach to determining
 * the file type.  Looking at the file
 * extention
 */
void AutoDetectFileType(char *sFilename) {
  char dummystr[80];
  glob_t globresults;
  int i;

  globresults.gl_offs = 500;
  if (glob(sFilename, GLOB_TILDE, NULL, &globresults) == 0) {
    for (i = 0; i < globresults.gl_pathc; i++) {

      if (strstr(sFilename, ".spk") != NULL) {
        /* --- it must be an .spk file --- */
        intype = SPK;
        ReadFile(sFilename);
      }
      if (strstr(sFilename, ".NSM") != NULL) {
        /* --- it must be a binary .NSM file --- */
        intype = NSM;
        ReadFile(sFilename);
      }
      if (strstr(sFilename, ".spe") != NULL) {
        /* --- it must be a spe file --- */
        //--ddc 10apr09	intype = ASCII;
        intype = SPE;
        ReadFile(sFilename);
      }
      if (strstr(sFilename, ".txt") != NULL) {
        /* --- it must be a general text file --- */
        intype = GEN;
        ReadFile(sFilename);
      }
      if (strstr(sFilename, ".nsm") != NULL) {
        /* --- it must be an ascii nsm file --- */
        intype = PCNSM;
        ReadFile(sFilename);
      }
      if ((strstr(sFilename, ".cnf") != NULL) || (strstr(sFilename, ".CNF") != NULL)) {
        /* --- it is probably some type of CAM file --- */
        intype = CNF;
        ReadFile(sFilename);
      }
      if (strstr(sFilename, ".asc") != NULL) {
        intype = ASC;
        ReadFile(sFilename);
      }
      if (strstr(sFilename, ".sig") != NULL) {
        intype = SIG;
        ReadFile(sFilename);
      }
      /* --- ok now for the setup files for pgam --- */
      /* --- the one that we really care about is the master setup file --- */
      /* --- but we should also include the option to read in a normal setup
         --- file, but for testing only --- */
      if (strstr(sFilename, ".msf") != NULL) {
        /* --- it is a master setup file --- */
        PgamReadMasterSetup(sFilename);
      }
      if (strstr(sFilename, ".sf") != NULL) {
        /* --- it is a normal setup file --- */
        /* --- the user should be warned that htis is only useful for
           --- testing --- */
        GetMessageDialog(
            "Warning:  Normal setup files are only read in this way to test their validity.  If "
            "you want to sort using this setup file please make a master setup file.\n");
        PgamReadSetup(sFilename);
      }
      /* --- now we should set this up to read in 2D spectra files --- */
      if ((strstr(sFilename, ".big") != NULL) || (strstr(sFilename, ".ede") != NULL)) {
        ReadBig(sFilename);
      }
      /* --- how about for bannana gate files --- */
      if ((strstr(sFilename, ".tgt") != NULL) || (strstr(sFilename, ".TGT") != NULL)) {
        ReadGates(sFilename);
      }
      /* --- radware matricies (.mat) --- */
      if ((strstr(sFilename, ".mat") != NULL) || (strstr(sFilename, ".MAT") != NULL)) {
        ReadRadwareMatrix(sFilename);
      }

      /* --- now for matricies --- */
      if ((strstr(sFilename, ".sqr") != NULL) || (strstr(sFilename, ".twd") != NULL)) {
        ReadAutoMatrix(sFilename);
      }
      /* --- now for command files --- */
      if (strstr(sFilename, ".cmd") != NULL) {
        CommandFileHandler(sFilename);
      }
      printf("Read file %s\n", globresults.gl_pathv[i]);
    }
    globfree(&globresults);
  }
}

void init_ornl(struct ornldir *spkdir, struct ornlheader *spkhead) {
  int i;
  /*
     reset the "ornl file pointer", which points to the position
     of various data in the file in units of "half-words" (2bytes!)
     to zero...
  */

  ornlfile = 0;

  /* initialize all the elements of the directory */

  strncpy(spkdir->datatype, "HHIRFSPK", 8);
  spkdir->numspec = 0;
  spkdir->nextrec = 1025; /*assumes no more than 254 histograms!*/
  for (i = 0; i < 254; i++) {
    spkdir->entry[i].id = 0;
    spkdir->entry[i].location = 0;
  }

  /* initialize all the elements of the header having "static" values */
  spkhead->bytes = -4;
  spkhead->length = 64;
  spkhead->dimensions = 1;
  spkhead->record = 0;
  spkhead->datalength = 0;
  for (i = 0; i < 3; i++)
    spkhead->calibration[i] = 0;
  strncpy(spkhead->label, "label test", 12);
  strncpy(spkhead->reserved, "        ", 8);
  strncpy(spkhead->timestamp, "            ", 12);
  strncpy(spkhead->timestamp, "INVALID", 12);
  return;
}

/*********************************************************************/

/*
   some routines for reading/writing out the FSU directory and header
   records, which required special treatment due to byte boundary problems
   in defining the structures.
*/

int read_fsuhead(struct fsuheader *head, FILE *input) {

  fread(&head->buffsize, 4, 1, input);  /*record starts with record size */
  fread(&head->numchn, 2, 1, input);    /*number channels in histogram*/
  fread(&head->itype, 2, 1, input);     /*type of histogram... must be 2! */
  fread(head->label, 6, 1, input);      /*histogram label*/
  fread(&head->truet, 4, 1, input);     /*truetime for ADC, or livetime*/
  fread(&head->buffcheck, 4, 1, input); /*record ends with record size :) */

  if (head->buffsize != head->buffcheck) {
    GetMessageDialog("\n Error reading NSM file, exiting.\n");
    exit(-1);
  }

  return (0);
}

int read_fsudir(struct fsudir *dir, FILE *input) {

  fread(&dir->buffsize, 4, 1, input);  /*each record starts with record size */
  fread(&dir->numrun, 2, 1, input);    /*the run number*/
  fread(&dir->numspec, 2, 1, input);   /*the number of spectra in the file*/
  fread(&dir->trutim, 4, 1, input);    /*the ACTUAL truetime */
  fread(dir->dat, 9, 1, input);        /*date field*/
  fread(dir->tim, 8, 1, input);        /*time field*/
  fread(&dir->buffcheck, 4, 1, input); /*fortran file record ends with size :) */

  if (dir->buffsize != dir->buffcheck) {
    GetMessageDialog("\n Error reading NSM file, exiting.\n");
    exit(-1);
  }

  return (0);
}

int write_fsuhead(struct fsuheader *head, FILE *output) {

  fwrite(&head->buffsize, 4, 1, output);  /*record starts with record size */
  fwrite(&head->numchn, 2, 1, output);    /*number channels in histogram*/
  fwrite(&head->itype, 2, 1, output);     /*type of histogram... must be 2! */
  fwrite(head->label, 6, 1, output);      /*histogram label*/
  fwrite(&head->truet, 4, 1, output);     /*truetime for ADC, or livetime*/
  fwrite(&head->buffcheck, 4, 1, output); /*record ends with record size :) */

  return (0);
}

int write_fsudir(struct fsudir *dir, FILE *output) {

  fwrite(&dir->buffsize, 4, 1, output);  /*each record starts with record size */
  fwrite(&dir->numrun, 2, 1, output);    /*the run number*/
  fwrite(&dir->numspec, 2, 1, output);   /*the number of spectra in the file*/
  fwrite(&dir->trutim, 4, 1, output);    /*the ACTUAL truetime */
  fwrite(dir->dat, 9, 1, output);        /*date field*/
  fwrite(dir->tim, 8, 1, output);        /*time field*/
  fwrite(&dir->buffcheck, 4, 1, output); /*fortran record ends with size :) */
  return (0);
}

/*
 * WriteFile
 *
 * sFilename - file to write
 *
 * writes out the file in the specified format
 */
void WriteFile(char *sFilename) {
  /* --- Variables --- */

  int i, j, k;
  int a, b, validspectratest;
  char dummystr[200];

  struct ornldir spkdir;
  struct ornlheader spkhead;
  struct fsudir nsmdir;
  struct fsuheader nsmhead;

  int data[262144], nspec, nchan;
  short int shorthist[16386];
  int ld[16];

  char tempchar[40];
  char tempstring[10][40], templine[132];

  FILE *OUTPUT;
  int items, buffsiz, buffsiz2;
  int *testbuf;
  float tempfloat;

  /* --- prompt for the spectra to write --- */
  //  GetDialog(24);
  a = writemin;
  b = writemax;

  sprintf(dummystr, "%s", sFilename);

  /* --- set the intype by the file extention --- */
  if (strstr(dummystr, ".NSM") != NULL)
    intype = NSM;
  if (strstr(dummystr, ".nsm") != NULL)
    intype = PCNSM;
  if (strstr(dummystr, ".txt") != NULL)
    intype = GEN;
  if (strstr(dummystr, ".spe") != NULL)
    //--ddc 10apr09	intype = ASCII;
    intype = SPE;
  if (strstr(dummystr, ".spk") != NULL)
    intype = SPK;

  /* --- make sure that those are valid spectra --- */
  validspectratest = 1;
  for (i = a; i <= b; i++) {
    if (histsize[i] == 0) {
      validspectratest = 0;
      GetMessageDialog("At least one of the spectra you want to write is empty.\n");
      GetMessageDialog("Write failed.\n");
    }
  }

  if (validspectratest != 0) {

    /* --- want to make sure that the correct extention is used
       --- for the files so we can easily read them back in --- */

    switch (intype) {
    case SPK:
      if (strstr(dummystr, ".spk") == NULL)
        strcat(dummystr, ".spk");
      break;
    case NSM:
      if (strstr(dummystr, ".NSM") == NULL)
        strcat(dummystr, ".NSM");
      break;
      //--ddc 10apr09    case ASCII:
    case SPE:
      if (strstr(dummystr, ".spe") == NULL)
        strcat(dummystr, ".spe");
      break;
    case PCNSM:
      if (strstr(dummystr, ".nsm") == NULL)
        strcat(dummystr, ".nsm");
      break;
    case GEN:
      if (strstr(dummystr, ".txt") == NULL)
        strcat(dummystr, ".txt");
      break;
    default:
      break;
    }

    /* --- make sure we can write --- */
    if ((OUTPUT = fopen(dummystr, "w")) == NULL) {
      GetMessageDialog("Some problem opening the output file, exiting.\n");
      exit(-1);
    }

    switch (intype) {

    case SPK:

      /* for each new file, must initialize the data structures */

      init_ornl(&spkdir, &spkhead);

      /*
         create the header for the spk file.  The variable ornlfile is the file
         location in "half-words" which are 2bytes long--ddc
      */

      ornlfile = spkdir.nextrec;

      for (i = a; i <= b; i++) {
        spkdir.entry[i].id = i + 1;
        spkdir.entry[i].location = ornlfile;
        ornlfile = ornlfile + spkhead.length + 2 * histsize[i];
      }

      spkdir.nextrec = ornlfile;

      /* and we put the number of entries in the directory...! */
      spkdir.numspec = b - a + 1;
      /* ok that's it for the directory, dump it out... */
      fwrite(&spkdir, 2048, 1, OUTPUT);
      /* and, now, write out the spectra... */
      for (j = a; j <= b; j++) {
        spkhead.id = spkdir.entry[j].id;
        spkhead.minchan = 0;
        spkhead.maxchan = histsize[j] - 1;
        spkhead.channels = histsize[j];
        /*      spkhead.raw=histsize[j]-1; */
        spkhead.raw = histsize[j];
        spkhead.scaled = histsize[j] - 1;
        strncpy(spkhead.title, field1[j], 40);
        strncpy(spkhead.label, field2[j], 12);
        strncpy(spkhead.timestamp, field3[j], 12);

        fwrite(&spkhead, 128, 1, OUTPUT);
        fwrite(histloc[j], 4 * histsize[j], 1, OUTPUT);
      }

      /*
        oops, oh boy, the spk files are fortran "direct" files with a
        recordlength of 2048 bytes, so write out "something" to fill in the
        last record.
      */
      for (i = 0; i < 2 * (2049 - ornlfile % 2048); i++)
        fwrite(" ", 1, 1, OUTPUT);

      break;

    case NSM:

      /*
         for each new file, must write the "directory"... the files are fortran
         variable length records, which means they have a four byte header with
         the length in ... well who knows :) but it is bytes on the DEC unix
         machines, and in g77.
      */

      nsmdir.buffsize = 25;
      nsmdir.numspec = b - a + 1;
      nsmdir.trutim = 0; /* FIX? */
      nsmdir.numrun = 0; /* FIX? */
      strncpy(nsmdir.dat, field3[0], 9);
      strncpy(nsmdir.tim, &field3[0][9], 8);
      nsmdir.buffcheck = 25;

      write_fsudir(&nsmdir, OUTPUT);

      for (i = a; i <= b; i++) {

        nsmhead.buffsize = 14;
        nsmhead.numchn = histsize[i];
        nsmhead.truet = 0;
        nsmhead.itype = 2; /* ONLY integer*4 histograms are written */
        strncpy(nsmhead.label, field2[i], 6);
        nsmhead.buffcheck = 14;

        write_fsuhead(&nsmhead, OUTPUT);

        buffsiz = 4 * (unsigned int)nsmhead.numchn;
        fwrite(&buffsiz, 4, 1, OUTPUT); /* write buffersize */
        fwrite(histloc[i], buffsiz, 1, OUTPUT);
        fwrite(&buffsiz, 4, 1, OUTPUT); /* again, for the dumb f77 programs */
      }

      break;
      //--ddc 10apr09    case ASCII:
    case SPE:
      i = 0;
      for (i = a; i <= b; i++) {
        buffsiz = 8 + 4 * sizeof(int);

        fwrite(&buffsiz, 4, 1, OUTPUT);

        sscanf(templine, "%s         ", field1[i]);
        fwrite(templine, 8, 1, OUTPUT);

        //-ddc 10apr09.. if the spe files are used with escl8r, they
        // must be at LEAST the size of the matrix (4096!).

        if (histsize[i] < 4096) {
          j = 4096;
        } else {
          j = histsize[i];
        }
        fwrite(&j, 4, 1, OUTPUT);
        j = 1;
        fwrite(&j, 4, 1, OUTPUT);
        fwrite(&j, 4, 1, OUTPUT);
        fwrite(&j, 4, 1, OUTPUT);

        fwrite(&buffsiz, 4, 1, OUTPUT);

        if (histsize[i] < 4096) {
          buffsiz = 4096 * sizeof(float);
        } else {
          buffsiz = histsize[i] * sizeof(float);
        }
        fwrite(&buffsiz, 4, 1, OUTPUT);
        //--ddc 10apr09 the binary spe files are floats, so convert...
        for (j = 0; j < histsize[i]; j++) {
          tempfloat = *(histloc[i] + j);
          fwrite(&tempfloat, 4, 1, OUTPUT);
        }
        if (histsize[i] < 4096) {
          tempfloat = 0;
          for (j = histsize[i]; j < 4096; j++)
            fwrite(&tempfloat, 4, 1, OUTPUT);
        }
        fwrite(&buffsiz, 4, 1, OUTPUT);
      }

      break;

    case PCNSM:
      i = 0;
      for (i = a; i <= b; i++) {
        fprintf(OUTPUT, "            %d %d %s\n", histid[i], histsize[i], field1[i]);
        for (j = 0; j < histsize[i]; j = j + 16) {
          fprintf(OUTPUT, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d \n", *(histloc[i] + j),
                  *(histloc[i] + j + 1), *(histloc[i] + j + 2), *(histloc[i] + j + 3),
                  *(histloc[i] + j + 4), *(histloc[i] + j + 5), *(histloc[i] + j + 6),
                  *(histloc[i] + j + 7), *(histloc[i] + j + 8), *(histloc[i] + j + 9),
                  *(histloc[i] + j + 10), *(histloc[i] + j + 11), *(histloc[i] + j + 12),
                  *(histloc[i] + j + 13), *(histloc[i] + j + 14), *(histloc[i] + j + 15));
        }
      }
      break;

    case GEN:
      i = 0;
      for (i = a; i <= b; i++) {
        fprintf(OUTPUT, "#            %d %d %s\n", histid[i], histsize[i], field1[i]);
        nchan = 0;
        for (j = 0; j < histsize[i]; j++, nchan++) {
          fprintf(OUTPUT, "%d %d\n", nchan, *(histloc[i] + j));
        }
      }
      break;
    default:
      GetMessageDialog("Sorry, the data type you specified is unavailable!\n");
      exit(-1);
    }

    fclose(OUTPUT);
  }
}

/*
 * WriteFilePrompt
 *
 * sets the prompt for writting spectra
 */
void WriteFilePrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Which histograms do you wish to write?\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "A to B\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * WriteFileEntry
 *
 * Get the entry about weither or not to retain the spectra
 */
void WriteFileEntry(GtkWidget *widget, GtkWidget *entry) {
  char dummystr[80];
  int test, a, b;
  overwriteflag = 0;
  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%d %d", &a, &b);
  if (test == 2) {
    writemin = a - 1;
    writemax = b - 1;

    GetFilename("Write", 1, WriteFile);
  }
}

/*
 * RetainSpectraPrompt
 *
 * Make a message saying how many spectra are in memory and ask to retain them
 */
void RetainSpectraPrompt(GtkWidget *text) {
  char dummystr[80];
  int i, j;

  j = 0;
  for (i = 0; i < 1028; i++) {
    if (histsize[i] > 0)
      j++;
  }
  sprintf(dummystr, "Do you wish to retain the %d spectra in memory? (YES,no)", j);
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * RetainSpectraEntry
 *
 * Get the entry about weither or not to retain the spectra
 */
void RetainSpectraEntry(GtkWidget *widget, GtkWidget *entry) {
  char dummystr[80];
  int test;
  overwriteflag = 0;
  //--ddc aug11 OOPS  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)),"%s",&dummystr);
  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%s", dummystr);
  if (test == 1) {
    if (strstr(dummystr, "n") != 0)
      overwriteflag = 1;
    if (strstr(dummystr, "N") != 0)
      overwriteflag = 1;
    GetFilename("Read", 0, AutoDetectFileType);
  }
}

/*
 * ReadFile
 *
 * sFilename - file to load
 *
 * reads in the file and stores it in the global histograms
 */
void ReadFile(char *sFilename) {
  int newmemsize;
  int i, j, k, l;

  struct ornldir spkdir;
  struct ornlheader spkhead;
  struct fsudir nsmdir;
  struct fsuheader nsmhead;

  int nspec, nchan;
  short int shorthist[16386];
  int ld[16];

  char tempchar[40];
  char tempstring[10][40], templine[132];
  float tempfloat;

  FILE *INPUT;
  int items, buffsiz, buffsiz2;
  int *testbuf;
  int existingspectra;
  char keepoldspectra[3];
  char dummystr[80];
  int spectrastart = 0;

  float wa, wb, a, b, olda;
  int test;

  INPUT = NULL;

  /* --- if there are existing spectra ask if they should be kept --- */
  existingspectra = 0;
  for (i = 0; i < 1028; i++) {
    if (histsize[i] != 0)
      existingspectra = existingspectra + 1;
  }
  if (overwriteflag) {
    /* --- we want to get rid of all of the current spectra and replace them --- */
    i = 0;
    while (histsize[i] != 0) {
      ObliterateHistogram(i);
      i = i + 1;
    }
    spectrastart = 0;
  } else {
    spectrastart = existingspectra;
  }

  /* --- read in each type of data --- */
  switch (intype) {
    /* --- let's handle these in order --- */
    /* --- 1 - Binary spk flies --- */
  case 1:
    /* --- Let's try to open the file --- */
    if ((INPUT = fopen(sFilename, "r")) == NULL) {
      GetMessageDialog("\nInput file not found, exiting.\n");
      exit(0);
    }
    /* for each new file, must read the directory... */
    fread(&spkdir, 2048, 1, INPUT);
    nspec = spkdir.numspec;
    for (i = spectrastart; i < (nspec + spectrastart); i++) {
      histid[i] = spkdir.entry[i].id;
      fread(&spkhead, 128, 1, INPUT);
      /* histsize[i]=spkhead.raw+1; NOTE:not confirmed as always true!*/
      histsize[i] = spkhead.raw;
      newmemsize = sizeof(int) * histsize[i];
      histloc[i] = (int *)malloc(newmemsize);
      strncpy(field1[i], spkhead.title, 40);
      strncpy(field2[i], spkhead.label, 12);
      strncpy(field3[i], spkhead.timestamp, 12);
      /* make sure char strings are null terminated */
      field1[i][39] = '\0';
      field2[i][11] = '\0';
      field3[i][11] = '\0';
      /*
        sprintf(dummystr,"Spk date field... %40s \n",field3[i]);
        WriteMainText(dummystr);
        sprintf(dummystr,"Spk minchn, channels, maxchan, rawsize:  %d %d %d %d\n",
        spkhead.minchan, spkhead.channels, spkhead.maxchan, spkhead.raw);
        WriteMainText(dummystr);
      */
      printf("Spectrum %d \t %s \n", histid[i], field1[i]);
      /*
       * a spk histogram may not be complete... it usually truncates
       * the histogram to exclude ranges that are filled with zero's at the
       * beginning and ends of the files... it also assumes the first element
       * is channel zero for counting purposes. start by zeroing the channels
       * to the "minimum non-zero" channel, then read in the data, and then
       * zero from above the "maximum non-zero channel" to the end of the histogram.
       */

      for (j = 0; j < spkhead.minchan; j++)
        histloc[i][j] = 0;
      /* fread(&histloc[i][j],4*spkhead.channels,1,INPUT); */
      fread(&histloc[i][j], 4 * (spkhead.maxchan - spkhead.minchan + 1), 1, INPUT);
      for (j = spkhead.maxchan + 1; j < spkhead.raw; j++)
        histloc[i][j] = 0;
    }
    break;
    /* --- 2 - Binary nsm files --- */
  case 2:
    /* --- Let's try to open the file --- */
    if ((INPUT = fopen(sFilename, "r")) == NULL) {
      GetMessageDialog("\nInput file not found, exiting.\n");
      exit(0);
    }
    /*
     * for each new file, must read the "directory"... the files are fortran
     * variable length records, which means they have a four byte header with
     * the length in ... well who knows :) but it is bytes on the DEC unix
     * machines, and in g77.
     */
    read_fsudir(&nsmdir, INPUT);
    sprintf(dummystr, "sizeof... %d bytes dir %d %d \n", sizeof(struct fsudir), nsmdir.buffsize,
            nsmdir.buffcheck);
    WriteMainText(dummystr);
    nspec = nsmdir.numspec;
    for (i = spectrastart; i < (spectrastart + nspec); i++) {
      histid[i] = i + 1;
      read_fsuhead(&nsmhead, INPUT);
      histsize[i] = nsmhead.numchn;
      newmemsize = sizeof(int) * histsize[i];
      histloc[i] = (int *)malloc(newmemsize);
      sprintf(field1[i], "Run %d TT: %d LT: %d \n", nsmdir.numrun, nsmdir.trutim, nsmhead.truet);
      strncpy(field2[i], nsmhead.label, 6);

      /* aug12 make sure char strings are null terminated, and place limits
         in sprintf! */
      field1[i][6] = '\0';
      sprintf(field3[i], "%.9s %.8s\n", nsmdir.dat, nsmdir.tim);
      sprintf(dummystr, "NSM date field... %s \n", field3[i]);
      WriteMainText(dummystr);
      fread(&buffsiz, 4, 1, INPUT); /* Get (and discard :) buffersize */
      //--ddc jul12 fix unintentional type conversion resulting in overflow.
      if (nsmhead.itype == 2) {
        fread(histloc[i], 4 * (unsigned int)nsmhead.numchn, 1, INPUT);
      } else if (nsmhead.itype == 1) {
        fread(shorthist, 2 * (unsigned int)nsmhead.numchn, 1, INPUT);
        for (j = 0; j < nsmhead.numchn; j++)
          histloc[i][j] = shorthist[j];
      } else {
        GetMessageDialog("\n Histogram type in NSM file wrong... exiting. \n");
        exit(-1);
      }
      fread(&buffsiz2, 4, 1, INPUT); /* Get (and discard :) buffersize */
      if (buffsiz != buffsiz2) {
        GetMessageDialog("\n Error reading NSM file, exiting.\n");
        exit(-1);
      }
    }
    break;
    /* --- 3 -  spe file --- */
  case 3:

    /* --- Let's try to open the file --- */
    if ((INPUT = fopen(sFilename, "r")) == NULL) {
      GetMessageDialog("\nInput file not found, exiting.\n");
      exit(0);
    }
    i = spectrastart;

    //--ddc 10apr09 This seems to be a mistake inherited from my translate
    // code (where I did not test that the text field made sense (no nulls!)
    // AND had EIGHT (not nine) characters...
    //
    //    while((items=fread(field1[i],9,1,INPUT)) != 0){
    //
    // while((items=fread(field1[i],8,1,INPUT)) != 0){
    //      /* Three of the four next parameters seem to have no function, and are 1,1,1 */
    //      fscanf(INPUT,"%d %d %d %d",&histsize[i],&j,&j,&j);
    //      newmemsize = sizeof(int) * histsize[i];
    //      histloc[i] = (int *) malloc(newmemsize);
    //      for(j=0;j<histsize[i];j=j+16){
    //	if((items=fscanf(INPUT,"%d %d %d %d %d %d %d %d ",
    //			 (histloc[i]+j),(histloc[i]+1+j),(histloc[i]+2+j),
    //			 (histloc[i]+3+j),(histloc[i]+4+j),(histloc[i]+5+j),
    //			 (histloc[i]+j+6),(histloc[i]+j+7))) != 8) {
    //	}
    //      }
    //       nspec=++i;
    //    }
    //--ddc 10apr09 I've replaced the above block to convert this to binary
    // files (the preferred by anyone who needs these!).
    while (items = fread(&buffsiz, 4, 1, INPUT) != 0) {
      fread(field1[i], 8, 1, INPUT);
      if ((items = fread(&histsize[i], 4, 1, INPUT)) != 1)
        break;
      fread(&j, 4, 1, INPUT); // and read the 3 ones
      fread(&j, 4, 1, INPUT);
      fread(&j, 4, 1, INPUT);
      fread(&buffsiz, 4, 1, INPUT); // end of the buffer

      newmemsize = sizeof(int) * histsize[i];
      histloc[i] = (int *)malloc(newmemsize);
      fread(&buffsiz, 4, 1, INPUT); // next buffer
      for (j = 0; j < histsize[i]; j = j + 1) {
        if ((items = fread(&tempfloat, 4, 1, INPUT)) != 1)
          break;
        *(histloc[i] + j) = tempfloat;
      }
      fread(&buffsiz, 4, 1, INPUT); // end buffer
      nspec = ++i;
    }
    break; /*--ddc may12. Oops should have been a break here*/

    /* --- 4 - ASCII nsm file --- */
  case 4:
    if (INPUT == NULL)
      /* --- Let's try to open the file --- */
      if ((INPUT = fopen(sFilename, "r")) == NULL) {
        GetMessageDialog("\nInput file not found, exiting.\n");
        exit(0);
      }
    i = spectrastart;
    while (items = fgets(templine, 132, INPUT) != NULL) {
      items = sscanf(templine, "%d %d %s %s %s %s %s %s %s %s %s %s\n", &histid[i], &histsize[i],
                     tempstring[0], tempstring[1], tempstring[2], tempstring[3], tempstring[4],
                     tempstring[5], tempstring[6], tempstring[7], tempstring[8], tempstring[9]);

      if (items == 3) {
        sprintf(field1[i], "Pol: %s    ", tempstring[0]);
      } else if (items > 3) {
        k = spectrastart;
        do {
          strcat(field1[i], " ");
          strcat(field1[i], tempstring[k]);
          k++;
        } while ((k < items - 2) && (k < 10));
      }
      sprintf(dummystr, "Entry %d histogram %d, size %d title: %s\n", i, histid[i], histsize[i],
              field1[i]);
      WriteMainText(dummystr);
      newmemsize = sizeof(int) * histsize[i];
      histloc[i] = (int *)malloc(newmemsize);
      for (j = 0; j < histsize[i]; j = j + 16) {
        if ((items = fscanf(INPUT, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                            (histloc[i] + j), (histloc[i] + j + 1), (histloc[i] + j + 2),
                            (histloc[i] + j + 3), (histloc[i] + j + 4), (histloc[i] + j + 5),
                            (histloc[i] + j + 6), (histloc[i] + j + 7), (histloc[i] + j + 8),
                            (histloc[i] + j + 9), (histloc[i] + j + 10), (histloc[i] + j + 11),
                            (histloc[i] + j + 12), (histloc[i] + j + 13), (histloc[i] + j + 14),
                            (histloc[i] + j + 15))) != 16) {
        }
      }
      nspec = ++i;
    }
    break;
    /* --- the case of general ascii --- */
  case GEN:

    /* --- Let's try to open the file --- */
    if ((INPUT = fopen(sFilename, "r")) == NULL) {
      GetMessageDialog("\nInput file not found, exiting.\n");
      exit(0);
    }
    {
      /* --- for general ascii, we will assume that the data is contained
         --- in a two-column list separated with either comas or spaces
         --- any header information is ignored --- */
      /* --- we assume that we are going to a new histogram when the x value
         --- drops to zero or 1 --- */
      /* --- data will be written to histograms via the <new> AddValToHistogram(spec,x,y)
         --- functions which will handle memory allocation --- */

      i = spectrastart;
      olda = 0;
      if (histloc[i] == NULL) {
        histloc[i] = (int *)malloc(sizeof(int) * 8192);
      } else {
        histloc[i] = (int *)realloc(histloc[i], sizeof(int) * 8192);
      }
      histsize[i] = 8192;
      // initialized
      {
        int ll;
        for (ll = 0; ll < 8192; ll++)
          histloc[i][ll] = 0;
      }
      while (fgets(dummystr, 80, INPUT)) {
        if (strncmp(dummystr, "#", 1)) {
          wa = -1;
          wb = -1;
          test = sscanf(dummystr, "%f %f", &a, &b);
          if (test == 2) {
            wa = a;
            wb = b;
          }
          test = sscanf(dummystr, "%f,%f", &a, &b);
          if (test == 2) {
            wa = a;
            wb = b;
          }
          if (wa > -1) {
            if (wa < olda) {
              histsize[i] = olda;
              histloc[i] = (int *)realloc(histloc[i], sizeof(int) * olda);
              olda = 0;
              i++;
              if (histsize[i] == 0) {
                histloc[i] = (int *)malloc(sizeof(int) * 8192);
                histsize[i] = 8192;

                {
                  int ll;
                  for (ll = 0; ll < 8192; ll++)
                    histloc[i][ll] = 0;
                }
              }
              histid[i] = i;
              sprintf(field1[i], "textin");
              sprintf(field2[i], "textin");
              sprintf(field3[i], "textin");
            }
            AddValToHistogram(i, (int)wa, (int)wb);
            olda = wa;
          }
        }
      }
      if (olda > 0) {
        histsize[i] = olda;
        histloc[i] = (int *)realloc(histloc[i], histsize[i] * sizeof(int));
      }
    }
    break;
  case CNF:

    /* --- Let's try to open the file --- */
    if ((INPUT = fopen(sFilename, "rb")) == NULL) {
      GetMessageDialog("\nInput file not found, exiting.\n");
      exit(0);
    }
    i = spectrastart;
    /* --- let's try doing this in a slightly less stupid way
       --- cnf files all seem to have an occurance of 8197 (unsigned short int)
       --- 42 bytes ahead of the number of channels --- */
    {
      /* --- some very local variables --- */
      unsigned short int verytemp[1024];
      int continuecycle, errorflag;
      int ll, realheaderstart;
      short int num_channels;

      continuecycle = 1;
      errorflag = 0;
      realheaderstart = 0;
      while (continuecycle) {
        if (fread(verytemp, sizeof(unsigned short int), 1024, INPUT) == EOF) {
          continuecycle = 0;
          errorflag = 1;
          goto cycleover;
        }
        for (ll = 0; ll < 1024; ll++) {
          if (verytemp[ll] == 8197) {
            realheaderstart = ll * 2;
            continuecycle = 0;
            goto cycleover;
          }
        }
      }
    cycleover:
      if (errorflag) {
        WriteMainText("Error opeing file.\n");
      } else {
        fseek(INPUT, realheaderstart + 42, SEEK_SET);
        // printf("realheadstart = %d\n",realheaderstart);
        fread(&num_channels, sizeof(short int), 1, INPUT);
        // printf("number of channels %d\n",num_channels);
      }
      fseek(INPUT, -(num_channels * 4), SEEK_END);
      histloc[i] = (int *)malloc(sizeof(int) * num_channels);
      histsize[i] = num_channels;
      fread(histloc[i], sizeof(int), num_channels, INPUT);
    }
    //   /* --- this is the idiot version of reading in cnf files
    //   --- we are going to skip the first 28160 bytes
    //  --- and then get the data directly to the histogram --- */
    // fseek(INPUT,28160,SEEK_SET);
    // /* --- then we need to create the histogram --- */
    // histloc[i] = (int *) malloc(512 * sizeof(int));
    // histsize[i] = 512;
    ///* --- now we read in the information --- */
    // fread(histloc[i],sizeof(int),512,INPUT);
    break;
  case ASC:
    /* --- now let us try to read asc files from SpecTCL --- */
    if ((INPUT = fopen(sFilename, "r")) != NULL) {
      i = spectrastart;
      while (fgets(dummystr, 80, INPUT) != NULL) {
        /* --- these are fairly simple --- */
        /* --- get the file name and size --- */
        if (strncmp(dummystr, "\"", 1) == 0) {
          /* --- if the line began with a quote then it is the beginning
             --- of a spectrum --- */
          sscanf(dummystr, "%s (%d)", field1[i], &histsize[i]);
          histloc[i] = (int *)malloc(histsize[i] * sizeof(int));
          for (j = 0; j < histsize[i]; j++) {
            histloc[i][j] = 0;
          }
          /* --- get teh date-time stamp --- */
          fgets(field3[i], 40, INPUT);
          /* --- skip over 2 lines --- */
          fgets(dummystr, 80, INPUT);
          fgets(dummystr, 80, INPUT);
          /*--- now we can get the title --- */
          fgets(dummystr, 80, INPUT);
          sscanf(dummystr, "(\"%s\")", field2[i]);
          printf("Reading Spectrum %s.\n", field2[i]);
          /* --- skip a line --- */
          fgets(dummystr, 80, INPUT);
          /* --- reading --- */
          fgets(dummystr, 80, INPUT);
          sscanf(dummystr, "(%d) %d", &j, &k);
          while (j >= 0) {
            if (j < histsize[i])
              histloc[i][j] = k;
            fgets(dummystr, 80, INPUT);
            if (sscanf(dummystr, "(%d) %d", &j, &k) != 2)
              j = -1;
          }

          i++;
        }
      }
    }
    break;
  case SIG:
    /* ---  60 channels of total energy
       ---  10 channels of segment energy
       ---  50 channels of trace
       ---  next segment
       --- up to 36 segments
       --- stop reading with 1028 signals --- */
    if ((INPUT = fopen(sFilename, "r")) != NULL) {
      i = spectrastart;
      /* --- size and allocation of histogram --- */
      while ((i < 1028) && (fgets(dummystr, 80, INPUT))) {
        histsize[i] = 2220; // 37 * 60
        histloc[i] = (int *)malloc(histsize[i] * sizeof(int));
        j = 0;
        /* --- get total energy --- */
        sscanf(dummystr, "%f", &a);
        for (k = 0; k < 60; k++) {
          histloc[i][j] = (int)a;
          j++;
        }
        /* --- now get the next 36 signals --- */
        for (k = 0; k < 36; k++) {
          fgets(dummystr, 80, INPUT);
          sscanf(dummystr, "%f", &a);
          for (l = 0; l < 10; l++) {
            histloc[i][j] = (int)a;
            j++;
          }
          for (l = 0; l < 50; l++) {
            fgets(dummystr, 80, INPUT);
            sscanf(dummystr, "%f", &a);
            if (a < 10000)
              histloc[i][j] = (int)(a * 10000);
            else
              histloc[i][j] = 0;
            j++;
          }
        }
        i++;
      }
    }
    break;
  default:
    GetMessageDialog("Sorry, the data type you specified is unavailable!\n");
    exit(0);
  }
  fclose(INPUT);
}

/* AddValToHistogram(int spec, int x, int y)
 *
 * adds a value to histloc[spec] in channel x with value y
 */
void AddValToHistogram(int spec, int x, int y) {
  // printf("got this far\n");
  if (x >= 0) {
    if (x > histsize[spec]) {
      histloc[spec] = (int *)realloc(histloc[spec], sizeof(int) * (x + 8192));
      // printf("got past allocating memory.\n");
      histsize[spec] = x + 8192;
    }
    *(histloc[spec] + x) = y;
    // printf("spec %d, (%d,%d)\n",spec,x,*(histloc[spec]+y));
  }
}

/*
 * LiphaFile
 *
 * Writes the data from the active histogram to a lipha file
 */
void LiphaFile(char *sFilename) {
  int i, j, k;
  FILE *output;
  int max;
  GdkPoint *point;
  float x, y;
  int width;
  long double currentscaling;
  float temp;
  int jj;

  if ((output = fopen(sFilename, "w")) == NULL) {
    GetMessageDialog("Some problem opening the output file, exiting.\n");
    exit(-1);
  } else {
    width = currentrange[1] - currentrange[0] + 1;
    if (width)
      currentscaling = (long double)((float)(graph->allocation.width - 100) / (float)(width));
    for (i = 0; i < width; i++) {
      if ((i % 250) == 0) {
        if ((width - i) > 250) {
          fprintf(output, "250 2 1 0 2\n");
        } else {
          fprintf(output, "%d 2 1 0 2\n", (width - i));
        }
      }
      jj = i + currentrange[0];
      if (globalcalibrationset) {
        temp = (float)UseCalibration(jj);
      } else {
        temp = jj;
      }
      fprintf(output, "%f %d\n", temp, *(histloc[spectra] + jj));
    }
    if (lastfitnumpoints) {
      for (i = 0; i < lastfitnumpoints; i++) {
        if ((i % 250) == 0) {
          if ((lastfitnumpoints - i) > 250) {
            fprintf(output, "250 2 1 0 2\n");
          } else {
            fprintf(output, "%d 2 1 0 2\n", (lastfitnumpoints - i));
          }
        }
        jj = i + currentrange[0];
        if (globalcalibrationset) {
          temp = (float)UseCalibration(jj);
        } else {
          temp = jj;
        }
        fprintf(output, "%f %d\n", temp, *(histloc[spectra] + jj));
      }
    }
    fclose(output);
  }
}

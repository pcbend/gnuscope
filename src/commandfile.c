/* commandfile.c
 *
 * by John Pavan
 *
 * For handling command files for gnuscope
 * also for command line
 */

//--ddc aug11 Removing 'unused' stuff. All 'greta' to 'matlab'.

/* --- includes --- */
#include "gnuscopefuncs.h"
#include "gnuscopeglobals.h"

/* --- globals --- */
/* --- should be handled by gnuscopeglobals.h --- */

/* --- function declarations --- */
/* --- should be handled by gnuscopefuncs.h --- */

/* --- functions --- */

/* CommandLineHandler
 *
 * Interprets the results of commandline stuff
 */
void CommandLineHandler(int argc, char *argv[]) {
  int i, j, k;
  char dummystr[120];
  /* --- cycle through the command line options
     --- looking for stuff we recognize --- */
  i = 1;
  while (i < argc) {
    if ((strstr(argv[i], "--help") != NULL) || (strstr(argv[i], "-h") != NULL)) {
      /* --- this means that the user is asking for help --- */
      printf("Gnuscope Commandline Help:\n");
      printf("--compress <filename>                  Converts a .mul to a .ev2.\n");
      printf("--file <filename>     -f <filename>    Open a file at start up\n");
      printf("--mat <size> <filename>                Read a .mat file for viewing\n");
      printf("--cnf2txt <filename>                   \n");
      printf("--stop                                 Stop after executing command line.\n");
      printf("--help                -h               Prints this help\n");
      printf("--colordepth #                         Sets the number of colors for 2D Plots\n");

      printf("--");
      printf("\n\nCONFIGURATION FILE HELP ----\n");
      printf("     Gnuscope configuration files should be named \n");
      printf("\".gnuscopeconfig\" and reside in either the local directory or\n");
      printf("your home directory.  Gnuscope will look at the local directory\n");
      printf("first, and stop if it finds a .gnuscopeconfig file.  The options\n");
      printf("are as follows.\n\n");
      printf("in filename -- read file\n");
      printf("Set2DPoly Number -- Sets the Polynomial Degree for 2D Display.\n");
      printf("OverlayYSclaeMode 1/0 -- Determines the way Gnuscope sets the \n");
      printf("vertical scale when overlaying one dimensional plots. \n");
      printf("DensityPlotColors Color/BW -- Determines weither or not the\n");
      printf("2D Plots are in color or black and white.\n");
      printf("DensityPlotType Contour/Density/DensityAndContour -- Determines\n");
      printf("the starting mode for the 2D plots.\n");
      printf("ContourMode Linear/Log/Root/InverseLog/InverseRoot -- Determines\n");
      printf("the initial contour mode in 2D plots\n");
      printf("NumContours number -- the number of contours in 2D plots.\n");
      printf("DisplayMode Linear/Log/Root/EffCor -- determines the default\n");
      printf("display mode for 1D histograms.\n");
      printf("LineColor Num Color -- determines the color for overlayed spectra.  Num is the layer "
             "of for the color.  Color can be any valid colorstring.");
      /* --- we should exit after giving the help --- */
      exit(0);
    }
    if ((strstr(argv[i], "--file") != NULL) || (strstr(argv[i], "-f") != NULL)) {
      /* --- for starting by reading a file --- */
      /* --- what we now want to do is read in files until we find another flag --- */
      i++;
      while ((i < argc) && (strncmp(argv[i], "-", 1))) {
        printf("Opening file %s\n", argv[i]);
        AutoDetectFileType(argv[i]);
        i++;
        if (i >= argc)
          goto stopreadingcommand;
      }
    }
    if (strstr(argv[i], "--cnf2txt") != NULL) {
      /* --- for batch converting cnf files to txt files --- */
      /* --- want to read in files until we find anothr flag --- */
      i++;
      j = 0;
      while ((i < argc) && (strncmp(argv[i], "-", 1)) &&
             ((strstr(argv[i], ".cnf") != NULL) || (strstr(argv[i], ".CNF") != NULL))) {
        strncpy(dummystr, argv[i], strnlen(argv[i], 120) - 4);
        if (strstr(dummystr, ".txt") == NULL)
          strcat(dummystr, ".txt");
        printf("Converting %s to %s.\n", argv[i], dummystr);
        AutoDetectFileType(argv[i]);
        writemin = writemax = j;
        j++;
        WriteFile(dummystr);
        i++;
        if (i >= argc)
          goto stopreadingcommand;
      }
    }
    if (strstr(argv[i], "--stop") != NULL) {
      /* --- if we only want the command line functions --- */
      commandlineonly = 1;
      i++;
      if (i >= argc)
        goto stopreadingcommand;
    }

    /* --- set the color depth info --- */
    if (strstr(argv[i], "--colordepth") != NULL) {
      i++;
      if (sscanf(argv[i], "%d", &j) == 1) {
        colordepth = j;
      }
      i++;
      if (i >= argc)
        goto stopreadingcommand;
    } // done with color depth stuff
    if (strstr(argv[i], "--compress") != NULL) {
      /* --- for compressing .mul files to .ev2 files --- */
      i++;
      while ((i < argc) && (strstr(argv[i], "-") == NULL)) {
        printf("Compressing %s", argv[i]);
        Compress2FileNOGTK(argv[i]);
        i++;
        if (i >= argc)
          goto stopreadingcommand;
      }
    }
    if (strstr(argv[i], "--mat") != NULL) {
      /* --- read in a matrix file into the pgam structs --- */
      /* --- these matrix files are assumed to be square of the size specified
         --- and in the short int form --- */
      /* --- a couple of very local variables --- */
      int size;
      unsigned short int *tempshortptr;
      FILE *infile;
      i++;
      /* --- first we get the size --- */
      if (sscanf(argv[i], "%d", &j) == 1) {
        size = j;
      } else {
        printf("Error determining the size of the matrix.\n");
      }
      i++;
      /* --- make sure the file is there by trying to open it --- */
      if ((infile = fopen(argv[i], "rb")) != NULL) {
        printf("Reading %s\n", argv[i]);
        i++;
        /* --- now we need to read in the silly thing --- */
        /* --- first make space for each buffer --- */
        tempshortptr = (short int *)malloc(sizeof(short int) * size * size);
        /* --- now we need to make space for the matrix --- */

        pgammatrixdata.type = 0;
        pgammatrixdata.size = size;

        /* --- allocate enough memory --- */
        pgammatrixdata.data = (float *)malloc(sizeof(float) * size * size);

        /* --- and commence reading --- */
        fread(tempshortptr, sizeof(short int), size * size, infile);
        for (j = 0; j < (size * size); j++) {
          if (tempshortptr[j] != 0) {
            pgammatrixdata.data[j] = (float)tempshortptr[j];
            // printf("%d %f\n",tempshortptr[j],pgammatrixdata.data[j]);
          } else
            pgammatrixdata.data[j] = 0;
        }

        /* --- finally we need to free the buffer --- */
        free(tempshortptr);
        /* --- and close the file --- */
        fclose(infile);
      } else
        printf("Error reading file.\n");
    }
  }
  //--ddc gcc 3.4.4 doesn't like labels followed by ... nothing (added return)
stopreadingcommand:
  return;
}

/* CommandFileHandler
 *
 * Hopefully this can interpret command files
 */
void CommandFileHandler(char *sFilename) {
  FILE *infile;
  char dummystr[120];
  char dummystr2[120];
  int i, j, k, l;
  float a, b, c, d, e;

  /* --- let's make sure that we have some way to have comments --- */
  /* --- we'll do it the dumb way and
     --- look for the beginning of the line --- */
  if ((infile = fopen(sFilename, "r")) != NULL) {
    while (fgets(dummystr, 80, infile) != NULL) {
      /* --- pull in a line from the file to the buffer
         --- so we can scan it a couple of times --- */
      /* --- now we should check to make sure it isn't a comment line --- */
      if (strncmp(dummystr, "//", 2)) {
        /* --- if the strncmp is non-zero that means that it is not a comment line --- */
        /* --- let's list the commands that we want to have --- */
        /* --- read files (using AutoDetectFileType) [in <filename>] *
           --- set calibrations [cal <histogram> <a> <b> <c>] *
           --- set markers [mark <a>] *
           --- set background [background <a>] *
           --- sum [sum <a> <b> <background>] *
           --- single gauss fit [gauss <a> <b> <background>] *
           --- double gauss fit [doublegauss <minchan> <maxchan> <peak1> <peak2> <width1> <width2>
           <background>] *
           --- fixed full width half max gauss fit [fixedFWHMgauss <minchan> <maxchan> <FWHM>
           <background>] *
           --- exponential fit [expfit <minchan> <maxchan>] *
           --- read calibration info
           --- Peak Fit/autocalibrate
           --- Pgam Sort (all options)
           --- In order to do the sorting, we need to have a way to set the options
           --- so the way this will have to work is a set of options,
           --- followed by the command [sort]
           --- this will work by setting each type of options via a single line
           --- for instance :
           --- sort clear <---- clears out all the sorting options
           --- sort output tac
           --- sort output histogram
           --- sort output part <opt telescope zoom>
           --- sort output twd
           --- sort output ggsqr <filename>
           --- sort output pgsqr <particle final calibration>
           --- sort output pairsqr <filename>
           --- sort gate tac
           --- sort gate part <opt filename>
           --- sort gate gamma <minchan> <maxchan> or <filename>
           --- sort gate veto <filename>
           --- sort gate requires <filename>
           --- sort gate selfsuppressclovers
           --- sort gate minmultiplciity <minmultiplicity>
           --- sort sort <---- actually starts the sorting
           --- */
        /* --- start with the ability to read a file --- */
        if (strstr(dummystr, "in") != NULL) {
          if (sscanf(dummystr, "in %s", dummystr2) == 1) {
            AutoDetectFileType(dummystr2);
          } else {
            WriteMainText("Error determining which file to read.\n");
          }
        }
        /* --- set calibrations --- */
        if (strstr(dummystr, "cal") != NULL) {
          if (sscanf(dummystr, "cal %d %f %f %f", &i, &a, &b, &c) == 4)
            SetCalibration(i, a, b, c);
        }
        /* --- set markers --- */
        if (strstr(dummystr, "mark") != NULL) {
          if (sscanf(dummystr, "mark %d", &i) == 1) {
            markers[3] = markers[2];
            markers[2] = markers[1];
            markers[1] = markers[0];
            markers[0] = (i + 1);
          }
        }
        /* --- set background --- */
        if (strstr(dummystr, "background") != NULL) {
          if (sscanf(dummystr, "background %f", &a) == 1) {
            background[0] = a;
          }
        }
        /* --- sum --- */
        if (strstr(dummystr, "sum") != NULL) {
          if (sscanf(dummystr, "sum %d %d %f", &i, &j, &a) == 3) {
            markers[0] = i;
            markers[1] = j;
            background[0] = a;
            GetSum();
          }
        }
        /* --- single gauss fit --- */
        if ((strstr(dummystr, "gauss") != NULL) && (strstr(dummystr, "doublegauss") == NULL) &&
            (strstr(dummystr, "fixedFWHMgauss") == NULL)) {
          if (sscanf(dummystr, "gauss %d %d %f", &i, &j, &a) == 3) {
            markers[0] = i;
            markers[1] = j;
            background[0] = a;
            GaussFit();
          }
        }
        /* --- double gauss fit --- */
        if (strstr(dummystr, "doublegauss") != NULL) {
          if (sscanf(dummystr, "doublegauss %d %d %d %d %f %f %f", &i, &j, &k, &l, &a, &b, &c) ==
              7) {
            markers[0] = i;
            markers[1] = j;
            markers[2] = k;
            markers[3] = l;
            background[0] = c;
            DoubleGaussFit(a, b);
          }
        }
        /* --- fixed FWHM fit --- */
        if (strstr(dummystr, "fixedFWHMgauss") != NULL) {
          if (sscanf(dummystr, "fixedFWHMgauss %d %d %f %f", &i, &j, &a, &b) == 4) {
            markers[0] = i;
            markers[1] = j;
            background[0] = b;
            FixedWidthGaussFit(a);
          }
        }
        /* --- now for lines with just sorting --- */
        if (strstr(dummystr, "sort") != NULL) {
          if (strstr(dummystr, "sort clear") != NULL) {
            /* --- set all gate and output types to zero (false) --- */
            pgamsortgatenone = pgamsortgatetac = pgamsortgatepart = 0;
            pgamsortgategamma = pgamsortoutputhists = pgamsortoutputpart = 0;
            pgamsortoutputtac = pgamsortoutputgg = pgamsortoutputsqr = 0;
            pgamsortoutputggtype = 0;
            pgamsortgateparten = 0;
            pgamsortgategammatypefile = 0;
            pgamsortgategammatypeentry = 1;
            pgamsortgateveto = 0;
            pgamsortgaterequires = 0;
            selfsuppressclovers = 0;
            min_clover_multipolarity = 0;
          }
          /* --- setting up sort output options --- */
          if (strstr(dummystr, "sort output") != NULL) {
            if (strstr(dummystr, "tac") != NULL) {
              pgamsortoutputtac = 1;
              pgamsortoutputhists = 0;
            }
            if (strstr(dummystr, "hist") != NULL) {
              pgamsortoutputtac = 0;
              pgamsortoutputhists = 1;
            }
            if (strstr(dummystr, "part") != NULL) {
              pgamsortoutputpart = 1;
              if (1 == sscanf(dummystr, "sort output part %f", &a))
                telfincal = 1 / a;
              else
                telfincal = 1;
            }
            if (strstr(dummystr, "twd") != NULL) {
              pgamsortoutputgg = 1;
              pgamsortoutputggtype = 1;
              pgamsortoutputsqr = 0;
              pgamsortoutputpairsqr = 0;
            }
            if (strstr(dummystr, "ggsqr") != NULL) {
              pgamsortoutputgg = 1;
              pgamsortoutputggtype = 2;
              pgamsortoutputsqr = 0;
              pgamsortoutputpairsqr = 0;
              if (1 == sscanf(dummystr, "sort output ggsqr %s", dummystr2)) {
                PgamReadDetectorInfo(dummystr2);
              }
            }
            if (strstr(dummystr, "pgsqr") != NULL) {
              pgamsortoutputgg = 0;
              pgamsortoutputggtype = 0;
              pgamsortoutputsqr = 1;
              pgamsortoutputpairsqr = 0;
              if (1 == sscanf(dummystr, "sort output pgsqr %f", &a))
                partfincal = a;
              else
                partfincal = 1;
            }
            if (strstr(dummystr, "pairsqr") != NULL) {
              pgamsortoutputgg = 0;
              pgamsortoutputggtype = 0;
              pgamsortoutputsqr = 0;
              pgamsortoutputpairsqr = 1;
              if (1 == sscanf(dummystr, "sort output pairsqr %s", dummystr2)) {
                sprintf(pgampairfilename, dummystr2);
              }
            }
          }
          if (strstr(dummystr, "sort gate") != NULL) {
            if (strstr(dummystr, "tac") != NULL) {
              pgamsortgatetac = 1;
            }
            if (strstr(dummystr, "part") != NULL) {
              pgamsortgatepart = 1;
              if (1 == sscanf(dummystr, "sort gate part %s", dummystr2)) {
                ReadGates(dummystr);
              }
            }
            if (strstr(dummystr, "gamma") != NULL) {
              if (2 == sscanf(dummystr, "sort gate gamma %d %d", &i, &j)) {
                pgamgammagates.num = 1;
                if (pgamgammagates.min == NULL) {
                  pgamgammagates.min = (float *)malloc(sizeof(float) * pgamgammagates.num);
                } else {
                  pgamgammagates.min =
                      (float *)realloc(pgamgammagates.min, sizeof(float) * pgamgammagates.num);
                }
                if (pgamgammagates.max == NULL) {
                  pgamgammagates.max = (float *)malloc(sizeof(float) * pgamgammagates.num);
                } else {
                  pgamgammagates.max =
                      (float *)realloc(pgamgammagates.max, sizeof(float) * pgamgammagates.num);
                }
                pgamgammagates.min[0] = i;
                pgamgammagates.max[0] = j;
              } else {
                if (1 == sscanf(dummystr, "sort gate gamma %s", dummystr2)) {
                  PgamReadGammaGate(dummystr2);
                }
              }
            }
            if (strstr(dummystr, "veto") != NULL) {
              if (1 == sscanf(dummystr, "sort gate veto %s", dummystr2)) {
                vetos = PgamReadVeto(dummystr2);
                pgamsortgateveto = 1;
              }
            }
            if (strstr(dummystr, "requires") != NULL) {
              if (1 == sscanf(dummystr, "sort gate requires %s", dummystr2)) {
                requires = PgamReadVeto(dummystr2);
                pgamsortgaterequires = 1;
              }
            }
            if (strstr(dummystr, "selfsup") != NULL) {
              selfsuppressclovers = 1;
            }
            if (strstr(dummystr, "minmulti") != NULL) {
              if (1 == sscanf(dummystr, "sort gate minmultipolarity %d", &i)) {
                min_clover_multipolarity = i;
              }
            }
          }
          if (strstr(dummystr, "sort sort") != NULL) {
            if (2 == sscanf(dummystr, "sort sort %d %d", &i, &j)) {
              runmin = i;
              runmax = j;
              PgamSort();
            }
          }
        }
      } // end of checking it isn't a comment line.
    }
    fclose(infile);
  }
}

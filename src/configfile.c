/* configfile.c
 *
 * By John Pavan
 *
 * The handler for configuration files for gnuscope
 *
 */

/* --- includes --- */
#include "gnuscopefuncs.h"
#include "gnuscopeglobals.h"

/* --- globals --- */
/* --- should be taken care of buy including gnuscopeglobals.h --- */

/* --- function declarations --- */
/* --- should be taken care of by including gnuscopefuncs.h --- */

/* --- functions --- */

/* ConfigFileSearcher
 *
 * Searches for the config files
 */
void ConfigFileSearcher() {
  FILE *infile;
  char dummystr[120];
  char homedir[120];

  /* --- Jan 5, 2004 --- */
  /* --- modified to search for the main configuration file
     --- and then the local file --- */

  /* --- first look for the global configuraiton file --- */
  sprintf(dummystr, "%s/.gnuscopeconfig", getenv("HOME"));
  if ((infile = fopen(dummystr, "r")) != NULL) {
    ConfigFileHandler(infile);
    fclose(infile);
  }

  /* --- now look for the local configuraiton file --- */
  if ((infile = fopen(".gnuscopeconfig", "r")) != NULL) {
    ConfigFileHandler(infile);
    fclose(infile);
  }

  /* --- first look for a configuration file in the local directory --- */
  //  if ((infile = fopen(".gnuscopeconfig","r")) != NULL) {
  //  ConfigFileHandler(infile);
  //  fclose(infile);
  //} else {
  /* --- otherwise let us see if it is in the main directory --- */
  //  sprintf(dummystr,"%s/.gnuscopeconfig",getenv("HOME"));
  //  if ((infile = fopen(dummystr,"r")) != NULL) {
  //    ConfigFileHandler(infile);
  //    fclose(infile);
  //  }
  //}
}

/* ConfigFileHandler
 *
 * This function handles the interpretation of configuration files
 */
void ConfigFileHandler(FILE *infile) {
  //  FILE *infile;
  char dummystr[120];
  char dummystr2[120];
  glob_t globresults;
  int i, j;

  //  if ((infile = fopen(sFilename,"r")) != NULL) {
  while (fgets(dummystr, 120, infile) != NULL) {
    if (strncmp(dummystr, "#", 1) != 0) {
      /* --- we are not on a comment line --- */
      /* --- let's figure out what is being set --- */
      if (strncmp(dummystr, "in", 2) == 0) {
        /* --- this is the case where the user is trying to read in a file --- */
        if (sscanf(dummystr, "in %s", dummystr2) == 1) {
          printf("About to Read %s\n", dummystr2);
          AutoDetectFileType(dummystr2);
        } else {
          WriteMainText("Error determining which file to read.\n");
        }
      }

      if (strncmp(dummystr, "Set2DPoly", 9) == 0) {
        /* --- the user is trying to set the default polynomial degree for
           --- the 2D interpolation --- */
        if (sscanf(dummystr, "Set2DPoly %d", &i) == 1) {
          interpolationmode = i;
          printf("Set 2D interpolation Mode to %d.\n", i);
        } else {
          WriteMainText("Error setting polynomial degree for 2D interpolation.\n");
        }
      }
      if (strncmp(dummystr, "Set2DColorDepth", 9) == 0) {
        /* --- the user is trying to set the default polynomial degree for
           --- the 2D interpolation --- */
        if (sscanf(dummystr, "Set2DColorDepth %d", &i) == 1) {
          colordepth = i;
          printf("Set the number of colors for density plots to %d.\n", i);
        } else {
          WriteMainText("Error setting the number of colors for density plots.\n");
        }
      }
      if (strncmp(dummystr, "OverlayYScaleMode", 17) == 0) {
        if (sscanf(dummystr, "OverlayYScaleMode %d", &i) == 1) {
          for (j = 0; j < 16; j++) {
            gtk_graph_set_y_scale_mode(GTK_GRAPH(graphsdisplayed[j]), i);
          }
        } else {
          WriteMainText("Error setting the y scale mode for overlayed plots.\n");
        }
      }
      if (strncmp(dummystr, "DensityPlotColors Color", 23) == 0) {
        pgamcolormode = 0;
      }
      if (strncmp(dummystr, "DensityPlotColors BW", 20) == 0) {
        pgamcolormode = 1;
      }
      /* --- DensityPlotType --- */
      if (strncmp(dummystr, "DensityPlotType DensityAndContour", 30) == 0) {
        pgamtype = 2;
      } else if (strncmp(dummystr, "DensityPlotType Density", 23) == 0) {
        pgamtype = 0;
      }
      if (strncmp(dummystr, "DensityPlotType Contour", 23) == 0) {
        pgamtype = 1;
      }
      /* --- ContourMode --- */
      if (strncmp(dummystr, "ContourMode Linear", 19) == 0) {
        pgamcontourtype = 0;
      }
      if (strncmp(dummystr, "ContourMode Log", 15) == 0) {
        pgamcontourtype = 1;
      }
      if (strncmp(dummystr, "ContourMode Root", 16) == 0) {
        pgamcontourtype = 2;
      }
      if (strncmp(dummystr, "ContourMode InverseLog", 23) == 0) {
        pgamcontourtype = 3;
      }
      if (strncmp(dummystr, "ContourMode InverseRoot", 23) == 0) {
        pgamcontourtype = 4;
      }
      /* --- NumContours --- */
      if (strncmp(dummystr, "NumContours", 12) == 0) {
        if (sscanf(dummystr, "NumContours %d", &i) == 1) {
          twodnumcontours = 1;
        }
      }
      if (strncmp(dummystr, "LineColor", 9) == 0) {
        if (sscanf(dummystr, "LineColor %d %s", &i, &dummystr2) == 2) {
          i--;
          if ((i >= 0) && (i < 10)) {
            snprintf(colorstring[i], sizeof(colorstring[i]), "%s", dummystr2);
          }
        }
      }
      /* --- DensityPlotInterpolationMode --- */
      if (strncmp(dummystr, "DensityPlotInterpolationMode", 29) == 0) {
        if (strncmp(dummystr, "DensityPlotInterpolationMode Off", 33) == 0) {
          interpolationmode = 0;
        } else {
          if (sscanf(dummystr, "DensityPlotInterpolationMode %d", &i) == 1) {
            interpolationmode = 0;
          }
        }
      }
      /* --- DisplayMode --- */
      if (strncmp(dummystr, "DisplayMode Linear", 19) == 0)
        plottype = 1;
      if (strncmp(dummystr, "DisplayMode Log", 16) == 0)
        plottype = 2;
      if (strncmp(dummystr, "DisplayMode Root", 17) == 0)
        plottype = 3;
      if (strncmp(dummystr, "DisplayMode EffCor", 19) == 0)
        plottype = 4;

      /* --- help path --- */
      if (strncmp(dummystr, "ManualPath", 10) == 0) {
        sscanf(dummystr, "ManualPath %s", manualpath);
        //	printf("Path for gnuscopemanual set to %s\n",manualpath);
      } // done with ManualPath
      if (strncmp(dummystr, "PDFApp", 6) == 0) {
        sscanf(dummystr, "PDFAapp %s", pdfapp);
      }
    } // done scrolling through the file
  }
  // fclose(infile);
  // }
}

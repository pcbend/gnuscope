/* peakfit.c
 *
 * by John Pavan
 *
 * For finding and identifying peaks in a histogram
 */

/* --- includes --- */

#include <gtk/gtk.h>
#include "gtkgraph.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "ecallib.h"

/* --- structs --- */

struct floatpoint {
  float x;
  float y;
};

struct peak_records {
  int num; // number of peaks we have information about
  float *centers;
  float *deviations;
  float *areas;
};

struct calibration_info {
  int num;
  float *energies;
  float *intensities;
};

struct my_list_struct {
  int num;
  double *list;
};

struct my_gauss_struct {
  double center;
  double deviation;
  double area;
  double chisqr;
};

/* --- globals --- */

int abortflag;
int histid[254], histsize[254], *histloc[254], histpointer;
char field1[254][40], field2[254][40], field3[254][40];
int intype, spectra, plottype;
GtkWidget *graph;
int markers[4], binsize, currentrange[2];
int background;
int backgroundploydeg;
int currentmax;
float globalcalibration[3], calibration[254][3];
int globalcalibrationset;
int ybinforce, yscaleforce;
float ybin, yscale;
long double displayused;
int numspectra;
GtkWidget *box1;
float efficiencycal[4];
int lastfitnumpoints;
int lastspectraclicked;
GtkWidget *lastgraphclicked;
struct floatpoint *lastfitpoints;
struct peak_records peakrecords;
int num_peaks;
int *peak_locs;
struct calibration_info calibrationinfo;

/* --- Function Declarations --- */

void FindPeaks(float minwidth, float maxwidth);
void PruneSmallPeaks();
float GaussChiSqr(int a, int b, float center, float deviation, float *area);
void MultipleGaussFit(int minwidth, int maxwidth);
void FindPeaksEntry(GtkWidget *widget, GtkWidget *entry);
void FindPeaksPrompt(GtkWidget *text);
void PeakRecordClear();
void PeakRecordAdd(float center, float deviation, float area);
float PeakRecordRetrieveCenter(int index);
float PeakRecordRetrieveDeviation(int index);
float PeakRecordRetrieveArea(int index);
void ModifiedDisplayGaussian(float center, float deviation, float area, int min_chan, int max_chan);
float CalibsRetrieveIntensity(int index);
float CalibsRetrieveEnergy(int index);
void CalibsClear();
void CalibAdd(float energy, float intensity);
void ReadCalibrationFile(char *sFilename);
void ListStructClear(struct my_list_struct *stru);
double ListStructRetrieve(struct my_list_struct *stru, int index);
void ListStructDestroy(struct my_list_struct *stru);
struct my_list_struct *ListStructNew();
void ListStructAdd(struct my_list_struct *stru, double element);
void ReturnCloseHits(double a, double b, double c, struct my_list_struct *centers,
                     struct my_list_struct *energies, double cruddiness);
void AutoCal(int width);
int IntComp(int a, int b);
struct my_gauss_struct *ModifiedSingleGauss(int lchan, int uchan);
int compare(const void *e1, const void *e2);
double DMin(double a, double b);
double DMax(double a, double b);

/* --- functions --- */

/* PeakRecordClear
 *
 * Clears the information in peakrecords
 */
void PeakRecordClear() {
  if (peakrecords.num > 0) {
    free(peakrecords.centers);
    free(peakrecords.deviations);
    free(peakrecords.areas);
    peakrecords.centers = NULL;
    peakrecords.deviations = NULL;
    peakrecords.areas = NULL;
    peakrecords.num = 0;
  }
}

/* PeakRecordAdd
 *
 * Adds a peak to the peak records structure
 */
void PeakRecordAdd(float center, float deviation, float area) {
  int index;

  index = peakrecords.num;
  if (peakrecords.num == 0) {
    peakrecords.centers = (float *)malloc(sizeof(float));
    peakrecords.deviations = (float *)malloc(sizeof(float));
    peakrecords.areas = (float *)malloc(sizeof(float));
    peakrecords.num = 1;
  } else {
    peakrecords.num = peakrecords.num + 1;
    peakrecords.centers = (float *)realloc(peakrecords.centers, sizeof(float) * peakrecords.num);
    peakrecords.deviations =
        (float *)realloc(peakrecords.deviations, sizeof(float) * peakrecords.num);
    peakrecords.areas = (float *)realloc(peakrecords.areas, sizeof(float) * peakrecords.num);
  }
  peakrecords.centers[index] = center;
  peakrecords.deviations[index] = deviation;
  peakrecords.areas[index] = area;
}

/* PeakRecordRetrieveCenter,deviation,area
 *
 * Returns the value of the center, deviation, and area in
 * the variables past to it.
 */
float PeakRecordRetrieveCenter(int index) {
  if ((index >= 0) && (index < peakrecords.num)) {
    return (peakrecords.centers[index]);
  }
}
float PeakRecordRetrieveDeviation(int index) {
  if ((index >= 0) && (index < peakrecords.num)) {
    return (peakrecords.deviations[index]);
  }
}
float PeakRecordRetrieveArea(int index) {
  if ((index >= 0) && (index < peakrecords.num)) {
    return (peakrecords.areas[index]);
  }
}

/* FindPeaksPrompt
 *
 * Text for the dialog box for find peaks
 */
void FindPeaksPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Please enter the approximate peak widths (in channels)");
  gtk_text_insert(GTK_TEXT(text), NULL, NULL, NULL, dummystr, strnlen(&dummystr, 80));
  sprintf(dummystr, "for channel 1 and channel %d.\n", histsize[spectra]);
  gtk_text_insert(GTK_TEXT(text), NULL, NULL, NULL, dummystr, strnlen(&dummystr, 80));
}

/* FindPeaksEntry
 *
 * Gets the minwidth and maxwidth from the dialog entry
 * Calls the peak finding stuff
 */
void FindPeaksEntry(GtkWidget *widget, GtkWidget *entry) {
  int test, c;
  float a, b;
  float minwidth, maxwidth;
  char dummystr[80];

  num_peaks = 0;
  peak_locs = NULL;

  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%f %f", &a, &b);
  if ((test == 2) && (b >= a)) {
    minwidth = a / 1.665;
    maxwidth = b / 1.665;
    WriteMainText("Attempting Peak Fit.\n");
    FindPeaks(minwidth, maxwidth);
    sprintf(dummystr, "Found %d peaks\n", num_peaks);
    WriteMainText(dummystr);
    if (num_peaks) {
      MultipleGaussFit(minwidth, maxwidth);
    }
  }
}

/* FindPeaks
 *
 * Finds the centroids of the peaks
 */
void FindPeaks(float minwidth, float maxwidth) {
  int Left_channel, Right_channel;
  float Chi1, Chi;
  int Channel;
  float Width0;
  float area;
  char dummystr[80];
  int i, j, k;
  int improving;

  Chi1 = 1000000000;
  improving = 0;

  for (Channel = 0; Channel < histsize[spectra]; Channel++) {
    Width0 = minwidth + Channel * (maxwidth - minwidth) / histsize[spectra];
    Left_channel = (Channel - 2 * Width0);
    Right_channel = (Channel + 2 * Width0);

    if ((Left_channel >= 0) && (Right_channel <= histsize[spectra]) && (Width0 > 0)) {

      /* --- set the background to the lowest channel between the left and right --- */
      background = *(histloc[spectra] + Left_channel);
      for (i = Left_channel + 1; i <= Right_channel; i++) {
        if (*(histloc[spectra] + i) < background)
          background = *(histloc[spectra] + i);
      }
      background--;

      Chi = GaussChiSqr(Left_channel, Right_channel, Channel, Width0, &area);
      if (Chi != -1) {
        Chi = Chi / area;

        if (Chi < Chi1) {
          improving = 1;
          Chi1 = Chi;
        } else {
          if (improving) {
            improving = 0;
            num_peaks++;
            if (peak_locs == NULL) {
              peak_locs = (int *)malloc(sizeof(int) * num_peaks);
            } else {
              peak_locs = (int *)realloc(peak_locs, sizeof(int) * num_peaks);
            }
            peak_locs[num_peaks - 1] = Channel;
            // Channel = Channel + Width0;
            printf("peak at %d.\n", Channel);
            Chi1 = Chi;
          }
        }
      }
    }
  }

  sprintf(dummystr, "peaks found %d.\n", num_peaks);
  WriteMainText(dummystr);
  printf(dummystr);
}

/* MultipleGaussFit
 *
 * Performs a single gauss fit for all
 * centers in the GSList centers, returning
 * the actually centers, FWHM, and Area
 */
void MultipleGaussFit(int minwidth, int maxwidth) {
  int width, left_chan, right_chan;
  int channel;
  float bestfit[4];
  float chiolddeviation;
  int flagw;
  float center, deviation;
  int exitflag;
  float deldeviation;
  float chiold, delx;
  float chisqr, area;
  float deldeviationold;
  float centerenergy;
  float deviationenergy;
  char dummystr[80];
  float delxold;
  float correctedcounts;
  int i, j;
  int count;

  exitflag = 0;
  abortflag = 0;

  PeakRecordClear();

  for (i = 1; (i < num_peaks); i++) {
    printf("%d\n", i);
    channel = peak_locs[i];
    width = minwidth - channel * (maxwidth - minwidth) / histsize[spectra];
    left_chan = channel - width * 3;
    right_chan = channel + width * 3;
    /* --- set the background to the lowest channel between the left and right --- */
    background = *(histloc[spectra] + left_chan);
    for (j = left_chan + 1; j <= right_chan; j++) {
      if (*(histloc[spectra] + j) < background)
        background = *(histloc[spectra] + j);
    }
    bestfit[0] = 1000000;
    chiolddeviation = 10000000;
    deldeviation = -0.05;
    flagw = 0;
    center = channel;
    deviation = width;
    /* --- let's see if I can write this without the goto statements --- */
    exitflag = 0;
    count = 0;
    while ((exitflag != 1) && ((deldeviation > 0.0001) || ((-deldeviation) > 0.0001))) {
      if (count > 1000) {
        GetMessageDialog("Peak Width too narrow.\n");
        goto quitprocess;
      }
      count++;
      if (deviation > width * 3)
        goto peakhasproblems;
      chiold = 1000000;
      delx = 0.1;
      chisqr = GaussChiSqr(left_chan, right_chan, center, deviation, &area);
      if (chisqr == -1)
        exitflag = 1;
      if (chisqr <= chiolddeviation) {
        chiolddeviation = chisqr;
        deviation = deviation + deldeviation;
      } else {
        deldeviationold = deldeviation;
        deldeviation = -deldeviation / 5;
        chiolddeviation = chisqr;
        deviation = deviation + deldeviation;
      }
      deviation = deviation - deldeviationold;
      while ((delx > 0.0009) || (-delx > 0.0009)) {
        /* --- in case the window hasn't been refreshed recently --- */
        while (gtk_events_pending()) {
          gtk_main_iteration();
        }
        if (abortflag)
          goto quitprocess;
        chisqr = GaussChiSqr(left_chan, right_chan, center, deviation, &area);
        if (chisqr == -1)
          exitflag = 1;
        if (chisqr <= chiold) {
          chiold = chisqr;
          center = center + delx;
        } else {
          delxold = delx;
          delx = -delx / 5;
          chiold = chisqr;
          center = center + delx;
        }
      }
      center = center - delxold;
      //    printf("Center: %.1f FWHM: %.1f, Area: %.1f Chisqr: %.1f \n",
      //           center,(deviation*1.665),(area*deviation *1.665),chisqr);
      if (chisqr < bestfit[0]) {
        bestfit[0] = chisqr;
        bestfit[1] = center;
        bestfit[2] = deviation;
        bestfit[3] = area;
      }
    }
    sprintf(dummystr, "Center: %.1f FWHM: %.1f, Area %.1f Chisqr:%.1f\n", bestfit[1] + 1,
            bestfit[2] * 1.665, bestfit[3] * bestfit[2] * 1.665, bestfit[0]);
    WriteMainText(dummystr);
    printf("%d to %d ", left_chan + 1, right_chan + 1);
    printf(dummystr);

    PeakRecordAdd(bestfit[1] + 1, bestfit[2], bestfit[3]);
    ModifiedDisplayGaussian(bestfit[1], bestfit[2], bestfit[3], left_chan, right_chan);
  peakhasproblems:
    abortflag = 0;
  }
  PruneSmallPeaks();
  AutoCal(maxwidth);
  printf("%d peaks left\n", peakrecords.num);
quitprocess:
  abortflag = 0;
}

/* ModifiedDisplayGaussian
 *
 * Displays multiple gaussian fits
 * accepting GSLists for the centers, deviations, and areas
 */
void ModifiedDisplayGaussian(float center, float deviation, float area, int min_chan,
                             int max_chan) {
  int i, j, k;
  int a, b;
  float stepsize;
  GdkPoint points;
  int max = 0;
  int bar_height, next_bar_height;
  float width, height;
  float column_width;
  float tempsum, tempavg;
  int localmarkers[2];
  long double rangewidth, currentscaling;
  float xchan, nextxchan;

  if (lastgraphclicked != NULL) {
    graph = lastgraphclicked;
    spectra = lastspectraclicked;
  }

  /* --- in case the window hasn't been refreshed recently --- */
  while (gtk_events_pending()) {
    gtk_main_iteration();
  }

  localmarkers[0] = min_chan + 1;
  localmarkers[1] = max_chan - 1;

  /* --- Get height and width --- */
  height = graph->allocation.height - 31;

  if (binsize > 0) {

    /* --- find the max value --- */
    /* --- The max value can be modivied by the user
       --- using yscaleforce (yscale) and ybinforce(ybin)
    */
    tempsum = 0;
    tempavg = 0;
    if (yscaleforce == 1) {
      max = yscale;
    } else {
      for (i = currentrange[0]; i <= currentrange[1]; i = i + binsize) {
        tempsum = 0;
        tempavg = 0;
        for (j = 0; j < binsize; j++) {
          tempsum = tempsum + *(histloc[spectra] + i + j);
        }
        tempavg = tempsum / binsize;
        if (max < tempsum)
          max = tempsum;
      }
    }
  }
  if (ybinforce == 1) {
    if (ybin > 0) {
      max = (int)(max * ybin);
    } else {
      if (ybin < 0) {
        max = (int)(max / (-ybin));
      } else {
        if (ybin == 0) {
          ybinforce = 0;
        }
      }
    }
  }
  max = max * 1.1;
  if (max < 10)
    max = 10;

  currentmax = max;
  rangewidth = currentrange[1] - currentrange[0];
  if (rangewidth != 0) {
    currentscaling = (long double)((graph->allocation.width - 100) / rangewidth);
  }
  //  printf("Rangewidth: %d Currentscaling : %ef ",rangewidth,currentscaling);

  /* --- Display each value --- */
  /* --- do this by figuring out which pixels on the screen are between the localmarkers[0-1]
     --- and then plot the gaussian between them. --- */
  if (binsize > 1) {
    a = (localmarkers[0] - currentrange[0]) * displayused + 100;
    b = (localmarkers[1] - currentrange[0]) * displayused + 100;
  } else {
    a = (localmarkers[0] - currentrange[0] + 0.5) * currentscaling + 100;
    b = (localmarkers[1] - currentrange[0] + 0.5) * currentscaling + 100;
  }

  for (i = a; i <= b; i++) {
    /* --- now we have to convert back to get the right channel numbers --- */
    if (binsize > 1) {
      xchan = (i - 100) / displayused + (float)currentrange[0];
      nextxchan = (i - 99) / displayused + (float)currentrange[0];
    } else {
      if ((binsize == 1) && (currentscaling != 0)) {
        xchan = (i - (100)) / currentscaling + currentrange[0] - .5;
        nextxchan = (i - 1 - (100)) / currentscaling + currentrange[0] - .5;
      }
    }
    // printf("Xchan: %f ",xchan);

    bar_height =
        ((area * exp(-(xchan - center) * (xchan - center) / deviation / deviation)) + background) *
        binsize;
    LastFitAddPoint((float)xchan, (float)bar_height);
    if (binsize > 1) {
      points.x = i;
    } else {
      points.x = (i);
    }
    if (plottype == 2) {
      bar_height = (int)1000 * log((double)bar_height);
    }
    if (plottype == 3) {
      bar_height = (int)sqrt((double)bar_height);
    }
    points.y = height - bar_height * height / max;
    gtk_graph_add_segment(GTK_GRAPH(graph), (GdkPoint *)&points);
    //  printf("Points: %d %d ",points[0].x,points[0].y);
  }
  Redraw();
}

/* PruneSmallPeaks
 *
 * Goes through the peaks which were returned and throws out all peaks
 * which are less than 1/10 the size of the largest peak found
 */
void PruneSmallPeaks() {
  int num_big_peaks;
  float *big_peak_centers;
  float *big_peak_deviations;
  float *big_peak_areas;
  int i, j, k, l;
  float largest_area, temp, ltemp;
  float lastsize;
  float ctmp, atmp, dtmp;
  int bigones[15];
  int smallestone;

  num_big_peaks = 0;
  big_peak_centers = (float *)malloc(sizeof(float) * peakrecords.num);
  big_peak_deviations = (float *)malloc(sizeof(float) * peakrecords.num);
  big_peak_areas = (float *)malloc(sizeof(float) * peakrecords.num);
  for (i = 0; i < 15; i++)
    bigones[i] = 0;

  /* --- find the largest area --- */

  largest_area = 0;
  for (i = 0; i < peakrecords.num; i++) {
    temp = PeakRecordRetrieveDeviation(i) * PeakRecordRetrieveArea(i);
    if (temp > largest_area) {
      largest_area = temp;
      j = i;
    }
  }

  // largest_area = largest_area;
  big_peak_centers[num_big_peaks] = PeakRecordRetrieveCenter(j);
  big_peak_deviations[num_big_peaks] = PeakRecordRetrieveDeviation(j);
  big_peak_areas[num_big_peaks] = PeakRecordRetrieveArea(j);
  num_big_peaks = 1;

  /* --- order peaks by area --- */
  /* --- make a list of biggest areas --- */

  bigones[0] = j;
  lastsize = largest_area;
  ltemp = 0;
  for (j = 1; ((j < 15) && (j < peakrecords.num)); j++) {
    for (i = 0; i < peakrecords.num; i++) {
      temp = PeakRecordRetrieveDeviation(i) * PeakRecordRetrieveArea(i);
      if ((temp > ltemp) && (temp < lastsize)) {
        k = i;
        ltemp = temp;
      }
    }
    lastsize = ltemp;
    ltemp = 0;
    bigones[j] = k;
    num_big_peaks++;
  }

  num_big_peaks -= 1;

  qsort(bigones, num_big_peaks, sizeof(int), compare);

  for (i = 0; i < num_big_peaks; i++) {
    big_peak_centers[i] = PeakRecordRetrieveCenter(bigones[i]);
    big_peak_deviations[i] = PeakRecordRetrieveDeviation(bigones[i]);
    big_peak_areas[i] = PeakRecordRetrieveArea(bigones[i]);
  }

  PeakRecordClear();
  for (i = 0; i < num_big_peaks; i++) {
    PeakRecordAdd(big_peak_centers[i], big_peak_deviations[i], big_peak_areas[i]);
    printf("Center: %.1f FWHM: %.1f Area:%.1f\n", PeakRecordRetrieveCenter(i),
           PeakRecordRetrieveDeviation(i) * 1.665,
           PeakRecordRetrieveDeviation(i) * 1.665 * PeakRecordRetrieveArea(i));
  }

  // GetFilename("CalibrationFile",ReadCalibrationFile);

  g_free(big_peak_centers);
  g_free(big_peak_deviations);
  g_free(big_peak_areas);
}

/* ReadCalibrationFile
 *
 * Reads in a calibration file.  The file should be
 * a list of <energy>,<intensity>
 */
void ReadCalibrationFile(char *sFilename) {
  FILE *infile;
  int test;
  float a, b;
  char dummystr[80];
  int i;

  CalibsClear();

  if ((infile = fopen(sFilename, "r")) != NULL) {
    while ((test = fscanf(infile, "%f %f", &a, &b)) != EOF) {
      if (test == 2) {
        CalibAdd(a, b);
      }
    }
  }
  for (i = 0; i < calibrationinfo.num; i++) {
    sprintf(dummystr, "%f\n", calibrationinfo.energies[i]);
    WriteMainText(dummystr);
  }
}

/* CalibAdd
 *
 * Add a value to the list of calibrations
 */
void CalibAdd(float energy, float intensity) {
  if (calibrationinfo.num) {
    calibrationinfo.energies =
        (float *)realloc(calibrationinfo.energies, sizeof(float) * calibrationinfo.num + 1);
    calibrationinfo.intensities =
        (float *)realloc(calibrationinfo.intensities, sizeof(float) * calibrationinfo.num + 1);
  } else {
    calibrationinfo.energies = (float *)malloc(sizeof(float));
    calibrationinfo.intensities = (float *)malloc(sizeof(float));
  }
  calibrationinfo.energies[calibrationinfo.num] = energy;
  calibrationinfo.intensities[calibrationinfo.num] = intensity;
  calibrationinfo.num++;
}

/* CalibsClear
 *
 * Clears the calibration info
 */
void CalibsClear() {
  calibrationinfo.num = 0;
  if (calibrationinfo.energies)
    free(calibrationinfo.energies);
  if (calibrationinfo.intensities)
    free(calibrationinfo.intensities);
}

/* CalibsRetrieveEnergy
 *
 * Returns the energy of the calibration given by index.
 */
float CalibsRetrieveEnergy(int index) {
  if ((index >= 0) && (index < calibrationinfo.num)) {
    return (calibrationinfo.energies[index]);
  } else
    return (-1);
}

/* CalibsRetrieveIntensity
 *
 * Returns the intensities of the calibration given by index.
 */
float CalibsRetrieveIntensity(int index) {
  if ((index >= 0) && (index < calibrationinfo.num)) {
    return (calibrationinfo.intensities[index]);
  } else
    return (-1);
}

/* AutoCal
 *
 * Autocalibration via brute force
 * Does nothing if only 2 peaks were found
 * Fits linear if only 3 points were found
 * Fits Quadratic if 4 or more peaks were found
 * Brute force method:
 * Try to corrolate all calibration points with found peaks
 * Then try removing 1 calibration point at a time and
 * trying again.  Stop when Chisqr goes less than 1.
 */
void AutoCal(int width) {
  double d, b, c;
  struct my_list_struct *clist;
  struct my_list_struct *elist;
  int i, j, k;
  double dela, delb, delc;
  double *dummy, a[3];
  double chisqr;
  char dummystr[120];
  struct my_gauss_struct *stru;
  int temp, f;
  int oldnum;

  clist = ListStructNew();
  elist = ListStructNew();

  f = 0;
  d = c = 0;
  b = 0;
  dela = 1;
  delb = 0.005;
  delc = 0.00000001;

  a[0] = 0;
  a[1] = 0;
  a[2] = 0;

  for (k = 0; k < 100; k++) {
    c = c + delc;
    d = 0;
    for (j = 0; j < 100; j++) {
      while (gtk_events_pending()) {
        gtk_main_iteration();
      }
      if (abortflag)
        goto quitprocess;

      d = d + dela;
      b = 0;
      for (i = 0; i < 200; i++) {
        b = b + delb;
        //	printf("b = %f.\n",b);
        ReturnCloseHits(d, b, c, clist, elist, 20);
        if (clist->num > 4) {
          PolFit(3, clist->list, elist->list, dummy, clist->num, 0, a);
          oldnum = clist->num - 1;
          while (clist->num > oldnum) {
            ListStructClear(clist);
            ListStructClear(elist);
            ReturnCloseHits(a[0], a[1], a[2], clist, elist, 50);
            PolFit(3, clist->list, elist->list, dummy, clist->num, 0, a);
            PolChiSqr(a, 3, clist->list, elist->list, clist->num, &chisqr);
            oldnum = clist->num;
            printf("chisqr = %f\n", chisqr);
            if (chisqr <= 10)
              goto stop;
          }
        }
        ListStructClear(clist);
        ListStructClear(elist);
      }
    }
  }
  f = 1;
  goto failure;
stop:
  printf("%f %f %f\n", a[0], a[1], a[2]);
  /* --- at this point we are pretty sure that we have a resonable calibration --- */
  /* --- however, we need to see if the peaks that were not found initially are
     --- actually there --- */

  // ListStructClear(clist);
  // ListStructClear(elist);
  // for (i = 0; i < calibrationinfo.num; i++) {
  //   temp = (- a[1] + sqrt(pow(a[1],2) - 4 * (a[0] - calibrationinfo.energies[i]) * a[2])) /
  //     (2 * a[2]);
  //   stru = ModifiedSingleGauss((temp-width),(temp+width));
  //   if (stru) {
  //     ListStructAdd(clist,stru->center);
  //     ListStructAdd(elist,calibrationinfo.energies[i]);
  //     g_free(stru);
  //  }
  // }
  // PolFit(3,clist->list,elist->list,dummy,clist->num,0,a);
  // PolChiSqr(a,3,clist->list,elist->list,clist->num,&chisqr);

failure:

  sprintf(dummystr, "y = %e + %e * x + %e * x^2 chisqr = %e.\n", a[0], a[1], a[2], chisqr);
  WriteMainText(dummystr);
  for (i = 0; i < clist->num; i++) {
    sprintf(dummystr, " %f %f\n", clist->list[i], elist->list[i]);
    WriteMainText(dummystr);
  }

quitprocess:

  ListStructDestroy(clist);
  ListStructDestroy(elist);
}

/* ReturnCloseHits
 *
 * Inserts the energies and centers into the appropriate
 * lists if the centers nearly line up with the energies
 * via an approximate calibration
 */
void ReturnCloseHits(double a, double b, double c, struct my_list_struct *centers,
                     struct my_list_struct *energies, double cruddiness) {
  int i, j, k;
  double temp, temp2, temp3;
  int step, point;

  for (i = 0; i < calibrationinfo.num; i++) {
    /* --- let's get the approximate channel for each energy --- */
    temp = (-b + sqrt(pow(b, 2) - 4 * (a - calibrationinfo.energies[i]) * c)) / (2 * c);
    for (j = 0; j < (peakrecords.num - 1); j++) {
      if ((peakrecords.centers[j] < temp) && (peakrecords.centers[j + 1] > temp)) {
        temp2 = fabs(temp - peakrecords.centers[j]);
        temp3 = fabs(temp - peakrecords.centers[j + 1]);
        if (temp3 < temp2) {
          temp2 = temp3;
          j++;
        }
        if (temp2 <= cruddiness) {
          ListStructAdd(centers, peakrecords.centers[j]);
          ListStructAdd(energies, calibrationinfo.energies[i]);
        }
      } else {
        if ((j + 2) == peakrecords.num) {
          temp2 = fabs(temp - peakrecords.centers[j + 1]);
          if (temp2 < cruddiness) {
            ListStructAdd(centers, peakrecords.centers[j + 1]);
            ListStructAdd(energies, calibrationinfo.energies[i]);
          }
        }
      }
    }
    // printf("checking %f.\n",calibrationinfo.energies[i]);
    /* --- now let's find the closest peak to that channel --- */
    // step = peakrecords.num / 2;
    // point = step;
    // while (step > 1) {
    //   if (point >= (peakrecords.num - 1)) point = peakrecords.num - 2;
    //   if ((peakrecords.centers[point] < temp) &&
    //	  (peakrecords.centers[point + 1] > temp)) {
    /* --- this is the case where we are pretty much at the right point --- */
    /* --- we now need to check which is closer to the energy-channel --- */
    //	temp2 = fabs(temp - peakrecords.centers[point]);
    //	temp3 = fabs(temp - peakrecords.centers[point + 1]);
    //	if (temp3 < temp2) {
    //	  point++;
    //	  temp2 = temp3;
    //	}
    /* --- now we need to see if either one is sufficiently non-cruddy --- */
    //	if (temp2 <= cruddiness) {
    /* --- this means we were sufficiently good --- */
    //  ListStructAdd(centers,peakrecords.centers[point]);
    //  ListStructAdd(energies,calibrationinfo.energies[i]);
    //}
    // step = 1;
    //} else {
    ///* --- have to do something else if we arn't in the right place --- */
    // if (peakrecords.centers[point] < temp) point += step;
    // else point -= step;
    // step = step / 2;
    // }

    //}
  }

  if (centers->num > 3) {
    for (i = 0; i < centers->num; i++) {
      printf("%f %f\n", centers->list[i], energies->list[i]);
    }
    if (centers->num)
      printf("\n");
  }
}

/* ListStructNew
 *
 * creates a my_list_struct
 */
struct my_list_struct *ListStructNew() {
  struct my_list_struct *ret;

  ret = (struct my_list_struct *)malloc(sizeof(struct my_list_struct));
  ret->num = 0;
  ret->list = NULL;
}

/* ListStructAdd
 *
 * Adds an element to my_list_struct
 */
void ListStructAdd(struct my_list_struct *stru, double element) {
  stru->num++;
  stru->list = (double *)g_realloc(stru->list, sizeof(double) * stru->num);
  stru->list[stru->num - 1] = element;
}

/* ListStructDestroy
 *
 * Destroys a my_list_struct
 */
void ListStructDestroy(struct my_list_struct *stru) {
  g_free(stru->list);
  g_free(stru);
}

/* ListStructRetrieve
 *
 * Retreives element[index] from the list struct
 */
double ListStructRetrieve(struct my_list_struct *stru, int index) {
  if ((index >= 0) && (index < stru->num))
    return (stru->list[index]);
  else
    return (-1);
}

/* ListStructClear
 *
 * resets a my_list_struct
 */
void ListStructClear(struct my_list_struct *stru) {
  stru->num = 0;
  g_free(stru->list);
  stru->list = NULL;
}

/* ModifiedSingleGauss
 *
 * Performs a single gauss fit given returning
 * a my_gauss_struct pointer
 * Requires a range to fit
 */
struct my_gauss_struct *ModifiedSingleGauss(int lchan, int uchan) {
  float deviation, chiolddeviation, deldeviation, flagdeviation, chiold, delx;
  float delxold, deldeviationold;
  float center;
  int i, j, k;
  int fixwflg, flagw;
  float chisqr, area, centerenergy, deviationenergy, correctedcounts;
  int exitflag;
  float bestfit[4];
  char dummystr[80];
  struct my_gauss_struct *stru;
  int count, max_count;

  count = 0;
  max_count = 1000;

  stru = (struct my_gauss_struct *)malloc(sizeof(struct my_gauss_struct));

  bestfit[0] = 1000000;
  chiolddeviation = 10000000;
  deldeviation = -0.05;
  flagw = 0;
  center = GetSumCenter(lchan, uchan, -1);
  deviation = GetSumDeviation(lchan, uchan, -1, center);
  /* --- let's see if I can write this without the goto statements --- */
  exitflag = 0;
  while ((exitflag != 1) && ((deldeviation > 0.0001) || ((-deldeviation) > 0.0001))) {
    count++;
    if (count > max_count)
      goto msgabort;
    chiold = 1000000;
    delx = 0.1;
    chisqr = GaussChiSqr(lchan, uchan, center, deviation, &area);
    if (chisqr == -1)
      exitflag = 1;
    if (chisqr <= chiolddeviation) {
      chiolddeviation = chisqr;
      deviation = deviation + deldeviation;
    } else {
      deldeviationold = deldeviation;
      deldeviation = -deldeviation / 5;
      chiolddeviation = chisqr;
      deviation = deviation + deldeviation;
    }
    deviation = deviation - deldeviationold;
    while ((delx > 0.0009) || (-delx > 0.0009)) {
      count++;
      if (count > max_count)
        goto msgabort;
      chisqr = GaussChiSqr(lchan, uchan, center, deviation, &area);
      if (chisqr == -1)
        exitflag = 1;
      if (chisqr <= chiold) {
        chiold = chisqr;
        center = center + delx;
      } else {
        delxold = delx;
        delx = -delx / 5;
        chiold = chisqr;
        center = center + delx;
      }
    }
    center = center - delxold;
    //    printf("Center: %.1f FWHM: %.1f, Area: %.1f Chisqr: %.1f \n",
    //           center,(deviation*1.665),(area*deviation *1.665),chisqr);
    if (chisqr < bestfit[0]) {
      bestfit[0] = chisqr;
      bestfit[1] = center;
      bestfit[2] = deviation;
      bestfit[3] = area;
    }
  }
  stru->center = bestfit[1];
  stru->deviation = bestfit[2];
  stru->area = bestfit[3];
  stru->chisqr = bestfit[0];

  return (stru);
msgabort:
  g_free(stru);
  stru = NULL;
  return (NULL);
}

int compare(const void *e1, const void *e2) {
  return ((int)(*(int *)e1 - *(int *)e2));
}

double DMin(double a, double b) {
  if (a <= b)
    return (b);
  else
    return (a);
}

double DMax(double a, double b) {
  if (a >= b)
    return (b);
  else
    return (a);
}

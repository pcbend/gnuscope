/* analysis.c
 *
 * by John Pavan
 *
 * The analysis functions for use with gnuscope
 */
//--ddc jan13, add corrections to formula for gaussian area by Tai Pei-Luan
//

#include "gnuscopefuncs.h"
#include "gnuscopeglobals.h"
//--ddc 3jun08 remaining problems with function prototypes
//--ddc nov10 gtk2 deprecations...

#include "ecallib.h"

/* --- Globals --- */
/* --- should be handled by including gnuscopeglobals.h --- */

/* --- Function Declarations --- */
/* --- should be handled by inlcuding gnuscopefuncs.h --- */

/* --- Functions --- */

/*
 * GetSum
 *
 * Sum the channels between (and including) the markers
 * return: Range, Center, Center Energy, Total, Total - Background
 */
void GetSum() {
  int i, j, k;
  int counts, width;
  int localmarkers[2];
  float center, deviation;
  float centerenergy, correctedcounts;
  float ff;
  char dummystr[80];
  int tempbackground;

  if (markers[1] > markers[0]) {
    localmarkers[0] = markers[0];
    localmarkers[1] = markers[1];
  } else {
    localmarkers[1] = markers[0];
    localmarkers[0] = markers[1];
  }

  width = localmarkers[1] - localmarkers[0] + 1;

  /* --- Let's get the counts --- */
  counts = GetSumCounts(localmarkers[0], localmarkers[1]);
  tempbackground = GetBackgroundCounts(localmarkers[0], localmarkers[1]);

  /* --- proceed only if the number of counts is > 0 --- */
  if (counts != 0) {
    /* --- Let's get the center --- */
    center = GetSumCenter(localmarkers[0], localmarkers[1], counts);

    /* --- Let's get the standard deviation --- */

    deviation = GetSumDeviation(localmarkers[0], localmarkers[1], counts, center);

    /* --- let's get the fractal dimension thing --- */

    //   ff = GetSumFF(localmarkers[0],localmarkers[1]);

    // sprintf(dummystr,"FF: %f\n",ff);
    // WriteMainText(dummystr);

    centerenergy = UseCalibration(center);
    if (centerenergy != -1) {
      sprintf(dummystr, "Energy: %.1f ", centerenergy);
      WriteMainText(dummystr);
    }
    correctedcounts = UseEfficiency(counts, centerenergy);
    if (centerenergy != -1) {
      sprintf(dummystr, "Corrected Counts: %.1f ", correctedcounts);
      WriteMainText(dummystr);
    }
    sprintf(dummystr, "Channel: %.1f FWHM: %.1f Counts: (%d - %d) = %d\n", (center + 1),
            (deviation * 1.665), counts, tempbackground, (counts - tempbackground));
    WriteMainText(dummystr);
    sprintf(dummystr, "\nSum for channels %d to %d:\n  Center ", (localmarkers[0] + 1),
            (localmarkers[1] + 1));
    WriteMainText(dummystr);
  } else {
    GetMessageDialog("There are no counts between your markers.\n");
  }
}

/* GetSumFF
 *
 * Tries to get the "fitness" of the region as a gaussian
 */
float GetSumFF(int a, int b) {
  int i, j, k;
  float ff;
  int width;
  int tempback[3], tempbackchan[3];
  float tempbak, tempdev;

  if (a > b)
    return (-1);

  /* --- let's start by trying to get the background --- */

  tempback[0] = tempback[1] = tempback[2] = *(histloc[spectra] + a);
  tempbackchan[0] = tempbackchan[1] = tempbackchan[2] = a;
  for (i = a + 1; i <= b; i++) {
    if (*(histloc[spectra] + i) < tempback[0]) {
      tempback[0] = *(histloc[spectra] + i);
      tempbackchan[0] = i;
    }
  }
  for (i = a + 1; i <= b; i++) {
    if ((*(histloc[spectra] + i) < tempback[1]) && (i != tempbackchan[0])) {
      tempback[1] = *(histloc[spectra] + i);
      tempbackchan[1] = i;
    }
  }
  for (i = a + 1; i <= b; i++) {
    if ((*(histloc[spectra] + i) < tempback[2]) && (i != tempbackchan[1]) &&
        (i != tempbackchan[0])) {
      tempback[2] = *(histloc[spectra] + i);
      tempbackchan[2] = i;
    }
  }

  /* --- now we should have the 3 channels with fewest counts --- */
  tempbak = (float)(tempback[0] + tempback[1] + tempback[2]) / (float)3;
  tempdev = sqrt(tempbak);
  if (tempdev <= 0)
    return (-1);

  /* --- it might be useful to know how many channels we are looking at --- */
  width = b - a + 1;

  ff = 0;
  for (i = a; i <= b; i++) {
    ff += (float)(*(histloc[spectra] + i) - tempbak) / tempdev;
  }
  ff = ff / width;
  return (ff);
}

/* GetSumDeviation
 *
 * Returns the standard deviation for a sum
 * both the center value and the counts can be passed to it
 * or determined automatically.  For automatic determination
 * put 0 in the center and/or counts location
 */
float GetSumDeviation(int a, int b, int counts, float center) {
  int i, j, k, width;
  float deviation;
  float tempbackground;

  if (a > b)
    return (-1);

  width = b - a + 1;

  if (counts == 0)
    counts = GetSumCounts(a, b);
  if (counts == 0)
    return (-1);
  if (center <= 0)
    center = GetSumCenter(a, b, counts);
  if (center <= 0)
    return (-1);

  deviation = 0;
  for (i = a; i <= b; i++) {
    deviation = deviation + (i - center) * (i - center) * (*(histloc[spectra] + i) - Background(i));
  }
  tempbackground = GetBackgroundCounts(a, b);
  deviation = sqrt(deviation / (counts - (tempbackground)));
  return (deviation);
}

/*
 * GetSumCenter
 *
 * Returns the "center" between channel a and channel b.
 * Counts can be done automatically or passed to this function.
 * for automatic count finding put 0 in the counts location.
 */
float GetSumCenter(int a, int b, int counts) {
  int i, j, k, width;
  float center;
  float tempbackground;

  if (a > b)
    return (-1);

  width = b - a + 1;

  if (counts == 0) {
    counts = GetSumCounts(a, b);
  }
  if (counts == 0)
    return (-1);

  center = 0;
  for (i = a; i <= b; i++) {
    center = center + i * (*(histloc[spectra] + i) - Background(i));
  }
  tempbackground = GetBackgroundCounts(a, b);
  center = center / (counts - (tempbackground));
  return (fabs(center));
}

/*
 * GetSumCounts
 *
 * Returns the counts between channel a and channel b.
 */
int GetSumCounts(int a, int b) {
  int i, j, k, counts;

  /* --- proceed only if a < b --- */
  if (a > b)
    return (0);

  counts = 0;
  for (i = a; i <= b; i++) {
    counts = counts + *(histloc[spectra] + i);
  }
  return (counts);
}

/*
 * FixedWidthGaussFitPrompt
 *
 * Makes the text for the FWGF dialog
 */
void FixedWidthGaussFitPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Width for Gauss Fit\n");
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * FixedWidthGaussFitEntry
 *
 * Interprets the Fixed Width Gauss fit dialog entry and does the fit
 */
void FixedWidthGaussFitEntry(GtkWidget *widget, GtkWidget *entry) {
  float deviation;
  int test;

  // WriteMainText("Stop 0.\n");
  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%f", &deviation);
  if (test == 1) {
    FixedWidthGaussFit(deviation);
  }
}

/* FixedWidthGaussFit
 *
 * actually does the fixed with gauss fit
 */
void FixedWidthGaussFit(float deviation) {
  float chiold, delx;
  int localmarkers[2];
  float delxold, deldeviationold;
  float center;
  int i, j, k;
  int fixwflg, flagw;
  float chisqr, area, centerenergy, deviationenergy;
  int exitflag;
  float bestfit[4];
  int test;
  char dummystr[80];

  //  WriteMainText("Stop0.1.\n");
  delx = 0.1;

  /* --- Get the markers in the right order --- */
  localmarkers[0] = markers[0];
  localmarkers[1] = markers[1];
  if (localmarkers[0] > localmarkers[1]) {
    i = localmarkers[1];
    localmarkers[1] = localmarkers[0];
    localmarkers[0] = i;
  }
  //--ddc 6jun06 add more inits    bestfit[0] = 10000000
  bestfit[0] = 10000000;
  for (i = 1; i < 4; i++)
    bestfit[i] = 0;
  chiold = 10000000;
  flagw = 0;
  exitflag = 0;
  center = GetSumCenter(localmarkers[0], localmarkers[1], 0);
  deviation = deviation / 1.665;
  //--ddc 1jun06, need init for deldeviationold,
  deldeviationold = 0;
  exitflag = 0;
  // WriteMainText("Stop 1.\n");
  while ((exitflag != 1) && ((delx > 0.009) || (-delx > 0.009))) {
    chisqr = GaussChiSqr(localmarkers[0], localmarkers[1], center, deviation, &area);
    if (chisqr == 0)
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
  // WriteMainText("Stop 2.\n");

  if (globalcalibrationset == 1) {
    if ((calibration[spectra][0] != 0) || (calibration[spectra][1] != 0) ||
        (calibration[spectra][2] != 0)) {
      centerenergy = calibration[spectra][0] + calibration[spectra][1] * (center + 1) +
                     calibration[spectra][2] * (center + 1) * (center + 1);
      //--ddc dbg 3jun08 calibration[spectra][2] * calibration[spectra][2] * (center + 1);
      deviationenergy =
          calibration[spectra][1] * deviation + calibration[spectra][2] * deviation * deviation;
      //--ddc dbg 3jun08 calibration[spectra][2] * calibration[spectra][2] * deviation;
    } else {
      centerenergy = globalcalibration[0] + globalcalibration[1] * (center + 1) +
                     globalcalibration[2] * (center + 1) * (center + 1);
      //--ddc dbg 3jun08 globalcalibration[2] * globalcalibration[2] * (center+1);
      deviationenergy =
          globalcalibration[1] * deviation + globalcalibration[2] * deviation * deviation;
      //--ddc dbg 3jun08 globalcalibration[2] * globalcalibration[2] * deviation;
    }
    sprintf(dummystr, "Energy: %.1f keV FWHM: %.1f keV\n", centerenergy, (deviationenergy * 1.665));
    WriteMainText(dummystr);
  }

  // centerenergy = UseCalibration(center);
  // if (centerenergy != -1) {
  //   deviationenergy = UseCalibration(deviation);
  //   sprintf(dummystr,"Energy: %.1f keV FWHM: %.1f keV\n",centerenergy,(1.665 * deviationenergy));
  //   WriteMainText(dummystr);
  // }
  sprintf(dummystr, "Channel: %.1f FWHM: %.1f Area: %.1f ChiSqr: %.1f\n", (center + 1),
          (deviation * 1.665), (deviation * area * 1.77245), chisqr);
  WriteMainText(dummystr);
  sprintf(dummystr, "\nFixed FWHM Gauss fit for channels %d to %d:\n Center ",
          (localmarkers[0] + 1), (localmarkers[1] + 1));
  WriteMainText(dummystr);
  DisplayGaussian(center, deviation, area);
}

/*
 * GaussFit
 *
 * an attempt to convert the fortran gauss-fit program to c
 * (of course we are trying to use it with this program)
 */
void GaussFit() {
  float deviation, chiolddeviation, deldeviation, flagdeviation, chiold, delx;
  int localmarkers[2];
  float delxold, deldeviationold;
  float center;
  int i, j, k;
  int fixwflg, flagw;
  float chisqr, area, centerenergy, deviationenergy, correctedcounts;
  int exitflag;
  float bestfit[4];
  char dummystr[80];

  // graph = lastgraphclicked;
  // spectra = lastspectraclicked;

  /* --- Get the markers in the right order --- */
  localmarkers[0] = markers[0];
  localmarkers[1] = markers[1];
  if (localmarkers[0] > localmarkers[1]) {
    i = localmarkers[1];
    localmarkers[1] = localmarkers[0];
    localmarkers[0] = i;
  }

  //--ddc 6jun06 add more inits to  bestfit[0] = 1000000;
  bestfit[0] = 1000000;
  ;
  for (i = 1; i < 4; i++)
    bestfit[i] = 0;
  chiolddeviation = 10000000;
  deldeviation = -0.05;
  //--ddc 1jun06, need init for deldeviationold,
  deldeviationold = 0;
  flagw = 0;
  center = GetSumCenter(localmarkers[0], localmarkers[1], 0);
  deviation = GetSumDeviation(localmarkers[0], localmarkers[1], 0, center);
  /* --- let's see if I can write this without the goto statements --- */
  exitflag = 0;
  while ((exitflag != 1) && ((deldeviation > 0.0001) || ((-deldeviation) > 0.0001))) {
    chiold = 1000000;
    delx = 0.1;
    chisqr = GaussChiSqr(localmarkers[0], localmarkers[1], center, deviation, &area);
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
      chisqr = GaussChiSqr(localmarkers[0], localmarkers[1], center, deviation, &area);
      //--ddc debug 062805!!! There are many times when trapped this loop with
      // chisqr = -1 !!! I'm adding a "break" to setting the exit flag.
      if (chisqr == -1) {
        exitflag = 1;
        break;
      }
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
    //           center,(deviation*1.665),(area*deviation *1.77245),chisqr);
    if (chisqr < bestfit[0]) {
      bestfit[0] = chisqr;
      bestfit[1] = center;
      bestfit[2] = deviation;
      bestfit[3] = area;
    }
  }
  if (globalcalibrationset == 1) {
    if ((calibration[spectra][0] != 0) || (calibration[spectra][1] != 0) ||
        (calibration[spectra][2] != 0)) {
      centerenergy = calibration[spectra][0] + calibration[spectra][1] * (bestfit[1] + 1) +
                     calibration[spectra][2] * (bestfit[1] + 1) * (bestfit[1] + 1);
      //--ddc dbg 3jun08 calibration[spectra][2] * calibration[spectra][2] * (bestfit[1] + 1);
      deviationenergy =
          calibration[spectra][1] * bestfit[2] + calibration[spectra][2] * bestfit[2] * bestfit[2];
      //--ddc dbg 3jun08 calibration[spectra][2] * calibration[spectra][2] * bestfit[2];
    } else {
      centerenergy = globalcalibration[0] + globalcalibration[1] * (bestfit[1] + 1) +
                     globalcalibration[2] * (bestfit[1] + 1) * (bestfit[1] + 1);
      //--ddc dbg 3jun08 globalcalibration[2] * globalcalibration[2] * (bestfit[1]+1);
      deviationenergy =
          globalcalibration[1] * bestfit[2] + globalcalibration[2] * bestfit[2] * bestfit[2];
      //--ddc dbg 3jun08  globalcalibration[2] * globalcalibration[2] * bestfit[2];
    }
    sprintf(dummystr, "Energy: %.1f keV FWHM: %.1f keV\n", centerenergy, (deviationenergy * 1.665));
    WriteMainText(dummystr);
  }
  correctedcounts = UseEfficiency((bestfit[3] * bestfit[2] * 1.77245), centerenergy);
  if (correctedcounts != -1) {
    sprintf(dummystr, " Efficiency corrected counts: %.1f   \n", correctedcounts);
    WriteMainText(dummystr);
  }
  sprintf(dummystr, "Channel: %.1f FWHM: %.1f Area: %.1f ChiSqr: %.1f\n", (bestfit[1] + 1),
          (bestfit[2] * 1.665), (bestfit[3] * bestfit[2] * 1.77245), bestfit[0]);
  WriteMainText(dummystr);

  sprintf(dummystr, "Gauss fit for channels %d to %d:\n  Center ", (localmarkers[0] + 1),
          (localmarkers[1] + 1));
  WriteMainText(dummystr);

  DisplayGaussian(bestfit[1], bestfit[2], bestfit[3]);

  if (chisqr == -1)
    GetMessageDialog("Gauss fit failed.\n");
}

/*
 * GaussChiSqr
 *
 * Return the ChiSqr from the comparison of data to a gausian
 */
float GaussChiSqr(int a, int b, float center, float deviation, float *area) {
  float yf, ff;
  int i, j, k;
  float x, y;
  float ye;
  //--ddc mar17 upsize arrays from 8192 to 32768 for XIA
  float gaussian[32768];
  float chisqr, area_value;

  yf = 0;
  ff = 0;

  if (a > b) {
    GetMessageDialog("Something went wrong with the gauss fit.\n");
    return (-1);
  }

  for (i = a; i <= b; i++) {
    x = i;
    y = *(histloc[spectra] + i);
    ye = y;
    if (ye == 0)
      ye = 1;
    gaussian[i] = (float)exp(-((i - center) * (i - center) / deviation / deviation));
    yf = yf + (y - Background(i)) * gaussian[i] / ye;
    ff = ff + gaussian[i] * gaussian[i] / ye;
  }

  chisqr = 0;
  if (ff == 0) {
    return (-1);
  }
  area_value = (float)yf / ff;
  *area = (float)area_value;
  for (i = a; i <= b; i++) {
    gaussian[i] = gaussian[i] * area_value + Background(i);
    ye = *(histloc[spectra] + i);
    if (ye == 0) {
      ye = 1;
    }

    chisqr = chisqr + (float)(*(histloc[spectra] + i) - gaussian[i]) *
                          (float)(*(histloc[spectra] + i) - gaussian[i]) / fabs(ye);
  }
  return (chisqr);
}

/*
 * DisplayGaussian
 *
 * Displays a gaussian fit given the center, deviation, and area
 * The gaussian will be displayed assuming that markers[0 - 1] is the
 * region which should contain the new plot
 */
void DisplayGaussian(float center, float deviation, float area) {
  int i, j, k;
  int a, b;
  float stepsize;
  GdkPoint points;
  int max = 0;
  int min = 0;
  int minmaxrange;
  int bar_height, next_bar_height;
  float width, height;
  float tempsum, tempavg;
  int localmarkers[2];
  long double rangewidth, currentscaling;
  float xchan, nextxchan;

  //  graph = lastgraphclicked;
  // spectra = lastspectraclicked;

  if (gtk_graph_has_segments(graph)) {
    gtk_graph_clear_segments(GTK_GRAPH(graph));
  }
  if (lastfitnumpoints) {
    LastFitClearPoints();
  }
  if (markers[0] > markers[1]) {
    localmarkers[1] = markers[0];
    localmarkers[0] = markers[1];
  } else {
    localmarkers[0] = markers[0];
    localmarkers[1] = markers[1];
  }
  /* --- for this function, we want to display twice the space
     --- between the markers --- */
  i = localmarkers[1] - localmarkers[0];
  localmarkers[0] = localmarkers[0] - i / 2;
  localmarkers[1] = localmarkers[1] + i / 2;

  rangewidth = currentrange[1] - currentrange[0];
  if (rangewidth != 0) {
    currentscaling = (long double)((graph->allocation.width - 100) / rangewidth);
  }
  //  printf("Rangewidth: %d Currentscaling : %ef ",rangewidth,currentscaling);

  /* --- Display each value --- */
  /* --- do this by figuring out which pixels on the screen are between the localmarkers[0-1]
     --- and then plot the gaussian between them. --- */
  if (binsize > 1) {
    a = (localmarkers[0] - currentrange[0]) * displayused * column_width + 100;
    b = (localmarkers[1] - currentrange[0]) * displayused * column_width + 100;
  } else {
    a = (localmarkers[0] - currentrange[0] + 0.5) * currentscaling + 100;
    b = (localmarkers[1] - currentrange[0] + 0.5) * currentscaling + 100;
  }

  for (i = a; i <= b; i++) {
    /* --- now we have to convert back to get the right channel numbers --- */
    if (binsize > 1) {
      xchan = (i - 100) / displayused / column_width + (float)currentrange[0];
    } else {
      if ((binsize == 1) && (currentscaling != 0)) {
        xchan = (i - (100)) / currentscaling + currentrange[0] - 0.5;
      }
    }

    bar_height = (area * exp(-(xchan - center) * (xchan - center) / deviation / deviation)) +
                 Background(xchan);
    gtk_graph_add_segment(GTK_GRAPH(graph), (float)xchan, (float)bar_height * binsize);
  }
  Redraw();
  // gtk_widget_draw(graph,NULL);
}

/*
 * SetBackground
 *
 * Sets the background based on the markers
 */
void SetBackground(int mode) {
  int width, tempsum, i, j, k;
  int localmarkers[4];
  char dummystr[80];
  double *xpts, *ypts, *sigpts;
  int numpts;

  /* --- Once John gets smart we will be
     able to set the polynomial degree of the background fit --- */
  /* --- Until then ... --- */

  localmarkers[0] = Min(markers[0], markers[1]);
  localmarkers[1] = Max(markers[0], markers[1]);
  localmarkers[2] = Min(markers[2], markers[3]);
  localmarkers[3] = Max(markers[2], markers[3]);

  /* --- John is going to try to get smart
     --- oop wait a minute, my shoe is ringing --- */

  /* --- for backgroundpolydeg == 0 we are looking
     --- at merely the average of the values
     --- between the first 2 markers --- */

  /* --- otherwise, we need to use PolFit --- */
  /* --- hmmm... how do we do multi-region fitting --- */
  /* --- we can do that via mode --- */

  switch (backgroundpolydeg) {
  case 0:
    width = localmarkers[1] - localmarkers[0];
    if (width < 1) {
      width = 1;
    }
    tempsum = 0;
    for (i = localmarkers[0]; i <= localmarkers[1]; i++) {
      tempsum = tempsum + *(histloc[spectra] + i);
    }
    backgroundpolydeg = 0;
    background[0] = tempsum / (width + 1);
    sprintf(dummystr, "Background set to %f.\n", background[0]);
    WriteMainText(dummystr);
    break;
  default:
    /* --- this is multi-region background fitting mode --- */
    /* --- we use it when we are in either linear or quadratic modes --- */
    numpts = localmarkers[1] - localmarkers[0] + localmarkers[3] - localmarkers[2] + 2;
    xpts = (double *)malloc(sizeof(double) * numpts);
    ypts = (double *)malloc(sizeof(double) * numpts);
    sigpts = (double *)malloc(sizeof(double) * numpts);
    j = 0;
    for (i = localmarkers[0]; i <= localmarkers[1]; i++) {
      xpts[j] = (double)i;
      ypts[j] = *(histloc[spectra] + i);
      sigpts[j] = 1;
      j++;
    }
    for (i = localmarkers[2]; i <= localmarkers[3]; i++) {
      xpts[j] = i;
      ypts[j] = *(histloc[spectra] + i);
      sigpts[j] = 1;
      j++;
    }
    PolFit(backgroundpolydeg + 1, xpts, ypts, sigpts, numpts, 0, &background);
    free(xpts);
    free(ypts);
    free(sigpts);
    //    width = localmarkers[1] - localmarkers[0];
    // if (width < 1) {
    //  width = 1;
    // }
    // tempsum = 0;
    // for (i = localmarkers[0]; i <= localmarkers[1]; i++) {
    //  tempsum = tempsum + *(histloc[spectra] + i);
    // }
    // background[0] = tempsum / (width+1) ;
    // width = localmarkers[3] - localmarkers[2];
    // if (width < 1) {
    //  width = 1;
    //}
    // tempsum = 0;
    // for (i = localmarkers[2]; i <= localmarkers[3]; i++) {
    //  tempsum = tempsum + *(histloc[spectra] + i);
    // }
    // background[0] += tempsum / (width+1) ;
    for (i = backgroundpolydeg; i > 0; i--) {
      sprintf(dummystr, ", %f", background[i]);
      WriteMainText(dummystr);
    }
    sprintf(dummystr, "Background set to %f", background[i]);
    WriteMainText(dummystr);
    WriteMainText(".\n");
    break;
  }

  DrawBackground(graph);
}

/*
 * ManualSetBackgroundPrompt
 *
 * Sets the message for the Manual Set Background dialog
 */
void ManualSetBackgroundPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Enter new background\n");
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * ManualSetBackgroundEntry
 *
 * Interprets the manual set background dialog and sets the background
 */
void ManualSetBackgroundEntry(GtkWidget *widget, GtkWidget *entry) {
  int test, input;

  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%d", &input);
  if (test == 1) {
    backgroundpolydeg = 0;
    background[0] = input;
    DrawBackground(graph);
  }
}

/*
 * OneChSetBackground
 *
 * Sets the background based on the location of the current marker (markers[0])
 */
void OneChSetBackground() {
  int i, tempsum;
  char dummystr[80];

  tempsum = 0;
  for (i = 0; i < binsize; i++) {
    tempsum = tempsum + *(histloc[spectra] + i + markers[0]);
  }
  backgroundpolydeg = 0;
  background[0] = tempsum / binsize;
  sprintf(dummystr, "\nBackground set to %f.", background[0]);
  WriteMainText(dummystr);
  DrawBackground(graph);
}

/*
 * DrawBackground
 *
 * Draws the background fit to the graph
 */
void DrawBackground() {
  GdkPoint points[2];
  int height;
  int i, j, k;
  int tempsum;
  int max;
  int min;
  int minmaxrange;
  max = 0;
  min = 0;

  height = graph->allocation.height - 31;

  //  printf("Height of window is %d.",height);

  /* --- have to determine the max value if more than one graph is displayed --- */

  if (yscaleforce == 1) {
    max = yscale;
    min = -yscale;
  } else {
    for (i = currentrange[0]; i <= currentrange[1]; i = i + binsize) {
      tempsum = 0;
      for (j = 0; j < binsize; j++) {
        tempsum = tempsum + *(histloc[spectra] + i + j);
      }
      if (max < tempsum)
        max = tempsum;
      if (min > tempsum)
        min = tempsum;
    }
  }
  //--ddc aug11 See gtkgraph.c for discussion with gtk_graph_y_scale

  max = max * gtk_graph_y_scale();
  min = min * gtk_graph_y_scale();
  max = max * 1.1;
  min = min * 1.1;
  if (max < 10)
    max = 10;
  if (min > 0)
    min = 0;

  minmaxrange = max - min;
  //    printf("Max is %d.\n",max);

  /* --- if we are going to have this done via polynomial background
     --- we should probably bite the bullet and use the "segments"
     --- option for the graph --- */

  if (gtk_graph_has_segments(graph)) {
    gtk_graph_clear_segments(GTK_GRAPH(graph));
  }
  for (i = currentrange[0]; i <= currentrange[1]; i++) {
    gtk_graph_add_segment(GTK_GRAPH(graph), i, (int)(Background(i) - min));
  }
  Redraw();

  //  if ((max > 0) && (binsize > 0)){
  //  points[0].x = 0;
  //  points[0].y = height - ((Background(currentrange[0]) - min) * height / minmaxrange);
  //  points[1].x = graph->allocation.width;
  //  points[1].y = height - ((Background(currentrange[1]) - min) * height / minmaxrange);
  //} else {
  //      printf("Max is %d, binsize is %d.\n",max,binsize);
  //}
  //    printf("attempting to draw a background line at pixel height %d.\n",points[1].y);
  // gdk_draw_lines(graph->window,
  //		 graph->style->black_gc,
  //		 points,2);
}

/* --- functions and subroutines for double gauss fit --- */

/*
 * DoubleGaussFitPrompt
 *
 * Prompts for the double gauss fit parameters
 */
void DoubleGaussFitPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Double Gauss Fit\n");
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "Enter the FWHM for each\n");
  gtk_text_buffer_insert_at_cursor(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "If widths are the same\n");
  gtk_text_buffer_insert_at_cursor(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "they will be varied together.\n");
  gtk_text_buffer_insert_at_cursor(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "FWHMA FWHMB\n");
  gtk_text_buffer_insert_at_cursor(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * DoubleGaussFitEntry
 *
 * Interprets the entry of the DGF dialog
 * based strongly on the fortran from scope.f
 */
void DoubleGaussFitEntry(GtkWidget *widget, GtkWidget *entry) {
  float deviation1, deviation2;
  int test;

  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%f %f", &deviation1, &deviation2);
  if (test == 2) {
    DoubleGaussFit(deviation1, deviation2);
  }
}

/* DoubleGaussFit
 *
 * actually does the double gauss fit
 */
void DoubleGaussFit(float deviation1, float deviation2) {
  float chiold;
  float deviationarray[4], centerarray[7], chioldarray[7];
  float center1, center2;
  int i, j, k;
  float chisqr, area1, area2, centerenergy1,
      centerenergy2 = 0; //--ddc 5jun06 initialize centerenergy2
  float correctedcounts1, correctedcounts2;
  float deviationenergy1, deviationenergy2;
  int exitflag;
  float delx1, delx2, delw1, delw2;
  int fwflg;
  int test;
  char dummystr[80];

  /* --- let's get the markers into the correct order --- */
  qsort(&markers[0], 4, sizeof(int), CompareInts);
  deviation1 = deviation1 / 1.665;
  deviation2 = deviation2 / 1.665;
  //    printf("Beginning double Gaussian fit.\n");
  /* --- initializing variables --- */
  exitflag = 0;
  center1 = markers[1];
  center2 = markers[2];
  deviationarray[0] = deviation1; /* w13 */
  deviationarray[1] = deviation1; /* w14 */
  deviationarray[2] = deviation2; /* w23 */
  deviationarray[3] = deviation2; /* w24 */
  centerarray[0] = markers[1];    /* xcenter11 */
  centerarray[1] = markers[1];    /* xcenter12 */
  centerarray[2] = markers[1];    /* xcenter13 */
  centerarray[3] = markers[1];    /* xcenter14 */
  centerarray[4] = markers[2];    /* xcenter22 */
  centerarray[5] = markers[2];    /* xcenter23 */
  centerarray[6] = markers[2];    /* xcenter24 */
  for (i = 0; i < 7; i++)
    chioldarray[i] = 10000000;
  /* --- following assigements --- */
  /* chiold11 = [0]    chiold22 = [1]
   * chiold33 = [2]    chiold44 = [3]
   * chiold4  = [4]    chiold3  = [5]
   * chiold2  = [6]
   */
  fwflg = 0;
  chiold = 10000000;
  delx2 = 0.5; /* delx2 */
  delx1 = 0.1; /* delx1 */
  delw1 = 0.5; /* delw1 */
  delw2 = 0.5; /* delw2 */
  if (deviation1 == deviation2) {
    fwflg = 1;
  }

  /* --- horray, we are done with the initializations --- */
  /* --- And now for the main loop --- */
  while (fabs((double)delw2) > (double)0.009) {
    //    printf("Entering main loop.\n");
    while (fabs((double)delw1) > (double)0.009) {
      //      printf("Entering secondary loop.\n");
      while (fabs((double)delx2) > (double)0.009) {
        //	printf("Entering Tertiary Loop.\n");
        delx1 = 5 * delx1;
        chiold = 10000000;

        /* --- begin loopone --- */
        while (fabs((double)delx1) > (double)0.009) {
          //	  printf("Entering Quadrary loop.\n");
          chisqr = DoubleGaussChiSqr(markers[0], markers[3], center1, center2, deviation1,
                                     deviation2, &area1, &area2);
          if (chisqr == -1) {
            if (center1 == center2) {
              goto jumpone;
            } else {
              goto aborted;
            }
          }

        jumpone:
          if (chisqr <= chiold) {
            centerarray[0] = center1;
            chioldarray[0] = chisqr;
            chiold = chisqr;
            center1 = center1 + delx1;
          } else {
            if (fabs((double)delx1) > (double)0.009) {
              delx1 = -delx1 / 5;
              chiold = chisqr;
              center1 = center1 + delx1;
            } else {
              center1 = centerarray[0];
              chisqr = chioldarray[0];
            }
          }
        }
        /* --- end loop 1 --- */

        /* --- on, now we deal with center2 --- */
        if (chisqr <= chioldarray[6]) {
          chioldarray[6] = chisqr;
          centerarray[4] = center2;
          centerarray[1] = center1;
          chioldarray[1] = chisqr;
          center2 = center2 + delx2;
        } else {
          if (fabs((double)delx2) > (double)0.009) {
            delx2 = -delx2 / 5;
            chioldarray[6] = chisqr;
            center2 = center2 + delx2;
          } else {
            center1 = centerarray[1];
            center2 = centerarray[4];
            chisqr = chioldarray[1];
            chioldarray[6] = 10000000;
          }
        }
      }
      //      printf("center1: %f center 2: %f chisqr: %f.\n",center1,center2,chisqr);
      /* --- now we deal with deviations --- */
      if (chisqr <= chioldarray[5]) {
        chioldarray[2] = chisqr;
        chioldarray[5] = chisqr;
        centerarray[2] = center1;
        centerarray[5] = center2;
        deviationarray[0] = deviation1;
        delx2 = -5 * delx2;
        deviation1 = deviation1 + delw1;
        if (fwflg == 1) {
          deviationarray[2] = deviation2;
          deviation2 = deviation2 + delw1;
        }
      } else {
        if (fabs((double)delw1) >= (double)0.009) {
          delw1 = -delw1 / 5;
          chioldarray[5] = chisqr;
          deviation1 = deviation1 + delw1;
          delx2 = -5 * delx2;
          if (fwflg == 1) {
            deviation2 = deviation1 + delw1;
          }
        } else {
          if (fwflg == 1)
            goto jumpseven;
          center1 = centerarray[2];
          center2 = centerarray[5];
          deviation1 = deviationarray[0];
          chisqr = chioldarray[2];
          chioldarray[5] = 10000000;
        }
      }
    }

    if (chisqr <= chioldarray[4]) {
      chioldarray[4] = chisqr;
      chioldarray[3] = chisqr;
      centerarray[3] = center1;
      centerarray[6] = center2;
      deviationarray[1] = deviation1;
      deviationarray[3] = deviation2;
      deviation2 = deviation2 + delw2;
      delx2 = -5 * delx2;
      delw1 = -5 * delw1;
    } else {
      if (fabs((double)delw2) > (double)0.009) {
        delw2 = -delw2 / 5;
        chioldarray[4] = chisqr;
        deviation2 = deviation2 + delw2;
      }
    }
  }
  if (fabs((double)delw2) < (double)0.01) {
    center1 = centerarray[3];
    center2 = centerarray[6];
    deviation1 = deviationarray[1];
    deviation2 = deviationarray[3];
    chisqr = chioldarray[3];
    goto jumpdone;
  }
jumpseven:
  center1 = centerarray[2];
  center2 = centerarray[5];
  deviation1 = deviationarray[0];
  deviation2 = deviationarray[2];
  chisqr = chioldarray[2];
  goto jumpdone;
  /* --- end main loop --- */

jumpdone:
  /* --- now we need to print the results --- */
  sprintf(dummystr, "ChiSqr = %.1f\n", chisqr);
  WriteMainText(dummystr);
  sprintf(dummystr, "Peak 2 Results in channels:  Center: %.1f FWHM: %.1f Area: %.1f\n",
          (center2 + 1), (deviation2 * 1.665), (deviation2 * area2 * 1.77245));
  WriteMainText(dummystr);
  sprintf(dummystr, "Peak 1 Results in channels:  Center: %.1f FWHM: %.1f Area: %.1f\n",
          (center1 + 1), (deviation1 * 1.665), (deviation1 * area1 * 1.77245));
  WriteMainText(dummystr);

  /* --- if there is a calibration we ought to use it --- */
  centerenergy1 = UseCalibration(center1);
  if (centerenergy1 != -1) {
    deviationenergy1 = UseCalibrationFWHM(deviation1);
    centerenergy2 = UseCalibration(center2);
    deviationenergy2 = UseCalibrationFWHM(deviation2);
    sprintf(dummystr, "Peak 2 Results in Energy:  Center: %.1f FWHM: %.1f\n", centerenergy2,
            (deviationenergy2 * 1.665));
    WriteMainText(dummystr);
    sprintf(dummystr, "Peak 1 Results in Energy:  Center: %.1f FWHM: %.1f\n", centerenergy1,
            (deviationenergy1 * 1.665));
    WriteMainText(dummystr);
  }
  correctedcounts1 = UseEfficiency((deviation1 * area1 * 1.77245), centerenergy1);
  if (correctedcounts1 != -1) {
    sprintf(dummystr, " Efficiency corrected counts for peak 1: %.1f   \n", correctedcounts1);
    WriteMainText(dummystr);
  }
  correctedcounts2 = UseEfficiency((deviation2 * area2 * 1.77245), centerenergy2);
  if (correctedcounts2 != -1) {
    sprintf(dummystr, " Efficiency corrected counts for peak 2: %.1f   \n", correctedcounts2);
    WriteMainText(dummystr);
  }

  /* --- guess we need to display it too --- */
  DisplayDoubleGaussian(center1, center2, deviation1, deviation2, area1, area2);
aborted:
  if (chisqr == -1)
    GetMessageDialog("Double Gauss fit Aborted.\n");
}

/*
 * DisplayDoubleGaussian
 *
 * Displays a double gaussian given the appropriate variables
 * The display will be in the active graph
 */
void DisplayDoubleGaussian(float center1, float center2, float deviation1, float deviation2,
                           float area1, float area2) {
  int i, j, k;
  int a, b;
  float stepsize;
  GdkPoint fpoints, gpoints, hpoints;
  int max = 0;
  int f_bar_height, f_next_bar_height, g_bar_height, g_next_bar_height, h_bar_height,
      h_next_bar_height;
  float width, height;
  float tempsum, tempavg;
  int localmarkers[2];
  long double rangewidth, currentscaling;
  float xchan, nextxchan;
  int min = 0;
  int minmaxrange;

  /* --- get the markers in order --- */
  if (markers[0] > markers[1]) {
    localmarkers[1] = markers[0];
    localmarkers[0] = markers[3];
  } else {
    localmarkers[0] = markers[0];
    localmarkers[1] = markers[3];
  }

  if (gtk_graph_has_segments(GTK_GRAPH(graph)))
    gtk_graph_clear_segments(GTK_GRAPH(graph));
  if (lastfitnumpoints)
    LastFitClearPoints();
  /* --- as with the single gauss fit we want to draw outside of the markers --- */
  i = localmarkers[1] - localmarkers[0];
  localmarkers[0] = localmarkers[0] - i / 2;
  localmarkers[1] = localmarkers[1] + i / 2;

  rangewidth = currentrange[1] - currentrange[0];
  if (rangewidth != 0) {
    currentscaling = (long double)((graph->allocation.width - 100) / rangewidth);
  }

  /* --- now we have to display things --- */
  /* --- do this by figuring out which pixels on the screen are between the
     --- markers[0-1] and then plot the gaussian between them. --- */
  if (binsize > 1) {
    a = (localmarkers[0] - currentrange[0]) * displayused * column_width + 100;
    b = (localmarkers[1] - currentrange[0]) * displayused * column_width + 100;
  } else {
    a = (localmarkers[0] - currentrange[0] + 0.5) * currentscaling + 100;
    b = (localmarkers[1] - currentrange[0] + 0.5) * currentscaling + 100;
  }

  /* --- time to draw --- */

  for (i = a; i <= b; i++) {
    /* --- now we need to convert back to the right channel numbers --- */
    if (binsize > 1) {
      xchan = (i - 100) / displayused / column_width + (float)currentrange[0];
    } else {
      if ((binsize == 1) && (currentscaling != 0)) {
        xchan = (i - (100)) / currentscaling + currentrange[0] - .5;
      }
    }
    f_bar_height =
        ((area1 * exp(-(xchan - center1) * (xchan - center1) / deviation1 / deviation1)));
    gtk_graph_add_segment(GTK_GRAPH(graph), (float)xchan,
                          (float)(f_bar_height + Background(xchan)) * binsize);
  }

  for (i = a; i <= b; i++) {
    /* --- now we need to convert back to the right channel numbers --- */
    if (binsize > 1) {
      xchan = (i - 100) / displayused / column_width + (float)currentrange[0];
    } else {
      if ((binsize == 1) && (currentscaling != 0)) {
        xchan = (i - (100)) / currentscaling + currentrange[0] - .5;
      }
    }
    g_bar_height =
        ((area2 * exp(-(xchan - center2) * (xchan - center2) / deviation2 / deviation2)));
    gtk_graph_add_segment(GTK_GRAPH(graph), (float)xchan,
                          (float)(g_bar_height + Background(xchan)) * binsize);
  }

  for (i = a; i <= b; i++) {
    /* --- now we need to convert back to the right channel numbers --- */
    if (binsize > 1) {
      xchan = (i - 100) / displayused / column_width + (float)currentrange[0];
    } else {
      if ((binsize == 1) && (currentscaling != 0)) {
        xchan = (i - (100)) / currentscaling + currentrange[0] - .5;
      }
    }
    /* --- assign the y possitions --- */
    f_bar_height =
        (((area1 * exp(-(xchan - center1) * (xchan - center1) / deviation1 / deviation1))));
    g_bar_height =
        (((area2 * exp(-(xchan - center2) * (xchan - center2) / deviation2 / deviation2))));
    h_bar_height = f_bar_height + g_bar_height + Background(xchan);
    gtk_graph_add_segment(GTK_GRAPH(graph), xchan, h_bar_height * binsize);
  }
  //  printf("\n");
  Redraw();
}

/*
 * DoubleGuassChiSqr
 *
 * calculates the chisqr for a double gaussian.
 */
float DoubleGaussChiSqr(int a, int b, float center1, float center2, float deviation1,
                        float deviation2, float *area1, float *area2) {
  float chisqr;
  int i, j, k;
  int x, y, ys;
  //--ddc mar17 upsize arrays from 8192 to 32768 for XIA
  float gaussone[32768], gausstwo[32768], gaussthree[32768];
  float yf, ff, fg, yg, gg, yse, ye;
  float xprime, xsecun;
  float det;

  /* --- initializations --- */
  yf = 0;
  ff = 0;
  fg = 0;
  yg = 0;
  gg = 0;

  for (i = a; i <= b; i++) {
    x = i;
    y = (int)*(histloc[spectra] + i);
    ye = y;
    if (y == 0)
      ye = 1;
    xprime = ((x - center1) / deviation1) * ((x - center1) / deviation1);
    xsecun = ((x - center2) / deviation2) * ((x - center2) / deviation2);
    gaussone[i] = 0;
    gausstwo[i] = 0;
    gaussone[i] = (float)exp(-xprime);
    gausstwo[i] = (float)exp(-xsecun);
    yf = yf + (y - Background(i)) * gaussone[i] / ye;
    yg = yg + (y - Background(i)) * gausstwo[i] / ye;
    ff = ff + gaussone[i] * gaussone[i] / ye;
    fg = fg + gaussone[i] * gausstwo[i] / ye;
    gg = gg + gausstwo[i] * gausstwo[i] / ye;
  }
  det = ff * gg - fg * fg;
  if (det == 0)
    return ((float)-1);
  *area1 = (float)((yf * gg - yg * fg) / det);
  *area2 = (float)((yg * ff - yf * fg) / det);
  chisqr = 0;
  for (i = a; i <= b; i++) {
    gaussthree[i] = (float)*area1 * gaussone[i] + (float)*area2 * gausstwo[i] + Background(i);
    ys = *(histloc[spectra] + i);
    yse = ys;
    if (ys == 0)
      yse = 1;
    chisqr = chisqr + (ys - gaussthree[i]) * (ys - gaussthree[i]) / fabs(yse);
  }
  //  printf("Intermediate results:\n");
  //  printf("For Peak 1 (in channels): Center: %.1f Deviation: %.1f Area: %.1f\n",
  //	 center1, deviation1, (float) *area1);
  //  printf("For Peak 2 (in channels): Center: %f Deviation: %f Area: %f\n",
  //	 center2, deviation2, (float) *area2);
  //  printf("ChiSqr = %f\n",chisqr);
  return (chisqr);
}

/*
 * CompareInts
 *
 * a function for use with Qsort,
 * returns -1 if a is less that b
 * returns 0  if a is equal to b
 * returns 1  if a is greater than b
 */
int CompareInts(const void *a, const void *b) {
  int i, j;

  i = *((int *)a);
  j = *((int *)b);
  if (i > j)
    return (1);
  if (i == j)
    return (0);
  if (i < j)
    return (-1);
}

/* LastFitAddPoint
 *
 * Adds a point to the lastfitpoints pointer and incrments the lastfitnumpoints counter
 */
void LastFitAddPoint(float x, float y) {
  if (lastfitpoints == NULL) {
    lastfitnumpoints = 1;
    lastfitpoints = (struct floatpoint *)malloc(sizeof(struct floatpoint));
    lastfitpoints[0].x = x;
    lastfitpoints[0].y = y;
  } else {
    lastfitnumpoints++;
    lastfitpoints =
        (struct floatpoint *)realloc(lastfitpoints, lastfitnumpoints * sizeof(struct floatpoint));
    lastfitpoints[lastfitnumpoints - 1].x = x;
    lastfitpoints[lastfitnumpoints - 1].y = y;
  }
}

/* LastFitClearPoints
 *
 * Clears out the appropriate stuff
 */
void LastFitClearPoints() {
  if (lastfitpoints == NULL) {
    free(lastfitpoints);
    lastfitpoints = NULL;
    lastfitnumpoints = 0;
  }
}

/* ExpFit
 *
 * Fits an exponential curve to the data between the markers
 * using a least squares fit to background
 *
 * mode = 0 Allow C to vary for the best fit (least squares)
 * mode = 1 Allow C to vary but not go less than zero
 * mode = 2 C cannot vary, remains as the static input
 * backa is the background a
 * backb is the background b
 */
void ExpFit(int mode, double c, double backa, double backb) {
  char dummystr[120];
  int localmarkers[2];
  int i;
  double a, b;
  double xbar, ybar, xybar, xsqrbar;
  double m;
  int n;
  double chisqr;
  double delc;
  double chiold;
  //--ddc mar17 upsize arrays from 8192 to 32768 for XIA
  double temphist[32768];
  double lowestcounts, highestcounts;
  double oldc;
  double oldbacka, delbacka;
  double temp;

  localmarkers[0] = Min(markers[0], markers[1]);
  localmarkers[1] = Max(markers[0], markers[1]);

  n = localmarkers[1] - localmarkers[0] + 1;
  //--ddc mar17 upsize arrays from 8192 to 32768 for XIA
  for (i = 0; i < 32768; i++) {
    temphist[i] = 0;
  }

  lowestcounts = temphist[0];
  highestcounts = temphist[0];
  for (i = 1; i < n; i++) {
    if (highestcounts < temphist[i])
      highestcounts = temphist[i];
    if (lowestcounts > temphist[i])
      lowestcounts = temphist[i];
  }

  if (lowestcounts < 1)
    lowestcounts = 1;
  /* --- this section deals with iterating the linear background --- */
  if (mode < 2) {
    c = 0;
    delc = lowestcounts / (float)2;
    chiold = 2147483647;
    while (abs(delc) > 0.005) {
      /* --- dealing with fitting the exponential background
         --- by fitting backa and leaving backb fixed --- */
      if (backa > 0) {
        delbacka = backa;
        while (delbacka > 1) {
          oldbacka = backa;
          for (i = localmarkers[0]; i <= localmarkers[1]; i++) {
            temp = abs(*(histloc[spectra] + i) - c - backa * exp(backb * i));
            if (temp > 0)
              temphist[i - localmarkers[0]] = log(temp);
            else
              temphist[i - localmarkers[0]] = 0;
          }
          // printf("a\n");
          xbar = 0;
          ybar = 0;
          xybar = 0;
          xsqrbar = 0;
          for (i = localmarkers[0]; i <= localmarkers[1]; i++) {
            xbar = xbar + (float)(i);
            ybar = ybar + temphist[i - localmarkers[0]];
            xybar = xybar + (float)(i)*temphist[i - localmarkers[0]];
            xsqrbar = xsqrbar + (float)i * i;
          }

          // printf("b\n");
          xbar = xbar / (double)n;
          ybar = ybar / (double)n;
          xybar = xybar / (double)n;
          xsqrbar = xsqrbar / (double)n;

          if ((xsqrbar - xbar * xbar) != 0)
            b = (xybar - xbar * ybar) / (xsqrbar - xbar * xbar);
          else
            b = 0;
          a = exp(ybar - b * xbar);

          chisqr = ExpChiSqr(a, b, c, backa, backb);
          if (chisqr > chiold) {
            chiold = chisqr;
            delbacka = -delbacka / 5;
            backa = delbacka + backa;
          } else {
            chiold = chisqr;
            backa = backa + delbacka;
          }
        }
        backa = oldbacka;
      }
      /* --- done fitting backa --- */
      oldc = c;
      for (i = localmarkers[0]; i <= localmarkers[1]; i++) {
        temp = fabs(*(histloc[spectra] + i) - c - backa * exp(backb * i));
        if (temp > 1)
          temphist[i - localmarkers[0]] = log(temp);
        else
          temphist[i - localmarkers[0]] = 0;
      }
      // printf("a\n");
      xbar = 0;
      ybar = 0;
      xybar = 0;
      xsqrbar = 0;
      for (i = localmarkers[0]; i <= localmarkers[1]; i++) {
        xbar = xbar + (float)(i);
        ybar = ybar + temphist[i - localmarkers[0]];
        xybar = xybar + (float)(i)*temphist[i - localmarkers[0]];
        xsqrbar = xsqrbar + (float)i * i;
      }
      // printf("b\n");
      xbar = xbar / (double)n;
      ybar = ybar / (double)n;
      xybar = xybar / (double)n;
      xsqrbar = xsqrbar / (double)n;

      if ((xsqrbar - xbar * xbar) != 0)
        b = (xybar - xbar * ybar) / (xsqrbar - xbar * xbar);
      else
        b = 0;
      a = exp(ybar - b * xbar);

      chisqr = ExpChiSqr(a, b, c, backa, backb);
      if (chisqr > chiold) {
        chiold = chisqr;
        delc = -delc / 5;
        c = delc + c;
      } else {

        chiold = chisqr;
        c = c + delc;
      }
    }
    c = oldc;
  }

  /* --- done dealing with the linear background --- */

  for (i = localmarkers[0]; i <= localmarkers[1]; i++) {
    temp = fabs(*(histloc[spectra] + i) - c - backa * exp(backb * i));
    if (temp > 1)
      temphist[i - localmarkers[0]] = log(temp);
    else
      temphist[i - localmarkers[0]] = 0;
  }
  // printf("a\n");
  xbar = 0;
  ybar = 0;
  xybar = 0;
  xsqrbar = 0;
  for (i = localmarkers[0]; i <= localmarkers[1]; i++) {
    xbar = xbar + (float)(i);
    ybar = ybar + temphist[i - localmarkers[0]];
    xybar = xybar + (float)(i)*temphist[i - localmarkers[0]];
    xsqrbar = xsqrbar + (float)i * i;
  }
  // printf("b\n");
  xbar = xbar / (double)n;
  ybar = ybar / (double)n;
  xybar = xybar / (double)n;
  xsqrbar = xsqrbar / (double)n;

  if ((xsqrbar - xbar * xbar) != 0)
    b = (xybar - xbar * ybar) / (xsqrbar - xbar * xbar);
  else
    b = 0;
  a = exp(ybar - b * xbar);

  chisqr = ExpChiSqr(a, b, c, backa, backb);

  if ((mode) && (c < 0)) {
    WriteMainText("Background attempted to go below zero.\n");
    c = 0;
    for (i = localmarkers[0]; i <= localmarkers[1]; i++) {
      temp = fabs(*(histloc[spectra] + i) - c - backa * exp(backb * i));
      if (temp > 1)
        temphist[i - localmarkers[0]] = log(temp);
      else
        temphist[i - localmarkers[0]] = 0;
    }
    // printf("a\n");
    xbar = 0;
    ybar = 0;
    xybar = 0;
    xsqrbar = 0;
    for (i = localmarkers[0]; i <= localmarkers[1]; i++) {
      xbar = xbar + (float)(i);
      ybar = ybar + temphist[i - localmarkers[0]];
      xybar = xybar + (float)(i)*temphist[i - localmarkers[0]];
      xsqrbar = xsqrbar + (float)i * i;
    }
    // printf("b\n");
    xbar = xbar / (double)n;
    ybar = ybar / (double)n;
    xybar = xybar / (double)n;
    xsqrbar = xsqrbar / (double)n;

    if ((xsqrbar - xbar * xbar) != 0)
      b = (xybar - xbar * ybar) / (xsqrbar - xbar * xbar);
    else
      b = 0;
    a = exp(ybar - b * xbar);

    chisqr = ExpChiSqr(a, b, c, backa, backb);
  }

  // printf("d\n");

  sprintf(dummystr, "b = %.6f\n", 1 / fabs(b));
  WriteMainText(dummystr);
  sprintf(
      dummystr,
      "Exponential fit: y = %.2f * Exp( %.6f * x) + %.2f + %.2f * Exp(%.6f * x) chisqr = %.1f\n", a,
      b, c, backa, backb, chisqr);
  WriteMainText(dummystr);
  WriteMainText("Exponential Fit in Channels.\n");

  if (globalcalibrationset == 1) {
    if (calibration[spectra][1] != 0) {
      sprintf(dummystr, "Meanlife = %.2f\n", 1 / fabs(b) / calibration[spectra][1]);
      WriteMainText(dummystr);
      sprintf(dummystr, "Halflife = %.2f\n", 1 / fabs(b) / calibration[spectra][1] * log(2));
      WriteMainText(dummystr);
      WriteMainText("Exponential Fit in time.\n");
    } else {
      if (globalcalibration[1] != 0) {
        sprintf(dummystr, "Meanlife = %.2f\n", 1 / fabs(b) * globalcalibration[1]);
        WriteMainText(dummystr);
        sprintf(dummystr, "Halflife = %.2f\n", 1 / fabs(b) * globalcalibration[1] * log(2));
        WriteMainText(dummystr);
        WriteMainText("Exponential Fit in time.\n");
      }
    }
  }

  WriteMainText("Use Q to Quit.\n");

  // printf("e\n");

  //  printf("f\n");
  DrawExpFit(a, b, c, backa, backb);
}

/* ExpChiSqr
 *
 * Determines the ChiSqr given a particular a and b
 */
double ExpChiSqr(double a, double b, double c, double bka, double bkb) {
  int i, j, k;
  double chisqr;
  int localmarkers[2];
  double temp;

  // printf("About to take Chisqr.\n");

  localmarkers[0] = Min(markers[0], markers[1]);
  localmarkers[1] = Max(markers[0], markers[1]);

  chisqr = 0;
  for (i = localmarkers[0]; i <= localmarkers[1]; i++) {
    if (bka == 0) {
      temp = c + a * exp(b * i);
    } else {
      temp = c + a * exp(b * i) + bka * exp(bkb * i);
    }
    if (*(histloc[spectra] + i) != 0)
      chisqr = chisqr + pow((temp - *(histloc[spectra] + i)), 2) / fabs(*(histloc[spectra] + i));
  }
  // printf("found chisqr.\n");
  return (chisqr);
}

/* DrawExpFit
 *
 * Draws the result of an exponential fit
 */
void DrawExpFit(double c, double d, double e, double backa, double backb) {
  int i, j, k;
  int a, b;

  int max;
  int min;

  int bar_height;

  int localmarkers[2];
  long double rangewidth, currentscaling;
  float xchan, nextxchan;

  max = 0;
  min = 0;

  graph = lastgraphclicked;
  spectra = lastspectraclicked;

  if (gtk_graph_has_segments(GTK_GRAPH(graph))) {
    gtk_graph_clear_segments(GTK_GRAPH(graph));
  }
  if (lastfitnumpoints) {
    LastFitClearPoints();
  }
  if (markers[0] > markers[1]) {
    localmarkers[1] = markers[0];
    localmarkers[0] = markers[1];
  } else {
    localmarkers[0] = markers[0];
    localmarkers[1] = markers[1];
  }
  /* --- for this function, we want to display twice the space
     --- between the markers --- */
  i = localmarkers[1] - localmarkers[0];
  localmarkers[0] = localmarkers[0] - i / 2;
  localmarkers[1] = localmarkers[1] + i / 2;
  rangewidth = currentrange[1] - currentrange[0];
  if (rangewidth != 0) {
    currentscaling = (long double)((graph->allocation.width - 100) / rangewidth);
  }

  /* --- Display each value --- */
  /* --- do this by figuring out which pixels on the screen are between the localmarkers[0-1]
     --- and then plot the gaussian between them. --- */
  if (binsize > 1) {
    a = (localmarkers[0] - currentrange[0]) * displayused * column_width + 100;
    b = (localmarkers[1] - currentrange[0]) * displayused * column_width + 100;
  } else {
    a = (localmarkers[0] - currentrange[0] + 0.5) * currentscaling + 100;
    b = (localmarkers[1] - currentrange[0] + 0.5) * currentscaling + 100;
  }

  for (i = a; i <= b; i++) {
    /* --- now we have to convert back to get the right channel numbers --- */
    if (binsize > 1) {
      xchan = (i - 100) / displayused / (float)column_width + (float)currentrange[0];
    } else {
      if ((binsize == 1) && (currentscaling != 0)) {
        xchan = (i - (100)) / currentscaling + currentrange[0] - .5;
      }
    }
    // printf("Xchan: %f ",xchan);

    bar_height = ((c * exp(d * (xchan))) + e + backa * exp(backb * xchan)) * binsize;
    gtk_graph_add_segment(GTK_GRAPH(graph), xchan, bar_height);
  }
  // printf("4\n");
  //  printf("Drew.\n");
  Redraw();
}

/* CloseExpWindow
 *
 * Closes the Exp Window/widget
 */
void CloseExpWindow(GtkWidget *widget, gpointer data) {
  // printf("about to close window.\n");
  if (ExpFitEntry[0]) {
    gtk_widget_destroy(ExpFitEntry[0]);
    ExpFitEntry[0] = NULL;
  }
  if (ExpFitEntry[1]) {
    gtk_widget_destroy(ExpFitEntry[1]);
    ExpFitEntry[1] = NULL;
  }
  if (ExpFitEntry[2]) {
    gtk_widget_destroy(ExpFitEntry[2]);
    ExpFitEntry[2] = NULL;
  }
  gtk_widget_destroy(GTK_WIDGET(data));
  gtk_widget_destroy(widget);
  widget = NULL;
  // printf("Closed window.\n");
}

/* ExpFitWindow
 *
 * Creates the Exp Fit window
 */
void ExpFitWindow() {
  char dummystr[80];
  GtkWidget *localwindow;
  GtkWidget *localbutton;
  GtkWidget *localvbox;
  GtkWidget *localhbox;
  GtkWidget *locallabel;
  GtkWidget *localframe;
  GtkWidget *framevbox;
  GSList *localgroup = NULL;

  expfitmode = 0; // the only toggle

  localwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(localwindow), "Exponential Fit Settings");
  gtk_window_set_modal(GTK_WINDOW(localwindow), TRUE);
  gtk_window_set_position(GTK_WINDOW(localwindow), GTK_WIN_POS_CENTER);

  localvbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(localwindow), localvbox);

  localframe = gtk_frame_new("Linear Background");
  gtk_container_add(GTK_CONTAINER(localvbox), localframe);

  framevbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(localframe), framevbox);

  localbutton = gtk_radio_button_new_with_label(NULL, "Fit Linear Background");
  //--ddc aug11  localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
  localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
  gtk_container_add(GTK_CONTAINER(framevbox), localbutton);
  //--ddc aug11  gtk_signal_connect(GTK_OBJECT(localbutton),"clicked",
  g_signal_connect(GTK_OBJECT(localbutton), "clicked", G_CALLBACK(ExpFitVarLinBackCallback), NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(localbutton), TRUE);

  localhbox = gtk_hbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(framevbox), localhbox);
  localbutton = gtk_radio_button_new_with_label(localgroup, "Use ");
  gtk_container_add(GTK_CONTAINER(localhbox), localbutton);
  //--ddc aug11  gtk_signal_connect(GTK_OBJECT(localbutton),"clicked",
  g_signal_connect(GTK_OBJECT(localbutton), "clicked", G_CALLBACK(ExpFitFixLinBackCallback), NULL);
  ExpFitEntry[0] = gtk_entry_new();
  gtk_container_add(GTK_CONTAINER(localhbox), ExpFitEntry[0]);
  sprintf(dummystr, "%f", background[0]);
  gtk_entry_set_text(GTK_ENTRY(ExpFitEntry[0]), dummystr);
  locallabel = gtk_label_new(" as the linear background.");
  gtk_container_add(GTK_CONTAINER(localhbox), locallabel);

  locallabel = gtk_label_new("Use Exponential Background");
  gtk_container_add(GTK_CONTAINER(localvbox), locallabel);

  localhbox = gtk_hbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(localvbox), localhbox);
  locallabel = gtk_label_new("Counts = ");
  gtk_container_add(GTK_CONTAINER(localhbox), locallabel);
  ExpFitEntry[1] = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(ExpFitEntry[1]), "0");
  gtk_container_add(GTK_CONTAINER(localhbox), ExpFitEntry[1]);
  locallabel = gtk_label_new(" * Exp( ");
  gtk_container_add(GTK_CONTAINER(localhbox), locallabel);
  ExpFitEntry[2] = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(ExpFitEntry[2]), "0");
  gtk_container_add(GTK_CONTAINER(localhbox), ExpFitEntry[2]);
  locallabel = gtk_label_new(" * Channel)");
  gtk_container_add(GTK_CONTAINER(localhbox), locallabel);
  localbutton = gtk_button_new_with_label("Recall");
  gtk_container_add(GTK_CONTAINER(localhbox), localbutton);
  //--ddc aug11  gtk_signal_connect(GTK_OBJECT(localbutton),"clicked",
  g_signal_connect(GTK_OBJECT(localbutton), "clicked", G_CALLBACK(ExpFitRecallExpBack), NULL);

  localbutton = gtk_button_new_with_label("Fit");
  gtk_container_add(GTK_CONTAINER(localvbox), localbutton);
  //--ddc aug11  gtk_signal_connect(GTK_OBJECT(localbutton),"clicked",
  g_signal_connect(GTK_OBJECT(localbutton), "clicked", G_CALLBACK(ExpFitDoIt), NULL);
  //--ddc aug11  gtk_signal_connect_object(GTK_OBJECT(localbutton),"clicked",
  g_signal_connect_swapped(GTK_OBJECT(localbutton), "clicked", G_CALLBACK(CloseExpWindow),
                           GTK_OBJECT(localwindow));
  gtk_widget_grab_focus(localbutton);

  localbutton = gtk_button_new_with_label("Cancel");
  gtk_container_add(GTK_CONTAINER(localvbox), localbutton);
  //--ddc aug11  gtk_signal_connect_object(GTK_OBJECT(localbutton),"clicked",
  g_signal_connect_swapped(GTK_OBJECT(localbutton), "clicked", G_CALLBACK(CloseExpWindow),
                           GTK_OBJECT(localwindow));

  gtk_widget_show_all(localwindow);
}

/* ExpFitVarLinBackCallback
 *
 * Toggles the Exponential Callback to mode 0;
 */
void ExpFitVarLinBackCallback(GtkWidget *widget, gpointer *data) {
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    expfitmode = 0;
  }
}

/* ExpFitFixLinBackCallback
 *
 * Toggles the Exponetial Callback mode to 2;
 */
void ExpFitFixLinBackCallback(GtkWidget *widget, gpointer *data) {
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    expfitmode = 2;
  }
}

/* ExpFitRecallExpBack
 *
 * Recalls the last exponential background used in an exponential fit
 */
void ExpFitRecallExpBack(GtkWidget *widget, gpointer *data) {
  char dummystr[80];

  sprintf(dummystr, "%f", expfitlasta);
  gtk_entry_set_text(GTK_ENTRY(ExpFitEntry[1]), dummystr);
  sprintf(dummystr, "%f", expfitlastb);
  gtk_entry_set_text(GTK_ENTRY(ExpFitEntry[2]), dummystr);
}

/* ExpFitDoIt
 *
 * Gets the appropriate information for the exponential fit and does it
 */
void ExpFitDoIt(GtkWidget *widget, gpointer *data) {
  float a, b, c, d;

  sscanf(gtk_entry_get_text(GTK_ENTRY(ExpFitEntry[0])), "%f", &c);
  sscanf(gtk_entry_get_text(GTK_ENTRY(ExpFitEntry[1])), "%f", &a);
  sscanf(gtk_entry_get_text(GTK_ENTRY(ExpFitEntry[2])), "%f", &b);
  expfitlasta = a;
  expfitlastb = b;
  //  printf("about to fit.\n");
  ExpFit(expfitmode, c, a, b);
  // printf("Fit.\n");
}

/* BackGround
 *
 * using the current polynomial degree
 * and background information
 * returns the value of the background
 * at a particluar channel
 */
float Background(int channel) {
  int i;
  float a;
  a = 0;
  for (i = 0; i <= backgroundpolydeg; i++) {
    a += background[i] * pow(channel, i);
  }
  return (a);
}

/* GetBackgroundCounts
 *
 * Gets the number of counts in the background
 * between (inclusive) two channels
 */
float GetBackgroundCounts(int a, int b) {
  int i;
  float temp;
  temp = 0;
  for (i = a; i <= b; i++) {
    temp += Background(i);
  }
  return (temp);
}

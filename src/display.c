/* Display.c
 *
 * By John Pavan
 *
 * The functions for displaying stuff in gnuscope
 */

#include <gtk/gtk.h>
#include "gtkgraph.h"
#include <math.h>
#include <stdio.h>
#include <gdk/gdkkeysyms.h>
#include "gnuscopefuncs.h"
#include "gnuscopeglobals.h"

//--ddc 3jun08 problems with function prototypes, and fixing calls like
//--ddc strnlen, strnlen(char *, size_t), so strnlen(&dummystr,80), IS
//--ddc incorrect.  With strnlen, there is however, a puzzling refusal
//--ddc to use the header file, string.h!!  So, the warnings persist, even
//--ddc with proper usage.  if the prototype in the man page is included,
//--ddc the warnings end.

//--ddc nov10 gtk2 deprecations..

/* --- Globals --- */
int column_width;
int binsizeforce;
int intype, spectra, plottype;
GtkWidget *graph;
int markers[4], binsize, currentrange[2];
float scaling;
int yscale, ybin, yscaleforce, ybinforce;
long double displayused;
int numspectra;
GtkWidget *box1, *graphsdisplayed[16];
int displaygaussian, displaydoublegaussian;
GtkWidget *mainwindow;
GtkWidget *maintext;
float efficiencycal[4];
GtkWidget *displaytable;
int cursorx;
GtkWidget *channelinfo;
int lastspectraclicked;
GtkWidget *lastgraphclicked;

/* --- Function Declarations --- */

void Refresh();
void DisplayCurrentRange();
void DrawMarkers();
void ZoomOut();
void Redraw();
void WhatCanIDisplay(GtkWidget *text);
void GetDialog(int type);
void ManualMarkerEntry(GtkWidget *widget, GtkWidget *entry);
void ManualMarkerPrompt(GtkWidget *text);
void ExpandPlotEntry(GtkWidget *widget, GtkWidget *entry);
void ExpandPlotPrompt(GtkWidget *text);
void SetYScaleEntry(GtkWidget *widget, GtkWidget *entry);
void SetYScalePrompt(GtkWidget *text);
void MultipleDisplayEntry(GtkWidget *widget, GtkWidget *entry);
void SetYScalePrompt(GtkWidget *text);
void GetCalibrationEntry(GtkWidget *widget, GtkWidget *entry);
void SetCalibration(int i, float a, float b, float c);
void GetCalibrationPrompt(GtkWidget *text);
void GetMessageDialog(const char *message);
void WriteMainText(const char *newtext);
float UseCalibration(float chan);
float UseCalibrationFWHM(float chan);
float UseEfficiency(float counts, float energy);
void SetEfficiencyPrompt(GtkWidget *text);
void SetEfficiencyEntry(GtkWidget *widget, GtkWidget *entry);
void SetMarker(GtkWidget *widget, GdkEventButton *event);
gint UpdateCursorX(GtkWidget *widget, GdkEventMotion *event);
void MultipleToSingleFast();
void DrawColoredLine(GdkDrawable *drawable, const char *colorparse, GdkPoint *points,
                     gint numpoints);

/* --- Functions --- */
/* UpdateCursorX
 *
 * Keeps track of the cursor x-position
 */
gint UpdateCursorX(GtkWidget *widget, GdkEventMotion *event) {
  char dummystr[80];
  gdouble x, y;
  //  gdouble pressure;
  long double currentscaling;
  GdkModifierType state;
  float result;
  int tempsum;
  int i;

  /* --- should probably figure out which plot the cursor is in --- */
  if (numspectra > 1) {
    for (i = 0; i < numspectra; i++) {
      if (widget == graphsdisplayed[i]) {
        spectra = spectradisplayed[i][0];
        graph = graphsdisplayed[i];
      }
    }
  } else {
    spectra = spectradisplayed[0][0];
    graph = graphsdisplayed[0];
  }

  if (histsize[spectra] > 0) {
    if (event->is_hint)
      /*
      //--ddc gtk2 deprecated?
      gdk_input_window_get_pointer(event->window,event->device,
                                   &x,&y,
                                   NULL, NULL, &state);
      */
      //?yeah, this is a pointer?
      /* This the 'preferred' version in a future version ;(
      gdk_display_get_device_state(event->window,event->device,NULL,
                                   &x,&y,&state);
      */
      gdk_display_get_pointer(event->window, NULL, &x, &y, &state);
    else {
      x = event->x;
      y = event->y;
      //      pressure = event->pressure;
      state = event->state;
    }

    currentscaling = (long double)((float)(graph->allocation.width - 100) /
                                   abs(currentrange[1] - currentrange[0]));
    if ((binsize > 1) && (displayused != 0)) {
      result = (float)(((float)x - 100) / (float)displayused) / (float)column_width +
               (float)currentrange[0];
    } else {
      result = (float)((float)x - 100) / currentscaling + (float)currentrange[0];
    }
    if (result > histsize[spectra])
      result = histsize[spectra] - 1;
    if (result < 0)
      result = 0;
    tempsum = 0;
    for (i = 0; i < binsize; i++) {
      if ((result + i) < histsize[spectra])
        tempsum = tempsum + *(histloc[spectra] + i + (int)result);
    }
    if (UseCalibration((int)result) == -1) {
      sprintf(dummystr, "Channel: %d     Counts: %d", (int)result + 1, tempsum);
    } else {
      sprintf(dummystr, "Channel: %d     Energy: %.1f     Counts: %d", (int)result + 1,
              UseCalibration((int)result), tempsum);
    }
    gtk_label_set_text(GTK_LABEL(channelinfo), dummystr);
    cursorx = (int)result;
    return TRUE;
  }
}

/*
 * WhatCanIDisplay
 *
 * writes to the GtkText the currently avalible histograms
 */
void WhatCanIDisplay(GtkWidget *text) {
  int i, j, k;
  char dummystr[80];

  sprintf(dummystr, "Please select a spectrum.\n");

  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  for (i = 0; (strlen(field2[i]) != 0) && (i < 1028); i++) {
    sprintf(dummystr, "Spectrum %3d: %40s\n", (i + 1), field2[i]);
    //    gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
    gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  }
}

/*
 * ManualExpandPlotEntry
 *
 * interpets the entry of the expand plot dialog
 */
void ExpandPlotEntry(GtkWidget *widget, GtkWidget *entry) {
  int i, j, domain;
  char dummystr[80];
  /* --- make sure that we get the right input --- */
  if (sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%d %d", &i, &j) == 2) {
    if (gtk_graph_has_segments(GTK_GRAPH(graph)))
      gtk_graph_clear_segments(GTK_GRAPH(graph));
    /* --- make sure that i and j make sense --- */
    domain = j - i;
    if ((abs(domain) > 1) && (0 <= i && i < histsize[spectra]) &&
        (0 <= j && j < histsize[spectra])) {
      // if ((abs(domain) > 1) && (0 <= i < histsize[spectra]) && (0 <= j < histsize[spectra])) {
      if (domain > 0) {
        currentrange[0] = i;
        currentrange[1] = j;
      } else {
        currentrange[0] = j;
        currentrange[1] = i;
      }
      DisplayCurrentRange();
      DrawMarkers();
    } else {
      sprintf(dummystr, "You cannot display a domain of %d to %d.\n", i, j);
      GetMessageDialog(dummystr);
    }
  }
}

/*
 * ManulaExpandPlotPrompt
 *
 * prompts the user for the parameters for expand plot
 */
void ExpandPlotPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Please enter a range to display.\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * ExpandPlot
 *
 * Expand between the markers
 */
void ExpandPlot() {
  int i, j, k;
  char dummystr[80];

  binsizeforce = 0;
  yscaleforce = 0;
  ybinforce = 0;
  ybin = 0;
  graph = NULL;

  i = markers[0] - markers[1];
  if (abs(i) > 1) {
    currentrange[0] = markers[1];
    currentrange[1] = markers[0];
    if (i <= 0) {
      i = markers[1] - markers[0];
      currentrange[0] = markers[0];
      currentrange[1] = markers[1];
    }
    if (currentrange[0] < 0)
      currentrange[0] = 0;
    if (currentrange[1] > histsize[spectra])
      currentrange[1] = histsize[spectra];
  } else {
    sprintf(dummystr, "You cannot display a range of %d to %d.\n", (markers[0] + 1),
            (markers[1] + 1));
    GetMessageDialog(dummystr);
  }
  scaling = (float)graphsdisplayed[0]->allocation.width / (float)i;
  //  printf("scaling is %f\n",scaling);
  DisplayCurrentRange();
  // DrawMarkers();
}

/*
 * ManualMarkerEntry
 *
 * deals with the entry dialog to manually set a marker
 */
void ManualMarkerEntry(GtkWidget *widget, GtkWidget *entry) {
  int i;
  int test;
  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%d", &i);
  if (test == 1) {
    markers[3] = markers[2];
    markers[2] = markers[1];
    markers[1] = markers[0];
    markers[0] = (i - 1);
  }
  DrawMarkers();
}

/*
 * ManualMarkerPrompt
 *
 * Sets the dialog message to ask for a manual marker set
 */
void ManualMarkerPrompt(GtkWidget *text) {
  char dummystr[80];

  sprintf(dummystr, "Please enter the channel number for the new marker.\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * ZoomOut
 *
 * re-displays the entire plot
 */
void ZoomOut() {
  int i, maxsize;
  currentrange[0] = 0;
  if (numspectra == 1) {
    currentrange[1] = histsize[spectra] - 1;
    if (gtk_graph_has_segments(GTK_GRAPH(graph)))
      gtk_graph_clear_segments(GTK_GRAPH(graph));
    ybinforce = 0;
    ybin = 0;
    yscale = 0;
    yscaleforce = 0;
  } else {
    if (numspectra > 1) {
      maxsize = 0;
      for (i = 0; i < numspectra; i++) {
        if (maxsize < histsize[spectradisplayed[i][0]]) {
          maxsize = histsize[spectradisplayed[i][0]];
        }
        // if (gtk_graph_has_segments(GTK_GRAPH(graphsdisplayed[i])))
        //   gtk_graph_clear_segments(GTK_GRAPH(graphsdisplayed[i]));
      }
      currentrange[1] = maxsize - 1;
    }
  }
  // Refresh();
  DisplayCurrentRange();
}

/*
 * SetYScaleEntry
 *
 * get the yscale entry and set the yscale
 */
void SetYScaleEntry(GtkWidget *widget, GtkWidget *entry) {
  yscaleforce = 1;
  if (gtk_graph_has_segments(GTK_GRAPH(graph)))
    gtk_graph_clear_segments(GTK_GRAPH(graph));
  sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%d", &yscale);
  DisplayCurrentRange();
}

/*
 * SetYScalePrompt
 *
 * Ask the User for a new Y scale
 */
void SetYScalePrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "What would you like the yscale to be?\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * YScaleUp
 *
 * doubles y max
 */
void YScaleUp() {

  if (gtk_graph_has_segments(GTK_GRAPH(graph)))
    gtk_graph_clear_segments(GTK_GRAPH(graph));
  ybinforce = 1;
  ybin = ybin + 1;
  DisplayCurrentRange();
}

/*
 * YScaleDown
 *
 * Halves y max
 */
void YScaleDown() {

  if (gtk_graph_has_segments(GTK_GRAPH(graph)))
    gtk_graph_clear_segments(GTK_GRAPH(graph));
  ybinforce = 1;
  ybin = ybin - 1;
  DisplayCurrentRange();
}

/*
 * ShiftLeft
 *
 * Shifts the portion of the spectrum being displayed to the left
 */
void ShiftLeft() {
  int i, j, k;
  int width;

  if (currentrange[0] > currentrange[1]) {
    i = currentrange[0];
    currentrange[0] = currentrange[1];
    currentrange[0] = i;
  }

  width = currentrange[1] - currentrange[0];

  if (currentrange[0] >= (int)(width / 2)) {
    currentrange[0] = currentrange[0] - (width / 2);
    currentrange[1] = currentrange[1] - (width / 2);
  }
  if (currentrange[0] < (int)(width / 2)) {
    currentrange[0] = 0;
    currentrange[1] = width;
  }
  if (gtk_graph_has_segments(GTK_GRAPH(graph)))
    gtk_graph_clear_segments(GTK_GRAPH(graph));
  DisplayCurrentRange();
}

/*
 * ShiftRight
 *
 * Shifts the portion of the spectrum being displayed to the left
 */
void ShiftRight() {
  int i, j, k;
  int width;

  if (currentrange[0] > currentrange[1]) {
    i = currentrange[0];
    currentrange[0] = currentrange[1];
    currentrange[0] = i;
  }

  width = currentrange[1] - currentrange[0];

  if (currentrange[1] + (width / 2) <= histsize[spectra]) {
    currentrange[0] = currentrange[0] + (width / 2);
    currentrange[1] = currentrange[1] + (width / 2);
  }
  if (currentrange[1] + (width / 2) > histsize[spectra]) {
    currentrange[0] = histsize[spectra] - width - 1;
    currentrange[1] = histsize[spectra] - 1;
  }
  if (gtk_graph_has_segments(GTK_GRAPH(graph)))
    gtk_graph_clear_segments(GTK_GRAPH(graph));
  DisplayCurrentRange();
}

/*
 * PrevSpectra
 *
 * Displays the previous spectrum (if there is one)
 */
void PrevSpectra() {
  int i, j;
  /* --- unfortunately we need to know which graph was clicked on last (and therefore which spectra
   * to update --- */
  j = 0;
  for (i = 0; i < numspectra; i++) {
    if (lastgraphclicked == graphsdisplayed[i])
      j = i;
  }
  graph = graphsdisplayed[j];

  if (spectradisplayed[j][0] > 0) {
    spectradisplayed[j][0] = spectradisplayed[j][0] - 1;
    spectra = spectradisplayed[j][0];
    if (gtk_graph_has_segments(GTK_GRAPH(graph)))
      gtk_graph_clear_segments(GTK_GRAPH(graph));
    DisplayCurrentRange();
  }
}

/*
 * NextSpectra
 *
 * Displays the next spectrum (if there is one)
 */
void NextSpectra() {
  int i, j;
  /* --- unfortunately we need to know which graph was clicked on last (and therefore which spectra
   * to update --- */
  j = 0;
  for (i = 0; i < numspectra; i++) {
    if (lastgraphclicked == graphsdisplayed[i])
      j = i;
  }
  graph = graphsdisplayed[j];

  if (histsize[spectradisplayed[j][0] + 1] > 0) {
    spectradisplayed[j][0] = spectradisplayed[j][0] + 1;
    spectra = spectradisplayed[j][0];
    if (gtk_graph_has_segments(GTK_GRAPH(graph)))
      gtk_graph_clear_segments(GTK_GRAPH(graph));
    DisplayCurrentRange();
  }
}

/*
 * MultipleDisplayEntry
 *
 * Interprets the entry of the multiple display dialog, automatically setting the number to display
 */
void MultipleDisplayEntry(GtkWidget *widget, GtkWidget *entry) {
  int test, i, j;
  char dummystr[80];
  test = 1;

  for (i = 0; i < 16; i++) {
    for (j = 0; j < 10; j++) {
      spectradisplayed[i][j] = -1;
    }
  }

  numspectra = 0;
  numspectra = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)),
                      "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &spectradisplayed[0][0],
                      &spectradisplayed[1][0], &spectradisplayed[2][0], &spectradisplayed[3][0],
                      &spectradisplayed[4][0], &spectradisplayed[5][0], &spectradisplayed[6][0],
                      &spectradisplayed[7][0], &spectradisplayed[8][0], &spectradisplayed[9][0],
                      &spectradisplayed[10][0], &spectradisplayed[11][0], &spectradisplayed[12][0],
                      &spectradisplayed[13][0], &spectradisplayed[14][0], &spectradisplayed[15][0]);
  test = numspectra;
  if (numspectra <= 0)
    numspectra = 1;
  for (i = 0; i < numspectra; i++) {
    spectradisplayed[i][0]--;
    if (spectradisplayed[i][0] < 0)
      spectradisplayed[i][0] = 0;
  }

  //  printf("Entries detected %d.\n",numspectra);
  for (i = 0; i < numspectra; i++) {
    if (histsize[spectradisplayed[i][0]] < 1)
      test = 0;
  }
  if (test) {
    spectra = spectradisplayed[0][0];
    currentrange[0] = 0;
    currentrange[1] = histsize[spectra] - 1;
    DisplayCurrentRange();
  } else {
    sprintf(dummystr, "At least one histogram you want to display is empty.\n");
    GetMessageDialog(dummystr);
  }
}

/* MultipleToSingleFast
 *
 * Expands from the most recently clicked graph of a multiple display to
 * a single display of that graph
 */
void MultipleToSingleFast() {
  numspectra = 1;
  spectradisplayed[0][0] = spectra;
  DisplayCurrentRange();
}

/*
 * MultipleDisplaySameRangeEntry
 *
 * Interprets the entry of multiple display dialog, automatically setting the number to
 * display.  This version does not change the current range settings
 */
void MultipleDisplaySameRangeEntry(GtkWidget *widget, GtkWidget *entry) {
  int test, i, j;
  char dummystr[80];
  test = 1;

  for (i = 0; i < 16; i++) {
    for (j = 0; j < 10; j++) {
      spectradisplayed[i][j] = -1;
    }
  }

  numspectra = 0;
  numspectra = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)),
                      "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &spectradisplayed[0][0],
                      &spectradisplayed[1][0], &spectradisplayed[2][0], &spectradisplayed[3][0],
                      &spectradisplayed[4][0], &spectradisplayed[5][0], &spectradisplayed[6][0],
                      &spectradisplayed[7][0], &spectradisplayed[8][0], &spectradisplayed[9][0],
                      &spectradisplayed[10][0], &spectradisplayed[11][0], &spectradisplayed[12][0],
                      &spectradisplayed[13][0], &spectradisplayed[14][0], &spectradisplayed[15][0]);
  test = numspectra;
  if (numspectra <= 0)
    numspectra = 1;
  for (i = 0; i < numspectra; i++) {
    spectradisplayed[i][0]--;
    if (spectradisplayed[i][0] < 0)
      spectradisplayed[i][0] = 0;
  }
  if (test) {
    spectra = spectradisplayed[0][0];
    DisplayCurrentRange();
  } else {
    sprintf(dummystr, "At least one histogram you want to display is empty.\n");
    GetMessageDialog(dummystr);
  }
}

/*
 * MultipleDisplayPrompt
 *
 * list the spectra to display in the MultipleDisplay dialog
 */
void MultipleDisplayPrompt(GtkWidget *text) {
  int i, j, k;
  char dummystr[80];

  sprintf(dummystr, "Display Multiple Spectra.\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "Enter spectra to display (max 16)\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  for (i = 0; (strlen(field2[i]) != 0) && (i < 1028); i++) {
    sprintf(dummystr, "Spectrum %d: %s\n", i + 1, field2[i]);
    //    gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
    gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  }
}

/*
 * MultiInOneEntry
 *
 * Interprets the entry of multiple display dialog, automatically setting the number to
 * display.  This version does not change the current range settings
 */
void MultiInOneEntry(GtkWidget *widget, GtkWidget *entry) {
  int test, i;
  int which;
  char dummystr[80];
  test = 1;

  which = 0;
  if (numspectra > 1) {
    for (i = 0; i < numspectra; i++) {
      if (spectra = spectradisplayed[i][0])
        which = i;
    }
  }

  for (i = 0; i < 10; i++) {
    spectradisplayed[which][i] = -1;
  }

  test =
      sscanf(gtk_entry_get_text(GTK_ENTRY(entry)),
             "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &spectradisplayed[which][0],
             &spectradisplayed[which][1], &spectradisplayed[which][2], &spectradisplayed[which][3],
             &spectradisplayed[which][4], &spectradisplayed[which][5], &spectradisplayed[which][6],
             &spectradisplayed[which][7], &spectradisplayed[which][8], &spectradisplayed[which][9]);
  for (i = 0; i < test; i++) {
    spectradisplayed[which][i]--;
    if (spectradisplayed[which][i] < 0)
      spectradisplayed[which][i] = 0;
  }
  if (test) {
    spectra = spectradisplayed[which][0];
    DisplayCurrentRange();
  } else {
    sprintf(dummystr, "At least one histogram you want to display is empty.\n");
    GetMessageDialog(dummystr);
  }
}

/*
 * MultiInOnePrompt
 *
 * list the spectra to display in the MultipleDisplay dialog
 */
void MultiInOnePrompt(GtkWidget *text) {
  int i, j, k;
  char dummystr[80];

  sprintf(dummystr, "Display Multiple Spectra.\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "Enter spectra to display in last spectra (max 16)\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  for (i = 0; (strlen(field2[i]) != 0) && (i < 1028); i++) {
    sprintf(dummystr, "Spectrum %d: %s\n", i + 1, field2[i]);
    //    gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
    gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  }
}

/*
 * DisplayCurrentRange
 *
 * Displays the values for the current spectra and range
 */
void DisplayCurrentRange() {
  char dummystr[80];
  int i, j, k, l, y, z;
  int width;
  int cols;
  int column_width, displaywidth;
  GdkPoint points[2];
  long double currentscaling;
  float currentenergy, currentcounts;

  width = abs(currentrange[1] - currentrange[0]);
  currentscaling = (long double)((graphsdisplayed[0]->allocation.width - 100) / width);
  if (numspectra > 16)
    numspectra = 16;

  /* --- we have to consider the case where we want to
     --- display more than one spectrum.
     --- in this case numspectra will be the number of
     --- spectra to display, spectra is the active
     --- spectrum, and spectradisplayed[16] is the
     --- array in which the spectra to display are
     --- stored.
     --- */

  /* --- unfortunately we are going to have to keep track of each
     --- graph seperately in the general memory
     --- the graph pointers will be stored in *graphsdisplayed[16]
     --- *graph will continue to be the location of the active
     --- graph.
  */

  //  for (i = numspectra; i < 16; i++) {
  for (i = 0; i < 16; i++) {
    //--ddc 24may06    if (graphsdisplayed[i] != NULL) {
    if (GTK_IS_WIDGET(graphsdisplayed[i])) {
      gtk_widget_hide(graphsdisplayed[i]);
    }
  }

  for (i = 0; i < numspectra; i++) {
    //--ddc 24may06    if (graphsdisplayed[i] != NULL) {
    if (GTK_IS_WIDGET(graphsdisplayed[i])) {
      gtk_widget_show(graphsdisplayed[i]);
    }
  }

  for (i = 0; i < numspectra; i++) {
    /* --- determine how may spectra are overlayed --- */
    z = 0;
    while ((histsize[spectradisplayed[i][z]] > 0) && (z < 10))
      z++;
    if (z < 1)
      z = 1;
    gtk_graph_set_num_lines(GTK_GRAPH(graphsdisplayed[i]), z);
    if (gtk_graph_has_segments(GTK_GRAPH(graphsdisplayed[i])))
      gtk_graph_clear_segments(GTK_GRAPH(graphsdisplayed[i]));
    for (y = 0; y < z; y++) {
      sprintf(dummystr, "\nSpectra: %d, Title: %s, Channels %d to %d.", spectradisplayed[i][y] + 1,
              field2[spectradisplayed[i][y]], currentrange[0] + 1, currentrange[1] + 1);
      WriteMainText(dummystr);
    }
    /* --- make the graph --- */
    //--ddc dbg apr11    gtk_widget_show(graphsdisplayed[i]);
    for (y = 0; y < z; y++) {
      sprintf(dummystr, "Spec %d: %s", spectradisplayed[i][y] + 1, field2[spectradisplayed[i][y]]);
      gtk_graph_title(GTK_GRAPH(graphsdisplayed[i]), y, dummystr);
    }
    if (globalcalibrationset == 1) {
      if ((calibration[spectradisplayed[i][0]][0] != 0) ||
          (calibration[spectradisplayed[i][0]][1] != 0) ||
          (calibration[spectradisplayed[i][0]][2] != 0)) {
        gtk_graph_set_calibration(
            GTK_GRAPH(graphsdisplayed[i]), calibration[spectradisplayed[i][0]][0],
            calibration[spectradisplayed[i][0]][1], calibration[spectradisplayed[i][0]][2]);
      } else {
        gtk_graph_set_calibration(GTK_GRAPH(graphsdisplayed[i]), globalcalibration[0],
                                  globalcalibration[1], globalcalibration[2]);
      }
      gtk_graph_scale_set_type(GTK_GRAPH(graphsdisplayed[i]), GTK_GRAPH_SCALE_CALIBRATED);
    }
    /* --- put the data in the graph --- */
    gtk_graph_size(GTK_GRAPH(graphsdisplayed[i]), (gint)width);
    switch (plottype) {
    case 1:
    case 4:
      gtk_graph_set_type(GTK_GRAPH(graphsdisplayed[i]), GTK_GRAPH_TYPE_LINEAR);
      break;
    case 2:
      gtk_graph_set_type(GTK_GRAPH(graphsdisplayed[i]), GTK_GRAPH_TYPE_SEMILOG);
      break;
    case 3:
      gtk_graph_set_type(GTK_GRAPH(graphsdisplayed[i]), GTK_GRAPH_TYPE_SEMISQRT);
      break;
    }
    switch (plottype) {
    case 1:
    case 2:
    case 3:
      for (y = 0; y < z; y++) {
        for (j = 0; j < width; j++) {
          if ((j + currentrange[0]) < histsize[spectradisplayed[i][y]]) {
            k = *(histloc[spectradisplayed[i][y]] + j + currentrange[0]);
          } else {
            k = 0;
          }
          gtk_graph_set_value(GTK_GRAPH(graphsdisplayed[i]), y, (gint)j, (gint)k);
        }
      }
    case 4:
      for (y = 0; y < z; y++) {
        for (j = 0; j < width; j++) {
          if ((j + currentrange[0]) < histsize[spectradisplayed[i][y]]) {
            k = *(histloc[spectradisplayed[i][y]] + j + currentrange[0]);
          } else {
            k = 0;
          }
          currentenergy = UseCalibration(j + currentrange[0]);
          if (currentenergy != -1) {
            currentcounts = UseEfficiency((float)k, currentenergy);
            if (currentcounts > -1) {
              k = (int)currentcounts;
            }
          }
          gtk_graph_set_value(GTK_GRAPH(graphsdisplayed[i]), y, (gint)j, (gint)k);
        }
      }
      break;
    default:
      /* --- this should never happen, if it does we set plottype to 1 and go again --- */
      plottype = 1;
      g_warning("Error in Plot type re-set plottype to 1.\n");
    }
    //--ddc apr11 dbg
    //    gtk_widget_show(graphsdisplayed[i]);

    //--ddc gtk2 deprecation    gtk_widget_draw(graphsdisplayed[i],NULL);
    gtk_graph_draw(graphsdisplayed[i], NULL);
  }
  graph = graphsdisplayed[0];
  // spectra = spectradisplayed[0][0];
  /*
  for (i = 0; i<numspectra; i++) {
    //--ddc 24may06    if (graphsdisplayed[i] != NULL) {
    if (GTK_IS_WIDGET(graphsdisplayed[i])) {
      gtk_widget_show(graphsdisplayed[i]);
    }
  }
  */
}

/*
 * SetMarker
 *
 * Detect the x position of the mouse click,
 * set the marker, and draw the marker
 */
void SetMarker(GtkWidget *widget, GdkEventButton *event) {
  GdkPoint points[2];
  float result;
  float width;
  long double currentscaling;
  float currentenergy;
  int tempsum, i, counts;
  char dummystr[80];
  float correctedcounts;
  int whatgraph;

  if (numspectra > 1) {
    for (i = 0; i < numspectra; i++) {
      if (widget == graphsdisplayed[i]) {
        spectra = spectradisplayed[i][0];
        graph = graphsdisplayed[i];
      }
    }
  } else {
    spectra = spectradisplayed[0][0];
    graph = graphsdisplayed[0];
  }

  lastspectraclicked = spectra;
  lastgraphclicked = graph;

  if (currentrange[0] > currentrange[1]) {
    i = currentrange[0];
    currentrange[0] = currentrange[1];
    currentrange[1] = i;
  }
  if (currentrange[0] == 0)
    i = 1;
  else
    i = currentrange[0];

  if ((i > 0) && (currentrange[1] > currentrange[0]))
    width = abs(currentrange[1] - i);
  if ((width != 0) && (graph->allocation.width != 0)) {
    currentscaling = (long double)((graph->allocation.width - 100) / width);
  }

  /* --- update the marker --- */
  /* The updating must be done differently if the number of channels
     displayed is smaller than the allocation width of the window.
     This only happens if the binsize is 1.  Therefore:
  */
  if ((binsize > 1) && (displayused != 0)) {
    result = ((event->x - 100) / displayused / column_width) + (float)currentrange[0];
  } else {
    if ((binsize == 1) && (currentscaling != 0)) {
      result = ((((float)event->x - 100) / currentscaling)) + currentrange[0];
    }
  }
  if (result > histsize[spectra])
    result = histsize[spectra] - 1;

  if (!(markers[0] == (int)result)) {
    markers[3] = markers[2];
    markers[2] = markers[1];
    markers[1] = markers[0];
    markers[0] = (int)result;
  }
  if (markers[0] < 0)
    markers[0] = 0;

  /* --- if there is a calibration, let's use it --- */
  WriteMainText("\n");
  currentenergy = UseCalibration(markers[0]);
  if (currentenergy != -1) {
    sprintf(dummystr, " Energy: %.1f   ", currentenergy);
    WriteMainText(dummystr);
  }

  /* --- why don't we return the number of counts too --- */
  /* Unfortunately, this is also affected by the binsize but that
     isn't a real problem. */
  tempsum = 0;
  if (histsize[spectra] > 0) {
    for (i = 0; i < binsize; i++) {
      if ((i + markers[0]) < histsize[spectra])
        tempsum = tempsum + *(histloc[spectra] + i + markers[0]);
    }
  }

  counts = tempsum;

  correctedcounts = UseEfficiency(counts, currentenergy);
  if (correctedcounts != -1) {
    sprintf(dummystr, " Efficiency corrected counts: %.1f   \n", correctedcounts);
    WriteMainText(dummystr);
  }

  sprintf(dummystr, "  Counts: %d  Channel: %d  ", counts, (markers[0] + 1));
  WriteMainText(dummystr);

  DrawMarkers();
}

/*
 * DrawMarkers
 *
 * Draws the current markers
 */
void DrawMarkers() {
  float result;
  GdkPoint points[2];
  int i, width, j, k;
  long double currentscaling;
  char cparse[30];

  width = abs(currentrange[0] - currentrange[1]);

  if (width != 0) {
    currentscaling = (long double)((graph->allocation.width - 101) / width);
  }

  /* --- Figure out where on the graph the marks are --- */
  /* --- This must be done differently if the number of
     channels displayed is smaller than the allocation
     width of the window.  This only happens if the binsize
     is 1.  Therefore:  */
  for (i = 0; i < 4; i++) {
    if (numspectra == 1) {
      result = (markers[i] - currentrange[0] + .5) * displayused * column_width;
      points[0].x = (int)result + 101;
      points[0].y = 0;
      points[1].x = (int)result + 101;
      points[1].y = graph->allocation.height;
      switch (i) {
      case 0:
        sprintf(cparse, "#000");
        break;
      case 1:
        sprintf(cparse, "#00F");
        break;
      case 2:
        sprintf(cparse, "#0F0");
        break;
      case 3:
        sprintf(cparse, "#F00");
        break;
      default:
        sprintf(cparse, "#000");
      }
      DrawColoredLine(graph->window, cparse, points, 2);
    } else {
      if (numspectra > 1) {
        for (j = 0; j < numspectra; j++) {
          result = (markers[i] - currentrange[0] + 0.5) * displayused * column_width;
          points[0].x = points[1].x = (int)result + 101;
          points[0].y = 0;
          points[1].y = graphsdisplayed[j]->allocation.height;

          switch (i) {
          case 0:
            sprintf(cparse, "#000");
            break;
          case 1:
            sprintf(cparse, "#00F");
            break;
          case 2:
            sprintf(cparse, "#0F0");
            break;
          case 3:
            sprintf(cparse, "#F00");
            break;
          default:
            sprintf(cparse, "#000");
          }
          DrawColoredLine(graphsdisplayed[j]->window, cparse, points, 2);
        }
      }
    }
  }
}

/*
 * Refresh
 *
 * Blanks the display area
 */
void Refresh() {
  gdk_draw_rectangle(graph->window, graph->style->white_gc, TRUE, 0, 0, graph->allocation.width,
                     graph->allocation.height);
}

/*
 * Redraw
 *
 * Simply re-draw the plot and the markers
 */
void Redraw() {
  int i;

  //--ddc jun11 deprecations of gtk_widget_draw.. must explicitly call

  if (numspectra == 1) {

    //--ddc jun11    gtk_widget_draw(graph,NULL);
    gtk_graph_draw(graph, NULL);
  } else {
    if (numspectra > 1) {
      for (i = 0; i < numspectra; i++) {
        //--ddc jun11  	gtk_widget_draw(graphsdisplayed[i],NULL);
        gtk_graph_draw(graphsdisplayed[i], NULL);
      }
    }
  }
  // DisplayCurrentRange();
}

/*
 * GetCalibrationPrompt
 *
 * Prompts the dialog to ask for a calibration
 */
void GetCalibrationPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Enter energy calibration\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * GetCalibrationEntry
 *
 * interprets the get calibration dialog entry and then sets the calibration
 */
void GetCalibrationEntry(GtkWidget *widget, GtkWidget *entry) {
  int i, test;
  float input[3];

  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%f %f %f", &input[0], &input[1], &input[2]);
  if (test == 3) {
    SetCalibration(spectra, input[0], input[1], input[2]);
    //    for (i = 0; i < 3; i++) {
    //     calibration[spectra][i] = input[i];
    // }
    // if (globalcalibrationset == 0) {
    //  for (i=0; i < 3 ; i++ ) {
    //	globalcalibration[i] = calibration[spectra][i];
    //  }
    //  globalcalibrationset = 1;
    // }
  }
}

/* SetCalibration
 *
 * Sets the calibration of histogram i
 */

//--ddc 3jun08 Ok, while following up on function protyping problems,
// I noticed that 'c' coefficient not set... :\

void SetCalibration(int i, float a, float b, float c) {
  calibration[i][0] = a;
  calibration[i][1] = b;
  //--ddc 3jun08  calibration[i][2];
  calibration[i][2] = c;
  if (globalcalibrationset == 0) {
    globalcalibration[0] = a;
    globalcalibration[1] = b;
    globalcalibration[2] = c;
    globalcalibrationset = 1;
  }
}

/*
 * ActiveDown
 *
 * In multiplot mode changes the active plot to the one
 * bellow the currently active plot
 */
void ActiveDown() {
  int i, j, k;
  char dummystr[80];
  j = -1;
  /* --- make sure we are displaying more than one plot --- */
  if (numspectra > 1) {
    /* --- make sure that we are not already on the bottom plot --- */
    if (graph != graphsdisplayed[(numspectra - 1)]) {
      /* --- figure out which spectra is currently being displayed --- */
      for (i = 0; i < numspectra; i++) {
        if (graph == graphsdisplayed[i]) {
          j = i;
        }
      }
      if (j == -1) {
        GetMessageDialog("The program could not determine which spectrum was currently active.\n");
      } else {
        spectra = spectradisplayed[(j + 1)][0];
        graph = graphsdisplayed[(j + 1)];
        sprintf(dummystr, "\nSpectrum %d is active.", spectra);
        WriteMainText(dummystr);
      }
    } else {
      GetMessageDialog("The bottom-most spectrum is already active.\n");
    }
  } else {
    GetMessageDialog("There is only one spectrum displayed.\n");
  }
}

/*
 * ActiveUp
 *
 * In multiplot mode changes the active plot to the one
 * above the currently active plot
 */
void ActiveUp() {
  int i, j, k;
  char dummystr[80];
  j = -1;
  /* --- make sure we are displaying more than one plot --- */
  if (numspectra > 1) {
    /* --- make sure we are not already on the top plot --- */
    if (graph != graphsdisplayed[0]) {
      /* --- figure out which spectra is currently being displayed --- */
      for (i = 0; i < numspectra; i++) {
        if (graph == graphsdisplayed[i]) {
          j = i;
        }
      }
      if (j == -1) {
        GetMessageDialog("The program got confused and could not figure out which spectrum was "
                         "currently active.\n");
      } else {
        spectra = spectradisplayed[(j - 1)][0];
        graph = graphsdisplayed[(j - 1)];
        sprintf(dummystr, "\nSpectrum %d is active.", spectra);
        WriteMainText(dummystr);
      }
    } else {
      GetMessageDialog("The top-most spectrum is already active.\n");
    }
  } else {
    GetMessageDialog("There is only one spectrum displayed.\n");
  }
}

/*
 * UseCalibartion
 *
 * returns -1 if no calibration is set
 * otherwise returns the energy calibration for a particluar channel
 * (adding one is done inside this loop, so don't add one to the marker you feed it)
 */
float UseCalibration(float chan) {
  float energy;
  if (globalcalibrationset == 1) {
    if ((calibration[spectra][0] != 0) || (calibration[spectra][1] != 0) ||
        (calibration[spectra][2] != 0)) {
      energy = calibration[spectra][0] + calibration[spectra][1] * (chan + 1) +
               calibration[spectra][2] * (chan + 1) * (chan + 1);
      //--ddc 3jun08 dbg calibration[spectra][2] * calibration[spectra][2] * (chan + 1);

    } else {
      energy = globalcalibration[0] + globalcalibration[1] * (chan + 1) +
               globalcalibration[2] * (chan + 1) * (chan + 1);
      //--ddc 3jun08 dbg globalcalibration[2] * globalcalibration[2] * (chan + 1);
    }
    return (energy);
  } else {
    return (-1);
  }
}

/*
 * UseCalibartionFWHM
 *
 * returns -1 if no calibration is set
 * otherwise returns the energy calibration for a particluar channel
 * (adding one is done inside this loop, so don't add one to the marker you feed it)
 */
float UseCalibrationFWHM(float chan) {
  float energy;
  if (globalcalibrationset == 1) {
    if ((calibration[spectra][0] != 0) || (calibration[spectra][1] != 0) ||
        (calibration[spectra][2] != 0)) {
      energy =
          calibration[spectra][1] * (chan + 1) + calibration[spectra][2] * (chan + 1) * (chan + 1);
      //--ddc 3jun08 dbg calibration[spectra][2] * calibration[spectra][2] * (chan + 1);
    } else {
      energy = globalcalibration[1] * (chan + 1) + globalcalibration[2] * (chan + 1) * (chan + 1);
      //--ddc 3jun08 dbg globalcalibration[2] * globalcalibration[2] * (chan + 1);
    }
    return (energy);
  } else {
    return (-1);
  }
}

/*
 * UseEfficiency
 *
 * returns -1 if no calibration set
 * Otherwise returns the efficency corrected counts for a particular channel
 */
float UseEfficiency(float counts, float energy) {
  float newcounts;
  float a, b, c;
  char dummystr[80];

  if (energy > 100) {
    if ((efficiencycal[0] != 0) || (efficiencycal[1] != 0) || (efficiencycal[2] != 0) ||
        (efficiencycal[3] != 0)) {
      a = (log(energy) / 2.302585);

      b = (efficiencycal[0] - (efficiencycal[1] * a) + (efficiencycal[2] * a * a) -
           efficiencycal[3] / energy / energy);
      c = exp(b * 2.302585);
      newcounts = ((float)counts / (float)c);
      return (newcounts);
    } else {
      return (-1);
    }
  } else {
    return (-1);
  }
}

/*
 * SetEfficiencyPrompt
 *
 * Prompts for the set efficiency dialog
 */
void SetEfficiencyPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Set Efficiency\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "Please enter the efficiency coefficients.\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "A B C D.\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "newcounts = counts * 10^(A - B * Log_10(E) + C * Log(2E) + D/E^2)\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * SetEfficiencyEntry
 *
 * gets the efficiency from the dialog
 */
void SetEfficiencyEntry(GtkWidget *widget, GtkWidget *entry) {
  int test;
  float a, b, c, d;

  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%f %f %f %f", &a, &b, &c, &d);
  if (test == 4) {
    efficiencycal[0] = a;
    efficiencycal[1] = b;
    efficiencycal[2] = c;
    efficiencycal[3] = d;
  }
}

/* DrawColoredLine
 *
 * Draws a colored line
 */
void DrawColoredLine(GdkDrawable *drawable, const char *colorparse, GdkPoint *points,
                     gint numpoints) {
  GdkGC *gc;
  GdkColor color;
  gboolean bool;

  gc = gdk_gc_new(drawable);
  bool = gdk_color_parse(colorparse, &color);
  gdk_color_alloc(gtk_widget_get_default_colormap(), &color);
  gdk_gc_set_foreground((GdkGC *)gc, &color);
  gdk_draw_lines(drawable, gc, points, numpoints);
  //--ddc jan11 gtk deprecations..
  //  gdk_gc_destroy(gc);
  g_object_unref(gc);
}

/* gainshift.c
 *
 * by John Pavan
 *
 * A set of functions for collecting the nessasary parameters for executing
 * a gainshift, and executing the gainshift
 */

/* --- incudes --- */

// #include <gtk/gtk.h>
#include "math.h"
// #include "stdio.h"
#include "gnuscopefuncs.h"
#include "gnuscopeglobals.h"
// #include "menus.h"
/* --- structs --- */

/* --- globals --- */

int spectra;
int currentrange[2];
float globalcalibration[3]; //,calibration[254][3];
GtkWidget *gainshiftentry1, *gainshiftentry3, *gainshiftentry5;
GtkWidget *gainshiftentry2, *gainshiftentry4, *gainshiftentry6;
GtkWidget *gainshiftwindow = NULL;
// int histid[254],histsize[254],*histloc[254],histpointer;

/* --- function declarations --- */

void Gainshift(int localspectrum, float oldconst, float oldlin, float oldquad, float newcalib,
               int newchannels);
void CreateGainShiftWindow();
void HideGainshiftWindow();
void HideWidget(GtkWidget *widget, gpointer *data);
void ObliterateHistogram(int delhist);
void GainshiftCallback(GtkWidget *widget, gpointer *data);

/* --- functions --- */

/* HideGainshiftWindow
 *
 * Hides the window for getting the gainshfit information
 */
void HideGainshiftWindow() {
  g_return_if_fail(gainshiftwindow != NULL);
  gtk_widget_hide(gainshiftwindow);
}

/* CreateGainShiftWindow
 *
 * Creates a window for getting the gainshift parameters
 */
void CreateGainShiftWindow() {
  GtkWidget *localbutton;
  GtkWidget *localvbox;
  GtkWidget *localhbox;
  GtkWidget *localframe;
  GtkWidget *framevbox;
  GtkWidget *locallabel;
  char dummystr[80];

  /* --- idiot check --- */
  g_return_if_fail(histsize[0] != 0);

  /* --- create a new gainshift window if it doesn't already exist --- */
  if (gainshiftwindow == NULL) {
    gainshiftwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gainshiftwindow), "Gainshift");
    gtk_window_set_modal(GTK_WINDOW(gainshiftwindow), TRUE);
    //--ddc aug11    gtk_signal_connect(GTK_OBJECT(gainshiftwindow),"delete_event",
    g_signal_connect(GTK_OBJECT(gainshiftwindow), "delete_event", G_CALLBACK(HideGainshiftWindow),
                     NULL);
    /* --- create a vbox to put stuff in --- */
    localvbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(gainshiftwindow), localvbox);
    /* --- now let's create each of the labels and entries --- */
    /* --- get the information about the spectrum  --- */
    localhbox = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(localvbox), localhbox);
    locallabel = gtk_label_new("Spectrum");
    gtk_container_add(GTK_CONTAINER(localhbox), locallabel);
    gainshiftentry1 = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(localhbox), gainshiftentry1);
    /* --- now lets get the information about the prior calibration --- */
    localframe = gtk_frame_new("Current Energy Calibration");
    gtk_container_add(GTK_CONTAINER(localvbox), localframe);
    framevbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(localframe), framevbox);
    /* --- constant --- */
    localhbox = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(framevbox), localhbox);
    locallabel = gtk_label_new("Constant");
    gtk_container_add(GTK_CONTAINER(localhbox), locallabel);
    gainshiftentry2 = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(localhbox), gainshiftentry2);
    /* --- linear --- */
    localhbox = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(framevbox), localhbox);
    locallabel = gtk_label_new("Linear");
    gtk_container_add(GTK_CONTAINER(localhbox), locallabel);
    gainshiftentry3 = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(localhbox), gainshiftentry3);
    /* --- quadratic --- */
    localhbox = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(framevbox), localhbox);
    locallabel = gtk_label_new("Quadratic");
    gtk_container_add(GTK_CONTAINER(localhbox), locallabel);
    gainshiftentry4 = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(localhbox), gainshiftentry4);
    /* --- now lets get the new calibration --- */
    localhbox = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(localvbox), localhbox);
    locallabel = gtk_label_new("New Calibration");
    gtk_container_add(GTK_CONTAINER(localhbox), locallabel);
    gainshiftentry5 = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(localhbox), gainshiftentry5);
    /* --- now lets get the number of channels for the output --- */
    localhbox = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(localvbox), localhbox);
    locallabel = gtk_label_new("New Channels");
    gtk_container_add(GTK_CONTAINER(localhbox), locallabel);
    gainshiftentry6 = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(localhbox), gainshiftentry6);
    /* --- now lets create a button for execution --- */
    localbutton = gtk_button_new_with_label("Gainshift");
    gtk_container_add(GTK_CONTAINER(localvbox), localbutton);
    //--ddc aug11    gtk_signal_connect(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect(GTK_OBJECT(localbutton), "clicked", G_CALLBACK(GainshiftCallback), NULL);
    //--ddc aug11    gtk_signal_connect_object(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect_swapped(GTK_OBJECT(localbutton), "clicked", G_CALLBACK(HideWidget),
                             GTK_OBJECT(gainshiftwindow));
    /* --- now lets create a button for canceling --- */
    localbutton = gtk_button_new_with_label("Cancel");
    gtk_container_add(GTK_CONTAINER(localvbox), localbutton);
    //--ddc aug11    gtk_signal_connect_object(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect_swapped(GTK_OBJECT(localbutton), "clicked", G_CALLBACK(HideWidget),
                             GTK_OBJECT(gainshiftwindow));
  }
  /* --- first we need to reset all the entries --- */
  if ((spectra >= 0) && (spectra < GetLastSpectrum()))
    sprintf(dummystr, "%d", spectra + 1);
  else
    sprintf(dummystr, "1");
  gtk_entry_set_text(GTK_ENTRY(gainshiftentry1), dummystr);
  if (calibration[spectra][0])
    sprintf(dummystr, "%f", calibration[spectra][0]);
  else if (globalcalibration[0])
    sprintf(dummystr, "%f", globalcalibration[0]);
  else
    sprintf(dummystr, "0");
  gtk_entry_set_text(GTK_ENTRY(gainshiftentry2), dummystr);
  if (calibration[spectra][1])
    sprintf(dummystr, "%f", calibration[spectra][1]);
  else if (globalcalibration[1])
    sprintf(dummystr, "%f", globalcalibration[1]);
  else
    sprintf(dummystr, "1");
  gtk_entry_set_text(GTK_ENTRY(gainshiftentry3), dummystr);
  if (calibration[spectra][2])
    sprintf(dummystr, "%f", calibration[spectra][2]);
  else if (globalcalibration[2])
    sprintf(dummystr, "%f", globalcalibration[2]);
  else
    sprintf(dummystr, "0");
  gtk_entry_set_text(GTK_ENTRY(gainshiftentry4), dummystr);
  gtk_entry_set_text(GTK_ENTRY(gainshiftentry5), "1");
  if (histsize[spectra] > 0)
    sprintf(dummystr, "%d", histsize[spectra]);
  else
    sprintf(dummystr, "3000");
  gtk_entry_set_text(GTK_ENTRY(gainshiftentry6), dummystr);
  /* --- now we show the window --- */
  gtk_widget_show_all(gainshiftwindow);
}

/* GainshiftCallback
 *
 * Gets the parameters from the entries and if all are seen
 * performs the gainshift function
 */
void GainshiftCallback(GtkWidget *widget, gpointer *data) {
  int localspectrum, newchans;
  float con, lin, quad, newlin;

  if (sscanf(gtk_entry_get_text(GTK_ENTRY(gainshiftentry1)), "%d", &localspectrum)) {
    if (!(sscanf(gtk_entry_get_text(GTK_ENTRY(gainshiftentry2)), "%f", &con)))
      con = 0;
    if (sscanf(gtk_entry_get_text(GTK_ENTRY(gainshiftentry3)), "%f", &lin)) {
      if (!(sscanf(gtk_entry_get_text(GTK_ENTRY(gainshiftentry4)), "%f", &quad)))
        quad = 0;
      if (sscanf(gtk_entry_get_text(GTK_ENTRY(gainshiftentry5)), "%f", &newlin))
        if (sscanf(gtk_entry_get_text(GTK_ENTRY(gainshiftentry6)), "%d", &newchans))
          /* --- check to make sure the parameters are such so that the program
             --- won't crash --- */
          if ((histsize[localspectrum - 1] > 1) && (newchans > 1))
            Gainshift(localspectrum - 1, con, lin, quad, newlin, newchans);
          else
            WriteMainText("Error in spectrum or new channel parameters.\n");
    }
  }
}

/*
 * Gainshift
 *
 * Changes the calibration from old values to new ones of spectra
 */
void Gainshift(int localspectrum, float oldconst, float oldlin, float oldquad, float newcalib,
               int newchannels) {
  int i, j, k;
  //--ddc 19jun09 numerous changes involving precision and roundoff
  //--ddc 19jun09 all 'float' variables and casts to float changed to double
  //--ddc 19jun09 the biggest contributor to improved spectra was changing
  //--ddc 19jun09 temporary array (temphist) from integer to double.  The
  //--ddc 19jun09 next biggest came with adding 0.5 to all temphist
  //--ddc 19jun09 when casting to an integer.
  // --ddc 19jun09 int *temphist;
  double *temphist;
  double oldm, oldmsqr, oldp, oldpsqr;
  double anewm, anewp;
  int newm, newp, counts;
  double frac;
  double xi, tm, tp;

  //--ddc 19jun09 add these fields so titles can be preserved after gainshift
  char tempfield1[40], tempfield2[40], tempfield3[40];

  //--ddc 8jun06 core dumping for going out of array boundaries.  JP seems to have a problem
  //deciding where
  //--ddc 8jun06 his histograms begin and end.  I'm going with the general thinking that they start
  //at zero, and so there
  //--ddc 8jun06 is no channel "maxchannelnumber".

  /* --- make sure we arn't doing something really really dumb --- */
  g_return_if_fail(histsize[localspectrum] > 0);

  /* --- Allocate memory for the temphist --- */
  //--ddc 19jun09  temphist = (int *) malloc(sizeof(int) * newchannels);
  temphist = (double *)malloc(sizeof(double) * newchannels);

  for (i = 0; i < newchannels; i++)
    temphist[i] = 0;

  for (i = 0; i < (histsize[localspectrum]); i++) {
    //--ddc 19jun09    oldm = (double) (i - 0.5);
    oldm = (double)i - 0.5;
    oldmsqr = oldm * oldm;
    oldp = (double)(oldm + 1);
    oldpsqr = oldp * oldp;
    anewm = (double)((oldconst + oldlin * oldm + oldquad * oldmsqr) / newcalib);
    newm = (int)anewm;
    if ((anewm - newm) > 0.5) {
      newm = newm + 1;
    }
    anewp = (oldconst + oldlin * oldp + oldquad * oldpsqr) / newcalib;
    newp = anewp;
    if ((anewp - newp) > 0.5) {
      newp = newp + 1;
    }
    if (newm > 0) {
      //--ddc 8jun06      if (newp <= 8192) {
      if (newp < newchannels) {
        frac = 1.0 / (anewp - anewm);
        counts = *(histloc[localspectrum] + i);
        //--ddc 8jun06	for (j = newm; j <= newp; j++) {
        //--ddc 16jun09 back this change out... for (j = newm; j < newp; j++) {
        for (j = newm; j <= newp; j++) {
          // printf("%f ",counts);
          xi = (double)j;
          tm = anewm;
          if (tm < (xi - 0.5))
            tm = (xi - 0.5);
          tp = anewp;
          if (tp > (xi + 0.5))
            tp = (xi + 0.5);
          temphist[j] = (temphist[j] + counts * frac * (tp - tm));
        }
      }
    }
  }

  if (newchannels < 0) {
    newchannels = histsize[localspectrum];
  }

  //--ddc 19jun09 get text fields...
  sprintf(tempfield1, field1[localspectrum]);
  sprintf(tempfield2, field2[localspectrum]);
  sprintf(tempfield3, field3[localspectrum]);

  ObliterateHistogram(localspectrum);
  histsize[localspectrum] = newchannels;
  histid[localspectrum] = localspectrum;
  histloc[localspectrum] = (int *)malloc(newchannels * sizeof(int));

  for (k = 0; k < histsize[localspectrum]; k++) {
    //--ddc 19jun09 compensate for bias on roundoff
    //--ddc 19jun09 *(histloc[localspectrum] + k) = (int) temphist[k];
    *(histloc[localspectrum] + k) = temphist[k] + 0.5;
  }

  /* --- make the calibrations for the new spectrum make sense --- */
  calibration[localspectrum][0] = 0;
  calibration[localspectrum][2] = 0;
  calibration[localspectrum][1] = newcalib;

  //--ddc 19jun09 put the text fields back
  sprintf(field1[localspectrum], tempfield1);
  sprintf(field2[localspectrum], tempfield2);
  sprintf(field3[localspectrum], tempfield3);

  currentrange[0] = 0;
  currentrange[1] = newchannels;
  spectra = localspectrum;
  DisplayCurrentRange();
  DrawMarkers();
}

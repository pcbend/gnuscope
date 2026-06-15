/* manipulate.c
 *
 * by John Pavan
 *
 * functions for the manipulation of 1D histograms for use with Gnuscope
 */

#include <gtk/gtk.h>
#include "gtkgraph.h"
#include "gnuscopefuncs.h"
#include "gnuscopeglobals.h"

//--ddc nov10 gtk2 deprecations...

/* --- global variables --- */

// int histid[254],histsize[254],*histloc[254],histpointer;
// char field1[254][40],field2[254][40],field3[254][40];
int intype, spectra, plottype;
GtkWidget *graph;
int markers[4], binsize, offset;
float globalcalibration[3]; //,calibration[254][3];
// int background;
int backgroundploydeg;
int oldscope;
int currentrange[2], offset;
void CopyHistogram(int a, int b);
GtkWidget *maintext;
// int spectradisplayed[16];
GtkWidget *manipulatetitlewindow;
GtkWidget *manipulatetitleentry1;
GtkWidget *manipulatetitleentry2;

/* --- function declarations --- */

void HideWidget(GtkWidget *widget, gpointer *data);
int GetLastSpectrum();
void ObliterateHistogram(int delhist);
void AddSpectraEntry(GtkWidget *widget, GtkWidget *entry);
void AddSpectraPrompt(GtkWidget *text);
void SubtractSpectraEntry(GtkWidget *widget, GtkWidget *entry);
void SubtractSpectraPrompt(GtkWidget *text);
void MoveSpectraEntry(GtkWidget *widget, GtkWidget *entry);
void MoveSpectraPrompt(GtkWidget *text);
void NormalizeSpectraEntry(GtkWidget *widget, GtkWidget *entry);
void NormalizeSpectraPrompt(GtkWidget *text);

void CompressSpectraEntry(GtkWidget *widget, GtkWidget *entry);
void CompressSpectraPrompt(GtkWidget *text);
void ClearSpectraEntry(GtkWidget *widget, GtkWidget *entry);
void ClearSpectraPrompt(GtkWidget *text);
void GainshiftSpectraEntry(GtkWidget *widget, GtkWidget *entry);
void GainshiftSpectraPrompt(GtkWidget *text);
void GetMessageDialog(const char *message);
void WriteMainText(const char *newtext);
void CreateTitleWindow();
void Gainshift(int localspectrum, float oldconst, float oldlin, float oldquad, float newcalib,
               int newchannels);
void CloseWidget(GtkWidget *widget, gpointer data);
void SetSpectra(GtkWidget *widget, GtkWidget *entry);
void ManipulateTitleCallback(GtkWidget *widget, gpointer *data);

/* --- functions --- */

/* ManipulateTitleCallback
 *
 * the callback function for the manipulate title window
 */
void ManipulateTitleCallback(GtkWidget *widget, gpointer *data) {
  int a;

  if (sscanf(gtk_entry_get_text(GTK_ENTRY(manipulatetitleentry1)), "%d", &a) == 1) {
    /* --- kinda need to know if we have a spectrum we are supposed to title --- */
    g_return_if_fail((a > 0) && (a <= GetLastSpectrum() + 1));
    /* --- now we know that we are looking at a valid spectrum --- */
    sprintf(field2[a - 1], gtk_entry_get_text(GTK_ENTRY(manipulatetitleentry2)));
    DisplayCurrentRange();
    DrawMarkers();
  }
}

/* CreateTitleWindow
 *
 * Creates a special window for manipulating the title
 */
void CreateTitleWindow() {
  GtkWidget *localbutton;
  GtkWidget *localvbox;
  GtkWidget *localhbox;
  GtkWidget *locallabel;
  char dummystr[80];

  /* --- create the window --- */
  //--ddc 24may06  if (manipulatetitlewindow == NULL) {
  if (!GTK_IS_WIDGET(manipulatetitlewindow)) {
    manipulatetitlewindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(manipulatetitlewindow), "Change Title");
    gtk_window_set_modal(GTK_WINDOW(manipulatetitlewindow), TRUE);
    //--ddc aug11    gtk_signal_connect(GTK_OBJECT(manipulatetitlewindow),"delete_event",
    g_signal_connect(GTK_OBJECT(manipulatetitlewindow), "delete_event", G_CALLBACK(HideWidget),
                     NULL);

    /* --- ok, now let's make a vbox to put this stuff in --- */
    localvbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(manipulatetitlewindow), localvbox);
    /* --- now let's make the contents --- */
    localhbox = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(localvbox), localhbox);
    locallabel = gtk_label_new("Please enter the new title for spectrum");
    gtk_container_add(GTK_CONTAINER(localhbox), locallabel);
    manipulatetitleentry1 = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(localhbox), manipulatetitleentry1);
    manipulatetitleentry2 = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(localvbox), manipulatetitleentry2);
    /* --- let's have this grab the focus and allow a hard activate to be used as
       --- the go function --- */
    gtk_widget_grab_focus(manipulatetitleentry2);
    //--ddc aug11    gtk_signal_connect(GTK_OBJECT(manipulatetitleentry2),"activate",
    g_signal_connect(GTK_OBJECT(manipulatetitleentry2), "activate",
                     G_CALLBACK(ManipulateTitleCallback), NULL);
    //--ddc aug11    gtk_signal_connect_object(GTK_OBJECT(manipulatetitleentry2),"activate",
    g_signal_connect_swapped(GTK_OBJECT(manipulatetitleentry2), "activate", G_CALLBACK(HideWidget),
                             (GtkObject *)manipulatetitlewindow);
    /* --- butons to go --- */
    localbutton = gtk_button_new_with_label("Set Title");
    gtk_container_add(GTK_CONTAINER(localvbox), localbutton);
    //--ddc aug11    gtk_signal_connect(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect(GTK_OBJECT(localbutton), "clicked", G_CALLBACK(ManipulateTitleCallback), NULL);
    //--ddc aug11   gtk_signal_connect_object(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect_swapped(GTK_OBJECT(localbutton), "clicked", G_CALLBACK(HideWidget),
                             (GtkObject *)manipulatetitlewindow);
    /* --- button to cancel --- */
    localbutton = gtk_button_new_with_label("Cancel");
    gtk_container_add(GTK_CONTAINER(localvbox), localbutton);
    //--ddc aug11    gtk_signal_connect_object(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect_swapped(GTK_OBJECT(localbutton), "clicked", G_CALLBACK(HideWidget),
                             (GtkObject *)manipulatetitlewindow);
  }

  sprintf(dummystr, "%d", spectra + 1);
  gtk_entry_set_text(GTK_ENTRY(manipulatetitleentry1), dummystr);
  sprintf(dummystr, "%s", field2[spectra]);
  gtk_entry_set_text(GTK_ENTRY(manipulatetitleentry2), dummystr);
  /* --- show the window --- */
  gtk_widget_show_all(manipulatetitlewindow);
}

/*
 * AddSpectraEntry
 *
 * Interprets the Add spectra entry from dialog
 */
void AddSpectraEntry(GtkWidget *widget, GtkWidget *entry) {
  int a, b, c, i, j, k;
  int input[20];
  int test;
  int lastspectra;
  int newsize, newmemsize;
  char dummystr[80];

  //  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)),"%d %d %d",&a,&b,&c);
  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)),
                "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &input[0], &input[1],
                &input[2], &input[3], &input[4], &input[5], &input[6], &input[7], &input[8],
                &input[9], &input[10], &input[11], &input[12], &input[13], &input[14], &input[15],
                &input[16], &input[17], &input[18], &input[19]);
  if (test >= 3) { // make sure we got a good input
    for (i = 0; i < test; i++) {
      input[i] = input[i] - 1;
    }
    c = input[test - 1];
    /* --- make sure everything we are going to add actually exists --- */
    for (i = 0; i < test - 1; i++) {
      if (histsize[input[i]] <= 0)
        GetMessageDialog("One (or more) of the spectra to sum is empty.");
      g_return_if_fail(histsize[input[i]] > 0);
    }

    /* --- make sure that the location to write is at
       most 1 more than the current last spectra --- */
    lastspectra = GetLastSpectrum();
    if (c > (lastspectra + 1)) {
      c = lastspectra + 1;
      sprintf(dummystr, "Destination spectrum changed to %d.\n", c);
      WriteMainText(dummystr);
    }
    /* --- make sure that we arn't going to overwrite shared memory --- */
    if (c >= online) {
      /* --- make sure we arn't going to write to a memory leak --- */
      if (c < 254) {
        /* --- get the new size for the spectra --- */
        newsize = 0;
        for (i = 0; i < test - 1; i++) {
          if (histsize[i] > newsize)
            newsize = histsize[i];
        }
        newmemsize = sizeof(int) * newsize;
        //        printf("Attempting to allocate %d bytes.\n", newmemsize);
        ObliterateHistogram(c);
        histloc[c] = (int *)malloc(newmemsize);

        /* --- now let's add them --- */

        a = input[0];
        histsize[c] = newsize;
        histid[c] = c;
        for (i = 0; i < 40; i++) {
          field1[c][i] = field1[a][i];
          field2[c][i] = field2[a][i];
          field3[c][i] = field3[a][i];
        }
        for (i = 0; i < newsize; i++) {
          *(histloc[c] + i) = 0;
        }
        for (i = 0; i < test - 1; i++) {
          for (j = 0; j < histsize[input[i]]; j++) {
            *(histloc[c] + j) = *(histloc[c] + j) + *(histloc[input[i]] + j);
          }
        }
      }
      /* --- let's draw the new spectrum --- */
      spectra = c;
      spectradisplayed[0][0] = spectra;
      DisplayCurrentRange();
      DrawMarkers();
    }
  } else {
    GetMessageDialog("The destination you selected is in shared memory and therefore protected.");
  }
}

/*
 * AddSpectraPrompt
 *
 * Prompts for Add Spectra
 */
void AddSpectraPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Add Spectra\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "A + B = C\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "A B C");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * SubtractSpectraEntry
 *
 * Interprets the Entry from the subtract spectra dialog
 * and then subtracts the spectra
 */
void SubtractSpectraEntry(GtkWidget *widget, GtkWidget *entry) {
  int a, b, c, i, j, k;
  int lastspectra;
  int newsize, newmemsize;
  int yadd;
  int tempcounts;
  int test;
  char dummystr[80];

  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%d %d %d %d", &a, &b, &yadd, &c);
  if (test == 4) {
    a = a - 1;
    b = b - 1;
    c = c - 1;
    /* --- make sure that the spectra exist --- */
    if ((histsize[a] > 0) && (histsize[b] > 0)) {

      /* --- make sure that the location to write is at
         most 1 more than the current last spectra --- */
      lastspectra = GetLastSpectrum();
      if (c > (lastspectra + 1)) {
        c = lastspectra + 1;
        sprintf(dummystr, "Destination spectrum changed to %d.\n", c);
        WriteMainText(dummystr);
      }
      /* --- make sure we arn't going to write to a memory leak --- */
      if (c < 254) {
        /* --- get the new size for the spectra --- */
        if (histsize[a] == histsize[b]) {
          newsize = histsize[a];
        } else {
          GetMessageDialog("You do know that these spectra are different sizes dont you.\nI'll try "
                           "to subtract them anyway.\n");
          if (histsize[a] > histsize[b]) {
            newsize = histsize[a];
          } else {
            newsize = histsize[b];
          }
        }
        ObliterateHistogram(c);
        newmemsize = sizeof(int) * newsize;
        histloc[c] = (int *)malloc(newmemsize);

        /* --- now to actually do the subtraction --- */
        /* --- have to be careful not to write negatives if
           oldscope = 1 --- */
        histsize[c] = newsize;
        histid[c] = c;
        for (i = 0; i < 40; i++) {
          field1[c][i] = field1[a][i];
          field2[c][i] = field2[a][i];
          field3[c][i] = field3[a][i];
        }
        for (i = 0; i < newsize; i++) {
          if (i > histsize[a]) { /* if a is the smaller spectra */
            tempcounts = yadd - *(histloc[b] + i);
          } else {
            if (i > histsize[b]) { /* if b is the smaller spectra */
              tempcounts = yadd + *(histloc[a] + i);
            } else {
              tempcounts = yadd + *(histloc[a] + i) - *(histloc[b] + i);
            }
          }
          // if ((oldscope == 1) && (tempcounts < 0)) {
          //   tempcounts = 0;
          //  }
          *(histloc[c] + i) = tempcounts;
        }
      }
      spectra = c;
      DisplayCurrentRange();
      DrawMarkers();
    } else {
      GetMessageDialog("One (or both) of the spectra you wish to subtract do not exist.\n");
    }
  }
}

/*
 * SubtractSpectraPrompt
 *
 * Prompts for Subtract Spectra Dialog
 */
void SubtractSpectraPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Subtract Spectra\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "A - B + const = C\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "A B const C");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * MoveSpectraEntry
 *
 * Interprets the entry from a dialog to move a spectrum
 */
void MoveSpectraEntry(GtkWidget *widget, GtkWidget *entry) {
  int test, a, b;
  int lastspectra;
  char dummystr[80];
  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%d %d", &a, &b);
  if (test == 2) {
    a = a - 1;
    b = b - 1;
    if (histsize[a] > 0) {
      /* --- make sure that the location to write is
         at most 1 more than the current last spectra --- */
      lastspectra = GetLastSpectrum();
      if (b > (lastspectra + 1)) {
        //      printf("Got to the Stop 1.\n");
        b = lastspectra + 1;
        sprintf(dummystr, "The destination histogram has been changed to %d.\n", b);
        WriteMainText(dummystr);
      }
      CopyHistogram(a, b);
      spectra = b;
      DisplayCurrentRange();
      DrawMarkers();
    } else {
      GetMessageDialog("The spectrum you want to move does not exist.\n");
    }
  }
}

/*
 * MoveSpectraPrompt
 *
 * Prompts to move a spectrum via a dialog
 */
void MoveSpectraPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Move Spectrum\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "A -> B\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "A B");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * CopyHistogram
 *
 * Writes histogram A in location B
 */
void CopyHistogram(int a, int b) {
  int i, j, k, newmemsize;

  newmemsize = sizeof(int) * histsize[a];
  // printf("Attempting to allocate %d bytes.\n", newmemsize);
  ObliterateHistogram(b);
  histloc[b] = (int *)malloc(newmemsize);

  histsize[b] = histsize[a];
  histid[b] = b;
  for (i = 0; i < 40; i++) {
    field1[b][i] = field1[a][i];
    field2[b][i] = field2[a][i];
    field3[b][i] = field3[a][i];
  }
  for (i = 0; i < histsize[a]; i++) {
    *(histloc[b] + i) = *(histloc[a] + i);
  }
}

/*
 * NormalizeSpectraEntry
 *
 * Interprets the results of the entry in the Normalize Spectra dialog
 * then normalizes the spectra
 */
void NormalizeSpectraEntry(GtkWidget *widget, GtkWidget *entry) {
  int a, i, j, k;
  float normalization;
  int test;

  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%f", &normalization);
  if (test = 1) {
    a = spectra;
    if (histsize[a] > 0) {
      for (i = 0; i < histsize[a]; i++) {
        *(histloc[a] + i) = normalization * *(histloc[a] + i);
      }
      DisplayCurrentRange();
      DrawMarkers();
    } else {
      GetMessageDialog("Ummm... That histogram is empty.\n");
    }
  }
}

/*
 * NormalizeSpectraPrompt
 *
 * Sets the prompt for the Normalize Spectra Dialog
 */
void NormalizeSpectraPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Normalize Spectra %d: %s by what factor?\n", spectra + 1, field2[spectra]);
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * CompressSpectraEntry
 *
 * Interprets the results of the entry in the Normalize Spectra dialog
 * then normalizes the spectra
 */
void CompressSpectraEntry(GtkWidget *widget, GtkWidget *entry) {
  int a, i, j, k;
  int cfactor, newsize;
  int tempsum;
  int temphist[8192];
  char tempfield1[40], tempfield2[40], tempfield3[40];
  int test;
  int newmemsize;

  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%d", &cfactor);
  if (test == 1) {
    a = spectra;
    /* --- make sure that histogram exists --- */
    if (histsize[a] > 0) {
      /* --- now let's squish it --- */
      newsize = (float)histsize[a] / (float)cfactor;
      for (i = 0; i < newsize; i++) {
        tempsum = 0;
        for (j = 0; j < cfactor; j++) {
          tempsum = tempsum + *(histloc[a] + i * cfactor + j);
        }
        temphist[i] = tempsum;
      }
      newmemsize = sizeof(int) * newsize;
      sprintf(tempfield1, field1[a]);
      sprintf(tempfield2, field2[a]);
      sprintf(tempfield3, field3[a]);
      ObliterateHistogram(a);
      histloc[a] = (int *)malloc(newmemsize);
      histsize[a] = newsize;
      sprintf(field1[a], tempfield1);
      sprintf(field2[a], tempfield2);
      sprintf(field3[a], tempfield3);
      for (i = 0; i < histsize[a]; i++)
        *(histloc[a] + i) = temphist[i];
      currentrange[0] = 0;
      currentrange[1] = newsize;
      DisplayCurrentRange();
      DrawMarkers();
    } else {
      GetMessageDialog("Ummm....That histogram is empty.\n");
    }
  }
}

/*
 * CompressSpectraPrompt
 *
 * Sets the prompt for the compress spectra Dialog
 */
void CompressSpectraPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Compress Spectra %d: %s by what factor?\n", spectra + 1, field2[spectra]);
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "For non-integer compression please use Gainshift.\n", spectra + 1,
          field2[spectra]);
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * ClearSpectraPrompt
 *
 * Sets the prompt to clear a spectrum
 */
void ClearSpectraPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Delete spectrum %d: %s? (yes,[NO])\n", spectra + 1, field2[spectra]);
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * ClearSpectraEntry
 *
 * Interprets the entry from the clear histogram dialog box
 */
void ClearSpectraEntry(GtkWidget *widget, GtkWidget *entry) {
  int test;
  int i, j, k;
  int a;
  int lastspectra;

  if ((strncmp(gtk_entry_get_text(GTK_ENTRY(entry)), "y", 1) != 0) ||
      (strncmp(gtk_entry_get_text(GTK_ENTRY(entry)), "Y", 1) != 0)) {
    a = spectra;
    if (a >= 0) {
      /* --- check to see if that histogram is in memmory --- */
      lastspectra = GetLastSpectrum();
      // printf("Last spectrum is %d.\n",lastspectra);
      if (a < lastspectra) {
        /* --- copy each histogram to the one before it (beginning with a +1)
           --- and then obliterate the last histogram. leaving us with one
           --- less histogram. */
        for (i = a; i < lastspectra; i++) {
          CopyHistogram((i + 1), i);
        }
        ObliterateHistogram(lastspectra);
      } else {
        if (a == lastspectra) {
          ObliterateHistogram(a);
          spectra--;
        }
      }
      DisplayCurrentRange();
      DrawMarkers();
    }
  }
}

/*
 * TitleSpectraEntry
 *
 * Interprets the Title dialog
 */
void TitleSpectraEntry(GtkWidget *widget, GtkWidget *entry) {
  sprintf(field2[spectra], gtk_entry_get_text(GTK_ENTRY(entry)));
  DisplayCurrentRange();
  DrawMarkers();
}

/*
 * TitleSpectraPrompt
 *
 * Prompts for the new title
 */
void TitleSpectraPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Title Spectrum\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "Please enter the new title for spectrum %d.\n", spectra + 1);
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * GainshiftSpectraPrompt
 *
 * Prompts for the Gainshift Spectra Dialog
 */
void GainshiftSpectraPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Gainshift Spectra\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "We need Spectrum, Old calibration,\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "New keV per channel,\n and new channels\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "SPEC A(old) B(old) C(old) b(new) #Chan\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/*
 * GainshiftSpectraEntry
 *
 * Interprets the entry of the Gainshift Spectra Dialog
 */
void GainshiftSpectraEntry(GtkWidget *widget, GtkWidget *entry) {
  int a, test;
  float oldcal[3], newcal;
  int i, j, k;
  int temphist[8192];
  float oldm, oldmsqr, oldp, oldpsqr;
  float anewm, anewp;
  int newm, newp, counts;
  float frac;
  float xi, tm, tp;
  int newchannels, newmemsize;

  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%d %f %f %f %f %d", &a, &oldcal[0],
                &oldcal[1], &oldcal[2], &newcal, &newchannels);
  printf("Test %d\n", test);
  if ((test == 6) && (histsize[a] > 0) && (newcal > 0)) {
    a = a;
    Gainshift(spectra, oldcal[0], oldcal[1], oldcal[2], newcal, newchannels);
  }
}

/*
 * Gainshift
 *
 * Changes the calibration from old values to new ones of spectra
 */
// void Gainshift (int localspectrum,float oldconst, float oldlin,float oldquad, float newcalib,
//		int newchannels)
//{
// int i,j,k;
// int *temphist;
// float oldm,oldmsqr,oldp,oldpsqr;
// float anewm,anewp;
// int newm,newp,counts;
// float frac;
// float xi,tm,tp;

/* --- make sure we arn't doing something really really dumb --- */
// g_return_if_fail(histsize[localspectrum] > 0);

/* --- Allocate memory for the temphist --- */
// temphist = (int *) malloc(sizeof(int) * newchannels);

// for (i = 0; i < newchannels; i++)
// temphist[i] = 0;

// for (i = 0; i < (histsize[localspectrum]); i ++) {
//       printf(".");
// oldm = (float) (i - 0.5);
// oldmsqr = oldm * oldm;
// oldp = (float) (oldm + 1);
// oldpsqr = oldp * oldp;
// anewm = (float) ((oldconst + oldlin * oldm + oldquad * oldmsqr) / newcalib);
//       printf("%f ",anewm);
// newm = (int) anewm;
// if ((anewm - newm) > 0.5) {
// newm = newm + 1;
// }
// printf("%d ",newm);
// anewp = (oldconst + oldlin * oldp + oldquad * oldpsqr) / newcalib;
//      printf("%f ", anewp);
// newp = anewp;
// if ((anewp - newp) > 0.5) {
// newp = newp+1;
//}
//  printf("%d ",newp);
// if (newm > 0) {
// if (newp <= 8192) {
// frac = 1.0 / (anewp - anewm);
// counts = *(histloc[localspectrum] + i);
// for (j = newm; j <= newp; j++) {
// printf("%f ",counts);
// xi = (float) j;
// tm = anewm;
// if (tm < (xi - 0.5)) tm = (xi - 0.5);
// tp = anewp;
// if (tp > (xi + 0.5)) tp = (xi + 0.5);
// temphist[j] = (temphist[j] + counts * frac * (tp-tm));
//}
//}
//}
//}

//    for (i = 0; i < histsize[a]; i ++) {
// printf("%d %d ",i,temphist[i]);
//}

// if (newchannels < 0) {
// newchannels = histsize[localspectrum];
//  }
// ObliterateHistogram(localspectrum);
// histsize[localspectrum] = newchannels;
// histid[localspectrum] = localspectrum;
// histloc[localspectrum] = (int *) malloc(newchannels * sizeof(int));

// for ( k = 0; k < histsize[localspectrum]; k++) {
//*(histloc[localspectrum] + k) = (int) temphist[k];
//       printf("%d %d ",k, *(histloc[a] + k));
// }

/* --- make the calibrations for the new spectrum make sense --- */
// calibration[localspectrum][0] = 0;
// calibration[localspectrum][2] = 0;
// calibration[localspectrum][1] = newcalib;

// currentrange[0] = 0;
// currentrange[1] = newchannels;
// offset = 0;
// spectra = localspectrum;
// DisplayCurrentRange();
// DrawMarkers();
// }

/*
 * GetLastSpectrum
 *
 * returns the value of the last spectrum
 */
int GetLastSpectrum() {
  int i, lastspectrum;

  //--ddc 19jul07, there are many functions creating spectra, which
  // rely on this, and if there is NO last spectrum, this thing should
  // return -1!
  //  lastspectrum = 0;
  lastspectrum = -1;
  for (i = 0; ((histsize[i] > 0) && (i < 254)); i++) {
    lastspectrum = i;
  }
  //  printf("Last Spectrum is %d.\n",lastspectrum);
  return (lastspectrum);
}

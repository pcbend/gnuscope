/* spec-u-lator.c
 * by John Pavan
 *
 * A graphical interface for spectrum manipulation
 */

/* --- includes --- */

//#include <gtk/gtk.h>
#include "gtkgraph.h"
#include "gtktwodplot.h"
//#include <stdio.h>
#include "gnuscopefuncs.h"
#include "gnuscopeglobals.h"

//--ddc aug11 global replace gtk_signal_connect with g_signal_connect


/* --- structs --- */

struct spec_u_toggle_struct {
  int add;
  int subtract;
  float norm;
};

/* --- globals --- */

//int histid[254],histsize[254],*histloc[254],histpointer;
//char field1[254][40],field2[254][40],field3[254][40];
int numspectra;
int spectra;
int currentrange[2];
struct spec_u_toggle_struct spec_u_toggle[254];
int spec_u_outputtoggle[254];
char spec_u_titles[254][20];
GtkWidget *spec_u_window = NULL;

/* --- function declarations --- */

void CloseWidget(GtkWidget *widget, gpointer *data);
void HideWidget(GtkWidget *widget);
void CreateSpecULatorWindow();
void SpecULatorInitialize();
int GetLastSpectrum();

/* --- functions --- */

/* SpecULatorGo
 *
 * Executes Spec-U-Lator
 */
void SpecULatorGo(GtkWidget *widget, gpointer *data) 
{
  short int output;
  short int adds[254],subtracts[254];
  short int num_adds,num_subtracts;
  int temphist[3][8192];
  int max_size;
  int i,j,k;
  int lastspectra;

  lastspectra = GetLastSpectrum();

  /* --- start by getting the output --- */
  /* --- (so we have some place to move it) --- */

  /* --- initialize the temphists --- */
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 8192; j++) {
      temphist[i][j] = 0;
    }
  }

  output = 0;

  for (i = 0; i < lastspectra+2; i++) {
    if (spec_u_outputtoggle[i]) {
      output = i;
    }
  }
  
  g_return_if_fail(output >= 0);

  /* --- ok, now we need to know which histograms we are adding (and subtracting) --- */
  
  num_adds = 0;
  num_subtracts = 0;
  //--ddc may15 lastspectra is NOT the number of spectra... add one to it
  for (i = 0; i < lastspectra+1; i++) {
    if (spec_u_toggle[i].add) {
      adds[num_adds] = i;
      num_adds++;
    }
    if (spec_u_toggle[i].subtract) {
      subtracts[num_subtracts] = i;
      num_subtracts++;
    }
  }
  
  /* --- now we determine the size of the largest histogram which is being manipulated --- */
  max_size = 0;

  for (i = 0; i < num_adds; i++) {
    if (max_size < histsize[adds[i]])
      max_size = histsize[adds[i]];
  }
  for (i = 0; i < num_subtracts; i++) {
    if (max_size < histsize[subtracts[i]])
      max_size = histsize[subtracts[i]];
  }

  g_return_if_fail((num_adds > 0) || (num_subtracts > 0));

  /* --- now we do some manipulations --- */
  //--ddc may15 lastspectra is NOT the number of spectra... add one to it
  for (i = 0; i < lastspectra+1; i++) {
    if (spec_u_toggle[i].add) {
      for (j = 0; j < histsize[i]; j++)
	temphist[2][j] += (float)(*(histloc[i] + j)) * spec_u_toggle[i].norm;
    }
    if (spec_u_toggle[i].subtract) {
      for (j = 0; j < histsize[i]; j++)
	temphist[2][j] -= (float)(*(histloc[i] + j)) * spec_u_toggle[i].norm;
    }
  }
  
  /* --- now we need to create space for the output --- */
  if (histsize[output] > 0)
    ObliterateHistogram(output);
  histloc[output] = (int *) malloc(max_size * sizeof(int));
  histsize[output] = max_size;
  histid[output] = output;
  sprintf(field2[output],spec_u_titles[output]); 
  
  for (i = 0; i < max_size; i++) {
    *(histloc[output] + i) = temphist[2][i];
  }

}

/* SpecULatorAddCallback
 *
 * Toggles the add flag in spec_u_toggle
 */
void SpecULatorAddCallback(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    spec_u_toggle[(int)data].add = 1;
  } else {
    spec_u_toggle[(int)data].add = 0;
  }
}

/* SpecULatorSubtractCallback
 *
 * Toggles the subtract flag in spec_u_toggle
 */
void SpecULatorSubtractCallback(GtkWidget *widget, gpointer *data) 
{
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    spec_u_toggle[(int)data].subtract = 1;
  } else {
    spec_u_toggle[(int)data].subtract = 0;
  }
}

/* SpecULatorNormCallback
 *
 * Updates the normalization in spec_u_toggle from what is displayed
 */
void SpecULatorNormCallback(GtkWidget *widget, gpointer *data)
{
  float temp;
  

  if (sscanf(gtk_entry_get_text(GTK_ENTRY(widget)),"%f",&temp) == 1)
    spec_u_toggle[(int)data].norm = temp;
}

/* SpecULatorTitleChanged
 *
 * Updates the new title in spec_u_titles from what is displayed
 */
void SpecULatorTitleChanged(GtkWidget *widget, gpointer *data)
{
  sprintf(spec_u_titles[(int) data],gtk_entry_get_text(GTK_ENTRY(widget)));
}

/* SpecULatorOutputCallback
 *
 * Toggles a particluar output on or off depending on if it is active or not
 */
void SpecULatorOutputCallback(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    spec_u_outputtoggle[(int)data] = 1;
  } else {
    spec_u_outputtoggle[(int)data] = 0;
  }
}

/* SpecULatorInitialize
 *
 * Initializes the variables for SpecULator
 */
void SpecULatorInitialize()
{
  int i,j;
  
  for (i = 0; i < 254; i++) {
    spec_u_toggle[i].add = 0;
    spec_u_toggle[i].subtract = 0;
    spec_u_toggle[i].norm = 1;
    spec_u_outputtoggle[i] = 0;
    sprintf(spec_u_titles[i],field2[i]);
  }
}


/* CreateSpecULatorWindow
 * 
 * Builds the spec-u-lator window
 */
void CreateSpecULatorWindow()
{
  GtkWidget *localhbox;
  GtkWidget *localvbox;
  GtkWidget *localhbox2;
  //GtkWidget *framevbox;
  GtkWidget *framehbox;
  GtkWidget *frametable;
  GtkWidget *localseparator;
  GtkWidget *localbutton;
  GtkWidget *localframe;
  GtkWidget *localentry;
  GtkWidget *locallabel;
  GtkWidget *columnboxes[6];
  GSList *localgroup = NULL;
  char dummystr[80];
  char dummystr2[80];
  int lastspectra;
  int i,j;

  /* --- idiot check --- */
  g_return_if_fail(histsize[0] > 0);

  /* --- only construct the window if we don't already have a window --- */

    SpecULatorInitialize();

    /* --- make the window --- */
    
    spec_u_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(spec_u_window),"Spec U Lator");
    gtk_window_set_modal(GTK_WINDOW(spec_u_window),TRUE);

    /* --- create the localhbox to put the elements in --- */

    localhbox = gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(spec_u_window),localhbox);

    /* --- create the frame for selecting what we want to manipulate --- */
    
    localframe = gtk_frame_new("Input");
    gtk_container_add(GTK_CONTAINER(localhbox),localframe);

    /* --- create an hbox in the frame to put stuff in --- */

    framehbox = gtk_hbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(localframe),framehbox);

    i = 0; // to keep track of how many entries we have in the input box
    
    while (histsize[i] > 0) {
      if ((i %10) == 0) {
	/* --- create a vbox to put stuff in --- */
	/* --- or perhaps a table would be better --- */

	frametable = gtk_table_new(12,7,FALSE);
	gtk_container_add(GTK_CONTAINER(framehbox),frametable);

	/* --- vertical sepearator --- */
	localseparator = gtk_vseparator_new();
	gtk_container_add(GTK_CONTAINER(framehbox),localseparator);
	
	/* --- make the columns in the vbox --- */
	/* --- add them into the framevbox --- */
	/* --- put in the legend --- */
	locallabel = gtk_label_new("NA");
	gtk_table_attach(GTK_TABLE(frametable),locallabel,
			 0,1,0,1,GTK_SHRINK,GTK_SHRINK,0,0);
	locallabel = gtk_label_new("+");
	gtk_table_attach(GTK_TABLE(frametable),locallabel,
			 1,2,0,1,GTK_SHRINK,GTK_SHRINK,0,0);
	locallabel = gtk_label_new("-");
	gtk_table_attach(GTK_TABLE(frametable),locallabel,
			 2,3,0,1,GTK_SHRINK,GTK_SHRINK,0,0);
	locallabel = gtk_label_new("Histogram:");
	gtk_table_attach(GTK_TABLE(frametable),locallabel,
			 5,6,0,1,GTK_SHRINK,GTK_SHRINK,0,0);
      }
      /* --- create the nessasary stuff for each entry --- */
      localbutton = gtk_radio_button_new(NULL);
      //--ddc aug11      localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
      localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
      gtk_table_attach(GTK_TABLE(frametable),localbutton,
		       0,1,(i%10)+1,(i%10)+2,GTK_SHRINK,GTK_SHRINK,0,0);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(localbutton),TRUE);
      localbutton = gtk_radio_button_new(localgroup);
      //--ddc aug11      localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
      localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
      gtk_table_attach(GTK_TABLE(frametable),localbutton,
		       1,2,(i%10)+1,(i%10)+2,GTK_SHRINK,GTK_SHRINK,0,0);
      g_signal_connect(GTK_OBJECT(localbutton),"clicked",
			 G_CALLBACK(SpecULatorAddCallback),(int *) i);
      localbutton = gtk_radio_button_new(localgroup);
      //--ddc aug11      localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
      localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
      gtk_table_attach(GTK_TABLE(frametable),localbutton,
		       2,3,(i%10)+1,(i%10)+2,GTK_SHRINK,GTK_SHRINK,0,0);
      g_signal_connect(GTK_OBJECT(localbutton),"clicked",
			 G_CALLBACK(SpecULatorSubtractCallback),(int *)i);
      locallabel = gtk_label_new("*");
      gtk_table_attach(GTK_TABLE(frametable),locallabel,
		       3,4,(i%10)+1,(i%10)+2,GTK_SHRINK,GTK_SHRINK,0,0);
      localentry = gtk_entry_new_with_max_length(6);
      gtk_widget_set_usize(localentry,20,20);
      gtk_entry_set_text(GTK_ENTRY(localentry),"1.0");
      gtk_table_attach(GTK_TABLE(frametable),localentry,
		       4,5,(i%10)+1,(i%10)+2,GTK_SHRINK,GTK_SHRINK,0,0);
      g_signal_connect(GTK_OBJECT(localentry),"changed",
			 G_CALLBACK(SpecULatorNormCallback), (int *) i);
      sprintf(dummystr,"%d: %s",i+1,field2[i]);
      locallabel = gtk_label_new(dummystr);
      gtk_table_attach(GTK_TABLE(frametable),locallabel,
		       5,6,(i%10)+1,(i%10)+2,GTK_SHRINK,GTK_SHRINK,0,0);
      
      i++;
    }
    
    localvbox = gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(localhbox),localvbox);
    locallabel = gtk_label_new("||\n\\/");
    gtk_container_add(GTK_CONTAINER(localvbox),locallabel);

    /* --- ok now we need to make the output histogram do-hicky --- */

    /* --- start by making the frame --- */
    sprintf(dummystr,"output");
    localframe = gtk_frame_new(dummystr);
    gtk_container_add(GTK_CONTAINER(localhbox),localframe);
    
    /* --- now make a hbox for the frame to put the columns in --- */
    framehbox = gtk_hbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(localframe),framehbox);
    
    /* --- now we start with the column stuff --- */
    
    i = 0;
    
    while (histsize[i] > 0) {
      if ((i % 10) == 0) {
	frametable = gtk_table_new(3,12,FALSE);
	gtk_container_add(GTK_CONTAINER(framehbox),frametable);
	locallabel = gtk_label_new("sel");
	gtk_table_attach(GTK_TABLE(frametable),locallabel,
			 0,1,0,1,GTK_SHRINK,GTK_SHRINK,0,0);
	locallabel = gtk_label_new(" ");
	gtk_table_attach(GTK_TABLE(frametable),locallabel,
			 1,2,0,1,GTK_SHRINK,GTK_SHRINK,0,0);	
	locallabel = gtk_label_new("Title");
	gtk_table_attach(GTK_TABLE(frametable),locallabel,
			 2,3,0,1,GTK_SHRINK,GTK_SHRINK,0,0);
      }
      
      if (i == 0) {
	localbutton = gtk_radio_button_new(NULL);
      } else {
	localbutton = gtk_radio_button_new(localgroup);
      }
      //--ddc aug11      localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
      localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
      g_signal_connect(GTK_OBJECT(localbutton),"clicked",
			 G_CALLBACK(SpecULatorOutputCallback),(int *) i);
      gtk_table_attach(GTK_TABLE(frametable),localbutton,
		       0,1,(i%10)+1,(i%10)+2,GTK_SHRINK,GTK_SHRINK,0,0);
      sprintf(dummystr,"%d:",i+1);
      locallabel = gtk_label_new(dummystr);
      gtk_table_attach(GTK_TABLE(frametable),locallabel,
		       1,2,(i%10)+1,(i%10)+2,GTK_SHRINK,GTK_SHRINK,0,0);
      localentry = gtk_entry_new_with_max_length(20);
      gtk_widget_set_usize(localentry,80,20);
      gtk_entry_set_text(GTK_ENTRY(localentry),field2[i]);
      g_signal_connect(GTK_OBJECT(localentry),"changed",
			 G_CALLBACK(SpecULatorTitleChanged), (int *) i);
      gtk_table_attach(GTK_TABLE(frametable),localentry,
		       2,3,(i%10)+1,(i%10)+2,GTK_SHRINK,GTK_SHRINK,0,0);
      i++;
    } // done with the output selection
    if ((i % 10) == 0) {
      frametable = gtk_table_new(3,12,FALSE);
      gtk_container_add(GTK_CONTAINER(framehbox),frametable);
      locallabel = gtk_label_new("sel");
      gtk_table_attach(GTK_TABLE(frametable),locallabel,
		       0,1,0,1,GTK_SHRINK,GTK_SHRINK,0,0);
      locallabel = gtk_label_new(" ");
      gtk_table_attach(GTK_TABLE(frametable),locallabel,
		       1,2,0,1,GTK_SHRINK,GTK_SHRINK,0,0);	
      locallabel = gtk_label_new("Title");
      gtk_table_attach(GTK_TABLE(frametable),locallabel,
		       2,3,0,1,GTK_SHRINK,GTK_SHRINK,0,0);
    }
    
    if (i == 0) {
      localbutton = gtk_radio_button_new(NULL);
    } else {
      localbutton = gtk_radio_button_new(localgroup);
    }
    //--ddc aug11    localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
    localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
    g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		       G_CALLBACK(SpecULatorOutputCallback),(int *) i);
    gtk_table_attach(GTK_TABLE(frametable),localbutton,
		     0,1,(i%10)+1,(i%10)+2,GTK_SHRINK,GTK_SHRINK,0,0);
    sprintf(dummystr,"%d:",i+1);
    locallabel = gtk_label_new(dummystr);
    gtk_table_attach(GTK_TABLE(frametable),locallabel,
		     1,2,(i%10)+1,(i%10)+2,GTK_SHRINK,GTK_SHRINK,0,0);
    localentry = gtk_entry_new_with_max_length(20);
    gtk_widget_set_usize(localentry,80,20);
    gtk_entry_set_text(GTK_ENTRY(localentry),field2[i]);
    g_signal_connect(GTK_OBJECT(localentry),"changed",
		       G_CALLBACK(SpecULatorTitleChanged), (int *) i);
    gtk_table_attach(GTK_TABLE(frametable),localentry,
		     2,3,(i%10)+1,(i%10)+2,GTK_SHRINK,GTK_SHRINK,0,0);
    i++;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(localbutton),TRUE);

    /* --- buttons for operation --- */

    localbutton = gtk_button_new_with_label("Go!");
    gtk_container_add(GTK_CONTAINER(localhbox),localbutton);
    g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		       G_CALLBACK(SpecULatorGo),NULL);
    //--ddc aug11    gtk_signal_connect_object(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect_swapped(GTK_OBJECT(localbutton),"clicked",
			      G_CALLBACK(CloseWidget),GTK_OBJECT(spec_u_window));
    
    
    localbutton = gtk_button_new_with_label("Cancel");
    gtk_container_add(GTK_CONTAINER(localhbox),localbutton);
    //--ddc aug11    gtk_signal_connect_object(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect_swapped(GTK_OBJECT(localbutton),"clicked",
			      G_CALLBACK(CloseWidget),GTK_OBJECT(spec_u_window));
    
    gtk_widget_show_all(spec_u_window);
}

/* projectionbox.c
 *
 * by John Pavan
 * the interface for projecting from
 * either integer .sqr or .twd
 * or floating point .sqr or .twd
 * matricies
 */

/* --- includes --- */

//#include <stdio.h>
//#include <gtk/gtk.h>
#include "gtktwodplot.h"
//#include "pgam.h"
#include <stdlib.h>
#include "gnuscopefuncs.h"
#include "gnuscopeglobals.h"


/* --- structs --- */


/* --- globals --- */

GtkWidget *projectionwindow;
GtkWidget *projaxisframe;
FILE *logfile;
int numspectra;//, spectradisplayed[16];
//int histid[254],histsize[254],*histloc[254],histpointer;
//char field1[254][40],field2[254][40],field3[254][40];
int intype,spectra,plottype;
int markers[4],binsize,offset,currentrange[2];
//int background;
struct gsdata twoddata;
struct pgam_matrix_info pgammatrixdata;
GtkWidget *projdestent;
GtkWidget *projconstent;
GtkWidget *projsubfracent;
GtkWidget *projsubranuppent;
GtkWidget *projsubranlowent;
GtkWidget *projaddranlowent;
GtkWidget *projaddranuppent;
int projectaddmarkers;
int projectaddrange;
int projectsubtractmarkers;
int projectsubtractrange;
int projectsubtractfrac;
int projectsqraxis;

/* --- function declarations --- */

int GetLastSpectrum();
void CloseWidgetProj(GtkWidget *widget, gpointer data);
void CloseWidget(GtkWidget *widget, gpointer data);
void PgamTwodProjectSqr (int addlower, int addupper,
                         int subtractlower, int subtractupper,
                         int histdest,int add,int xory);
void PgamTwodProjectTwd (int addlower, int addupper,
                         int subtractlower, int subtractupper,
                         int histdest, int add);
void TwdProject(int addlower, int addupper, 
		int subtractlower, int subtractupper, 
		int desthist, int add);
void SqrProject(int addlower, int addupper, 
		int subtractlower, int subtractupper, 
		int desthist, int add,int xory);

void FullProjectCallback (GtkWidget *widget, gpointer *data);
void ProjectCallback (GtkWidget *widget, gpointer *data);
void ProjectXAxisCallback(GtkWidget *widget, gpointer *data);
void ProjectYAxisCallback(GtkWidget *widget,gpointer *data);
void ProjectSubtractFracCallback(GtkWidget *widget, gpointer *data);
void ProjectSubtractRangeCallback(GtkWidget *widget, gpointer *data);
void ProjectAddMarkersCallback (GtkWidget *widget, gpointer *data);
void ProjectAddRangeCallback (GtkWidget *widget,gpointer *data);
void ProjectSubtractMarkersCallback(GtkWidget *widget,gpointer *data);
void HideWidget(GtkWidget *widget, gpointer data);

/* --- functions --- */
/* ClosesWidget
 *
 * Closes a widget it a proper way
 */
void CloseWidget(GtkWidget *widget, gpointer data)
{
  /* --- first close the widget --- */
  gtk_widget_destroy(GTK_WIDGET(data));
  gtk_widget_destroy(widget);
  widget = NULL;
}

/* Hidewidget
 *
 * hides a widget
 */
void HideWidget(GtkWidget *widget, gpointer data)
{
  gtk_widget_hide(widget);
}

/* ClosesWidgetProj
 *
 * Closes a widget it a proper way
 * also properly disposes of the entry boxes.
 */
void CloseWidgetProj(GtkWidget *widget, gpointer data)
{
  /* --- first close the entry widgets --- */
  gtk_widget_destroy(projdestent);
  projdestent = NULL;
  gtk_widget_destroy(projconstent);
  projconstent = NULL;
  gtk_widget_destroy(projsubfracent);
  projsubfracent = NULL;
  gtk_widget_destroy(projsubranuppent);
  projsubranuppent = NULL;
  gtk_widget_destroy(projsubranlowent);
  projsubranlowent = NULL;
  gtk_widget_destroy(projaddranlowent);
  projaddranlowent = NULL;
  gtk_widget_destroy(projaddranuppent);
  projaddranuppent = NULL;

  gtk_widget_destroy(GTK_WIDGET(data));
  gtk_widget_destroy(widget);
  widget = NULL;
}

/* ProjectWindow
 *
 * Generates a window with the appropriate options to execute a sort
 */
void ProjectWindow()
{
  char dummystr[80];
  //GtkWidget *localwindow;
  GtkWidget *localbutton;
  GtkWidget *localvbox;
  GtkWidget *localhbox;
  GtkWidget *locallabel;
  GtkWidget *localframe;
  GtkWidget *framevbox;
  GSList *localgroup = NULL;

  //--ddc 4may06  if (projectionwindow == NULL) {
 if (!GTK_IS_WIDGET(projectionwindow) ) {

    /* --- initialize all the toggles --- */
    //fprintf(logfile,"start initializing toggle variables.\n");
    projectaddmarkers = 1;
    projectaddrange = 0;
    projectsubtractmarkers = 0;
    projectsubtractrange = 0;
    projectsubtractfrac = 1;
    projectsqraxis = 0;
    //fprintf(logfile,"done initializing toggle variables.\n");
    
    /* --- spawn the window --- */
    
    //fprintf(logfile,"about to generate the window widget.\n");
    projectionwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    //fprintf(logfile,"successfully generated the window widget.\n");
    gtk_window_set_title(GTK_WINDOW(projectionwindow),"Projection Selection");
    //fprintf(logfile,"successfully set the title of the window widget.\n");
    
    /* --- set the polocies for the window  --- */
    
    gtk_window_set_modal(GTK_WINDOW(projectionwindow),TRUE);
    //fprintf(logfile,"Successfully set the projectionwindow to modal.\n");
    gtk_window_set_position(GTK_WINDOW(projectionwindow),GTK_WIN_POS_CENTER);
    //fprintf(logfile,"set the localwindow position to center.\n");

    /* --- done spawning the window --- */
    /* --- now we need to make a vbox to put all the elements in --- */
    
    localvbox = gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(projectionwindow),localvbox);
    gtk_widget_show(localvbox);
    
    /* --- now we need to make a frame for the add options --- */
    
    localframe = gtk_frame_new("Add Gate");
    gtk_container_add(GTK_CONTAINER(localvbox),localframe);
    framevbox = gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(localframe),framevbox);
    gtk_widget_show_all(localframe);

    /* --- now the components of the addition frame --- */
    
    
    localbutton = gtk_radio_button_new_with_label(NULL,"From Markers (most recent 2)");
    //--ddc aug11    localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
    localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
    gtk_container_add(GTK_CONTAINER(framevbox),localbutton);
    //--ddc aug11    gtk_signal_connect(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		       G_CALLBACK(ProjectAddMarkersCallback),NULL);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(localbutton),TRUE);
    gtk_widget_show(localbutton);
    
    localhbox = gtk_hbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(framevbox),localhbox);
    localbutton = gtk_radio_button_new_with_label(localgroup,"From ");

    //--ddc aug11 deprecation    localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
    localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
    gtk_container_add(GTK_CONTAINER(localhbox),localbutton);
    //--ddc aug11    gtk_signal_connect(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		       G_CALLBACK(ProjectAddRangeCallback),NULL);
    projaddranlowent = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(localhbox),projaddranlowent);
    locallabel = gtk_label_new(" to ");
    gtk_container_add(GTK_CONTAINER(localhbox),locallabel);
    projaddranuppent = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(localhbox),projaddranuppent);
    locallabel = gtk_label_new(" Channels");
    gtk_container_add(GTK_CONTAINER(localhbox),locallabel);
    gtk_widget_show_all(localhbox);
    
    /* --- now a frame for the subtract gate choices --- */
    
    localframe = gtk_frame_new("Subtract Gate");
    gtk_container_add(GTK_CONTAINER(localvbox),localframe);
    framevbox = gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(localframe),framevbox);
    gtk_widget_show_all(localframe);
    
    /* --- now for the components of the subtract frame --- */
    
    localbutton = gtk_radio_button_new_with_label(NULL,"From Markers (3rd and 4th most recent)");
    //--ddc aug11 deprecation    localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
    localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
    gtk_container_add(GTK_CONTAINER(framevbox),localbutton);
    //--ddc aug11    gtk_signal_connect(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		       G_CALLBACK(ProjectSubtractMarkersCallback),NULL);
    gtk_widget_show(localbutton);

    localhbox = gtk_hbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(framevbox),localhbox);
    localbutton = gtk_radio_button_new_with_label(localgroup,"From ");
    //--ddc aug11 deprecation    localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
    localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
    gtk_container_add(GTK_CONTAINER(localhbox),localbutton);
    //--ddc aug11    gtk_signal_connect(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		       G_CALLBACK(ProjectSubtractRangeCallback),NULL);
    projsubranlowent = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(localhbox),projsubranlowent);
    locallabel = gtk_label_new(" to ");
    gtk_container_add(GTK_CONTAINER(localhbox),locallabel);
    projsubranuppent = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(localhbox),projsubranuppent);
    locallabel = gtk_label_new(" Channels");
    gtk_container_add(GTK_CONTAINER(localhbox),locallabel);
    gtk_widget_show_all(localhbox);
    
    localhbox = gtk_hbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(framevbox),localhbox);
    localbutton = gtk_radio_button_new_with_label(localgroup,"Fraction of the Total Projection");
    //--ddc aug11    localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
    localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
    gtk_container_add(GTK_CONTAINER(localhbox),localbutton);
    //--ddc aug11    gtk_signal_connect(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		       G_CALLBACK(ProjectSubtractFracCallback),NULL);
    projsubfracent = gtk_entry_new();
    sprintf(dummystr,"%f",background[0]);
    gtk_entry_set_text(GTK_ENTRY(projsubfracent),dummystr);
    gtk_container_add(GTK_CONTAINER(localhbox),projsubfracent);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(localbutton),TRUE);
    gtk_widget_show_all(localhbox);
    
    /* --- now, if we are dealing with a .sqr matrix
       --- we need to make the option to select which axis
       --- to project from --- */

    projaxisframe = gtk_frame_new("Projection Axis");
    gtk_container_add(GTK_CONTAINER(localvbox),projaxisframe);
    framevbox = gtk_hbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(projaxisframe),framevbox);
    
    /* --- the buttons to sellect the projection axis --- */
    localbutton = gtk_radio_button_new_with_label(NULL,"X-axis");
    gtk_container_add(GTK_CONTAINER(framevbox),localbutton);
    //--ddc aug11    localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
    localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
    //--ddc aug11    gtk_signal_connect(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		       G_CALLBACK(ProjectXAxisCallback),NULL);
    localbutton = gtk_radio_button_new_with_label(localgroup,"Y-axis");
    gtk_container_add(GTK_CONTAINER(framevbox),localbutton);
    //--ddc aug11    localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
    localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
    //--ddc aug11    gtk_signal_connect(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		       G_CALLBACK(ProjectYAxisCallback),NULL);
    if ((pgammatrixdata.type == 0) || (twoddata.filetype == 2)) {
      gtk_widget_show_all(projaxisframe);
    }
    
    /* --- entry for the constant to add --- */
    
    localhbox = gtk_hbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(localvbox),localhbox);
    locallabel = gtk_label_new("Constant to Add");
    gtk_container_add(GTK_CONTAINER(localhbox),locallabel);
    projconstent = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(projconstent),"0");
    gtk_container_add(GTK_CONTAINER(localhbox),projconstent);
    gtk_widget_show_all(localhbox);
    
    /* --- now we need to get the destination histogram --- */
    localhbox = gtk_hbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(localvbox),localhbox);
    locallabel = gtk_label_new("Destination Histogram");
    gtk_container_add(GTK_CONTAINER(localhbox),locallabel);
    projdestent = gtk_entry_new();
    sprintf(dummystr,"%d",GetLastSpectrum()+2);
    gtk_entry_set_text(GTK_ENTRY(projdestent),dummystr);
    gtk_container_add(GTK_CONTAINER(localhbox),projdestent);
    gtk_widget_show_all(localhbox);
    
    
    /* --- and a button for a full project --- */
    localbutton = gtk_button_new_with_label("Project Full");
    //--ddc aug11   gtk_signal_connect(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		       G_CALLBACK(FullProjectCallback),NULL);
    //--ddc aug11    gtk_signal_connect_object(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect_swapped(GTK_OBJECT(localbutton),"clicked",
			      G_CALLBACK(HideWidget),GTK_OBJECT(projectionwindow));
    gtk_container_add(GTK_CONTAINER(localvbox),localbutton);
    gtk_widget_show(localbutton);
    
    /* --- now we make a button to project --- */
    localbutton = gtk_button_new_with_label("Project");
    //--ddc aug11    gtk_signal_connect(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		       G_CALLBACK(ProjectCallback),NULL);
    //--ddc aug11    gtk_signal_connect_object(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect_swapped(GTK_OBJECT(localbutton),"clicked",
			      G_CALLBACK(HideWidget),GTK_OBJECT(projectionwindow));
    gtk_container_add(GTK_CONTAINER(localvbox),localbutton);
    gtk_widget_show(localbutton);

    /* --- now we make a button to cancel --- */
    localbutton = gtk_button_new_with_label("Cancel");
    //--ddc aug11    gtk_signal_connect_object(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect_swapped(GTK_OBJECT(localbutton),"clicked",
			      G_CALLBACK(HideWidget),GTK_OBJECT(projectionwindow));
    gtk_box_pack_start(GTK_BOX(localvbox),localbutton,FALSE,FALSE,0);
    gtk_widget_show(localbutton);
    
    gtk_widget_show(projectionwindow);
  } else {

    sprintf(dummystr,"%d",GetLastSpectrum()+2);
    gtk_entry_set_text(GTK_ENTRY(projdestent),dummystr);

    sprintf(dummystr,"%f",background[0]);
    gtk_entry_set_text(GTK_ENTRY(projsubfracent),dummystr);

    if ((pgammatrixdata.type == 0) || (twoddata.filetype == 2)) {
      gtk_widget_show_all(projaxisframe);
    } else {
      gtk_widget_hide(projaxisframe);
    }

    gtk_widget_show(projectionwindow);
  }
}

void FullProjectCallback (GtkWidget *widget, gpointer *data) 
{
  if ((pgammatrixdata.type == 0) || (pgammatrixdata.type == 3)) {
    PgamTwodProjectFull();
  } else {
    if ((twoddata.filetype == 1) || (twoddata.filetype == 2))
      ProjectFull();
  }
}

void ProjectCallback (GtkWidget *widget, gpointer *data)
{
  int test;
  int i,j,k;
  int addlower,addupper,subtractlower,subtractupper,desthist,add;

  addlower = addupper = -1;
  subtractlower = subtractupper = -1;
  desthist = -1;
  add = 0;

  if (projectaddmarkers) {
    addlower = Min(markers[0],markers[1]);
    addupper = Max(markers[0],markers[1]);
  }
  if (projectaddrange) {
    test = sscanf(gtk_entry_get_text(GTK_ENTRY(projaddranlowent)),"%d",&i);
    if (test == 1) {
      test = sscanf(gtk_entry_get_text(GTK_ENTRY(projaddranuppent)),"%d",&j);
      if (test == 1) {
	addlower = Min(i,j);
	addupper = Max(i,j);
      }
    }
  }

  if (projectsubtractmarkers) {
    subtractlower = Min(markers[2],markers[3]);
    subtractupper = Max(markers[2],markers[3]);
  }
  if (projectsubtractrange) {
    test = sscanf(gtk_entry_get_text(GTK_ENTRY(projsubranlowent)),"%d",&i);
    if (test == 1) {
      test = sscanf(gtk_entry_get_text(GTK_ENTRY(projsubranlowent)),"%d",&i);
      if (test == 1) {
	subtractlower = Min(i,j);
	subtractupper = Max(i,j);
      }
    }
  }
  if (projectsubtractfrac) {
    subtractupper = 0;
    test = sscanf(gtk_entry_get_text(GTK_ENTRY(projsubfracent)),"%d",&i);
    if (test == 1) {
      subtractlower = - (int) abs((double)i);
    } else {
      subtractlower = 0;
    }
  }
  
  test = sscanf(gtk_entry_get_text(GTK_ENTRY(projconstent)),"%d",&i);
  if (test == 1) add = i;

  test = sscanf(gtk_entry_get_text(GTK_ENTRY(projdestent)),"%d",&i);
  if (test == 1) desthist = i-1;

  if ((desthist < 0) || (desthist > (GetLastSpectrum() + 1))) {
    desthist = GetLastSpectrum() + 1;
  }
  spectradisplayed[0][0] = desthist;
  
  /* --- now that we have the information that we need, we need to
     --- issue the appropriate command to sort --- */

  switch (twoddata.filetype) {
  case 1: 
    TwdProject(addlower,addupper,subtractlower,subtractupper,desthist,add);
    break;
  case 2:
    SqrProject(addlower,addupper,subtractlower,subtractupper,desthist,add,projectsqraxis);
    break;
  default:
    switch (pgammatrixdata.type) {
    case 0:
      PgamTwodProjectSqr(addlower,addupper,subtractlower,subtractupper,desthist,add,projectsqraxis);
      break;
    case 3:
      PgamTwodProjectTwd(addlower,addupper,subtractlower,subtractupper,desthist,add);
      break;
    default:
      /* --- do nothing --- */
      break;
    }
  }

}

void ProjectXAxisCallback(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    projectsqraxis = 0;
  }
  WriteMessageText("Projection axis is X.\n");
}

void ProjectYAxisCallback(GtkWidget *widget,gpointer *data)
{
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    WriteMessageText("Project axis is Y.\n");
    projectsqraxis = 1;
  }
}

void ProjectSubtractFracCallback(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    WriteMessageText("Subtract gate is a fraction of total projection.\n");
    projectsubtractfrac = 1;
  } else {
    projectsubtractfrac = 0;
  }
}

void ProjectSubtractRangeCallback(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    WriteMessageText("Subtract Gate is now from the range in the entries.\n");
    projectsubtractrange = 1;
  } else {
    projectsubtractrange = 0;
  }
}

void ProjectSubtractMarkersCallback(GtkWidget *widget,gpointer *data) 
{
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    WriteMessageText("Subtract Gate is now from markers.\n");
    projectsubtractmarkers = 1;
  } else {
    projectsubtractmarkers = 0;
  }
}

void ProjectAddRangeCallback (GtkWidget *widget,gpointer *data) 
{
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    WriteMessageText("Add Gate is from range in the entries.\n");
    projectaddrange = 1;
  } else {
    projectaddrange = 0;
  }
}

void ProjectAddMarkersCallback (GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    WriteMessageText("Add Gate is now from markers.\n");
    projectaddmarkers = 1;
  } else {
    projectaddmarkers = 0;
  }
}

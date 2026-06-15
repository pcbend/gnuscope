/*
 * pgamsort.c
 *
 * by John Pavan
 * the actual sorting and setup for particle gamma sorting
 * kinda includes TAC support
 */

/* --- includes --- */

#include <stdio.h>
#include <math.h>
//--ddc in gnuscopefuncs.h #include <gtk/gtk.h>

#include "gtktwodplot.h"
//--ddc 3jun08 this is in gnuscopefuncs.h: #include "pgam.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include "gnuscopeglobals.h"
//--ddc 3jun08 add more protyping!
#include "gnuscopefuncs.h"

/* --- structs --- */

struct setup_file_info {
  int runs[2];            // first and last run to use this setup file for
  char filename[120];         // name of the file
};

//--ddc aug11. Globally changed all gtk_signal_connect calls to g_signal_connect

/* --- globals --- */

int datafiletype;
int abortflag;
FILE *logfile;
char twodtitles[254][80];
struct pgam_gamma_gates pgamgammagates;
//float partfincal;
int pgam;
int min_clover_multipolarity;
int truesingles;
int pgammax;
//int pgamsortgatenone,pgamsortgatetac,pgamsortgatepart,pgamsortgategamma;
//int pgamsortoutputhists,pgamsortoutputpart,pgamsortoutputtac,pgamsortoutputgg;
//int pgamsortoutputggtype,pgamsortgateparten;
//int pgamsortgategammatypeentry,pgamsortgategammatypefile;
//int pgamsortoutputsqr;
int num_hpge;
struct detector_info *hpges;
int num_telescopes;
struct telescope_info *telescopes;
int num_clovers;
struct multi_hpge_info *clovers;
int num_tacs;
struct tac_info *tacs;
int num_setups;
struct setup_file_info *setupfiles;
int fin_chans;       // the number of channels in the final spectra
int num_adcs;        // number of adcs used in the experiment
float fin_cal;       // final calibration (keV/chan)
float beta;          // beta for doppler shift correction
int pgamsorttype;    // kind of sorting to do
int sortrecords;     // sort all records or not? 
int runmin,runmax;   // first and last runs to sort
char evtpath[80];    // path to .evt files
struct bigdata **big_data_info;
struct gatedata *gate_info;
//int histid[254],histsize[254],*histloc[254],histpointer;
//char field1[254][40],field2[254][40],field3[254][40];
/* --- addarray and subtract array sizes are determined by the max
   --- number of channels in our adc's, xgatearray and ygatearray sizes
   --- are chosen as such because gammasphere has 110 elements. --- */
int addarray[8192],subtractarray[8192],xgatearray[110],ygatearray[110];
struct axis_info xaxis_info,yaxis_info;
struct gamma_gate gammagates[128];        // this means no more than 128 gamma gates
struct pgam_matrix_info pgammatrixdata;
GtkWidget *pgamentry0,*pgamentry1;
GtkWidget *pgamentry2,*pgamentry3;
GtkWidget *pgamentry4,*pgamentry5;
int numxdets,numydets;
int *xdets,*ydets;
//--ddc jan15 ftaux,ftauy for SLT's doppler shift correction
float ftaux=1.0, ftauy = 1.0;
GtkWidget *pgamentry6,*pgamentry7;
GtkWidget *pgamentry8,*pgamentry9;
GtkWidget *pgamentry10,*pgamentry11;
GtkWidget *pgamentry12;
GtkWidget *pgamentry13;  // for the filename of the pairs file
int recordmin,recordmax;
//--ddc may17 THIS is a global variable!! Declaring it here, may
//cause 'scope' confusion. 
//int edesize;
GtkWidget *pgamentry14; // for the filename of the veto gate file (.vto)
GtkWidget *pgamentry15;  // for the filename of the requires gate file (.req)
int pgam1dgatemin; // for the minimum level number of times to pass the 1d (gamma) gates by
GtkWidget *pgamentry16; // for the pgam1dgatemin

/* --- constants --- */


/* --- function declarations --- */


int LongBitLog2(unsigned long long arg);
struct veto_struct *VetoStructNew();
void PgamGateVetoToggle(GtkWidget *widget, gpointer *data) ;
void PgamGateRequiresToggle(GtkWidget *widget, gpointer *data);
struct veto_struct *PgamReadVeto(char *sFilename);
void VetoStructAdd(struct veto_struct *vetos,int adc,int min, int max);
void VetoStructDestroy(struct veto_struct *vetos);
void SetPgamEntry13(char *sFilename);
void OutputPairSqrToggle(GtkWidget *widget, gpointer *data);
void PgamSortBrowse13(GtkWidget *widget, gpointer *data);
void PgamSortBrowse15(GtkWidget *widget, gpointer *data);
void CloverSelfSuppressToggle(GtkWidget *widget, gpointer *data);
int CompareInts(const void *a, const void *b);
int BitLog2(uint arg);
void CloseWidgetPgamSort(GtkWidget *widget, gpointer data);
void PgamReadSetup(char *sFilename);
void WriteTwodText(const char *newtext);
void WriteMessageText(const char *newtext);
void GetDialog(int type);
void PgamSortTypePrompt(GtkWidget *text);
void PgamSortTyepEntry(GtkWidget *widget, GtkWidget *entry);
void PgamRecordsSortEntry(GtkWidget *widget, GtkWidget *entry);
void PgamRecordsSortPrompt(GtkWidget *text);
void tac_adc_add(struct tac_info *tac, int adc);
void PgamSort();
void RawSinglesSort(GtkWidget *widget, gpointer *data);
void PgamSortTypeEntry(GtkWidget *widget, GtkWidget *entry);
void PgamSortTypePrompt(GtkWidget *text);
void PgamSortSelectionWindow();
void GammaGateToggle(GtkWidget *widget, gpointer *data);
void PartGateToggle(GtkWidget *widget, gpointer *data);
void TACGateToggle(GtkWidget *widget, gpointer *data);
void OutputSqrToggle(GtkWidget *widget, gpointer *data);
void OutputTwdToggle(GtkWidget *widget, gpointer *data);
void OutputTACToggle(GtkWidget *widget, gpointer *data);
void OutputPartToggle(GtkWidget *widget, gpointer *data);
void OutputHistToggle(GtkWidget *widget, gpointer *data);
void PgamSortCallback(GtkWidget *widget, gpointer *data);
void OutputGGSToggle(GtkWidget *widget, gpointer *data);
void OutputNoMatrix(GtkWidget *widget, gpointer *data);
void PartEnGateToggle(GtkWidget *widget, gpointer *data);
void PgamSortGateGammaTypeEntry (GtkWidget *widget, gpointer *data);
void PgamSortGateGammaTypeFile (GtkWidget *widget, gpointer *data);
void PgamReadGammaGate(char *sFilename);
void PgamReadDetectorInfo(char *sFilename);
void PgamSortBrowse4(GtkWidget *widget, gpointer *data);
void SetPgamEntry4(char *sFilename);
void PgamSortBrowse5(GtkWidget *widget, gpointer *data);
void SetPgamEntry5(char *sFilename);
struct kinmat_info_struct *ReadKinmatInfo(char *sFilename);
float KinmatInfoStructGetExcit(struct kinmat_info_struct *stru, float ener);
float KinmatInfoStructGetEnergy(struct kinmat_info_struct *stru, float ener);
struct bigdata *BigDataStructNew(int x, int y);
void BigDataStructDestroy(struct bigdata *stru);
int BigDataStructGetVal(struct bigdata *stru, int x, int y);
void BigDataStructSetVal(struct bigdata *stru, int x, int y, int z);
void BigDataStructValPP(struct bigdata *stru, int x, int y);
void CloverMinMultipolarityToggle(GtkWidget *widget, gpointer *data);
struct pgam_pairs *PgamReadPairs(char *sFilename);
void PgamPairsAdd(struct pgam_pairs *pairs, int a, int b);
void PgamPairsDestroy(struct pgam_pairs *pairs);
struct pgam_pairs *PgamPairsNew();
void SetPgamEntry14(char *sFilename);
void SetPgamEntry15(char *sFilename);
void PgamSortBrowse14(GtkWidget *widget, gpointer *data);

/* --- functions --- */

/* ClosesWidgetProj
 *
 * Closes a widget it a proper way
 * also properly disposes of the entry boxes.
 */
void CloseWidgetPgamSort(GtkWidget *widget, gpointer data)
{
  /* --- first close the entry widgets --- */
  gtk_widget_destroy(pgamentry0);
  pgamentry0 = NULL;
  gtk_widget_destroy(pgamentry1);
  pgamentry1 = NULL;
  gtk_widget_destroy(pgamentry2);
  pgamentry2 = NULL;
  gtk_widget_destroy(pgamentry3);
  pgamentry3 = NULL;
  gtk_widget_destroy(pgamentry4);
  pgamentry4 = NULL;
  gtk_widget_destroy(pgamentry5);
  pgamentry5 = NULL;
  gtk_widget_destroy(pgamentry6);
  pgamentry6 = NULL;
  gtk_widget_destroy(pgamentry7);
  pgamentry7 = NULL;
  gtk_widget_destroy(pgamentry8);
  pgamentry8 = NULL;
  gtk_widget_destroy(pgamentry9);
  pgamentry9 = NULL;
  gtk_widget_destroy(pgamentry10);
  pgamentry10= NULL;
  gtk_widget_destroy(pgamentry11);
  pgamentry11= NULL;
  gtk_widget_destroy(pgamentry12);
  pgamentry12= NULL;
  gtk_widget_destroy(pgamentry13);
  pgamentry13= NULL;
  gtk_widget_destroy(pgamentry14);
  pgamentry14= NULL;
  gtk_widget_destroy(pgamentry15);
  pgamentry15= NULL;
  gtk_widget_destroy(pgamentry16);
  pgamentry16= NULL;

  gtk_widget_destroy(GTK_WIDGET(data));
  gtk_widget_destroy(widget);
  widget = NULL;
}

/*
 * PgamSortSelectionWindow
 * 
 * Generates a window for determining the pgam sort gating and output types
 */
void PgamSortSelectionWindow()
{
  GtkWidget *localwindow;
  GtkWidget *localbutton;
  GtkWidget *localvbox;
  GtkWidget *localhbox;
  GtkWidget *localhbox2;
  GtkWidget *localmainbox;
  GtkWidget *locallabel;
  GSList *localgroup = NULL;
  GtkWidget *localframe;
  GtkWidget *framevbox;
  char dummystr[80];
  
  /* --- set all gate and output types to zero (false) --- */
 pgamsortgatenone = pgamsortgatetac = pgamsortgatepart= 0;
 pgamsortgategamma = pgamsortoutputhists = pgamsortoutputpart= 0;
 pgamsortoutputtac = pgamsortoutputgg = pgamsortoutputsqr = 0;
 pgamsortoutputggtype = 0;
 pgamsortgateparten =  0;
 pgamsortgategammatypefile = 0;
 pgamsortgategammatypeentry = 1;
 pgamsortgateveto = 0;
 pgamsortgaterequires = 0;
 selfsuppressclovers = 0;

  /* --- spawn the window --- */

  localwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(localwindow), "PGAM Sort Selection");

  /* --- window properties and positions --- */

  gtk_window_set_modal(GTK_WINDOW(localwindow),TRUE);
  gtk_window_set_position(GTK_WINDOW(localwindow),GTK_WIN_POS_CENTER);
  
  /* --- done spawning the window --- */
  
  /* --- make a vbox to put everything in --- */

  localmainbox = gtk_vbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(localwindow),localmainbox);

  /* --- make a box for prompting for the runs to sort --- */
  
  localhbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(localmainbox),localhbox2);
  locallabel = gtk_label_new("     Runs to sort ");
  gtk_container_add(GTK_CONTAINER(localhbox2),locallabel);
  pgamentry8 = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(pgamentry8),"all");
  gtk_container_add(GTK_CONTAINER(localhbox2),pgamentry8);
  locallabel = gtk_label_new(" to ");
  gtk_container_add(GTK_CONTAINER(localhbox2),locallabel);
  pgamentry9 = gtk_entry_new();
  gtk_container_add(GTK_CONTAINER(localhbox2),pgamentry9);
  locallabel = gtk_label_new("                                                                     ");
  gtk_container_add(GTK_CONTAINER(localhbox2),locallabel);
  
  /* --- make a box for prompting for the records to sort --- */
  
  localhbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(localmainbox),localhbox2);
  locallabel = gtk_label_new("     Records to sort ");
  gtk_container_add(GTK_CONTAINER(localhbox2),locallabel);
  pgamentry6 = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(pgamentry6),"all");
  gtk_container_add(GTK_CONTAINER(localhbox2),pgamentry6);
  locallabel = gtk_label_new(" to ");
  gtk_container_add(GTK_CONTAINER(localhbox2),locallabel);
  pgamentry7 = gtk_entry_new();
  gtk_container_add(GTK_CONTAINER(localhbox2),pgamentry7);
  locallabel = gtk_label_new("                                                                     ");
  gtk_container_add(GTK_CONTAINER(localhbox2),locallabel);
  
  /* --- now we need to make the vbox and hbox --- */
  
  localhbox = gtk_hbox_new(FALSE,10);
  gtk_box_pack_start(GTK_BOX (localmainbox),
		     localhbox,FALSE,FALSE,0);
 
  /* --- make a button for sorting raw singles --- */
  
  localbutton = gtk_button_new_with_label("Sort Raw Singles");
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(RawSinglesSort),NULL);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
			    G_CALLBACK(CloseWidgetPgamSort),GTK_OBJECT(localwindow));
  gtk_container_add(GTK_CONTAINER(localmainbox),localbutton);

  /* --- make the button to sort --- */

  localbutton = gtk_button_new_with_label("Sort");
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(PgamSortCallback),NULL);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
			    G_CALLBACK(CloseWidgetPgamSort),GTK_OBJECT(localwindow));
  gtk_container_add(GTK_CONTAINER(localmainbox),localbutton);

  /* --- make a cancel button --- */

  localbutton = gtk_button_new_with_label("Cancel");
  //--ddc aug11  gtk_signal_connect_object(GTK_OBJECT(localbutton),"clicked",
  g_signal_connect_swapped(GTK_OBJECT(localbutton),"clicked",
			    G_CALLBACK(CloseWidgetPgamSort),GTK_OBJECT(localwindow));
  gtk_container_add(GTK_CONTAINER(localmainbox),localbutton);
  
  /* --- make the gate list --- */

  localvbox = gtk_vbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(localhbox),localvbox,FALSE,FALSE,0);
  
  /* --- make the buttons for gating  --- */

  localbutton = gtk_check_button_new_with_label("TAC gate");
  gtk_box_pack_start(GTK_BOX(localvbox),localbutton,FALSE,FALSE,0);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(TACGateToggle),
		     NULL);
  localbutton = gtk_check_button_new_with_label("2D (bannana) Gate");
  gtk_container_add(GTK_CONTAINER(localvbox),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(PartGateToggle),
		     NULL);
  /* --- the button for Particle Energy Gate, needs some way to get
     --- the energies to accept --- */
  localhbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(localvbox),localhbox2);
  localbutton = gtk_check_button_new_with_label("Particle Energy Gate");
  gtk_container_add(GTK_CONTAINER(localhbox2),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(PartEnGateToggle),
		     NULL);
  pgamentry0 = gtk_entry_new();
  gtk_widget_set_size_request(GTK_WIDGET(pgamentry0), 50, 22);
  //gtk_widget_set_usize(GTK_WIDGET(pgamentry0),50,22);
  gtk_container_add(GTK_CONTAINER(localhbox2),pgamentry0);
  locallabel = gtk_label_new(" to ");
  gtk_container_add(GTK_CONTAINER(localhbox2),locallabel);
  pgamentry1 = gtk_entry_new();
  gtk_widget_set_size_request(GTK_WIDGET(pgamentry1), 50, 22);
  //gtk_widget_set_usize(GTK_WIDGET(pgamentry1),50,22);
  gtk_container_add(GTK_CONTAINER(localhbox2),pgamentry1);
  locallabel = gtk_label_new(" keV");
  gtk_container_add(GTK_CONTAINER(localhbox2),locallabel);
  /* --- done with particle energy gate button  --- */

  /* --- deal with the gamma gate option --- */
  localhbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(localvbox),localhbox2);
  localbutton = gtk_check_button_new_with_label("1D gate");
  gtk_container_add(GTK_CONTAINER(localhbox2),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(GammaGateToggle),
		     NULL);
  pgamentry16 = gtk_entry_new();
  gtk_widget_set_size_request(GTK_WIDGET(pgamentry16), 50, 22);
  //gtk_widget_set_usize(GTK_WIDGET(pgamentry16),20,22);
  gtk_container_add(GTK_CONTAINER(localhbox2),pgamentry16);
  gtk_entry_set_text(GTK_ENTRY(pgamentry16),"1");

  localhbox2 = gtk_hbox_new(FALSE,0);
  gtk_box_set_homogeneous(GTK_BOX(localhbox2),0);
  gtk_container_add(GTK_CONTAINER(localvbox),localhbox2);
  locallabel = gtk_label_new("   ");
  gtk_container_add(GTK_CONTAINER(localhbox2),locallabel);
  //localgroup = gtk_radio_button_group(NULL);
  localbutton = gtk_radio_button_new_with_label(NULL,"From ");
  gtk_container_add(GTK_CONTAINER(localhbox2),localbutton);
  //--ddc aug11  localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
  localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(PgamSortGateGammaTypeEntry),
		     NULL);
  pgamentry2 = gtk_entry_new();
  gtk_widget_set_size_request(GTK_WIDGET(pgamentry2), 50, 22);
  //gtk_widget_set_usize(GTK_WIDGET(pgamentry2),50,22);
  gtk_container_add(GTK_CONTAINER(localhbox2),pgamentry2);
  locallabel = gtk_label_new(" to ");
  gtk_container_add(GTK_CONTAINER(localhbox2),locallabel);
  pgamentry3 = gtk_entry_new();
  gtk_widget_set_size_request(GTK_WIDGET(pgamentry3), 50, 22);
  //gtk_widget_set_usize(GTK_WIDGET(pgamentry3),50,22);
  gtk_container_add(GTK_CONTAINER(localhbox2),pgamentry3);
  locallabel = gtk_label_new(" keV                           ");
  gtk_container_add(GTK_CONTAINER(localhbox2),locallabel);

  localhbox2 = gtk_hbox_new(FALSE,0);
  gtk_box_set_spacing(GTK_BOX(localhbox2),0);
  gtk_container_add(GTK_CONTAINER(localvbox),localhbox2);
  locallabel = gtk_label_new("   ");
  gtk_box_pack_start(GTK_BOX(localhbox2),locallabel,TRUE,FALSE,0);
  //gtk_container_add(GTK_CONTAINER(localhbox2),locallabel);
  localbutton = gtk_radio_button_new_with_label(localgroup,"From File:");
  //--ddc aug11  localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
  localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
  gtk_container_add(GTK_CONTAINER(localhbox2),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(PgamSortGateGammaTypeFile),
		     NULL);
  pgamentry4 = gtk_entry_new();
  gtk_container_add(GTK_CONTAINER(localhbox2),pgamentry4);
  localbutton = gtk_button_new_with_label("Browse");
  gtk_container_add(GTK_CONTAINER(localhbox2),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
  		     G_CALLBACK(PgamSortBrowse4),
  		     NULL);
  locallabel = gtk_label_new("     ");
  gtk_container_add(GTK_CONTAINER(localhbox2),locallabel);
 
  /* --- make a check button for clover self suppression --- */
  localbutton = gtk_check_button_new_with_label("Clover Self Suppression.");
  gtk_container_add(GTK_CONTAINER(localvbox),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(CloverSelfSuppressToggle),NULL);

  /* --- make a check button for minimum clover multipolarity --- */
  localhbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(localvbox),localhbox2);
  localbutton = gtk_check_button_new_with_label("Minimum Clover Multipolarity");
  gtk_container_add(GTK_CONTAINER(localhbox2),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(CloverMinMultipolarityToggle),NULL);
  pgamentry12 = gtk_entry_new();
  gtk_container_add(GTK_CONTAINER(localhbox2),pgamentry12);
  gtk_entry_set_text(GTK_ENTRY(pgamentry12),"0");

  /* --- make a button for veto gates --- */
  localhbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(localvbox),localhbox2);
  localbutton = gtk_check_button_new_with_label("Veto Gate");
  gtk_container_add(GTK_CONTAINER(localhbox2),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(PgamGateVetoToggle),NULL);
  pgamentry14 = gtk_entry_new();
  gtk_container_add(GTK_CONTAINER(localhbox2),pgamentry14);
  localbutton = gtk_button_new_with_label("Browse");
  gtk_container_add(GTK_CONTAINER(localhbox2),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(PgamSortBrowse14),
		     NULL);

  /* --- make a button for the requires gates --- */
  localhbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(localvbox),localhbox2);
  localbutton = gtk_check_button_new_with_label("Signals required");
  gtk_container_add(GTK_CONTAINER(localhbox2),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(PgamGateRequiresToggle),NULL);
  pgamentry15 = gtk_entry_new();
  gtk_container_add(GTK_CONTAINER(localhbox2),pgamentry15);
  localbutton = gtk_button_new_with_label("Browse");
  gtk_container_add(GTK_CONTAINER(localhbox2),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		    G_CALLBACK(PgamSortBrowse15),
		    NULL);

  /* --- make buttons for output --- */
  
  localvbox = gtk_vbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(localhbox),localvbox);
  
  /* --- 1-d histogram outputs actually have two mutally exclusive
     --- options, TAC output and "standard" or Energy Output.
     --- therefore we will use a frame and radiobuttons
     --- to select between the two --- */
  localframe = gtk_frame_new("1D Histogram Output");
  gtk_box_pack_start(GTK_BOX(localvbox),localframe,FALSE,FALSE,0);
  framevbox = gtk_vbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(localframe),framevbox);
  
  //localgroup = gtk_radio_button_group(NULL);
  localbutton = gtk_radio_button_new_with_label(NULL,"No 1D Histogram Output");
  gtk_box_pack_start(GTK_BOX(framevbox),localbutton,FALSE,FALSE,0);
  //--ddc aug11  localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
  localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
  localbutton = gtk_radio_button_new_with_label(localgroup,"Energy Output");
  gtk_container_add(GTK_CONTAINER(framevbox),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(OutputHistToggle),
		     NULL);
  //--ddc aug11  localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
  localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
  localbutton = gtk_radio_button_new_with_label(localgroup,"TAC Output");
  gtk_container_add(GTK_CONTAINER(framevbox),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(OutputTACToggle),
		     NULL);
  //--ddc aug11  localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
  localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));


  localhbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(localvbox),localhbox2);
  localbutton = gtk_check_button_new_with_label("E-dE Output  Zoom Fac:");
  gtk_container_add(GTK_CONTAINER(localhbox2),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(OutputPartToggle),
		     NULL);
  pgamentry11 = gtk_entry_new();
  sprintf(dummystr,"%f",1/telfincal);
  gtk_entry_set_text(GTK_ENTRY(pgamentry11),dummystr);
  gtk_container_add(GTK_CONTAINER(localhbox2),pgamentry11);
  /* --- buttons for matrix output --- */
  /* --- since these are mutually exclusive (due to memory considerations)
     --- they are selected with radio buttons and are marked off
     --- from the rest of the selection area by frame --- */
  localframe = gtk_frame_new("Matrix Output");
  gtk_container_add(GTK_CONTAINER(localvbox),localframe);
  framevbox = gtk_vbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(localframe),framevbox);
  //localgroup = gtk_radio_button_group (NULL);
  localbutton = gtk_radio_button_new_with_label(NULL,"No 2D Matrix output");
  gtk_container_add(GTK_CONTAINER(framevbox),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(OutputNoMatrix),
		     NULL);
  //--ddc aug11  localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
  localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
  localbutton = gtk_radio_button_new_with_label(localgroup,"2D Gamma Matrix Output Triangle");
  gtk_container_add(GTK_CONTAINER(framevbox),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(OutputTwdToggle),
		     NULL);
  //--ddc aug11  localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
  localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
  localbutton = gtk_radio_button_new_with_label(localgroup,"2D Gamma Matrix Output Square");
  gtk_container_add(GTK_CONTAINER(framevbox),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(OutputGGSToggle),
		     NULL);
  //--ddc aug11  localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
  localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));

  localhbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(framevbox),localhbox2);
  locallabel = gtk_label_new("      Axis info file");
  gtk_container_add(GTK_CONTAINER(localhbox2),locallabel);
  pgamentry5 = gtk_entry_new();
  gtk_container_add(GTK_CONTAINER(localhbox2),pgamentry5);
  localbutton = gtk_button_new_with_label("Browse");
  gtk_container_add(GTK_CONTAINER(localhbox2),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(PgamSortBrowse5),
		     NULL);

  localbutton = gtk_radio_button_new_with_label(localgroup,"2D Particle - Gamma Matrix Output");
  gtk_container_add(GTK_CONTAINER(framevbox),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(OutputSqrToggle),
		     NULL);
  //--ddc aug11  localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
  localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
  localhbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(framevbox),localhbox2);
  locallabel = gtk_label_new("      Final Calibration of X-axis");
  gtk_container_add(GTK_CONTAINER(localhbox2),locallabel);
  pgamentry10 = gtk_entry_new();
  gtk_container_add(GTK_CONTAINER(localhbox2),pgamentry10);
  sprintf(dummystr,"%f",partfincal);
  gtk_entry_set_text(GTK_ENTRY(pgamentry10),dummystr);
  //partfincal = 1;

  localhbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(framevbox),localhbox2);
  localbutton = gtk_radio_button_new_with_label(localgroup,"2D Gamma Pairs - All Matrix Output");
  //--ddc aug11  localgroup = gtk_radio_button_group(GTK_RADIO_BUTTON(localbutton));
  localgroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(localbutton));
  gtk_container_add(GTK_CONTAINER(localhbox2),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(OutputPairSqrToggle),
		     NULL);
  pgamentry13 = gtk_entry_new();
  gtk_container_add(GTK_CONTAINER(localhbox2),pgamentry13);
  localbutton = gtk_button_new_with_label("Browse");
  gtk_container_add(GTK_CONTAINER(localhbox2),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(PgamSortBrowse13),
		     NULL);


  gtk_widget_show_all(localwindow);	     
}


/* PgamSortCallback
 *
 * a callback function which checks to see if an output was selected and then
 * calls the pgamsort function
 */
void PgamSortCallback(GtkWidget *widget, gpointer *data)
{
  int test;
  float a;
  int i,j,k;
  char dummystr[80];

  if ((pgamsortoutputhists) || (pgamsortoutputpart) || (pgamsortoutputsqr) || (pgamsortoutputgg)
      || (pgamsortoutputtac) || (pgamsortoutputpairsqr)) {
    truesingles = 0;

    /* --- need to interpret the values of the records and runs entries --- */
    /* --- first for runs --- */
    if (strstr(gtk_entry_get_text(GTK_ENTRY(pgamentry8)),"all") != NULL) {
      runmin = 0;
      runmax = 1000000;
    } else {
      test = sscanf(gtk_entry_get_text(GTK_ENTRY(pgamentry8)),"%d",&i);
      if (test == 1) runmin = i;
      test = sscanf(gtk_entry_get_text(GTK_ENTRY(pgamentry9)),"%d",&i);
      if (test == 1){
	runmax = i;
      } else {
	runmax = runmin;
      }
    }

    printf("runs from %d to %d",runmin,runmax);
    /* --- now records --- */

    if (strstr(gtk_entry_get_text(GTK_ENTRY(pgamentry6)),"all") != NULL) {
      sortrecords = 1;
    } else {
      sortrecords  = 0;
      test = sscanf(gtk_entry_get_text(GTK_ENTRY(pgamentry6)),"%d",&i);
      if (test == 1) recordmin = i;
      test = sscanf(gtk_entry_get_text(GTK_ENTRY(pgamentry7)),"%d",&i);
      if (test == 1) recordmax = i;
    }
    
    if (sortrecords == 0) {
      WriteMainText("records from %d to %d");
    }

    /* --- if a gate or output type requires an additional input file
       --- or input, we need to prompt for it --- */
    
    /* --- for instance if the pgamsortgateparten flag is on we need to get
       --- the information from the corresponding entries --- */

    if (pgamsortgateparten) {
      partenergies[0] = partenergies[1] = -1;
      test = sscanf(gtk_entry_get_text(GTK_ENTRY(pgamentry0)),"%f",&a);
      if (test == 1) {
	partenergies[0] = a;
      }
      test = sscanf(gtk_entry_get_text(GTK_ENTRY(pgamentry1)),"%f",&a);
      if (test == 1) {
	partenergies[1] = a;
      }
      sprintf(dummystr,"Accepting particles with an energy range of %f to %f\n",
	      partenergies[0], partenergies[1]);
      WriteMessageText(dummystr);
    }

    /* --- if gamma gating is going to be used, we need to either
       --- get the information from the entries
       --- or get the information for the gates from the file --- */
    if (pgamsortgategamma) {
      if (pgamsortgategammatypeentry) {
	pgamgammagates.num = 1;
	if (pgamgammagates.min == NULL) {
	  pgamgammagates.min = (float *) malloc(sizeof(float) * pgamgammagates.num);
	} else {
	  pgamgammagates.min = (float *) realloc(pgamgammagates.min,
						 sizeof(float) * pgamgammagates.num);
	}
	if (pgamgammagates.max == NULL) {
	  pgamgammagates.max = (float *) malloc(sizeof(float) * pgamgammagates.num);
	} else {
	  pgamgammagates.max = (float *) realloc(pgamgammagates.max,
						 sizeof(float) *pgamgammagates.num);
	}
	test = sscanf(gtk_entry_get_text(GTK_ENTRY(pgamentry2)),"%f",&a);
	if (test == 1) pgamgammagates.min[0] = a;
	test = sscanf(gtk_entry_get_text(GTK_ENTRY(pgamentry3)),"%f",&a);
	if (test == 1) pgamgammagates.max[0] = a;
      }
      if (pgamsortgategammatypefile) {
	PgamReadGammaGate(gtk_entry_get_text(GTK_ENTRY(pgamentry4)));
      }
      test = sscanf(gtk_entry_get_text(GTK_ENTRY(pgamentry16)),"%d",&i);
      if (test == 1) pgam1dgatemin = i;
      else { 
	pgam1dgatemin = 1;
	WriteMainText("Could not determine the minimum number of passes in 1D gate, assuming 1.\n");
      }
    }

    /* --- if we are using a requires gate we should try to read in that gate ---*/

    if (pgamsortgaterequires) {
      requires = PgamReadVeto(gtk_entry_get_text(GTK_ENTRY(pgamentry15)));
    }

    /* --- if we are using a veto gate we should try to read in that gate --- */
    if (pgamsortgateveto) {
      vetos = PgamReadVeto(gtk_entry_get_text(GTK_ENTRY(pgamentry14)));
    }

    /* --- if we are going to do the gamma-gamma square output
       --- we need read in a file to get the detectors
       --- for each axis of the square --- */
    if (pgamsortoutputggtype == 2) {
      PgamReadDetectorInfo(gtk_entry_get_text(GTK_ENTRY(pgamentry5)));
    }


    //--ddc 25jan06 give user chance to use gates w/o 
    //--ddc if (pgamsortoutputpart) {
      test = sscanf(gtk_entry_get_text(GTK_ENTRY(pgamentry11)),"%f",&a);
      if ((test == 1) && (a > 0)) {
	telfincal = 1/a;
      } else {
	telfincal = 1;
      }
    //--ddc }
    //--ddc debug....
    printf("telfincal= %f\n",telfincal);

    if ((pgamsortoutputsqr) || (pgamsortoutputpart) || (pgamsortoutputhists)) {
      test = sscanf(gtk_entry_get_text(GTK_ENTRY(pgamentry10)),"%f",&a);
      if (test == 1) {
	partfincal = a;
      } else {
	partfincal = 1;
      }
    }

    if (minclovermultipolaritytoggle) {
      if (sscanf(gtk_entry_get_text(GTK_ENTRY(pgamentry12)),"%d",&i) == 1) {
	min_clover_multipolarity = i;
      } else {
	min_clover_multipolarity = 0;
      }
      
    }
    if (pgamsortoutputpairsqr) {
      if (sprintf(pgampairfilename,gtk_entry_get_text(GTK_ENTRY(pgamentry13))) == 0) {
	GetMessageDialog("Pair Information Required for Pair Square sort.");
	pgamsortoutputpairsqr = 0;
      }
    }

    PgamSort();
  }
}

/* RawSinglesSort
 *
 * a callback makes sure that all toggles are set to 0, and
 * truesingles is true, then calls PgamSort
 */
void RawSinglesSort(GtkWidget *widget, gpointer *data)
{
  int test;
  int i;
  truesingles = 1;
  pgamsortgatenone = pgamsortgatetac = pgamsortgatepart= 0;
  pgamsortgategamma = pgamsortoutputhists = pgamsortoutputpart= 0;
  pgamsortoutputtac = pgamsortoutputgg = pgamsortoutputsqr = 0;

    if (strstr(gtk_entry_get_text(GTK_ENTRY(pgamentry8)),"all") != NULL) {
      runmin = 0;
      runmax = 1000000;
    } else {
      test = sscanf(gtk_entry_get_text(GTK_ENTRY(pgamentry8)),"%d",&i);
      if (test == 1) runmin = i;
      test = sscanf(gtk_entry_get_text(GTK_ENTRY(pgamentry9)),"%d",&i);
      if (test == 1) runmax = i;
    }

    //    printf("runs from %d to %d",runmin,runmax);
    /* --- now records --- */

    if (strstr(gtk_entry_get_text(GTK_ENTRY(pgamentry6)),"all") != NULL) {
      sortrecords = 1;
    } else {
      sortrecords  = 0;
      test = sscanf(gtk_entry_get_text(GTK_ENTRY(pgamentry6)),"%d",&i);
      if (test == 1) recordmin = i;
      test = sscanf(gtk_entry_get_text(GTK_ENTRY(pgamentry7)),"%d",&i);
      if (test == 1) recordmax = i;
    }
    
    if (sortrecords == 0) {
      //printf("records from %d to %d");
    }
    
    PgamSort();
}

void CloverMinMultipolarityToggle(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    minclovermultipolaritytoggle = 1;
  } else {
    minclovermultipolaritytoggle = 0;
  }
}

void CloverSelfSuppressToggle(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    selfsuppressclovers = 1;
  } else {
    selfsuppressclovers = 0;
  }
}

void PgamSortGateGammaTypeFile (GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    pgamsortgategammatypefile = 1;
    WriteMessageText("Reading 1D from file.\n");
  } else {
    pgamsortgategammatypefile = 0;
  }
}

void PgamGateRequiresToggle(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    pgamsortgaterequires = 1;
  } else {
    pgamsortgaterequires = 0;
  }
}

void PgamGateVetoToggle(GtkWidget *widget, gpointer *data) 
{
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    pgamsortgateveto = 1;
  } else {
    pgamsortgateveto = 0;
  }
}

void OutputPairSqrToggle(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    pgamsortoutputpairsqr = 1;
  } else {
    pgamsortoutputpairsqr = 0;
  }
}

void PgamSortGateGammaTypeEntry (GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON(widget)->active) {
    pgamsortgategammatypeentry = 1;
    WriteMessageText("Reading 1D Gate from prompt.\n");
  } else {
    pgamsortgategammatypeentry = 0;
  }
}

void PgamSortBrowse14(GtkWidget *widget, gpointer *data)
{
  GetFilename("Veto Gate File",0,SetPgamEntry14);
}

void PgamSortBrowse15(GtkWidget *widget, gpointer *data)
{
  GetFilename("Requires Gate File",0, SetPgamEntry15);
}

void PgamSortBrowse4(GtkWidget *widget, gpointer *data)
{
  GetFilename("1D Gate File",0,SetPgamEntry4);
}

void PgamSortBrowse13(GtkWidget *widget, gpointer *data)
{
  GetFilename("Pairs File",0,SetPgamEntry13);
}

void SetPgamEntry13(char *sFilename)
{
  gtk_entry_set_text(GTK_ENTRY(pgamentry13),sFilename);
}

void SetPgamEntry15(char *sFilename)
{
  gtk_entry_set_text(GTK_ENTRY(pgamentry15),sFilename);
}

void SetPgamEntry14(char *sFilename)
{
  gtk_entry_set_text(GTK_ENTRY(pgamentry14),sFilename);
}

void SetPgamEntry4(char *sFilename)
{
  gtk_entry_set_text(GTK_ENTRY(pgamentry4),sFilename);
}

void PgamSortBrowse5(GtkWidget *widget, gpointer *data)
{
  GetFilename("Axis File",0,SetPgamEntry5);
}

void SetPgamEntry5(char *sFilename)
{
  gtk_entry_set_text(GTK_ENTRY(pgamentry5),sFilename);
}

void PartEnGateToggle(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON (widget)->active) {
    pgamsortgateparten = 1;
    WriteMessageText("Using particle energy gate from prompt.\n");
  } else {
    pgamsortgateparten = 0;
    WriteMessageText("Not using particle energy gate.\n");
  }
}

void OutputSqrToggle(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON (widget)->active) {
    pgamsortoutputsqr = 1;
    WriteMessageText("Particle - gamma square matrix output on.\n");
  } else {
    pgamsortoutputsqr = 0;
    WriteMessageText("Particle - gamma square matrix output off.\n");
  }
}

void OutputNoMatrix(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON (widget)->active) {
    /* --- do nothing --- */
  } else {
    pgamsortoutputsqr = 0;
    pgamsortoutputgg = 0;
    pgamsortoutputpairsqr = 0;
  }
}

void OutputGGSToggle(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON (widget)->active) {
    pgamsortoutputgg = 1;
    pgamsortoutputggtype = 2;
    WriteMessageText("Gamma - Gamma square matrix output on.\n");
  } else {
    pgamsortoutputgg = 0;
    pgamsortoutputggtype = 0;
    WriteMessageText("Gamma - Gamma square matrix output off.\n");
  }
}

void OutputTwdToggle(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON (widget)->active) {
    pgamsortoutputgg = 1;
    pgamsortoutputggtype = 1;
    WriteMessageText("Gamma - Gamma symeterized matrix output on.\n");
  } else {
    pgamsortoutputgg = 0;
    pgamsortoutputggtype = 0;
    WriteMessageText("Gamma - Gamma symeterized matrix output off.\n");
  }
}

void OutputTACToggle(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON (widget)->active) {
    pgamsortoutputtac = 1;
    WriteMessageText("TAC histogram output on.\n");
  } else {
    pgamsortoutputtac = 0;
    WriteMessageText("TAC histogram output off.\n");
  }
}

void OutputPartToggle(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON (widget)->active) {
    pgamsortoutputpart = 1;
    WriteMessageText("E - dE output on\n");
  } else {
    pgamsortoutputpart = 0;
    WriteMessageText("E - dE output off.\n");
  }
}

void OutputHistToggle(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON (widget)->active) {
    pgamsortoutputhists = 1;
    WriteMessageText("Energy Histogram output on.\n");
  } else {
    pgamsortoutputhists = 0;
    WriteMessageText("Energy Histogram output on.\n");
  }
}

void TACGateToggle(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON (widget)->active) {
    pgamsortgatetac = 1;
    WriteMessageText("TAC gating on.\n");
  } else {
    pgamsortgatetac = 0;
    WriteMessageText("TAC gating off.\n");
  }
}

void PartGateToggle(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON (widget)->active) {
    pgamsortgatepart = 1;
    WriteMessageText("Particle gating on.\n");
  } else {
    pgamsortgatepart = 0;
    WriteMessageText("Particle gating off.\n");
  }
}

void GammaGateToggle(GtkWidget *widget, gpointer *data)
{
  if (GTK_TOGGLE_BUTTON (widget)->active) {
    pgamsortgategamma = 1;
    WriteMessageText("Gamma gating on.\n");
  } else {
    pgamsortgategamma = 0;
    WriteMessageText("Gamma gating off.\n");
  }
}

/* PgamReadMasterSetup
 *
 * reads in a master setup file for Pgam sorting
 */
void PgamReadMasterSetup(char *sFilename)
{
  FILE *infile;
  int test,i,j,k;
  int tempint[2];
  char tempchar[120];

  datafiletype=0;
  d_max_adcs = 0;

  if ((infile = fopen(sFilename,"r")) != NULL) {
    test = fscanf(infile,"%s",&evtpath);
    if (test != 1) {
      GetMessageDialog("Error Reading Setup.");
      goto donereading;
    }
    test = fscanf(infile,"%d",&num_setups);
    if (test != 1) {
      GetMessageDialog("Error Reading Setup.");
      goto donereading;
    }
    if (num_setups > 0) { // do something if we have to
      if (setupfiles == NULL) {
	setupfiles = (struct setup_file_info *) malloc(num_setups * sizeof(struct setup_file_info));
      } else {
	setupfiles = (struct setup_file_info *) realloc(setupfiles,
							num_setups * sizeof(struct setup_file_info));
      }
      for (i = 0; i < num_setups; i++) {
	test = fscanf(infile,"%d,%d\n%s",&tempint[0],&tempint[1],tempchar);
	if (test != 3) {
	  GetMessageDialog("Error Reading Master Setup file");
	  goto donereading;
	} else {
	  setupfiles[i].runs[0] = Min(tempint[0],tempint[1]);
	  setupfiles[i].runs[1] = Max(tempint[0],tempint[1]);
	  //	  printf("Is the error below this line?\n");
	  sprintf((setupfiles[i].filename),tempchar);
	  sprintf(tempchar,"%d to %d uses %s\n",setupfiles[i].runs[0],
		  setupfiles[i].runs[1],setupfiles[i].filename);
	  WriteMessageText(tempchar);
	}
      }
    } // done reading in setup file names
    /* --- add the ability to get the data file type --- */
    while (fgets(tempchar,120,infile)) { 
    /* --- add the ability to get the data file type --- */
      if (strncasecmp(tempchar,"ev2",3) == 0)
	datafiletype = 1;
      /* --- allow max_adcs to be defined in the .msf file --- */
      if ((strstr(tempchar,"MAXADC=") != NULL)){
	test = sscanf(tempchar,"MAXADC=%d",&i);
	if (test == 1) d_max_adcs = i;
      }
      if ((strstr(tempchar,"maxadc=") != NULL)){
	test = sscanf(tempchar,"maxadc=%d",&i);
	if (test == 1) d_max_adcs = i;
      }
    }
    /* --- better close the file when done --- */
  donereading:
    fclose(infile);

  } else {
    GetMessageDialog("Failed to read Master Setup File.\n");
  }
}

/* PgamReadDetectorInfo
 *
 * Reads in the detectors on the x-axis and the detectors on the y-axis
 * for use with making gamma-gamma squares
 * also makes the appropriate gate matrix
 */
void PgamReadDetectorInfo(char *sFilename)
{
  FILE *infile;
  int test;
  int i,j,k;
  int counter;
  int dummyints[10];
  char dummystr[80];
  
  if ((infile = fopen(sFilename,"r")) != NULL) {
    /* --- first input the number of x-detectors --- */
    fgets(dummystr,80,infile);
    //--ddc jan15 add SLT read for ftaux    test = sscanf(dummystr,"%d",&i);
    test = sscanf(dummystr,"%d %f",&i,&ftaux);
    if (test == 1) {
      numxdets = i;
      ftaux = 1.;
    } else if (test ==2) numxdets = i;
    if (xdets == NULL) {
      xdets = (int *) malloc(sizeof(int) * numxdets);
    } else {
      xdets = (int *) realloc(xdets,sizeof(int) * numxdets);
    }
//--ddc problems with parsing this file, replace with a function.
//    counter = 0;
//    while (counter < numxdets) {
//      if (! fgets(dummystr,80,infile)) break;
//      test = sscanf(dummystr,"%d %d %d %d %d %d %d %d %d %d",
//		    &dummyints[0],&dummyints[1],&dummyints[2],
//		    &dummyints[3],&dummyints[4],&dummyints[5],
//		    &dummyints[6],&dummyints[7],&dummyints[8],
//		    &dummyints[9]);
//      for (i = 0; i < test; i++) {
//	if ((counter + i) < numxdets) {
//	  xdets[counter + i] = dummyints[i];
//	}
//      }
//      counter = counter + test;
//    }
// --ddc and the replacement...
    counter=getints(infile, xdets, numxdets);
    if(counter!=numxdets) GetMessageDialog("Fail on detector xaxis info.\n");

    /* --- now for the y-detectors --- */
    fgets(dummystr,80,infile);
    //--ddc jan15 add SLT read for ftauy    test = sscanf(dummystr,"%d",&i);
    test = sscanf(dummystr,"%d %f",&i,&ftauy);
    if (test == 1) {
      numydets = i;
      ftauy = 1.;
    } else if (test ==2) numydets = i;
    if (ydets == NULL) {
      ydets = ( int *) malloc(sizeof(int) * numydets);
    } else {
      ydets = (int *) realloc(ydets,sizeof(int) * numydets);
    }
// --ddc problems parsing this file, replace with function.
//    counter = 0;
//    while (counter < numydets) {
//      if (! fgets(dummystr,80,infile)) break;
//      test = sscanf(dummystr,"%d %d %d %d %d %d %d %d %d %d",
//		    &dummyints[0],&dummyints[1],&dummyints[2],
//		    &dummyints[3],&dummyints[4],&dummyints[5],
//		    &dummyints[6],&dummyints[7],&dummyints[8],
//		    &dummyints[9]);
//      for (i = 0; i < test; i++) {
//	if ((counter + i) < numydets) {
//	  ydets[counter + i] = dummyints[i];
//	}
//      }
//      counter = counter + test;
//    }
//--ddc and replace with:
    counter=getints(infile, ydets, numydets);
    if(counter!=numydets) GetMessageDialog("Fail on detector yaxis info.\n");
    fclose(infile);
  } else {
    GetMessageDialog("Failed to read detector axis info.\n");
  }
  /* --- now we read it back --- */
  for ( i = (numxdets - 1); i >= 0; i--) {
    sprintf(dummystr,"%d ",xdets[i]);
    WriteMessageText(dummystr);
    if (((i + 1) % 10) == 0) {
      WriteMessageText("\n");
    }
  }
  sprintf(dummystr,"%d detectors on the x-axis: ",numxdets);
  WriteMessageText(dummystr);
  WriteMessageText("\n");
  for (i = (numydets - 1); i >= 0; i--) {
    sprintf(dummystr,"%d ",ydets[i]);
    WriteMessageText(dummystr);
    if (((i + 1) % 10) == 0) {
      WriteMessageText("\n");
    }
  }
  sprintf(dummystr,"%d detectors on the y-axis: ",numydets);
  WriteMessageText(dummystr);
  WriteMessageText("\n");
}


/*
 * PgamReadGammaGate
 *
 * reads in gamma gates from file
 */
void PgamReadGammaGate(char *sFilename)
{
  FILE *infile;
  int test;
  int i,j,k;
  float min,max;
  int counter;
  char dummystr[140];

  if ((infile = fopen(sFilename,"r")) != NULL) {
    /* --- only do stuff if we can open the file --- */
    fgets(dummystr,80,infile);
    test = sscanf(dummystr,"%d",&i);
    if (test == 1) pgamgammagates.num = i;
    if (pgamgammagates.num) {
      /* --- if that read in successfully, then lets allocate (or re-allocate)
	 --- memory for the data --- */
      if (pgamgammagates.min == NULL) {
	pgamgammagates.min = (float *) malloc(sizeof(float) * pgamgammagates.num);
      } else {
	pgamgammagates.min = (float *) realloc(pgamgammagates.min,
					       sizeof(float) * pgamgammagates.num);
      }
      if (pgamgammagates.max == NULL) {
	pgamgammagates.max = (float *) malloc(sizeof(float) * pgamgammagates.num);
      } else {
	pgamgammagates.max = (float *) realloc(pgamgammagates.max,
					       sizeof(float) * pgamgammagates.num);
      }
      for (i = 0; i < pgamgammagates.num; i++) {
	fgets(dummystr,80,infile);
	test = sscanf(dummystr,"%f %f",&min,&max);
	if (test == 2) {
	  pgamgammagates.min[i] = min;
	  pgamgammagates.max[i] = max;
	}
      }
    } 
    fclose(infile);
  } else {
    sprintf(dummystr,"Failed to open %s\n",sFilename);
    GetMessageDialog(dummystr);
  } // end of successfully opened file switch
  /* --- attempt a read back --- */
  for (i = 0; i < pgamgammagates.num;i++) {
    sprintf(dummystr,"1D Gate %d: %.1f to %.1f\n",i,pgamgammagates.min[i],pgamgammagates.max[i]);
    WriteMessageText(dummystr);
  }
}

/*
 * PgamReadSetup
 *
 * reads in the setup file for Pgam sorting
 */
void PgamReadSetup(char *sFilename)
{
  FILE *infile;
  int nchar, test, total;
  int i,j,k;
  char dummystr[120];
  char *cptr;
  int newmemsize;
  int counter;
  int tempints[10];

  /* --- adding a modification on 9/25/2003 to allow
     --- comment lines.  Comment lines will
     --- be those beginning with a #
     --- Method will be to keep calling fgets until the
     --- a non-comment line is reached ---*/

  if ((infile = fopen(sFilename,"r")) != NULL) {
    /* --- initialize everything --- */
    /* --- all globals should have been initialized to null or zero at
       --- the beginning of the program, so we should be able to skip that here
       --- */

    /* --- actually to prevent memory leaks, we should check to see if they still are
       --- NULL, and if they are not free them and set them to NULL --- */

    if (hpges != NULL) {
      free(hpges);
      hpges = NULL;
    }
    if (telescopes != NULL) {
      for (i = 0; i < num_telescopes; i++) {
	//if (telescopes[i].kinmatinfo != NULL) 
	//  free (telescopes[i].kinmatinfo);
      }
      free (telescopes);
      telescopes = NULL;
      //num_telescopes = 0;
    }
    if (clovers != NULL) {
      for (i = 0; i < num_clovers; i++) {
	if (clovers[i].adcs != NULL) {
	  free(clovers[i].adcs);
	  clovers[i].adcs = NULL;
	} 
      }
      free(clovers);
      clovers = NULL;
    }
    if (tacs != NULL) {
      for (i = 0; i < num_tacs; i++) {
	if (tacs[i].gates != NULL) {
	  free(tacs[i].gates);
	  tacs[i].gates = NULL;
	}
      }
      free(tacs);
      tacs = NULL;
    }

    /* --- let's start by reading in the final number of channels --- */

    fgets(dummystr,120,infile);
    while (strncmp("#",dummystr,1) == 0) {
      fgets(dummystr,120,infile);
    }
    if ((sscanf(dummystr,"%d",&fin_chans) != 1)) {
      GetMessageDialog("Error reading the final number of channels.");
      goto stopreading;
    } else printf("fin_chans %d\n",fin_chans);

    /* --- let's get the final calibration (keV per channel), and the beta --- */

    fgets(dummystr,120,infile);
    while (strncmp("#",dummystr,1) == 0) {
      fgets(dummystr,120,infile);
    }
    if ((sscanf(dummystr,"%f,%f",&fin_cal,&beta)) !=2) {
      GetMessageDialog("Error reading the final energy calibration and the beat.");
      goto stopreading;
    } else printf("fin_cal %f, beta %f\n",fin_cal,beta);
    
    /* --- now we get the number of detectors in use --- */

    fgets(dummystr,120,infile);
    while (strncmp("#",dummystr,1) == 0) {
      fgets(dummystr,120,infile);
    }
    if ((sscanf(dummystr,"%d",&num_adcs)) != 1) {
      GetMessageDialog("Error reading the number of ADCs in use.");
      goto stopreading;
    } else printf("num_adcs %d\n",num_adcs);
    
    if (num_adcs > 0) {
      
      /* --- now we need to allocate memory for the detectors and read in the info --- */
      
      hpges = (struct detector_info *) malloc(num_adcs * sizeof(struct detector_info));
      
    /* --- now we read in the information for the adcs being used --- */
      
      for (i = 0; i < num_adcs; i++) {
	fgets(dummystr,120,infile);
	while (strncmp("#",dummystr,1) == 0) {
	  fgets(dummystr,120,infile);
	}
	sprintf(hpges[i].title,dummystr);
	printf("%s\n",hpges[i].title);
	fgets(dummystr,120,infile);
	while (strncmp("#",dummystr,1) == 0) {
	  fgets(dummystr,120,infile);
	}
	
	if (strstr(dummystr,"(") == NULL) {
	  /* --- case where we do not input the overflow --- */
	  if (sscanf(dummystr,"%d,%d,%f",&hpges[i].adc,&hpges[i].channels,&hpges[i].angle) != 3) {
	    GetMessageDialog("Error reading detector information from setup file.");
	    goto stopreading;
	  } else {
	    printf("hpge adc %d, channels %d, angle %f\n",
		   hpges[i].adc,hpges[i].channels,hpges[i].angle);
	    hpges[i].overflow = hpges[i].channels;
	  }
	} else {
	  /* --- case where we do input the overflow --- */
	  if (sscanf(dummystr,"%d,%d(%d),%f",&hpges[i].adc,
		     &hpges[i].channels,&hpges[i].overflow,
		     &hpges[i].angle) != 4) {
	    GetMessageDialog("Error reading detector information from setup file.");
	  } else {
	    printf("hpge adc %d, channels %d, overflow starts at %d, angle %d\n"
		   ,hpges[i].adc,hpges[i].channels,
		   hpges[i].overflow,hpges[i].angle);
	  }
	}
	fgets(dummystr,120,infile);
	while (strncmp("#",dummystr,1) == 0) {
	  fgets(dummystr,120,infile);
	}
	 


	if (sscanf(dummystr,"%f,%f,%f",
		   &hpges[i].calib[0],&hpges[i].calib[1],&hpges[i].calib[2]) != 3) {
	  GetMessageDialog("Error reading detector calibration from setup file.");
	  goto stopreading;
	} else printf("hpge calibration %f %f %f\n",
		      hpges[i].calib[0],hpges[i].calib[1],hpges[i].calib[2]);
      }
    }
    
    /* --- that should be it for the detector info --- */

    /* --- now we shoul read in the number of particle detector telescopes
       --- and then deal with them --- */

   
    fgets(dummystr,120,infile);
    while (strncmp("#",dummystr,1) == 0) {
      fgets(dummystr,120,infile);
    }
    if (sscanf(dummystr,"%d",&num_telescopes) != 1) {
      GetMessageDialog("Error reading the number of particle telescopes.");
      goto stopreading;
    } else printf("number of telescopes %d\n",num_telescopes);

    if (num_telescopes > 0) {

      /* --- now we need to allocate the memory for the particle telescopes --- */
      
      telescopes = (struct telescope_info *) malloc(num_telescopes * sizeof(struct telescope_info));
      
      /* --- now we need to read in the information about the telescopes --- */
      
      for (i = 0; i < num_telescopes; i++) {
	fgets(dummystr,120,infile);
	while (strncmp("#",dummystr,1) == 0) {
	  fgets(dummystr,120,infile);
	}
	sprintf(telescopes[i].totaltitle,dummystr);
	printf("%s\n",telescopes[i].totaltitle);
	fgets(dummystr,120,infile);
	while (strncmp("#",dummystr,1) == 0) {
	  fgets(dummystr,120,infile);
	}
	/* --- if there is a "*" in the line, it is considered to be a "special" and not
	   --- excluded from non E-DE Sorts --- */
	if (strstr(dummystr,"*") != NULL) {
	  telescopes[i].special = 1;
	} else telescopes[i].special = 0;
	/* --- unfortunately to maintain backwards
	   --- compatability we need to make
	   --- the program read two(or three) different
	   --- formats for this line --- */
	/* --- we have to start by determining the format for the E detectors --- */
	/* --- the two formats for the E detectors is differentiated by
	   --- wheither or not the "(" character appears before the "," character --- */
	if ((strstr(dummystr,",") <
	    strstr(dummystr,"(")) || (strstr(dummystr,"(") == NULL)) {
	  /* --- this is the case where the comma occurs before the parentasis --- */
	  /* --- therefore the Eadc is what is before the comma --- */
	  telescopes[i].e_adc = strtol(dummystr,&cptr,10);
	  telescopes[i].num_e_sup = 0;
	} else {
	  /* --- this is the case where the comma occurs after the parentasis --- */
	  /* --- therefore there should be some information about
	     --- other ADC's to monitor for addback --- */
	  /* --- the number before the comma is the E-adc --- */
	  telescopes[i].e_adc = strtol(dummystr,&cptr,10);
	  /* --- the stuff between the parentasis are the supplamental detectors --- */
	  j = 0;
	  while (strncmp(cptr,")",1)) {
	    cptr = cptr + sizeof(char);
	    tempints[j] = strtol(cptr,&cptr,10);
	    j++;
	  }
	  cptr = cptr + sizeof(char);
	  telescopes[i].num_e_sup = j;
	  telescopes[i].e_sup_adc = (int *) malloc(sizeof(int)*telescopes[i].num_e_sup);
	  for (k = 0; k < j; k++) telescopes[i].e_sup_adc[k] = tempints[k];
	}
	cptr = cptr + sizeof(char);
	/* --- ok, now we need to deal with the second half of the array --- */
	/* --- it should be basically the same format as what we just did --- */
	if ((strstr(cptr,",") > strstr(cptr,"(")) && (strstr(cptr,"(") != NULL)) {
	  /* --- this is the case where the comma occurs after the parenthasis --- */
	  telescopes[i].delta_e_adc = strtol(cptr,&cptr,10);
	  j = 0;
	  while (strncmp(cptr,")",1) != 0) {
	    cptr = cptr + sizeof(char);
	    tempints[j] = strtol(cptr,&cptr,10);
	    j++;
	  }
	  cptr = cptr + sizeof(char);
	  telescopes[i].num_de_sup = j;
	  telescopes[i].de_sup_adc = (int *) malloc(sizeof(int) * telescopes[i].num_de_sup);
	  for (k = 0; k < j; k++) telescopes[i].de_sup_adc[k] = tempints[k];
	} else {
	  /* --- this is the case where the comma occurs before the parenthasis --- */
	  telescopes[i].delta_e_adc = strtol(cptr,&cptr,10);
	  telescopes[i].num_de_sup = 0;
	}
	cptr = cptr + sizeof(char);
	/* --- now let's get the relative gain --- */
	/* --- we actually need to add an optional constant, linear, and quadratic calibration --- */
	if (sscanf(cptr,"%f",&telescopes[i].gain) == 1)
	  printf(" gain: %f ",telescopes[i].gain);
	/* --- Let's see if we have the supplamental calibrations.  We will check for a "(" --- */
	if ((strstr(cptr,",") <
	     strstr(cptr,"(")) || (strstr(cptr,"(") != NULL)) {
	  cptr = strstr(cptr,"(");
	  /* --- if we are here than there should be a supplamental calibration --- */
	  /* --- The first number should be the constant, second linear, third quadratic --- */
	  /* --- in theory they should be seporated by commas, so we will assume that --- */
	  sscanf(cptr,"(%f,%f,%f)",&telescopes[i].sup_cal[0],&telescopes[i].sup_cal[1],&telescopes[i].sup_cal[2]);
	  printf("sup_cal: %f %f %f ",telescopes[i].sup_cal[0],telescopes[i].sup_cal[1],telescopes[i].sup_cal[2]);
	  cptr = strstr(cptr,")") + sizeof(char);
	} else {
	  telescopes[i].sup_cal[0] = 0;
	  telescopes[i].sup_cal[1] = 1;
	  telescopes[i].sup_cal[2] = 0;
	} 
	/* --- now we need to get the location for the total ADC. --- */
	cptr = strstr(cptr,",");
	  cptr = cptr + sizeof(char);
	  if (sscanf(cptr,"%d",
		   &telescopes[i].totaladc) != 1) {
	    GetMessageDialog("Error reading Telescope information from setup file.");
	    goto stopreading;
	} else printf("telescopes e_adc: %d delta_e_adc: %d gain: %f\n",
		      telescopes[i].e_adc,telescopes[i].delta_e_adc,
		      telescopes[i].gain);
      }
    }
    
    /* --- that should be it for telescope info --- */
    printf("Finished reading telescope info.\n");

    /* --- now we should read in the number of multi-crystal detectors (gamma-type) --- */

    fgets(dummystr,120,infile);
    while (strncmp("#",dummystr,1) == 0) {
      fgets(dummystr,120,infile);
    }
    if (sscanf(dummystr,"%d",&num_clovers) != 1) {
      GetMessageDialog("Error reading the number of clover detectors.");
      goto stopreading;
    } else printf("number of clovers %d\n",num_clovers);

    if (num_clovers > 0) {

      /* --- now we need to allocate memory for the multi-crystal detectors --- */
      
      clovers = 
	(struct multi_hpge_info *) malloc(num_clovers * sizeof(struct multi_hpge_info));

      /* --- now we need to read in the information about each clover --- */
      for (i = 0; i < num_clovers; i++) {
	fgets(dummystr,120,infile);
	while (strncmp("#",dummystr,1) == 0) {
	  fgets(dummystr,120,infile);
	}
	sprintf(clovers[i].totaltitle,dummystr);
	printf("%s\n",clovers[i].totaltitle);
	/* --- now we need to know how many crystals are in this detector --- */
	/* --- and the virtual adc for the sum --- */
	fgets(dummystr,120,infile);
	while (strncmp("#",dummystr,1) == 0) {
	  fgets(dummystr,120,infile);
	}
	printf(dummystr);
	if (sscanf(dummystr,"%d,%d",&clovers[i].num_adcs,&clovers[i].totadc) != 2) {
	  GetMessageDialog("Error reading number of crystals in clovers.");
	  goto stopreading;
	} else printf("clover: num crystals %d sum adc: %d\n",
		      clovers[i].num_adcs,clovers[i].totadc);
	/* --- now we need to read the input file until we have the adcs for each crystal --- */
	/* --- first we have to allocate memory for the crystal info --- */
	clovers[i].adcs = (int *) malloc(sizeof(int) * clovers[i].num_adcs);
	counter = 0;
	printf("clover: adcs:  ");
	while (counter < clovers[i].num_adcs) {
	  fgets(dummystr,120,infile);
	  while (strncmp("#",dummystr,1) == 0) {
	    fgets(dummystr,120,infile);
	  }
	  test =  sscanf(dummystr,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
			 &tempints[0],&tempints[1],&tempints[2],&tempints[3],&tempints[4],
			 &tempints[5],&tempints[6],&tempints[7],&tempints[8],&tempints[9]);
	  for (j = 0; j < test; j++) {
	    clovers[i].adcs[j + counter] = tempints[j];
	    printf("%d ",clovers[i].adcs[j + counter]);
	  }
	  counter = counter + test;
	}
	printf("\n");
      }

    }
    /* --- that should be the end of reading in clovers --- */

    /* --- now we need to get the number of tacs --- */

    fgets(dummystr,120,infile);
    while (strncmp("#",dummystr,1) == 0) {
      fgets(dummystr,120,infile);
    }
    if (sscanf(dummystr,"%d",&num_tacs) != 1) {
      GetMessageDialog("Error reading the number of tacs.");
      goto stopreading;
    } else printf("number of tacs: %d\n",num_tacs);

    if (num_tacs > 0) {

      /* --- let's allocate memory for the tacs --- */
      
      tacs = (struct tac_info *) malloc(num_tacs * sizeof(struct tac_info));

      /* --- now let's read in the information about the tacs --- */
      for (i = 0; i < num_tacs; i++) {
	fgets(dummystr,120,infile);
	while (strncmp("#",dummystr,1) == 0) {
	  fgets(dummystr,120,infile);
	}
	sprintf(tacs[i].title,dummystr);
	printf("%s\n",tacs[i].title);
	fgets(dummystr,120,infile);
	while (strncmp("#",dummystr,1) == 0) {
	  fgets(dummystr,120,infile);
	}
	if(sscanf(dummystr,"%d",&tacs[i].adc) != 1) {
	  GetMessageDialog("Error reading the adc of a tac.");
	  goto stopreading;
	} else printf("Tac adc: %d\n",tacs[i].adc);
	fgets(dummystr,120,infile);
	while (strncmp("#",dummystr,1) == 0) {
	  fgets(dummystr,120,infile);
	}
	if (sscanf(dummystr,"%d",&tacs[i].num_adcs) != 1) {
	  GetMessageDialog("Error reading the number of adcs used with the tac.");
	  goto stopreading;
	} else printf("number of adcs with in tac %d\n",tacs[i].num_adcs);
	/* --- now we have to allocate memory to put the gate info into --- */
	tacs[i].gates = (struct tac_gate_info *) 
	  malloc(tacs[i].num_adcs * sizeof(struct tac_gate_info));
	for (j = 0; j < tacs[i].num_adcs; j++) {
	fgets(dummystr,120,infile);
	while (strncmp("#",dummystr,1) == 0) {
	  fgets(dummystr,120,infile);
	}
	  if (sscanf(dummystr,"%d,%d,%d",
		     &tacs[i].gates[j].adc,&tacs[i].gates[j].min,&tacs[i].gates[j].max) != 3) {
	    GetMessageDialog("Error reading tac gate info from setup file.");
	    goto stopreading;
	  } else printf("TAC gate adc: %d, min: %d, max: %d\n",tacs[i].gates[j].adc,
			tacs[i].gates[j].min,tacs[i].gates[j].max);
	}
      }

    } else {
      tacs = (struct tac_info *) malloc(sizeof(struct tac_info));
      sprintf(tacs[0].title,"");
      tacs[0].num_adcs = 0;
      tacs[0].adc = -1;
    }
    /* --- done reading tac info --- */

    /* --- that should be it for the setup file --- */

    /* --- actually not quite, the user could have entered a
       --- option.  Therefore we should scan the rest of the file
       --- for these options --- */

    while (fgets(dummystr,120,infile) != 0) {
      while (strncmp("#",dummystr,1) == 0) {
	fgets(dummystr,120,infile);
      }
      /* --- we'll use the # character to denote a comment --- */
      if (strncmp(dummystr,"#",1) != 0) {
	/* --- then this is not a comment --- */
	if (strncmp(dummystr,"EDESIZE",7) == 0) {
	  if (sscanf(dummystr,"EDESIZE %d",&i) == 1)
	    edesize = i;
	  else
	    WriteMainText("Error reading EDESIZE.\n");
	}
      }
    }
 
  stopreading:
    fclose(infile);  // have to close the file if we opened it
    printf("Done with the setup file.\n");
  } // end file reading condition
  //--ddc 18aug06 What NO ERROR HANDLING FOR FILE OPEN! :(
  //I'm adding it in this else.
  else { 
    printf("\nFatal error in opening setupfile %s!\n",sFilename);
    exit(-1);
  }
  
}

/* PgamSort
 *
 * does a particle-gamma sort thing according to what has already been entered
 */
void PgamSort ()
{
  /* --- local variables --- */
  float temphist[254][8192];
  int temphistsize[254];
  int index;
  FILE *infile;
  int tempuint;
  int reads, writes;
  int tellook;
  long int fileLen = 0;
  struct stat filebuffer;
  int i,j,k,l,m,n,o,p,q,r,x,y,z;
  int kx,ky;
  char filename[80];
  char dummystr[80];
  char tempstr[80];
  short int *buffer;
  guchar *ev2buffer;
  guchar extbuffer[1028];
  int extbufferflag;
  int howmuch;
  const int EV2BUFFERSIZE = 8192;
  int max_adcs;
  int *adc_map,*inverse_adc_map;
  int test;
  int biggest_adc;
  int telescopecheck,detrec;
  int irec,stillreading,numshorts,passtacgate;
  int buffersize[2];
  short int head;
  int numadc,ichan;
  float randomnum;
  short int event_adcs[256];
  int event_channel[256];
  uint *clover_gates;
  int *clover_hit,*clover_hit2;
  unsigned long long *telescope_gate,*detelescope_gate;
  unsigned long long *special_telescope_gate;
  int echan,dechan;
  int passpartgate;
  int isize;
  int newm,newp,newm2,newp2;
  float anewm,anewp,frac,tm,tp,anewm2,anewp2,frac2,tm2,tp2;
  int newm3,newp3,newm4,newp4;
  float anewm3,anewp3,frac3,tm3,tp3,anewm4,anewp4,frac4,tm4,tp4;
  int newme,newpe,newmde,newpde;
  float anewme,anewpe,frace,tme,tpe,anewmde,anewpde,fracde,tmde,tpde;
  //--ddc jan15 additional variables for SLT's doppler correction.
  float anewym,anewyp;
  float uchanm[256],uchanp[256],stangle[256],uchanym[256],uchanyp[256];
  int passgammagate;
  int *tacgate;
  int tempmin,tempmax;
  int *xaxisgate,*yaxisgate;
  float tempfloatm,tempfloatp;
  unsigned long int progbarlastbytes,progbarpos;
  int *not_a_sum_hit;
  int ll;
  int passedpairs;
  int adc1,adc2,adc3,adc16,adc14; // for diagnostic purposes
  int adc9,adc10,adc11,adc12;    //for diagnostic purposes
  int most_telescopes; // largest # of telescopes in any run
  int *detector_info_map;  // a map of the in-memory adc location (adc_map) 
                           // to the hpge which stores the relevant info.
  int max_channels; // will be the most channels in an adc.  used
                        //for tac memory allocation and raw singles sort
  int telescopes_hit;
  int clover_multiplicity;
  uint *pair_flags;
  struct pgam_pairs *pairs;
  int num_scopes; // number of telescopes that fired in a particular event;
//--ddc 24jul13 oh no.  Many of the fixed dimension arrays were not appropriately
// sized for larger data acquistion systems.  replaced all arrays dimensioned with
// 64 dimension 264.
  int scope_adcs[254];
  float scope_newm[254],scope_newp[254];
  int scope_x[254],scope_y[254];
  int scope_addr[254];
  int g_num;
  float g_newm[254];
  float g_newp[254];
  int e_sup_hits, de_sup_hits;
  int e_sup_ent[254];
  int de_sup_ent[254];
//--ddc xaxiscount,yaxiscount were UNINITIALIZED VALUES WHICH ARE USED AS INDEXES!!
//I'm also adding xadcs and yadcs arrays to map adcs to each axis to prevent the 
//same adc being on each axis in square sorts.
//
  int xaxiscount=0,yaxiscount=0; 
  int xadcs[254],yadcs[254];  
  //--ddc 22jun06 I'm adding some arrays to keep track of which gamma's are in a gate!
  int xgated[254],ygated[254], xgatedpointer, ygatedpointer;
  //--ddc 3may07 I'm adding a variable to keep the upper dimension of gate arrays
  //for singles sorting.
  int singles_gates;


  float nuchanxm[254],nuchanxp[254];
  float nuchanym[254],nuchanyp[254];

  /* --- we only get this far going through setup windows --- */

  /* --- initialization of local variables --- */
  abortflag = 0;
  adc_map = inverse_adc_map = NULL;
  //event_adcs = NULL;
  pairs = NULL;
  pair_flags = NULL;
  //  event_channel = 
  clover_gates = NULL;
  telescope_gate = NULL;
  detelescope_gate = NULL;
  special_telescope_gate = NULL;
  buffer = NULL;
  ev2buffer = NULL;
  detector_info_map = NULL;
  //uchanm = NULL;
  //uchanp = NULL;
  clover_hit2 = NULL;
  reads = 0;
  not_a_sum_hit = NULL;
  most_telescopes = 0;
  passpartgate = 1;
  writes = 0;
  for (i = 0; i < 254; i++) {
  //  temphist[i] = NULL;
    temphistsize[i] = 0;
  }
  xaxisgate = NULL;
  yaxisgate = NULL;
  tacgate = NULL;
  clover_hit = NULL;
  /* --- allocate memory and define stuff for the structures --- */
  /* --- in order to do this we need to know the maximum number of adcs we will have to 
     --- use in any of the setup files */
  /* --- need to create a mapping for the adc number to the adc number in memory --- */
  /* --- this mapping should be used when writing the 1D histograms
     --- to memory */
  /* --- moved so that this is initialized when we read in a .msf file --- */
  /*   max_adcs = 0; */
  adc_map = NULL;
  max_channels = 0;
  for (i = 0; i < num_setups; i++) {
    PgamReadSetup(setupfiles[i].filename);
    if (num_telescopes > most_telescopes) most_telescopes = num_telescopes;
    if (num_adcs != 0) {
      for (j = 0; j < num_adcs; j++) {
	if (hpges[j].channels > max_channels) max_channels = hpges[j].channels;
	if (adc_map == NULL) {
	  adc_map = (int *) malloc(sizeof(int));
	  adc_map[0] = hpges[j].adc;
	  max_adcs = 1;
	} else {
	  test = 0;
	  for (k = 0; k < max_adcs; k++) {
	    if (hpges[j].adc == adc_map[k]) test = 1;
	  }
	  if (test == 0) {
	    max_adcs++;
	    adc_map = (int *) realloc(adc_map, max_adcs * sizeof(int));
	    adc_map[max_adcs - 1] = hpges[j].adc;
	  }
	}
      }
      for (j = 0; j < num_clovers; j++) {
	if (adc_map == NULL) {
	  adc_map = (int *) malloc(sizeof(int));
	  adc_map[0] = clovers[j].totadc;
	  max_adcs = 1;
	} else {
	  test = 0;
	  for (k = 0; k < max_adcs; k++) {
	    if (clovers[j].totadc == adc_map[k]) test = 1;
	  }
	  if (test == 0) {
	    max_adcs++;
	    adc_map = (int *) realloc(adc_map, max_adcs * sizeof(int));
	    adc_map[max_adcs - 1] = clovers[j].totadc;
	  }
	}
      }
      for (j = 0; j < num_telescopes; j++) {
	if (adc_map == NULL) {
	  adc_map = (int *) malloc(sizeof(int));
	  adc_map[0] = telescopes[j].totaladc;
	  max_adcs = 1;
	} else {
	  test = 0;
	  for (k = 0; k < max_adcs; k++) {
	    if (telescopes[j].totaladc == adc_map[k]) test = 1;
	  }
	  if (test == 0) {
	    max_adcs++;
	    adc_map = (int *) realloc(adc_map, max_adcs * sizeof(int));
	    adc_map[max_adcs - 1] = telescopes[j].totaladc;
	  }
	}
      }
    }
  }

  /* --- if d_max_adcs is defined (and greater than max_adcs) we need to set max_adcs to that --- */
  //--ddc  if (d_max_adcs > max_adcs)
  //--ddc    max_adcs = d_max_adcs;

  /* --- guess what, that was the reverse of the mapping that we wanted --- */
  /* --- first let's sort that and then remap it --- */
  /* --- sorting mapping --- */
  qsort(adc_map,max_adcs,sizeof(int),CompareInts);
  /* --- done sorting map --- */
  /* --- invert mapping --- */
  
  inverse_adc_map = (int *) malloc(sizeof(int) * (max_adcs));
  //--ddc biggest_adc = 0;
  if (d_max_adcs > max_adcs) biggest_adc=d_max_adcs;
  else biggest_adc=0;

  for (i = 0; i < max_adcs; i++) {
    inverse_adc_map[i] = adc_map[i];
    if (inverse_adc_map[i] > biggest_adc) biggest_adc = inverse_adc_map[i];
  }
  adc_map = (int *) realloc (adc_map, sizeof(int) * (biggest_adc+1));
  for (i = 0; i <= biggest_adc; i++) {
    adc_map[i] = -1;
    //--ddc need to initialize the arrays for xadcs and yadcs as well...
    xadcs[i]=0;  yadcs[i]=0;
  }
  for (i = 0; i < max_adcs; i++) {
    adc_map[inverse_adc_map[i]] = i;
  }

  /* --- done inverting mapping --- */

  /* --- if we want to do true singles sorting, we need to do a 1-1 mapping --- */
  /* --- however, since initialization is done for each setup file,
     --- we need to do this only just before sorting each setup file --- */

  /* --- allocating histogram memory --- */

  /* --- true singles are handled seperately (and more easily) than all other cases --- */
  if ((pgamsortoutputtac)) { // this is the case for tac output
    /* --- allocate memory for the histograms --- */
    /* --- for tac output we want the number of
       --- final channels to be the number
       --- of channels for the tac adc --- */
    /* --- let's get that information --- */
    /* --- oh, crap! we have to get that earlier --- */
    /* --- it is easier to get the max_channels than max_tac_channels --- */
    fin_chans = max_channels;
    for (i = 0; i < max_adcs; i++ ){
      ObliterateHistogram(i);
      histloc[i] = malloc((sizeof(int) * max_channels));
      histsize[i] = max_channels;
      histid[i] = i;
      sprintf(field1[i],"");
      sprintf(field2[i],"adc %d",i);
      sprintf(field3[i],"");
      /* --- now initialize the new histogram --- */
      for (j = 0; j < max_channels; j++) {
	*(histloc[i] + j) = 0;
      }
      /* --- now we should be done making the histograms --- */
      
    } // end histogram creation loop
  } else {
    if (truesingles) {
      /* --- allocate memory for the histograms --- */
      fin_chans = max_channels;
      for (i = 0; i < (biggest_adc +1) ; i++ ) {
	ObliterateHistogram(i);
	histloc[i] = malloc((sizeof(int) * max_channels));
	histsize[i] = max_channels;
	histid[i] = i;
	sprintf(field1[i],"");
	sprintf(field2[i],"adc %d",i+1);
	sprintf(field3[i],"");
	/* --- now initialize the new histogram --- */
	for (j = 0; j < max_channels; j++) {
	  *(histloc[i] + j) = 0;
	}
      }
    }
      /* --- allocating memory for all other cases --- */
      /* --- first if the pgamsortoutputhists is true --- */
      if (pgamsortoutputhists) {
	/* --- in the case of singles we will need sufficient histograms and a big data struct --- */
	for (i = 0; (i <= max_adcs);  i++ ) { // histogram creation loop
	  ObliterateHistogram(i);
	  histloc[i] = malloc((sizeof(int) * fin_chans));
	  histsize[i] = fin_chans;
	  histid[i] = i;
	  sprintf(field1[i],"");
	  sprintf(field2[i],"",i);
	  sprintf(field3[i],"");
	  /* --- initialize histogram --- */
	  for (j = 0; j < histsize[i]; j++) {
	    *(histloc[i] +j) = 0;
	  }
	} // end creation histogram loop
	/* --- we need the "float" type temporary histograms as well --- */
	for (i = 0; i <= max_adcs; i++) {
	  //--ddc nov14 (wrong!)	  temphistsize[i] = max_channels;
	  temphistsize[i] = 8192;
	  //temphist[i] = (float *) malloc( sizeof(float) * max_channels);
	  //--ddc nov14 match with size..	  for (j = 0; j < max_channels; j++) {
	  for (j = 0; j < temphistsize[i]; j++) {
	    *(temphist[i] + j) = 0;
	  }      
	}
      }
      /* --- now if pgamsortoutputpart is true --- */
      if (pgamsortoutputpart) {
	if (big_data_info == NULL) {
	  big_data_info = (struct bigdata **) malloc(sizeof(struct bigdata *) * most_telescopes);
	} else {
	  big_data_info = (struct bigdata **) realloc(big_data_info,
						     sizeof(struct bigdata *) *
						     (pgammax + most_telescopes));
	}
	for (j = pgammax; j < (pgammax + most_telescopes); j++) {
	  big_data_info[j] = BigDataStructNew(edesize,edesize);
	}
	for (j = 0; j < most_telescopes; j++) {
	  sprintf(twodtitles[j+pgammax],telescopes[j].totaltitle);
	}
	pgammax += most_telescopes;
      }
      /* --- if pgamsortoutputsqr is true --- */
      /* --- also if pgamsortoutputpairsqr --- */
      if ((pgamsortoutputsqr) || (pgamsortoutputpairsqr)) {
	/* --- in addition to the obvious stuff, we need
	   --- write the information which will be used 
	   --- when writing to a file --- */
	pgammatrixdata.headbuffer[0] = 12;
	pgammatrixdata.headbuffer[1] = 12;
	pgammatrixdata.size = fin_chans;
	pgammatrixdata.what = 0;
	pgammatrixdata.type = 0;  // type is 0 for a "real" square
	pgammatrixdata.databuffer[0] = pgammatrixdata.databuffer[1] =
	  sizeof(float) * pgammatrixdata.size * pgammatrixdata.size;
	if (pgammatrixdata.data == NULL) {
	  pgammatrixdata.data = (float *) malloc(pgammatrixdata.databuffer[0]);
	} else {
	  pgammatrixdata.data = (float *) realloc(pgammatrixdata.data,
						  pgammatrixdata.databuffer[0]);
	}
	for ( l = 0 ; l < (pgammatrixdata.databuffer[0] / (float) sizeof(float)); l++) {
	  *(pgammatrixdata.data + l) = 0;
	}
	/* --- because the adc_map could change between setup files, we should
	   --- set up the pair_flags and pairs when we get there --- */
      }
      /* --- if pgamsortoutputgg is true --- */
      if (pgamsortoutputgg) {
	/* --- in addition to the obvious stuff, we need to 
	   --- write the information which will be used 
	   --- when writing to a file  --- */
	/* --- this should be handled slightly differently if we
	   --- are outputting to a square or triangle --- */
	pgammatrixdata.headbuffer[0] = 12;
	pgammatrixdata.headbuffer[1] = 12;
	pgammatrixdata.size = fin_chans;
	pgammatrixdata.what = 0;
	switch (pgamsortoutputggtype) {
	case 1:
	pgammatrixdata.type = 3;  // type is 3 for a "real" triangle
	pgammatrixdata.databuffer[0] = pgammatrixdata.databuffer[1] = 
	  (float) sizeof(float) / (float) 2 * (float) pgammatrixdata.size * 
	  (float) (pgammatrixdata.size + 1);
	isize = 2 * pgammatrixdata.size + 1;
	if (pgammatrixdata.data == NULL) {
	  pgammatrixdata.data = (float *) malloc(pgammatrixdata.databuffer[0]);
	} else {
	  pgammatrixdata.data = (float *) realloc(pgammatrixdata.data,
						  pgammatrixdata.databuffer[0]);
	}
	break;
        case 2:
          //--ddc debug 06-20-05, xaxisgate and yaxisgate arrays use 
          //adc numbers as indexes, SO, arrays must be dimensioned for
          //the biggest_adc PLUS ONE.
	if (xaxisgate == NULL) {
	  xaxisgate = (int *) malloc(sizeof(int) * (biggest_adc+1));
	} else {
	  xaxisgate = (int *) realloc(xaxisgate,sizeof(int) * (biggest_adc+1));
	}
	if (yaxisgate == NULL) {
	  yaxisgate = (int *) malloc(sizeof(int) * (biggest_adc+1));
	} else {
	  yaxisgate = (int *) realloc(yaxisgate,sizeof(int) * (biggest_adc+1));
	}
	for (l = 0; l < biggest_adc+1; l++) {
	  xaxisgate[l] = 0;
	  yaxisgate[l] = 0;
	}
	for ( l = 0; l < numxdets; l++) {
	  xaxisgate[xdets[l]] = 1;
	}
	for ( l = 0; l < numydets; l++) {
	  yaxisgate[ydets[l]] = 1;
	}
	pgammatrixdata.type = 0;  // type is 0 for a "real" square
	pgammatrixdata.databuffer[0] = pgammatrixdata.databuffer[1] = 
	   sizeof(float) * pgammatrixdata.size * pgammatrixdata.size;
	isize = 2 * pgammatrixdata.size + 1;
	if (pgammatrixdata.data == NULL) {
	  pgammatrixdata.data = (float *) malloc(pgammatrixdata.databuffer[0]);
	} else {
	  pgammatrixdata.data = (float *) realloc(pgammatrixdata.data,
						  pgammatrixdata.databuffer[0]);
	}
	break;
        }
      for ( l = 0 ; l < ((float) pgammatrixdata.databuffer[0] / (float) sizeof(float)); l++) {
	*(pgammatrixdata.data + l) = 0;
      }
    }
  }
  
  fileLen = 0;
  StartSortProgress();
  for (i = 0; i < num_setups; i++) {
    tempmin = Max(runmin,setupfiles[i].runs[0]);
    tempmax = Min(runmax,setupfiles[i].runs[1]);
    for (j = tempmin; j <= tempmax; j++) {
      switch (datafiletype) {
      case 1:
	sprintf(filename,"%srun%d.ev2",evtpath,j);
	break;
      default:
	sprintf(filename,"%srun%d.evt",evtpath,j);
	break;
      }
      stat(filename,&filebuffer);
      fileLen = fileLen + filebuffer.st_size;
    }
  }
  progbarlastbytes = 0;
    

  for (i = 0; i < num_setups; i++) { // go through all the setup files
    /* --- start by reading the file --- */
    PgamReadSetup(setupfiles[i].filename);
    telescopes_hit = 0;
    /* --- need to map from the locations in memory for the adc's to
       --- the location in memory for the calibration info --- */
    //--ddc nov14 shouldn't happen... but  Repeat what was done for adc_map
    //--ddc nov14 so out-of-bounds access less likely.
    //
    if (detector_info_map == NULL) {
      //--ddc nov14      detector_info_map = (int *) malloc(sizeof(int) * max_adcs);
      //--ddc jan17.. still a problem, parentheses!
      //      detector_info_map = (int *) malloc(sizeof(int) * biggest_adc+1);
      detector_info_map = (int *) malloc(sizeof(int) * (biggest_adc+1));
    } else {
      //--ddc nov14 detector_info_map = (int *) realloc(detector_info_map,sizeof(int) * max_adcs);
      //--ddc jan17.. still a problem, parentheses!
      //      detector_info_map = (int *) realloc(detector_info_map,sizeof(int) * biggest_adc+1);
      detector_info_map = (int *) realloc(detector_info_map,sizeof(int) * (biggest_adc+1));
    }
    //--ddc nov14    for ( k = 0; k < max_adcs; k++) {
    for ( k = 0; k < biggest_adc+1; k++) {
      detector_info_map[k] = -1;
    }
      for ( k = 0; k < max_adcs; k++) {
	for (l = 0; l < num_adcs; l++) {
	  if (adc_map[hpges[l].adc] == k) detector_info_map[k] = l;
	}
      }
      printf("Made detector info map.\n");

      /* --- need to set up the clover gates and telescope gates --- */

      if (clover_gates == NULL) {
	clover_gates = (int *) malloc(max_adcs * sizeof(int));
      } else {
	clover_gates = (int *) realloc(clover_gates,max_adcs * sizeof(int));
      }
      for ( k = 0; k < max_adcs; k++) {
	clover_gates[k] = 0;
      }
      for (k = 0; k < num_clovers; k++) {
	for (l = 0; l < clovers[k].num_adcs; l++) {
	  if (adc_map[clovers[k].adcs[l]] != -1) {
	    clover_gates[adc_map[clovers[k].adcs[l]]] |= ((uint) 1 << k);
	    //	    clover_gates[adc_map[clovers[k].adcs[l]]] = k + 1;
	    // the value of clover_gates will be a positive real integer
	    // for adc's that map to a clover, the clover
	    // they map to is the one stored at clovers[k-1];
	}
      }
    }
    printf("Made clover gate map.\n");
    for (k = 0; k < max_adcs; k++) {
      printf(" %d ",clover_gates[k]);
    }
    printf("\n");
    //    scanf("%d",&n);

    /* --- now if the adc that fired is in a multi-crystal 
       --- detector it's value for clover_gates[adc_map[adc]]
       --- will be 1. --- */

    /* --- let's do the same thing for the particle telescope
       --- that means that telescope_gate[adc_map[adc]] will be 
       --- 1 for adc's in the telescope --- */
    
    if (telescope_gate == NULL) {
      telescope_gate = (unsigned long long *) malloc((max_adcs+1) * sizeof(unsigned long long));
    } else {
      telescope_gate = (unsigned long long *) realloc(telescope_gate,(max_adcs+1) * sizeof (unsigned long long));
    }
    for ( k = 0; k <= max_adcs; k++) {
      telescope_gate[k] = 0;
    }
    for (k = 0; k < num_telescopes; k++) {
      /* --- let's just do this for the e's to avoid double counting --- */
      if (adc_map[telescopes[k].e_adc] != -1) {
	telescope_gate[adc_map[telescopes[k].e_adc]] |= (unsigned long long) ((unsigned long long)1 << k);
	//	telescope_gate[adc_map[telescopes[k].delta_e_adc]] = k+1;
      }
    }

    if (special_telescope_gate == NULL) {
      special_telescope_gate = (unsigned long long *) malloc((max_adcs + 1) * sizeof(unsigned long long));
    } else {
      special_telescope_gate = (unsigned long long *) realloc(special_telescope_gate,
					       (max_adcs + 1) * sizeof(unsigned long long));
    }
    for (k = 0; k < max_adcs; k++) special_telescope_gate[k] = 0;
    for (k = 0; k < num_telescopes; k++) {
      if ((adc_map[telescopes[k].e_adc] != -1) && 
	  (telescopes[k].special)) {
	special_telescope_gate[adc_map[telescopes[k].e_adc]] |= (unsigned long long) ((unsigned long long) 1 << k);
      }
      if ((adc_map[telescopes[k].delta_e_adc] != -1) && (telescopes[k].special))
	special_telescope_gate[adc_map[telescopes[k].delta_e_adc]] |= (unsigned long long) ((unsigned long long) 1 << k);
    }

    if (detelescope_gate == NULL) {
      detelescope_gate = (unsigned long long *) malloc((max_adcs+1) * sizeof(unsigned long long));
    } else {
      detelescope_gate = (unsigned long long *) realloc(detelescope_gate ,
					 (max_adcs + 1) * sizeof(unsigned long long));
    }
    for (k = 0; k <= max_adcs; k++) {
      detelescope_gate[k] = 0;
    }
    for ( k = 0; k < num_telescopes; k++) {
      if (adc_map[telescopes[k].delta_e_adc] != -1) {
	detelescope_gate[adc_map[telescopes[k].delta_e_adc]] |= (unsigned long long)((unsigned long long)1 << k);
      }
    }

    /* --- the telescope_gate is slightly different.  The value for the telescope_gate
       --- coresponding to the adc is the number of the telescope it came from
       --- plus one. --- */
    //    printf("Made telescope gate map.\n");

    /* --- if we have to lets make the pair_flags --- */
    if (pgamsortoutputpairsqr) {
      /* --- the pair information only needs to be read in once,
	 --- similarly the number of pairs never changes --- */
      if (pairs == NULL) {
	pairs = PgamReadPairs(pgampairfilename);
      }
      if (pairs == NULL) {
	GetMessageDialog("Error reading pairs data from file.\n");
      } else {
	if (pair_flags == NULL)
	  pair_flags = (uint *) malloc(sizeof(uint) * pairs->num);
      }
      for (k = 0; k < pairs->num; k++) {
	if (adc_map[pairs->pairs[k].a] != -1)
	  pair_flags[k] = (uint) (1 << adc_map[pairs->pairs[k].a]);
	if (adc_map[pairs->pairs[k].b] != -1)
	  pair_flags[k] |= (uint) (1 << adc_map[pairs->pairs[k].b]);
      }
    }

    /* --- is-a-tac flag --- */

      if (tacgate == NULL) {
	tacgate = (int *) malloc(sizeof(int) * max_adcs);
      } else {
	tacgate = (int *) realloc(tacgate,sizeof(int) * max_adcs);
      }
      for (l = 0; l < max_adcs; l++) {
	tacgate[l] = 0;
      }
      if (num_tacs) {
	for (l = 0; l < num_tacs; l++) {
	  tacgate[adc_map[tacs[l].adc]] = l+1;
	}
      }

    /* --- let's also take a moment to set the titles of the histograms 
       --- if we have them --- */
    
    if ((pgamsortoutputhists) || (pgamsortoutputtac)) {
      for (k = 0; k < num_adcs; k++) {
	if (adc_map[hpges[k].adc] != -1) {
	  sprintf(field2[adc_map[hpges[k].adc]],hpges[k].title);
	}
      }
      for (k = 0; k < num_telescopes; k++) {
	if (adc_map[telescopes[k].totaladc] != -1) {
	  sprintf(field2[adc_map[telescopes[k].totaladc]],telescopes[k].totaltitle);
	}
      }
      for (k = 0; k < num_clovers; k++) {
	if (adc_map[clovers[k].totadc] != -1) {
	  sprintf(field2[adc_map[clovers[k].totadc]],clovers[k].totaltitle);
	}
      }
    }

    if (num_clovers > 0) {
      if (clover_hit == NULL) {
	clover_hit = (int *) malloc(sizeof(int) * num_clovers);
      } else {
	clover_hit = (int *) realloc(clover_hit, sizeof(int) * num_clovers);
      }
      for ( l = 0; l < num_clovers; l++) {
	clover_hit[l] = 0;
      }
      if (clover_hit2 == NULL) {
	clover_hit2 = (int *) malloc(sizeof(int) * num_clovers);
      } else {
	clover_hit2 = (int *) realloc(clover_hit2, sizeof(int) * num_clovers);
      }
      for ( l = 0; l < num_clovers; l++) {
	clover_hit2[l] = 0;
      }
    }

    not_a_sum_hit = (int *) g_realloc(not_a_sum_hit,sizeof(int) * biggest_adc);
    for (l = 0; l < biggest_adc; l++) 
      not_a_sum_hit[l] = 0;
    for (l = 0; l < num_adcs; l++)
      if (adc_map[hpges[l].adc] >= 0)
	not_a_sum_hit[adc_map[hpges[l].adc]] = 1;
    //--ddc debug 06-24-05, adcmap and inverse_adc_map arrays use  
    //adc numbers as indexes, SO, arrays must be dimensioned for
    //the biggest_adc PLUS ONE.
    if (truesingles) {
      adc_map = (int *) g_realloc(adc_map,sizeof(int) * (biggest_adc+1));
      inverse_adc_map = 
       (int *) g_realloc(inverse_adc_map,sizeof(int) * (biggest_adc+1));
      //--ddc debug, and if you want all adcs to be mapped you have to
      //test k<=biggest_adc!
      for (k = 1; k <= biggest_adc; k++) {
	adc_map[k] = k-1;
	inverse_adc_map[k] = k-1;
      }
      //--ddc 2may07 Found that we either have to extend the gates for singles,
      //or skip them, so I'm adding 'singles_gates' to keep the upperlimit
      singles_gates = max_adcs;
      max_adcs = biggest_adc;
    }
	
    printf("Made it to allocating memory for the event.\n Starting sorting.\n");
    /* --- guess we should start sorting something --- */
    
    /* --- cycling through the runs --- */
    
    tempmin = Max(runmin,setupfiles[i].runs[0]);
    tempmax = Min(runmax,setupfiles[i].runs[1]);
    
    for (j = tempmin;  j <= tempmax; j++ ) { // runs
      
      /* --- let's attempt to open the .evt file and sort it if we can --- */
      switch (datafiletype) {
      case 1:
	sprintf(filename,"%srun%d.ev2",evtpath,j);
	break;
      default:
	sprintf(filename,"%srun%d.evt",evtpath,j);
	break;
      }
      stat(filename,&filebuffer);
      if ((infile = fopen(filename,"r")) != NULL) { // we managed to open the file
	/* --- tell the user what file we are looking at --- */
	printf("Successfully opened %s.\n",filename);
	//	sprintf(dummystr,"Sorting %s.\n",filename);
	//WriteMessageText(dummystr);

	/* --- reset the record incramenter --- */
	irec = 1;

	/* --- in the case of evt files we need to --- */
	/* --- read in the beginning of the buffer --- */
	if (datafiletype != 1) 
	  fread(&buffersize[0],sizeof(int),1,infile);
	
	/* --- allocate, or re-allocate memory for the buffer --- */
	/* --- this must be done differently for .evt and .ev2 files --- */
	switch(datafiletype) {
	case 1:
	  if (ev2buffer == NULL) // it hasn't been allocated yet
	    ev2buffer = (guchar *) malloc(EV2BUFFERSIZE);
	  break;
	default:
	  if (buffer == NULL) { // we havn't allocated it before
	    buffer = (short int *) malloc(buffersize[0]);
	  } else { // we have allocated it before
	    buffer = (short int *) realloc(buffer,buffersize[0]);
	  }
	  break;
	}
	/* --- now we read in the buffer --- */
	/* --- the buffer gets read in differently for .evt and .ev2 files --- */
	switch (datafiletype) {
	case 1:
	  howmuch = fread(ev2buffer,1,EV2BUFFERSIZE,infile);
	  break;
	default:
	  howmuch = (fread(buffer,1,buffersize[0],infile) == buffersize[0]);
	  break;
	}
	if (howmuch == 0) goto stopreading;
	stillreading = 1;
	while (stillreading) {
	  /* --- if we are reading a fortran buffer we need to check to see if it was read correctly --- */
	  if (datafiletype != 1) {
	    if (fread(&buffersize[1],sizeof(int),1,infile) == 0) goto stopreading;
	    if (buffersize[0] != buffersize[1]) goto stopreading;
	  }
	  /* --- increment records count, and if nessacary the progressbar --- */
	  irec++;
	  if (!(sortrecords)) {
	    if (irec <= recordmin) goto bufferdone;
	    if (irec >= recordmax) goto stopreading;
	  }
	  if ((irec % 100) == 0) {
	    progbarpos = progbarlastbytes + ftell(infile);
	    UpdateSortProgress(progbarpos,fileLen);
	    //    printf("%ld, %ld\n",progbarpos,progbarlastbytes);
	    sprintf(dummystr,"Reads: %d, Writes: %d\n",reads,writes);
	    if (abortflag) {
	      abortflag = 0;
	      goto stopsort;
	    }
	    WriteMessageText(dummystr);
	  }
	  /* --- if we successfully read in the buffer, then we can sort --- */
	  /* --- we need the correct number of short ints in the buffer --- */
	  /* --- end of the count is different for each type --- */
	  switch (datafiletype) {
	  case 1:
	    numshorts = howmuch;
	    break;
	  default:
	    numshorts = (float) buffersize[0]/ (float) sizeof(short int);
	    break;
	  }
	  k = 0;
	  while (k < numshorts) { // cycling through the events
	  nextheader:
	    switch (datafiletype) {
	    case 1:
	      /* --- what we are going to try to do, instead of the "bufferdone" thing
		 --- is that when we reach the end of the buffer (k == howmuch)
		 --- we are going to read in the next buffer and reset k --- */
	      if (k >= howmuch) {
		k = 0;
		howmuch = fread(ev2buffer,1,EV2BUFFERSIZE,infile);
		if (howmuch == 0)
		  goto stopreading;
		irec++;
	      }
	      numadc = (guchar) *(ev2buffer + k);
	      /* --- oops we can get into trouble if numadc == 0 (why is that happening anyway?)
		 --- therefore if we see this we need to skip until we see a valud number --- */
	      while (numadc <= 0) {
		k++;
		if (k >= howmuch) {
		  k = 0;
		  howmuch = fread(ev2buffer,1,EV2BUFFERSIZE,infile);
		  if (howmuch == 0) goto stopreading;
		  if (!(sortrecords)) {
		    if (irec <= recordmin) goto bufferdone;
		    if (irec >= recordmax) goto stopreading;
		  }
		  if ((irec % 100) == 0) {
		    progbarpos = progbarlastbytes + ftell(infile);
		    UpdateSortProgress(progbarpos,fileLen);
		    //    printf("%ld, %ld\n",progbarpos,progbarlastbytes);
		    sprintf(dummystr,"Reads: %d, Writes: %d\n",reads,writes);
		    if (abortflag) {
		      abortflag = 0;
		      goto stopsort;
		    }
		    WriteMessageText(dummystr);
		  }
		  if (howmuch == 0) 
		    goto stopreading;
		  irec++;
		}
		numadc = (guchar) *(ev2buffer + k);
	      }
	      
	      /* --- Previously we tried to calculate the length of the event
		 --- and then look for the other end, this is not possible 
		 --- since events can cross buffers.  Instead, we are going
		 --- to check at the end of unpacking the event, and
		 --- do a rudimentary error check on the "numadc" by
		 --- seeing if it is less than the "max_adcs" --- */
	      //--ddc	      if (numadc > max_adcs)
	      if (numadc > biggest_adc)
		printf("Error reading number of adcs. k = %d\n",k);
	      /* --- make sure there isn't any old data in the "event_adcs" and
		 --- "event_channel" buffers --- */
	      for (n = 0; n < numadc; n++) {
		event_adcs[n] = -1;
		event_channel[n] = -1;
	      }
	      /* --- ok, now let's try to extract the data --- */
	      for (l = 0; l < numadc; l++) {
		/* --- make sure that it is a valid adc --- */
		k++;
		if (k >= howmuch) {
		  k = 0;
		  irec++;
		  howmuch = fread(ev2buffer,1,EV2BUFFERSIZE,infile);
		  if (howmuch == 0)
		    goto stopreading;
		}
		if (adc_map[(guchar) *(ev2buffer + k)] != -1) {
		  event_adcs[l] = adc_map[(guchar) *(ev2buffer + k)];
		  /* --- now for the channel information --- */
		  k++;
		  if (k >= howmuch) {
		    k = 0;
		    irec++;
		    howmuch = fread(ev2buffer,1,EV2BUFFERSIZE,infile);
		    if (howmuch == 0)
		      goto stopreading;
		  }
		  event_channel[l] = (guchar) *(ev2buffer + k);
		  k++;
		  if (k >= howmuch) {
		    k = 0;
		    irec++;
		    howmuch = fread(ev2buffer,1,EV2BUFFERSIZE,infile);
		    if (howmuch == 0)
		      goto stopreading;
		  }
		  event_channel[l] |= ((guchar) *(ev2buffer + k)) << 8;
		  //event_channel[l] -= 1;
		} else {
		  k += 2;
		}
		if (k >= howmuch) {
		  //--ddc nov14 WRONG. if we skipped bytes NOT  k = 0;
		  k=k-howmuch;
		  irec++;
		  howmuch = fread(ev2buffer,1,EV2BUFFERSIZE,infile);
		  if (howmuch == 0)
		    goto stopreading;
		}
	      }
	      k++;
	      if (k >= howmuch) {
		k = 0;
		irec++;
		howmuch = fread(ev2buffer,1,EV2BUFFERSIZE,infile);
		if (howmuch == 0)
		  goto stopreading;
	      }
	      /* --- now to make sure that we read in a valid event --- */
	      if ((guchar) numadc != (guchar) ev2buffer[k]) {
		printf("error reading file. k = %d\n",k);
		
		k = 0;
		irec++;
		howmuch = fread(ev2buffer,1,EV2BUFFERSIZE,infile);
		if (howmuch == 0)
		  goto stopreading;
		goto nextheader;
	      }
	      k++;
	      
	      /* --- got a good event, incrament reads --- */
	      reads++;
	      break;
	    default:

	      if (k >= numshorts) goto bufferdone;
	      head = buffer[k];
	      k++;
	      if (k >= numshorts) goto bufferdone;
	      if (head > 0) {
		//		WriteMessageText("Badheader.\n");
		goto nextheader;
	      } else { // got a good event
		reads++;
		
		numadc = head & (short int) 15;
		
		/* --- now we allocate memory for the adc's and channels of this event --- */
		for (n = 0; n < numadc; n++) {
		  event_adcs[n] = -1;
		  event_channel[n] = -1;
		}
		
		/* --- now we start dealing with the channels --- */
		
//--ddc 1may06  Surely, JP did NOT intend to throw away high multiplicty events!  Each of these 'event cases'
// MUST allow numadc to be as great as the number of ADCs.  Also, this test seems to be redundant after
// the first test, but, I will leave it. 
//--ddc		if ((numadc >= 1) && (numadc <= num_adcs)) { // adc check
		if ((numadc >= 1) && (numadc <= biggest_adc)) { // adc check
		  m = (float) ((short int) 496 & head) / (float) 16;
		  /* --- m is the adc that fired --- */
		  /* --- map it to it's location in memory --- */
		  if ((adc_map[m] >= 0) && (adc_map[m] < max_adcs) && (m <= biggest_adc)) {
		    event_adcs[0] = adc_map[m];
		    /* --- the buffer[k] should be the channel that fired --- */
		    event_channel[0] = buffer[k];// - 1;
		  }
		  k++;
		  if (k >= numshorts) goto bufferdone;
//--ddc		  if ((numadc >= 2) && (numadc <= num_adcs)) { // second adc check
		  if ((numadc >= 2) && (numadc <= biggest_adc)) { // second adc check
		    /* --- now we deal with the second event --- */
		  m = (float) (head & (short int) 15872) / (float) 512;
		  /* --- gain m is the adc that fired --- */
		  
		  if ((adc_map[m] >= 0) && (adc_map[m] < max_adcs) && (m <= biggest_adc)) {
		    event_adcs[1] = adc_map[m];
		    event_channel[1] = buffer[k];// - 1;
		  }
		  } else {
		    goto writevent;
		  } // done second adc check
//--ddc  	  if ((numadc >=3) && (numadc <= num_adcs)) { // third adc check
		  if ((numadc >=3) && (numadc <= biggest_adc)) { // third adc check
//--ddc AND here, we must do the sum over the actual number of adcs, not the number we are using!! 
//--ddc		    for (l = 2; l < num_adcs; l++) {
		    for (l = 2; l < numadc; l++) {
		      k++;
		      if (k >= numshorts) goto bufferdone;
		      m = buffer[k];
//--ddc 1may07  Oops. it is clearly possible that m is less than zero, however JP didn't test
//this until AFTER using m to update event_adcs.  I have copied the test for m to here, where it
//will do even more good.
       		      if (m < 0) { 
			goto writevent;
		      }
		      if ((adc_map[m] >= 0) && (adc_map[m] < max_adcs) && (m <= biggest_adc)) {
			event_adcs[l] = adc_map[m];
		      }
//--ddc 1may07        if (m < 0) { 
//--ddc 1may07		goto writevent;	
//--ddc 1may07 	      }
		      k++;
		      if (k >= numshorts) goto bufferdone;
		      if (buffer[k] > 0) {
			event_channel[l] = buffer[k];// - 1;
		      } else {
			goto writevent;
		      } 
		    }
		  } else {
		    k++;
		    if ((numadc < 0) || (numadc > biggest_adc)) {
		      GetMessageDialog("Either a negative number of ADCs fired or more ADCs fired than possible.\n");
		      goto stopreading;
		    }
		  } // done third adc check
		}  // end of adc check
	      } // done interpreting events
	      break;
	    } //  end of switch
	    /* --- now that we have read the thing off the tape, we need to sort it --- */
	    
	  writevent:
	    writes++;
	    /* --- deal with truesingles seperately --- */
	    if (truesingles) {
	      for ( l = 0; l < numadc; l++) {
		if ((event_channel[l] < max_channels) && (event_adcs[l] >= 0))
		  //--ddc 24jul13 *(histloc[event_adcs[l]] + event_channel[l] - 1) = 
		 //--ddc 24jul13 *(histloc[event_adcs[l]] + event_channel[l] - 1) + 1;
		  //--ddc jan17.. still a problem? test index... although WHEN could 
		  //--ddc event_channel be less than 1?
		  if( event_channel[l]<1 ) {
		    //--ddc only print for dbg.	 printf("error filling raw histogram = %d, val=%d\n",event_adcs[l],event_channel[l]);
		    continue;
		  }
		  *(histloc[event_adcs[l]] + event_channel[l] - 1) = 
		    *(histloc[event_adcs[l]] + event_channel[l] - 1) + 1;
	      }
	      /* --- let's be snazzy and return some statistics --- */
	      /* --- first let's give statistics about the # of events in each
		 --- adc */
	      for (l = 0; l < numadc; l++) {
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
		if(event_adcs[l]<0) continue;

		*(histloc[biggest_adc] + event_adcs[l]) += 1;
	      }
	      /* --- now let's give statistics about the multiplicity of each event --- */
	      *(histloc[biggest_adc] + 32 + numadc) += 1;
	      /* --- now lets give information about the # of hits in each telescope --- */
	      if (num_telescopes) { // provide we have some
		for (l = 0; l < numadc; l++) {
		  /* --- while we're at it lets get the multiplicity of telescopes --- */
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
//--ddc 2may07  add skip for adcs with no gate mapped
		  if(event_adcs[l]<0 || event_adcs[l] > singles_gates) continue;

		  telescopes_hit = 0;
		  if (telescope_gate[event_adcs[l]]) {
		    for (ll = 0; ll < numadc; ll++) {
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
//--ddc 2may07  add skip for adcs with no gate mapped
		      if(event_adcs[ll]<0 || event_adcs[ll] > singles_gates) continue;

		      if (detelescope_gate[event_adcs[ll]] & telescope_gate[event_adcs[l]]){
			telescopes_hit++;
		      }
		    }
		    /* --- now let's cycle through the telescope gate and write the appropriate
		       --- statistics --- */		    
		    for (ll = 0; ll < most_telescopes; ll++) {
		      if (telescope_gate[l] & (unsigned long long)((unsigned long long)1 << ll)) {
			*(histloc[biggest_adc] + 64 + ll) += 1;
			*(histloc[biggest_adc] + 96 + telescopes_hit) += 1;
		      }
		    }
		  }
		}
	      }
	      /* --- now we need appropriate statistics for clovers --- */
	      if (num_clovers) { // provided we have some
		/* --- need to do the clover_hit thingy --- */
		for (l = 0; l < num_clovers ; l++)
		  clover_hit[l] = 0;
		for (l = 0; l < numadc; l++) {
//--ddc debug.  If an event had an adc that was not in a map (or has no map), it is 
//              mapped to -1!!! so skip the rest if this is true!
//--ddc 2may07  add skip for adcs with no gate mapped
		  if(event_adcs[l]<0 || event_adcs[l] >= singles_gates ) continue;

		  if (clover_gates[event_adcs[l]]) {
		    /* --- make sure it hasn't already been counted --- */
		    if (!(clover_hit[BitLog2(clover_gates[event_adcs[l]])])) {
		      clover_multiplicity = 0;
		      /* --- case of it not already being used --- */
		      clover_hit[BitLog2(clover_gates[event_adcs[l]])] = 1;
		      /* --- now we cycle through the rest of the event to gather the multiplicity of
			 --- that detector --- */
		      for ( ll = 0; ll < numadc; ll++) {
//--ddc debug.  If an event had an adc that was not in a map (or has no map), it is 
//              mapped to -1!!! so skip the rest if this is true!
//--ddc 2may07  add skip for adcs with no gate mapped
			if(event_adcs[ll]<0  || event_adcs[ll] >= singles_gates ) continue;

			if (clover_gates[event_adcs[ll]] & clover_gates[event_adcs[l]]) {
			  clover_multiplicity += 1;
			}
		      }
		      /* --- write down this information --- */
		      *(histloc[biggest_adc] + 127 + BitLog2(clover_gates[event_adcs[l]])) += 1;
		      *(histloc[biggest_adc] + 160 + 
			(BitLog2(clover_gates[event_adcs[l]]) - 1) * 10 + clover_multiplicity) += 1;
		      /* --- also write down the real multiplicity --- */
		      *(histloc[biggest_adc] + 200 + numadc + clover_multiplicity) += 1;
		    }
		  } 
		}
	      }
	    } else {

	      /* --- if there are telescopes, we need to know how may were hit, otherwise, leave it alone --- */
	      if (num_telescopes) {
		telescopes_hit = 0;
		for (l = 0; l < numadc; l++) {
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
		  if(event_adcs[l]<0) continue;

		  if (telescope_gate[event_adcs[l]]) {
		    /* --- we need to scan through the telescopes to see if any were hit --- */
		    for (ll = 0; ll < numadc; ll++) {
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
		      if(event_adcs[ll]<0) continue;

		      if (detelescope_gate[event_adcs[ll]] & telescope_gate[event_adcs[l]])
			telescopes_hit++;
		    }
		  }
		}
	      }
	     
	    
	      /* --- perhaps it would make sense to do the veto gates first since they should cut out
		 --- a fair amount of unwanted stuff --- */
	      if (pgamsortgateveto) {
		int failvetogate;
		failvetogate = 0;
		for (l = 0; l < numadc; l++) {
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
		  if(event_adcs[l]<0) continue;

		  for (m = 0; m < vetos->num; m++) {
		    if ((event_adcs[l] == adc_map[vetos->adc[m]]) && (event_adcs[l] != -1)) {
		      if ((event_channel[l] >= vetos->min[m]) && (event_channel[l] <= vetos->max[m])) {
			failvetogate = 1;
			goto failedvetogate;
		      }
		    }
		  }
		}
	      failedvetogate:
		if (failvetogate) goto nextheader;
	      }
	      /* --- done with veto gate --- */


	      /* --- ok, to speed things up let's try unpacking from channels to energy at the beginning --- */
	      
	      /* --- to try and save time let's look to the statistics we gathered earlier and the output types
		 --- which we are using --- */
	      /* --- if we have either pgamsortoutputhists or pgamsortoutputtac we accept all types --- */
	      /* --- if we have either pgamsortoutputpart we need 1 telescope --- */
	      /* --- if we have pgamsortoutputgg we need 2 adc's fired beyond the partiles --- */
	      /* --- if we have pgamsortoutputsqr we need 1 telescope and 1 extra --- */
	      /* --- if we have pgamsortoutputggpair we need 2 adc's beyond the particles --- */
	      if (pgamsortoutputpart) {
		if (!(telescopes_hit)) goto nextheader;
	      } else {
		if (pgamsortoutputgg) {
		  if ((numadc - telescopes_hit) < 2) goto nextheader;
		} else {
		  if (pgamsortoutputsqr) {
		    if (!((telescopes_hit) && ((numadc - telescopes_hit) > 1))) goto nextheader; 
		  } else {
		    if (pgamsortoutputpairsqr) 
		      //--ddc oct14 this does not allow events with one gamma
		      //AND one pair.
		      //--ddc nov14 for testing accept all events with a possible pair
		      //--ddc nov14 (2->1)
		      //		      if (numadc < 4) goto nextheader;
		      if ( (numadc - telescopes_hit) < 1 ) goto nextheader;
		  }
		}
	      }

	      /* --- let's make sure that if there are any
		 --- signals past the overflow bit they are not
		 --- added into whatever output there is --- */
	      for (l = 0; l < numadc; l++) {
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
		if(event_adcs[l]<0) continue;
//--ddc debug.  If an event had an adc that is in a map, BUT it is 
//              virtual adc (which happens to share a number with an 
//              unused physical adc) detector_info_map is -1!!! 
//              skip the rest if this is true!
		if(detector_info_map[event_adcs[l]]<0) continue;

	      	if (event_channel[l] >= 
	      	    hpges[detector_info_map[event_adcs[l]]].overflow) 
		  event_channel[l] = -8192;
	      }

	      /* --- now we unpack the event --- */
	      for (l = 0; l < numadc; l++) {


	      /* --- let's make sure that if there are any
		 --- signals past the overflow bit they are not
		 --- added into whatever output there is --- */
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
		if(event_adcs[l]<0) continue;
//--ddc debug.  If an event had an adc that is in a map, BUT it is 
//              virtual adc (which happens to share a number with an 
//              unused physical adc) detector_info_map is -1!!! 
//              skip the rest if this is true!
		if(detector_info_map[event_adcs[l]]<0) continue;
		//--ddc jan15 add SLT's doppler correction, ftaux, ftauy
		uchanm[l] = (float) (((hpges[detector_info_map[event_adcs[l]]].calib[0]+
				       hpges[detector_info_map[event_adcs[l]]].calib[1] *
				       ((float)event_channel[l] - 0.5) +
				       hpges[detector_info_map[event_adcs[l]]].calib[2] *
				       ((float)event_channel[l] - 0.5) *
				       ((float)event_channel[l] - 0.5)) / (float) fin_cal) /
				     (float) (1 + beta * ftaux *  
					      cos(hpges[detector_info_map[event_adcs[l]]].angle * 0.01745329))) ;
		uchanp[l] = (float) (((hpges[detector_info_map[event_adcs[l]]].calib[0]+
				       hpges[detector_info_map[event_adcs[l]]].calib[1] *
				       ((float)event_channel[l] + 0.5) +
				       hpges[detector_info_map[event_adcs[l]]].calib[2] *
				       ((float)event_channel[l] + 0.5) *
				       ((float)event_channel[l] + 0.5)) / (float) fin_cal) /
				     (float) (1 + beta * ftaux *
					      cos(hpges[detector_info_map[event_adcs[l]]].angle * 0.01745329))) ;
		uchanym[l] = (float) (((hpges[detector_info_map[event_adcs[l]]].calib[0]+
				       hpges[detector_info_map[event_adcs[l]]].calib[1] *
				       ((float)event_channel[l] - 0.5) +
				       hpges[detector_info_map[event_adcs[l]]].calib[2] *
				       ((float)event_channel[l] - 0.5) *
				       ((float)event_channel[l] - 0.5)) / (float) fin_cal) /
				     (float) (1 + beta * ftauy *
					      cos(hpges[detector_info_map[event_adcs[l]]].angle * 0.01745329))) ;
		uchanyp[l] = (float) (((hpges[detector_info_map[event_adcs[l]]].calib[0]+
				       hpges[detector_info_map[event_adcs[l]]].calib[1] *
				       ((float)event_channel[l] + 0.5) +
				       hpges[detector_info_map[event_adcs[l]]].calib[2] *
				       ((float)event_channel[l] + 0.5) *
				       ((float)event_channel[l] + 0.5)) / (float) fin_cal) /
				     (float) (1 + beta * ftauy *
					      cos(hpges[detector_info_map[event_adcs[l]]].angle * 0.01745329))) ;
		}
	      /* --- done unpacking the event --- */


	      /* --- now we should do the requires gate since that should also cut out a lot of stuff --- */
	      if (pgamsortgaterequires) {
		int failrequiresgate;
		failrequiresgate = 1;
		for (l = 0; l < numadc; l++) {
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
		  if(event_adcs[l]<0) continue;

		  for (m = 0; m < requires->num; m++) {
		    if ((event_adcs[l] == adc_map[requires->adc[m]]) && (event_adcs[l] != -1)) {
		      if ((event_channel[l] >= requires->min[m]) && (event_channel[l] <= requires->max[m])) {
			failrequiresgate = 0;
			goto passedreqiresgate;
		      }
		    }
		  }
		}
	      passedreqiresgate:
		if (failrequiresgate) goto nextheader;
	      }
	      /* --- done with requires gate --- */


	      /* --- if we are sorting with something that would use telescopes, let's unpack the
		 --- telescopes --- */
	      num_scopes = 0;
	      if ((pgamsortgatepart || pgamsortoutputhists || pgamsortoutputpart || pgamsortoutputsqr
		   || pgamsortgateparten) && (telescopes_hit >= 1)) {
		/* --- list of variables used here --- */
		
		/* --- actual work --- */
		for (l = 0; l < numadc; l++) {
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
		  if(event_adcs[l]<0) continue;

		  if (telescope_gate[event_adcs[l]]) {
		    for (ll = 0; ll < most_telescopes; ll++) {
		      z = -1;
		      if (telescope_gate[event_adcs[l]] & (unsigned long long)((unsigned long long)1 << ll)) {
			n = 0;
			while ((z == -1) && (n < numadc)) {
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
			  if(event_adcs[n]<0) {
                                n++; 
				continue;
			  }

			  if (detelescope_gate[event_adcs[n]] & telescope_gate[event_adcs[l]] & 
			      (unsigned long long)((unsigned long long)1 << ll)) 
			    z = n;
			  n++;
			}
		      }
		      if (detelescope_gate[event_adcs[z]] & telescope_gate[event_adcs[l]] & 
			  (unsigned long long) ((unsigned long long)1 << ll)) {
			tellook = LongBitLog2((detelescope_gate[event_adcs[z]] & telescope_gate[event_adcs[l]]) &
					  (unsigned long long)((unsigned long long )1 << ll));
			//if (tellook > 32) {
			// printf("Just a trap.\n");
			//}
			/* --- we would like to try to use the supplamental ADC channels for both of the 
			   --- E and dE detectors --- */
			/* --- in order to do this, we scan through the rest of the event to determine
			   --- if any of the supplamental detectors fired.  If they did
			   --- we then compair the energies of the different ADC channels
			   --- if the main channel is the largest, then we assume that this is 
			   --- a valid event for the main channel and use addback
			   --- otherwise we assume that it is not a valid event --- */

			/* --- first de do this for the E detector --- */
			e_sup_hits = 0;
			if (telescopes[tellook].num_e_sup) {
			  for (n = 0; n < numadc; n++) {
			    if (n != l) {
			      /* --- now we scan through the e_sup_adc array --- */

			      for (o = 0; o < telescopes[tellook].num_e_sup; o++) {
				if (adc_map[telescopes[tellook].e_sup_adc[o]] == event_adcs[n]) {
				  e_sup_ent[e_sup_hits] = n;
				  e_sup_hits++;
				}
			      }
			      /* --- done scanning through the e_sup_adc --- */
			    }
			  }
			  if (e_sup_hits) {
			    /* --- this is where we make sure the supplamental hits are not the
			       --- main hits --- */
			    for (n = 0; n < e_sup_hits; n++) {
			      if (uchanm[l] < uchanm[e_sup_ent[n]]) goto nextlpresortpart;
			    }
			  }
			}
			/* --- done scanning for supplamentals for the de-detector --- */
			/* --- first de do this for the E detector --- */
			de_sup_hits = 0;
			if (telescopes[tellook].num_de_sup) {
			  for (n = 0; n < numadc; n++) {
			    if (n != z) {
			      /* --- now we scan through the e_sup_adc array --- */
			      for (o = 0; o < telescopes[tellook].num_de_sup; o++) {
				if (adc_map[telescopes[tellook].de_sup_adc[o]] == event_adcs[n]) {
				  de_sup_ent[de_sup_hits] = n;
				  de_sup_hits++;
				}
			      }
			      /* --- done scanning through the de_sup_adc --- */
			    }
			  }
			  if (de_sup_hits) {
			    /* --- this is where we make sure the supplamental hits are not the
			       --- main hits --- */
			    for (n = 0; n < de_sup_hits; n++) {
			      if (uchanm[z] < uchanm[de_sup_ent[n]]) goto nextlpresortpart;
			    }
			  }
			}
			/* --- done scanning for supplamentals for the de-detector --- */

			if (adc_map[telescopes[tellook].totaladc] != -1)
			  scope_adcs[num_scopes] = adc_map[telescopes[tellook].totaladc];
			scope_addr[num_scopes] = tellook;
			if (!telescopes[tellook].special) {
			  tempfloatm = uchanm[l] + uchanm[z] * telescopes[tellook].gain;
			  tempfloatp = uchanp[l] + uchanp[z] * telescopes[tellook].gain;
			  //  scope_newm[num_scopes] = uchanm[l] + uchanm[z] * telescopes[tellook].gain;
			  //scope_newp[num_scopes] = uchanp[l] + uchanp[z] * telescopes[tellook].gain;


			} else {
			  tempfloatm = -1;
			  tempfloatp = -1;
			}
			//scope_x[num_scopes] = (float) (tempfloatm) 
			//  / (float) fin_chans * 
			// (float) edesize / (float) telfincal;
			scope_y[num_scopes] = (float) uchanm[z] / (float) fin_chans * 
			  (float) edesize / (float) telfincal;
			/* --- addback for the e_sups --- */
			for (n = 0; n < e_sup_hits; n++) {
			  if (!telescopes[tellook].special) {
			    tempfloatm += uchanm[e_sup_ent[n]];
			    tempfloatp += uchanp[e_sup_ent[n]];
			  }
			  //scope_x[num_scopes] += uchanm[e_sup_ent[n]] / (float) fin_chans * (float) edesize /
			   // (float) telfincal;
			}
			/* --- addback for the de_sups --- */
			for (n = 0; n < de_sup_hits; n++) {
			  if (!telescopes[tellook].special) {
			    tempfloatm += uchanm[de_sup_ent[n]] * telescopes[tellook].gain;
			    tempfloatp += uchanp[de_sup_ent[n]] * telescopes[tellook].gain;
			  }
			  // scope_x[num_scopes] += uchanm[de_sup_ent[n]] / (float) fin_chans * (float) edesize /
			  //  (float) telfincal * telescopes[tellook].gain;
			  scope_y[num_scopes] += uchanm[de_sup_ent[n]] / (float) fin_chans * (float) edesize /
			    (float) telfincal;
			}
			  
			/* --- adding the supplamental calibration --- */
                        //--ddc 17jan06// Note... scope_y does NOT have partfincal applied, but because where JP chose to
                        //--ddc 17jan06// do the final calculation for scope_x IT does have partfincal applied!
                        //--ddc Fix this by removing partfincal here, and applying AFTER the gates... the scope_x
                        //--ddc final calculation being LEFT where JP left it for now... Ah, so it is actually the total
                        //--ddc energy (tempfloatm/p after summing for all e and de detectors).

			tempfloatm = tempfloatm * (float) fin_cal;
			tempfloatp = tempfloatp * (float) fin_cal;
			scope_newm[num_scopes] = (telescopes[tellook].sup_cal[0] +
			  telescopes[tellook].sup_cal[1] * tempfloatm +
			  telescopes[tellook].sup_cal[2] * tempfloatm * tempfloatm) / (float) fin_cal ;
			scope_newp[num_scopes] = (telescopes[tellook].sup_cal[0] +
			  telescopes[tellook].sup_cal[1] * tempfloatp +
			  telescopes[tellook].sup_cal[2] * tempfloatp * tempfloatp) / (float) fin_cal ;

                        scope_x[num_scopes] = (float) scope_newm[num_scopes] / (float) fin_chans * 
                        			  (float) edesize / (float) telfincal;

                        //--ddc NOW apply partfincal to the scope_newm and scope_newp values only...
                        scope_newm[num_scopes]=scope_newm[num_scopes]/(float)partfincal;
                        scope_newp[num_scopes]=scope_newp[num_scopes]/(float)partfincal;

			num_scopes++;
		      }
		    } // done cycling through all the possible telescopes
		  } // done with making sure it is in a telescope gate
		nextlpresortpart:
		  continue;
		} // done cycling through the event
	      }
	      /* --- done unpacking telescope events --- */
	      
	    /* --- first use tac gating --- */
	    /* --- the tac gating should probably be changed to a similar method
	       --- to the clover_gates and the telescope-gate and the gamma_gate etc.
	       --- at some point --- */
	    /* --- but since we don't have sufficient TACs to actually do
	       --- tac gates properly... we can do this the slow way for now --- */
	      if (pgamsortgatetac){
		int passtacgatea,passtacgateb;
		passtacgatea = 0;
      		passtacgateb = 0;
		passtacgate = 0;
//--ddc dbg. jun16 probably meant to 'passtacgatea' IF it didn't FAIL ANY
//           TAC gates... so default should be PASS initially, and then check
//           for FAIL and stop if there is a fail, instead of what he did
//           (check for pass, and stop for pass).
//  
		passtacgatea = 1;  //--ddc jun16 make default PASS.
		if (num_tacs > 0) {
		  for (l = 0; l < numadc; l++) {
		    for (m = 0; m < num_tacs; m++) {
		    if (event_adcs[l] == adc_map[tacs[m].adc]) {
		      for (n = 0; n < tacs[m].num_adcs; n++) {
			int zzz;
			for (zzz = 0; zzz < numadc; zzz++) {
			  /* //--ddc dbg this is the original keep if pass ANY!!
			  if ((event_adcs[zzz] == adc_map[tacs[m].gates[n].adc]) &&
			      (event_channel[l] >= tacs[m].gates[n].min) && 
			      (event_channel[l] <= tacs[m].gates[n].max) ) {
			    passtacgatea = 1;
			    goto passtacgatea;
			  }
			  */
			  if ((event_adcs[zzz] == adc_map[tacs[m].gates[n].adc]) &&
			      (event_channel[l] < tacs[m].gates[n].min) || 
			      (event_channel[l] > tacs[m].gates[n].max) ) {
			    passtacgatea = 0;
			    goto passtacgatea;
			  }
			}
			}
		      }
		    }
		  }
		} else {
		  //--ddc dbg jun16 default... PASS.  didn't even
		  //make sense ever, does this?		  passtacgatea = 0;
		  passtacgatea = 1; 

		}
	      passtacgatea:
		if (pgamsortgatepart) {
//--ddc 18aug06 Fix for failing TAC test when sorting particles (with no 
//TAC associated with any telescope).  I've added making passtacb true by 
//default.  It is set to false for any case which associated with a TAC,
//and then, only back to true if it passes some test for the TAC.  
		  passtacgateb=1;
		  for (l = 0; l < num_scopes; l++) {
		    for (m = 0; m < num_tacs; m++) {
		      if (telescopes[scope_addr[l]].e_adc == tacs[m].adc) {
			passtacgateb=0;
			for (n = 0; n < tacs[m].num_adcs; n++) {
			  if (telescopes[scope_addr[l]].delta_e_adc == tacs[m].gates[n].adc) {
			    passtacgateb = GateDataStructTest(gate_info,scope_addr[l],scope_x[l],scope_y[l]);
			    if (passtacgateb) goto passtacgateb;
			  }
			}
		      }
		    }
		  }
		} // done with 2D's
	      passtacgateb:
	      passtacgate:
		if (passtacgatea) {
		  if (pgamsortgatepart) {
		    passtacgate = passtacgatea && passtacgateb;
		  } else {
		    passtacgate = passtacgatea;
		  }   
		}
		if (!(passtacgate)) goto nextheader;
		/* --- if we didn't pass the tac gate we read the next header --- */
		//printf("Got pass tac gate check.\n");
	      }

	      /* --- let's do the particle energy gating --- */
	      /* --- unpack each event's E and dE information
		 --- and determine if the total particle energy
		 --- is within the given range --- */
	      if ((pgamsortgateparten)) {
		
		/* --- if the particle energy range was not entered, abort the sort --- */
		if ((partenergies[0] == -1) || (partenergies[1] == -1)) goto stopsort;

		/* --- only proceed with this if we actually have telescopes --- */
		if (num_telescopes) {

		  passpartgate = 0;
		  
		  for (l = 0; l < num_scopes; l++) {
		    if ((scope_newm[l] <= partenergies[1]) && ( scope_newp[l] >= partenergies[0])) {
		      passpartgate = 1;
		      goto passpartengate;
		    }
		  }
		} // end of checking if we have telescopes
	      passpartengate:
		if(passpartgate != 1) goto nextheader;
	      } // done dealing with the particle energy gate
	      
	      /* --- end of particle energy gate process --- */
	      
	      /* --- let's do the particle gate --- */
	      /* --- this is the version where a region of the
		 --- E vs dE spectrum is selected
		 --- and we only accept events which correspond to that
		 --- */
	      if ((pgamsortgatepart)) {
		passpartgate = 0;
		if (gate_info == NULL) {
		  GetMessageDialog("No E - Delta E gates in memory.\n");
		  goto stopsort;
		} 
		if (num_scopes > 0) {
		  for (l = 0; l < num_scopes; l++) {
		    if ((0 < scope_x[l]) && (scope_x[l] < gate_info->axes[0]) &&
			(0 < scope_y[l]) && (scope_y[l] < gate_info->axes[1])) {
		      if (GateDataStructTest(gate_info,scope_addr[l],scope_x[l],scope_y[l])) {
		      //if (BigDataStructGetVal(gate_info,scope_x[l],scope_y[l]) & (uint)((uint)1 << scope_addr[l])) {
			passpartgate = 1;
			goto passpartgate;
		      }
		    }
		  }
		}
	      passpartgate:
		if (passpartgate != 1) goto nextheader;
	      }

	      /* --- need to deal with gamma gating --- */
	      /* --- 6-11-04 JRP modified for general 1D gating --- */
	      if (pgamsortgategamma) {
		passgammagate = 0;

		/* --- this needs to be changed so that we get the number of gamma rays which 
		   --- are inside the gate --- this information will be stored in the 
		   --- passgammagate variable --- */

		if (pgamgammagates.num) {

		  /* --- only do something if we actually have gamma gates --- */
		  //--ddc 22jun06, and for pete's sake, make sure if you've 
		  // clovers that the hit mask is zeroed!
		  for (l=0; l<num_clovers; l++) clover_hit[l]=0;

		  for (l = 0; l < numadc; l++) {
		    /* --- removed to facilitate general 1D gating  (1)
		       if (((!(telescope_gate[event_adcs[l]]) && !(detelescope_gate[event_adcs[l]]))
		       || (special_telescope_gate[event_adcs[l]])) &&
		       !(tacgate[event_adcs[l]])) { --- */
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
		    if(event_adcs[l]<0) continue;

		    anewm = uchanm[l];
		    anewp = uchanp[l];
  //--ddc jan15 additional variables for SLT's doppler correction.
		    anewym = uchanym[l];
		    anewyp = uchanyp[l];

		    /* --- have to deal with multicrystal detectors --- */
		    /* --- provided we have some --- */
		    if (num_clovers) {
		      if (clover_gates[event_adcs[l]]) {
			/* --- have to get any energy from the other crystals --- */
			/* --- first we need to check if we have already looked at
			   --- this clover --- */
			
			if (!(clover_hit[BitLog2(clover_gates[event_adcs[l]])])) {
			  //printf("Event in clover %d.\n",clover_gates[event_adcs[l]] - 1);
			  /* --- this if statement is executed if the clover hasn't already
			     --- been dealt with --- */
			  /* --- first let's mark that we have seen this clover --- */
			  clover_hit[BitLog2(clover_gates[event_adcs[l]])] = 1;
			  /* --- now let's sum up the energies from each crystal --- */
			  
			  anewm2 = anewm;
			  anewp2 = anewp;
			  
			  for (n = 0; n < numadc; n++) {
			  if (n != l) {
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
			    if(event_adcs[n]<0) continue;

			    if (clover_gates[event_adcs[n]]) {
			      /* --- due to compiler issues, the following space should not 
				 --- be removed --- */
			      
			      if (clover_gates[event_adcs[n]] 
				  & clover_gates[event_adcs[l]]) {
				clover_hit[BitLog2(clover_gates[event_adcs[l]])] += 1;
				if (selfsuppressclovers) {
				  anewm = anewp = -1;
				  anewm2 = anewp2 = -1;
				  } else {
				    anewm2 += uchanm[n];
				    anewp2 += uchanp[n];
				  }
			      }
			    }
			  }
			  }
			/* --- now we have the energies for the rest of the crystals --- */
			} else anewm2 = anewp2 = -1;
		      } else anewm2 = anewp2 = -1;
		    } else {// done dealing with the clover
		      anewm2 = anewm; anewp2 = anewp;
		    }
		    for (m = 0; m < pgamgammagates.num; m++) {
		      if ((anewm2 * fin_cal >= pgamgammagates.min[m]) && (anewp2 * fin_cal <= pgamgammagates.max[m])) {
			passgammagate++;
		      }
		    }
		    
		    /* --- } removed to facilitate 1D gating (1) --- */
		  } // done looping through the event
		}
	      passedgammagate:
		if (!(passgammagate >= pgam1dgatemin)) goto nextheader;
	      } // done dealing with gamma gating /**********/
	      

	      /* --- now that the events are un-packed and processed 
		 --- we need to write the events to memory --- */

	      /* --- this will have to be done with if's --- */
	      /* --- first if pgamsortoutputhists is true --- */
	      if (pgamsortoutputhists) {
		
		/* --- first deal with the 1D histograms --- */
		
		if (num_clovers) {
		  for ( l = 0; l < num_clovers; l++) {
		    clover_hit[l] = 0;
		  }
		}
		for ( l = 0; l < numadc; l++) {

//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
		  if(event_adcs[l]<0) continue;

		  if (fin_cal > 0) {
		    /* --- use calibrations (and beta/angle combination too) --- */
		    anewm = uchanm[l];
		    anewp = uchanp[l];
  //--ddc jan15 additional variables for SLT's doppler correction.
		    anewym = uchanym[l];
		    anewyp = uchanyp[l];
		  }


		  /* --- have to deal with multicrystal detectors --- */
		  /* --- provided we have some --- */
		  if (num_clovers) {
		    if (clover_gates[event_adcs[l]]) {
		      /* --- have to get any energy from the other crystals --- */
		      /* --- first we need to check if we have already looked at
			 --- this clover --- */
		      
		      if (!(clover_hit[BitLog2(clover_gates[event_adcs[l]])])) {
			//printf("Event in clover %d.\n",clover_gates[event_adcs[l]] - 1);
			/* --- this if statement is executed if the clover hasn't already
			   --- been dealt with --- */
			/* --- first let's mark that we have seen this clover --- */
			clover_hit[BitLog2(clover_gates[event_adcs[l]])] = 1;
			/* --- now let's sum up the energies from each crystal --- */
			
			anewm2 = anewm;
			anewp2 = anewp;

			for (n = 0; n < numadc; n++) {
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
			  if(event_adcs[n]<0) continue;

			  if (n != l) {
			    if (clover_gates[event_adcs[n]]) {
				/* --- due to compiler issues, the following space should not 
				   --- be removed --- */
			      
			      if (clover_gates[event_adcs[n]] 
				  & clover_gates[event_adcs[l]]) {
				  clover_hit[BitLog2(clover_gates[event_adcs[l]])] += 1;
				  if (selfsuppressclovers) {
				    anewm = anewp = -1;
				    anewm2 = anewp2 = -1;
				  } else {
				    anewm2 += uchanm[n];
				    anewp2 += uchanp[n];
				  }
			      }
			    }
			  }
			}
			/* --- now we have the energies for the rest of the crystals --- */
		      } else anewm2 = anewp2 = -1;
		    } else anewm2 = anewp2 = -1;
		  } // done dealing with the clover
		  //--ddc 21jun06 ... hmmm.  Ok well, if we DON'T have a clover,
                  // we'll have unitialized values of anewm2 and anewp2, which are 
		  // used in JRP's gate testing next!!!  Geez. add the next "else"
		  // for the 'if' on the num_clovers
		  else {
		    anewm2=anewm; anewp2=anewp;
		  }

		  /* --- now we have enough information to deal with the gamma gate --- */
		  if ((pgamsortgategamma) && (passgammagate)) {
		    /* --- 6-11-04 JRP commented out to make for more general 1D gating
		       if (((!(telescope_gate[event_adcs[l]]) && !(detelescope_gate[event_adcs[l]])) || (special_telescope_gate[event_adcs[l]])) && !(tacgate[event_adcs[l]])) { --- */
		    for (o = 0; o < pgamgammagates.num; o++) {
		      if ((anewm2 * fin_cal >= pgamgammagates.min[o]) && (anewp2 * fin_cal <= pgamgammagates.max[o])){
			anewm2 = anewp2 = -1;
			goto nextlhist;
			}
		      if ((anewm * fin_cal >= pgamgammagates.min[o]) && (anewp * fin_cal <= pgamgammagates.max[o])){
			anewm = anewp = anewm2 = anewp2 = -1;
			goto nextlhist;
		      }
		    }
		    /*   }  commented out to make for more general 1D gating */
		  }
		  
		  if (num_clovers) {
		    if ((anewm2 > 0) && (anewp2 > 0)) {
		      newm = (int) anewm2;
		      newp = (int) anewp2;
		      if ((anewm2 - newm) > 0.5) newm++;
		      if ((anewp2 - newp) > 0.5) newp++;
		      if ((newm > 0) && (newp < fin_chans)) {
			if (anewp2 != anewm2) {
			  frac2 = 1.0 / (anewp2 - anewm2);
			  for (o = newm; o <= newp; o++) {
			    tm2 = anewm2;
			    tp2 = anewp2;
			    if (tm2 < ((float) o - 0.5)) tm2 = ((float) o - 0.5);
			    if (tp2 > ((float) o + 0.5)) tp2 = ((float) o + 0.5);
			    if (clover_hit[BitLog2(clover_gates[event_adcs[l]])] >= min_clover_multipolarity)
			      *(temphist[adc_map[clovers[BitLog2(clover_gates[event_adcs[l]])].totadc]] + o) = *(temphist[adc_map[clovers[BitLog2(clover_gates[event_adcs[l]])].totadc]] + o) + ( frac2 * (tp2 - tm2));
			  }
			}
		      }
		      anewm2 = anewp2 = -1;
		    }
		  }
		  
		  if ((anewm > 0) && (anewp > 0)) {
		    newm = (int) anewm;
		    newp = (int) anewp;
		    if ((anewm - newm) > 0.5) newm++;
		    if ((anewp - newp) > 0.5) newp++;
		    if ((newm > 0) && (newp < fin_chans)) {
		      if (anewp != anewm)
			frac = 1.0 / (anewp - anewm);
		      for (m = newm; m <= newp; m++) {
			tm = anewm;
			tp = anewp;
			if (tm < ((float) m - 0.5)) tm = ((float)m - 0.5);
			if (tp > ((float) m + 0.5)) tp = ((float)m + 0.5);
			if (event_adcs[l] >= 0)
			  *(temphist[event_adcs[l]] + m) = *(temphist[event_adcs[l]] + m) + (frac * (tp - tm));
				/* --- also want to write to telescope and multicrystal
				   --- hpge total virtual adcs --- */
		      }  
		    }
		  }
				  
		nextlhist:
		  continue;
		}

		  /* --- particle telescopes --- */
		if ((num_scopes) && (telescopes_hit >= 1)) {
		    for (ll = 0; ll < num_scopes; ll++) {
		      newm = (int) scope_newm[ll];
		      newp = (int) scope_newp[ll];
		      if ((scope_newm[ll] - newm) > 0.5) newm++;
		      if ((scope_newp[ll] - newp) > 0.5) newp++;
		      if ((newm > 0) && (newp < fin_chans)) {
			if (scope_newm[ll] != scope_newp[ll]) 
			  frac = 1.0 / (scope_newp[ll] - scope_newm[ll]);
			for (m = newm;m <= newp; m++) {
			  tm = scope_newm[ll];
			  tp = scope_newp[ll];
			  if (tm < ((float) m - 0.5)) tm = ((float) m - 0.5);
			  if (tp < ((float) m + 0.5)) tp = ((float) m + 0.5);
			  *(temphist[scope_adcs[ll]] + m) += (frac * (tp - tm));
			}
		      }
		    } // done cycling through scope events
		  }
	      }
	      
	      /* --- deal with tac output --- */
	      if (pgamsortoutputtac) {
		if (num_tacs) {
		  //printf("attempting to process a tac event.\n");
		  for ( l = 0; l < numadc; l++) {
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
		    if(event_adcs[l]<0) continue;

		    /* --- first we need to determine if the tac fired --- */
		    if (tacgate[event_adcs[l]]) {
		      //printf("got a tac event.\n");
		      /* --- if the tac fired, we need to write
			 --- its signal to all the other adcs
			 --- that fired --- */
		      
		      if ((event_channel[l] < 8192) && (event_channel[l] >= 0)) {
			//printf("%d\n",event_channel[l]);
			for (m = 0; m < numadc; m++) {
			  //printf("start");
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
			  if(event_adcs[m]<0) continue;

			  if ((l != m) && (event_adcs[m] != -1)) {
			    //printf("madc: %d chan: %d ",event_adcs[m],event_channel[l]);
			    *(histloc[event_adcs[m]] + event_channel[l]) =
			      *(histloc[event_adcs[m]] + event_channel[l]) + 1;
			  }
			  //printf(" stop\n");
			}
		      }
		    } // end of the condition where the tac fired
		  }
		}
	      } // done dealing with tac output
	      
	      
	      /* --- now deal with the particle histogram --- */
	      if ((pgamsortoutputpart) && (num_scopes > 0)) {
		for (l = 0; l < num_scopes; l++) {
		  if (( 0 <= scope_x[l]) && 
		      (scope_x[l] < big_data_info[pgammax - most_telescopes + scope_addr[l]]->axes[0]) &&
		      ( 0 < scope_y[l]) && 
		      (scope_y[l] < big_data_info[pgammax - most_telescopes + scope_addr[l]]->axes[1]))
		    BigDataStructValPP(big_data_info[pgammax - most_telescopes + scope_addr[l]],
				       scope_x[l],scope_y[l]);
		  //if (scope_addr[l] > 32) {
		  // /* --- this is just a trap --- */
		  // printf("%d\n",scope_addr[l]);
		  //}
		}
	      }

	      /* --- done dealing with E - dE twod plot --- */
	      
	      /* --- now we deal with particle - gamma squares --- */

	      if ((pgamsortoutputsqr) && (telescopes_hit >= 1)) {
		if (num_clovers) {
		  for (l = 0; l < num_clovers; l++) {
		    clover_hit[l] = 0;
		  }
		}
		/* --- let's extract the information about the gamma rays --- */
		g_num = 0;
		for (l = 0; l < numadc; l++) {
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
		  if(event_adcs[l]<0) continue;

		  /* --- make sure this adc isn't inside a telescope --- */
		  if (((!(telescope_gate[event_adcs[l]]) && !(detelescope_gate[event_adcs[l]])) || 
		       special_telescope_gate[event_adcs[l]]) && !(tacgate[event_adcs[l]])) {
		    g_newm[g_num] = uchanm[l];
		    g_newp[g_num] = uchanp[l];
		    /* --- now let's see if it is a clover --- */
		    if (num_clovers) {
		      /* --- we have clovers --- */
		      if ((clover_gates[event_adcs[l]]) && (!(clover_hit[BitLog2(clover_gates[event_adcs[l]])]))) {
			/* --- mark the clover as used --- */
			clover_hit[BitLog2(clover_gates[event_adcs[l]])] = 1;
			/* --- it is a clover and we have not already used it --- */
			for (n = l+1; n < numadc; n++) {
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
			  if(event_adcs[n]<0) continue;

			  /* --- cycling through the rest of the event --- */
			  if (clover_gates[event_adcs[n]] & clover_gates[event_adcs[l]]) {
			    /* -- it is the same clover --- */
			    //--ddc this is not necessarily at event_adc[0]!
			    //clover_hit[BitLog2(clover_gates[event_adcs[0]])] += 1;
			    clover_hit[BitLog2(clover_gates[event_adcs[l]])] += 1;
			    if (selfsuppressclovers) {
			      g_newm[g_num] = -1;
			      g_newp[g_num] = -1;
			    } else { // end of self suppress clovers condition
			      g_newm[g_num] += uchanm[n];
			      g_newp[g_num] += uchanp[n];
			    } // not self suppress clovers
			  }
			}
		      }
		    }
		    g_num++;
		  } // end of codition where it is only a gamma detector
		  } // done getting the gamma energies
	      
		/* --- at this point we have both the gamma info and the part info --- */
		for (l = 0; l < g_num; l++) {
		  /* --- first let's deal with the gamma gate --- */
		  if ((pgamsortgategamma) && (passgammagate)) {
		    for (q = 0; q < pgamgammagates.num; q++) {
		      if ((g_newm[l] * fin_cal >= pgamgammagates.min[q]) && (g_newp[l] * fin_cal <= pgamgammagates.max[q])) goto nextgammapartsqr;
		    }
		  }
		  for (m = 0; m < num_scopes; m++) {
		    /* --- first let's deal with the energy particle gate --- */
		    if ((scope_newm[m] >= partenergies[0]) && (scope_newp[m] <= partenergies[1]) && pgamsortgateparten) goto nextpartpartsqr;
		      newm = (int) g_newm[l];
		      newp = (int) g_newp[l];
		      if ((g_newm[l] - newm) > 0.5) newm++;
		      if ((g_newp[l] - newp) > 0.5) newp++;
		      if ((newm > 0) && (newp < pgammatrixdata.size)) {
			if (g_newp[l] != g_newm[l]) frac = 1.0 / (g_newp[l] - g_newm[l]);
			else frac = 0;
			newme = (int) scope_newm[m];
			newpe = (int) scope_newp[m];
			if ((scope_newm[m] - newme) > 0.5) newme++;
			if ((scope_newp[m] - newpe) > 0.5) newpe++;
			if ((newme > 0) && (newpe < pgammatrixdata.size)) {
			  if (scope_newm[m] != scope_newp[m]) frace = 1.0 / (scope_newp[m] - scope_newm[m]);
			  else frace = 0;
			  for (n = newm; n <= newp; n++) {
			    tm = g_newm[l];
			    tp = g_newp[l];
			    if (tm < ((float) n - 0.5)) tm = ((float) n - 0.5);
			    if (tp > ((float) n + 0.5)) tp = ((float) n + 0.5);
			    for (o = newme; o <= newpe; o++) {
			      tme = scope_newm[m];
			      tpe = scope_newp[m];
			      if (tme < ((float) o - 0.5)) tme = ((float) o - 0.5);
			      if (tpe > ((float) o + 0.5)) tpe = ((float) o + 0.5);
			      *(pgammatrixdata.data + pgammatrixdata.size * n + o) += frac * frace * (tp - tm) * (tpe - tme);
			    }
			  }
			}
		      }
		  nextpartpartsqr:
		      continue;
		  }		  
		nextgammapartsqr:
		  continue;
		}
	      }	    
	      /* --- done dealing with particle - gamma squares --- */
	      
	      /* --- now we have to deal with gamma-gamma matrices --- */
	      
	      if ((pgamsortoutputgg) && ((numadc) > 1)) {
		/* 6-11-04 JRP on previous line, numadc was numadc-telescopes_hit, change
		   to make compatiable with generic 2D sort when using an axis file */
		int whichgamma;
		
		whichgamma = -1;
		/* --- we should begin by initializing the clover_hit
		   --- array, provided that we have clovers --- */
		if (num_clovers) {
		  for (l = 0; l < num_clovers; l++) {
		    clover_hit[l] = 0;
		  }
		}
		/* --- the best way to do this is to begin by constructing the list of events for each axis
		   --- for each axis --- */
		/* --- begin by initializing the nuchanmx, nuchanpx,
		   nuchanmy, nuchanpy, xaxiscount, and yaxiscount variables --- */
		for (l = 0; l < xaxiscount; l++) {
		  nuchanxm[l] = 0; nuchanxp[l] = 0;
		}
		xaxiscount = 0;
		for (l = 0; l < yaxiscount; l++) {
		  nuchanym[l] = 0; nuchanyp[l] = 0;
		}
		yaxiscount = 0;
		/* --- now we need to populate those lists --- */
		for (l = 0; l < numadc; l++) {
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
		  if(event_adcs[l]<0) continue;

		  /* --- begin by getting the channel numbers for each --- */
		  //--ddc jan15 additional variables for SLT's doppler correction.
		  anewm = uchanm[l];
		  anewp = uchanp[l];
		  anewym = uchanym[l];
		  anewyp = uchanyp[l];

		  /* --- deal with possiblity of multi-segmented detectors --- */
		  if (num_clovers) {
		    
		    /* --- first let's see if this event was in a clover --- */
		    if (clover_gates[event_adcs[l]]) {
		      
		      /* --- if it is in a clover, make sure we havn't already used it --- */
		      if ((clover_hit[BitLog2(clover_gates[event_adcs[l]])])) {
			
			/* --- this is the case where the event was 
			   --- already used in a addback --- */
			anewm = -1;
			anewp = -1;

		      } else {
			
			/* --- this is the case where he event has not 
			   --- already been used in an addback --- */
			
			/* --- first mark the clover as used --- */
			clover_hit[BitLog2(clover_gates[event_adcs[l]])] = 1;
			
			/* --- now perform the addback function --- */
			/* --- begin by cycling through the rest of the adcs that
			   --- fired in this event --- */
			for (n = 0; n < numadc; n++) {

//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
			  if(event_adcs[n]<0) continue;
			  
			  /* --- avoid double counting --- */
			  if (n != l) {
			    
			    /* --- first check to see if n was in the same clover as l --- */
			    if (clover_gates[event_adcs[n]] & clover_gates[event_adcs[l]]) {
			      
			      /* --- if n is in the same clover as l, 
				 --- add the energies of n to those of l --- */
			      clover_hit[BitLog2(clover_gates[event_adcs[l]])] += 1;
	    //--ddc jan15 additional variables for SLT's doppler correction.
			      if (selfsuppressclovers) {
				anewm = -1;
				anewp = -1;
				anewym = -1;
				anewyp = -1;
			      } else {
				anewm += uchanm[n];
				anewp += uchanp[n];
				anewym += uchanym[n];
				anewyp += uchanyp[n];
			      } // done with selfsuppressclovers;
			      
				/* --- that should do it --- */
			      
			    } // done checking if n is in the same clover as l
			    
			  } // done avoiding double counting while cycling through l
			  
			} // done cycling through the event for addback to l

                        //--ddc ??!! shouldn't the "virtual adc" for the clover be filled here?
                        //--ddc ??!! the entire concept of virtual adc seems to be moot for 2d arrays
                        //--ddc ??!! if one were to fill the virtual adc here, it would have to be (it appears)
                        //--ddc ??!! put into the arrays (xadcs,yadcs,etc) in case2 in the next "if" right here....
			
		      } // done checking if event l was already used in a previous clover addback
		      /* --- clover min multipolarity --- */
		      if (clover_hit[BitLog2(clover_gates[event_adcs[l]])] < min_clover_multipolarity) {
			anewm = -1; anewp = -1;
		      }
		    } // done checking if the event l is in a clover
		    /* --- ok, now we have the new upper and lower channel numbers,
		       let us now determine if this event belongs in either the x or y (or both) lists --- */
		  }
		  if ((anewm > 0) && (anewp > 0)) {
		    switch (pgamsortoutputggtype) {
		    case 1:
		      /* --- this is the case of making a gamma-gamma triangle --- */
		      /* --- make sure that the event is not designated as something other
			 --- than a gamma ray detector --- */
		      if (!(tacgate[event_adcs[l]]) && 
			  ((!(telescope_gate[event_adcs[l]]) && 
			    !(detelescope_gate[event_adcs[l]])) || 
			   (special_telescope_gate[event_adcs[l]]))) {
			/* --- in this case every event belongs on both axes --- */
			nuchanxm[xaxiscount] = nuchanym[yaxiscount] = anewm;
			nuchanxp[xaxiscount] = nuchanyp[yaxiscount] = anewp;
		      xaxiscount++;
		      yaxiscount++;
		      }
		      break;
		    case 2:
		      /* --- this is the case of a generic 2D matrix --- */
		      /* --- in this case we begin by determining if we have 
			 --- lists of detectors --- */
		      if ((numxdets) && (numydets)) {
		      /* --- ok we have a list of detectors --- */
			if (xaxisgate[inverse_adc_map[event_adcs[l]]]) {
			  nuchanxm[xaxiscount] = anewm;
			  nuchanxp[xaxiscount] = anewp;
			  //--ddc add map of adcs on axis
			  //	       xaxiscount++;
			  xadcs[xaxiscount++]=event_adcs[l];

			}
			//--ddc jan15 changes for SLT's doppler correction.
			if (yaxisgate[inverse_adc_map[event_adcs[l]]]) {
			  //nuchanym[yaxiscount] = anewm;
			  //nuchanyp[yaxiscount] = anewp;
			  nuchanym[yaxiscount] = anewym;
			  nuchanyp[yaxiscount] = anewyp;
			  //--ddc add map of adcs on axis
			  //yaxiscount++;
			  yadcs[yaxiscount++]=event_adcs[l];
			}
		      } else {
			/* --- wide open otherwise --- */
			nuchanxm[xaxiscount] = anewm;
			nuchanxp[xaxiscount] = anewp;
			nuchanym[yaxiscount] = anewym;
			nuchanyp[yaxiscount] = anewyp;
			xaxiscount++;
			yaxiscount++;
		      }
		      break;
		    } // end of output type switch
		  }
		} // end of l loop
		
		/* --- ok now we have the list of everything that should go on the x and y axes --- */
		
		/* --- we now want to cycle through the events and write them to the matrix --- */
		/* --- we have to do this slightly differently if we want to apply gamma gates --- */

		//--ddc 22jun06 This section for gamma gates is "hosed".  WHY would JRP only test
                // gammarays from the xaxis against the gates?  All gammarays have to be tested
                // if any ONE makes the test, ALL the others that DO NOT match a gate should be 
		// put in the matrix! By doing this HERE, when we have a gamma that passes the test, 
		// we have to eliminate IT from both the x and y axes.  Some duplication in testing
		// can probably be eliminated if the gating is applied when the lists are created...
		// but I'll put it here to minimize the impact of any unintended change:( 
		if (pgamsortgategamma) {
		  //--ddc 22jun06 First, make lists of gammas in the gates (xgated and ygated)
		  xgatedpointer=0; ygatedpointer=0;
		  for (p = 0; p < xaxiscount; p++) {
		    for (q = 0; q < pgamgammagates.num; q++) {
		      if ((nuchanxm[p] * fin_cal >= pgamgammagates.min[q]) && 
                          (nuchanxp[p] * fin_cal <= pgamgammagates.max[q])){
			xgated[xgatedpointer++] = p;
			break;  //--ddc only one gamma in the list please, in case any gates overlap.
		      }
		    }
		  }
		  for (p = 0; p < yaxiscount; p++) {
		    for (q = 0; q < pgamgammagates.num; q++) {
		      if ((nuchanym[p] * fin_cal >= pgamgammagates.min[q]) && 
                          (nuchanyp[p] * fin_cal <= pgamgammagates.max[q])) {
			ygated[ygatedpointer++] = p;
			break;
		      }

		    }
		  }
		  //--ddc 22jun06 now, IF there is ANY gated gamma, go through and put the rest in the
		  // matrix.  I'll use the indexes p and q to keep track of the gammarays already 
		  // used for a gate. if a gammaray index matches one that was in a gate, 
		  // the loop is skipped
		  p = q = 0;
		  if(ygatedpointer != 0 || xgatedpointer !=0 ) {
			for (l=0; l < xaxiscount; l++) {
			  //--ddc 22jun06 skip the rest if we are here because this was in gate
			  if(p<xgatedpointer) {
			    if ( l == xgated[p] ) {
			      p++;
			      continue;
			    }
			  }
			  anewm = nuchanxm[l];
			  anewp = nuchanxp[l];
			  newm = (int) anewm;
			  newp = (int) anewp;
			  if (anewm != anewp) frac = 1.0 / (anewp - anewm); 
			  if ((anewm - newm) > 0.5) newm++;
			  if ((anewp - newp) > 0.5) newp++;
			  q=0;
			  for (m=0; m < yaxiscount; m++) {
			  //--ddc 22jun06 skip the rest if we are here because this was in gate
			    if(q<ygatedpointer) {
			      if ( m == ygated[q] ) {
				q++;
				continue;
			      }
			    }
			    anewm3 = nuchanym[m];
			    anewp3 = nuchanyp[m];
			    newm3 = (int) anewm3;
			    newp3 = (int) anewp3;
			    if ((anewm3 - newm3) > 0.5) newm3++;
			    if ((anewp3 - newp3) > 0.5) newp3++;
			    if (anewp3 != anewm3) frac3 = 1.0 / (anewp3 - anewm3);
			    if ((newm >=0 ) && (newp < pgammatrixdata.size) &&
				(newm3 >= 0) && (newp3 < pgammatrixdata.size)) {
			      for (n = newm; n <= newp; n++) {
				/* --- x-axis variables --- */
				tm = anewm;
				tp = anewp;
				if (tm < ((float) n - 0.5)) tm = ((float) n - 0.5);
				if (tp > ((float) n + 0.5)) tp = ((float) n + 0.5);
				for (o = newm3; o <= newp3; o++) {
				  /* --- y-axis variables --- */
				  tm3 = anewm3;
				  tp3 = anewp3;
				  if (tm3 < ((float) o - 0.5)) tm3 = ((float) o - 0.5);
				  if (tp3 > ((float) o + 0.5)) tp3 = ((float) o + 0.5);
				  if ((n < pgammatrixdata.size) && (o < pgammatrixdata.size)) {
				      
				    switch (pgamsortoutputggtype) {
				    case 1:
				      if (l > m) {
					index = Max(n,o) - Min(n,o) + (isize - Min(n,o)) * 
					  ((float) Min(n,o)/ (float) 2);
					*(pgammatrixdata.data + index) = 
					  *(pgammatrixdata.data + index) + 
					  frac * frac3 * (tp - tm) * (tp3 - tm3);
				      }
				      break;
				    case 2:
				      //--ddc. don't use pairs from the same ADC.
				      if(xadcs[l]==yadcs[m]) break;
				      *(pgammatrixdata.data + n + o * pgammatrixdata.size) = 
					*(pgammatrixdata.data + n + o * pgammatrixdata.size) + 
					frac * frac3 * (tp - tm) * (tp3 - tm3);
					 
				      break;
				    }
				  }
				}
			      }
			    }
			  } // done with m count
			} // done with l count
		  } //--ddc 22jun06 Done with code changes for applying gates to gamma-gamma

		} else { // end of applying gamma gates
		for (l=0; l < xaxiscount; l++) {
		  anewm = nuchanxm[l];
		  anewp = nuchanxp[l];
		  newm = (int) anewm;
		  newp = (int) anewp;
		  if (anewm != anewp) frac = 1.0 / (anewp - anewm); 
		  if ((anewm - newm) > 0.5) newm++;
		  if ((anewp - newp) > 0.5) newp++;
		  for (m=0; m < yaxiscount; m++) {
		    anewm3 = nuchanym[m];
		    anewp3 = nuchanyp[m];
		    newm3 = (int) anewm3;
		    newp3 = (int) anewp3;
		    if ((anewm3 - newm3) > 0.5) newm3++;
		    if ((anewp3 - newp3) > 0.5) newp3++;
		    if (anewp3 != anewm3) frac3 = 1.0 / (anewp3 - anewm3);
		    if ((newm >=0 ) && (newp < pgammatrixdata.size) &&
			(newm3 >= 0) && (newp3 < pgammatrixdata.size)) {
		      for (n = newm; n <= newp; n++) {
			/* --- x-axis variables --- */
			tm = anewm;
			tp = anewp;
			if (tm < ((float) n - 0.5)) tm = ((float) n - 0.5);
			if (tp > ((float) n + 0.5)) tp = ((float) n + 0.5);
			for (o = newm3; o <= newp3; o++) {
			  /* --- y-axis variables --- */
			  tm3 = anewm3;
			  tp3 = anewp3;
			  if (tm3 < ((float) o - 0.5)) tm3 = ((float) o - 0.5);
			  if (tp3 > ((float) o + 0.5)) tp3 = ((float) o + 0.5);
			  if ((n < pgammatrixdata.size) && (o < pgammatrixdata.size)) {
			    
			    switch (pgamsortoutputggtype) {
			    case 1:
			      if (l > m) {
				index = Max(n,o) - Min(n,o) + (isize - Min(n,o)) * 
				  ((float) Min(n,o)/ (float) 2);
				*(pgammatrixdata.data + index) = 
				  *(pgammatrixdata.data + index) + 
				    frac * frac3 * (tp - tm) * (tp3 - tm3);
			      }
			      break;
			    case 2:
				//--ddc. don't use pairs from the same ADC.
				if(xadcs[l]==yadcs[m]) break;

				*(pgammatrixdata.data + n + o * pgammatrixdata.size) = 
				*(pgammatrixdata.data + n + o * pgammatrixdata.size) + 
				  frac * frac3 * (tp - tm) * (tp3 - tm3);
				break;
			    }
			  }
			}
		      }
		    }
		  } // done with m count
		} // done with l count
		}
		//		  if (pgamsortgategamma) {
		//   for (m = 0; m < pgamgammagates.num; m++) {
		//     if ((anewm * fin_cal >= pgamgammagates.min[m]) && (anewp * fin_cal <= pgamgammagates.max[m])) {
		//	whichgamma = m;
		//	if (passgammagate == 1)
		//	  goto nextlgg;
		//     }
		//   }
		// }
		
	      }  // end of pgamsortoutputgg switch

	      /* --- done dealing with gamma-gamma matricies --- */

	      /* --- now we need to deal with pair squares --- */
	      /* --- pretty much this requires at least 3 gamma-ray adcs firing --- */
	      
	      //--ddc nov14 for testing accept all events with a possible pair
	      //--ddc nov14 (2->1)
	      if ((pgamsortoutputpairsqr) && ((numadc - telescopes_hit) > 1)) {
		

		
		/* --- now we should determine if a pair was hit --- */
		/* --- this is going to be done by producing a bitfield uint
		   --- with the active adcs as 1's and & that with each
		   --- pair_flags entry --- */
		tempuint = 0;
		passedpairs = 0;
		//--ddc oct14 sum over num_clovers, when it should be numadc
		//		for ( l = 0; l < num_clovers; l++) {
		for ( l = 0; l < numadc; l++) {
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
		  if(event_adcs[l]<0) continue;

		  tempuint |= (uint)(1 << event_adcs[l]);
		}
		for (l = 0; l < pairs->num; l++ ) {
		  if ((tempuint & pair_flags[l]) == pair_flags[l]) {
		    passedpairs = 1;
		    goto passpairs;
		  }
		}
	      passpairs:
		if (!(passedpairs))
		  goto nextheader;
		
		/* --- at this point we know that we have an event with a valid pair --- */
		for (l = 0; l < numadc; l++) {

//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
		  if(event_adcs[l]<0) continue;
		  //-ddc debugging nov14 skip pair if mask won't make sense.
		  if(event_adcs[l]>31) continue;
		  /* --- lets do the processing of the pair first --- */
		  for (m = 0; m < pairs->num; m++) {
		    if (((uint)(1 << event_adcs[l])) & pair_flags[m]) {

		      /* --- now that we know it is in a pair, we need to find the other pair --- */
		      /* --- if there are clovers, we should initialize a clover_hit array --- */

//--ddc oct14 if we don't want to double count, we only check the remaining adcs... n should start with l
//--ddc		      for (n = 0; n < numadc; n++) {
		      for (n = l; n < numadc; n++) {
			
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
			if(event_adcs[n]<0) continue;
			//-ddc debugging nov14 skip pair if mask won't make sense.
			if(event_adcs[n]>31) continue;
			/* --- now we need to check to see if n is in the same pair as l --- */
			if ((((uint) (1 << event_adcs[l])) | ((uint) (1 << event_adcs[n]))) ==
			  pair_flags[m]) {

			  /* --- at this point we know that both l and n are in the same pair --- */
			  /* --- therefore let's do the add-back thingy --- */
			  //--ddc jan15 add SLT's doppler correction.
			  if (fin_cal > 0) {
			    anewm = uchanm[l];
			    anewm += uchanm[n];
			    anewp = uchanp[l];
			    anewp+= uchanp[n];
			    anewym = uchanym[l];
			    anewym += uchanym[n];
			    anewyp = uchanyp[l];
			    anewyp+= uchanyp[n];
			    /* --- now we have anewm and anewp for the pair --- */
			    /* --- let's do a little bit of pre-processing and then unpack
			       --- the rest of the event --- */
			    newm = (int) anewm;
			    newp = (int) anewp;
			    if ((anewm - newm) > 0.5) newm++;
			    if ((anewp - newp) > 0.5) newp++;
			    if ((newm > 0) && (newp < fin_chans)) {
			      if (anewm != anewp) frac = 1.0 / (anewp - anewm);

			      /* --- ok that's all the pre-processing we can do --- */
			      /* --- now we want to get the information about the other gamma
				 --- ray detectors --- */		
			      /* --- if there are clovers, we should initialize a clover_hit array --- */
			      if (num_clovers) {
			      				for ( o = 0; o < num_clovers; o++) {
			      				  clover_hit[o] = 0;
			      				}
			      }
			      /* --- there now that is initialized --- */
			      for (o = 0; o < numadc; o++) {
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
				if(event_adcs[o]<0) continue;

				  /* --- we don't want a telescope, or tac --- */
				//--ddc nov14 for testing accept all events with a possible pair
				//--ddc nov14 even a pair against itself..  REMOVE o!=l !
			        if ((o!=l) && (o!=n) && (((!(telescope_gate[event_adcs[o]])) &&
				    (!(detelescope_gate[event_adcs[o]]))) || special_telescope_gate[event_adcs[o]]) &&
				     (!(tacgate[event_adcs[o]]))) {
				  
				  /* --- start by unpacking the event assuming 1 crystal --- */
				  anewm2 = uchanm[o];
				  anewp2 = uchanp[o];

				  /* --- at this point we need to know if A: we have clovers
				     --- and B: is this event in a clover --- */
				  
				  if (num_clovers) {
				    
				    if (clover_gates[event_adcs[o]]) { 
				      /* --- need to make sure this isn't the clover a pair is in --- */
				      if ((clover_gates[event_adcs[l]] & clover_gates[event_adcs[o]]) ||
					  (clover_gates[event_adcs[n]] & clover_gates[event_adcs[o]])){
					//--ddc nov14 can't toss EVERYTHING because the pair is in ANY clover!!
					//--ddc newm = -1;
					//--ddc newp = -1;
					//--ddc nov14 BUT if we set the 'clover_hit' for clover with event_adcs[o]
					//--ddc nov14 AND just let it continue, this clover is excluded, but
					//--ddc nov14 the pair should still be histogrammed with other clovers..
					//--ddc nov14 HOWEVER for testing accept all events with a possible pair
					//--ddc nov14 comment this out.. 
					clover_hit[BitLog2(clover_gates[event_adcs[o]])] = 1;
				      }

				      /* --- need to check if it is already used --- */
				      if (clover_hit[BitLog2(clover_gates[event_adcs[o]])]) {
					/* --- this is the case where we have already used the clover --- */
					anewm2 = -1;
					anewp2 = -1;
				      } else {
					/* --- case where we have not already used the clover --- */

					/* --- first mark it as used --- */
					//--ddc oct14.  Fix index for clover_hit array... not for adc 'l'!
					//clover_hit[BitLog2(clover_gates[event_adcs[l]])] = 1;
					clover_hit[BitLog2(clover_gates[event_adcs[o]])] = 1;

					/* --- now lets start the add-back function --- */
					for (p = 0; p < numadc; p++) {
//--ddc debug.  If an event had an adc that was not in a map, it is 
//              mapped to -1!!! so skip the rest if this is true!
					  if(event_adcs[p]<0) continue;

					  //--ddc nov14 LOOKS like a redundancy, test on l and n ... removing
					  //if ((p != o) && (p != l) && (p != n) && 
					  if ((p != o) && (clover_gates[event_adcs[o]] 
							   & clover_gates[event_adcs[p]])) {
					    //--ddc oct14.  Fix index for clover_hit array... not for adc 'l'!
					    //clover_hit[BitLog2(clover_gates[event_adcs[l]])] += 1;
					    clover_hit[BitLog2(clover_gates[event_adcs[p]])] += 1;
					    if (selfsuppressclovers) {
					      anewm2 = -1;
					      anewp2 = -1;
					    } else {
					      anewm2 += uchanm[p];
					      //--ddc oct14 variable error
					      // anewm2 += uchanp[p];
					      anewp2 += uchanp[p];
					    } // done with addback function
					    					    
					  } // done making sure we don't double count
					  // and that we are in the same clover
					  
					} // cycling through the rest of the event to find
					// other parts of the crystal p
					
				      } // case where the clover has not already been used
				      
				    } // done with case where the clover was hit

				  } // done seeing if we have a clover 

				  /* --- now we have anewm,anewp,anewm2,anewp2 and can write --- */

				  /* --- clover_hit is only defined if there are clovers --- */
				  if ((anewm2 != -1) && (anewp2 != -1)) { 
				    if (num_clovers) {
				      //--ddc oct14 this test for clovers is completely wrong...
				      //      if (clover_hit[BitLog2(clover_gates[event_adcs[o]])] 
				      //		  >= min_clover_multipolarity) {
				      //--ddc change to way test was done in gamma gamma
				      if (clover_hit[BitLog2(clover_gates[event_adcs[o]])] 
				      		  < min_clover_multipolarity) {
					goto nextminclovermultipolarityc;
				      }
				    }
				    
				    newm2 = (int) anewm2;
				    newp2 = (int) anewp2;
				    if ((anewm2 - newm2) > 0.5) newm2++;
				    if ((anewp2 - newp2) > 0.5) newp2++;
				    if (anewm2 != anewp2) frac2 = 1.0 / (anewp2 - anewm2);
				    if ((newm2 > 0) && (newp2 < fin_chans)
					&& (newm > 0) && (newp < fin_chans)) {
				      for (p = newm; p <= newp; p++) {
					tm = anewm;
					tp = anewp;
					if (tm < ((float) p - 0.5)) tm = ((float) p - 0.5);
					if (tp > ((float) p + 0.5)) tp = ((float) p + 0.5);
					for (q = newm2; q <= newp2; q++) {
					  tm2 = anewm2;
					  tp2 = anewp2;
					  if (tm2 < ((float) q - 0.5)) tm2 = ((float) q - 0.5);
					  if (tp2 > ((float) q + 0.5)) tp2 = ((float) q + 0.5);
					    *(pgammatrixdata.data + p + q * pgammatrixdata.size) +=
					      frac * frac2 * (tp - tm) * (tp2 - tm2);
					}
				      }

				    } // done making sure we don't go out of bounds
				    

				  nextminclovermultipolarityc:
				    continue;
				  } // done with should we write check

				} // done with making sure we arn't double counting

			      } // done cycling through the 2nd event o

			    } // making sure we don't go out of bounds

			  } // making sure we don't get a divide by zero error

			} // done seeing if n is in the same pair as l

		      } // done looping through finding the other adc (n)

		    } // done seeing if l is in a pair

		  } // done looping through the list of pairs for l

		} // done cycling the adcs/event

	      } // done dealing with pgamsortoutputpairsqr

	    } 
	    /* --- */
	    
	    /* --- if we are using ev2 files then we need to incrament k --- */
	    //if (datafiletype == 1) k++;
	    //printf("Got to stop 1.\n");

	  } // cycled through the events
	bufferdone:
	  //k = 0;
	  /* --- the end of a buffer, and the reading of the next buffer 
	     --- must be handled differently for .evt and .ev2 files --- */
	  switch (datafiletype) {
	  case 1:
	    //	    printf("Bufferdone called.\n");
	    //howmuch = fread(ev2buffer,1,EV2BUFFERSIZE,infile);
	    if (ftell(infile) >= filebuffer.st_size)
	      goto stopreading;
            //--ddc debug 0605 ok... how did this not happen before?!
            //if at this point k is 8192, this endless superloop is going
            //to go back to where it tests 'stillreading' and shortly 
            //thereafter, set k to zero without reading another buffer!!
            //The next two lines are to prevent this!
            howmuch = fread(ev2buffer,1,EV2BUFFERSIZE,infile);
            if(howmuch==0) goto stopreading;
	    break;
	  default:
	    k = 0;
	    /* --- at the end of the buffer-reading loop, we need to
	       --- read in and deal with the beginning of the next fortran record */
	    if (fread(&buffersize[0],sizeof(int),1,infile) != 1) {
	      stillreading = 0;
	      goto stopreading;
	    } else {
	      buffer = (short int *) realloc(buffer,buffersize[0]);
	    }
	    howmuch = (fread(buffer,1,buffersize[0],infile) == buffersize[0]);
	    break;
	  }
	} // done reading the buffer(s)
	//printf("Got to test print 1.\n");
	/* --- close out the important stuff --- */
      stopreading:
	if (infile != NULL) {
	  printf("%d records sorted.\n",irec);
	  //printf("attempting to close .evt file.\n");
	  progbarlastbytes += ftell(infile);
	  fclose(infile);
	  infile = NULL;
	}
	//printf("Attempting to close the progress bar window.\n");
	//printf("successfully closed the progress bar window.\n");
      } // done with the file
    } // done going through each run for a particular setup file
  } // done with all the stup files

  /* --- have to copy all the temphists to memory --- */
  if (!(truesingles) && (pgamsortoutputhists)){
    for (i = 0; i < max_adcs; i++) {
      for (j = 0; j < fin_chans; j++) {
	*(histloc[i] + j) = (int) *(temphist[i] + j);
      }
    }
  }

 stopsort:
  EndSortProgress();
  /* --- when we are done with the sort, we need to get
     --- rid of the local memory allocations --- */
  if (pgamsortgateveto)
    VetoStructDestroy(vetos);
  if (pgamsortgaterequires)
    VetoStructDestroy(requires);
  if (adc_map !=NULL) {
    free(adc_map);
    adc_map = NULL;
  }
  //  for (i = (max_adcs - 1); i >= 0; i--) {
  //    if (temphist != NULL) {
  //      printf("%d\n",i);
  //      g_free (temphist[i]);
  //     temphist[i] = NULL;
  //  }
  // }
  if (not_a_sum_hit != NULL) {
    free(not_a_sum_hit);
    not_a_sum_hit = NULL;
  }
  if (buffer != NULL) {
    free(buffer);
    buffer = NULL;
  }
  if (ev2buffer != NULL) {
    free(ev2buffer);
    ev2buffer = NULL;
  }
  //  if (uchanm != NULL) {
  //  free(uchanm);
  //  uchanm = NULL;
  //}
  //if (uchanp != NULL) {
  //  free(uchanp);
  //  uchanp = NULL;
  // }
  if (inverse_adc_map != NULL) {
    free(inverse_adc_map);
    inverse_adc_map = NULL;
  }
  if (pair_flags != NULL) {
    free(pair_flags);
    pair_flags = NULL;
  }
  // if (event_adcs != NULL) {
  //  free(event_adcs);
  //  event_adcs = NULL;
  // }
  //  if (event_channel != NULL) {
  //  free(event_channel);
  //  event_channel = NULL;
  // }
  if (clover_gates != NULL) {
    free(clover_gates);
    clover_gates = NULL;
  }
  if (telescope_gate != NULL) {
    free(telescope_gate);
    telescope_gate = NULL;
  }
  if (special_telescope_gate != NULL) {
    free(special_telescope_gate);
    special_telescope_gate = NULL;
  }
  if (detector_info_map != NULL) {
    free(detector_info_map);
    detector_info_map = NULL;
  }
  if (clover_hit != NULL) {
    free(clover_hit);
    clover_hit = NULL;
  }
  if (clover_hit2 != NULL) {
    free(clover_hit2);
    clover_hit2 = NULL;
  }
}

/* PassTACGate
 *
 * Checks to see if a particular adc and channel are inside a tac gate
 * if there are no tac gates in the setupfile then it returns true
 * otherwise, it returns true if the event passes the tac gate conditions
 */
int PassTACGate(int adc, int channel)
{
  int i,j,k,test;
  
  if (num_tacs == 0) {
    return (1);
  } else {
    for (i = 0; i < num_tacs; i++) {
      for (j = 0; j < tacs[i].num_adcs; j++) {
	if ((adc == tacs[i].gates[j].adc)
	    && (channel >= tacs[i].gates[j].min) && (channel <= tacs[i].gates[j].max)) {
	  return (1);
	}
      }
    }
  }
}

/* LongBitLog2
 *
 * Returns the number of the first bit which is positive in arg
 */
int LongBitLog2(unsigned long long arg)
{
  int i;
  for (i = 0; i < 64; i++) {
    if (arg & (unsigned long long)((unsigned long long ) 1 << i))
      return (i);
  }
  return(0);
}

/* BitLog2
 *
 * Returns the number of the first bit which is positive in arg
 */
int BitLog2(uint arg)
{
  int digit;

  /* --- binary search method --- */

  if (arg >= 65536) {
    /* --- in the upper 2 bytes --- */
    if (arg >= 16777216) {
      /* --- in the upper byte --- */
      if (arg >= 268435456) {
	/* --- in the upper nyble --- */
	if (arg >= 1073741824) {
	  /* --- in the upper 2 bits --- */
	  if (arg == 1073741824) {
	    return(30);
	  } else {
	    /* --- arg == 2147483648 --- */
	    return(31);
	  }
	} else {
	  /* --- in the lower 2 bits --- */
	  if (arg == 268435456) {
	    return(28);
	  } else {
	    /* --- arg == 536870912 --- */
	    return(29);
	  }
	}
      } else {
	/* --- in the lower nyble --- */
	if (arg >= 67108864) {
	  /* --- in the upper 2 bits --- */
	  if (arg == 67108864) {
	    return(26);
	  } else {
	    /* --- arg == 134217728 --- */
	    return(27);
	  }
	} else {
	  /* --- in the lower 2 bits --- */
	  if (arg == 16777216) {
	    return(24);
	  } else {
	    /* --- arg == 33554432 --- */
	    return(25);
	  }
	}
      }
    } else {
      /* --- in the lower byte --- */
      if (arg >= 1048576) {
	/* --- in the upper nyble --- */
	if (arg >= 4194304) {
	  /* --- upper 2 bits --- */
	  if (arg == 4194304) {
	    return(22);
	  } else {
	    /* --- arg == 8388608 --- */
	    return(23);
	  }
	} else {
	  /* --- lower 2 bits --- */
	  if (arg == 1048576) {
	    return(20);
	  } else {
	    /* --- arg == 2097152 --- */
	    return(21);
	  }
	}
      } else {
	/* --- in the lower nyble --- */
	if (arg >= 262144) {
	  /* --- upper 2 bits --- */
	  if (arg == 262144) {
	    return(18);
	  } else {
	    /* --- arg == 524288 --- */
	    return(19);
	  }
	} else {
	  /* --- lower 2 bits --- */
	  if (arg == 65536) {
	    return(16);
	  } else {
	    /* --- arg == 131072 --- */
	    return(17);
	  }
	}
      }
    }    
  } else {
    /* --- in the lower 2 bytes --- */
    if ( arg >= 256) {
      /* --- in the upper byte --- */
      if (arg >= 4096) {
	/* --- in the upper nyble --- */
	if (arg >= 16384) {
	  /* --- in the upper 2 bits --- */
	  if (arg == 16384) {
	    return(14);
	  } else {
	    /* --- arg = 32768 --- */
	    return(15);
	  }
	} else {
	  /* --- in the lower 2 bits --- */
	  if (arg == 4096) {
	    return(12);
	  } else {
	    /* --- arg == 8192 --- */
	    return(13);
	  }
	}
      } else {
	/* --- in the lower nyble --- */
	if (arg >= 1024) {
	  /* --- in the upper 2 bits --- */
	  if (arg == 1024){
	    return(10);
	  } else {
	    /* --- arg == 2048 --- */
	    return(11);
	  }
	} else {
	  /* --- in the lower 2 bits --- */
	  if (arg == 256) {
	    return(8);
	  } else {
	    /* --- arg == 512 --- */
	    if (arg == 512) {
	      return(9);
	    }
	  }
	}
      }
    } else {
      /* --- in the lower byte --- */
      if (arg >= 16) {
	/* --- in the upper nyble --- */
	if (arg >= 64) {
	  /* --- in the upper 2 bits --- */
	  if (arg == 64) {
	    return(6);
	  } else {
	    /* --- arg == 128 --- */
	    return(7);
	  }
	} else {
	  /* --- in the lower 2 bits --- */
	  if (arg == 16) {
	    return (4);
	  } else {
	    /* --- arg == 32 --- */
	    return(5);
	  }
	}
      } else {
	/* --- in the lower nyble --- */
	if (arg >= 4) {
	  /* --- in the upper 2 bits --- */
	  if (arg == 4) {
	    return (2);
	  } else {
	    /* --- arg == 8 --- */
	    return(3);
	  }
	} else {
	  /* --- in the lower 2 bits --- */
	  if (arg == 1) {
	    return (0);
	  } else {
	    /* --- arg == 2 --- */
	    return (1);
	  }
	}
      }
    }
  }

  /* --- below is the commented out linear search method --- */

  //  for (digit = 0; arg; digit++)
  //  arg = arg >> 1;
  //return(digit - 1);
}

/* PgamPairsNew
 *
 * creates a PgamPairs struct
 */
struct pgam_pairs *PgamPairsNew()
{
  struct pgam_pairs *pairs;

  pairs = (struct pgam_pairs *) malloc(sizeof(struct pgam_pairs));
  pairs->num = 0;
  pairs->pairs = NULL;

  return(pairs);
}

/*  PgamPairsDestroy
 *
 * Destroys a PgamPairs struct
 */
void PgamPairsDestroy(struct pgam_pairs *pairs)
{
  if (pairs != NULL) {
    if (pairs->pairs != NULL)
      free(pairs->pairs);
    free(pairs);
  }
}

/* PgamPairsAdd
 *
 * Adds a pair to a pgampairs struct
 */
void PgamPairsAdd(struct pgam_pairs *pairs, int a, int b)
{
  if (pairs->pairs == NULL) {
    pairs->pairs = (struct pgam_pair *) malloc(sizeof(struct pgam_pair));
    pairs->num = 1;
  } else {
    pairs->num += 1;
    pairs->pairs = (struct pgam_pair *) realloc(pairs->pairs,
						sizeof(struct pgam_pair) * pairs->num);
  }
  pairs->pairs[pairs->num - 1].a = a;
  pairs->pairs[pairs->num - 1].b = b;
}

/* PgamReadPairs
 *
 * Reads in a file containing information about pairs
 */
struct pgam_pairs *PgamReadPairs(char *sFilename)
{
  struct pgam_pairs *pairs;
  int i,j,k;
  int test;
  FILE *infile;
  char dummystr[80];

  if ((infile = fopen(sFilename,"r")) != NULL) {
    pairs = PgamPairsNew();
    while (fgets(dummystr,80,infile) != NULL) {
      while(strncmp(dummystr,"(",1) == 0) {
	if (sscanf(dummystr,"(%d,%d)",&i,&j) == 2) {
	  PgamPairsAdd(pairs,i,j);
	  printf("added pairs, (%d,%d)\n",i,j);
	  if (strstr((char *)(dummystr + 1),"(") != NULL) 
	    strncpy(dummystr,strstr((char*)(dummystr + 1),"("),80);
	  else sprintf(dummystr," ");
	} 
      }
    }

    fclose(infile);
    return (pairs);
  }
}

/* PgamReadVeto
 *
 * Reads in a file containing information about the veto gate
 */
struct veto_struct *PgamReadVeto(char *sFilename)
{
  struct veto_struct *vetos;
  int i,j,k;
  int test;
  FILE *infile;
  char dummystr[80];

  if ((infile = fopen(sFilename,"r")) != NULL) {
    vetos = VetoStructNew();
    while (fgets(dummystr,80,infile) != NULL) {
      test = sscanf(dummystr,"%d,%d,%d",&i,&j,&k);
      if (test != 3)
	test = sscanf(dummystr,"%d %d %d",&i,&j,&k);
      if (test == 3){
	VetoStructAdd(vetos,i,j,k);
	printf("Added veto gate: %d %d %d\n",i,j,k);
      }
    }
  }
  return(vetos);
}

/* VetoStructNew
 *
 * Creates a new veto struct
 */
struct veto_struct *VetoStructNew()
{
  struct veto_struct *vetos;

  vetos = (struct veto_struct *) malloc(sizeof(struct veto_struct));
  vetos->num = 0;
  return (vetos);
}

/* VetoStructDestroy
 *
 * Destroys a veto_struct
 */
void VetoStructDestroy(struct veto_struct *vetos)
{
  g_return_if_fail(vetos != NULL);
  
  if (vetos->num != 0) {
    free(vetos->adc);
    free(vetos->min);
    free(vetos->max);
  }
  free(vetos);
  vetos = NULL;
}

/* VetoStructAdd
 *
 * Adds a veto gate to a veto struct
 */
void VetoStructAdd(struct veto_struct *vetos,int adc,int min, int max)
{
  g_return_if_fail(vetos != NULL);

  if (vetos->num == 0) {
    vetos->adc = (short *) malloc(sizeof(short));
    vetos->min = (short *) malloc(sizeof(short));
    vetos->max = (short *) malloc(sizeof(short));
    vetos->num = 1;
  } else {
    vetos->num += 1;
    vetos->adc = (short *) realloc(vetos->adc,sizeof(short) * vetos->num);
    vetos->min = (short *) realloc(vetos->min,sizeof(short) * vetos->num);
    vetos->max = (short *) realloc(vetos->max,sizeof(short) * vetos->num);
  }
  vetos->adc[vetos->num - 1] = adc;
  vetos->min[vetos->num - 1] = min;
  vetos->max[vetos->num - 1] = max;
}

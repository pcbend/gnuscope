/*

 * File: main.c
 * Auth: John Pavan
 *
 * Modeled on the custom widget demonstration from 
 *  "Developing Linux Applications" by Eric Harlow and
 *  "translate" a program by Dave Caussyn.
 *
 * Read in and display files in the following formats:
 *   .NSM traditional format for scope-fit type programs
 *   .spk ornl file formats
 *   ASCII files from Radware package
 *   ASCII NSM files from PC data acquisition.
 *   ASCII files for general use.
 */

//--ddc 3jun08 dbg, wrong pointer sent to strnlen (eg. replace &dummystr with
//--ddc with dummystr.
//--ddc nov10 many deprecations from gtk2 (eg. GTK_TEXT)

#include "gnuscopefuncs.h"
#include "cursors.h"

#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
//#include "fits.h"

#include "gnuscopeglobals.h"
#include "menus.h"
#include "logo.h"


/* --- Globals --- */
/* --- this should be handled by including gnuscopeglobals.h --- */

/* --- Function Declarations --- */

void PgamSort();
void ReadPgamAdd(char *sFilename);
void ReadPgamSub(char *sFilename);
struct veto_struct *PgamReadVeto(char *sFilename);
void DoubleGaussFit(float deviation1, float deviation2);
void AutoDetectFileType(char *sFilename);
float UseEfficiency (float counts, float energy);
float UseCalibration(float chan);
void TwodCreateDisplaySelectionWindow();
void FixedWidthGaussFit(float deviation);
void ReadAutoMatrix(char *sFilename);
void WriteAutoMatrix(char *sFilename);
void ExpFitWindow();
void AbortFunction(GtkWidget *widget, gpointer data);
void ExpFit(int mode, double c, double backa, double backb);
gint UpdateCursorX(GtkWidget *widget, GdkEventMotion *event);
void CursorPosQuer();
void Fit();
void CreateGainShiftWindow();
void MultipleToSingleFast();
void ProjectWindow();
void InitializeGlobals();
void SetCalibration(int i, float a, float b, float c);
void GetMessageDialog(const char *message);
void Help();
void DisplayBig();
void UpdateProgress (long pos, long len);
void StartProgress ();
void EndProgress ();
gint CloseAppWindow (GtkWidget *widget, gpointer *data);
void ReadGS (char *sFilename);
void WriteGS (char *sFilename);
void ShowParams();
void Project();

static gint CloseApp(GtkWidget *widget, gpointer data);
void ReadFile(char *sFilename);
void WriteFile(char *sFilename);
void LiphaFile(char *sFilename);
void ManualProject();
void ActiveUp();
void ActiveDown();
void CreateMainWindow();
void Sort(); 
void CreateDisplaySelectionWindow(int mode);
void GaussFit();
void ExpandPlot();
void ManualMarkSet(); 
void ShiftLeft();
void ShiftRight();
void NextSpectra();
void PrevSpectra();
void YScaleUp();
void YScaleDown();
void SetBackground (int mode);
void OneChSetBackground();
void ZoomOut();
void GetSum();
void Redraw ();
void AddSpectra();
void CompressSpectra();
void ReadBig();
void ProjectFull();
void OneChSetBackground ();
void SetMarker (GtkWidget *widget, GdkEventButton *event);
void create_bitmap_and_mask_from_xpm (GdkBitmap **bitmap, GdkBitmap **mask, gchar **xpm);
void PgamSort();
void PgamReadSetup(char *sFilename);
void PgamReadMasterSetup(char *sFilename);
void ReadPgamTwod(char *sFilename);
void WritePgamTwod(char *sFilename);
void PgamTwodProjectFull ();
void WriteBig(char *sFilename);
void ReadCalibrationFile(char *sFilename);
void SpawnPrintEntry(GtkWidget *widget,GtkWidget *entry);
void SpawnPrintPrompt(GtkWidget *text);
void SpawnPrint(char *cprinter);
void WritePostscript(char *sFilename);
void CreateSpecULatorWindow();
void ReadGates(char *sFilename);
void CreateTitleWindow();
void Compress2FileFromMenu(char *sFilename);
void Compress2FileNOGTK(char *sFilename);

//--ddc nov10 some functions to make gtk->gtk2
GtkTextBuffer* mytextbuffer( GtkWidget * );

/* --- Constants --- */
static int nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);

/* --- Functions --- */

/* --- Menu item functions --- */

static void MenuCompress2(GtkWidget *widget,
			    guint *callback_action)
{
  GetFilename("MUL->EV2",0,Compress2FileFromMenu);
}

static void MenuWriteRadwareMatrix(GtkWidget *widget,
			    guint *callback_action)
{
  GetFilename("Write Radware Matrix",1,WriteRadwareMatrix);
}

static void MenuSetYScaleMode(GtkWidget *widget,
			       guint *callback_action)
{
  int i;

  for (i = 0; i < 16; i++) {
    gtk_graph_set_y_scale_mode(GTK_GRAPH(graphsdisplayed[i]),*callback_action);
  }
  DisplayCurrentRange();
}

static void MenuSetBackgroundPoly(GtkWidget *widget,
				  guint *callback_action)
{
  backgroundpolydeg = *callback_action;
}

static void MenuReadPgamAdd(GtkWidget *widget,
			       guint *callback_action)
{
  GetFilename("Add Matrix",0,ReadPgamAdd);
}
static void MenuReadPgamSub(GtkWidget *widget,
			       guint *callback_action)
{
  GetFilename("Subtract Matrix",0,ReadPgamSub);
}
static void MenuAutoWriteMatrix(GtkWidget *widget,
			       guint *callback_action)
{
  GetFilename("Write Matrix",1,WriteAutoMatrix);
}
static void MenuAutoReadMatrix(GtkWidget *widget,
			       guint *callback_action)
{
  GetFilename("ReadMatrix",0,ReadAutoMatrix);
}
static void MenuSpecULator(GtkWidget *widget,
			       guint *callback_action)
{
  CreateSpecULatorWindow();
}

static void MenuPgamMatrix2Big(GtkWidget *widget,
				 guint *callback_action)
{
  PgamMatrix2Big();
}

static void MenuMultiInOne (GtkWidget *widget, guint *callback_action)
{
  CreateDisplaySelectionWindow(2);
  //  GetDialog(30);
}

static void MenuReadCalibInfo (GtkWidget *widget, guint *callback_action)
{
  GetFilename("CalibrationFile",0,ReadCalibrationFile);
}

static void MenuPeakFit (GtkWidget *widget, guint *callback_action)
{
  PeakFitWindow();
  //GetDialog(50);
}

static void MenuExpFit (GtkWidget *widget, guint *callback_action)
{
  ExpFitWindow();
}

static void MenuMultipleToSingleFast (GtkWidget *widget, guint *callback_action)
{
  MultipleToSingleFast();
}

static void MenuWriteBig (GtkWidget *widget, guint *callback_action)
{
  GetDialog(28);
  //GetFilename("Write Big", WriteBig);
}

static void MenuProjectionBox (GtkWidget *widget, 
				     guint *callback_action)
{
  ProjectWindow();
}

static void MenuWritePgamTwod (GtkWidget *widget, 
                                     guint *callback_action)
{
  GetFilename("Read Pgam Matrix", 1, WritePgamTwod);
}

static void MenuReadPgamTwod (GtkWidget *widget, 
                                     guint *callback_action)
{
  GetFilename("Read Pgam Matrix", 0, ReadPgamTwod);
}

static void MenuTwodProjectFull (GtkWidget *widget, guint *callback_action)
{
  PgamTwodProjectFull();
}

static void MenuReadPGAMSetup (GtkWidget *widget, guint *callback_action)
{
  GetFilename("Read Pgam Setup",0, PgamReadSetup);
}

static void MenuReadPGAMMasterSetup (GtkWidget *widget, 
                                     guint *callback_action)
{
    GetFilename("Read Pgam Master Setup",0, PgamReadMasterSetup);
}

static void MenuPGAMSort (GtkWidget *widget, guint *callback_action)
{
  PgamSortSelectionWindow();
}

static void MenuAbout (GtkWidget *widget, guint *callback_action)
{
  WriteMainText(" ---  Gnuscope  --- \n");
  WriteMainText("   version: beta 1.2  \n");
  WriteMainText("   Written by: John Pavan\n");
  WriteMainText("   Currently maintained by: D. Caussyn\n");
}


static void MenuDisplayBig (GtkWidget *widget, guint *callback_action)
{
  TwodCreateDisplaySelectionWindow();
  //DisplayBig();
}

static void MenuSetEfficiency (GtkWidget *widget, guint *callback_action)
{
  GetDialog(25);
}

static void MenuHelp (GtkWidget *widget, guint *callback_action)
{
  Help();
}

static void MenuSort (GtkWidget *widget, guint *callback_action)
{
  Sort();
}

static void MenuSetup (GtkWidget *widget, guint *callback_action)
{
  GetDialog(19);
}

static void MenuManualProject (GtkWidget *widget, guint *callback_action)
{
  ManualProject();
}

static void MenuProjectFull (GtkWidget *widget, guint *callback_action)
{
  ProjectFull();
}

static void MenuProject (GtkWidget *widget, guint *callback_action)
{
  Project();
}

static void MenuFixedWidthGaussFit (GtkWidget *widget, 
				    guint *callback_action)
{
  GetDialog(15);
}

static void MenuActiveUp (GtkWidget *widget, guint *callback_action)
{
  ActiveUp();
}

static void MenuActiveDown (GtkWidget *widget, guint *callback_action)
{
  ActiveDown();
}

static void MenuDoubleGaussFit (GtkWidget *widget, guint *callback_action)
{
  GetDialog(16);
}

static void MenuRedraw (GtkWidget *widget, guint *callback_action)
{
  if (gtk_graph_has_segments(GTK_GRAPH(graph))) gtk_graph_clear_segments(GTK_GRAPH(graph));
  Redraw();
}

static void MenuClearSpectra (GtkWidget *widget, guint *callback_action)
{
  GetDialog(12);
}

static void MenuGaussFit (GtkWidget *widget, guint *callback_action)
{
  GaussFit();
}

static void MenuGainshiftSpectra (GtkWidget *widget, 
				  guint *callback_action)
{
  CreateGainShiftWindow();
}

static void MenuNormalizeSpectra (GtkWidget *widget, 
				  guint *callback_action)
{
  GetDialog(10);
}

static void MenuSubtractSpectra (GtkWidget *widget, guint *callback_action)
{
  GetDialog(8);
}

static void MenuTitleSpectra (GtkWidget *widget, guint *callback_action)
{
  CreateTitleWindow();
  //  GetDialog(13);
}

static void MenuCompressSpectra (GtkWidget *widget, guint *callback_action)
{
  GetDialog(11);
}

static void MenuMoveSpectra (GtkWidget *widget, guint *callback_action)
{
  GetDialog(9);
}

static void MenuAddSpectra (GtkWidget *widget, guint *callback_action)
{
  GetDialog(7);
}

static void MenuGetSum (GtkWidget *widget, guint *callback_action)
{
  GetSum();
}

static void MenuManualSetBackground (GtkWidget *widget, 
				     guint *callback_action)
{
  GetDialog(18);
}

static void MenuOneChSetBackground (GtkWidget *widget, 
				    guint *callback_action)
{
  OneChSetBackground();
}

static void MenuManualExpandPlot (GtkWidget *widget, guint *callback_action)
{
  GetDialog(4);
}

static void MenuGetCalibration (GtkWidget *widget, guint *callback_action)
{
  GetDialog(17);
}

static void MenuZoomOut (GtkWidget *widget, guint *callback_action)
{
  ZoomOut();
}

static void MenuSetBackground (GtkWidget *widget, guint *callback_action)
{
  SetBackground(*callback_action);
}

static void MenuSetYScale (GtkWidget *widget, guint *callback_action)
{
  GetDialog(5);
}

static void MenuYScaleUp (GtkWidget *widget, guint *callback_action)
{
  YScaleUp();
}

static void MenuYScaleDown (GtkWidget *widget, guint *callback_action)
{
  YScaleDown();
}

static void MenuManualMarkSet (GtkWidget *widget, guint *callback_action)
{
  GetDialog(3);
}

static void MenuShiftRight (GtkWidget *widget, guint *callback_action)
{
  ShiftRight();
}

static void MenuShiftLeft (GtkWidget *widget, guint *callback_action)
{
  ShiftLeft();
}

static void MenuDisplayPlot (GtkWidget *widget, guint *callback_action)
{
  CreateDisplaySelectionWindow(0);
  //  GetDialog(1);
}

static void MenuDisplayPlotSameRange (GtkWidget *widget, guint *callback_action)
{
  CreateDisplaySelectionWindow(1);
  //GetDialog(6);
}

static void MenuExpandPlot (GtkWidget *widget, guint *callback_action)
{
  ExpandPlot();
}

static void MenuNextSpectra (GtkWidget *widget, guint *callback_action)
{
  NextSpectra();
}

static void MenuPrevSpectra (GtkWidget *widget, guint *callback_action)
{
  PrevSpectra();
}

static void MenuPrint (GtkWidget *widget, guint *callback_action)
{
  //  GetDialog(26);
  CreatePrintWindow(graph);
}

static void MenuPrintToFile (GtkWidget *widget, guint *callback_action)
{
  GetFilename("Postscript Output",1, WritePostscript);
}

/* --- Functions that do something --- */
/* WritePostscript
 *
 * does what it says
 */
void WritePostscript(char *sFilename)
{
  gtk_graph_make_ps(GTK_GRAPH(graph),sFilename);
}

/* SpawnPrintPrompt
 *
 * Prompts the user for which printer to print to
 */
void SpawnPrintPrompt(GtkWidget *text)
{
  char dummystr[80],dummystr2[80];
  char dummystr3[80];
  FILE *printcap;
  int test;

  sprintf(dummystr,"Print to which printer?\n");

  gtk_text_buffer_set_text(mytextbuffer(text),dummystr,strnlen(dummystr,80));

  if ((printcap = fopen("/etc/printcap","r")) != NULL) {
    while (fgets(dummystr3,80,printcap) != NULL) {
      if ((strncmp(dummystr3," ",1) != 0) || 
	  (strncmp(dummystr3,"#",1) != 0)) {
	sscanf(dummystr3,"%s",dummystr2);
	gtk_text_buffer_set_text(mytextbuffer(text),dummystr2,strnlen(dummystr2,80));
      }
    }
  fclose(printcap);
  }
}

/* SpawnPrintEntry
 *
 * uses import to generate a postscript
 * then, uses lpr to print
 * finally, delete the postscript
 */
void SpawnPrintEntry(GtkWidget *widget,GtkWidget *entry)
{
  int test;
  char dummystr[120];

  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)),
		"%s",dummystr);
  if (test == 1) {
    SpawnPrint(dummystr);
  }
}
 
/* SpawnPrint
 *
 * does the printing
 */
void SpawnPrint(char *cprinter)
{
  pid_t pid;
  char dummystr[120];
  

  sprintf(dummystr,"/tmp/gnuscope%d%d.ps",(int)time(NULL),getpid());

  gtk_graph_make_ps(GTK_GRAPH(graph),dummystr);


  if ((pid = fork()) < 0)
    g_error("fork error");
  else if (pid == 0) {
    if ((pid = execl("/usr/bin/lpr","lpr","-r","-P",cprinter,dummystr,(char *) 0)) == -1)
     g_error("execl error");
  }

  if (waitpid(pid, NULL, 0) < 0)
    g_error("wait error");
}

/* AbortFunction
 *
 * A callback function which causes a hung function to stop iterating
 * provided that such a function recognizes the abort flag
 */
void AbortFunction(GtkWidget *widget, gpointer data)
{
  abortflag = 1;
}

/*
 * CloseApp
 *
 * Close down GTK when the application window is closed
 */
static gint CloseApp(GtkWidget *widget, gpointer data)
{
  gtk_main_quit();
  return(TRUE);
}

/*
 * MenuReadFile
 *
 * Read a file when prompted to by the menu
 */
static void MenuReadFile (GtkWidget *widget, guint *callback_action)
{
  int i,j;
  
  j = 0;
  for (i = 0; i < 1028; i++) {
    if (histsize[i] !=0) j++;
  }
  
  if (j == 0) {
    GetFilename("Read Files",0,AutoDetectFileType);
    //    GetFilename("Read", ReadFile);
  } else {
    GetDialog(2);
  }
}

static void MenuReadBig (GtkWidget *widget, guint *callback_action)
{
  if (pgammax)
    GetDialog(27);
  else
    GetFilename("Read Big", 0, ReadBig);
}

/*
 * MenuWriteFile
 *
 * Write a file when prompted by the menu
 */
static void MenuWriteFile (GtkWidget *widget, guint *callback_action)
{
  GetDialog(24);
}

/*
 * MenuReadGS
 *
 * Reads in a gs92 .sqr or .twd
 */
static void MenuReadGS (GtkWidget *widget, guint *callback_action)
{
  GetFilename("Read GS92",0, ReadGS);
}

/*
 * MenuWriteGS
 *
 * Writes a gs92 .sqr or .twd
 */
static void MenuWriteGS (GtkWidget *widget, guint *callback_action)
{
  GetFilename("Write GS92",1, WriteGS);
}

/*
 * MenuLiphaFile
 *
 * Prompts for a file to dump the screen data to.
 */
static void MenuLiphaFile (GtkWidget *widget, guint *callback_action)
{
  GetFilename("Lipha",1, LiphaFile);
}


/*
 * SetPlotType
 *
 * Set the data type when selected from the menu
 */
static void SetPlotType (GtkWidget *widget, guint *callback_action)
{
  //*--ddc aug11 Heavily modified to take out dependence on gtkitemfactory.

  switch(*callback_action){
  default:
  case 1:
    plottype = 1;
    WriteMainText("Plot type set to linear.\n");
    break;
  case 2:
    plottype = 2;
    WriteMainText("Plot type set to log.\n");
    break;
  case 3:
    plottype = 3;
    WriteMainText("Plot type set to root.\n");
    break;
  case 4:
    plottype = 4;
    WriteMainText("Plot type set to efficiency corrected.\n");
  }
   
  DisplayCurrentRange();
}

/*
 * QuitApp
 *
 * Close from menu
 */
static void QuitApp(GtkWidget *widget, guint *callback_action)
{
  gtk_main_quit();
}

/*
 * CloseAppWindow
 *
 * Window is closing down, need to shut down GTK+.
 */
gint CloseAppWindow (GtkWidget *widget, gpointer *data)
{
  gtk_main_quit();
  return (FALSE);
}

/* ShowParams
 *
 * Show the command line options avalible for running the program
 */
void ShowParams()
{
  printf("To select from the command line please follow the following format.\n");
  printf("gnuscope <filetype> <filename> <spectra>\n");
  printf("<filetype> may be:\n");
  printf("1 - Binary spk file\n");
  printf("2 - Binary nsm file\n");
  printf("3 - ASCii spk file\n");
  printf("4 - ASCii NSM file\n");
  printf("5 - ASCii general file\n\n");
  exit(0);
}

/*
 * KeyPress
 *
 * Interprets a key press
 */
//--ddc opinion... these keypress functions are for the 1d displays and
//--ddc should be in the display.c file!
//--ddc gtk2 revs, there NEVER was a return value for this..
static gint KeyPress(GtkWidget *widget,GdkEventKey *event)
{
  //--ddc gtk2 deprecation  gtk_adjustment_set_value(GTK_ADJUSTMENT(GTK_TEXT(maintext)->vadj),0);
  gtk_adjustment_set_value(GTK_ADJUSTMENT(GTK_TEXT_VIEW(maintext)->vadjustment),0);
  switch (event->keyval) {
  case GDK_Right:
    g_signal_stop_emission_by_name(GTK_OBJECT(widget),"key_press_event");
    ShiftRight();
    break;
  case GDK_Left:
    g_signal_stop_emission_by_name(GTK_OBJECT(widget),"key_press_event");
    ShiftLeft();
    break;
  case GDK_KP_6:
    ShiftRight();
    break;
  case GDK_KP_Right:
    g_signal_stop_emission_by_name(GTK_OBJECT(widget),"key_press_event");
    ShiftRight();
    break;
  case GDK_KP_4:
    ShiftLeft();
    break;
  case GDK_KP_Left:
    g_signal_stop_emission_by_name(GTK_OBJECT(widget),"key_press_event");
    ShiftLeft();
    break;
  case GDK_KP_Up:
    g_signal_stop_emission_by_name(GTK_OBJECT(widget),"key_press_event");
    NextSpectra();
    break;
  case GDK_Up:
    g_signal_stop_emission_by_name(GTK_OBJECT(widget),"key_press_event");
    NextSpectra();
    break;
  case GDK_KP_8:
    NextSpectra();
    break;
  case GDK_KP_Down:
    g_signal_stop_emission_by_name(GTK_OBJECT(widget),"key_press_event");
    PrevSpectra();
    break;
  case GDK_Down:
    g_signal_stop_emission_by_name(GTK_OBJECT(widget),"key_press_event");
    PrevSpectra();
    break;
  case GDK_KP_2:
    PrevSpectra();
    break;
  case GDK_KP_7:
    YScaleDown();
    break;
  case GDK_KP_Home:
    g_signal_stop_emission_by_name(GTK_OBJECT(widget),"key_press_event");
    YScaleDown();
    break;
  case GDK_KP_1:
    YScaleUp();
    break;
  case GDK_KP_End:
    g_signal_stop_emission_by_name(GTK_OBJECT(widget),"key_press_event");
    YScaleUp();
    break;
  case GDK_KP_9:
    ActiveUp();
    break;
  case GDK_KP_Page_Up:
    g_signal_stop_emission_by_name(GTK_OBJECT(widget),"key_press_event");
    ActiveUp();
    break;
  case GDK_Page_Up:
    g_signal_stop_emission_by_name(GTK_OBJECT(widget),"key_press_event");
    ActiveUp();
    break;
  case GDK_KP_3:
    ActiveDown();
    break;
  case GDK_KP_Page_Down:
    g_signal_stop_emission_by_name(GTK_OBJECT(widget),"key_press_event");
    ActiveDown();
    break;
  case GDK_Page_Down:
    g_signal_stop_emission_by_name(GTK_OBJECT(widget),"key_press_event");
    ActiveDown();
    break;
  case GDK_Home:
    g_signal_stop_emission_by_name(GTK_OBJECT(widget),"key_press_event");
    if (binsizeforce >= 0) binsizeforce++;
    DisplayCurrentRange();
    break;
  case GDK_End:
    g_signal_stop_emission_by_name(GTK_OBJECT(widget),"key_press_event");
    if (binsizeforce > 0) binsizeforce--;
    DisplayCurrentRange();
    break;
  case GDK_0:
    if (histsize[9] >0) {
      spectra = 9;
      spectradisplayed[0][0] = 9;
      DisplayCurrentRange();
    }
    break;
  case GDK_1:
    if (histsize[0] >0) {
      spectra = 0;
      spectradisplayed[0][0] = 0;
      DisplayCurrentRange();
    }
    break;
  case GDK_2:
    if (histsize[1] >0) {
      spectra = 1;
      spectradisplayed[0][0] = 1;
      DisplayCurrentRange();
    }
    break;
  case GDK_3:
    if (histsize[2] >0) {
      spectra = 2;
      spectradisplayed[0][0] = 2;
      DisplayCurrentRange();
    }
    break;
  case GDK_4:
    if (histsize[3] >0) {
      spectra = 3;
      spectradisplayed[0][0] = 3;
      DisplayCurrentRange();
    }
    break;
  case GDK_5:
    if (histsize[4] >0) {
      spectra = 4;
      spectradisplayed[0][0] = 4;
      DisplayCurrentRange();
    }
    break;
  case GDK_6:
    if (histsize[5] >0) {
      spectra = 5;
      spectradisplayed[0][0] = 5;
      DisplayCurrentRange();
    }
    break;
  case GDK_7:
    if (histsize[6] >0) {
      spectra = 6;
      spectradisplayed[0][0] = 6;
      DisplayCurrentRange();
    }
    break;
  case GDK_8:
    if (histsize[7] >0) {
      spectra = 7;
      spectradisplayed[0][0] = 7;
      DisplayCurrentRange();
    }
    break;
  case GDK_9:
    if (histsize[8] >0) {
      spectra = 8;
      spectradisplayed[0][0] = 8;
      DisplayCurrentRange();
    }
    break;
  case GDK_KP_0:
  case GDK_KP_Insert:
    CursorPosQuer(GTK_WIDGET(widget));
    break;
  }

  //--ddc aug11 must return 0 for any other event callbacks to run
  //  return 1;
  return 0;
}

void CursorPosQuer()
{
  char dummystr[80];
  int i;
  float currentenergy;
  int tempsum;
  int counts;
  int correctedcounts;

  markers[3] = markers[2];
  markers[2] = markers[1];
  markers[1] = markers[0];
  markers[0] = cursorx;
  /* --- if there is a calibration, let's use it --- */
  WriteMainText("\n");
  currentenergy = UseCalibration(markers[0]);
  if (currentenergy != -1) {
    sprintf(dummystr, " Energy: %.1f   ",currentenergy);
    WriteMainText(dummystr);
  }

  /* --- why don't we return the number of counts too --- */
  /* Unfortunately, this is also affected by the binsize but that
     isn't a real problem. */
  tempsum = 0;
  if (histsize[spectra] >0) {
    for (i = 0; i < binsize ; i++) {
      if ((i + markers[0]) < histsize[spectra]);
      tempsum = tempsum + *(histloc[spectra]+i+markers[0]);
    }
  }
  
  counts = tempsum;
  

  correctedcounts = UseEfficiency(counts,currentenergy);
  if (correctedcounts !=-1) {
    sprintf(dummystr, " Efficiency corrected counts: %.1f   \n",correctedcounts);
    WriteMainText(dummystr);
  }

  sprintf(dummystr,"  Counts: %d  Channel: %d  ",counts,(markers[0]+1));
  WriteMainText(dummystr);


  DrawMarkers();
}

/*
 * CreateMainWindow
 *
 * Creates the main window
 */
void CreateMainWindow()
{
  /* --- GTK variables --- */

  //--ddc dec10 unused  GtkWidget *box2;
  GtkWidget *localimage;
  //--ddc jan11 unused  GtkWidget *separator;
  GtkWidget *label;
  GtkWidget *button, *button2;
  GtkAccelGroup *accel_group;
  //--ddc aug11  GtkItemFactory *item_factory;
  GtkActionGroup *action_group;
  GtkUIManager *menu_manager;
  GtkWidget *localmenu;

  int i,j,k;
  GdkBitmap *bitmap;
  GdkBitmap *mask;
  GdkColor *color;
  int red,green,blue;
  //  GtkWidget *hscrollbar,*vscrollbar,*table;
  GtkWidget *table;
  GtkWidget *localwindow;
  int cols;

  /* --- Create the top-level window --- */
  mainwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  //--ddc jun11 key_press_event handler HAD to be connected using
  //--ddc signal_connect_after (AFTER).

  g_signal_connect(GTK_OBJECT (mainwindow), "key_press_event",
		   G_CALLBACK(KeyPress),NULL);

  //gtk2 deprecated itemfactory... here is the updated way to make menus.
  {
    GError *error=NULL;
    action_group = gtk_action_group_new ("MainActions");
    gtk_action_group_set_translation_domain (action_group, "blah");


    for(i=0;i<nmenu_items;i++)  {
      gtk_action_group_add_actions (action_group, &menu_items[i].gtkentry , 1, 
				    &menu_items[i].userdata);
    }

    menu_manager = gtk_ui_manager_new ();
    gtk_ui_manager_insert_action_group (menu_manager, action_group, 0);
    gtk_ui_manager_add_ui_from_string (menu_manager,menu_ui,-1,&error);
    
    if (error)
    {
        g_message ("building menus failed: %s", error->message);
        g_error_free (error);
    }

    accel_group=gtk_ui_manager_get_accel_group(menu_manager);
    localmenu=gtk_ui_manager_get_widget(menu_manager,"/MainMenu");

  }


  gtk_window_add_accel_group(GTK_WINDOW (mainwindow), accel_group);
 
  gtk_window_set_title(GTK_WINDOW (mainwindow), "Gnuscope");

  /* --- You should remember to connect the delete_vent to the main window --- */
  //--ddc aug11  gtk_signal_connect (GTK_OBJECT (mainwindow), "delete_event",
  g_signal_connect (GTK_OBJECT (mainwindow), "delete_event",
		    G_CALLBACK (CloseAppWindow), NULL);
  //--ddc aug11 gtk_signal_connect (GTK_OBJECT (mainwindow), "destroy",
  g_signal_connect (GTK_OBJECT (mainwindow), "destroy",
  		      G_CALLBACK (gtk_widget_destroyed), &mainwindow);

 
  /* --- Give the window border --- */

  //--ddc aug11 deprecation
  //  gtk_container_border_width (GTK_CONTAINER (mainwindow), 1);
  gtk_container_set_border_width (GTK_CONTAINER (mainwindow), 1);


  /* --- Create the vbox (what the graph(s) go in --- */
  
  box1 = gtk_vbox_new (FALSE,0);
  //--ddc dec10 gtk2 changes..
  gtk_widget_set_redraw_on_allocate(box1,TRUE);

  gtk_container_add(GTK_CONTAINER (mainwindow), box1);

  gtk_widget_show(localmenu);

  gtk_box_pack_start(GTK_BOX (box1),localmenu, FALSE, FALSE, 0);

  
  /* --- Connect the events for the vbox --- */

  /* --- make our text box --- */

  table = gtk_table_new(6,6,FALSE);
  //--ddc dec10 gtk2 changes..
  gtk_widget_set_redraw_on_allocate(table,TRUE);

  gtk_box_pack_start(GTK_CONTAINER(box1),table,FALSE,FALSE,0);
  
  /* --- insert the channel info label --- */

  channelinfo = gtk_label_new("Channel: 0 Counts: 0");

  gtk_table_attach(GTK_TABLE(table),channelinfo,0,1,0,1,
		   GTK_FILL,GTK_SHRINK,0,0);

  /* --- insert the logo in the upper right-hand part of the window --- */

  button = gtk_button_new();

  localimage = gtk_image_new_from_pixmap(gdk_pixmap_colormap_create_from_xpm_d(NULL,gdk_rgb_get_colormap(),NULL,NULL,logo_xpm),NULL);

  gtk_container_add(GTK_CONTAINER(button),localimage);
  //--ddc aug11  gtk_signal_connect (GTK_OBJECT (button), "clicked",
  g_signal_connect (GTK_OBJECT (button), "clicked",
		    G_CALLBACK(gtk_widget_destroy),NULL);
  gtk_widget_show(localimage);

  gtk_table_attach(GTK_TABLE(table),button,2,3,0,3,
  		   GTK_SHRINK | GTK_FILL,
		   GTK_SHRINK | GTK_FILL, 0, 0);


  maintext = init_text_view();

  //--ddc gtk2
  {
    int x,y;
    localwindow=gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(localwindow),
					maintext);
    pango_layout_get_pixel_size(gtk_label_get_layout(channelinfo),&x,&y);
    gtk_widget_set_size_request(localwindow,-1,10*y);
  }
  //--ddc debug
  g_signal_connect(GTK_OBJECT(localwindow),"key_press_event",
		   G_CALLBACK(TextViewKeyCallback),NULL);


  WriteMainText("Welcome to Gnuscope\n");

  gtk_table_attach(GTK_TABLE(table),GTK_SCROLLED_WINDOW(localwindow),0,2,1,3,
		   GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                   GTK_EXPAND | GTK_SHRINK | GTK_FILL,0,0);

  /*
   * --- Create a Graph
   */

  displaytable = gtk_table_new(6,6,FALSE);

  /* --- Create a new graph --- */ 

  numspectra = 16;
  for (i = 0; i < 16; i++) {
    graphsdisplayed[i] = gtk_graph_new();

    gtk_graph_set_num_lines(GTK_GRAPH(graphsdisplayed[i]),1);
  

    /* --- make some dummy entries in the graph --- */
    gtk_graph_size(GTK_GRAPH(graphsdisplayed[i]),100);
    for (j = 0; j < 100; j++) {
      gtk_graph_set_value(GTK_GRAPH(graphsdisplayed[i]),0,j,j+1);
    }
    gtk_widget_set_events (graphsdisplayed[i], GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_KEY_PRESS_MASK );
    //--ddc jun11 gtk2 upgrading, (seperate keypress from accel group?)

    //--ddc aug11 gtk_signal_connect (GTK_OBJECT (graphsdisplayed[i]), "button_press_event",			(GtkSignalFunc) SetMarker, NULL);
    g_signal_connect (GTK_OBJECT (graphsdisplayed[i]), "button_press_event",			G_CALLBACK(SetMarker), NULL);
    //--ddc aug11    gtk_signal_connect (GTK_OBJECT (graphsdisplayed[i]), "motion_notify_event",
    g_signal_connect (GTK_OBJECT (graphsdisplayed[i]), "motion_notify_event",
		      G_CALLBACK(UpdateCursorX),NULL);

    gtk_graph_title(GTK_GRAPH(graphsdisplayed[i]),0,"title");

    /* --- Show the graph --- */
    //--ddc apr11 oops bug in cols, goes out of range..
    //    cols = (i - (i %4));
    cols = (i - (i %4))/4;
    gtk_table_attach(GTK_TABLE(displaytable),graphsdisplayed[i],
		     cols,cols+1,(i %4), (i%4) +1,
		      GTK_EXPAND | GTK_SHRINK | GTK_FILL,
		      GTK_EXPAND | GTK_SHRINK | GTK_FILL,
		     0,0);

    gtk_widget_show(graphsdisplayed[i]);//--ddc

  }  
  
  //--ddc apr11 dbg (and general advice for adding container) gtk_container_add(GTK_CONTAINER(box1),displaytable);
  gtk_box_pack_start(GTK_CONTAINER(box1),displaytable,TRUE,TRUE,0);

  /* --- make our text box --- */


  /*
   * --- Make the main window visible
   */

  gtk_window_set_resizable(mainwindow, TRUE); //?
  //  gtk_window_resize((GtkWindow*)mainwindow, 1, 1); //? necessary?

  for(i=1;i<16;i++) gtk_widget_hide(graphsdisplayed[i]);

  numspectra=1;
  graph = graphsdisplayed[0];
  gtk_widget_show(displaytable);


  gtk_widget_show(channelinfo);
  gtk_widget_show(maintext);
  gtk_widget_show(localwindow);
  gtk_widget_show(button);
  //--ddc apr11  button2 seems to be noop.  gtk_widget_show(button2);
  gtk_widget_show(table);

  gtk_widget_show(box1);

  //--ddc nov15 move the show to AFTER the size is set...
  //--ddc show the top window
  //  gtk_widget_show(mainwindow);

  //--ddc nov15 This goes across monitors.. find dimensions of the 
  // first monitor in default screen
  //  gtk_window_set_default_size((GtkWindow*)mainwindow,-1,-1);
  GdkRectangle rect;
  gdk_screen_get_monitor_geometry((GdkScreen*)gdk_screen_get_default(),0, &rect);
  gtk_window_set_default_size((GtkWindow*)mainwindow,rect.width,rect.height);

  //--ddc jan15
  //hmm.  let user resize.
  GdkGeometry hints;
  //--ddc nov15  int wwww, hhhh;
  //--ddc nov15  gtk_window_get_size((GtkWindow*)mainwindow,&wwww,&hhhh);
  hints.min_width = rect.width*0.5;
  hints.min_height = rect.height*0.5;
  hints.max_width = rect.width;
  hints.max_height = rect.height;
  hints.base_width = rect.width;
  hints.base_height = rect.height;
  hints.width_inc = 1;
  hints.height_inc = 1;
  hints.min_aspect = 0.1;
  hints.max_aspect = 10.0;
  hints.win_gravity = GDK_GRAVITY_NORTH_EAST;
     
  gtk_window_set_geometry_hints((GtkWidget*)mainwindow, 
				(GtkWidget*)mainwindow,&hints,
				(GDK_HINT_RESIZE_INC |
				 GDK_HINT_MIN_SIZE|GDK_HINT_BASE_SIZE));

  gtk_widget_show(mainwindow);

  /* --- set the cursor to crosshairs --- */

  cursor = gdk_cursor_new(GDK_CROSSHAIR);
  gdk_window_set_cursor(box1->window, cursor);


  
  WriteMainText("\nJanuary 1, 2003\n  Major changes:  PGAM Sort now supports .ev2 data file types.  To use this add a line containing ev2 or EV2 to then end of your master setup file.\nDecember 30, 2002\n  Major Changes:  1.  Added support for fairly primative autodetection of file types.  It works by looking at the file extention.  If you have not been good about your file extentions this could cause you problems.  (eg if you have been using .nsm instead of .NSM).  If this is the case you should use the \"mv\" command in linux to correct, or use the read file option in the DEGRADED menu.  The autodection of file types also allows the READ function to be used for .msf, .sf, .ede, .big, .twd, .sqr, and .cmd files.\n2:  Command file and command line support has been added.  Unfortunately there is no guide to writing command files yet, and command line options are basically just for reading a file at start up(which would allow you to do just about everything in conjunction with a command file).\n3:  Gnuscope can now display more than 1 Density plot.  Unfortunately, screen resolution has an effect on the maxium which can be easily displayed.  For most computers 8 is the limit, but the software supports up to 16.  \n4:  Bugs with displaying the gaussians have been fixed.  \nNovember 22, 2002\n  Major Changes:  1.  Added Spec-U-Lator (press F12), a graphical interface for the add, subtract, normalize, move and re-title options of the Manipulate menu.  Try it, You'll like it.\n2:  Added a Veto and Required options to the PGAM sort window (in the gate section).  These take files which are lists of <ADC#>,<Min>,<max><CRT>.\nSeptember 24, 2002\n  Major Changes:  Added a \"pairs\" sort option for pgamsort function.  To use this function, select the appropriate option and enter the name of a file with pairs information in the entry widget to the right.  The format of the file is listing the pairs in the form (a,b) where a and b are ADC numbers.\nSeptember 18, 2002\n  Major Changes:  Histogram display has been modified to allow for display of negative numbers.  Analysis functions have been appropriately modified, and an oversubtract when projecting from a pgammatrix will result in negative counts (as will the MANIPULATE-SUBTRACT command).\nSeptember 17, 2002\n  Major Changes:  Fixed a bug in the pgamsort routines which was resulting in a compression dependant shift energy results for sum and double gauss fit.  Also fixed the energy return for gauss fit.\n  Minor Changes:  Clover identification during sorting was changed so that a crystal can now belong to more than 1 clover.\nSeptember 9, 2002\n  Major Changes:  Re-fresh method for 2D plots has been changed so the user can see something happening (should be faster too).  AUTOCAL and PEAKFIT functions have been added under the analysis menu.  To use the AUTOCAL function you must first read in a file containing information using the READ CALIB INFO option under the Analysis menu.  Peak fit also requires a FITNESS THRESHOLD which is related to the signal to noise ration for the peaks.  A recomended starting value is 30.  The interface should improve in further updates.  \nAugust 19, 2002\n Major Changes:\n the .ede format has been changed to support titles, the rest of the program will be changed soon to accomodate titles as well.  If you need a conversion utility for your old .ede files please let me know by sending e-mail to pavan@nucmar.physics.fsu.edu.  Also, post-script printing support has been added for both the 1D and 2D histograms.\n\nAugust 9, 2002\n Major Changes:\n   Contours have been added to E-dE plots.  The three plot modes, listed under Display in the EdE menu, are Contours only, Densities only, and Contours and Densities.\n   The creation and display of banana gates has also been improved, and will continue to be.\n\nAugust 6,2002\nMajor Changes:\n   Multiple spectra can now be displayed in 1 histogram.  Use <Ctrl>+D to display more than 1 histogram in the last spectrum clicked.\n   Support for reading and writing multiple 2D plots has been added.  Prompting should be straight forward. \n   Added support for big files in binary format, set as the default.  The ASCII format is used if the file read in or written out uses the .big file extention.  The suggested file extention for the binary format is .ede\n\n July 19,2002\nMajor Changes: fixed a bug in E-dE gates\nMinor Change: Improved memory managment for \"big\" files.\n Previous changes: July,17,2002\n  Major Changes: Fixed a bug in E-dE 2D plot sorting.  Better separation of the contors is now possible.\n");

  WriteMainText("\nJanuary 27, 2003:\n  Major Changes:  Fixed problems with gamma and particle energy gates inside of a PGAM sort process.  If there are problems please let me know.\n  Minor Changes:  Due to some optimizations sorting (especially energy histogram) should go faster.");
  WriteMainText("\nFebuary 4, 2003:\n(2)\n  Minor Changes: Fixed problem with Gainshift function.\n(1)\n  Minor Changes:  Fixed bugs in overlaying Histograms and some bugs in the 2D display which could cause the program to crash.");  
  WriteMainText("\nFebuary 12, 2003:\n  Major Changes:  support for wildcard characters in the read file from command line option.  To use try \"gnuscope -f *.NSM\".\n  Minor Changes:  Fixed two bugs with double gauss fit.");
  WriteMainText("\nFebuary 17, 2003:\n  Major Changes:  Fixed problems with the construction of the Spec-U-Lator window.");
  WriteMainText("Febuary 18, 2003:\n  Major Changes:  Fixed an error in the manual set marker window.  Fixed display problems for gauss, double gauss, exponential fit, and peakfit displays for semilog and semiroot display modes.");
  WriteMainText("Febuary 25, 2003\n  Major Changes:  Added addback to multi-segment particle telescopes.  This resulted in a major overhall of how particle telescopes were sorted.  This is important to you because there may be problems.  If you notice any please let me know.\n  There was a bug in the 1D Histogram Energy sorting for multiple telescopes.  This has been fixed.\n");
  WriteMainText("March 4, 2003\n  Major Changes:  Added support for non-particle detector 2D plots.  These feature can be used by putting an * in the like defining the particle telescope in the setup file.  2D plots designated in such a way will generate 2D histograms, and can be used with 2D gates, but will not be included in any of the \"sum\" outputs (most matrices).  The ADCs used will also not be excluded by any of the \"sum\" outputs.\n  Minor Change:  Added support for up to 64 2D matricies during sorting.\n");
  WriteMainText("April 11, 2003\n  Major Changes:  For 2D plots.  Added support for both negative numbers, and interpolation (quadratic).  Quadratic interpolation is turned on by default.  It makes the plots look better.  However, interpolation is slower than no interpolation, especially with contour plots.  Therefore, if you can tolerate the difference in quality you might want to consider no interpolation.\n  Minor Changes:  Changed the way the graphics for the density plot is drawn.  This should now be faster over the network.\n");
  WriteMainText("May 25, 2003\n  Major Changes:  Well, there are really quite a few.  These are the ones that I remember.  First, there have been quite a few bugs worked out of sorting from the ev2 data structures.  There are still occasional screw ups, but Gnuscope now handles these by dropping the rest of the 8192 byte buffer and starting fresh with the next one.  It does tell you when it does this, and which byte it was on, so you can have some idea how much data was lost.  You should be loosing well less than 1% of your data.  Second, the setup files now have the option of including a maximum reasonable channel in the setup file.  The purpose of this is to allow the user to throw out overflow events.  This option would change the second line of the ADC definition of the setup file from \"ADC,CHANNELS,ANGLE\" to \"ADC,CHANNELS(MAX_CHANNEL),ANGLE\".  Third, the color contrast on the two dimensional plots has been changed to allow you see low-count channels more easily.  Fourth, Gnuscope now supports configuration files.  These should be named \".gnuscopeconfig\" and can be placed in either the directory from which Gnuscope is being read, or in your home directory.  It will look to the current directory first.  Refer to \"An Almost Certainly Incomplete Guide to Gnuscope\" for help, or run Gnuscope with the --help flag from the command line.  Fifth, the Gnuscope Guide, \"An Almost Certainly Incomplete Guide to Gnuscope\" is avalible either from John Pavan (Me) or online at \"http://nucalf.physics.fsu.edu/~pavan\".  Supposedly, it is in PDF format, but acroread can read it, and xpdf cannot.  Finally,  Hmmm...I've fogotten whatelse I've done.  Oh.  There is tentative (very early) Radware matrix support.  Gnuscope seems to be able to write out 4Kx4K radware square matricies (from PGAM square matricies) and can read in radware matricies from the command line.  Eventually, this support will be better.\n");
  WriteMainText("June 8th, 2003\nMajor Changes:  To make it easier on everyone, compress2 has been incorporated into Gnuscope.  There are two ways to use this feature.  One way is to use it under the \"File\" pull-down menu.  The other is to use the command line option, \"gnuscope --compress file.mul --stop\".  Second, there have been some bugs fixed with printing and with how Gnuscope finds printers.  Third, the 2D display mode now has color scale choices.  Currently the default is linear, but Log or Root are better for E-DeltaE plots.\n");
  WriteMainText("June 11th, 2003\nMinor Change:  Fixed a bug in marker based subtract gates.\n");
  WriteMainText("June 23rd, 2003\nMajor Changes:  Radware matricies can now be written using the standard \"Write Histogram\" function.  Several updates to the gtktwodplot.c and gtktwodplot.h which are hopefully not visible to the end user.Think that is about it.\n");
  WriteMainText("June 25th, 2003\nMajor Change:  Added \"--cnf2txt\" option to the command line.  In conjunction with the \"--stop\" command line option it can be used to make a batch conversion of .cnf files to generic ASCII files with the extention of .txt.\n");
  WriteMainText("July 28th, 2003\nMinor Fix:  Gnuscope should no longer be capable of producing a huge number of error message windows.\n");
  WriteMainText("July 29th, 2003\nMajor Change:  Fixed bug with triples mode for gs92 sorting.\n");
  WriteMainText("November 4th, 2003\nBug Fix:  Fixed compression bug in pgamsort sorting.  Prior sorts should have calibration \"-(n-1) n 0\" where n is the compression factor.  New sorts should be able to use \"0 n 0\".\n");
  WriteMainText("Mar, 2005  Bug Fix: A few: misc. pointer errors, double counting in square and triangle sorting, 'telescopes_hit' initialization.  \n");
  WriteMainText("May, 2005  Bug Fix: Accumulation of roundoff for projected spectra reduced \n");
  WriteMainText("May, 2005  Bug Fix: Detector map underflow references for unused detectors fixed \n");
  WriteMainText("May, 2005  Bug Fix: Addback index error fixed \n");
  WriteMainText("June, 2005  Bug Fix: Detector_info map index underflow when virtual adc numbers match unused physical adc numbers fixed \n");
  WriteMainText("June, 2005  Bug Fix: 'one off' index error in allocating memory for detectors in list for square sort AND in singles sort detector maps \n");
  WriteMainText("June, 2005  Bug Fix: gtk indexes out of range in drawing histograms in gtk_graph_draw \n");
  WriteMainText("June, 2005  Bug Fix: Infinite loops in GaussFit and PgamSort broken (input buffer processing). \n");
  WriteMainText("January, 2006 Bug Fix: Error in writing 'big' files.\n"); 
  WriteMainText("January, 2006 Bug Fix: Multiple color bugs in twod plots, PLUS implemented contrast feature.\n"); 
  WriteMainText("January, 2006 Bug Fix: Bug fix in interpolation routine leading to core dump.\n"); 
  WriteMainText("January, 2006 Bug Fix: Fixed crash for clearing undefined gates, drawing undefined spectra.\n");
  WriteMainText("January, 2006 Implementation Change: Gate points are only included AFTER selecting gating.\n");
  WriteMainText("January, 2006 Implementation Change: Default twod plot mode changed to density only.\n");  
  WriteMainText("January, 2006 Bug Fixes: Writing twod spectra to ede files.\n");
  WriteMainText("January, 2006 Bug Fix: Crashing resulting from bad twod spectra.\n");  
  WriteMainText("May, 2006 Bug Fix: Index test failure in twod histogram sum and difference functions.\n");  
  WriteMainText("May, 2006 Bug Fix: Nonnegative values only in difference.\n");
  WriteMainText("May, 2006 Bug Fix: Attempt to reuse gtk objects destroyed by user (by kp), and others...\n");
  WriteMainText("June, 2006 Bug Fix: Various uninitialized variables in gaussian fits.\n");
  WriteMainText("June, 2006 Bug Fix: Memory issues for filenames.\n");
  WriteMainText("June, 2006 Bug Fix: Serious memory allocation issue for twod histograms created by add/sub function.\n");
  WriteMainText("June, 2006 Bug Fix: Array boundary issue for gainshift function.\n");
  WriteMainText("June, 2006 Bug Fix: Multiple uninitialized variables for gate tests for 'Energy Output' in pgamsort.\n");
  WriteMainText("June, 2006 Logic error: gated gammarays not properly accounted for when placing gammas in the matrix.\n");
  WriteMainText("August, 2006 Logic error: Fixed failing TAC test when sorting particles.\n");
  WriteMainText("May, 2007 Bug Fixes: Potentially short reads for evt files, and undefined gates for raw singles sorting.\n");
  WriteMainText("July, 2007 Bug Fixes: No background subtraction from triangle projection, and error for finding last histogram id.\n");
  WriteMainText("June, 2008 Bug Fixes: Error applying quadratic term for spectrum calibrations, and cleanup of function prototypes to fix error using commandfiles to enter calibrations\n");
  WriteMainText("April, 2009 Bug Fixes: Dimensionality of radware MAT file fixed (spe files changed to match and made _binary_).  Redundant windows for twod histograms fixed.\n");
  WriteMainText("April, 2009 Feature: Matrix _subtraction_ added.\n");
  WriteMainText("June, 2009 Bug Fix: 2006 Fix for gainshift out-of-bounds, fixed AND roundoff errors.\n");
  WriteMainText("June, 2012 Fix for using 32k channels, and bugs in binary SPE file reads.\n");
  WriteMainText("July, 2012 Fix for overflows in implicit type conversion for NSM files with 32k channels.\n");
WriteMainText("January, 2013 Fix for formula for area of Gauss Fits, from Pei-Laun Tai.\n");
  WriteMainText("August, 2013 Fix for array bounds errors in pgamsort, rawsorting.\n");
  WriteMainText("June, 2014 Misc fixes for text displays, and spectra list\n");
  WriteMainText("November, 2014 Fixes from 'pair' testing with V.Tripathi, including 'rare' split buffer error.\n");
  WriteMainText("Feb, 2015 S.Tabor's grouped doppler correction, plus resizing window, and errors on menu callbacks.\n");
  WriteMainText("May, 2015 Mertin/Dungan spec-u-lator error (for last spectrum).\n");
  WriteMainText("Jul, 2016 V.Tripathi change to reject event for any gamma missing time gate.\n");
  WriteMainText("Jan, 2017 Fix typo in memory allocation pgamsort, and trap histogramming rawsort ADCs with zero values\n");
  WriteMainText("March, 2017 Fix temporary array dimension in analysis.c\n");
  WriteMainText("May, 2017 Fix array bounds problems in init global variables\n");  WriteMainText("June, 2017 E.Rubino fix more overflows in pgammatrix.c for projections\n");
  //  WriteMainText("At: http://fsunuc/~caussyn \n\n");
  //  WriteMainText("\nGnuscope's last major stable rebuild: Jan 12, 2017 \n");
  WriteMainText("\nGnuscope's last major stable rebuild: June 28, 2017 \n");
  WriteMainText("\n supported gtk2 version!!!! report problems to maintainer.\n\n");

}

/*
 * Help
 *
 * modified to call xpdf for the gnuscopemanual.pdf file
 *
 * Display the help file in a text window
 */
void Help() 
{
  /* --- Jan 5, 2004 --- */
  /* --- modified to call xpdf to read the file pointed to by manualpath --- */

  pid_t pid;
  char dummystr[120];

  //  printf("manualpath = %s\n",manualpath);

  if ((pid = fork()) < 0)
    g_error("fork error");
  else if (pid == 0) {
    if ((pid = execlp(pdfapp," ",manualpath,(char *) 0)) == -1)
      g_error("excel error");
  }

}

/*
 * InitializeGlobals --- initializes global variables
 */
void InitializeGlobals()
{
  int i,j;
  
  sprintf(pdfapp,"xpdf");
  sprintf(manualpath,"/home/pavan/gnuscope/docs/gnuscopemanual.pdf");
  ornlfile = 0;
    oldscope = 0;
  backgroundpolydeg = 0;
  numspectra = 1;
  fade_mode = 0;

  intype = 2;
  for (i = 0; i < 1028; i++) {
    histid[i] = histsize[0] = 0;
    histloc[i] = NULL;
    sprintf(field1[i],"");
    sprintf(field2[i],"");
    sprintf(field3[i],"");
  }
  histpointer = 0;
  intype = 2;
  plottype = 1;
  spectra = 0;
  graph = NULL;
  drawing_gates = 0;
  for (i = 0; i < 4; i++) {
    markers[i] = 0;
  }
  binsize = 1;
  displaytable = NULL;
  offset = 1;
  partfincal = 1;
  for (i = 0; i < 3; i++) {
    background[i] = 0;
  }
  oldscope = 1;
  numspectra = 1;
  currentrange[0] = 0;
  currentrange[1] = 0;
  graph = NULL;
  for (i = 0; i<16; i++) {
    for (j = 0; j < 10; j++) {
      spectradisplayed[i][j] = -1;
    }
    graphsdisplayed[i] = NULL;
  }
  box1 = NULL;
  mainwindow = NULL;
  maintext = NULL;
  num_hpge = 0;
  hpges = NULL;
  num_telescopes = 0;
  telescopes = NULL;
  num_clovers = 0;
  clovers = NULL;
  num_tacs = 0;
  tacs = NULL;
  telfincal = 1;
  num_setups = 0;
  datafiletype = 0;
  setupfiles = NULL;
  fin_chans = 0;       // the number of channels in the final spectra
  num_adcs = 0;        // number of adcs used in the experiment
  fin_cal = 0;       // final calibration (keV/chan)
  beta = 0;          // beta for doppler shift correction
  pgamsorttype = 0;    // kind of sorting to do
  sortrecords = 0;     // sort all records or not? 
  runmin = runmax = -1;   // first and last runs to sort
  sprintf(evtpath,"");    // path to .evt files
  big_data_info = NULL;
  gate_info = NULL;
  for (i = 0; i < 4; i++) {
    twodmarkers[i].x = twodmarkers[i].y = 0;
  }
  xcurrentrange[0] = xcurrentrange[1] = 0;
  ycurrentrange[0] = ycurrentrange[1] = 0;
  twodplot = NULL;
  pgammax = 0;
  ybinsize = 1;
  xbinsize = 1;
  min_clover_multipolarity = 0;
  twodvbox = NULL;
  twodtext = NULL;
  firstpoint.x = firstpoint.y = 0;
  messagetext = NULL;
  pgam = 0;
  pgammax = 0;
  pgammatrixdata.data = NULL;
  pgamgammagates.num = 0;
  pgamgammagates.min = NULL;
  pgamgammagates.max = NULL;
  pgammatrixdata.type = -1;
  //--ddc 23jul07  JP commented out the initialization of twoddata.filetype. 
  //I'm not sure why JP thought that he didn't need to init
  //the twodata.filetype, but he does!  I'm putting it back, but I'm 
  //initializing it to 'zero' so that that the default is for various functions
  //to work correctly with a matrix from the sorting program!  This will
  //actually still be broken, if the users switch back and forth between the
  //pgammatrix and twoddata!
  //woddata.filetype = -1;
  twoddata.filetype = 0;

  stoppingpowerinfo = NULL;
  lastspectraclicked = 0;
  lastgraphclicked = NULL;
  abortflag = 0;
  binsizeforce = 0;
  twodnumcontours = 10;
  edesize = 200;
  //--ddc may17 the range on twodtitles is
  //254.. this had to overwriting at least edesize
  //  for (i = 0; i < 1028; i++)
  for (i = 0; i < 254; i++)
    sprintf(twodtitles[i],"");
  peakfita[0] = 0;
  peakfita[1] = 100;
  peakfita[2] = 1;
  peakfitb[0] = 0;
  peakfitb[1] = 1;
  commandlineonly = 0;
  peakfitb[2] = 0.005;
  peakfitc[0] = 0;
  peakfitc[1] = 0.000001;
  online = 0;
  peakfitc[2] = 0.00000001;
  //--ddc may17 AND the range on display_toggle is only 254
  //  for (i = 0; i < 1028; i++) display_toggle[i] = 0;
  for (i = 0; i < 254; i++) display_toggle[i] = 0;
  projectionwindow = NULL;
  Window2D = NULL;
  num_2D_gates = 0;
  interpolationmode = 2;

  pgamcolormode = 0;
  pgamcontourtype = 1;
  pgamtype = 1;  //--ddc 19jan06 change from 2 (which was density+contour)
  //--ddc mar11 gtk2 deprecations... undocumented use of 'rgbi'.
  sprintf(colorstring[0],"#000");
  sprintf(colorstring[1],"#800");
  sprintf(colorstring[2],"#080");
  sprintf(colorstring[3],"#008");
  sprintf(colorstring[4],"#880");
  sprintf(colorstring[5],"#808");
  sprintf(colorstring[6],"#f00");
  sprintf(colorstring[7],"#0f0");
  sprintf(colorstring[8],"#00f");
  sprintf(colorstring[9],"#f0f");
  
 
  for (i = 0; i < 16; i++) {
    twoddisplayed[i] = NULL;
    twodspectradisplayed[i] = 0;
  }
    numtwoddisplayed = 1;
    manipulatetitlewindow = NULL;
    radwarematrixsize = 4096;
    colordepth = 256;
}




  
/*
 * main - program begins here
 */
int main (int argc, char *argv[])
{
  pid_t pid;

  /* --- GTK+ initialization --- */
  InitializeGlobals();
  gtk_init (&argc, &argv);
  //--ddc jan11 deprecation gtk_widget_push_visual(gdk_rgb_get_visual(  ));
  //--ddc dec10 deprecation  gtk_widget_push_colormap(gdk_rgb_get_cmap(  ));
  //gdk_rgb_init();

  //--ddc feb11 DEBUG LINE
  //  gdk_window_set_debug_updates (TRUE);
  

  /* --- we ought to allow some command line stuff --- */
  /* --- perhaps we ought to allow for a seperate function to handle this stuff --- */
  CommandLineHandler(argc, argv);
  if (!commandlineonly) {
    ConfigFileSearcher();
    
    currentrange[0] = 0;
    currentrange[1] = 1;
    logfile = fopen("logfile","w");
    CreateMainWindow();
    gtk_main();

    /* --- close the log file --- */
    if (logfile)
      fclose(logfile);
  }
  exit(0);
}


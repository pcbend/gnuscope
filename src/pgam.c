/*
 * pgam.c
 *
 * by John Pavan
 * for use with pgam sort type stuff
 */

//--ddc 3jun08 dbg, wrong pointer sent to strnlen (replace &dummystr with
//--ddc with dummystr.

/* --- includes --- */

//--ddc 3jun08 add some function prototypes.
#include <unistd.h>

#include "gtktwodplot.h"
#include "cursors.h"
#include "twodplotmenus.h"
#include <gdk/gdkkeysyms.h>

#include <sys/wait.h>
#include <sys/types.h>

#include "gnuscopefuncs.h"
#include "gnuscopeglobals.h"

/* --- structs --- */

/* --- globals --- */

int pgamcolormode;
int pgamcontourtype;
int pgamtype;
int interpolationmode;
//--ddc next tis already defined in gnuscopeglobals.h
// GtkWidget *Window2D;
int pgam;
//--ddc int pgammax;
struct bigdata **big_data_info;
// struct gatedata *gate_info;
GtkWidget *maintext;
GdkPoint twodmarkers[4];
int xcurrentrange[2], ycurrentrange[2];
GtkWidget *twodplot;
int ybinsize, xbinsize;
GdkColor white, black, red, blue;
static int twodplot_nmenu_items = sizeof(twodplot_menu_items) / sizeof(twodplot_menu_items[0]);
GtkWidget *twodvbox;
int gatetype;
GtkWidget *twodtext;
GdkPoint firstpoint;
int drawing_gates;
GtkWidget *twodplottable;
int twodmaxoverride;
int twodwritemin, twodwritemax;
int twodnumcontours;
char twodtitles[254][80];
struct pgam_matrix_info pgammatrixdata;
int printnotdone;
GtkWidget *twoddisplayed[16];
short int twodspectradisplayed[16];
short int numtwoddisplayed;
float twodglobalxcalibs[3];
float twodglobalycalibs[3];
int fade_mode;

/* --- function declarations --- */

static gint TwodGateSet(GtkWidget *widget, GdkEventKey *event);

/* --- item factory functions --- */

static void MenuYCalibration(GtkWidget *widget, guint *callback_action) {
  GetDialog(39);
}

static void MenuXCalibration(GtkWidget *widget, guint *callback_action) {
  GetDialog(38);
}

static void MenuPgamSetFadeMode(gpointer(callback_data), guint *callback_action) {
  fade_mode = *callback_action;
  TwodDisplayCurrentRange();
}

static void MenuPgamSetColorMode(GtkWidget *widget, guint *callback_action) {
  pgamcolormode = *callback_action;
  TwodDisplayCurrentRange();
}
static void MenuPgamSetContourType(GtkWidget *widget, guint *callback_action) {
  pgamcontourtype = *callback_action;
  TwodDisplayCurrentRange();
}
static void MenuPgamSetType(GtkWidget *widget, guint *callback_action) {
  pgamtype = *callback_action;
  TwodDisplayCurrentRange();
}

static void MenuPgamSetInterpolation(GtkWidget *widget, guint *callback_action) {
  interpolationmode = *callback_action;
  // printf("interpolationmode = %d \n",interpolationmode);
  TwodDisplayCurrentRange();
}

static void MenuTwodReadFile(GtkWidget *widget, guint *callback_action) {
  GetFilename("Read Files", 0, AutoDetectFileType);
}

static void MenuPgamSubtract2D(GtkWidget *widget, guint *callback_action) {
  GetDialog(37);
}
static void MenuPgamAdd2D(GtkWidget *widget, guint *callback_action) {
  GetDialog(36);
}
static void MenuPgamClear2D(GtkWidget *widget, guint *callback_action) {
  GetDialog(35);
}
static void MenuPgamSelect2D(GtkWidget *widget, guint *callback_action) {
  TwodCreateDisplaySelectionWindow();
  // GetDialog(34);
}

static void MenuPgamPrint(GtkWidget *widget, guint *callback_action) {
  CreatePrintWindow(twodplot);
  //  GetDialog(32);
  // gtk_twodplot_make_ps(GTK_TWODPLOT(twodplot),"tt.ps",1);
}

static void MenuPgamSetTwodTitle(GtkWidget *widget, guint *callback_action) {
  GetDialog(33);
}

static void MenuPgamWritePostscript(GtkWidget *widget, guint *callback_action) {
  GetFilename("Postscript Output", 1, WritePgamPostscript);
}

static void MenuPgamSetNumContours(GtkWidget *widget, guint *callback_action) {
  GetDialog(31);
}

static void MenuPgamBWDensity(GtkWidget *widget, guint *callback_action) {
  gtk_twodplot_set_density_type(GTK_TWODPLOT(twodplot), GTK_TWODPLOT_DENSITY_TYPE_BW);
  TwodDisplayCurrentRange();
}

static void MenuPgamColorDensity(GtkWidget *widget, guint *callback_action) {
  gtk_twodplot_set_density_type(GTK_TWODPLOT(twodplot), GTK_TWODPLOT_DENSITY_TYPE_COLOR);
  TwodDisplayCurrentRange();
}

static void MenuPgamInverseRootContours(GtkWidget *widget, guint *callback_action) {
  gtk_twodplot_set_contour_type(GTK_TWODPLOT(twodplot), GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_ROOT);
  TwodDisplayCurrentRange();
}

static void MenuPgamInverseLogContours(GtkWidget *widget, guint *callback_action) {
  gtk_twodplot_set_contour_type(GTK_TWODPLOT(twodplot), GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_LOG);
  TwodDisplayCurrentRange();
}

static void MenuPgamRootContours(GtkWidget *widget, guint *callback_action) {
  gtk_twodplot_set_contour_type(GTK_TWODPLOT(twodplot), GTK_TWODPLOT_CONTOUR_TYPE_ROOT);
  TwodDisplayCurrentRange();
}

static void MenuPgamLogContours(GtkWidget *widget, guint *callback_action) {
  gtk_twodplot_set_contour_type(GTK_TWODPLOT(twodplot), GTK_TWODPLOT_CONTOUR_TYPE_LOG);
  TwodDisplayCurrentRange();
}

static void MenuPgamLinearContours(GtkWidget *widget, guint *callback_action) {
  gtk_twodplot_set_contour_type(GTK_TWODPLOT(twodplot), GTK_TWODPLOT_CONTOUR_TYPE_LINEAR);
  TwodDisplayCurrentRange();
}

static void MenuPgamDisplayContour(GtkWidget *widget, guint *callback_action) {
  gtk_twodplot_set_plot_mode(GTK_TWODPLOT(twodplot), GTK_TWODPLOT_CONTOUR_ONLY);
  TwodDisplayCurrentRange();
}

static void MenuPgamDisplayDensity(GtkWidget *widget, guint *callback_action) {
  gtk_twodplot_set_plot_mode(GTK_TWODPLOT(twodplot), GTK_TWODPLOT_DENSITY_ONLY);
  TwodDisplayCurrentRange();
}

static void MenuPgamDisplayContourAndDensity(GtkWidget *widget, guint *callback_action) {
  gtk_twodplot_set_plot_mode(GTK_TWODPLOT(twodplot), GTK_TWODPLOT_CONTOUR_AND_DENSITY);
  TwodDisplayCurrentRange();
}

static void MenuPgamWritePgamTwod(GtkWidget *widget, guint *callback_action) {
  GetFilename("Write Pgam Matrix", 1, WritePgamTwod);
}

static void MenuPgamReadPgamTwod(GtkWidget *widget, guint *callback_action) {
  GetFilename("Read Pgam Matrix", 0, ReadPgamTwod);
}

static void MenuPgamTwodProjectFull(GtkWidget *widget, guint *callback_action) {
  PgamTwodProjectFull();
}

static void MenuNextTwod(GtkWidget *widget, guint *callback_action) {
  NextTwod();
}

static void MenuPrevTwod(GtkWidget *widget, guint *callback_action) {
  PrevTwod();
}

static void MenuPgamSort(GtkWidget *widget, guint *callback_action) {
  PgamSortSelectionWindow();
}

static void MenuTwodClearGates(GtkWidget *widget, guint *callback_action) {
  //--ddc 19jan06 add test before bombing NULL
  if (gate_info == NULL)
    return;

  GateDataStructDestroy(gate_info);
  gate_info = NULL;
  TwodDisplayCurrentRange();
}

static void MenuTwodRefresh(GtkWidget *widget, guint *callback_action) {
  TwodRefresh();
}

static void MenuTwodMaxOverRideUp(GtkWidget *widget, guint *callback_action) {
  twodmaxoverride--;
  TwodDisplayCurrentRange();
}

static void MenuTwodMaxOverRideDown(GtkWidget *widget, guint *callback_action) {
  twodmaxoverride++;
  TwodDisplayCurrentRange();
}

static void MenuTwodExpand(GtkWidget *widget, guint *callback_action) {
  TwodExpand();
}

static void MenuTwodZoomOut(GtkWidget *widget, guint *callback_action) {
  if (big_data_info != NULL) {
    xcurrentrange[0] = ycurrentrange[0] = 0;
    xcurrentrange[1] = big_data_info[pgam]->axes[0] - 1;
    ycurrentrange[1] = big_data_info[pgam]->axes[1] - 1;
    TwodDisplayCurrentRange();
  }
}

static void MenuTwodRedraw(GtkWidget *widget, guint *callback_action) {
  TwodDisplayCurrentRange();
}

static void MenuTwodReadBig(GtkWidget *widget, guint *callback_action) {

  if (pgammax)
    GetDialog(27);
  else
    GetFilename("Read Big", 0, ReadBig);
}

static void MenuTwodWriteBig(GtkWidget *widget, guint *callback_action) {
  //--ddc 26jan06 pgam? I don't think so  if (pgam > 1) {
  if (pgammax > 1) {
    GetDialog(28);
  } else
    GetFilename("Write Big", 1, WriteBig);
}

static void MenuTwodWriteGates(GtkWidget *widget, guint *callback_action) {
  GetFilename("Write Gates", 1, WriteGates);
}

static void MenuTwodReadGates(GtkWidget *widget, guint *callback_action) {
  GetFilename("Read Gates", 0, ReadGates);
}

static void MenuTwodGateFill(GtkWidget *widget, guint *callback_action) {
  TwodGateFill();
}

static void MenuPgamReadSetup(GtkWidget *widget, guint *callback_action) {
  GetFilename("Read Pgam Setup", 0, PgamReadSetup);
}

static void MenuPgamReadMasterSetup(GtkWidget *widget, guint *callback_action) {
  GetFilename("Read Pgam Master Setup", 0, PgamReadMasterSetup);
}

/* --- functions --- */

/* XCalibrationPrompt
 *
 * Prompts the user for the X calibration of the twodplot
 */
void XCalibrationPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Please enter the constant, \n");
  // gtk2 deprecation  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));

  sprintf(dummystr, "linear, and quadratic calibrations.\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "for the X-Axis.\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/* YCalibrationPrompt
 *
 * Prompts the user for the Y calibration of the twodplot
 */
void YCalibrationPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Please enter the constant, \n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "linear, and quadratic calibrations.\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "for the Y-Axis.\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/* XCalibrationEntry
 *
 * interperts the entry for the X calibration
 */
void XCalibrationEntry(GtkWidget *widget, GtkWidget *entry) {
  float i, j, k;
  if (sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%f %f %f", &i, &j, &k) == 3) {
    twodglobalxcalibs[0] = i;
    twodglobalxcalibs[1] = j;
    twodglobalxcalibs[2] = k;
    TwodDisplayCurrentRange();
  }
}

/* YCalibrationEntry
 *
 * Interpretst the entry of for the ycalibration
 */
void YCalibrationEntry(GtkWidget *widget, GtkWidget *entry) {
  float i, j, k;
  if (sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%f %f %f", &i, &j, &k) == 3) {
    twodglobalycalibs[0] = i;
    twodglobalycalibs[1] = j;
    twodglobalycalibs[2] = k;
    TwodDisplayCurrentRange();
  }
}

/* GateDataStructNew(max x, max y, depth (in bits))
 * Creates a new Gate Data Struct
 */
struct gatedata *GateDataStructNew(int x, int y, int depth) {
  struct gatedata *stru;
  int i, j, k;

  g_return_val_if_fail(x > 0, NULL);
  g_return_val_if_fail(y > 0, NULL);
  g_return_val_if_fail(depth > 0, NULL);

  stru = (struct gatedata *)malloc(sizeof(struct gatedata));

  stru->axes[0] = x;
  stru->axes[1] = y;
  stru->depth = depth;

  /* --- now we need to allocate the space for the data --- */
  //--ddc 19jan06 OUCH. So, if 'depth' is less than 32, we STILL
  //--ddc want to allocates some space, yes?
  //--ddc  stru->vals = (int ***) malloc(sizeof(int **) * (depth / 32));
  stru->vals = (int ***)malloc(sizeof(int **) * (depth / 32 + 1));
  for (i = 0; i <= (depth / 32); i++) {
    stru->vals[i] = (int **)malloc(sizeof(int *) * x);
    for (j = 0; j < x; j++) {
      stru->vals[i][j] = (int *)malloc(sizeof(int) * y);
      for (k = 0; k < y; k++) {
        stru->vals[i][j][k] = 0;
      }
    }
  }

  return (stru);
}

/* GateDataStructDestroy()
 *
 * Destroys a GateData struct
 */
void GateDataStructDestroy(struct gatedata *stru) {
  int i, j;
  for (j = (stru->depth / 32) - 1; j >= 0; j--) {
    for (i = stru->axes[0] - 1; i >= 0; i--) {
      free(stru->vals[j][i]);
    }
    free(stru->vals[j]);
  }
  free(stru->vals);
  free(stru);
}

/* GateDataStructGetVal()
 *
 * Assigns the integer possition of the "on" toggled bits for a particular channel to a pointer
 * passed to it returns 1 on success, 0 on failure
 */
int GateDataStructGetVal(struct gatedata *stru, int *result, int x, int y) {
  int i, j, k;
  /* --- rather than using the g_return_if_fail function, let's use normal logic
     --- the g_return_if_fail function can really slow down the prgoram --- */
  if (((0 <= x) && (x < stru->axes[0])) && ((0 <= y) && (y < stru->axes[1]))) {
    k = 0;
    for (i = 0; i < stru->depth; i++) {
      for (j = 0; j < 32; j++) {
        if (stru->vals[i][x][y] & (1 << j)) {
          result[k] = j;
          k++;
        }
      }
    }
    return (1);
  } else
    return (0);
}

/* GateDataStructTest
 *
 * Tests stru to see if bit is 0 or 1 at position x and y
 */
int GateDataStructTest(struct gatedata *stru, int bit, int x, int y) {
  int i, j;
  if (((0 <= x) && (x < stru->axes[0])) && ((0 <= y) && (y < stru->axes[1])) &&
      ((0 <= bit) && (bit < stru->depth))) {
    i = bit % 32; // which bit inside the depth field
    j = bit / 32; // which depth field
    return ((stru->vals[j][x][y] & (1 << i)));
  } else
    return (0);
}

/* GateDataStructSetVal()
 *
 * Accepts an int (as a bit map) for a depth, x  and y
 * returns 1 if successful, 0 if fail
 */
int GateDataStructSetVal(struct gatedata *stru, int map, int x, int y, int depth) {
  if (((0 <= x) && (x < stru->axes[0])) && ((0 <= y) && (y < stru->axes[1])) &&
      ((0 <= depth) && (depth < stru->depth))) {
    stru->vals[depth][x][y] = map;
    return (1);
  } else
    return (0);
}

/* GateDataStructToggleOn()
 * Toggles on a particular bit
 * returns 1 if successful 0 if fail
 */
int GateDataStructToggleOn(struct gatedata *stru, int bit, int x, int y) {
  int i, j;
  if (((0 <= x) && (x < stru->axes[0])) && ((0 <= y) && (y < stru->axes[1])) &&
      ((0 <= bit) && (bit < stru->depth))) {
    i = bit % 32;
    j = bit / 32;
    stru->vals[j][x][y] |= (1 << i);
    return (1);
  } else
    return (0);
}

/* GateDataStructToggleOff()
 * Toggles off a particular bit
 * Returns 1 on success, 0 if fail
 */
int GateDataStructToggleOff(struct gatedata *stru, int bit, int x, int y) {
  int i, j;
  if ((0 <= x && x < stru->axes[0]) && (0 <= y && y < stru->axes[1]) &&
      (0 <= bit && bit < stru->depth)) {
    // if ((0 <= x < stru->axes[0]) && (0 <= y < stru->axes[1]) && (0 <= bit < stru->depth)) {
    i = bit % 32;
    j = bit / 32;
    // stru->vals[j][x][y] = stru->vals[j][x][y] & (!(1 << i));
    stru->vals[j][x][y] = stru->vals[j][x][y] & ~(1 << i);
    return (1);
  } else
    return (0);
}

/* PgamSubtract2DPrompt
 *
 * lists the 2D plots in memory, and asks the user which two to add
 */
void PgamSubtract2DPrompt(GtkWidget *text) {
  char dummystr[120];
  int i;

  //--ddc 8jun06  sprintf(dummystr,"Please select 2 2Ds to add, and the output location.\n");
  sprintf(dummystr, "Please select 2 2Ds to subtract, and the output location.\n");
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 120));
  //--ddc 8jun06  sprintf(dummystr,"A + B -> C\n");
  sprintf(dummystr, "A - B -> C\n");
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 120));
  for (i = 0; i < pgammax; i++) {
    sprintf(dummystr, "%d: %s\n", i + 1, twodtitles[i]);
    gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 120));
  }
}

/* PgamSubtract2DEntry
 *
 * Interprets the result of the 2D add dialog and adds them if it makes sense
 */
void PgamSubtract2DEntry(GtkWidget *widget, GtkWidget *entry) {
  int i, j, k;
  int a, b, c;
  int tempa, tempb, tempc; //--ddc 01may06for neg test.

  if (sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%d %d %d", &a, &b, &c) == 3) {
    a--;
    b--;
    c--;
    //--ddc 01may06 ... FIX range check for entries
    //    if ((a >= 0) && (a < (pgammax - 1)) && (b >= 0) && (b < (pgammax - 1)) && (c >= 0)) {
    if ((a >= 0) && (a < pgammax) && (b >= 0) && (b < pgammax) && (c >= 0)) {
      if (c >= pgammax) { // we need to allocate more memory
        c = pgammax;
        pgammax += 1;
        //--ddc 8jun06 Need to resize big_data_info before stuffing more into it!
        big_data_info =
            (struct bigdata **)realloc(big_data_info, sizeof(struct bigdata *) * pgammax);
        big_data_info[c] = BigDataStructNew(big_data_info[a]->axes[0], big_data_info[a]->axes[1]);
      }
      //--ddc 01may06 ... ADD test for negative values
      for (i = 0; i < big_data_info[a]->axes[0]; i++) {
        for (j = 0; j < big_data_info[b]->axes[1]; j++) {

          //	  BigDataStructSetVal(big_data_info[c],i,j,
          //			      BigDataStructGetVal(big_data_info[a],i,j) -
          //			      BigDataStructGetVal(big_data_info[b],i,j));

          tempa = BigDataStructGetVal(big_data_info[a], i, j);
          tempb = BigDataStructGetVal(big_data_info[b], i, j);
          tempc = tempa - tempb;
          if (tempc < 0)
            tempc = 0;
          BigDataStructSetVal(big_data_info[c], i, j, tempc);

          if (BigDataStructGetVal(big_data_info[c], i, j) == 0)
            BigDataStructSetVal(big_data_info[c], i, j, 0);
        }
      }
      pgam = c;
      TwodDisplayCurrentRange();
    }
  }
}

/* PgamAdd2DPrompt
 *
 * lists the 2D plots in memory, and asks the user which two to add
 */
void PgamAdd2DPrompt(GtkWidget *text) {
  char dummystr[120];
  int i;

  sprintf(dummystr, "Please select 2 2Ds to add, and the output location.\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,120));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 120));
  sprintf(dummystr, "A + B -> C\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,120));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 120));
  for (i = 0; i < pgammax; i++) {
    sprintf(dummystr, "%d: %s\n", i + 1, twodtitles[i]);
    //    gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,120));
    gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 120));
  }
}

/* PgamAdd2DEntry
 *
 * Interprets the result of the 2D add dialog and adds them if it makes sense
 */
void PgamAdd2DEntry(GtkWidget *widget, GtkWidget *entry) {
  int i, j, k;
  int a, b, c;

  if (sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%d %d %d", &a, &b, &c) == 3) {
    a--;
    b--;
    c--;

    //--ddc 01may06 ... FIX range check for entries
    //    if ((a >= 0) && (a < (pgammax - 1)) && (b >= 0) && (b < (pgammax - 1)) && (c >= 0)) {
    if ((a >= 0) && (a < pgammax) && (b >= 0) && (b < pgammax) && (c >= 0)) {
      if (c >= pgammax) { // we need to allocate more memory
        c = pgammax;
        pgammax += 1;
        //--ddc 8jun06 Need to resize big_data_info before stuffing more into it!
        big_data_info =
            (struct bigdata **)realloc(big_data_info, sizeof(struct bigdata *) * pgammax);
        big_data_info[c] = BigDataStructNew(big_data_info[a]->axes[0], big_data_info[a]->axes[1]);
      }
      for (i = 0; i < big_data_info[a]->axes[0]; i++) {
        for (j = 0; j < big_data_info[b]->axes[1]; j++) {
          BigDataStructSetVal(big_data_info[c], i, j,
                              BigDataStructGetVal(big_data_info[a], i, j) +
                                  BigDataStructGetVal(big_data_info[b], i, j));
        }
      }
      pgam = c;
      TwodDisplayCurrentRange();
    }
  }
}

/* PgamClear2DPrompt
 *
 * lists the 2D plots in memory, and asks the user which one they want to delete
 */
void PgamClear2DPrompt(GtkWidget *text) {
  char dummystr[120];
  int i;
  sprintf(dummystr, "Please select a spectrum to delete.\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,120));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 120));
  for (i = 0; i < pgammax; i++) {
    sprintf(dummystr, "%d: %s\n", i + 1, twodtitles[i]);
    //    gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,120));
    gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 120));
  }
}

/* PgamClear2DEntry
 *
 * interprets the results of the entry box for clearing a 2D spectra from memory
 */
void PgamClear2DEntry(GtkWidget *widget, GtkWidget *entry) {
  int i, j;

  if (sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%d", &i) == 1) {
    i = i - 1;
    if ((i >= 0) && (i < pgammax)) { // make sure there is something to delete
      /* --- easy case is if it is the last one in memory anyway --- */
      if (i == (pgammax - 1)) {
        BigDataStructDestroy(big_data_info[i]);
        pgammax = pgammax - 1;
        if (pgam >= pgammax)
          pgam = pgammax - 1;
      } else {
        /* --- have to iterate all them down --- */
        for (j = i; j < (pgammax - 1); j++) {
          big_data_info[j] = big_data_info[j + 1];
        }
        BigDataStructDestroy(big_data_info[pgammax - 1]);
        pgammax = pgammax - 1;
        if (pgam >= pgammax)
          pgam = pgammax - 1;
      }
      if (pgammax > 0)
        TwodDisplayCurrentRange();
      else
        CloseTwodPlotWindow(GTK_WIDGET(Window2D), NULL);
    } else {
      GetMessageDialog("That 2D spectrum is already empty.\n");
    }
  }
}

/* PgamDisplaySelectionPrompt
 *
 * Lists the number of 2D plots in memory and their titles, asking
 * for the user to select the one they want
 */
void PgamDisplaySelectionPrompt(GtkWidget *text) {
  char dummystr[120];
  int i;

  sprintf(dummystr, "Please select a spectrum to display.\n");
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 120));
  for (i = 0; i < pgammax; i++) {
    sprintf(dummystr, "%d: %s\n", i + 1, twodtitles[i]);
    //    gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,120));
    gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 120));
  }
}

/* PgamDisplaySelectionEntry
 *
 * interprets the result of the pgamdisplayselectiondialog
 */
void PgamDisplaySelectionEntry(GtkWidget *widget, GtkWidget *entry) {
  int i;
  int array[16];
  int test;

  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)),
                "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &array[0], &array[1],
                &array[2], &array[3], &array[4], &array[5], &array[6], &array[7], &array[8],
                &array[9], &array[10], &array[11], &array[12], &array[13], &array[14], &array[15]);

  if (test > 0) {
    numtwoddisplayed = test;
    for (i = 0; i < test; i++) {
      twodspectradisplayed[i] = array[i] - 1;
    }
  }
  TwodDisplayCurrentRange();
  //  if (sscanf(gtk_entry_get_text(GTK_ENTRY(entry)),"%d",&i) == 1) {
  // i = i - 1;
  // if ((i >= 0) && (i < pgammax)) {
  //   pgam = i;
  //   TwodDisplayCurrentRange();
  // }
  //}
}

/* PgamSetTwodTitlePrompt
 *
 * prompts the user to set the title of the current twodplot
 */
void PgamSetTwodTitlePrompt(GtkWidget *text) {
  char dummystr[80];

  sprintf(dummystr, "What title do you want for the current twodplot?");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/* PgamSetTwodTitleEntry
 *
 * interprets what the user said they wanted for the twodplot entry
 */
void PgamSetTwodTitleEntry(GtkWidget *widget, GtkWidget *entry) {
  int test;
  char dummystr[120];
  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%s", dummystr);
  if (test == 1) {
    sprintf(twodtitles[pgam], gtk_entry_get_text(GTK_ENTRY(entry)));
    TwodDisplayCurrentRange();
  }
}

/* WritePgamPostscript
 *
 * postscript output from the current 2d plot
 */
void WritePgamPostscript(char *sFilename) {
  gtk_twodplot_make_ps(GTK_TWODPLOT(twodplot), sFilename, 0);
}

/* PgamSpawnPrintEntry
 *
 * uses import to generate a postscript
 * then, uses lpr to print
 * finally, delete the postscript
 */
void PgamSpawnPrintEntry(GtkWidget *widget, GtkWidget *entry) {
  int test;
  char dummystr[120];

  test = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%s", dummystr);
  if (test == 1) {
    PgamSpawnPrint(dummystr);
  }
}

/* PgamSpawnPrint
 *
 * does the printing
 */
void PgamSpawnPrint(char *cprinter) {
  pid_t pid;
  char dummystr[120];

  sprintf(dummystr, "/tmp/gnuscope%d%d.ps", (int)time(NULL), getpid());

  gtk_twodplot_make_ps(GTK_TWODPLOT(twodplot), dummystr, 0);

  // while (printnotdone) {
  /* --- do nothing --- */
  //}

  if ((pid = fork()) < 0)
    g_error("fork error");
  else if (pid == 0) {
    if ((pid = execl("/usr/bin/lpr", "lpr", "-r", "-P", cprinter, dummystr, (char *)0)) == -1)
      g_error("execl error");
  }
  //  if ((pid = fork()) < 0)
  // g_error("fork error");
  // else if (pid == 0) {
  // if ((pid = execl("/bin/rm","rm",dummystr,(char *) 0)) == -1 )
  //   g_error("execl error");
  // }

  if (waitpid(pid, NULL, 0) < 0)
    g_error("wait error");
}

/* NumContoursPrompt
 *
 * Queries for the number of contours
 */
void NumContoursPrompt(GtkWidget *text) {
  char dummystr[80];

  sprintf(dummystr, "How many contours do you want to display (<100)");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/* NumContoursEntry
 *
 * Interprets the result of the contour setting entry
 */
void NumContoursEntry(GtkWidget *widget, GtkWidget *entry) {
  int i;

  if (sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%d", &i) == 1) {
    if (i > 100)
      i = 100;
    twodnumcontours = i;
    gtk_twodplot_set_num_contours(GTK_TWODPLOT(twodplot), twodnumcontours);
    TwodDisplayCurrentRange();
  }
}

/* BigDataStructNew
 *
 * Creates a big data struct of size x by size y
 */
struct bigdata *BigDataStructNew(int x, int y) {
  struct bigdata *stru;
  int i, j;

  stru = (struct bigdata *)malloc(sizeof(struct bigdata));

  stru->axes[0] = x;
  stru->axes[1] = y;

  stru->vals = (int **)malloc(sizeof(int *) * x);
  for (i = 0; i < x; i++) {
    stru->vals[i] = (int *)malloc(sizeof(int) * y);
    for (j = 0; j < y; j++) {
      stru->vals[i][j] = 0;
    }
  }
  return (stru);
}

/* BigDataStructDestroy
 *
 * Destroys a BigData struct
 */
void BigDataStructDestroy(struct bigdata *stru) {
  int i;
  g_return_if_fail(stru != NULL);
  for (i = 0; i < stru->axes[0]; i++) {
    free(stru->vals[i]);
  }
  free(stru->vals);
  free(stru);
}

/* BigDataStructGetVal
 *
 * Returns the values of the big file at index x,y
 */
int BigDataStructGetVal(struct bigdata *stru, int x, int y) {
  g_return_val_if_fail((x < stru->axes[0]) && (x >= 0), 0);
  g_return_val_if_fail((y < stru->axes[1]) && (y >= 0), 0);
  return (stru->vals[x][y]);
}

/* BigDataStructSetVal
 *
 * Sets the value of the stru at index x and y.
 */
void BigDataStructSetVal(struct bigdata *stru, int x, int y, int z) {
  g_return_if_fail((x < stru->axes[0]) && (x >= 0));
  g_return_if_fail((y < stru->axes[1]) && (y >= 0));
  stru->vals[x][y] = z;
}

/* BigDataStructValPP
 *
 * increments the value of stru at x,y
 */
void BigDataStructValPP(struct bigdata *stru, int x, int y) {
  g_return_if_fail((x < stru->axes[0]) && (x >= 0));
  g_return_if_fail((y < stru->axes[1]) && (y >= 0));
  stru->vals[x][y] += 1;
}

/* KinmatInfoStructAdd
 *
 * Adds a value to a kinmat_info_struct
 */
void KinmatInfoStructAdd(struct kinmat_info_struct *stru, float ex, float en) {
  int temp;

  temp = stru->num;
  if (stru->num == 0) {
    stru->ex = (float *)malloc(sizeof(float));
    stru->en = (float *)malloc(sizeof(float));
    stru->num = 1;
  } else {
    stru->num++;
    stru->ex = (float *)realloc(stru->ex, sizeof(float) * stru->num);
    stru->en = (float *)realloc(stru->en, sizeof(float) * stru->num);
  }
  stru->ex[temp] = ex;
  stru->en[temp] = en;
}

/* KinmatInfoStructNew
 *
 * Creates a KinmatInfoStruct
 */
struct kinmat_info_struct *KinmatInfoStructNew() {
  struct kinmat_info_struct *stru;

  stru = (struct kinmat_info_struct *)malloc(sizeof(struct kinmat_info_struct));

  stru->num = 0;
  stru->ex = NULL;
  stru->en = NULL;
  return (stru);
}

/* KinmatInfoStructDestroy
 *
 * Disposes of a struct kinmat_info_struct
 */
void KinmatInfoStructDestroy(struct kinmat_info_struct *stru) {
  if (stru->num > 0) {
    free(stru->ex);
    free(stru->en);
  }
  free(stru);
}

/* KinmatInfoStructGetExcit
 *
 * Returns the interpolated excitation energy using the
 * kinmat info stored in the struct
 */
float KinmatInfoStructGetExcit(struct kinmat_info_struct *stru, float ener) {
  int i, j, k;
  float excit;

  if (stru == NULL)
    return (ener);

  if ((ener < stru->en[0]) || (ener > stru->en[stru->num - 1]))
    return (-1);
  for (i = 0; i < stru->num - 2; i++) {
    if ((ener <= stru->en[i]) && (ener >= stru->en[i + 1])) {
      return (stru->ex[i] + (stru->ex[i + 1] - stru->ex[i]) * (stru->en[i] - ener) /
                                (stru->en[i] - stru->en[i + 1]));
    }
  }
}

/* KinmatInfoStructGetEnergy
 *
 * Returns the interpolated excitation energy using the
 * kinmat info stored in the struct
 */
float KinmatInfoStructGetEnergy(struct kinmat_info_struct *stru, float ener) {
  int i, j, k;
  float excit;

  if (stru == NULL)
    return (ener);

  return (ener);

  //  if ((ener < stru->en[0]) || (ener > stru->en[stru->num - 1]))
  // return(0);
  // for (i = 0; i < stru->num - 2; i++) {
  //  if ((ener <= stru->en[i]) && (ener >= stru->en[i + 1])) {
  //   return( (stru->ex[i+1] - stru->ex[i]) *
  //	      (stru->en[i] - ener) /
  //	      (stru->en[i] - stru->en[i+1]));
  // }
  //}
}

/* ReadKinmatInfo
 *
 * reads in a kinmat info file
 */
struct kinmat_info_struct *ReadKinmatInfo(char *sFilename) {
  FILE *infile;
  int nchar, test, total;
  float dummy1, dummy2;
  struct kinmat_info_struct *stru;
  char dummystr[80];

  /* --- read the kinmat info file into memory --- */

  if ((infile = fopen(sFilename, "r")) != NULL) {
    // printf("Opened file successfully.\n");
    /* --- well, we opened the file, let's read it in until we get to the end
       --- of file marker --- */

    stru = KinmatInfoStructNew(); // need something to write the data to

    while ((fgets(dummystr, 80, infile) != NULL)) {
      test = sscanf(dummystr, "%f %f", &dummy1, &dummy2);
      if (test == 2) {
        KinmatInfoStructAdd(stru, dummy1, dummy2);
      } else {
        test = sscanf(dummystr, "%f,%f", &dummy1, &dummy2);
        if (test == 2) {
          KinmatInfoStructAdd(stru, dummy1, dummy2);
        } else {
          GetMessageDialog("Error Reading KinmatInfoStruct");
          KinmatInfoStructDestroy(stru);
          return (NULL);
        }
      }
    }

    // for (test = 0; test < stru->num; test++) {
    // printf("%f, %f\n",stru->en[test],stru->ex[test]);
    // }

    return (stru);
    fclose(infile);
  } else
    return (NULL);
}

/*
 * CloseTwodPlotWindow
 *
 * Closes the Towdplot window and frees memory associated with it
 */
gint CloseTwodPlotWindow(GtkWidget *widget, gpointer *data) {
  /* --- let's just hide it --- */
  gtk_widget_hide(widget);
  return 1;
}

/* TwodCloseGate
 *
 * closes the gate for a banna gate and fills
 */
void TwodCloseGate() {
  twodmarkers[3].y = twodmarkers[2].y;
  twodmarkers[3].x = twodmarkers[2].x;
  twodmarkers[2].y = twodmarkers[1].y;
  twodmarkers[2].x = twodmarkers[1].x;
  twodmarkers[1].y = twodmarkers[0].y;
  twodmarkers[1].x = twodmarkers[0].x;
  twodmarkers[0].x = firstpoint.x;
  twodmarkers[0].y = firstpoint.y;

  TwodDrawMarkers();
  TwodGateFill();
}

/* TwodGateFill
 *
 * fills in gates between the existing gates-marks in the y-direction
 */
void TwodGateFill() {
  // int *temparray;
  int i, j, k, l;
  int tempdepth;
  int newmemsize;
  int temparray[1000];
  // int first[32],last[32];
  int first, last;

  //  for (i = 0; i < 32; i++) {
  //  first[i] = -1;
  //  last[i] = -1;
  // }

  /* --- only do this if we have a gate_info struct and a big_data_info struct --- */
  if ((gate_info != NULL) && (big_data_info != NULL)) {

    /* --- Make the array --- */
    // newmemsize = sizeof(int) * big_data_info[pgam]->axes[0];
    // temparray = (int *) malloc(newmemsize);

    tempdepth = Min(pgammax, gate_info->depth);
    for (i = 0; i < tempdepth; i++) {
      for (j = 0; j < gate_info->axes[1]; j++) {
        for (k = 0; k < gate_info->axes[0]; k++) {
          temparray[k] = GateDataStructTest(gate_info, i, j, k);
        } // done making temparray
        first = FirstOccurance(temparray, gate_info->axes[1], 1 << i);
        last = LastOccurance(temparray, gate_info->axes[1], 1 << i);
        for (l = first; l < last; l++) {
          GateDataStructToggleOn(gate_info, i, j, l);
        }
      } // scanning through the y axis
    } // scanning through the gates for the depth of the gate array (or pgammax whichever is less

    TwodDisplayCurrentRange();
  } else
    GetMessageDialog("There is either no gate info or 2D info in memory.");
}

/* First Occurance
 *
 * Returns the index of the first occurance of needle
 */
int FirstOccurance(int *haystack, int straws, int needle) {
  int i;

  for (i = 0; i < straws; i++) {
    if ((*(haystack + i) & needle))
      return (i);
  }
  return (0);
}

/* LastOccurance
 *
 * Returns the index of the last occurance of needle in haystack
 */
int LastOccurance(int *haystack, int straws, int needle) {
  int i;
  for (i = straws - 1; i >= 0; i--) {
    if ((*(haystack + i) & needle))
      return (i);
  }
  return (0);
}

/* TwodShiftUpRight
 *
 * Shifts the display region of the twodplot to the Right
 */
void TwodShiftUpRight() {
  int width, height;

  width = abs(xcurrentrange[1] - xcurrentrange[0]);
  height = abs(ycurrentrange[1] - ycurrentrange[0]);

  if ((xcurrentrange[1] + (float)width / (float)2) < big_data_info[pgam]->axes[0]) {
    xcurrentrange[0] = xcurrentrange[0] + (float)width / (float)2;
    xcurrentrange[1] = xcurrentrange[1] + (float)width / (float)2;
  } else {
    xcurrentrange[0] = big_data_info[pgam]->axes[0] - width - 1;
    xcurrentrange[1] = big_data_info[pgam]->axes[0] - 1;
  }
  if ((ycurrentrange[1] + (float)height / (float)2) < big_data_info[pgam]->axes[1]) {
    ycurrentrange[0] = ycurrentrange[0] + (float)height / (float)2;
    ycurrentrange[1] = ycurrentrange[1] + (float)height / (float)2;
  } else {
    ycurrentrange[0] = big_data_info[pgam]->axes[1] - height - 1;
    ycurrentrange[1] = big_data_info[pgam]->axes[1] - 1;
  }
  TwodDisplayCurrentRange();
}

/* TwodShiftDownRight
 *
 * Shifts the display region of the twodplot to the down
 */
void TwodShiftDownRight() {
  int height, width;

  height = abs(ycurrentrange[1] - ycurrentrange[0]);
  width = abs(xcurrentrange[1] - xcurrentrange[0]);

  if ((xcurrentrange[1] + (float)width / (float)2) < big_data_info[pgam]->axes[0]) {
    xcurrentrange[0] = xcurrentrange[0] + (float)width / (float)2;
    xcurrentrange[1] = xcurrentrange[1] + (float)width / (float)2;
  } else {
    xcurrentrange[0] = big_data_info[pgam]->axes[0] - width - 1;
    xcurrentrange[1] = big_data_info[pgam]->axes[0] - 1;
  }
  if ((ycurrentrange[0] - (float)height / (float)2) > 0) {
    ycurrentrange[0] = ycurrentrange[0] - (float)height / (float)2;
    ycurrentrange[1] = ycurrentrange[1] - (float)height / (float)2;
  } else {
    ycurrentrange[0] = 0;
    ycurrentrange[1] = height;
  }
  TwodDisplayCurrentRange();
}

/* TwodShiftRight
 *
 * Shifts the display region of the twodplot to the Right
 */
void TwodShiftRight() {
  int width;

  width = abs(xcurrentrange[1] - xcurrentrange[0]);

  if ((xcurrentrange[1] + (float)width / (float)2) < big_data_info[pgam]->axes[0]) {
    xcurrentrange[0] = xcurrentrange[0] + (float)width / (float)2;
    xcurrentrange[1] = xcurrentrange[1] + (float)width / (float)2;
  } else {
    xcurrentrange[0] = big_data_info[pgam]->axes[0] - width - 1;
    xcurrentrange[1] = big_data_info[pgam]->axes[0] - 1;
  }
  TwodDisplayCurrentRange();
}

/* TwodShiftUp
 *
 * Shifts the display region of the twodplot to the Up
 */
void TwodShiftUp() {
  int height;

  height = abs(ycurrentrange[1] - ycurrentrange[0]);

  if ((ycurrentrange[1] + (float)height / (float)2) < big_data_info[pgam]->axes[1]) {
    ycurrentrange[0] = ycurrentrange[0] + (float)height / (float)2;
    ycurrentrange[1] = ycurrentrange[1] + (float)height / (float)2;
  } else {
    ycurrentrange[0] = big_data_info[pgam]->axes[1] - height - 1;
    ycurrentrange[1] = big_data_info[pgam]->axes[1] - 1;
  }
  TwodDisplayCurrentRange();
}

/* TwodShiftUpLeft
 *
 * Shifts the display region of the twodplot to the Up
 */
void TwodShiftUpLeft() {
  int height, width;

  height = abs(ycurrentrange[1] - ycurrentrange[0]);
  width = abs(xcurrentrange[1] - xcurrentrange[0]);

  if ((ycurrentrange[1] + (float)height / (float)2) < big_data_info[pgam]->axes[1]) {
    ycurrentrange[0] = ycurrentrange[0] + (float)height / (float)2;
    ycurrentrange[1] = ycurrentrange[1] + (float)height / (float)2;
  } else {
    ycurrentrange[0] = big_data_info[pgam]->axes[1] - height - 1;
    ycurrentrange[1] = big_data_info[pgam]->axes[1] - 1;
  }
  if ((xcurrentrange[0] - (float)width / (float)2) > 0) {
    xcurrentrange[0] = xcurrentrange[0] - (float)width / (float)2;
    xcurrentrange[1] = xcurrentrange[1] - (float)width / (float)2;
  } else {
    xcurrentrange[0] = 0;
    xcurrentrange[1] = width;
  }
  TwodDisplayCurrentRange();
}

/* TwodShiftLeft
 *
 * Shifts the display region of the twodplot to the left
 */
void TwodShiftLeft() {
  int width;

  width = abs(xcurrentrange[1] - xcurrentrange[0]);

  if ((xcurrentrange[0] - (float)width / (float)2) > 0) {
    xcurrentrange[0] = xcurrentrange[0] - (float)width / (float)2;
    xcurrentrange[1] = xcurrentrange[1] - (float)width / (float)2;
  } else {
    xcurrentrange[0] = 0;
    xcurrentrange[1] = width;
  }
  TwodDisplayCurrentRange();
}

/* TwodShiftDownLeft
 *
 * Shifts the display region of the twodplot to the down
 */
void TwodShiftDownLeft() {
  int height, width;

  height = abs(ycurrentrange[1] - ycurrentrange[0]);
  width = abs(xcurrentrange[1] - xcurrentrange[0]);

  if ((ycurrentrange[0] - (float)height / (float)2) > 0) {
    ycurrentrange[0] = ycurrentrange[0] - (float)height / (float)2;
    ycurrentrange[1] = ycurrentrange[1] - (float)height / (float)2;
  } else {
    ycurrentrange[0] = 0;
    ycurrentrange[1] = height;
  }
  if ((xcurrentrange[0] - (float)width / (float)2) > 0) {
    xcurrentrange[0] = xcurrentrange[0] - (float)width / (float)2;
    xcurrentrange[1] = xcurrentrange[1] - (float)width / (float)2;
  } else {
    xcurrentrange[0] = 0;
    xcurrentrange[1] = width;
  }
  TwodDisplayCurrentRange();
}

/* TwodShiftDown
 *
 * Shifts the display region of the twodplot to the down
 */
void TwodShiftDown() {
  int height;

  height = abs(ycurrentrange[1] - ycurrentrange[0]);

  if ((ycurrentrange[0] - (float)height / (float)2) > 0) {
    ycurrentrange[0] = ycurrentrange[0] - (float)height / (float)2;
    ycurrentrange[1] = ycurrentrange[1] - (float)height / (float)2;
  } else {
    ycurrentrange[0] = 0;
    ycurrentrange[1] = height;
  }
  TwodDisplayCurrentRange();
}

/* TwodGateSet
 *
 * Sets the channel of the last marker to gate type <keypress>
 */
static gint TwodGateSet(GtkWidget *widget, GdkEventKey *event) {
  float x, y, z;
  float slope;
  int i, j, k;

  if (big_data_info != NULL) {
    //    if (gate_info == NULL) TwodInitializeGate();
    switch (event->keyval) {
    case GDK_0:
      if (gate_info != NULL) {
        if (abs(twodmarkers[0].x - firstpoint.x) != 0) {
          slope =
              (float)(twodmarkers[0].y - firstpoint.y) / (float)(twodmarkers[0].x - firstpoint.x);
        } else {
          slope = 1000000;
        }
        y = Min(twodmarkers[0].x, firstpoint.x);
        z = Max(twodmarkers[0].x, firstpoint.x);
        if (twodmarkers[0].x == y)
          x = twodmarkers[0].y;
        else
          x = firstpoint.y;
        for (i = y; i <= z; i++) {
          j = slope * (i - y) + x;
          GateDataStructToggleOn(gate_info, gatetype, i, j);
          // BigDataStructSetVal(gate_info,i,j,BigDataStructGetVal(gate_info,i,j) | (1 <<
          // gatetype));
        }
        if (abs(twodmarkers[0].y - firstpoint.y) != 0) {
          slope =
              (float)(twodmarkers[0].x - firstpoint.x) / (float)(twodmarkers[0].y - firstpoint.y);
        } else {
          slope = 1000000;
        }
        x = Min(twodmarkers[0].y, firstpoint.y);
        z = Max(twodmarkers[0].y, firstpoint.y);
        if (twodmarkers[0].y == x)
          y = twodmarkers[0].x;
        else
          y = firstpoint.x;
        for (i = x; i <= z; i++) {
          j = slope * (i - x) + y;
          GateDataStructToggleOn(gate_info, gatetype, j, i);
          //	  BigDataStructSetVal(gate_info,j,i,BigDataStructGetVal(gate_info,j,i) | (1 <<
          //gatetype));
        }
      }
      if (drawing_gates) {
        TwodCloseGate();
      } // handle the end of a gate
      gatetype = -1;
      //      gate_info->vals[twodmarkers[0].x + twodmarkers[0].y * gate_info->axes[0]] = 0;
      drawing_gates = 0;
      break;
    case GDK_1:
    case GDK_2:
    case GDK_3:
    case GDK_4:
    case GDK_5:
    case GDK_6:
    case GDK_7:
    case GDK_8:
    case GDK_9:
      drawing_gates = 1;
      //--ddc      firstpoint.x = twodmarkers[0].x;
      //--ddc      firstpoint.y = twodmarkers[0].y;
      gatetype = pgam;
      break;
    case GDK_Right:
    case GDK_KP_6:
    case GDK_KP_Right:
      g_signal_stop_emission_by_name(GTK_OBJECT(widget), "key_press_event");
      TwodShiftRight();
      break;
    case GDK_Left:
    case GDK_KP_4:
    case GDK_KP_Left:
      g_signal_stop_emission_by_name(GTK_OBJECT(widget), "key_press_event");
      TwodShiftLeft();
      break;
    case GDK_Up:
    case GDK_KP_8:
    case GDK_KP_Up:
      g_signal_stop_emission_by_name(GTK_OBJECT(widget), "key_press_event");
      TwodShiftUp();
      break;
    case GDK_Down:
    case GDK_KP_2:
    case GDK_KP_Down:
      g_signal_stop_emission_by_name(GTK_OBJECT(widget), "key_press_event");
      TwodShiftDown();
      break;
    case GDK_KP_3:
    case GDK_KP_Page_Down:
      //      gtk_signal_emit_stop_by_name(GTK_OBJECT(widget),"key_press_event");
      g_signal_stop_emission_by_name(GTK_OBJECT(widget), "key_press_event");
      TwodShiftDownRight();
      break;
    case GDK_KP_1:
    case GDK_KP_End:
      TwodShiftDownLeft();
      g_signal_stop_emission_by_name(GTK_OBJECT(widget), "key_press_event");
      break;
    case GDK_KP_9:
    case GDK_KP_Page_Up:
      g_signal_stop_emission_by_name(GTK_OBJECT(widget), "key_press_event");
      TwodShiftUpRight();
      break;
    case GDK_KP_7:
    case GDK_KP_Home:
      g_signal_stop_emission_by_name(GTK_OBJECT(widget), "key_press_event");
      TwodShiftUpLeft();
      break;
    case GDK_Page_Up:
      g_signal_stop_emission_by_name(GTK_OBJECT(widget), "key_press_event");
      NextTwod();
      break;
    case GDK_Page_Down:
      g_signal_stop_emission_by_name(GTK_OBJECT(widget), "key_press_event");
      PrevTwod();
      break;
    default:
      // do nothing
      break;
    } // end switch
  }
  return (0); //--ddc aug11 Return ZERO, or no other eventhandlers are executed!!
}

/* TwodInitializeGate
 *
 * Creates and initializes the gate_info data structure
 */
void TwodInitializeGate() {
  int newmemsize, i;

  if (gate_info == NULL) {
    gate_info =
        GateDataStructNew(big_data_info[pgam]->axes[0], big_data_info[pgam]->axes[1], pgammax);
  } else
    GetMessageDialog("No 2D data in memory.");
}

/* TwodRefresh
 *
 * Refreshes the twodplot display
 */
void TwodRefresh() {
  //--ddc 24may06  if (twodplot != NULL) {
  if (GTK_IS_WIDGET(twodplot)) {
    gtk_twodplot_refresh(twodplot);
  }
}

/* TwodExpand
 *
 * Expands the twodplot display using the last two markers as points of the square
 */
void TwodExpand() {
  int i, tempx[2], tempy[2];
  twodmaxoverride = 0;
  /* --- there can be problems if we try to display with a region
     --- that is zero channels wide or zero channels high --- */
  /* --- therefore we should scan for the first marker which won't
     --- result in this problem --- */
  tempx[0] = twodmarkers[0].x;
  i = 1;
  tempx[1] = twodmarkers[1].x;
  while ((tempx[0] == twodmarkers[i].x) && (i < 3)) {
    tempx[1] = twodmarkers[i + 1].x;
    i++;
  }

  tempy[0] = twodmarkers[0].y;
  i = 1;
  tempy[1] = twodmarkers[1].y;
  while ((tempy[0] == twodmarkers[i].y) && (i < 3)) {
    tempy[1] = twodmarkers[i + 1].y;
    i++;
  }

  if ((tempx[0] != tempx[1]) && (tempy[0] != tempy[1])) {
    xcurrentrange[0] = Min(tempx[0], tempx[1]);
    xcurrentrange[1] = Max(tempx[0], tempx[1]);
    ycurrentrange[0] = Min(tempy[0], tempy[1]);
    ycurrentrange[1] = Max(tempy[0], tempy[1]);
    TwodDisplayCurrentRange();
  } else {
    GetMessageDialog("Expand would result in a plot with zero area.");
  }
}

/* ReadBigPrompt
 *
 * Queries the user if they want to retain any spectra in
 * memory before reading in the spectra from a file
 */
void ReadBigPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "Do you wish to retain the 2D plots in memory?(y,n)[y]");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/* ReadBigEntry
 *
 * Receives the answer to the query about retaining spectra
 */
void ReadBigEntry(GtkWidget *widget, GtkWidget *entry) {
  char dummystr[80];
  int i;

  if (sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%s", dummystr) == 1) {
    if ((strstr(dummystr, "n")) || strstr(dummystr, "N")) {
      for (i = 0; i < pgammax; i++) {
        BigDataStructDestroy(big_data_info[i]);
      }
      pgammax = 0;
      pgam = 0;
    }
    GetFilename("Read Big", 0, ReadBig);
  }
}

/* WriteBigPrompt
 *
 * Queries the user which 2Ds they want to write
 */
void WriteBigPrompt(GtkWidget *text) {
  char dummystr[80];
  sprintf(dummystr, "There are %d 2D spectra in memory.\n", pgammax);
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "Please type the 1rst and last 2D to save.\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
  sprintf(dummystr, "Min Max\n");
  //  gtk_text_insert(GTK_TEXT(text),NULL,NULL,NULL,dummystr,strnlen(dummystr,80));
  gtk_text_buffer_set_text(mytextbuffer(text), dummystr, strnlen(dummystr, 80));
}

/* WriteBigEntry
 *
 * Interprets the answer to the WriteBigPrompt
 */
void WriteBigEntry(GtkWidget *widget, GtkWidget *entry) {
  int i, j;

  if (sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%d %d", &i, &j) == 2) {
    if ((i > 0) && (j > 0) && (i <= (pgammax)) && (j <= (pgammax))) {
      twodwritemin = i - 1;
      twodwritemax = j - 1;
      GetFilename("Write Big", 1, WriteBig);
    }
  }
}

/* ReadBig
 *
 * Reads in two d e-deltae plot
 */
void ReadBig(char *sFilename) {
  FILE *infile;
  int nchar, test, total;
  int i, j, k, l;
  int y, z;
  int *tempints;
  char dummystr[80];
  int newmemsize;
  int a1, a2, a3, a4, a5, a6, a7, a8, a9, a0;

  //--ddc error trapping on reads
  int numelem;

  if (strstr(sFilename, ".big")) {
    /* --- this is the method of reading if it is an ascii based
       --- big file for compatablility with prior versions of scope --- */

    if ((infile = fopen(sFilename, "r")) != NULL) {
      /* --- have to start by allocating memory for the darn struct --- */
      /* --- let's do this so that if there is already one
         --- in memory we put the new one in the next spot --- */

      //--ddc 8jun06 the logic in adjusting pgammax to 1 if big_data_info is NULL escapes me.  If
      //this
      //--ddc 8jun06 is EVER true, you are in big trouble!!! Just increment pgammax!
      //  if (big_data_info == NULL) {
      //    pgammax = 1;
      //  } else {
      //    pgammax++;
      //  }
      pgammax++;

      //--ddc 8jun06 ... AND why would you make it one bigger than pgammax?!  this is the only
      //reason this bug has persisted
      //--ddc 8jun06 for as long as it has....
      //  big_data_info = (struct bigdata **) realloc(big_data_info,
      //					      sizeof(struct bigdata *) * (pgammax + 1));

      big_data_info = (struct bigdata **)realloc(big_data_info, sizeof(struct bigdata *) * pgammax);

      fscanf(infile, "%d,%d", &i, &j);

      big_data_info[pgammax - 1] = BigDataStructNew(i, j);

      test = 10;
      for (k = 0; k < i; k++) {
        l = 0;
        while (l < j) {
          //--ddc 30jul09 eliminate the read 10 items, or fail
          test = fscanf(infile, "%d,", &a1);
          if (test) {
            BigDataStructSetVal(big_data_info[pgammax - 1], k, l, a1);
          } else {
            break;
          }
          l = l + test;
        }
        //--ddc 30jul09 eliminate the read 10 items, or fail
        /*--ddc eliminate this commented section on a cleanup
        if (test < 10) {
          GetMessageDialog("Error Reading Bigfile.\n");
          //--ddc clean up pgammax
          pgammax--;
          break;
        }
        */
        if (test == 0) {
          GetMessageDialog("Error Reading Bigfile.\n");
          //--ddc clean up pgammax
          pgammax--;
          break;
        }
      }
      //--ddc and make the message right
      //--ddc 30jul09      if (test==10) {
      if (test != 0) {
        WriteMainText("Successfully read pgam big file.\n");
      } else {
        WriteMainText("Failed read pgam big file.\n");
      }

      fclose(infile);
    } else
      GetMessageDialog("Error opening file.\n"); // end open file condition
  } else {
    /* --- this is the method for reading as a binary file --- */
    if ((infile = fopen(sFilename, "r")) != NULL) {
      /* --- have to start by allocating memory for the darn struct --- */
      /* --- let's do this so that if there is already one
         --- in memory we put the new one in the next spot --- */
      /* --- get number to read in --- */
      numelem = fread(&z, 4, 1, infile);
      for (y = 0; y < z; y++) {

        //--ddc 8jun06 the logic in adjusting pgammax to 1 if big_data_info is NULL escapes me.  If
        //this
        //--ddc 8jun06 is EVER true, you are in big trouble!!! Just increment pgammax!
        //  if (big_data_info == NULL) {
        //    pgammax = 1;
        //  } else {
        //    pgammax++;
        //  }
        pgammax++;

        //--ddc 8jun06 ... AND why would you make it one bigger than pgammax?!  this is the only
        //reason this bug has persisted
        //--ddc 8jun06 for as long as it has....
        //  big_data_info = (struct bigdata **) realloc(big_data_info,
        //					      sizeof(struct bigdata *) * (pgammax + 1));

        big_data_info =
            (struct bigdata **)realloc(big_data_info, sizeof(struct bigdata *) * pgammax);
        /* --- get the x and y sizes --- */
        numelem = fread(&twodtitles[pgammax - 1], sizeof(char), 80, infile);
        numelem = fread(&i, 4, 1, infile);
        numelem = fread(&j, 4, 1, infile);
        /* --- make a place to put the information --- */
        big_data_info[pgammax - 1] = BigDataStructNew(i, j);
        /* --- allocate memory to temporarially store the information --- */
        tempints = (int *)malloc(sizeof(int) * i * j);
        numelem = fread(tempints, sizeof(int), i * j, infile);
        for (k = 0; k < i; k++) {
          for (l = 0; l < j; l++) {
            BigDataStructSetVal(big_data_info[pgammax - 1], k, l, tempints[k + (l * i)]);
          }
        }
        /* --- must avoid memory leaks --- */
        free(tempints);
      }
      fclose(infile);
    } else
      GetMessageDialog("Error opening file.\n"); // end open file condition
  }
}

/* WriteBig
 *
 * Writes out a twod e-deltae plot
 */
void WriteBig(char *sFilename) {
  /* --- okay, something weird happened with this function --- */
  /* --- however, it does to be internal to this funciton --- */
  /* --- therefore we will have a re-rwite of it --- */
  FILE *outfile;
  int i, j, k, l;
  int outbuffer[8192];
  int outbuffercounter;
  char dummystr[120];
  int filetype;
  enum { BIG, EDE, GNU };

  /* --- first of all we want to either auto-detect the file type
     --- from the name, or add the .ede file extention --- */
  sprintf(dummystr, sFilename);
  if (strstr(dummystr, ".big") != NULL) {
    filetype = BIG;
  } else {
    if (strstr(dummystr, ".gnu") != NULL) {
      filetype = GNU;
    } else {
      filetype = EDE;
      if (strstr(dummystr, ".ede") == NULL) {
        strcat(dummystr, ".ede");
      }
    }
  }

  switch (filetype) {
  case GNU:
    if ((outfile = fopen(dummystr, "w")) != NULL) {
      WriteMainText("GNU Write only writes one plot.\n");
      for (i = 0; i < big_data_info[twodwritemin]->axes[0]; i++) {
        for (j = 0; j < big_data_info[twodwritemin]->axes[1]; j++) {
          fprintf(outfile, "%d %d %d\n", i, j,
                  BigDataStructGetVal(big_data_info[twodwritemin], i, j));
        }
      }
    }
    break;
  case BIG:
    if ((outfile = fopen(dummystr, "w")) != NULL) {
      /* --- in the case of the BIG file, we are only writting
         --- one 2D spectrum to file, in ascii format --- */
      if (big_data_info[twodwritemin] != NULL) {
        /* --- now things get pretty bone headed --- */
        //	fprintf(outfile,"%d,%d",big_data_info[twodwritemin]->axes[0],
        //		big_data_info[twodwritemin]->axes[1]);
        //--ddc 17jan06 This needs a linefeed to separate from data!!
        fprintf(outfile, "%d,%d\n", big_data_info[twodwritemin]->axes[0],
                big_data_info[twodwritemin]->axes[1]);

        /* --- now write out the counts per channel --- */
        k = 0;
        for (i = 0; i < big_data_info[twodwritemin]->axes[0]; i++) {
          for (j = 0; j < big_data_info[twodwritemin]->axes[1]; j++) {
            k++;
            fprintf(outfile, "%d", BigDataStructGetVal(big_data_info[twodwritemin], i, j));
            if (k % 10) {
              fprintf(outfile, ",");
            } else {
              fprintf(outfile, "\n");
            }
          }
        }
      }
      fclose(outfile);
    }
    break;
  case EDE:
  default:
    if ((outfile = fopen(dummystr, "wb")) != NULL) {
      /* --- this is somewhat more complex
         --- we are writing however many spectra are designated by
         --- twodwritemin and twodwritemax in binary format --- */
      /* --- oh, and we want to do it buffered --- */
      i = abs(twodwritemax - twodwritemin) + 1;
      /* --- file begins with the number of spectra it will contain --- */
      fwrite(&i, 4, 1, outfile);
      //--ddc 26jan06      outbuffercounter = 0;

      /* --- now we want to start writting each spectrum --- */
      for (l = twodwritemin; l <= twodwritemax; l++) {
        /* --- each one begins with it's title --- */
        fwrite(&twodtitles[l], sizeof(char), 80, outfile);
        fwrite(&big_data_info[l]->axes[0], 4, 1, outfile);
        fwrite(&big_data_info[l]->axes[1], 4, 1, outfile);
        /* --- now for writing stuff out --- */
        //--ddc 26jan06 only buffering the data! moved counter to here!
        outbuffercounter = 0;
        for (i = 0; i < big_data_info[l]->axes[0]; i++) {
          for (j = 0; j < big_data_info[l]->axes[1]; j++) {
            outbuffer[outbuffercounter] = BigDataStructGetVal(big_data_info[l], j, i);
            outbuffercounter++;
            if (outbuffercounter >= 8192) {
              fwrite(outbuffer, 4, outbuffercounter, outfile);
              outbuffercounter = 0;
            }
          }
        }
        //--ddc 26jan06 BIG OOPS spectrum titles,sizes not in buffer!!!
        //--ddc 26jan06 The outbuffer must be cleared before next spectrum!!!
        fwrite(outbuffer, 4, outbuffercounter, outfile);
      } // done with all spectra
      //--ddc 26jan06 next no longer needed
      //     fwrite(outbuffer,4,outbuffercounter,outfile);
      fclose(outfile);
    }
    break;
  };
}

/* ReadGates
 *
 * reades in gates from file *sFilename
 */
void ReadGates(char *sFilename) {
  FILE *infile;
  int nchar, test, total;
  int i, j, k, l;
  char dummystr[80];
  int newmemsize;
  int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
  // TwodInitializeGate();

  /* --- we have to have two casses so the old gate files can be used --- */
  /* --- we will do this the file extention for the old-style will
     --- be .gde, and that for the new will be .tgt --- */
  if (strstr(sFilename, ".gde") != NULL) {
    if (gate_info != NULL) {
      if ((infile = fopen(sFilename, "r")) != NULL) {
        if (gate_info != NULL) {
          GateDataStructDestroy(gate_info);
        }
        fscanf(infile, "%d,%d", &i, &j);
        gate_info = GateDataStructNew(i, j, 32);
        test = 10;
        for (k = 0; k < i; k++) {
          test = 10;
          l = 0;
          while (l < j) {
            test = fscanf(infile, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &a1, &a2, &a3, &a4, &a5, &a6,
                          &a7, &a8, &a9, &a0);
            GateDataStructSetVal(gate_info, a1, k, l, 0);
            GateDataStructSetVal(gate_info, a2, k, l + 1, 0);
            GateDataStructSetVal(gate_info, a3, k, l + 2, 0);
            GateDataStructSetVal(gate_info, a4, k, l + 3, 0);
            GateDataStructSetVal(gate_info, a5, k, l + 4, 0);
            GateDataStructSetVal(gate_info, a6, k, l + 5, 0);
            GateDataStructSetVal(gate_info, a7, k, l + 6, 0);
            GateDataStructSetVal(gate_info, a8, k, l + 7, 0);
            GateDataStructSetVal(gate_info, a9, k, l + 8, 0);
            GateDataStructSetVal(gate_info, a0, k, l + 9, 0);
            l = l + 10;
          }
          if (test < 10) {
            GetMessageDialog("Error Reading Gatefile.\n");
            break;
          }
        }
        fclose(infile);
      } else
        GetMessageDialog("Error opening file.\n"); // end open file condition
    }
  } else {
    /* --- this is the new type --- */
    /* --- this file type will have x, y, and depth followed by the binary data --- */
    /* --- some local variables --- */
    int tempints[1000]; // should be more than large enough to hold some data

    if ((infile = fopen(sFilename, "rb")) != NULL) {
      if (gate_info != NULL) { // clear the gate data structure if it exists.
        GateDataStructDestroy(gate_info);
        gate_info = NULL;
      }
      /* --- now let's read out the header --- */
      fread(tempints, sizeof(int), 3, infile);
      /* --- at this point tempints[0] is the x dimension --- */
      /* --- tempints[1] is the y dimension, and tempints[2] is the depth --- */
      gate_info = GateDataStructNew(tempints[0], tempints[1], tempints[2]);
      /* --- now we should have a sufficiently large gateinfo struct --- */
      for (i = 0; i <= (gate_info->depth / 32); i++) {
        for (j = 0; j < gate_info->axes[0]; j++) {
          fread(tempints, sizeof(int), gate_info->axes[1], infile);
          for (k = 0; k < gate_info->axes[0]; k++) {
            gate_info->vals[i][j][k] = tempints[k];
          }
        }
      }

      fclose(infile);
    }
  }
}

/* WriteGates
 *
 * Writes the gates in memory to file
 */
void WriteGates(char *sFilename) {
  FILE *outfile;
  int nchar, test, total;
  int i, j, k, l;
  char dummystr[129];
  int tempints[1000];

  /* --- we write the one that is currently displayed --- */
  /* --- in a manner similar to that of read gates, we need to do the file-extention thing here too
   * --- */
  /* --- again .gde is the ascii format and .tgt is the binary format --- */

  if (gate_info != NULL) {
    if (strstr(sFilename, ".gde") != NULL) {
      if ((outfile = fopen(sFilename, "w")) != NULL) {
        fprintf(outfile, "%d,%d\n", gate_info->axes[0], gate_info->axes[1]);
        k = 0;
        for (i = 0; i < gate_info->axes[0]; i++) {
          for (j = 0; j < gate_info->axes[1]; j++) {
            k++;
            fprintf(outfile, "%d", gate_info->vals[0][i][j]);
            if (k % 10) {
              fprintf(outfile, ",");
            } else {
              fprintf(outfile, "\n");
            }
          }
        }
        fclose(outfile);
        WriteTwodText("Successfully wrote out twoddata.\n");
      } else
        GetMessageDialog("Error Writing file.\n");
    } else {
      /* --- this is the binary case --- */
      /* --- we want to make sure that the file name contains .tgt as the file extention --- */
      strncpy(dummystr, sFilename, 116);
      if (strstr(dummystr, ".tgt") == NULL) {
        strcat(dummystr, ".tgt");
      }
      /* --- ok now let's open the file --- */
      if ((outfile = fopen(dummystr, "wb")) != NULL) {
        fwrite(&gate_info->axes[0], sizeof(int), 1, outfile);
        fwrite(&gate_info->axes[1], sizeof(int), 1, outfile);
        fwrite(&gate_info->depth, sizeof(int), 1, outfile);
        l = 0;
        for (i = 0; i <= (gate_info->depth / 32); i++) {
          for (j = 0; j < gate_info->axes[0]; j++) {
            for (k = 0; k < gate_info->axes[1]; k++) {
              tempints[l] = gate_info->vals[i][j][k];
              l++;
              if (l == 1000) {
                fwrite(tempints, sizeof(int), l, outfile);
                l = 0;
              }
            }
          }
        }
        fwrite(tempints, sizeof(int), l, outfile);
        fclose(outfile);
      } else
        GetMessageDialog("Error Writing Gate information to file.\n");
    }
  } else
    GetMessageDialog("No 2D data in memory.\n");
}

/* NextTwod
 *
 * Display the next twodplot in memory
 */
void NextTwod() {
  char dummystr[80];
  int i;

  /* --- in the case of multiple displays we want to make sure that we
     --- change only the one which is active --- */

  for (i = 0; i < numtwoddisplayed; i++) {
    if (twodplot == twoddisplayed[i]) {
      if (twodspectradisplayed[i] < pgammax - 1) {
        twodspectradisplayed[i] += 1;
        TwodDisplayCurrentRange();
        sprintf(dummystr, "Displaying %d: %s\n", (twodspectradisplayed[i] + 1),
                twodtitles[twodspectradisplayed[i]]);
        WriteTwodText(dummystr);
      }
    }
  }
}

/* PrevTowd
 *
 * Display the previous twodplot in memory
 */
void PrevTwod() {
  char dummystr[80];
  int i;

  /* --- in the case of multiple displays we want to make sure that we
     --- only change the one which is active --- */
  for (i = 0; i < numtwoddisplayed; i++) {
    if (twodplot == twoddisplayed[i]) {
      if (twodspectradisplayed[i] > 0) {
        twodspectradisplayed[i] -= 1;
        TwodDisplayCurrentRange();
        sprintf(dummystr, "Displaying %d: %s\n", (twodspectradisplayed[i] + 1),
                twodtitles[twodspectradisplayed[i]]);
        WriteTwodText(dummystr);
      }
    }
  }
}

/*
 * TwodDisplayCurrentRange
 *
 * Analogous to the histogram function DisplayCurrentRange, but with two axes
 */
void TwodDisplayCurrentRange() {
  int i, j, k, m, n;
  int height, width;
  GtkWidget *temptable;

  //--ddc 24may06  if (Window2D == NULL) DisplayBig();
  if (!GTK_IS_WIDGET(Window2D))
    DisplayBig();
  else
    gtk_widget_show(Window2D);

  /* --- we have problems if we try to display something zero
     --- channels across in either direction --- */

  /* --- do the initalizations --- */

  /* --- the first time this function is called we need to create the structs
     --- for the twod display and enter them in an appropriate table
     --- but then only show the ones which are supposed to contain data --- */

  //--ddc 24may06  if (twoddisplayed[0] == NULL) {
  if (!GTK_IS_WIDGET(twoddisplayed[0])) {
    temptable = gtk_table_new(6, 6, FALSE);
    gtk_container_add(GTK_CONTAINER(twodvbox), temptable);
    gtk_table_set_row_spacing(GTK_TABLE(temptable), 0, 2);
    gtk_table_set_col_spacing(GTK_TABLE(temptable), 2, 0);
    for (i = 0; i < 16; i++) {
      twoddisplayed[i] = gtk_twodplot_new();
      gtk_widget_set_events(twoddisplayed[i], GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK);
      //--ddc jul11 gtk deprecation.. more aug12
      g_signal_connect_after(GTK_OBJECT(twoddisplayed[i]), "button_press_event",
                             (GCallback)TwodSetMarker, NULL);
      gtk_table_attach(GTK_TABLE(temptable), twoddisplayed[i], (i % 4), (i % 4) + 1, (i / 4),
                       (i / 4) + 1, GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                       GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0);
      gtk_widget_show(temptable);
      gtk_widget_hide(twoddisplayed[i]);
    }
  }

  /* --- hide the ones that arn't displayed --- */

  for (i = numtwoddisplayed; i < 16; i++)
    gtk_widget_hide(twoddisplayed[i]);

  /* --- now we interpret the information that we have --- */

  for (i = 0; i < numtwoddisplayed; i++) {
    if (big_data_info[twodspectradisplayed[i]] != NULL) {

      /* --- set the pgam type --- */
      gtk_twodplot_set_plot_mode(GTK_TWODPLOT(twoddisplayed[i]), pgamtype);

      /* --- set the color mode --- */
      gtk_twodplot_set_density_type(GTK_TWODPLOT(twoddisplayed[i]), pgamcolormode);

      /* --- set the interpolation mode --- */
      gtk_twodplot_set_interpolation_type(GTK_TWODPLOT(twoddisplayed[i]), interpolationmode);

      /* --- set the fade mode --- */
      gtk_twodplot_set_fade(GTK_TWODPLOT(twoddisplayed[i]), fade_mode);

      /* --- set the number of contours --- */
      gtk_twodplot_set_num_contours(GTK_TWODPLOT(twoddisplayed[i]), twodnumcontours);

      /* --- set the color depth --- */
      gtk_twodplot_set_color_depth(GTK_TWODPLOT(twoddisplayed[i]), colordepth);

      /* --- set the calibrations --- */
      gtk_twodplot_set_x_calibration(GTK_TWODPLOT(twoddisplayed[i]), twodglobalxcalibs[0],
                                     twodglobalxcalibs[1], twodglobalxcalibs[2]);
      gtk_twodplot_set_y_calibration(GTK_TWODPLOT(twoddisplayed[i]), twodglobalycalibs[0],
                                     twodglobalycalibs[1], twodglobalycalibs[2]);

      /* --- we have to tell the program how to scale the twodplots --- */
      {
        /* --- really local variables --- */
        float xfraction;
        float yfraction;

        /* --- xfraction should be between 1/4 and 1 --- */
        if (numtwoddisplayed <= 4)
          xfraction = .95 / (float)(numtwoddisplayed);
        else
          xfraction = 0.95 * 0.25;

        yfraction = 0.65 / (float)((numtwoddisplayed) / 4 + 1);
        gtk_twodplot_set_frac(GTK_TWODPLOT(twoddisplayed[i]), xfraction, yfraction);
      }

      if (ycurrentrange[1] >= big_data_info[twodspectradisplayed[i]]->axes[1])
        ycurrentrange[1] = big_data_info[twodspectradisplayed[i]]->axes[1] - 1;
      if (xcurrentrange[1] >= big_data_info[twodspectradisplayed[i]]->axes[0])
        xcurrentrange[1] = big_data_info[twodspectradisplayed[i]]->axes[0] - 1;
      if (ycurrentrange[0] < 0)
        ycurrentrange[0] = 0;
      if (xcurrentrange[0] < 0)
        xcurrentrange[0] = 0;
      height = (int)abs(ycurrentrange[1] - ycurrentrange[0]) + 1;
      width = (int)abs(xcurrentrange[1] - xcurrentrange[0]) + 1;
      gtk_twodplot_set_size(GTK_TWODPLOT(twoddisplayed[i]), width, height);
      for (j = ycurrentrange[0]; j <= ycurrentrange[1]; j++) {
        for (k = xcurrentrange[0]; k <= xcurrentrange[1]; k++) {
          gtk_twodplot_set_value(
              GTK_TWODPLOT(twoddisplayed[i]), (k - xcurrentrange[0]), (j - ycurrentrange[0]),
              (float)BigDataStructGetVal(big_data_info[twodspectradisplayed[i]], k, j));
          if (gate_info != NULL)
            if (GateDataStructTest(gate_info, twodspectradisplayed[i], k, j))
              //	    if (BigDataStructGetVal(gate_info,k,j) & (1 << twodspectradisplayed[i]))
              gtk_twodplot_toggle_gate(GTK_TWODPLOT(twoddisplayed[i]), (int)(k - xcurrentrange[0]),
                                       (int)(j - ycurrentrange[0]));
        }
      }
      gtk_twodplot_set_x_range(GTK_TWODPLOT(twoddisplayed[i]), xcurrentrange[0], xcurrentrange[1]);
      gtk_twodplot_set_y_range(GTK_TWODPLOT(twoddisplayed[i]), ycurrentrange[0], ycurrentrange[1]);
      gtk_twodplot_set_title(GTK_TWODPLOT(twoddisplayed[i]), twodtitles[twodspectradisplayed[i]]);
      gtk_twodplot_set_contour_type(GTK_TWODPLOT(twoddisplayed[i]), pgamcontourtype);
      /* --- we ought to re-size to plot widget based on the space
         --- avalable in the window --- */
      //--ddc jun11 replace deprecations of gtk_widget_draw
      //  gtk_widget_draw(twoddisplayed[i],NULL);
      gtk_twodplot_draw(twoddisplayed[i], NULL);

      gtk_widget_show(twoddisplayed[i]);
    }

    twodplot = twoddisplayed[0];
    pgam = twodspectradisplayed[0];
  }
}

/*
 * TwodSetMarker
 *
 * analogous to the histogram function SetMarker, but for two dimensions
 */
void TwodSetMarker(GtkWidget *widget, GdkEventButton *event) {
  float xresult, yresult;
  float width, height;
  long double xcurrentscaling, ycurrentscaling;
  int tempsum, i, j, counts, y, z, x;
  char dummystr[80];
  float slope;

  /* --- when we have more than 1 displayed we need to figure out which one
     --- the signal was sent from --- */
  if (numtwoddisplayed > 1) {
    for (i = 0; i < numtwoddisplayed; i++) {
      if (twoddisplayed[i] == widget) {
        twodplot = twoddisplayed[i];
        pgam = twodspectradisplayed[i];
      }
    }
  } else {
    twodplot = twoddisplayed[0];
    pgam = twodspectradisplayed[0];
  }

  //  printf("Twodplot was clicked.\n");

  /* ------- do some error checking --------- */
  //--ddc 24may06  if (twodplot != NULL) {
  if (GTK_IS_WIDGET(twodplot)) {

    if (xcurrentrange[0] > xcurrentrange[1]) {
      i = xcurrentrange[0];
      xcurrentrange[0] = xcurrentrange[1];
      xcurrentrange[1] = i;
    }

    if (ycurrentrange[0] > ycurrentrange[1]) {
      i = ycurrentrange[0];
      ycurrentrange[0] = ycurrentrange[1];
      ycurrentrange[1] = i;
    }

    height = ycurrentrange[1] - ycurrentrange[0] + 1;
    width = xcurrentrange[1] - xcurrentrange[0] + 1;

    if ((width != 0) && (twodplot->allocation.width != 0)) {
      xresult = ((event->x - 30) / (twodplot->allocation.width - 30) * width) + xcurrentrange[0];
    }
    if ((height != 0) && (twodplot->allocation.height != 0)) {
      yresult = ((twodplot->allocation.height - 30) - event->y) /
                    (twodplot->allocation.height - 30) * height +
                ycurrentrange[0];
    }
    //    if ((twodmarkers[0].x != (int) xresult) &&
    //	(twodmarkers[0].y != (int)yresult)){

    twodmarkers[3].y = twodmarkers[2].y;
    twodmarkers[3].x = twodmarkers[2].x;
    twodmarkers[2].y = twodmarkers[1].y;
    twodmarkers[2].x = twodmarkers[1].x;
    twodmarkers[1].y = twodmarkers[0].y;
    twodmarkers[1].x = twodmarkers[0].x;

    twodmarkers[0].y = (int)yresult;
    twodmarkers[0].x = (int)xresult;
    //}
    /* --- if we need it and we don't have it we need a gate info matrix --- */
    /* --- now for gate "lines" --- */
    if (abs(twodmarkers[0].x - twodmarkers[1].x) != 0) {
      slope = (float)(twodmarkers[0].y - twodmarkers[1].y) /
              (float)(twodmarkers[0].x - twodmarkers[1].x);
    } else {
      slope = 10000000;
    }
    //    printf("slope : %f\n",slope);
    if (drawing_gates) {
      if (gate_info == NULL)
        TwodInitializeGate();
    }
    //--ddc 17jan06 add condition on drawing_gates so we don't use the 'first' point!
    if (gate_info != NULL && drawing_gates > 1) {
      //      printf(" %d \n",abs(twodmarkers[1].x - twodmarkers[0].x + 1));
      y = Min(twodmarkers[0].x, twodmarkers[1].x);
      z = Max(twodmarkers[0].x, twodmarkers[1].x);
      if (twodmarkers[0].x == y)
        x = twodmarkers[0].y;
      else
        x = twodmarkers[1].y;
      for (i = y; i <= z; i++) {
        j = slope * (i - y) + x;
        GateDataStructToggleOn(gate_info, gatetype, i, j);
        //	BigDataStructSetVal(gate_info,i,j,BigDataStructGetVal(gate_info,i,j) | (1 <<
        //gatetype));
      }
      if (abs(twodmarkers[0].y - firstpoint.y) != 0) {
        slope = (float)(twodmarkers[0].x - twodmarkers[1].x) /
                (float)(twodmarkers[0].y - twodmarkers[1].y);
      } else {
        slope = 1000000;
      }
      x = Min(twodmarkers[0].y, twodmarkers[1].y);
      z = Max(twodmarkers[0].y, twodmarkers[1].y);
      if (twodmarkers[0].y == x)
        y = twodmarkers[0].x;
      else
        y = twodmarkers[1].x;
      for (i = x; i <= z; i++) {
        j = slope * (i - x) + y;
        GateDataStructToggleOn(gate_info, gatetype, j, i);
        //	BigDataStructSetVal(gate_info,j,i,BigDataStructGetVal(gate_info,j,i) | (1 <<
        //gatetype));
      }
    }
    sprintf(dummystr, "x: %d, y: %d\n", (twodmarkers[0].x + 1), (twodmarkers[0].y + 1));
    WriteTwodText(dummystr);
    tempsum = 0;
    for (i = twodmarkers[0].x; i < (ybinsize + twodmarkers[0].x); i++) {
      for (j = twodmarkers[0].y; j < (xbinsize + twodmarkers[0].y); j++) {
        tempsum = tempsum + BigDataStructGetVal(big_data_info[pgam], i, j);
      }
    }
    sprintf(dummystr, " counts: %d \n", tempsum);
    WriteTwodText(dummystr);
    TwodDrawMarkers();
  }
}

/*
 * TwodDrawMarkers
 *
 * TwodDrawMarkers is analogous to DrawMarkers, but in 2d
 */
void TwodDrawMarkers() {
  float xresult, yresult;
  float xresult1, yresult1;
  GdkPoint points[2];
  int i, width, height, j, k;
  long double xcurrentscaling, ycurrentscaling;

  height = abs(ycurrentrange[1] - ycurrentrange[0]) + 1;
  width = abs(xcurrentrange[1] - xcurrentrange[0]) + 1;

  if ((width != 0) && (height != 0)) {
    xresult = ((float)twodmarkers[0].x - (float)xcurrentrange[0] + (float)0.5) / (float)width *
                  (float)(twodplot->allocation.width - 30) +
              30;
    yresult = (float)(twodplot->allocation.height - 30) -
              ((float)twodmarkers[0].y - (float)ycurrentrange[0] + (float)0.5) / (float)height *
                  (float)(twodplot->allocation.height - 30);
    if (!(drawing_gates)) {
      points[0].x = points[1].x = xresult;
      points[0].y = 0;
      points[1].y = twodplot->allocation.height - 30;
      gdk_draw_lines(twodplot->window, twodplot->style->white_gc, points, 2);
      //    printf ("tried to draw a line from (%d,%d) to (%d,%d).\n",
      //	    points[0].x,points[0].y,points[1].x,points[1].y);
      points[0].y = points[1].y = yresult;
      points[0].x = 30;
      points[1].x = twodplot->allocation.width;
      gdk_draw_lines(twodplot->window, twodplot->style->white_gc, points, 2);
      //    printf ("tried to draw a line from (%d,%d) to (%d,%d).\n",
      //	    points[0].x,points[0].y,points[1].x,points[1].y);
    } else { // draw lines if we are setting bannana gates
      //--ddc first gate... do nothing but record start, added test
      //--ddc and set of firstpoint, to original 'else'
      if (drawing_gates == 1) {
        firstpoint.x = twodmarkers[0].x;
        firstpoint.y = twodmarkers[0].y;
        drawing_gates++;
      } else {

        xresult1 = ((float)twodmarkers[1].x - (float)xcurrentrange[0] + (float)0.5) / (float)width *
                       (float)(twodplot->allocation.width - 30) +
                   30;
        yresult1 = (float)(twodplot->allocation.height - 30) -
                   ((float)twodmarkers[1].y - (float)ycurrentrange[0] + (float)0.5) /
                       (float)height * (float)(twodplot->allocation.height - 30);
        points[0].x = xresult;
        points[0].y = yresult;
        points[1].x = xresult1;
        points[1].y = yresult1;
        gdk_draw_lines(twodplot->window, twodplot->style->white_gc, points, 2);
      }
    }
  } else
    GetMessageDialog("Error displaying markers.");
}

/* copies gdkpoints */
void gdk_point_copy(GdkPoint point1, GdkPoint *point2) {
  point2->x = point1.x;
  point2->y = point1.y;
}

/* PgamMatrix2Big
 *
 * Does an internal conversion of the pgammatrix data to the internal big structure for display from
 * "above"
 */
void PgamMatrix2Big() {
  int i, j, k, l;
  int new_size, compression_factor;
  int tempsum;
  int isize, ind;
  int ix, iy;
  int totalsize;

  totalsize = 1000;

  //--ddc if nothing has been done, then for crying out loud let us do nothing. next line returns
  if (pgammatrixdata.data == NULL)
    return;

  /* --- let's figure out how big we should make the appropriate "big" struct --- */
  compression_factor = pgammatrixdata.size / totalsize;
  if (compression_factor < ((float)pgammatrixdata.size / (float)totalsize))
    compression_factor++;
  new_size = (float)pgammatrixdata.size / (float)compression_factor;

  /* --- now we need to make a new big data struct --- */
  //--ddc 8jun06 the logic in adjusting pgammax to 1 if big_data_info is NULL escapes me.  If this
  //--ddc 8jun06 is EVER true, you are in big trouble!!! Just increment pgammax!
  //  if (big_data_info == NULL) {
  //    pgammax = 1;
  //  } else {
  //    pgammax++;
  //  }

  pgammax++;

  //--ddc 8jun06 ... AND why would you make it one bigger than pgammax?!  this is the only reason
  //this bug has persisted
  //--ddc 8jun06 for as long as it has....
  //  big_data_info = (struct bigdata **) realloc(big_data_info,
  //					      sizeof(struct bigdata *) * (pgammax + 1));

  big_data_info = (struct bigdata **)realloc(big_data_info, sizeof(struct bigdata *) * pgammax);
  big_data_info[pgammax - 1] = BigDataStructNew(new_size, new_size);

  switch (pgammatrixdata.type) {
  case 0:
    /* --- square --- */
    for (i = 0; i < new_size; i++) {
      for (j = 0; j < new_size; j++) {
        tempsum = 0;
        for (k = 0; k < compression_factor; k++) {
          for (l = 0; l < compression_factor; l++) {
            tempsum += *(pgammatrixdata.data + pgammatrixdata.size * (i * compression_factor + k) +
                         (j * compression_factor + l));
          }
        }
        BigDataStructSetVal(big_data_info[pgammax - 1], i, j, tempsum);
      }
    }
    break;
  case 3:
    /* --- doesn't really make too much sense to do this to a triangle,
       --- but what the hell... --- */
    isize = 2 * pgammatrixdata.size + 1;
    for (i = 0; i < new_size; i++) {
      for (j = 0; j < new_size; j++) {
        tempsum = 0;
        for (k = 0; k < compression_factor; k++) {
          for (l = 0; l < compression_factor; l++) {
            ix = Max(i * compression_factor + k, j * compression_factor + l);
            iy = Min(i * compression_factor + k, j * compression_factor + l);
            ind = ix - iy + (isize - iy) * ((float)iy / (float)2);
            tempsum += *(pgammatrixdata.data + ind);
            if (ix == iy)
              tempsum += *(pgammatrixdata.data + ind);
          }
        }
        BigDataStructSetVal(big_data_info[pgammax - 1], i, j, tempsum);
      }
    }
    break;
  default:
    /* --- do nothing --- */
    break;
  }

  /* --- display the monstrosity --- */
  DisplayBig();
}

/*
 * DisplayBig
 *
 * displays the contents of "big" in it's own window
 * this is also where the handy functions get tied to it.
 */
void DisplayBig() {
  int i, j, k;

  int numvals;
  GdkCursor *cursor;
  GdkBitmap *mask;
  GdkBitmap *bitmap;
  GtkAccelGroup *local_accel_group;
  //  GtkItemFactory *local_item_factory;
  GtkActionGroup *action_group;
  GtkUIManager *menu_manager;
  //--ddc aug11 gtk dep  GtkWidget *hscrollbar,*vscrollbar,*table;
  GtkWidget *table;
  GtkWidget *localwindow;

  //--ddc unused..  GtkWidget *button;

  //  printf("Attempting to Display Big.\n");

  /* --- make the local window for the twodplot --- */

  pgam = 0;

  g_return_if_fail(big_data_info != NULL);

  /* --- let's initialize the calibrations while we are here --- */
  twodglobalxcalibs[0] = twodglobalycalibs[0] = 0;
  twodglobalxcalibs[1] = twodglobalycalibs[1] = 1;
  twodglobalxcalibs[2] = twodglobalycalibs[2] = 0;

  //--ddc 24may06   if (Window2D == NULL) {
  if (GTK_IS_WIDGET(Window2D))
    return; //--ddc aug11 cleanup.

  Window2D = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title((GtkWindow *)Window2D, "Gnuscope PGAM");
  /* --- make it so that the window can be shrunk --- */
  gtk_window_set_policy(GTK_WINDOW(Window2D), 1, 1, 1);

  //    g_signal_connect_after(GTK_OBJECT(Window2D),"key_press_event",
  g_signal_connect(GTK_OBJECT(Window2D), "key_press_event", (GCallback)TwodGateSet, NULL);

  //--ddc aug11    gtk_signal_connect(GTK_OBJECT(Window2D),"delete_event",
  g_signal_connect(GTK_OBJECT(Window2D), "delete_event", G_CALLBACK(CloseTwodPlotWindow), NULL);

  //--ddc this was creating exceptions when we created and added twovbox
  // each time so I've moved it in this "if"

  /* --- create the local vbox so we can have more than one element in the display --- */

  twodvbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(Window2D), twodvbox);

  /* --- attach the item factory and insert it into the vbox --- */
  //--ddc aug11 (get this from uimanager)
  //            local_accel_group = gtk_accel_group_new();

  {
    GError *error = NULL;
    action_group = gtk_action_group_new("TwodActions");
    gtk_action_group_set_translation_domain(action_group, "blah");

    for (i = 0; i < twodplot_nmenu_items; i++) {
      gtk_action_group_add_actions(action_group, &twodplot_menu_items[i].gtkentry, 1,
                                   &twodplot_menu_items[i].userdata);
    }

    menu_manager = gtk_ui_manager_new();
    gtk_ui_manager_insert_action_group(menu_manager, action_group, 0);
    gtk_ui_manager_add_ui_from_string(menu_manager, twodplot_menu_ui, -1, &error);

    if (error) {
      g_message("building menus failed: %s", error->message);
      g_error_free(error);
    }
    local_accel_group = gtk_ui_manager_get_accel_group(menu_manager);
  }

  // gtk2 deprecated  gtk_accel_group_attach(local_accel_group,GTK_OBJECT(Window2D));
  gtk_window_add_accel_group(GTK_OBJECT(Window2D), local_accel_group);
  // gtk2 deprecated
  /*
  gtk_box_pack_start(GTK_BOX(twodvbox),
                     gtk_item_factory_get_widget(local_item_factory,"<blah>"),
                     FALSE,FALSE,0);
  */

  gtk_box_pack_start(GTK_BOX(twodvbox), gtk_ui_manager_get_widget(menu_manager, "/MainMenu"), FALSE,
                     FALSE, 0);

  /* --- set the current range --- */
  xcurrentrange[0] = 0;
  ycurrentrange[0] = 0;
  xcurrentrange[1] = big_data_info[pgam]->axes[0] - 1;
  ycurrentrange[1] = big_data_info[pgam]->axes[1] - 1;

  /* --- text box --- */
  /*
  twodtext = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(twodtext),GTK_WRAP_WORD);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(twodtext),FALSE);
  */
  twodtext = init_text_view();
  //--ddc aug11 gtk deprecations.
  {
    int x, y;
    PangoLayout *layout; // we only need this temporarily, for sizing..
    layout = gtk_widget_create_pango_layout(Window2D, NULL);
    localwindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(localwindow), twodtext);
    pango_layout_get_pixel_size(layout, &x, &y);
    gtk_widget_set_size_request(localwindow, -1, 10 * y);

    g_object_unref(layout);
  }

  gtk_container_add(GTK_CONTAINER(twodvbox), localwindow);

  gtk_widget_show(twodtext);
  gtk_widget_show(localwindow);

  /* --- show the window --- */

  gtk_widget_show(twodvbox);

  /* --- set the window policies and setermine where it shows up --- */

  gtk_window_set_policy(GTK_WINDOW(Window2D), 0, 1, 1);
  gtk_window_set_position(GTK_WINDOW(Window2D), GTK_WIN_POS_CENTER);

  gtk_widget_show_all(Window2D);

  /* --- cursor --- */
  // create_bitmap_and_mask_from_xpm (&bitmap,&mask,cursor_hairs);
  cursor = gdk_cursor_new(GDK_CROSSHAIR);
  gdk_window_set_cursor(Window2D->window, cursor);

  WriteTwodText("Gnuscope 2D display.\n");
  /* --- we don't need the initialize the gtk main loop, or
   * implement the main loop becuase we are already in the program
   * if this is moved to an independant program the following lines must be added
   * to the "main" routine:
   *
   * at the beginning(before any gtk functions are called):  gtk_init(&argc,&argv);
   * at the end (after all functions/subroutines are called); gtk_main();
   */
}

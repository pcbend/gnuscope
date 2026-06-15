#include <gtk/gtk.h>
//--ddc feb15.  sadly all these are "callback" functions, and they
// were implemented DIFFERENTLY for gtk+1 the 'callback_data' was not
// used by JP, but IS by the new callbacks.  Eliminate call_back_data
// EVERYWHERE, and replace it with &callback_action (which HAS the right
// data type for the the callback).  Also, the ORDER
// of the arguements is REVERSED for all these functions.  Enjoy.

/* --- Must declare all functions used in menu here --- */

static void MenuTwodRefresh(GtkWidget *widget, guint *callback_action);
static void MenuTwodExpand(GtkWidget *widget, guint *callback_action);
static void MenuTwodZoomOut(GtkWidget *widget, guint *callback_action);
static void MenuTwodWriteBig(GtkWidget *widget, guint *callback_action);
static void MenuTwodReadBig(GtkWidget *widget, guint *callback_action);
static void MenuTwodClearGates(GtkWidget *widget, guint *callback_action);
static void MenuTwodReadGates(GtkWidget *widget, guint *callback_action);
static void MenuTwodWriteGates(GtkWidget *widget, guint *callback_action);
static void MenuTwodClearGates(GtkWidget *widget, guint *callback_action);
static void MenuTwodRedraw(GtkWidget *widget, guint *callback_action);
static void MenuTwodGateFill(GtkWidget *widget, guint *callback_action);
static void MenuPgamReadSetup(GtkWidget *widget, guint *callback_action);
static void MenuPgamReadMasterSetup(GtkWidget *widget, guint *callback_action);
static void MenuPgamSort(GtkWidget *widget, guint *callback_action);
static void MenuPrevTwod(GtkWidget *widget, guint *callback_action);
static void MenuNextTwod(GtkWidget *widget, guint *callback_action);
static void MenuPgamReadPgamTwod(GtkWidget *widget, guint *callback_action);
static void MenuPgamWritePgamTwod(GtkWidget *widget, guint *callback_action);
static void MenuPgamTwodProjectFull(GtkWidget *widget, guint *callback_action);
static void MenuTwodMaxOverRideDown(GtkWidget *widget, guint *callback_action);
static void MenuTwodMaxOverRideUp(GtkWidget *widget, guint *callback_action);
static void MenuPgamDisplayContourAndDensity(GtkWidget *widget, guint *callback_action);
static void MenuPgamDisplayDensity(GtkWidget *widget, guint *callback_action);
static void MenuPgamDisplayContour(GtkWidget *widget, guint *callback_action);
static void MenuPgamSetNumContours(GtkWidget *widget, guint *callback_action);
static void MenuPgamPrint(GtkWidget *widget, guint *callback_action);
static void MenuPgamWritePostscript(GtkWidget *widget, guint *callback_action);
static void MenuPgamInverseRootContours(GtkWidget *widget, guint *callback_action);
static void MenuPgamInverseLogContours(GtkWidget *widget, guint *callback_action);
static void MenuPgamLogContours(GtkWidget *widget, guint *callback_action);
static void MenuPgamLinearContours(GtkWidget *widget, guint *callback_action);
static void MenuPgamRootContours(GtkWidget *widget, guint *callback_action);
static void MenuPgamSetTwodTitle(GtkWidget *widget, guint *callback_action);
static void MenuPgamSelect2D(GtkWidget *widget, guint *callback_action);
static void MenuPgamClear2D(GtkWidget *widget, guint *callback_action);
static void MenuPgamAdd2D(GtkWidget *widget, guint *callback_action);
static void MenuPgamSubtract2D(GtkWidget *widget, guint *callback_action);
static void MenuPgamColorDensity(GtkWidget *widget, guint *callback_action);
static void MenuPgamBWDensity(GtkWidget *widget, guint *callback_action);
static void MenuTwodReadFile(GtkWidget *widget, guint *callback_action);
static void MenuPgamSetInterpolation(GtkWidget *widget, guint *callback_action);
static void MenuPgamSetColorMode(GtkWidget *widget, guint *callback_action);
static void MenuPgamSetContourType(GtkWidget *widget, guint *callback_action);
static void MenuPgamSetType(GtkWidget *widget, guint *callback_action);
static void MenuXCalibration(GtkWidget *widget, guint *callback_action);
static void MenuYCalibration(GtkWidget *widget, guint *callback_action);
static void MenuPgamSetFadeMode(gpointer(callback_data), guint *callback_action);

/*
 * Structure to build the twodplot menus
 */

struct myActions {
  GtkActionEntry gtkentry;
  int userdata;
};

/*--ddc gtk deprecation of the 'gtkitemfactory'.  The 'new' way of creating
menus is _close_ to the itemfactory (if you use the GtkActionEntry structure,
and the 'gtk_action_group_add_actions' function), but unfortunately the developers
chose put the user_data in a separate array (rather than in the structure!).  This
invites misalignment, and I think makes it harder to appreciate how J.Pavan was
doing some of his menus.  It will look clunky when I load it in one element at
a time, but I think this is most straightforward way.
*/

/*  GtkActionEntry , user_data... ie:
  {name, stock_id, label, accelerator, tooltip, callback},{int user_data}
*/

/*
 * Structure to build the twodplot menus
 */

static struct myActions twodplot_menu_items[] = {
    {{"FileMenuAction", NULL, "_File", NULL, NULL, NULL}, 0},
    {{"FileReadAction", NULL, "_Read", NULL, NULL, G_CALLBACK(MenuTwodReadFile)}, 0},
    {{"FileWriteAction", NULL, "_Write Big", NULL, NULL, G_CALLBACK(MenuTwodWriteBig)}, 0},
    {{"FilePrintAction", NULL, "Print", "<control>P", NULL, G_CALLBACK(MenuPgamPrint)}, 0},
    {{"FilePrintFileAction", NULL, "Print To File", NULL, NULL,
      G_CALLBACK(MenuPgamWritePostscript)},
     0},

    {{"ManipMenuAction", NULL, "_Manipulate", NULL, NULL, NULL}, 0},
    {{"ManipTitleAction", NULL, "Set Title", "<control>T", NULL, G_CALLBACK(MenuPgamSetTwodTitle)},
     0},
    {{"ManipClearAction", NULL, "Clear 2D", "<control>C", NULL, G_CALLBACK(MenuPgamClear2D)}, 0},
    {{"ManipAddAction", NULL, "Add 2D", "<control>A", NULL, G_CALLBACK(MenuPgamAdd2D)}, 0},
    {{"ManipSubAction", NULL, "Subtract 2D", "<control>S", NULL, G_CALLBACK(MenuPgamSubtract2D)},
     0},

    {{"DisplayMenuAction", NULL, "_Display", NULL, NULL, NULL}, 0},
    {{"DisplayDispAction", NULL, "Display", "D", NULL, G_CALLBACK(MenuPgamSelect2D)}, 0},
    {{"DisplayXCalAction", NULL, "X Calibration", "X", NULL, G_CALLBACK(MenuXCalibration)}, 0},
    {{"DisplayYCalAction", NULL, "Y Calibration", "Y", NULL, G_CALLBACK(MenuYCalibration)}, 0},
    {{"DisplayRefreshAction", NULL, "_Refresh", "N", NULL, G_CALLBACK(MenuTwodRefresh)}, 0},
    {{"DisplayExpandAction", NULL, "_Expand", "E", NULL, G_CALLBACK(MenuTwodExpand)}, 0},
    {{"DisplayOutAction", NULL, "Zoom _Out", "O", NULL, G_CALLBACK(MenuTwodZoomOut)}, 0},

    {{"DisplayRedrawAction", NULL, "Re_Draw", "R", NULL, G_CALLBACK(MenuTwodRedraw)}, 0},
    {{"DisplayNextAction", NULL, "Next TwoD", NULL, NULL, G_CALLBACK(MenuNextTwod)}, 0},
    {{"DisplayPrevAction", NULL, "Prev TwoD", NULL, NULL, G_CALLBACK(MenuPrevTwod)}, 0},
    {{"DisplayContUpAction", NULL, "Contrast Up", "C", NULL, G_CALLBACK(MenuTwodMaxOverRideUp)}, 0},
    {{"DisplayContDownAction", NULL, "Contrast Down", "<shift>C", NULL,
      G_CALLBACK(MenuTwodMaxOverRideDown)},
     0},
    {{"DisplayContAction", NULL, "Num Contours", NULL, NULL, G_CALLBACK(MenuPgamSetNumContours)},
     0},

    {{"DisplayScaleMenuAction", NULL, "Scale Mode", NULL, NULL, NULL}, 0},

    {{"DisplayLinearAction", NULL, "Linear", NULL, NULL, G_CALLBACK(MenuPgamSetContourType)}, 0},
    {{"DisplayLogAction", NULL, "Log", NULL, NULL, G_CALLBACK(MenuPgamSetContourType)}, 1},
    {{"DisplayRootAction", NULL, "Root", NULL, NULL, G_CALLBACK(MenuPgamSetContourType)}, 2},
    {{"DisplayInvLogAction", NULL, "Inverse Log", NULL, NULL, G_CALLBACK(MenuPgamSetContourType)},
     3},
    {{"DisplayInvRootAction", NULL, "Inverse Root", NULL, NULL, G_CALLBACK(MenuPgamSetContourType)},
     4},

    {{"DisplayColorMenuAction", NULL, "Color Mode", NULL, NULL, NULL}, 0},

    {{"DisplayColorAction", NULL, "Color", NULL, NULL, G_CALLBACK(MenuPgamSetColorMode)}, 0},
    {{"DisplayBWAction", NULL, "BW", NULL, NULL, G_CALLBACK(MenuPgamSetColorMode)}, 1},

    {{"DisplayPlotMenuAction", NULL, "Plot Type", NULL, NULL, NULL}, 0},
    {{"DisplayDensityAction", NULL, "Density", NULL, NULL, G_CALLBACK(MenuPgamSetType)}, 1},
    {{"DisplayContourAction", NULL, "Contour", NULL, NULL, G_CALLBACK(MenuPgamSetType)}, 0},
    {{"DisplayDandCAction", NULL, "Density and Contour", NULL, NULL, G_CALLBACK(MenuPgamSetType)},
     2},

    {{"DisplayInterpMenuAction", NULL, "Interpolation Mode", NULL, NULL, NULL}, 0},
    {{"DisplayInterpOffAction", NULL, "Off", NULL, NULL, G_CALLBACK(MenuPgamSetInterpolation)}, 0},
    {{"DisplayInterpLinAction", NULL, "Linear", NULL, NULL, G_CALLBACK(MenuPgamSetInterpolation)},
     1},
    {{"DisplayInterpQuadAction", NULL, "Quadratic", NULL, NULL,
      G_CALLBACK(MenuPgamSetInterpolation)},
     2},
    {{"DisplayInterpCubAction", NULL, "Cubic", NULL, NULL, G_CALLBACK(MenuPgamSetInterpolation)},
     3},
    {{"DisplayInterpFourAction", NULL, "Four", NULL, NULL, G_CALLBACK(MenuPgamSetInterpolation)},
     4},
    {{"DisplayInterpFiveAction", NULL, "Five", NULL, NULL, G_CALLBACK(MenuPgamSetInterpolation)},
     5},
    {{"DisplayInterpSixAction", NULL, "Six", NULL, NULL, G_CALLBACK(MenuPgamSetInterpolation)}, 6},
    {{"DisplayInterpSevenAction", NULL, "Seven", NULL, NULL, G_CALLBACK(MenuPgamSetInterpolation)},
     7},
    {{"DisplayInterpEightAction", NULL, "Eight", NULL, NULL, G_CALLBACK(MenuPgamSetInterpolation)},
     8},

    {{"DisplayFadeMenuAction", NULL, "Fade Mode", NULL, NULL, NULL}, 0},
    {{"DisplayFadeOnAction", NULL, "On", NULL, NULL, G_CALLBACK(MenuPgamSetFadeMode)}, 0},
    {{"DisplayFadeOffAction", NULL, "Off", NULL, NULL, G_CALLBACK(MenuPgamSetFadeMode)}, 1},

    {{"GateMenuAction", NULL, "Gates", NULL, NULL, NULL}, 0},
    {{"GateReadAction", NULL, "_Read Gate", NULL, NULL, G_CALLBACK(MenuTwodReadGates)}, 0},
    {{"GateWriteAction", NULL, "_Write Gates", NULL, NULL, G_CALLBACK(MenuTwodWriteGates)}, 0},
    {{"GateClearAction", NULL, "_Clear Gates", NULL, NULL, G_CALLBACK(MenuTwodClearGates)}, 0},

    {{"SortMenuAction", NULL, "_Sort", NULL, NULL, NULL}, 0},
    {{"SortPgamAction", NULL, "Pgam _Sort", NULL, NULL, G_CALLBACK(MenuPgamSort)}, 0},

};

/*
note on syntax... Putting in a long string in a C initialization requires 'escaping' """
with the "\" character.  Also each line needs explicit linefeed added '\n' AND a '\' for
a continuation.
*/
static char *twodplot_menu_ui = " \n\
<ui>\n\
  <menubar name=\"MainMenu\"> \n\
  <menu action=\"FileMenuAction\"> \n\
      <menuitem action=\"FileReadAction\"  /> \n\
      <menuitem action=\"FileWriteAction\" /> \n\
      <menuitem action=\"FilePrintAction\" /> \n\
      <menuitem action=\"FilePrintFileAction\" /> \n\
  </menu> \n\
  <menu action=\"ManipMenuAction\"> \n\
      <menuitem action=\"ManipTitleAction\" /> \n\
      <menuitem action=\"ManipClearAction\" /> \n\
      <menuitem action=\"ManipAddAction\" /> \n\
      <menuitem action=\"ManipSubAction\" /> \n\
  </menu> \n\
  <menu action=\"DisplayMenuAction\"> \n\
      <menuitem action=\"DisplayDispAction\" /> \n\
      <menuitem action=\"DisplayXCalAction\" /> \n\
      <menuitem action=\"DisplayYCalAction\" /> \n\
      <menuitem action=\"DisplayRefreshAction\" /> \n\
      <menuitem action=\"DisplayExpandAction\" /> \n\
      <menuitem action=\"DisplayOutAction\" /> \n\
      <menuitem action=\"DisplayRedrawAction\" /> \n\
      <menuitem action=\"DisplayNextAction\" /> \n\
      <menuitem action=\"DisplayPrevAction\" /> \n\
      <menuitem action=\"DisplayContUpAction\" /> \n\
      <menuitem action=\"DisplayContDownAction\" /> \n\
      <menuitem action=\"DisplayContAction\" /> \n\
      <menu action=\"DisplayScaleMenuAction\"> \n\
          <menuitem action=\"DisplayLinearAction\" /> \n\
          <menuitem action=\"DisplayLogAction\" /> \n\
          <menuitem action=\"DisplayRootAction\" /> \n\
          <menuitem action=\"DisplayInvLogAction\" /> \n\
          <menuitem action=\"DisplayInvRootAction\" /> \n\
      </menu> \n\
      <menu action=\"DisplayColorMenuAction\"> \n\
          <menuitem action=\"DisplayColorAction\" /> \n\
          <menuitem action=\"DisplayBWAction\" /> \n\
      </menu> \n\
      <menu action=\"DisplayPlotMenuAction\"> \n\
          <menuitem action=\"DisplayDensityAction\" /> \n\
          <menuitem action=\"DisplayContourAction\" /> \n\
          <menuitem action=\"DisplayDandCAction\"  /> \n\
      </menu> \n\
      <menu action=\"DisplayInterpMenuAction\"> \n\
          <menuitem action=\"DisplayInterpOffAction\" /> \n\
          <menuitem action=\"DisplayInterpLinAction\" /> \n\
          <menuitem action=\"DisplayInterpQuadAction\" /> \n\
          <menuitem action=\"DisplayInterpCubAction\" /> \n\
          <menuitem action=\"DisplayInterpFourAction\" /> \n\
          <menuitem action=\"DisplayInterpFiveAction\" /> \n\
          <menuitem action=\"DisplayInterpSixAction\" /> \n\
          <menuitem action=\"DisplayInterpSevenAction\" /> \n\
          <menuitem action=\"DisplayInterpEightAction\" /> \n\
      </menu> \n\
      <menu action=\"DisplayFadeMenuAction\"> \n\
          <menuitem action=\"DisplayFadeOnAction\" /> \n\
          <menuitem action=\"DisplayFadeOffAction\" /> \n\
      </menu> \n\
  </menu> \n\
  <menu action=\"GateMenuAction\"> \n\
      <menuitem action=\"GateReadAction\" /> \n\
      <menuitem action=\"GateWriteAction\" /> \n\
      <menuitem action=\"GateClearAction\" /> \n\
  </menu> \n\
  <menu action=\"SortMenuAction\"> \n\
      <menuitem action=\"SortPgamAction\" /> \n\
  </menu>  \n\
</menubar> \n\
</ui>      \n\
";

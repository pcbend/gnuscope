#include <gtk/gtk.h>
//--ddc feb15.  sadly all these are "callback" functions, and they were implemented DIFFERENTLY for
//gtk+1 the 'callback_data' was not used by JP, but IS by the new callbacks.  Eliminate
// call_back_data EVERYWHERE, and replace it with &callback_action (which HAS the right data type
// for the the callback).  Also, the ORDER of the arguements is REVERSED for all these functions.
// Enjoy.
/* --- Must declare all functions used in menu here --- */

//--ddc aug11 degraded static void ShowMenu (gpointer callback_data, guint callback_action,
//GtkWidget *widget);
static void QuitApp(GtkWidget *widget, guint *callback_action);
//--ddc aug11 degraded static void SetType (GtkWidget *widget, guint callback_action);
static void SetPlotType(GtkWidget *widget, guint *callback_action);
static void MenuReadFile(GtkWidget *widget, guint *callback_action);
static void MenuWriteFile(GtkWidget *widget, guint *callback_action);
static void MenuDisplayPlot(GtkWidget *widget, guint *callback_action);
static void MenuDisplayPlotSameRange(GtkWidget *widget, guint *callback_action);
static void MenuExpandPlot(GtkWidget *widget, guint *callback_action);
static void MenuManualMarkSet(GtkWidget *widget, guint *callback_action);
static void MenuShiftLeft(GtkWidget *widget, guint *callback_action);
static void MenuShiftRight(GtkWidget *widget, guint *callback_action);
static void MenuNextSpectra(GtkWidget *widget, guint *callback_action);
static void MenuPrevSpectra(GtkWidget *widget, guint *callback_action);
static void MenuSetYScale(GtkWidget *widget, guint *callback_action);
static void MenuYScaleUp(GtkWidget *widget, guint *callback_action);
static void MenuYScaleDown(GtkWidget *widget, guint *callback_action);
static void MenuSetBackground(GtkWidget *widget, guint *callback_action);
static void MenuManualSetBackground(GtkWidget *widget, guint *callback_action);
static void MenuOneChSetBackground(GtkWidget *widget, guint *callback_action);
static void MenuZoomOut(GtkWidget *widget, guint *callback_action);
static void MenuGetCalibration(GtkWidget *widget, guint *callback_action);
static void MenuManualExpandPlot(GtkWidget *widget, guint *callback_action);
static void MenuGetSum(GtkWidget *widget, guint *callback_action);
static void MenuAddSpectra(GtkWidget *widget, guint *callback_action);
static void MenuMoveSpectra(GtkWidget *widget, guint *callback_action);
static void MenuCompressSpectra(GtkWidget *widget, guint *callback_action);
static void MenuTitleSpectra(GtkWidget *widget, guint *callback_action);
static void MenuSubtractSpectra(GtkWidget *widget, guint *callback_action);
static void MenuNormalizeSpectra(GtkWidget *widget, guint *callback_action);
static void MenuGainshiftSpectra(GtkWidget *widget, guint *callback_action);
static void MenuGaussFit(GtkWidget *widget, guint *callback_action);
static void MenuClearSpectra(GtkWidget *widget, guint *callback_action);
static void MenuRedraw(GtkWidget *widget, guint *callback_action);
static void MenuDoubleGaussFit(GtkWidget *widget, guint *callback_action);
static void MenuActiveUp(GtkWidget *widget, guint *callback_action);
static void MenuActiveDown(GtkWidget *widget, guint *callback_action);
static void MenuFixedWidthGaussFit(GtkWidget *widget, guint *callback_action);
static void MenuLiphaFile(GtkWidget *widget, guint *callback_action);
static void MenuReadGS(GtkWidget *widget, guint *callback_action);
static void MenuWriteGS(GtkWidget *widget, guint *callback_action);
static void MenuProject(GtkWidget *widget, guint *callback_action);
static void MenuProjectFull(GtkWidget *widget, guint *callback_action);
static void MenuManualProject(GtkWidget *widget, guint *callback_action);
static void MenuSetup(GtkWidget *widget, guint *callback_action);
static void MenuSort(GtkWidget *widget, guint *callback_action);
static void MenuHelp(GtkWidget *widget, guint *callback_action);
static void MenuAbout(GtkWidget *widget, guint *callback_action);
// static void MenuListBetweenMarkers (GtkWidget *widget, guint callback_action);
static void MenuReadBig(GtkWidget *widget, guint *callback_action);
static void MenuSetEfficiency(GtkWidget *widget, guint *callback_action);
static void MenuDisplayBig(GtkWidget *widget, guint *callback_action);
static void MenuReadPGAMSetup(GtkWidget *widget, guint *callback_action);
static void MenuReadPGAMMasterSetup(GtkWidget *widget, guint *callback_action);
static void MenuPGAMSort(GtkWidget *widget, guint *callback_action);

static void MenuWritePgamTwod(GtkWidget *widget, guint *callback_action);
static void MenuReadPgamTwod(GtkWidget *widget, guint *callback_action);
static void MenuTwodProjectFull(GtkWidget *widget, guint *callback_action);
static void MenuProjectionBox(GtkWidget *widget, guint *callback_action);
static void MenuWriteBig(GtkWidget *widget, guint *callback_action);
// static void MenuReadStop (gpointer callback_data, guint callback_action);
// static void MenuMakeFeedingWindow (gpointer callback_data,
//				   guint callback_action, GtkWidget *widget);
// static void MenuReadFitsSetup (gpointer callback_data, guint callback_action, GtkWidget *widget);
// static void MenuFits (gpointer callback_data, guint callback_action, GtkWidget *widget);
static void MenuMultipleToSingleFast(GtkWidget *widget, guint *callback_action);
static void MenuExpFit(GtkWidget *widget, guint *callback_action);
static void MenuPeakFit(GtkWidget *widget, guint *callback_action);
static void MenuReadCalibInfo(GtkWidget *widget, guint *callback_action);
static void MenuPrint(GtkWidget *widget, guint *callback_action);
static void MenuPrintToFile(GtkWidget *widget, guint *callback_action);
static void MenuMultiInOne(GtkWidget *widget, guint *callback_action);
static void MenuPgamMatrix2Big(GtkWidget *widget, guint *callback_action);
static void MenuSpecULator(GtkWidget *widget, guint *callback_action);
static void MenuAutoReadMatrix(GtkWidget *widget, guint *callback_action);
static void MenuAutoWriteMatrix(GtkWidget *widget, guint *callback_action);
static void MenuReadPgamAdd(GtkWidget *widget, guint *callback_action);
static void MenuReadPgamSub(GtkWidget *widget, guint *callback_action);
static void MenuSetYScaleMode(GtkWidget *widget, guint *callback_action);
static void MenuWriteRadwareMatrix(GtkWidget *widget, guint *callback_action);
static void MenuCompress2(GtkWidget *widget, guint *callback_action);
static void MenuSetBackgroundPoly(GtkWidget *widget, guint *callback_action);

/*
 * Structure to build the menus
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

NOTE: Any additions/subtractions require appropriate changes to the gtkactionentries,
AND the menus_ui string below...
*/

/*  GtkActionEntry , user_data... ie:
  {name, stock_id, label, accelerator, tooltip, callback},{int user_data}
*/

static struct myActions menu_items[] = {
    {{"FileMenuAction", NULL, "_File", NULL, NULL, NULL}, 0},
    {{"FileReadAction", NULL, "_Read", "R", NULL, G_CALLBACK(MenuReadFile)}, 0},
    {{"FileWriteAction", NULL, "_Write Histograms", "W", NULL, G_CALLBACK(MenuWriteFile)}, 0},

    {{"FileWLiphaAction", NULL, "Write _Lipha", "L", NULL, G_CALLBACK(MenuLiphaFile)}, 0},
    {{"FileW2DAction", NULL, "Write _2D Histograms", NULL, NULL, G_CALLBACK(MenuWriteBig)}, 0},
    {{"FileWMatrixAction", NULL, "Write _Matrix", NULL, NULL, G_CALLBACK(MenuAutoWriteMatrix)}, 0},
    {{"FileCompMulAction", NULL, "Compress MUL", NULL, NULL, G_CALLBACK(MenuCompress2)}, 0},
    {{"FilePrintAction", NULL, "_Print", "<control>P", NULL, G_CALLBACK(MenuPrint)}, 0},
    {{"FilePrintFileAction", NULL, "Print to _File", NULL, NULL, G_CALLBACK(MenuPrintToFile)}, 0},
    {{"FileQuitAction", NULL, "_Quit", "<control>Q", NULL, G_CALLBACK(QuitApp)}, 0},

    {{"ManipMenuAction", NULL, "_Manipulate", NULL, NULL, NULL}, 0},
    {{"ManipAddAction", NULL, "_Add", "<control>A", NULL, G_CALLBACK(MenuAddSpectra)}, 0},
    {{"ManipCompressAction", NULL, "_Compress", "<control>C", NULL,
      G_CALLBACK(MenuCompressSpectra)},
     0},
    {{"ManipGainAction", NULL, "_Gainshift", "<control>G", NULL, G_CALLBACK(MenuGainshiftSpectra)},
     0},
    {{"ManipMoveAction", NULL, "_Move", "<control>M", NULL, G_CALLBACK(MenuMoveSpectra)}, 0},
    {{"ManipNormAction", NULL, "_Normalize", "<control>N", NULL, G_CALLBACK(MenuNormalizeSpectra)},
     0},
    {{"ManipSubAction", NULL, "_Subtract", "<control>S", NULL, G_CALLBACK(MenuSubtractSpectra)}, 0},
    {{"ManipTitleAction", NULL, "_Title", "<control>T", NULL, G_CALLBACK(MenuTitleSpectra)}, 0},
    {{"ManipClearAction", NULL, "_Clear", "<control>C", NULL, G_CALLBACK(MenuClearSpectra)}, 0},
    {{"ManipSpecUlatorAction", NULL, "Spec-_U-Lator", "F12", NULL, G_CALLBACK(MenuSpecULator)}, 0},

    {{"DisplayMenuAction", NULL, "_Display", NULL, NULL, NULL}, 0},
    {{"DisplayDispAction", NULL, "_Display", "D", NULL, G_CALLBACK(MenuDisplayPlot)}, 0},
    {{"DisplayDispOverAction", NULL, "Display_Overlayed", "<control>D", NULL,
      G_CALLBACK(MenuMultiInOne)},
     0},
    {{"DisplayChangeAction", NULL, "_Change Spectrum", "<shift>D", NULL,
      G_CALLBACK(MenuDisplayPlotSameRange)},
     0},
    {{"DisplayMtoSAction", NULL, "M -> S fast", "Z", NULL, G_CALLBACK(MenuMultipleToSingleFast)},
     0},
    {{"DisplayGetCalAction", NULL, "Calibration", "K", NULL, G_CALLBACK(MenuGetCalibration)}, 0},
    {{"DisplaySetEffAction", NULL, "Set Efficiency", NULL, NULL, G_CALLBACK(MenuSetEfficiency)}, 0},

    {{"DisplayTypeMenuAction", NULL, "_Type", NULL, NULL, NULL}, 0},

    {{"DisplayTypeLinearAction", NULL, "_Linear", NULL, NULL, G_CALLBACK(SetPlotType)}, 1},
    {{"DisplayTypeLogAction", NULL, "_Log", NULL, NULL, G_CALLBACK(SetPlotType)}, 2},
    {{"DisplayTypeRootAction", NULL, "_Root", NULL, NULL, G_CALLBACK(SetPlotType)}, 3},
    {{"DisplayTypeEfficAction", NULL, "_Efficency Corrected", NULL, NULL, G_CALLBACK(SetPlotType)},
     4},

    {{"DisplayMarkAction", NULL, "_Mark", "M", NULL, G_CALLBACK(MenuManualMarkSet)}, 0},
    {{"DisplayExpandAction", NULL, "_Expand", "E", NULL, G_CALLBACK(MenuExpandPlot)}, 0},
    {{"DisplayManExpAction", NULL, "Manual Expand", "<shift>E", NULL,
      G_CALLBACK(MenuManualExpandPlot)},
     0},
    {{"DisplayZoomAction", NULL, "Zoom _Out", "O", NULL, G_CALLBACK(MenuZoomOut)}, 0},
    {{"DisplaySRightAction", NULL, "Slide Right", NULL, NULL, G_CALLBACK(MenuShiftRight)}, 0},
    {{"DisplaySLeftAction", NULL, "Slide Left", NULL, NULL, G_CALLBACK(MenuShiftLeft)}, 0},
    {{"DisplayNextSpecAction", NULL, "Next Spectrum", NULL, NULL, G_CALLBACK(MenuNextSpectra)}, 0},
    {{"DisplayPrevSpecAction", NULL, "Prev Spectrum", NULL, NULL, G_CALLBACK(MenuPrevSpectra)}, 0},

    {{"DisplayRangeMenuAction", NULL, "Range", NULL, NULL, NULL}, 0},
    {{"DisplayRangeSetYAction", NULL, "Set Y Max", "Y", NULL, G_CALLBACK(MenuSetYScale)}, 0},
    {{"DisplayRangeScaleUpAction", NULL, "Scale Up", NULL, NULL, G_CALLBACK(MenuYScaleDown)}, 0},
    {{"DisplayRangeScaleDnAction", NULL, "Scale Down", NULL, NULL, G_CALLBACK(MenuYScaleUp)}, 0},

    {{"DisplayOverlayMenuAction", NULL, "Overlay Mode", NULL, NULL, NULL}, 0},
    {{"DisplayOverlayOffAction", NULL, "Auto Y Scale Off", NULL, NULL,
      G_CALLBACK(MenuSetYScaleMode)},
     0},
    {{"DisplayOverlayOnAction", NULL, "Auto Y Scale On", NULL, NULL, G_CALLBACK(MenuSetYScaleMode)},
     1},

    {{"DisplayRedrawAction", NULL, "Redraw", "N", NULL, G_CALLBACK(MenuRedraw)}, 0},
    {{"DisplayActiveUpAction", NULL, "Active Spectrum Up", NULL, NULL, G_CALLBACK(MenuActiveUp)},
     0},
    {{"DisplayActiveDnAction", NULL, "Active Spectrum Down", NULL, NULL,
      G_CALLBACK(MenuActiveDown)},
     0},

    {{"AnalysisMenuAction", NULL, "_Analysis", NULL, NULL, NULL}, 0},
    {{"AnalysisGaussAction", NULL, "_Gaussian", "G", NULL, G_CALLBACK(MenuGaussFit)}, 0},
    {{"AnalysisDGaussAction", NULL, "_Double Gaussian", "<shift>G", NULL,
      G_CALLBACK(MenuDoubleGaussFit)},
     0},
    {{"AnalysisFWidGaussAction", NULL, "_Fixed FWHM Gaussian", "<control>G", NULL,
      G_CALLBACK(MenuFixedWidthGaussFit)},
     0},
    {{"AnalysisSumAction", NULL, "_Sum", "S", NULL, G_CALLBACK(MenuGetSum)}, 0},
    {{"AnalysisSetBgAction", NULL, "Set _Background", "B", NULL, G_CALLBACK(MenuSetBackground)}, 0},
    {{"AnalysisManBgAction", NULL, "_Manual Set Background", "<alt>B", NULL,
      G_CALLBACK(MenuManualSetBackground)},
     0},
    {{"AnalysisOneChnBgAction", NULL, "S_ingle Channel Background", "<shift>B", NULL,
      G_CALLBACK(MenuOneChSetBackground)},
     0},
    {{"AnalysisExpFitAction", NULL, "_Exponential Fit", "X", NULL, G_CALLBACK(MenuExpFit)}, 0},
    {{"AnalysisPkFitAction", NULL, "_Peak Fit", NULL, NULL, G_CALLBACK(MenuPeakFit)}, 0},
    {{"AnalysisRdCalibAction", NULL, "Read _Calib Info", NULL, NULL, G_CALLBACK(MenuReadCalibInfo)},
     0},

    {{"AnalysisBgSetMenuAction", NULL, "Background _Poly", NULL, NULL, NULL}, 0},
    {{"AnalysisBgSetConstAction", NULL, "_Constant", NULL, NULL, G_CALLBACK(MenuSetBackgroundPoly)},
     0},
    {{"AnalysisBgSetLinAction", NULL, "_Linear", NULL, NULL, G_CALLBACK(MenuSetBackgroundPoly)}, 1},
    {{"AnalysisBgSetQuadAction", NULL, "_Quadratic", NULL, NULL, G_CALLBACK(MenuSetBackgroundPoly)},
     2},

    {{"SortMenuAction", NULL, "Sort", NULL, NULL, NULL}, 0},
    {{"SortGS92SetupAction", NULL, "GS92 Sort Setup", NULL, NULL, G_CALLBACK(MenuSetup)}, 0},
    {{"SortGS92SortAction", NULL, "GS92 Sort", NULL, NULL, G_CALLBACK(MenuSort)}, 0},
    {{"SortPGAMAction", NULL, "PGAM S_ort", NULL, NULL, G_CALLBACK(MenuPGAMSort)}, 0},

    {{"TwoDMenuAction", NULL, "_2D Histograms", NULL, NULL, NULL}, 0},
    {{"TwoDDisplayAction", NULL, "Display", NULL, NULL, G_CALLBACK(MenuDisplayBig)}, 0},
    {{"TwoDHistMatrixAction", NULL, "Histogram Matrix", NULL, NULL, G_CALLBACK(MenuPgamMatrix2Big)},
     0},

    {{"MatrixMenuAction", NULL, "_Matrix", NULL, NULL, NULL}, 0},
    {{"MatrixProjAction", NULL, "_Projection", "P", NULL, G_CALLBACK(MenuProjectionBox)}, 0},
    {{"MatrixHistAction", NULL, "Histogram Matrix", NULL, NULL, G_CALLBACK(MenuPgamMatrix2Big)}, 0},
    {{"MatrixAddAction", NULL, "Add Matrix", NULL, NULL, G_CALLBACK(MenuReadPgamAdd)}, 0},
    {{"MatrixSubAction", NULL, "Subtract Matrix", NULL, NULL, G_CALLBACK(MenuReadPgamSub)}, 0},

    {{"RadwareMenuAction", NULL, "_Radware", NULL, NULL, NULL}, 0},
    {{"RadwareWriteAction", NULL, "Write Matrix", NULL, NULL, G_CALLBACK(MenuWriteRadwareMatrix)},
     0},

    {{"DegradedMenuAction", NULL, "D_egraded", NULL, NULL, NULL}, 0},

    {{"HelpMenuAction", NULL, "_Help", NULL, NULL, NULL}, 0},
    {{"HelpAction", NULL, "Help", NULL, NULL, G_CALLBACK(MenuHelp)}, 0},
    {{"HelpAboutAction", NULL, "_About", NULL, NULL, G_CALLBACK(MenuAbout)}, 0}

};

/*
note on syntax... Putting in a long string in a C initialization requires 'escaping' """
with the "\" character.  Also each line needs explicit linefeed added '\n' AND a '\' for
a continuation.
*/

static char *menu_ui = " \n\
<ui>\n\
  <menubar name=\"MainMenu\"> \n\
  <menu action=\"FileMenuAction\"> \n\
      <menuitem action=\"FileReadAction\"  /> \n\
      <menuitem action=\"FileWriteAction\" /> \n\
      <menuitem action=\"FileWLiphaAction\" /> \n\
      <menuitem action=\"FileW2DAction\" /> \n\
      <menuitem action=\"FileWMatrixAction\" /> \n\
      <menuitem action=\"FileCompMulAction\" /> \n\
      <menuitem action=\"FilePrintAction\" /> \n\
      <menuitem action=\"FilePrintFileAction\" /> \n\
      <menuitem action=\"FileQuitAction\" /> \n\
  </menu> \n\
  <menu action=\"ManipMenuAction\"> \n\
      <menuitem action=\"ManipAddAction\" /> \n\
      <menuitem action=\"ManipCompressAction\" /> \n\
      <menuitem action=\"ManipGainAction\" /> \n\
      <menuitem action=\"ManipMoveAction\" /> \n\
      <menuitem action=\"ManipNormAction\" /> \n\
      <menuitem action=\"ManipSubAction\" /> \n\
      <menuitem action=\"ManipTitleAction\" /> \n\
      <menuitem action=\"ManipClearAction\" /> \n\
      <menuitem action=\"ManipSpecUlatorAction\" /> \n\
  </menu> \n\
  <menu action=\"DisplayMenuAction\"> \n\
      <menuitem action=\"DisplayDispAction\" /> \n\
      <menuitem action=\"DisplayDispOverAction\" /> \n\
      <menuitem action=\"DisplayChangeAction\" /> \n\
      <menuitem action=\"DisplayMtoSAction\" /> \n\
      <menuitem action=\"DisplayGetCalAction\" /> \n\
      <menuitem action=\"DisplaySetEffAction\" /> \n\
      <menu action=\"DisplayTypeMenuAction\"> \n\
          <menuitem action=\"DisplayTypeLinearAction\" /> \n\
          <menuitem action=\"DisplayTypeLogAction\" /> \n\
          <menuitem action=\"DisplayTypeRootAction\" /> \n\
          <menuitem action=\"DisplayTypeEfficAction\" /> \n\
      </menu> \n\
      <menuitem action=\"DisplayMarkAction\" /> \n\
      <menuitem action=\"DisplayExpandAction\" /> \n\
      <menuitem action=\"DisplayManExpAction\" /> \n\
      <menuitem action=\"DisplayZoomAction\" /> \n\
      <menuitem action=\"DisplaySRightAction\" /> \n\
      <menuitem action=\"DisplaySLeftAction\" /> \n\
      <menuitem action=\"DisplayNextSpecAction\" /> \n\
      <menuitem action=\"DisplayPrevSpecAction\" /> \n\
      <menu action=\"DisplayRangeMenuAction\"> \n\
          <menuitem action=\"DisplayRangeSetYAction\" /> \n\
          <menuitem action=\"DisplayRangeScaleUpAction\" /> \n\
          <menuitem action=\"DisplayRangeScaleDnAction\" /> \n\
      </menu> \n\
      <menu action=\"DisplayOverlayMenuAction\"> \n\
          <menuitem action=\"DisplayOverlayOffAction\" /> \n\
          <menuitem action=\"DisplayOverlayOnAction\" /> \n\
      </menu> \n\
      <menuitem action=\"DisplayRedrawAction\" /> \n\
      <menuitem action=\"DisplayActiveUpAction\" /> \n\
      <menuitem action=\"DisplayActiveDnAction\" /> \n\
  </menu> \n\
  <menu action=\"AnalysisMenuAction\"> \n\
      <menuitem action=\"AnalysisGaussAction\" /> \n\
      <menuitem action=\"AnalysisDGaussAction\" /> \n\
      <menuitem action=\"AnalysisFWidGaussAction\" /> \n\
      <menuitem action=\"AnalysisSumAction\" /> \n\
      <menuitem action=\"AnalysisSetBgAction\" /> \n\
      <menuitem action=\"AnalysisManBgAction\" /> \n\
      <menuitem action=\"AnalysisOneChnBgAction\" /> \n\
      <menuitem action=\"AnalysisExpFitAction\" /> \n\
      <menuitem action=\"AnalysisPkFitAction\" /> \n\
      <menuitem action=\"AnalysisRdCalibAction\" /> \n\
      <menu action=\"AnalysisBgSetMenuAction\"> \n\
          <menuitem action=\"AnalysisBgSetConstAction\" /> \n\
          <menuitem action=\"AnalysisBgSetLinAction\" /> \n\
          <menuitem action=\"AnalysisBgSetQuadAction\" /> \n\
      </menu> \n\
  </menu> \n\
  <menu action=\"SortMenuAction\"> \n\
      <menuitem action=\"SortGS92SetupAction\" /> \n\
      <menuitem action=\"SortGS92SortAction\" /> \n\
      <menuitem action=\"SortPGAMAction\" /> \n\
  </menu>  \n\
  <menu action=\"TwoDMenuAction\"> \n\
      <menuitem action=\"TwoDDisplayAction\" /> \n\
      <menuitem action=\"TwoDHistMatrixAction\" /> \n\
  </menu>  \n\
  <menu action=\"MatrixMenuAction\"> \n\
      <menuitem action=\"MatrixProjAction\" /> \n\
      <menuitem action=\"MatrixHistAction\" /> \n\
      <menuitem action=\"MatrixAddAction\" /> \n\
      <menuitem action=\"MatrixSubAction\" /> \n\
  </menu>  \n\
  <menu action=\"RadwareMenuAction\"> \n\
      <menuitem action=\"RadwareWriteAction\" /> \n\
  </menu>  \n\
  <menu action=\"HelpMenuAction\"> \n\
      <menuitem action=\"HelpAction\" /> \n\
      <menuitem action=\"HelpAboutAction\" /> \n\
  </menu>  \n\
</menubar> \n\
</ui>      \n\
";

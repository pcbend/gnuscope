/* gnuscopefuncs.h
 *
 * By John Pavan
 *
 * Function declarations for gnuscope
 * Please try to keep it organized by file
 */

/* --- includes for system functions --- */

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <glob.h>
#include <gdk/gdk.h>
//--ddc 3jun08 add gdkksyms to resolve implicit declaration issues
#include <gdk/gdkkeysyms.h>
#include <math.h>
//--ddc 3jun08 all function prototypes SHOULD be in this file.  Here is
// one that seems to be missing alot
#include <stdlib.h>

/* --- includes from custom .h files --- */
#include "gtkgraph.h"
// #include "ecallib.h"
#include "filereading.h"
// #include "gnuscopeglobals.h"
#include "pgam.h"

/* --- from analysis.c --- */
float GetSumFF(int a, int b);
void CloseExpWindow(GtkWidget *widget, gpointer data);
void FixedWidthGaussFit(float deviation);
void ExpFitDoIt(GtkWidget *widget, gpointer *data);
void ExpFitRecallExpBack(GtkWidget *widget, gpointer *data);
void ExpFitFixLinBackCallback(GtkWidget *widget, gpointer *data);
void ExpFitVarLinBackCallback(GtkWidget *widget, gpointer *data);
void ExpFitWindow();
void CloseExpWindow(GtkWidget *widget, gpointer data);
void ExpFit(int mode, double c, double backa, double backb);
void DrawExpFit(double c, double d, double e, double backa, double backb);
void DrawBackground();
float Background(int channel);
float GetBackgroundCounts(int a, int b);
int GetSumCounts(int a, int b);
void DoubleGaussFit(float deviation1, float deviation2);
float GetSumCenter(int a, int b, int counts);
float GetSumDeviation(int a, int b, int counts, float center);
void DisplayGaussian(float center, float deviation, float area);
float GaussChiSqr(int a, int b, float center, float deviation, float *area);
void DisplayGaussian(float center, float deviation, float area);
float DoubleGaussChiSqr(int a, int b, float center1, float center2, float deviation1,
                        float deviation2, float *area1, float *area2);
void DisplayDoubleGaussian(float center1, float center2, float deviation1, float deviation2,
                           float area1, float area2);
int CompareInts(const void *a, const void *b);
void FixedWidthGaussFitEntry(GtkWidget *widget, GtkWidget *entry);
void FixedWidthGaussFitPrompt(GtkWidget *text);
void DoubleGaussFitEntry(GtkWidget *widget, GtkWidget *entry);
void DoubleGaussFitPrompt(GtkWidget *text);
void ManualSetBackgroundPrompt(GtkWidget *text);
void ManualSetBackgroundEntry(GtkWidget *widget, GtkWidget *entry);
float UseCalibration(float chan);
float UseCalibrationFWHM(float chan);
float UseEfficiency(float counts, float energy);
void LastFitAddPoint(float x, float y);
void LastFitClearPoints();
double ExpChiSqr(double a, double b, double c, double bka, double bkb);
void Redraw();

/* --- functions in commandfile.c --- */
void CommandLineHandler(int argc, char *argv[]);
void CommandFileHandler(char *sFilename);

/* --- functions in configfile.c --- */
void ConfigFileSearcher();
void ConfigFileHandler(FILE *infile);

/* --- from output.c --- */
void WriteMainText(const char *newtext);
void WriteTwodText(const char *newtext);
void WriteMessageText(const char *newtext);
void CreateNewMessageTextWindow();

/* --- from pgam.c --- */
void TwodCreateDisplaySelectionWindow();
void WriteTwodText(const char *newtext);
void TwodDisplayCurrentRange();
void TwodSetMarkers();
void TwodDrawMarkers();
void gdk_point_copy(GdkPoint point1, GdkPoint *point2);
void TwodRefresh();
void TwodExpand();
void ReadBig(char *sFilename);
void WriteBig(char *sFilename);
void ReadGates(char *sFilename);
void WriteGates(char *sFilename);
//--ddc TwodGateSet, static, only in pgam.c!
// gint TwodGateSet(GtkWidget *widget,GdkEventKey *event);
void TwodInitializeGate();
void TwodShiftUpLeft();
void TwodShiftUp();
void TwodShiftUpRight();
void TwodShiftLeft();
void TwodShiftRight();
void TwodShiftDownLeft();
void TwodShiftDown();
int FirstOccurance(int *haystack, int straws, int needle);
int LastOccurance(int *haystack, int straws, int needle);
void TwodShiftDownRight();
void TwodGateFill();
void TwodCloseGate();
void PgamReadSetup(char *sFilename);
void PgamReadMasterSetup(char *sFilename);
gint CloseTwodPlotWindow(GtkWidget *widget, gpointer *data);
void PgamSort();
void PrevTwod();
void NextTwod();
void ReadPgamTwod(char *sFilename);
void PgamTwodProjectFull();
void WritePgamTwod(char *sFilename);
void KinmatInfoStructAdd(struct kinmat_info_struct *stru, float ex, float en);
struct kinmat_info_struct *KinmatInfoStructNew();
void KinmatInfoStructDestroy(struct kinmat_info_struct *stru);
float KinmatInfoStructGetExcit(struct kinmat_info_struct *stru, float ener);
struct kinmat_info_struct *ReadKinmatInfo(char *sFilename);
struct bigdata *BigDataStructNew(int x, int y);
void BigDataStructDestroy(struct bigdata *stru);
int BigDataStructGetVal(struct bigdata *stru, int x, int y);
void BigDataStructSetVal(struct bigdata *stru, int x, int y, int z);
void BigDataStructValPP(struct bigdata *stru, int x, int y);
void NumContoursPrompt(GtkWidget *text);
void NumContoursEntry(GtkWidget *widget, GtkWidget *entry);
void PgamSpawnPrint(char *cprinter);
void PgamSpawnPrintEntry(GtkWidget *widget, GtkWidget *entry);
void WritePgamPostscript(char *sFilename);
void PgamSetTwodTitleEntry(GtkWidget *widget, GtkWidget *entry);
void PgamSetTwodTitlePrompt(GtkWidget *text);
void PgamDisplaySelectionEntry(GtkWidget *widget, GtkWidget *entry);
void PgamDisplaySelectionPrompt(GtkWidget *text);
void PgamClear2DEntry(GtkWidget *widget, GtkWidget *entry);
void PgamClear2DPrompt(GtkWidget *text);
void PgamAdd2DEntry(GtkWidget *widget, GtkWidget *entry);
void PgamAdd2DPrompt(GtkWidget *text);
void PgamSubtract2DEntry(GtkWidget *widget, GtkWidget *entry);
void PgamSubtract2DPrompt(GtkWidget *text);
void PgamMatrix2Big();
void DisplayBig();
void TwodSetMarker(GtkWidget *widget, GdkEventButton *event);
struct gatedata *GateDataStructNew(int x, int y, int depth);
void GateDataStructDestroy(struct gatedata *stru);
int GateDataStructGetVal(struct gatedata *stru, int *result, int x, int y);
int GateDataStructTest(struct gatedata *stru, int bit, int x, int y);
int GateDataStructSetVal(struct gatedata *stru, int map, int x, int y, int depth);
int GateDataStructToggleOn(struct gatedata *stru, int bit, int x, int y);
int GateDataStructToggleOff(struct gatedata *stru, int bit, int x, int y);

/* --- from pgamsort.c --- */
struct veto_struct *PgamReadVeto(char *sFilename);

/* --- from radwarecompat.c --- */
void WriteRadwareSquare(char *sFilename);
void WriteRadwareTriangle(char *sFilename);
void ReadRadwareSquare(char *sFilename);
void ReadRadwareTriangle(char *sFilename);
void ReadRadwareMatrix(char *sFilename);
void WriteRadwareIntTriangle(char *sFilename);
void WriteRadwareIntSquare(char *sFilename);
void WriteRadwareMatrix(char *sFilename);

/* --- from rwfiles.c --- */
void ReadFile(char *sFilename);
void WriteFile(char *sFilename);
void ReadGates(char *sFilename);
void init_ornl(struct ornldir *, struct ornlheader *);
int read_fsuhead(struct fsuheader *, FILE *);
int read_fsudir(struct fsudir *, FILE *);
int write_fsuhead(struct fsuheader *, FILE *);
int write_fsudir(struct fsudir *, FILE *);
void RetainSpectraPrompt(GtkWidget *text);
void RetainSpectraEntry(GtkWidget *widget, GtkWidget *entry);
void WriteFilePrompt(GtkWidget *text);
void WriteFileEntry(GtkWidget *widget, GtkWidget *entry);
void AutoDetectFileType(char *sFilename);
void AddValToHistogram(int spec, int x, int y);

/* --- as yet unlocated functions --- */
struct bigdata *BigDataStructNew(int x, int y);
void BigDataStructDestroy(struct bigdata *stru);
int BigDataStructGetVal(struct bigdata *stru, int x, int y);
void BigDataStructSetVal(struct bigdata *stru, int x, int y, int z);
void BigDataStructValPP(struct bigdata *stru, int x, int y);

void CommandFileHandler(char *sFilename);

/* --- greta stuff --- */
void GRETAReadTrace(char *sFilename);
void GretaTraceToMatLab(char *sFilename, char *outFilename);
void GretaMSTToMatLab(char *sFilename, char *outFilename);

//--ddc 3jun08 all function prototypes SHOULD be in this file.
// Add the one that seems to be big problem for the commandlinehandler,
// SetCalibration, and while I'm going to the trouble, some other
// functions the compiler complainsabout.
void SetCalibration(int, float, float, float);
void GaussFit();
void GetSum();
void PgamReadDetectorInfo(char *);
void PgamReadGammaGate(char *);
void ObliterateHistogram(int);
void GetMessageDialog(const char *);
void PgamSortSelectionWindow();
void CreatePrintWindow(GtkWidget *);
void GetDialog(int);
void SetPgamEntry14(char *sFilename);
int GetLastSpectrum();
void DisplayCurrentRange();
void DrawMarkers();
void PgamClear2DPrompt(GtkWidget *text);

//--ddc from peakfit.c
void PeakFitWindow();

//--ddc from progress.c
void StartProgress(void);
void EndProgress(void);
void UpdateProgress(long, long);
void StartSortProgress(void);
void EndSortProgress(void);
void UpdateSortProgress(long, long);

//--ddc from gs92.c
int Min(int, int);
int Max(int, int);

//--ddc from filesel.c
void GetFilename(char *, int, void (*callback)(char *));

//--ddc aug11 new from output.c

GtkTextBuffer *mytextbuffer(GtkWidget *);
GtkTextView *init_text_view(void);
gint TextViewKeyCallback(GtkWidget *, gpointer *);

//-- trying to make cmake work
void Compress2FileNOGTK(char *sFilename);
void ProjectFull(void);
int getints(FILE *infile, int *array, int n);
void ReadAutoMatrix(char *sFilename);
GtkTextBuffer *mytextbuffer(GtkWidget *text);
//void PgamClear2Dprompt(GtkWidget *text);

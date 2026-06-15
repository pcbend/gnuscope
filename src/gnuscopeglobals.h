#define MAXPATH 180

/* --- globals used in all gnuscope functions --- */
/* --- I think --- */
/* --- structs --- */

struct gsdata {
  int headbuffer[2];    /*  Buffer size of the header */
  int size;             /*  Number of channels on a side */
  int what;             /*  Don't know, was in original fortran --- only used for .twd */
  int type;             /*  Don't know, was in original fortran */
  int df;               /*  Don't know, was in original fortran */
  int databuffer[2];    /*  Buffer size of data */
  short int *data;      /*  A pointer to the data for the square or triangle */
  int filetype;         /*  Is it a triangle or a square 1 = triangle, 2 = square */
};

struct axis_info {
  int num;                            /* number of detectors on that axis */
  short unsigned int detectors[254];  /* detector numbers */
};

struct gamma_gate {
  int addorsubtract;     /* +1 for add gate -1 for subtract gate */
  int lower;
  int upper;
  int gamma_ray;         /* energy of the coresponding gamma ray */
};

struct floatpoint {
  float x;
  float y;
};


//struct my_matrix_struct {
//  int size;           // size of the matrix
//  double *data;       // elements of the matrix
//};

/* --- globals --- */

int online; // zero if not online, number of last online histogram if online;
int spectradisplayed[16][10];
struct gsdata twoddata;
/* --- histogram stuff --- */

int histid[1028],histsize[1028],histpointer;
char field1[1028][40],field2[1028][40],field3[1028][40];
int *histloc[1028];

/* --- not quite histogram stuff --- */
char manualpath[120];
char pdfapp[120];
int pgamsortgatenone,pgamsortgatetac,pgamsortgatepart,pgamsortgategamma;
int pgamsortoutputhists,pgamsortoutputpart,pgamsortoutputtac,pgamsortoutputgg;
int pgamsortoutputggtype,pgamsortgateparten;
int pgamsortgategammatypeentry,pgamsortgategammatypefile;
int pgamsortoutputsqr;
int pgamsortgateveto;
int pgamsortgaterequires;
struct veto_struct *vetos;
struct veto_struct *requires;
float telfincal;
int pgamsortoutputpairsqr;
int minclovermultipolaritytoggle;
int selfsuppressclovers;
float partenergies[2];
float partfincal;
char pgampairfilename[120];
int num_gates;
int radwarematrixsize; // the size for radware matricies (no header)
int globalcalibrationset;
float globalcalibration[3],calibration[1028][3];
char colorstring[10][80];
int colordepth; // sets the color depth for gtktwodplot (default will be 256).
int commandlineonly;
int pgamcolormode;
int pgamcontourtype;
int pgamtype;
int interpolationmode;
int num_2D_gates;
GtkWidget *manipulatetitlewindow;
int datafiletype;
GtkWidget *projectionwindow;
float peakfita[3];
float peakfitb[3];
float peakfitc[3];
int min_clover_multipolarity;
short int display_toggle[254];
int printnotdone;
int pgammax;
int binsizeforce;
int abortflag;
FILE *logfile;
GtkWidget *channelinfo;
int lastspectraclicked;
int cursorx;
GtkWidget *lastgraphclicked;
struct pgam_gamma_gates pgamgammagates;
struct pgam_matrix_info pgammatrixdata;
int drawing_gates;
int fade_mode;
int pgam;
int pgammax;
GtkWidget *messagetext;
int intype,spectra,plottype;
GtkWidget *graph;
int markers[4],binsize,offset;
//int background;
GdkCursor *cursor;
int oldscope;
int backgroundpolydeg;
double background[3];
int numspectra;
int currentrange[2];
GtkWidget *graphsdisplayed[16];
GtkWidget *box1,*mainwindow;
GtkWidget *maintext;
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
//--ddc 24may06, already declared! GtkWidget *maintext;
GdkPoint twodmarkers[4];
int xcurrentrange[2],ycurrentrange[2];
GtkWidget *twodplot;
int ybinsize,xbinsize;
GtkWidget *twodvbox;
GtkWidget *twodtext;
GdkPoint firstpoint;
GtkWidget *displaytable;
struct stoppingpower *stoppingpowerinfo;
float telfincal;
int twodnumcontours;
int edesize;
char twodtitles[254][80];
GtkWidget *twoddisplayed[16];
short int twodspectradisplayed[16];
short int numtwoddisplayed;
GtkWidget *Window2D;
int writemin,writemax;
int intype ;
int column_width;
int lastfitnumpoints;
long double displayused;
int ybinforce,yscaleforce;
int ybin,yscale;
GtkWidget *ExpFitEntry[3]; // 0 = Lbg 1 = EbgA 2 = EbgB
int expfitmode;
double expfitlasta;
double expfitlastb;
struct floatpoint *lastfitpoints;
int d_max_adcs;

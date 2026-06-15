
/* --- structs --- */

struct bigdata {
  int axes[2];            // number of channels per axis
  int **vals;              // pointer to the memory which holds the values
};

struct gatedata {
  int axes[2];            // number of channels per axis ([0] = x, [1] = y)
  int depth;              // depth of the vals feild (in bits)
  int ***vals;            // pointer to the array(s) of values.  index 2 = x, 3 = y, 1 = depth ( / 32).
};

struct detector_info {
  char title[40];         // title of the detector (in English or your favorite language)
  int adc;                // adc number
  int channels;           // number of channels written to tape
  int overflow;           // channel number at which the overflow events are written.
  float angle;            // angle of that detector from the beam in degrees (90 is perp)
  float calib[3];         // polynomial energy calibration
};



/* multi_hpge_info struct
 *
 * for sorting from multi-hpge detectors
 * by including adcs in this struct the "add-back" feature will be used
 * with results going to the "total" spectrum
 * the num_adcs and *adcs will be handled using the functions:
 * void multi_hpge_adc_add(struct multi_hpge_info *multi,int adc);
 * int multi_hpge_adc_list(struct multi_hpge_info *multi, int index);
 * void multi_hpge_adc_initialize(struct multi_hpge_info *multi);
 * void multi_hpge_adc_clear(struct multi_hpge_info *multi);
 */
struct multi_hpge_info {
  char totaltitle[40];    // title of the totalcrystal
  int num_adcs;           // the number of adcs used by the multi-hpge detector
  int totadc;             // adc of the total spectrum (shouldn't be a real adc)
  int *adcs;              // adcs of the crystals
};

/* telescope_info struct
 *
 * for sorting using a e/delta-e telescope
 * the total energy will goto a "virtual" adc
 */
struct telescope_info {
  char totaltitle[40];
  int e_adc;
  int delta_e_adc;
  int totaladc;
  float gain;
  int special;       // special flag to indicate that this telescope should not be excluded from other sorts (like for a TAC vs E plot)
  int num_e_sup;
  int num_de_sup;
  int *e_sup_adc;
  int *de_sup_adc;
  float sup_cal[3]; // a supplemental calibration for kinematic corrections
  struct kinmat_info_struct *kinmatinfo; // kinmat info for kinematic corrections 
};

/* struct kinmat_info_struct
 *
 * a struct for Ex and En
 */
struct kinmat_info_struct {
  int num;     // number of kinmat_info lines
  float *ex;   // values of ex
  float *en;   // values of en
};

/*
 * tac_gate_info struct
 *
 * for storing the adc, min and max of a tac gate
 */
struct tac_gate_info {
  int adc;
  int min;
  int max;
};

/*
 * tac_info struct
 *
 * for storing relevant information about the tac
 * the num_adcs and *adcs_used will be handled in a somewhat sophisticated manner
 * using the functions:
 * void tac_adc_add(struct tac_info *tac, int adc);
 * int tac_adc_list(stuct tac_info *tac,int index);
 * void tac_adc_initialize(struct tac_info *tac);
 * void tac_adc_clear(struct tac_info *tac);
 */
struct tac_info {
  char title[40];         // title of the tac
  short int adc;                // adc number for the tac
  short int num_adcs;           // number of adcs using the tac
  struct tac_gate_info *gates;   // gates
};

/* 
 * struct pgam_matrix_data
 *
 * for storing the relevant information about the twd and sqr files
 * for use with pgamsort.  These differ from those used with gs92
 * because they are using floats instead of short ints
 */
struct pgam_matrix_info {
  int headbuffer[2];           //  fortran buffer sizes for the header
  int size;                    //  number of channels to a side
  int what;                    //  no clue, included for compatability
  int type;                    //  serial number of the data type
  /* --- with regaurd to type --- */
  /* --- type = 0 for squares --- */
  /* --- type = 3 for triangles --- */
  /* --- there was also a variable called df in gs92, but isn't used here --- */
  int databuffer[2];           // fortran buffer size for the body of the data
  float *data;                 // the pointer to the data (in floats)
  /* --- in gs92 we used "filetype" as a variable which was determined by
     --- the file extention.  Here we will use the "type" from the header --- */
};

/*
 * struct pgam_gamma_gates
 *
 * for use with gamma gating during sort
 */
struct pgam_gamma_gates {
  int num;                  // number of gamma gates
  float *min;               // mins for the gamma gates
  float *max;               // maxes for the gamma gates
};

/* struct pgam_pair
 *
 * for use with polarization
 */
struct pgam_pair {
  int a;  // first detector in pair
  int b;  // second detector in pair
};

/* struct pgam_pairs
 *
 * for use with polarization
 */
struct pgam_pairs{
  int num;  // number of pairs 
  struct pgam_pair *pairs;
};

/* struct veto_struct
 * 
 * for use with veto gate
 */
struct veto_struct {
  short num;    // number of veto gates
  short *adc;    // adc to monitor for the veto signal
  short *min;    // min channel to veto
  short *max;    // max channel to veto
};

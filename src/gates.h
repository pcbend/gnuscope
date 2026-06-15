/* gates.h
 *
 * by John Pavan
 * structs for making gates on detectors (for twd and sqr matrices)
 * and for gamma-gating
 */

struct axis_info {
  int num;                       // number of detectors on that axis
  short unsigned int *detectors; // list of the detector numbers
};

struct gamma_gte {
  int addorsubtract; // +1 for add gate -1 for subtract gate
  int lower;
  int upper;
  int gamma_ray; // energy of the corresponding gamma ray
};

struct gate_array {
  int size;        // size of the gate array
  short int *vals; // values
}

/*
 * File: gtkgraph.h
 * Auth: Eric Harlow
*/
#ifndef __GTK_TWODPLOT_H__
#define __GTK_TWODPLOT_H__

#include <gdk/gdk.h>
//--ddc jan15 #include <gtk/gtkvbox.h>
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  /*
   * --- Macros for conversion and type checking
   */
  //--ddc aug12 GTK_CHECK_CAST G_TYPE_CHECK_INSTANCE_CAST
  //            GTK_CHECK_TYPE G_TYPE_CHECK_INSTANCE_TYPE 
  #define GTK_TWODPLOT(obj) \
     G_TYPE_CHECK_INSTANCE_CAST(obj, gtk_twodplot_get_type(), GtkTwodplot)
  #define GTK_TWODPLOT_CLASS(klass) \
    GTK_CHECK_CLASS_CAST (klass, gtk_twodplot_get_type, GtkTwodplotClass)
  #define GTK_IS_TWODPLOT(obj) \
    G_TYPE_CHECK_INSTANCE_TYPE (obj, gtk_twodplot_get_type())

  /* --- enumration for the type of plotting --- */
  enum gtk_twodplot_type {GTK_TWODPLOT_CONTOUR_ONLY,
			  GTK_TWODPLOT_DENSITY_ONLY,
			  GTK_TWODPLOT_CONTOUR_AND_DENSITY};

  enum gtk_twodplot_contour_type {GTK_TWODPLOT_CONTOUR_TYPE_LINEAR,
				  GTK_TWODPLOT_CONTOUR_TYPE_LOG,
				  GTK_TWODPLOT_CONTOUR_TYPE_ROOT,
				  GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_LOG,
				  GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_ROOT};

  enum gtk_twodplot_density_type {GTK_TWODPLOT_DENSITY_TYPE_COLOR,
				  GTK_TWODPLOT_DENSITY_TYPE_BW};
  
  enum gtk_twodplot_interpolation_type {GTK_TWODPLOT_INTERPOLATION_OFF,
					GTK_TWODPLOT_INTERPOLATION_LINEAR,
					GTK_TWODPLOT_INTERPOLATION_QUADRATIC,
					GTK_TWODPLOT_INTERPOLATION_CUBIC,
					GTK_TWODPLOT_INTERPOLATION_FOUR,
					GTK_TWODPLOT_INTERPOLATION_FIVE,
					GTK_TWODPLOT_INTERPOLATION_SIX};
  enum gtk_twodplot_fade {GTK_TWODPLOT_FADE_ON,
			  GTK_TWODPLOT_FADE_OFF};
 
  /*
   * --- Defining data structures
   */

  typedef struct _GtkTwodplot                GtkTwodplot;
  typedef struct _GtkTwodplotClass   GtkTwodplotClass;

  /*
   * --- Here's the graph data
   */

  struct _GtkTwodplot
  {
    GtkWidget vbox;
    short int **gate;
    float xcalibs[3];
    float ycalibs[3];
    float **values;
    gint num_values;
    int color_depth;    // the number of colors to be used
    float x_frac;     // horizontal fraction of the screen to occupy
    float y_frac;     // vertical fraction of the screen to occupy
    gint x_num;
    gint y_num;
    gint x_range[2];
    gint y_range[2];
    char title[80];
    gint num_contours;
    enum gtk_twodplot_type plot_mode;
    enum gtk_twodplot_contour_type contour_mode;
    enum gtk_twodplot_density_type density_mode;
    enum gtk_twodplot_interpolation_type interpolation_mode;
    enum gtk_twodplot_fade fade_mode;
    gint num_markers;
    float *markers_x;
    float *markers_y;
    char **marker_text;
    GdkPixmap *pixmap;
  };

  /*
   * Here's the class data
   */

  struct _GtkTwodplotClass
  {
    GtkWidgetClass parent_class;
  };

  /*
   * Function prototypes
   */
  GtkWidget* gtk_twodplot_new(void);
  void gtk_twodplot_set_size (GtkTwodplot *twodplot, int x,int y);
  void gtk_twodplot_set_value (GtkTwodplot *twodplot, int x,int y, float value);
  void gtk_twodplot_set_plot_mode(GtkTwodplot *twodplot, enum gtk_twodplot_type mode);
  void gtk_twodplot_set_num_contours(GtkTwodplot *twodplot, int num_contours);
  void gtk_twodplot_set_x_range(GtkTwodplot *twodplot, int x_min, int x_max);
  void gtk_twodplot_set_y_range(GtkTwodplot *twodplot, int y_min, int y_max);
  void gtk_twodplot_toggle_gate (GtkTwodplot *twodplot, int x, int y);
  void gtk_twodplot_make_ps (GtkTwodplot *twodplot, const char *sFilename, int publish);
  void gtk_twodplot_set_contour_type(GtkTwodplot *twodplot, enum gtk_twodplot_contour_type type);
  void gtk_twodplot_set_title(GtkTwodplot *twodplot, const char *title);
  void gtk_twodplot_set_density_type(GtkTwodplot *twodplot, enum gtk_twodplot_density_type type);
  void gtk_twodplot_set_interpolation_type(GtkTwodplot *twodplot, enum gtk_twodplot_interpolation_type type);
  void gtk_twodplot_set_x_calibration(GtkTwodplot *twodplot, float a,float b, float c);
  void gtk_twodplot_set_y_calibration(GtkTwodplot *twodplot, float a, float b, float c);
  void gtk_twodplot_set_fade(GtkTwodplot *twodplot, enum gtk_twodplot_fade fade);
  void gtk_twodplot_set_frac(GtkTwodplot *twodplot, float x, float y);
  void gtk_twodplot_add_marker(GtkTwodplot *twodplot, float x, float y, char *text);
  void gtk_twodplot_clear_markers(GtkTwodplot *twodplot);
  void gtk_twodplot_set_color_depth(GtkTwodplot *twodplot, int colordepth);
  //--ddc 03jun08 missing..
  //--ddc aug11  guint gtk_twodplot_get_type (void);
  GType gtk_twodplot_get_type (void);
  void gtk_twodplot_refresh (GtkWidget *widget);

  //--ddc jun11 gtk2 deprecation of gtk_widget_draw
 void gtk_twodplot_draw (GtkWidget *widget, GdkRectangle *area);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_TWODPLOT_H__ */


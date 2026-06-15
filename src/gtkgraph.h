/*
 * File: gtkgraph.h
 * Auth: John Pavan
 * Kinda based on a widget by the same name by Eric Harlow
 */

#ifndef __GTK_GRAPH_H__
#define __GTK_GRAPH_H__

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
#define GTK_GRAPH(obj) G_TYPE_CHECK_INSTANCE_CAST(obj, gtk_graph_get_type(), GtkGraph)
#define GTK_GRAPH_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, gtk_graph_get_type, GtkGraphClass)
#define GTK_IS_GRAPH(obj) G_TYPE_CHECK_INSTANCE_TYPE(obj, gtk_graph_get_type())

/*
 * --- Defining data structures
 */

typedef struct _GtkGraph GtkGraph;
typedef struct _GtkGraphClass GtkGraphClass;

/* --- enumuration for the way the scale is displayed --- */

enum gtk_graph_scale_type { GTK_GRAPH_SCALE_CHANNELS, GTK_GRAPH_SCALE_CALIBRATED };

enum gtk_graph_type { GTK_GRAPH_TYPE_LINEAR, GTK_GRAPH_TYPE_SEMILOG, GTK_GRAPH_TYPE_SEMISQRT };

enum gtk_graph_y_scale_mode { GTK_GRAPH_Y_SCALE_AUTO_OFF, GTK_GRAPH_Y_SCALE_AUTO_ON };

/*
 * --- Here's the graph data
 */

struct _GtkGraph {
  GtkWidget vbox;
  gint **values;
  gint num_values;
  gint num_lines;
  char **title;
  gint num_segments;
  float calibration[3];
  enum gtk_graph_scale_type scale_type;
  enum gtk_graph_type type;
  enum gtk_graph_y_scale_mode y_scale_mode;
  float *segments_x;
  float *segments_y;
  GdkPixmap *pixmap;
};

/*
 * Here's the class data
 */

struct _GtkGraphClass {
  GtkWidgetClass parent_class;
};

/*
 * Function prototypes
 */
GtkWidget *gtk_graph_new(void);
void gtk_graph_size(GtkGraph *graph, int size);
void gtk_graph_set_value(GtkGraph *graph, int linenum, int index, int value);
void gtk_graph_title(GtkGraph *graph, int linenum, const char *title);
void gtk_graph_clear_segments(GtkGraph *graph);
void gtk_graph_add_segment(GtkGraph *graph, float x, float y);
//--ddc 03jun8 spelling error...int gtk_graph_has_segements(GtkGraph *graph);
int gtk_graph_has_segments(GtkGraph *graph);
//--ddc 03jun08 missing prototype
//--ddc aug11   guint gtk_graph_get_type (void);
GType gtk_graph_get_type(void);
//  GdkPoint *gtk_graph_get_segment(GtkGraph *graph,int index);
int gtk_graph_get_max(GtkGraph *graph);
void gtk_graph_set_num_lines(GtkGraph *graph, int num_lines);
GdkPixmap *gtk_graph_get_pixmap(GtkGraph *graph);
void gtk_graph_make_ps(GtkGraph *graph, const char *sFilename);
void gtk_graph_scale_set_type(GtkGraph *graph, enum gtk_graph_scale_type type);
void gtk_graph_set_calibration(GtkGraph *graph, float a, float b, float c);
void gtk_graph_set_type(GtkGraph *graph, enum gtk_graph_type type);
void gtk_graph_set_y_scale_mode(GtkGraph *graph, enum gtk_graph_y_scale_mode mode);

void gtk_graph_draw(GtkWidget *widget, GdkRectangle *area);

//--ddc aug11
double gtk_graph_y_scale(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_GRAPH_H__ */

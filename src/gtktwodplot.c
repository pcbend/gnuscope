/*
 * File: GtkTwodplot.c
 * Auth: John Pavan
 *
 * Modified for use with gnuscope, based on the
 * original code by Eric Harlow
 *
 * Simple gtk 2d plot widget
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "gtktwodplot.h"
#include <math.h>
#define MALLOC_CHECK_ = 2

static GtkWidgetClass *parent_class = NULL;

struct bigdata {
  int axes[2]; // number of channels per axis
  int *vals;   // pointer to the memory which holds the values
};
int printnotdone;
long double xdisplayused;
long double ydisplayused;
int ybinsize, xbinsize;
struct bigdata *gate_info, *big_data_info;
int xcurrentrange[2], ycurrentrange[2];
GdkGC *gc;

//--ddc 17jan06 add extern for contrast function...
//--ddc nov10 many gtk2 deprecations.  Globally change font->font_desc in
//      struct references
extern int twodmaxoverride;

/*
 * forward declorations
 */
int BigDataStructGetVal(struct bigdata *stru, int x, int y);
static void gtk_twodplot_class_init(GtkTwodplotClass *class);
static void gtk_twodplot_init(GtkTwodplot *twodplot);
static void gtk_twodplot_realize(GtkWidget *widget);
void gtk_twodplot_refresh(GtkWidget *widget);
//--ddc jun11 gtk_widget_draw deprecation
// static void gtk_twodplot_draw (GtkWidget *widget, GdkRectangle *area);
void gtk_twodplot_draw(GtkWidget *widget, GdkRectangle *area);
void gtk_twodplot_redraw(GtkWidget *widget, GdkRectangle *area);
static void gtk_twodplot_size_request(GtkWidget *widget, GtkRequisition *req);
static gint gtk_twodplot_expose(GtkWidget *widget, GdkEventExpose *event);
static void gtk_twodplot_destroy(GtkObject *object);
void DrawColoredRectangle(GdkWindow *window, gint filled, gint x, gint y, gint width, gint height,
                          const char *colorparse);
//--ddc jul11
// deprecation of draw_text... requires 'widget' passed to DrawColoredText.

void DrawColoredText(GdkDrawable *drawable, GtkWidget *widget, const char *colorparse, gint x,
                     gint y, const char *text);

void DrawRGBColoredRectangle(GdkDrawable *drawable, GdkGC *gcd, gint x, gint y, gint width,
                             gint height, int red, int green, int blue);

/*
 * gtk_twodplot_get_type
 * Internal class. Used to define the GtkGraph class to GTK+
 *
 */
//--ddc aug11 guint gtk_twodplot_get_type (void)
GType gtk_twodplot_get_type(void) {
  //--ddc aug11
  static GType twodplot_type = 0;

  /* --- If not created yet --- */
  if (!twodplot_type) {
    /* --- Create a graph_info object --- */
    //--ddc gtk2 deprecations in this structure...
    /*
    GtkTypeInfo twodplot_info =
    {
      "GtkTwodplot",
      sizeof(GtkTwodplot),
      sizeof(GtkTwodplotClass),
      (GtkClassInitFunc) gtk_twodplot_class_init,
      (GtkObjectInitFunc) gtk_twodplot_init,
      NULL,
      NULL,
    };
    */
    const GTypeInfo twodplot_info = {sizeof(GtkTwodplotClass),
                                     NULL,
                                     NULL,
                                     (GClassInitFunc)gtk_twodplot_class_init,
                                     NULL,
                                     NULL,
                                     sizeof(GtkTwodplot),
                                     0,
                                     (GInstanceInitFunc)gtk_twodplot_init};
    /* --- Tell GTK+ about it - get a unique identifying key --- */
    //--ddc aug11     twodplot_type = gtk_type_unique(gtk_widget_get_type(),
    twodplot_type = g_type_register_static(gtk_widget_get_type(), "GtkTwodplot", &twodplot_info, 0);
  }
  return twodplot_type;
}

/*
 * gtk_twodplot_class_init
 *
 * Override any methods for the twodplot class that are needed for
 * the graph class to behave properly.  Here, the functions that
 * cause painting to occur are overriden.
 *
 * class - object definition class.
 */
static void gtk_twodplot_class_init(GtkTwodplotClass *class) {
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  /* --- get the widget class --- */
  object_class = (GtkObjectClass *)class;
  widget_class = (GtkWidgetClass *)class;
  parent_class = gtk_type_class(gtk_widget_get_type());

  /* --- Override object destroy --- */
  object_class->destroy = gtk_twodplot_destroy;

  /* --- Override these widget methods --- */
  widget_class->realize = gtk_twodplot_realize;
  //--ddc deprecated  widget_class->draw = gtk_twodplot_draw;
  widget_class->size_request = gtk_twodplot_size_request;
  widget_class->expose_event = gtk_twodplot_expose;
}

/*
 * gtk_twodplot_init
 *
 * Called each time a graph item gets created.
 * This initializes fields in our structure.
 */
static void gtk_twodplot_init(GtkTwodplot *twodplot) {
  GtkWidget *widget;

  widget = (GtkWidget *)twodplot;

  /* --- initial values --- */
  twodplot->values = NULL;
  twodplot->num_values = 0;
  twodplot->x_num = 0;
  twodplot->y_num = 0;
  twodplot->pixmap = NULL;
  twodplot->num_contours = 10;
  twodplot->plot_mode = GTK_TWODPLOT_DENSITY_ONLY;
  twodplot->contour_mode = GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_LOG;
  twodplot->density_mode = GTK_TWODPLOT_DENSITY_TYPE_COLOR;
  twodplot->interpolation_mode = GTK_TWODPLOT_INTERPOLATION_QUADRATIC;
  twodplot->fade_mode = GTK_TWODPLOT_FADE_ON;
  twodplot->gate = NULL;
  twodplot->x_frac = 1;
  twodplot->y_frac = 1;
  twodplot->xcalibs[0] = 0;
  twodplot->xcalibs[1] = 1;
  twodplot->xcalibs[2] = 0;
  twodplot->ycalibs[0] = 0;
  twodplot->ycalibs[1] = 1;
  twodplot->ycalibs[2] = 0;
  twodplot->num_markers = 0;
  twodplot->markers_x = NULL;
  twodplot->markers_y = NULL;
  twodplot->marker_text = NULL;
  twodplot->color_depth = 256;
  sprintf(twodplot->title, "GTK_2D");
}

/*
 * gtk_twodplot_new
 *
 * Create a new GtkTwodplot item
 */
GtkWidget *gtk_twodplot_new(void) {
  return gtk_type_new(gtk_twodplot_get_type());
}

/*
 * gtk_twodplot_realize
 *
 * Associate the widget with an X window.
 */
static void gtk_twodplot_realize(GtkWidget *widget) {
  GtkTwodplot *darea;
  GdkWindowAttr attributes;
  gint attributes_mask;

  /* --- Check for failures --- */
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(widget));

  darea = GTK_TWODPLOT(widget);
  GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

  /* --- attributes to create the window --- */
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual(widget);
  attributes.colormap = gtk_widget_get_colormap(widget);
  attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

  /* --- We're passing in x, y, visiual and colormap values --- */
  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

  /* --- Create the Window --- */
  widget->window =
      gdk_window_new(gtk_widget_get_parent_window(widget), &attributes, attributes_mask);
  gdk_window_set_user_data(widget->window, darea);

  widget->style = gtk_style_attach(widget->style, widget->window);
  gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
}

/*
 * gtk_twodplot_set_value
 *
 * Custom Method to set a value
 */
void gtk_twodplot_set_value(GtkTwodplot *twodplot, int x, int y, float value) {
  g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));
  g_return_if_fail(x < twodplot->x_num && x >= 0);
  g_return_if_fail(y < twodplot->y_num && y >= 0);

  twodplot->values[x][y] = value;
}

/* gtk_twodplot_set_color_depth
 *
 * Sets the number of colors to use when rendering
 */
void gtk_twodplot_set_color_depth(GtkTwodplot *twodplot, int colordepth) {
  g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));
  g_return_if_fail(colordepth >= 0);
  twodplot->color_depth = colordepth;
}

/*
 * gtk_twodplot_set_size
 *
 * Custom Method to set size.
 */
void gtk_twodplot_set_size(GtkTwodplot *twodplot, int x, int y) {
  int i, j;

  g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));

  if (twodplot->values != NULL) {
    for (i = 0; i < twodplot->x_num; i++)
      g_free(twodplot->values[i]);
    g_free(twodplot->values);
  }

  if (twodplot->gate != NULL) {
    for (i = (twodplot->x_num - 1); i >= 0; i--)
      g_free(twodplot->gate[i]);
    g_free(twodplot->gate);
  }

  twodplot->x_num = x;
  twodplot->y_num = y;
  twodplot->num_values = x * y;
  twodplot->values = (float **)malloc(x * sizeof(float *));
  for (i = 0; i < x; i++) {
    twodplot->values[i] = (float *)malloc(sizeof(float) * y);
    for (j = 0; j < y; j++) {
      twodplot->values[i][j] = 0;
    }
  }
  twodplot->gate = (short int **)malloc(sizeof(short int *) * x);
  for (i = 0; i < x; i++) {
    twodplot->gate[i] = (short int *)malloc(sizeof(short int) * y);
    for (j = 0; j < y; j++) {
      twodplot->gate[i][j] = 0;
    }
  }
}

/* gtk_twodplot_set_num_contours
 *
 * Sets the number of isobars to plot
 */
void gtk_twodplot_set_num_contours(GtkTwodplot *twodplot, int num_contours) {
  g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));

  twodplot->num_contours = num_contours;
}

/* gtk_twodplot_set_x_range
 *
 * sets the x range for the axes on the twodplot
 */
void gtk_twodplot_set_x_range(GtkTwodplot *twodplot, int x_min, int x_max) {
  g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));

  twodplot->x_range[0] = x_min;
  twodplot->x_range[1] = x_max;
}

/* gtk_twodplot_set_y_range
 *
 * sets the y range for the axes of the twodplot
 */
void gtk_twodplot_set_y_range(GtkTwodplot *twodplot, int y_min, int y_max) {
  g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));

  twodplot->y_range[0] = y_min;
  twodplot->y_range[1] = y_max;
}

/* gtk_twodplot_toggle_gate
 *
 * Toggles on the gate for a particular channel in x and y
 */
void gtk_twodplot_toggle_gate(GtkTwodplot *twodplot, int x, int y) {
  g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));

  twodplot->gate[x][y] = 1;
}

/*
 * gtk_twodplot_draw
 *
 * Draw the widget.  Draws the widget based on the
 * number of values in the 2D data.  (makes a relief plot)
 */

void gtk_twodplot_draw(GtkWidget *widget, GdkRectangle *area) {
  GtkTwodplot *twodplot;
  int width;
  char dummystr[20];
  int height;
  int x_column_width, y_column_width;
  float max = 0;
  float min = 0;
  int i, j, k, l, m, n, z;
  int bar_height, next_bar_height;
  GdkPoint points[4];
  float tempsum;
  char colorstr[40];
  int result;
  int pixoff;
  long double currentscaling;
  int currentmax;
  float redval = 0, greenval = 0, blueval = 0;
  int tempint;
  float contour_vals[100];
  gboolean bool;
  GdkColor color;
  float mag, intensity;
  guchar *tempbuffer;
  int tempbufferptr;
  guchar *red_vals, *green_vals, *blue_vals;

  //--ddc 17jan06 contrast addition
  int itempcontrasted;

  //--ddc gtk_text deprecation..
  PangoLayout *layout = gtk_widget_create_pango_layout(widget, NULL);
  int x, y;

  xbinsize = 1;
  ybinsize = 1;

  /* --- check for obvious problems --- */
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(widget));

  /* --- make sure it's a drawable widget --- */
  //--ddc aug12, part of the never ending deprecations.  if (GTK_WIDGET_DRAWABLE(widget)) {
  if (gtk_widget_is_drawable(widget)) {

    twodplot = GTK_TWODPLOT(widget);
    if (twodplot->num_values == 0) {
      return;
    }

    /* --- if we don't have a pixmap we need one -- */

    if (twodplot->pixmap != NULL)
      g_object_unref(twodplot->pixmap);

    twodplot->pixmap =
        gdk_pixmap_new(widget->window, widget->allocation.width, widget->allocation.height, -1);

    /* --- Get height and width --- */
    width = widget->allocation.width - 30;
    height = widget->allocation.height - 30;

    /* --- draw a black square on the area of the plot --- */
    gdk_draw_rectangle(twodplot->pixmap, widget->style->black_gc, 1, 0, 0, widget->allocation.width,
                       widget->allocation.height);

    /* --- Calculate width of the columns --- */
    x_column_width = (float)(width * xbinsize) / (float)twodplot->x_num;
    y_column_width = (float)(height * ybinsize) / (float)twodplot->y_num;
    while (x_column_width < 1) {
      x_column_width = (float)width * (float)xbinsize / (float)twodplot->x_num;
      xbinsize = xbinsize + 1;
    }
    while (y_column_width < 1) {
      y_column_width = (float)height * (float)ybinsize / (float)twodplot->y_num;
      ybinsize = ybinsize + 1;
    }

    /* --- Find the max value --- */
    /* --- this is slightly more difficult with different x and y bins --- */
    max = 0;
    /* --- also find the min value so we don't
       --- flub up when we find negative values --- */
    min = 0;
    for (i = 0; i <= (twodplot->x_num - xbinsize); i = i + xbinsize) {
      for (j = 0; j <= (twodplot->y_num - ybinsize); j = j + ybinsize) {
        tempsum = 0;
        for (k = 0; k < xbinsize; k++) {
          for (l = 0; l < ybinsize; l++) {
            tempsum = tempsum + twodplot->values[i + k][j + l];
          }
        }
        if (max < tempsum)
          max = tempsum;
        if (min > tempsum)
          min = tempsum;
      }
    }

    /* --- allocate memory for and populate the height to color arrays --- */
    red_vals = (guchar *)malloc(sizeof(guchar) * twodplot->color_depth);
    green_vals = (guchar *)malloc(sizeof(guchar) * twodplot->color_depth);
    blue_vals = (guchar *)malloc(sizeof(guchar) * twodplot->color_depth);
    /* --- now to popluate these --- */

    //--ddc 17jan06 There is a one-off error in filling the color arrays
    //--ddc though not completely clear the intention, for consistency,
    //--ddc if the color goes from 0-255, then in his loop his scaling factors
    //--ddc must be i/(color_depth MINUS ONE)!!!  UNKNOWN... did original author
    //--ddc try to compensate for this (in log scales...)?  I only ask, because of
    //--ddc of the odd fraction added here and there.
    for (i = 0; i < twodplot->color_depth; i++) {
      if ((twodplot->plot_mode == GTK_TWODPLOT_DENSITY_ONLY) ||
          (twodplot->plot_mode == GTK_TWODPLOT_CONTOUR_AND_DENSITY)) {
        if (twodplot->density_mode == GTK_TWODPLOT_DENSITY_TYPE_COLOR) {
          if (i == 0) {
            redval = 0;
            greenval = 0;
            blueval = 0;
          } else {
            switch (twodplot->contour_mode) {
            case GTK_TWODPLOT_CONTOUR_TYPE_LINEAR:
              //--ddc		mag = (float) i / (float) twodplot->color_depth * 3;
              mag = (float)i / (float)(twodplot->color_depth - 1) * 3;
              break;
            case GTK_TWODPLOT_CONTOUR_TYPE_LOG:
              //--ddc		mag = 3 * (log((float)i + 1.01) /
              //			   log((float)twodplot->color_depth + 1.1));
              mag = 3 * (log((float)i + 1.01) / log((float)(twodplot->color_depth - 1) + 1.1));

              break;
            case GTK_TWODPLOT_CONTOUR_TYPE_ROOT:
              //--ddc mag = 3 * sqrt((float)i / (float)twodplot->color_depth);
              mag = 3 * sqrt((float)i / (float)(twodplot->color_depth - 1));
              break;
            case GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_LOG:
              //--ddc		mag = 3 * (1 - (log((float)i + 1.01) /
              //	log((float)twodplot->color_depth + 1.1)));
              mag =
                  3 * (1 - (log((float)i + 1.01) / log((float)(twodplot->color_depth - 1) + 1.1)));

              break;
            case GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_ROOT:
              //--ddc   mag = 3 * (1 - sqrt((float)i / (float)twodplot->color_depth));
              mag = 3 * (1 - sqrt((float)i / (float)(twodplot->color_depth - 1)));
            default:
              break;
            };
            if (mag < 0.9999) {
              /* --- this is the region where we are moving from blue to green --- */
              blueval = mag;
              greenval = 1 - mag;
              redval = 1;
            } else if (mag < 1.9999) {
              /* --- this is the region where we are moving from green to red --- */
              greenval = mag - 1;
              redval = 2 - mag;
              blueval = 1;
            } else if (i == twodplot->color_depth) {
              /* --- this is easy, just blue --- */
              redval = 1;
              blueval = greenval = 0;
            } else {
              /* --- going from red back to blue --- */
              redval = mag - 2;
              blueval = 3 - mag;
              greenval = 1;
            }

            /* --- normalize and set the intensity --- */
            switch (twodplot->fade_mode) {
            case GTK_TWODPLOT_FADE_ON:
              switch (twodplot->contour_mode) {
              case GTK_TWODPLOT_CONTOUR_TYPE_LINEAR:
                //--ddc	  intensity = 255 * (float)i / (float)twodplot->color_depth;
                intensity = 255 * (float)i / (float)(twodplot->color_depth - 1);
                break;
              case GTK_TWODPLOT_CONTOUR_TYPE_LOG:
                //--ddc		  intensity =
                //  255 * log((float)i + 1.01) / log((float)twodplot->color_depth + 1.01) ;
                intensity =
                    255 * log((float)i + 1.01) / log((float)twodplot->color_depth - 1 + 1.01);
                break;
              case GTK_TWODPLOT_CONTOUR_TYPE_ROOT:
                //--ddc  intensity = 255 * sqrt((float)i / (float)twodplot->color_depth);
                intensity = 255 * sqrt((float)i / (float)(twodplot->color_depth - 1));
                break;
              case GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_LOG:
                //--ddc  intensity = 255 * (1 - log((float) i + 1.01) /
                //log((float)twodplot->color_depth+ 1.01));
                intensity =
                    255 * (1 - log((float)i + 1.01) / log((float)twodplot->color_depth - 1 + 1.01));
                break;
              case GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_ROOT:
                intensity = 255 * (1 - sqrt((float)i / (float)twodplot->color_depth));
              default:
                break;
              };
              break;
            case GTK_TWODPLOT_FADE_OFF:
              intensity = 255;
              break;
            default:
              break;
            }
            mag = sqrt(pow(redval, 2) + pow(blueval, 2) + pow(greenval, 2));
            redval = redval / mag * intensity;
            blueval = blueval / mag * intensity;
            greenval = greenval / mag * intensity;
          }
        }
        if (twodplot->density_mode == GTK_TWODPLOT_DENSITY_TYPE_BW) {
          if (i == 0)
            redval = greenval = blueval = 0;
          else
            redval = greenval = blueval =
                256 * sqrt(log(i + .01) / log(twodplot->color_depth + .01));
        }

      } else {
        redval = 0;
        greenval = 0;
        blueval = 0;
      }
      red_vals[i] = redval;
      green_vals[i] = greenval;
      blue_vals[i] = blueval;
    } // done populating the color mapping tables

    /* --- display data --- */

    //--ddc 26jan06 if max is zero, make 1 to prevent divide by zero.
    if (max < 0.1)
      max = 1;

    currentmax = max;

    //--ddc 17jan06 change max for contrast
    max = max * pow(2, twodmaxoverride);

    xdisplayused = (long double)((long double)(widget->allocation.width - 31) /
                                 (long double)twodplot->x_num / (long double)x_column_width);
    ydisplayused = (long double)((long double)(widget->allocation.height - 31) /
                                 (long double)twodplot->y_num / (long double)y_column_width);
    gc = gdk_gc_new(widget->window);

    /* --- Display each bar graph --- */
    {

      /* --- interpolation is only used if it is turned on
         --- and if the resolution is sufficent to
         --- use interpolation --- */

      /* --- unlike the way this was perviously done, let's try to do this by constructing
         --- the pixmap --- */

      /* --- very local variables --- */
      float tempfloatx, tempfloaty;
      float **tempheight;
      float tempterm;
      int tempx, tempy;
      int mx[100], my[100], px[100], py[100];
      int xcoords[201], ycoords[201];
      int numpoints = 0;

      tempheight = (float **)malloc(sizeof(float *) * width);
      for (i = 0; i < width; i++) {
        tempheight[i] = (float *)malloc(sizeof(float) * height);
      }
      tempbuffer = (guchar *)malloc(sizeof(guchar) * width * height * 3);
      tempbufferptr = 0;

      for (j = height - 1; j >= 0; j--) {
        for (i = 0; i < width; i++) {

          //--ddc 19jan06.  There'll be no interpolation if there aren't
          //--ddc enough points!!! Add conditional test here...
          if (twodplot->x_num <= twodplot->interpolation_mode ||
              twodplot->y_num <= twodplot->interpolation_mode) {
            tempheight[i][j] = 0;
            // Interpolation not possible!!
          } else { //--ddc there are enough points, continue.

            tempheight[i][j] = 0;
            /* --- there are actually two versions --- */
            /* --- first if the interpolation_mode is odd --- */
            if (twodplot->interpolation_mode % 2) {
              tempfloatx = (float)i / (float)width * (float)twodplot->x_num;
              tempfloaty =
                  (float)twodplot->y_num - (float)j / (float)height * (float)twodplot->y_num;
              /* --- ok we want to get the closest points to this --- */
              /* --- we do this by rounding down both and then adding --- */
              tempx = tempfloatx - 0.5;
              tempy = tempfloaty - 0.5;
              numpoints = twodplot->interpolation_mode + 1;
              if (tempfloatx >= (twodplot->x_num - numpoints)) {
                tempx = twodplot->x_num - numpoints - 1;
              } else {
                if (tempfloatx < numpoints)
                  tempx = numpoints;
                else
                  tempx = tempfloatx;
              }
              if (tempfloaty >= (twodplot->y_num - numpoints)) {
                tempy = twodplot->y_num - numpoints - 1;
              } else {
                if (tempfloaty < numpoints)
                  tempy = numpoints;
                else
                  tempy = tempfloaty;
              }
              /* --- we want numpoints to round down --- */
              for (k = 0; k < numpoints; k++) {
                xcoords[k] = tempx - twodplot->interpolation_mode / 2 + k;
                ycoords[k] = tempy - twodplot->interpolation_mode / 2 + k;
              }
            } else {
              /* --- if the interpolation mode is even --- */
              tempfloatx = (float)i / (float)width * (float)twodplot->x_num;
              tempfloaty =
                  (float)twodplot->y_num - (float)j / (float)height * (float)twodplot->y_num;
              numpoints = twodplot->interpolation_mode / 2;
              /* --- make sure that everything will end up in bounds --- */
              if (tempfloatx >= (twodplot->x_num - numpoints)) {
                tempx = twodplot->x_num - numpoints - 1;
              } else {
                if (tempfloatx < numpoints)
                  tempx = numpoints;
                else
                  tempx = tempfloatx;
              }
              if (tempfloaty >= (twodplot->y_num - numpoints)) {
                tempy = twodplot->y_num - numpoints - 1;
              } else {
                if (tempfloaty < numpoints)
                  tempy = numpoints;
                else
                  tempy = tempfloaty;
              }
              /* --- now let's assign get the mx's,px's,my's, and py's --- */
              for (k = 0; k < numpoints; k++) {
                mx[k] = tempx + k - numpoints;
                px[k] = tempx + k + 1;
                my[k] = tempy + k - numpoints;
                py[k] = tempy + k + 1;
              }
              /* --- now let's transfer the coordinates to a single array --- */
              l = 0;
              for (k = 0; k < numpoints; k++) {
                xcoords[l] = mx[k];
                ycoords[l] = my[k];
                l++;
              }
              xcoords[l] = tempx;
              ycoords[l] = tempy;
              l++;
              for (k = 0; k < numpoints; k++) {
                xcoords[l] = px[k];
                ycoords[l] = py[k];
                l++;
              }
            }

            /* --- now we have our coordinates in xcoords, ycoords, tempfloatx, and tempfloaty ---
             */
            /* --- let's do the actual interpolation --- */

            for (k = 0; k <= twodplot->interpolation_mode; k++) { // for interpolationmode + 1 terms
              for (l = 0; l <= twodplot->interpolation_mode; l++) {
                tempterm = twodplot->values[xcoords[k]][ycoords[l]] - min;
                for (m = 0; m <= twodplot->interpolation_mode; m++) {
                  if (m != k) {
                    tempterm *= (tempfloatx - xcoords[m]);
                    tempterm /= (xcoords[k] - xcoords[m]);
                  } // else tempterm = 0;
                  if (m != l) {
                    tempterm *= (tempfloaty - ycoords[m]);
                    tempterm /= (ycoords[l] - ycoords[m]);
                  } // else tempterm = 0;
                }
                tempheight[i][j] += tempterm;
              } // done cycling through the y coordinates
            } // done cycling through the x coordinates
            if (tempheight[i][j] < 0)
              tempheight[i][j] = 0; //--ddc NO NEGATIVES!
          } //--ddc done interpolating
        }
      }

      /* --- display contours --- */
      if ((twodplot->plot_mode == GTK_TWODPLOT_CONTOUR_ONLY) ||
          (twodplot->plot_mode == GTK_TWODPLOT_CONTOUR_AND_DENSITY)) {

        switch (twodplot->contour_mode) {
        case GTK_TWODPLOT_CONTOUR_TYPE_LINEAR:
          for (i = 0; i < twodplot->num_contours; i++)
            contour_vals[i] = (float)max / (float)(twodplot->num_contours + 1) * (float)(i + 1);
          break;
        case GTK_TWODPLOT_CONTOUR_TYPE_LOG:
          for (i = 0; i < twodplot->num_contours; i++)
            contour_vals[i] = (float)max * log(i + 1.1) / log(twodplot->num_contours + 1.1);
          break;
        case GTK_TWODPLOT_CONTOUR_TYPE_ROOT:
          for (i = 0; i < twodplot->num_contours; i++)
            contour_vals[i] = (float)max * sqrt(i + 1) / sqrt(twodplot->num_contours);
          break;
        case GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_LOG:
          for (i = 0; i < twodplot->num_contours; i++)
            contour_vals[i] = (float)max * (1 - (log(i + 1.1) / log(twodplot->num_contours + 1.1)));
          break;
        case GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_ROOT:
          for (i = 0; i < twodplot->num_contours; i++)
            contour_vals[i] = (float)max * (1 - (sqrt(i + 1) / sqrt(twodplot->num_contours))) + 0.1;
          break;
        default:
          /* --- linear --- */
          for (i = 0; i < twodplot->num_contours; i++)
            contour_vals[i] = (float)max / (float)(twodplot->num_contours + 1) * (float)(i + 1);
          twodplot->contour_mode = GTK_TWODPLOT_CONTOUR_TYPE_LINEAR;
          break;
        };
      }

      /* --- now we set the color information for that pixel --- */

      for (j = height - 1; j >= 0; j--) {
        for (i = 0; i < width; i++) {

          //--ddc 17jan06 adding 'contrast' here.
          //--ddc all computations with tempcontrasted below are changed for this
          //--ddc ALSO, there was a one-off error in computing array index (oops)
          //--ddc fixed that by here too.
          itempcontrasted = (twodplot->color_depth - 1) * tempheight[i][j] / max;
          if (itempcontrasted > 255) {
            itempcontrasted = 255;
          }

          redval = red_vals[itempcontrasted];
          blueval = blue_vals[itempcontrasted];
          greenval = green_vals[itempcontrasted];

          if ((twodplot->plot_mode == GTK_TWODPLOT_CONTOUR_ONLY) ||
              (twodplot->plot_mode == GTK_TWODPLOT_CONTOUR_AND_DENSITY)) {
            if ((i > 0) && (i < (width - 1)) && (j > 0) && (j < (height - 1))) {
              for (k = 0; k < twodplot->num_contours; k++) {
                if (((tempheight[i][j] <= contour_vals[k]) &&
                     (tempheight[i + 1][j] >= contour_vals[k])) ||
                    ((tempheight[i][j] <= contour_vals[k]) &&
                     (tempheight[i][j + 1] >= contour_vals[k])) ||
                    ((tempheight[i][j] <= contour_vals[k]) &&
                     (tempheight[i - 1][j] >= contour_vals[k])) ||
                    ((tempheight[i][j] <= contour_vals[k]) &&
                     (tempheight[i][j - 1] >= contour_vals[k]))) {
                  redval = fabs((double)(255 - redval));
                  greenval = fabs((double)(255 - greenval));
                  blueval = fabs((double)(255 - blueval));
                }
              }
            }
          }

          //--ddc 23jan06 index computation off (array access out of bounds)
          //	  if (twodplot->gate[(int)((float) i / (float) width * (float) twodplot->x_num)]
          //	      [(int)((float) twodplot->y_num -
          //		     (float) j / (float) height * (float) twodplot->y_num)]) {

          if (twodplot->gate[(int)((float)i / (float)width * (float)twodplot->x_num)]
                            [(int)((float)twodplot->y_num -
                                   (float)j / (float)height * (float)twodplot->y_num - 1)]) {
            redval = fabs((double)(128 - ((int)redval % 256)));
            greenval = fabs((double)(128 - ((int)greenval % 256)));
            blueval = fabs((double)(128 - ((int)blueval % 256)));
          }

          *(tempbuffer + i * 3 + j * width * 3) = (guchar)(int)redval;
          *(tempbuffer + i * 3 + j * width * 3 + 1) = (guchar)(int)greenval;
          *(tempbuffer + i * 3 + j * width * 3 + 2) = (guchar)(int)blueval;
        }
      }

      gdk_draw_rgb_image(twodplot->pixmap, widget->style->fg_gc[GTK_STATE_NORMAL], 31, 0, width,
                         height, GDK_RGB_DITHER_MAX, tempbuffer, width * 3);

      for (k = width - 1; k >= 0; k--)
        free(tempheight[k]);
      free(tempheight);
      free(tempbuffer);
    }
    //--ddc jul11 modified call arguements for deprecations.
    DrawColoredText(twodplot->pixmap, widget, "white", (widget->allocation.width) * 7 / 10, 15,
                    twodplot->title);

    //--ddc. 17jan06  I don't know why we have to CALCULATE all these
    //--ddc   colors again!  Comment the loop out and start over, using the
    //--ddc   the lookup!
    if ((twodplot->plot_mode == GTK_TWODPLOT_DENSITY_ONLY) ||
        (twodplot->plot_mode == GTK_TWODPLOT_CONTOUR_AND_DENSITY)) {
      //--ddc 17jan06 replaced a bunch of code in NEXT LOOP!...
      for (i = 10; i >= 1; i--) {
        itempcontrasted = (i * (twodplot->color_depth - 1)) / 10;
        redval = red_vals[itempcontrasted];
        blueval = blue_vals[itempcontrasted];
        greenval = green_vals[itempcontrasted];
        //--ddc mar11 gtk2 deprecation of rgbi...
        //	  sprintf(colorstr,"rgbi:%.2f/%.2f/%.2f",
        //		  redval/255,greenval/255,blueval/255);
        sprintf(colorstr, "#%.2x%.2x%.2x", (unsigned short)redval, (unsigned short)greenval,
                (unsigned short)blueval);

        //--ddc 17jan06 Unused code for contour removed here...

        sprintf(dummystr, "%d", (int)((float)max * (float)i / (float)(10)));
        //--ddc jul11 modified call arguements for deprecations.
        DrawColoredText(twodplot->pixmap, widget, colorstr, (widget->allocation.width) * 7 / 10,
                        15 + 12 * (11 - i), dummystr);
      }
    }

    /* --- axes --- */
    {
      GdkColor color;
      gboolean bool;
      int x_num_to_display, y_num_to_display;
      float x_display_interval, y_display_interval;
      int temp_channel;
      int x_string_size, y_string_size;

      bool = gdk_color_parse("white", &color);
      gdk_color_alloc(gtk_widget_get_default_colormap(), &color);
      gdk_gc_set_foreground((GdkGC *)gc, &color);

      points[0].x = 30;
      points[0].y = 0;
      points[1].x = 30;
      points[1].y = widget->allocation.height - 30;
      points[2].x = widget->allocation.width;
      points[2].y = widget->allocation.height - 30;
      gdk_draw_lines(twodplot->pixmap, gc, points, 3);

      /* --- ok we need to label the axes --- */

      /* --- determine the size of the characters --- */
      /*--ddc jul11
        there are multiple problems here... this is a bug in the original code uncorrected till
        this time.  Originally, the WIDTH the the strings for the vertical axis labels
        were used figure number of labels, and compute spacing... Should be the height!
        Some of this confusion probably arose from using 'y_column_width' as a name for
        the vertical height.. FURTHERMORE vertical position for the markers was NOT
        computed correctly (it was counting down) so there was (in the right views)
        a disconnect between labels and tic marks.
      */

      sprintf(dummystr, " %d ", twodplot->x_range[1]);

      pango_layout_set_text(layout, dummystr, strlen(dummystr));
      pango_layout_get_pixel_size(layout, &x_string_size, &y);

      sprintf(dummystr, " %d ", twodplot->y_range[1]);

      pango_layout_set_text(layout, dummystr, strlen(dummystr));
      //--ddc jul11 (oops, wrong dimension!) pango_layout_get_pixel_size(layout,&y_string_size,&y);
      pango_layout_get_pixel_size(layout, &x, &y_string_size);

      x_num_to_display = (float)width / (float)(x_string_size * 2);
      //--ddc jul11 (oops, wrong dimension!) y_num_to_display = (float) width / (float)
      //(y_string_size * 2);
      y_num_to_display = (float)height / (float)(y_string_size * 2);
      /*
      x_display_interval = (float) abs(twodplot->x_range[1] - twodplot->x_range[0]) /
      x_num_to_display; y_display_interval = (float) abs(twodplot->y_range[1] -
      twodplot->y_range[0]) / y_num_to_display;
      */
      //--ddc jul11 to prevent 'staggering in tic marks' you really need to round off higher integer
      // the display interval, and recompute the number of points.

      x_display_interval =
          ceil((float)abs(twodplot->x_range[1] - twodplot->x_range[0]) / x_num_to_display);
      y_display_interval =
          ceil((float)abs(twodplot->y_range[1] - twodplot->y_range[0]) / y_num_to_display);
      x_num_to_display =
          ceil((float)abs(twodplot->x_range[1] - twodplot->x_range[0]) / x_display_interval);
      y_num_to_display =
          ceil((float)abs(twodplot->y_range[1] - twodplot->y_range[0]) / y_display_interval);

      points[0].y = widget->allocation.height - 30;
      points[1].y = widget->allocation.height - 25;

      for (i = 0; i < x_num_to_display; i++) {
        temp_channel = twodplot->x_range[0] + x_display_interval * i + 1;
        result = (temp_channel - twodplot->x_range[0]) * x_column_width * xdisplayused;
        points[1].x = points[0].x = (int)result + 30;
        gdk_draw_lines(twodplot->pixmap, widget->style->white_gc, points, 2);
        sprintf(dummystr, "%d",
                (int)(twodplot->xcalibs[0] + temp_channel * twodplot->xcalibs[1] +
                      temp_channel * temp_channel * twodplot->xcalibs[2]));

        pango_layout_set_text(layout, dummystr, strlen(dummystr));
        pango_layout_get_pixel_size(layout, &x, &y);
        gdk_draw_layout(twodplot->pixmap, widget->style->white_gc, points[0].x - x / 2,
                        widget->allocation.height - 5 - y, layout);
      }
      points[0].x = 30;
      points[1].x = 35;
      for (i = 0; i < y_num_to_display; i++) {

        temp_channel = twodplot->y_range[0] + y_display_interval * i + 1;
        result = (temp_channel - twodplot->y_range[0]) * y_column_width * ydisplayused;
        //	points[1].y = points[0].y = (int)result;
        points[1].y = points[0].y = widget->allocation.height - 30 - (int)result;
        gdk_draw_lines(twodplot->pixmap, widget->style->white_gc, points, 2);

        sprintf(dummystr, "%d",
                (int)(twodplot->ycalibs[0] + temp_channel * twodplot->ycalibs[1] +
                      temp_channel * temp_channel * twodplot->ycalibs[2]));

        pango_layout_set_text(layout, dummystr, strlen(dummystr));
        pango_layout_get_pixel_size(layout, &x, &y);
        gdk_draw_layout(twodplot->pixmap, widget->style->white_gc, 1,
                        widget->allocation.height - 30 - result - y / 2, layout);
      }
    }

    /* --- draw markers --- */
    for (i = 0; i < twodplot->num_markers; i++) {
      gdk_draw_rectangle(twodplot->pixmap, gc, 0,
                         28 + (twodplot->markers_x[i] + 0.5) * x_column_width,
                         height - (twodplot->markers_y[i] + 0.5) * y_column_width - 2, 5, 5);

      DrawColoredText(
          twodplot->pixmap, widget, NULL, 34 + (twodplot->markers_x[i] + 0.5) * x_column_width,
          height - (twodplot->markers_y[i] + 0.5) * y_column_width - 3, twodplot->marker_text[i]);
    }

    /* --- clean up --- */

    //--ddc jan11 gtk deprecation    gdk_gc_destroy(gc);
    g_object_unref(gc);
    g_object_unref(layout);

    free(red_vals);
    free(blue_vals);
    free(green_vals);

    /* --- draw --- */

    gdk_draw_drawable(widget->window, widget->style->fg_gc[gtk_widget_get_state(widget)],
                      twodplot->pixmap, 0, 0, 0, 0, widget->allocation.width,
                      widget->allocation.height);
  }
}

/* DrawColoredText
 *
 * Draws text in a certain color
 */
void DrawColoredText(GdkDrawable *drawable, GtkWidget *widget, const char *colorparse, gint x,
                     gint y, const char *text) {
  GdkColor color;
  gboolean bool;
  PangoLayout *layout;

  bool = gdk_color_parse(colorparse, &color);

  if (colorparse != NULL) {
    gdk_color_alloc(gtk_widget_get_default_colormap(), &color);
    gdk_gc_set_foreground((GdkGC *)gc, &color);
  }

  /*--ddc jul11 gdk_draw_text deprecation...
  gdk_draw_text(drawable,font,gc,x,y,text,strlen(text));
  */

  layout = gtk_widget_create_pango_layout(widget, text);
  gdk_draw_layout(drawable, gc, x, y, layout);

  g_object_unref(layout);
}

/* DrawRGBColoredRectangle
 *
 * draws a colored rectangle
 */
void DrawRGBColoredRectangle(GdkDrawable *drawable, GdkGC *gcd, gint x, gint y, gint width,
                             gint height, int red, int green, int blue) {
  guchar *data, *blah;
  int i, j;

  if (height < 1)
    height = 1;
  if (width < 1)
    width = 1;

  data = (guchar *)malloc(sizeof(guchar) * width * 3);
  blah = data;
  for (i = 0; i < width; i++) {
    *blah++ = (guchar)red;
    *blah++ = (guchar)green;
    *blah++ = (guchar)blue;
  }
  gdk_draw_rgb_image(drawable, gcd, x, y, width, height, GDK_RGB_DITHER_MAX, data, 0);
  free(data);
}

/*
 * DrawColoredRectangle
 *
 * draws a colored rectangle
 */
void DrawColoredRectangle(GdkWindow *window, gint filled, gint x, gint y, gint width, gint height,
                          const char *colorparse) {

  GdkColor color;
  gboolean bool;

  bool = gdk_color_parse(colorparse, &color);
  gdk_color_alloc(gtk_widget_get_default_colormap(), &color);
  gdk_gc_set_foreground((GdkGC *)gc, &color);
  gdk_draw_rectangle(window, gc, filled, x, y, width, height);
}

/*
 * gtk_twodplot_size_request
 *
 * How big should the widget be?
 * It can be modified.
 */
static void gtk_twodplot_size_request(GtkWidget *widget, GtkRequisition *req) {

  req->width = gdk_screen_width() * GTK_TWODPLOT(widget)->x_frac;
  req->height = gdk_screen_height() * GTK_TWODPLOT(widget)->y_frac;
}

/*
 * gtk_twodplot_expose
 *
 * The twodplot widget has been exposed and needs to be painted.
 */
static gint gtk_twodplot_expose(GtkWidget *widget, GdkEventExpose *event) {
  GtkTwodplot *twodplot;

  /* --- Do error checking --- */
  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(GTK_IS_TWODPLOT(widget), FALSE);
  g_return_val_if_fail(event != NULL, FALSE);

  /* --- Get the graph widget --- */
  twodplot = GTK_TWODPLOT(widget);

  /* --- Draw the graph --- */
  /* --ddc jul11 this is fundamentally WRONG
  if (twodplot->pixmap) {
    gdk_draw_drawable(widget->window,
                    widget->style->fg_gc[gtk_widget_get_state (widget)],
                    twodplot->pixmap,
                    event->area.x,event->area.y,
                    event->area.x,event->area.y,
                    event->area.width, event->area.height);
    return FALSE;
  } else {
  gtk_twodplot_draw(widget,NULL);
  }
  */

  gtk_twodplot_draw(widget, NULL);
  gdk_draw_drawable(widget->window, widget->style->fg_gc[gtk_widget_get_state(widget)],
                    twodplot->pixmap, event->area.x, event->area.y, event->area.x, event->area.y,
                    event->area.width, event->area.height);
}

/* gtk_twodplot_destroy
 *
 * Destroy the widget and free up any allocated memory.
 * When done, call he parent destroy to make sure any
 * memory allocated by the parent is freed.
 */
static void gtk_twodplot_destroy(GtkObject *object) {
  int i;
  GtkTwodplot *deltwodplot;

  /* --- Check type --- */
  g_return_if_fail(object != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(object));

  /* --- Convert to a graph object --- */
  deltwodplot = GTK_TWODPLOT(object);

  /* --- Free memory --- */
  for (i = 0; i < deltwodplot->x_num; i++)
    g_free(deltwodplot->values[i]);
  g_free(deltwodplot->values);

  if (deltwodplot->pixmap != NULL)
    g_object_unref(deltwodplot->pixmap);

  for (i = 0; i < deltwodplot->x_num; i++)
    g_free(deltwodplot->gate[i]);
  g_free(deltwodplot->gate);

  /* --- call parent destroy --- */
  GTK_OBJECT_CLASS(parent_class)->destroy(object);
}

/*
 * gtk_twodplot_refresh
 *
 * redraws the twodplot from its pixmap
 */
void gtk_twodplot_refresh(GtkWidget *widget) {
  GtkTwodplot *twodplot;

  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(widget));

  twodplot = GTK_TWODPLOT(widget);

  g_return_if_fail(twodplot->pixmap != NULL);

  gdk_draw_drawable(widget->window, widget->style->fg_gc[gtk_widget_get_state(widget)],
                    twodplot->pixmap, 0, 0, 0, 0, widget->allocation.width - 1,
                    widget->allocation.height - 1);
}

/* gtk_twodplot_set_plot_mode
 *
 * sets the plot mode of the twodplot
 */
void gtk_twodplot_set_plot_mode(GtkTwodplot *twodplot, enum gtk_twodplot_type mode) {

  //--ddc  24may06 g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));

  twodplot->plot_mode = mode;
}

/* gtk_twodplot_make_ps
 *
 * Produces a file with sFilename acording to the twodplotmode
 */
void gtk_twodplot_make_ps(GtkTwodplot *twodplot, const char *sFilename, int publish) {
  FILE *outfile;
  int i, j, k, l, m, n, o, p;
  guint x, y, z;
  float fx, fy, fz;
  int max;
  float x_col_width, y_col_width;
  float tempx1, tempx2, tempx3;
  float tempy1, tempy2, tempy3;
  float xdif, ydif;
  guint whatval;
  int numdisplay;
  float tempfloat, tempfloat2;
  int a;
  float contour_vals[100];
  GdkPoint points[10];
  int numstrips;
  int biggest_i;
  float tempheight[100][100];
  float redval[100][100], greenval[100][100], blueval[100][100];
  float tempfloatx, tempfloaty;
  int xcoords[100], ycoords[100];
  int mx[50], px[50], my[50], py[50];
  int min, numpoints;
  int tempx, tempy;
  float tempterm;
  double intensity;
  double mag;

  g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));
  g_return_if_fail(twodplot->x_num > 0);
  g_return_if_fail(twodplot->y_num > 0);

  if ((outfile = fopen(sFilename, "w")) != NULL) {
    printnotdone = 1;
    fprintf(outfile, "%%!PS-Adobe-2.0\n");
    fprintf(outfile, "%%%%BoundingBox: 0 0 600 800\n");
    fprintf(outfile, "%%%%DocumentData: Clean7Bit\n");
    fprintf(outfile, "%%%%Orientation: Portrait\n");
    fprintf(outfile, "%%%%Pages:1\n");
    fprintf(outfile, "%%%%Title: gtktwodplot\n");
    fprintf(outfile, "%%%%PageOrder: Ascend\n");
    fprintf(outfile, "%%%%EndComments\n");
    fprintf(outfile, "0.95 0.95 scale\n");
    fprintf(outfile, "30 70 translate\n");

    /* --- setup the text --- */
    fprintf(outfile, "/Times-Roman findfont\n15 scalefont\nsetfont\n");

    /* --- we ought to print the title on the graph --- */
    fprintf(outfile, "560 300 moveto -90 rotate gsave 2 2 scale (%s)show 90 grestore rotate\n",
            twodplot->title);

    /* --- calculate the width of columns --- */
    x_col_width = -(float)500 / (float)twodplot->x_num;
    y_col_width = (float)500 / (float)twodplot->y_num;

    /* --- determine the max value --- */
    /* --- and the min value --- */

    max = 0;
    min = 0;
    for (i = 0; i < twodplot->x_num; i++) {
      for (j = 0; j < twodplot->y_num; j++) {
        if (max < twodplot->values[i][j])
          max = twodplot->values[i][j];
        if (min > twodplot->values[i][j])
          min = twodplot->values[i][j];
      }
    }

    /* --- we are going to break up the display into 100 x 100 chunks --- */
    for (i = 0; i < 5; i++) {
      for (j = 0; j < 5; j++) {
        /* --- let's start on the chunk --- */
        for (l = 99; l >= 0; l--) {
          for (k = 0; k < 100; k++) {
            /* --- two modes ->odd and even --- */
            if (twodplot->interpolation_mode % 2) {
              /* --- odd mode --- */
              printf("In odd mode \n");

            } else {
              /* --- even mode --- */
              tempfloatx = (float)(l + 100 * j) / (float)500 * (float)twodplot->x_num;
              tempfloaty = (float)(k + 100 * i) / (float)500 * (float)twodplot->y_num;
              numpoints = twodplot->interpolation_mode / 2;
              /* --- make sure that everything will end up in bounds --- */
              if (tempfloatx >= (twodplot->x_num - numpoints)) {
                tempx = twodplot->x_num - numpoints - 1;
              } else {
                if (tempfloatx < numpoints)
                  tempx = numpoints;
                else
                  tempx = tempfloatx;
              }
              if (tempfloaty >= (twodplot->y_num - numpoints)) {
                tempy = twodplot->y_num - numpoints - 1;
              } else {
                if (tempfloaty < numpoints)
                  tempy = numpoints;
                else
                  tempy = tempfloaty;
              }

              /* --- now let's assign get the mx's,px's,my's, and py's --- */
              for (m = 0; m < numpoints; m++) {
                mx[m] = tempx + m - numpoints;
                px[m] = tempx + m + 1;
                my[m] = tempy + m - numpoints;
                py[m] = tempy + m + 1;
              }
              /* --- now let's transfer the coordinates to a single array --- */
              n = 0;
              for (m = 0; m < numpoints; m++) {
                xcoords[n] = mx[m];
                ycoords[n] = my[m];
                n++;
              }
              xcoords[n] = tempx;
              ycoords[n] = tempy;
              n++;
              for (m = 0; m < numpoints; m++) {
                xcoords[n] = px[m];
                ycoords[n] = py[m];
                n++;
              }
            }
            /* --- now we have our coordinates --- */
            /* --- let's do the interpolation --- */
            tempheight[k][l] = 0;
            for (m = 0; m <= twodplot->interpolation_mode; m++) {
              for (n = 0; n <= twodplot->interpolation_mode; n++) {
                tempterm = twodplot->values[xcoords[m]][ycoords[n]] - min;
                for (o = 0; o <= twodplot->interpolation_mode; o++) {
                  if (o != m) {
                    tempterm *= tempfloatx - xcoords[o];
                    tempterm /= xcoords[m] - xcoords[o];
                  }
                  if (o != n) {
                    tempterm *= tempfloaty - ycoords[o];
                    tempterm /= ycoords[n] - ycoords[o];
                  }
                }

                tempheight[k][l] += tempterm;
              }
            }

          } // done wiht k
        } // done with l

        /* --- if there are contours we need to know the values to use --- */

        /* --- set the color information for the pixel --- */

        if ((twodplot->plot_mode == GTK_TWODPLOT_DENSITY_ONLY) ||
            (twodplot->plot_mode == GTK_TWODPLOT_CONTOUR_AND_DENSITY)) {
          for (l = 99; l >= 0; l--) {
            for (k = 0; k < 100; k++) {
              if (tempheight[k][l] > max)
                tempheight[k][l] = max;
              if (twodplot->plot_mode == GTK_TWODPLOT_DENSITY_TYPE_BW) {
                if (tempheight[k][l] < 1)
                  redval[k][l] = greenval[k][l] = blueval[k][l] = 255;
                else
                  redval[k][l] = greenval[k][l] = blueval[k][l] =
                      255 - 255 * log(tempheight[k][l] + 0.01) / log(max + 0.01);
              }
              if (twodplot->density_mode == GTK_TWODPLOT_DENSITY_TYPE_COLOR) {
                if (tempheight[k][l] < 1) {
                  redval[k][l] = 255;
                  greenval[k][l] = 255;
                  blueval[k][l] = 255;
                } else {

                  switch (twodplot->contour_mode) {
                  case GTK_TWODPLOT_CONTOUR_TYPE_LINEAR:
                    mag = (float)tempheight[k][l] / (float)max * 3;
                    break;
                  case GTK_TWODPLOT_CONTOUR_TYPE_LOG:
                    mag = 3 * log(tempheight[k][l] + 1.1) / log(max + 1.1);
                    break;
                  case GTK_TWODPLOT_CONTOUR_TYPE_ROOT:
                    mag = 3 * sqrt(tempheight[k][l] / max);
                    break;
                  case GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_LOG:
                    mag = 3 * (1 - log(tempheight[k][l] + 1.1) / log(max + 1.1));
                    break;
                  case GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_ROOT:
                    mag = 3 * (1 - sqrt(tempheight[k][l] / max));
                    break;
                  default:
                    break;
                  };
                  mag = (float)tempheight[k][l] / (float)max * 3;
                  if (mag < 0.99999) {
                    blueval[k][l] = 1 - mag;
                    greenval[k][l] = mag;
                    redval[k][l] = 0;
                  } else if (mag < 1.9999) {
                    greenval[k][l] = 2 - mag;
                    redval[k][l] = mag - 1;
                    blueval[k][l] = 0;
                  } else {
                    redval[k][l] = 3 - mag;
                    blueval[k][l] = mag - 2;
                    greenval[k][l] = 0;
                  }

                  mag = sqrt(pow(redval[k][l], 2) + pow(blueval[k][l], 2) + pow(greenval[k][l], 2));
                  /* --- normalize and set the intensity --- */
                  switch (twodplot->fade_mode) {
                  case GTK_TWODPLOT_FADE_ON:
                    switch (twodplot->contour_mode) {
                    case GTK_TWODPLOT_CONTOUR_TYPE_LINEAR:
                      intensity = 255 * tempheight[k][l] / max;
                      break;
                    case GTK_TWODPLOT_CONTOUR_TYPE_LOG:
                      intensity = 255 * log(tempheight[k][l] + 1.01) / log(max + 1.01);
                      break;
                    case GTK_TWODPLOT_CONTOUR_TYPE_ROOT:
                      intensity = 255 * sqrt(tempheight[k][l] / max);
                      break;
                    case GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_LOG:
                      intensity = 255 * (1 - log(tempheight[k][l] + 1.01) / log(max + 1.01));
                      break;
                    case GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_ROOT:
                      intensity = 255 * (1 - sqrt(tempheight[k][l] / max));
                    default:
                      break;
                    };
                    break;
                  case GTK_TWODPLOT_FADE_OFF:
                    intensity = 255;
                    break;
                  default:
                    break;
                  };

                  redval[k][l] /= mag;
                  blueval[k][l] /= mag;
                  greenval[k][l] /= mag;
                  redval[k][l] *= intensity;
                  blueval[k][l] *= intensity;
                  greenval[k][l] *= intensity;
                  redval[k][l] = 255 - redval[k][l];
                  greenval[k][l] = 255 - greenval[k][l];
                  blueval[k][l] = 255 - blueval[k][l];
                }
              }
            }
          }
        } // done setting color information
        /* --- now let's print it --- */

        if ((twodplot->plot_mode == GTK_TWODPLOT_DENSITY_ONLY) ||
            (twodplot->plot_mode == GTK_TWODPLOT_CONTOUR_AND_DENSITY)) {

          fprintf(outfile, "gsave\n %f %f translate\n -90 rotate\n 100 100 scale\n",
                  (float)(30 + 100 * i), (float)(500 - 100 * j));
          fprintf(outfile, "100 100 8 [100 0 0 100 0 0] {< ");
          for (k = 0; k < 100; k++) {
            for (l = 0; l < 100; l++) {
              fprintf(outfile, "%02x %02x %02x ", (guchar)(int)redval[k][l],
                      (guchar)(int)greenval[k][l], (guchar)(int)blueval[k][l]);
            }
          }
          fprintf(outfile, " >}\n false 3 colorimage\n grestore\n");
        }
        /* --- deal with the contour output if we are supposed to do that --- */

        if ((twodplot->plot_mode == GTK_TWODPLOT_CONTOUR_ONLY) ||
            (twodplot->plot_mode == GTK_TWODPLOT_CONTOUR_AND_DENSITY)) {
          fprintf(outfile, "0 0 0 setrgbcolor\n");

          /* --- we need to set the values of the isobars, and provide some information to the user
             --- */
          fprintf(outfile, "gsave\n");
          fprintf(outfile, "0.125 setlinewidth\n");
          if (publish)
            fprintf(outfile, "1 setgray\n");

          switch (twodplot->contour_mode) {
          case GTK_TWODPLOT_CONTOUR_TYPE_LINEAR:
            for (k = 0; k < twodplot->num_contours; k++)
              contour_vals[k] = (float)max / (float)(twodplot->num_contours + 1) * (float)(k + 1);
            fprintf(outfile,
                    "300 730 moveto -90 rotate (Linearly Spaced Contours) show 90 rotate\n",
                    (int)max);
            fprintf(outfile, "280 730 moveto -90 rotate (Max = %d) show 90 rotate\n", (int)max);
            fprintf(outfile, "260 730 moveto -90 rotate (Contour Spacing = %d) show 90 rotate\n",
                    (int)((float)max / (float)(twodplot->num_contours + 1)));
            break;
          case GTK_TWODPLOT_CONTOUR_TYPE_LOG:
            for (k = 0; k < twodplot->num_contours; k++)
              contour_vals[k] = (float)max * log(k + 1.1) / log(twodplot->num_contours + 1.1);
            fprintf(outfile, "300 730 moveto -90 rotate (Logrythmic Contours) show 90 rotate\n",
                    (int)max);
            fprintf(outfile, "280 730 moveto -90 rotate (Max = %d) show 90 rotate\n", (int)max);
            fprintf(outfile, "260 730 moveto -90 rotate (%d Contours) show 90 rotate\n",
                    (int)twodplot->num_contours);
            fprintf(outfile, "240 730 moveto -90 rotate (Sample Isobar Values:) show 90 rotate\n",
                    (int)((float)max / (float)(twodplot->num_contours + 1)));
            if (twodplot->num_contours > 10)
              l = 10;
            else
              l = twodplot->num_contours;
            for (k = 0; k < l; k++) {
              m = (float)twodplot->num_contours * (float)k / l;
              if (m >= twodplot->num_contours)
                m = twodplot->num_contours - 1;
              fprintf(outfile, "%f %f moveto -90 rotate (%d = %d) show 90 rotate\n",
                      (float)220 - 20 * (k % 5), (float)730 - 72 * ((int)((float)k / (float)5) % 2),
                      m + 1, (int)contour_vals[m]);
            }
            break;
          case GTK_TWODPLOT_CONTOUR_TYPE_ROOT:
            for (k = 0; k < twodplot->num_contours; k++)
              contour_vals[k] = (float)max * sqrt(k + 1) / sqrt(twodplot->num_contours);
            fprintf(outfile, "300 730 moveto -90 rotate (Root Contours) show 90 rotate\n",
                    (int)max);
            fprintf(outfile, "280 730 moveto -90 rotate (Max = %d) show 90 rotate\n", (int)max);
            fprintf(outfile, "260 730 moveto -90 rotate (%d Contours) show 90 rotate\n",
                    (int)twodplot->num_contours);
            fprintf(outfile, "240 730 moveto -90 rotate (Sample Isobar Values:) show 90 rotate\n",
                    (int)((float)max / (float)(twodplot->num_contours + 1)));
            if (twodplot->num_contours > 10)
              l = 10;
            else
              l = twodplot->num_contours;
            for (k = 0; k < l; k++) {
              m = (float)twodplot->num_contours * (float)k / l;
              if (m >= twodplot->num_contours)
                m = twodplot->num_contours - 1;
              fprintf(outfile, "%f %f moveto -90 rotate (%d = %d) show 90 rotate\n",
                      (float)220 - 20 * (k % 5), (float)730 - 72 * ((int)((float)k / (float)5) % 2),
                      m + 1, (int)contour_vals[m]);
            }
            break;
          case GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_LOG:
            for (k = 0; k < twodplot->num_contours; k++)
              contour_vals[k] =
                  (float)max * (1 - (log(k + 1.1) / log(twodplot->num_contours + 1.1)));
            fprintf(outfile,
                    "300 730 moveto -90 rotate (Inverse Logrythmic Contours) show 90 rotate\n",
                    (int)max);
            fprintf(outfile, "280 730 moveto -90 rotate (Max = %d) show 90 rotate\n", (int)max);
            fprintf(outfile, "260 730 moveto -90 rotate (%d Contours) show 90 rotate\n",
                    (int)twodplot->num_contours);
            fprintf(outfile, "240 730 moveto -90 rotate (Sample Isobar Values:) show 90 rotate\n",
                    (int)((float)max / (float)(twodplot->num_contours + 1)));
            if (twodplot->num_contours > 10)
              l = 10;
            else
              l = twodplot->num_contours;
            for (k = 0; k < l; k++) {
              m = (float)twodplot->num_contours * (float)k / l;
              if (m >= twodplot->num_contours)
                m = twodplot->num_contours - 1;
              fprintf(outfile, "%f %f moveto -90 rotate (%d = %d) show 90 rotate\n",
                      (float)220 - 20 * (k % 5), (float)730 - 72 * ((int)((float)k / (float)5) % 2),
                      m + 1, (int)contour_vals[m]);
            }
            break;
          case GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_ROOT:
            for (k = 0; k < twodplot->num_contours; k++)
              contour_vals[k] =
                  (float)max * (1 - (sqrt(k + 1) / sqrt(twodplot->num_contours))) + 0.1;
            fprintf(outfile, "300 730 moveto -90 rotate (Inverse Root Contours) show 90 rotate\n",
                    (int)max);
            fprintf(outfile, "280 730 moveto -90 rotate (Max = %d) show 90 rotate\n", (int)max);
            fprintf(outfile, "260 730 moveto -90 rotate (%d Contours) show 90 rotate\n",
                    (int)twodplot->num_contours);
            fprintf(outfile, "240 730 moveto -90 rotate (Sample Isobar Values:) show 90 rotate\n",
                    (int)((float)max / (float)(twodplot->num_contours + 1)));
            if (twodplot->num_contours > 10)
              l = 10;
            else
              l = twodplot->num_contours;
            for (k = 0; k < l; k++) {
              m = (float)twodplot->num_contours * (float)k / l;
              if (m >= twodplot->num_contours)
                m = twodplot->num_contours - 1;
              fprintf(outfile, "%f %f moveto -90 rotate (%d = %d) show 90 rotate\n",
                      (float)220 - 20 * (k % 5), (float)730 - 72 * ((int)((float)k / (float)5) % 2),
                      m + 1, (int)contour_vals[m]);
            }
            break;
          default:
            /* --- linear --- */
            for (k = 0; k < twodplot->num_contours; k++)
              contour_vals[k] = (float)max / (float)(twodplot->num_contours + 1) * (float)(k + 1);
            twodplot->contour_mode = GTK_TWODPLOT_CONTOUR_TYPE_LINEAR;
            break;
          };
          /* --- now that we have the isobar values we need to make the lines --- */
          /* --- oh crud, how do we set the colors? --- */
          /* --- let's set the colors so they are the inverse of the colors for the
             --- density plot --- */
          for (m = 0; m < twodplot->num_contours; m++) {

            if (twodplot->density_mode == GTK_TWODPLOT_DENSITY_TYPE_COLOR) {
              if (m == 0) {
                x = 0;
                y = 0;
                z = 0;
              } else {
                if (((float)m / (float)twodplot->num_contours) < 0.3333) {
                  fz = (float)m / (float)twodplot->num_contours * (float)3;
                  fy = 1 - (float)m / (float)twodplot->num_contours * (float)3;
                  fx = 1;
                } else if (((float)m / (float)twodplot->num_contours) < 0.6666) {
                  fz = 1;
                  fy = (((float)i / (float)twodplot->num_contours) - 0.3333) * 3;
                  fx = 1 - (((float)i / (float)twodplot->num_contours) - 0.3333) * 3;
                } else {
                  fz = 1 - ((float)m / (float)twodplot->num_contours - 0.6666) * 3;
                  fy = 1;
                  fx = ((float)m / (float)twodplot->num_contours - 0.6666) * 3;
                }

                fx = (float)fx / sqrt(pow(fx, 2) + pow(fy, 2) + pow(fz, 2));
                fy = (float)fy / sqrt(pow(fx, 2) + pow(fy, 2) + pow(fz, 2));
                fz = (float)fz / sqrt(pow(fx, 2) + pow(fy, 2) + pow(fz, 2));
              }
            }
            if (twodplot->density_mode == GTK_TWODPLOT_DENSITY_TYPE_BW) {
              fx = fy = fz = 0;
            }
            /* --- now we have the color for the line set --- */
            /* --- now let's cycle through the interpolated points to try to
               --- find the places to put the isobars --- */
            fprintf(outfile, "%f %f %f setrgbcolor\n", fabs(fx), fabs(fy), fabs(fz));
            for (k = 1; k < 99; k++) {
              for (l = 1; l < 99; l++) {
                if ((tempheight[k][l] <= contour_vals[m]) &&
                    (tempheight[k + 1][l] >= contour_vals[m])) {
                  fprintf(outfile, "newpath\n %f %f moveto\n %f %f lineto\n stroke\n",
                          k + 100 * i + 30.5, 500 - (l - 0.5 + 100 * j), k + 100 * i + 30.5,
                          500 - (l + 0.5 + 100 * j));
                }
                if ((tempheight[k][l] <= contour_vals[m]) &&
                    (tempheight[k][l + 1] >= contour_vals[m])) {
                  fprintf(outfile, "newpath\n %f %f moveto\n %f %f lineto\n stroke\n",
                          k + 100 * i + 29.5, 500 - (l + 0.5 + 100 * j), k + 100 * i + 30.5,
                          500 - (l + 0.5 + 100 * j));
                }
                if ((tempheight[k][l] <= contour_vals[m]) &&
                    (tempheight[k - 1][l] >= contour_vals[m])) {

                  fprintf(outfile, "newpath\n %f %f moveto\n %f %f lineto\n stroke\n",
                          k + 100 * i + 29.5, 500 - (l - 0.5 + 100 * j), k + 100 * i + 29.5,
                          500 - (l + 0.5 + 100 * j));
                }
                if ((tempheight[k][l] <= contour_vals[m]) &&
                    (tempheight[k][l - 1] >= contour_vals[m])) {
                  fprintf(outfile, "newpath\n %f %f moveto\n %f %f lineto\n stroke\n",
                          k + 100 * i + 29.5, 500 - (l - 0.5 + 100 * j), k + 100 * i + 30.5,
                          500 - (l - 0.5 + 100 * j));
                }
                if ((tempheight[k][l] <= contour_vals[m]) &&
                    (tempheight[k + 1][l + 1] >= contour_vals[m])) {
                  fprintf(outfile, "newpath\n %f %f moveto\n %f %f lineto\n stroke\n",
                          k + 100 * i + 30, 500 - (l + 0.5 + 100 * j), k + 100 * i + 30.5,
                          500 - (l + 100 * j));
                }
                if ((tempheight[k][l] <= contour_vals[m]) &&
                    (tempheight[k + 1][l - 1] >= contour_vals[m])) {
                  fprintf(outfile, "newpath\n %f %f moveto\n %f %f lineto\n stroke\n",
                          k + 100 * i + 30, 500 - (l - 0.5 + 100 * j), k + 100 * i + 30.5,
                          500 - (l + 100 * j));
                }
                if ((tempheight[k][l] <= contour_vals[m]) &&
                    (tempheight[k - 1][l - 1] >= contour_vals[m])) {
                  fprintf(outfile, "newpath\n %f %f moveto\n %f %f lineto\n stroke\n",
                          k + 100 * i + 30, 500 - (l - 0.5 + 100 * j), k + 100 * i + 29.5,
                          500 - (l + 100 * j));
                }
                if ((tempheight[k][l] <= contour_vals[m]) &&
                    (tempheight[k - 1][l + 1] >= contour_vals[m])) {
                  fprintf(outfile, "newpath\n %f %f moveto\n %f %f lineto\n stroke\n",
                          k + 100 * i + 30, 500 - (l + 0.5 + 100 * j), k + 100 * i + 29.5,
                          500 - (l + 100 * j));
                }
              }
            }
          } // done cycling through the contour values
        }
      }
    }
    fprintf(outfile, "0 0 0 setrgbcolor\n");
    {
      /* --- we should probably have a scale of some sort --- */
      fprintf(outfile, "530 700 moveto\n");
      fprintf(outfile, "gsave\n");
      for (i = 0; i <= 10; i++) {
        if (twodplot->density_mode == GTK_TWODPLOT_DENSITY_TYPE_COLOR) {
          if (i == 0) {
            x = 255;
            y = 255;
            z = 255;
          } else {
            if (((float)i / (float)10) < 0.3333) {
              fz = 1 - (float)i / (float)10 * (float)3;
              fy = (float)i / (float)10 * (float)3;
              fx = 0;
            } else if (((float)i / (float)10) < 0.6666) {
              fz = 0;
              fy = 1 - ((float)i / (float)10 - 0.3333) * 3;
              fx = ((float)i / (float)10 - 0.3333) * 3;
            } else if (((float)i / (float)10) < 0.9999) {
              fz = ((float)i / (float)10 - 0.6666) * 3;
              fy = 0;
              fx = 1 - ((float)i / (float)10 - 0.6666) * 3;
            } else {
              fz = 1;
              fx = fy = 0;
            }

            fx = (float)fx / sqrt(pow(fx, 2) + pow(fy, 2) + pow(fz, 2));
            fy = (float)fy / sqrt(pow(fx, 2) + pow(fy, 2) + pow(fz, 2));
            fz = (float)fz / sqrt(pow(fx, 2) + pow(fy, 2) + pow(fz, 2));

            switch (twodplot->contour_mode) {
            case GTK_TWODPLOT_CONTOUR_TYPE_LINEAR:
              intensity = 255 * (float)i / (float)10;
              break;
            case GTK_TWODPLOT_CONTOUR_TYPE_LOG:
              intensity = 255 * log(i + 1.01) / log(11.01);
              break;
            case GTK_TWODPLOT_CONTOUR_TYPE_ROOT:
              intensity = 255 * sqrt((float)i / (float)10);
              break;
            case GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_LOG:
              intensity = 255 * (1 - log(i) / log(11.01));
              break;
            case GTK_TWODPLOT_CONTOUR_TYPE_INVERSE_ROOT:
              intensity = 255 * (1 - sqrt((float)i / (float)10));
            default:
              break;
            };
            x = 255 - fx * intensity;
            y = 255 - fy * intensity;
            z = 255 - fz * intensity;
          }
        }
        if (twodplot->density_mode == GTK_TWODPLOT_DENSITY_TYPE_BW) {
          x = y = z = 255 - 255 * sqrt(log(i + 0.01) / log(10.01));
        }

        if (x >= 256)
          x = 0;
        if (y >= 256)
          y = 0;
        if (z >= 256)
          z = 0;
        fprintf(outfile,
                "gsave %d 700 translate 15 15 scale 1 1 8 [1 0 0 1 0 0] {<%02x %02x %02x>} false 3 "
                "colorimage grestore\n",
                530 - 20 * i, x, y, z);
        fprintf(outfile, "%d 690 moveto -90 rotate (%d) show 90 rotate\n", 530 - 20 * i,
                (int)(max * (float)i / (float)10));
      }
      fprintf(outfile, "grestore\n");
    }

    /* --- draw the markers (if there are any) --- */
    for (i = 0; i < twodplot->num_markers; i++) {
      /* --- determine the location of the marker on the plot --- */
      fprintf(outfile,
              "newpath\n %f %f moveto 0 -5 rlineto -5 0 rlineto 0 5 rlineto closepath stroke\n",
              y_col_width * (twodplot->markers_y[i] + 0.5) + 32.5,
              x_col_width * (twodplot->markers_x[i] + 0.5) + 502.5);
      fprintf(outfile, "%f %f moveto -90 rotate (%s) show 90 rotate\n",
              y_col_width * (twodplot->markers_y[i] + 0.5) + 35,
              x_col_width * (twodplot->markers_x[i] + 0.5) + 495, twodplot->marker_text[i]);
    }

    /* --- draw the axes and the bounding box --- */

    fprintf(
        outfile,
        "newpath\n30 0 moveto 0 500 rlineto 500 0 rlineto 0 -500 rlineto -500 0 rlineto stroke\n");

    /* --- determine the location of the hash marks and write the labels --- */
    /* --- first the y scale --- */
    numdisplay = (float)500 / (float)15 / (float)7;
    tempfloat2 = (int)((float)pow(10, (int)log10(twodplot->y_num)) / (float)4);
    while ((twodplot->y_num / tempfloat2) > numdisplay)
      tempfloat2 = tempfloat2 * 2;
    for (i = 0; (i * tempfloat2) <= twodplot->y_num; i++) {
      tempfloat = tempfloat2 * i;
      tempy1 = tempfloat * y_col_width + 30;
      fprintf(outfile, "newpath\n %f 500 moveto 0 -10 rlineto stroke\n", tempy1);
      fprintf(outfile, "%f 550 moveto\n -90 rotate\n (%d) show\n 90 rotate\n", tempy1 - 7,
              (int)tempfloat + ycurrentrange[0]);
    }

    numdisplay = (float)500 / (float)15 / (float)7;
    tempfloat2 = (int)((float)pow(10, (int)log10(twodplot->x_num)) / (float)4);
    while ((twodplot->x_num / tempfloat2) > numdisplay)
      tempfloat2 = tempfloat2 * 2;
    for (i = 0; (i * tempfloat2) <= twodplot->x_num; i++) {
      tempfloat = tempfloat2 * i;
      tempx1 = tempfloat * x_col_width + 500;
      fprintf(outfile, "newpath\n 30 %f moveto 10 0 rlineto stroke\n", tempx1);
      fprintf(outfile, "0 %f moveto\n -90 rotate\n (%d) show\n 90 rotate\n",
              (tempx1 + 7 * (int)log10(tempfloat + 0.9)), (int)tempfloat + xcurrentrange[0]);
    }

    fprintf(outfile, "showpage\n");
    fprintf(outfile, "%%EOF\n");
    fclose(outfile);
    printnotdone = 0;
  } else {
    printf("Error opening output file.\n");
  }
}

/* gtk_twodplot_set_contour_type
 *
 * sets the mode for determining the isobars for the contour plot
 */
void gtk_twodplot_set_contour_type(GtkTwodplot *twodplot, enum gtk_twodplot_contour_type type) {
  g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));

  twodplot->contour_mode = type;
}

/* gtk_twodplot_set_density_type
 * Sets the mode for determining the colors in the density plot
 */
void gtk_twodplot_set_density_type(GtkTwodplot *twodplot, enum gtk_twodplot_density_type type) {
  g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));

  twodplot->density_mode = type;
}

/* gtk_twodplot_set_interpolation_type
 * sets the method of interpolation
 */
void gtk_twodplot_set_interpolation_type(GtkTwodplot *twodplot,
                                         enum gtk_twodplot_interpolation_type type) {
  /* --- currently only type is off --- */
  g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));

  twodplot->interpolation_mode = type;
}

/* gtk_twodplot_set_title
 *
 * sets the title of the twodplot
 */
void gtk_twodplot_set_title(GtkTwodplot *twodplot, const char *title) {
  g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));

  snprintf(twodplot->title, sizeof(twodplot->title), "%s", title);
}

/* gtk_twodplot_set_x_calibration
 *
 * Sets the calibration for the x axis of the twodplot
 */
void gtk_twodplot_set_x_calibration(GtkTwodplot *twodplot, float a, float b, float c) {
  g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));

  twodplot->xcalibs[0] = a;
  twodplot->xcalibs[1] = b;
  twodplot->xcalibs[2] = c;
}

/* gtk_twodplot_set_y_calibration
 *
 * Sets the calibration for the y axis of the twodplot
 */
void gtk_twodplot_set_y_calibration(GtkTwodplot *twodplot, float a, float b, float c) {

  g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));

  twodplot->ycalibs[0] = a;
  twodplot->ycalibs[1] = b;
  twodplot->ycalibs[2] = c;
}

/* gtk_twodplot_set_fade
 *
 * Sets the fade mode (on/off) for the twodplot
 */
void gtk_twodplot_set_fade(GtkTwodplot *twodplot, enum gtk_twodplot_fade fade) {
  g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));

  twodplot->fade_mode = fade;
}

/* gtk_twodplot_set_frac
 *
 * Sets the portion of the screen for the twodplot to occupy
 */
void gtk_twodplot_set_frac(GtkTwodplot *twodplot, float x, float y) {
  g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));

  twodplot->x_frac = x;
  twodplot->y_frac = y;
}

/* gtk_twodplot_add_marker
 *
 * Adds a marker to the plot, x and y should be in channels
 */
void gtk_twodplot_add_marker(GtkTwodplot *twodplot, float x, float y, char *text) {
  int i;
  g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));

  /* --- incrament the number of markers, and make space in memory --- */
  if (twodplot->num_markers <= 0) {
    twodplot->num_markers = 1;
    twodplot->markers_x = (float *)malloc(sizeof(float));
    twodplot->markers_y = (float *)malloc(sizeof(float));
    twodplot->marker_text = (char **)malloc(sizeof(char *));
    twodplot->marker_text[0] = (char *)malloc(sizeof(char) * 8);
  } else {
    twodplot->num_markers++;
    twodplot->markers_x =
        (float *)realloc(twodplot->markers_x, sizeof(float) * twodplot->num_markers);
    twodplot->markers_y =
        (float *)realloc(twodplot->markers_y, sizeof(float) * twodplot->num_markers);
    twodplot->marker_text =
        (char **)realloc(twodplot->marker_text, sizeof(char *) * twodplot->num_markers);
    twodplot->marker_text[twodplot->num_markers - 1] = (char *)malloc(sizeof(char) * 8);
  }
  /* --- now actually put the info in the structs --- */
  twodplot->markers_x[twodplot->num_markers - 1] = x;
  twodplot->markers_y[twodplot->num_markers - 1] = y;
  if (strlen(text) < 8)
    i = strlen(text);
  else
    i = 8;
  strncpy(twodplot->marker_text[twodplot->num_markers - 1], text, i);
}

/* gtk_twodplot_clear_markers
 *
 * deletes all markers
 */
void gtk_twodplot_clear_markers(GtkTwodplot *twodplot) {
  int i;
  g_return_if_fail(twodplot != NULL);
  g_return_if_fail(GTK_IS_TWODPLOT(twodplot));

  if (twodplot->num_markers > 0) {
    for (i = twodplot->num_markers - 1; i >= 0; i--) {
      free(twodplot->marker_text[i]);
    }
    twodplot->num_markers = 0;
    free(twodplot->markers_x);
    free(twodplot->markers_y);
    free(twodplot->marker_text);
  }
}

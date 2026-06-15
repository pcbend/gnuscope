/*
 * File: GtkGraph.c
 * Auth: John Pavan
 *
 * Modified for use with gnuscope, based on the
 * original code by Eric Harlow
 *
 * Simple gtk graphing widget
 */


#include <stdlib.h>

#include "gtkgraph.h"
#include <gdk/gdk.h>
//#include <gdk/gdktypes.h>
//#include <gdk/gdkrgb.h>
#include <math.h>
#include <glib.h>
#include "gnuscopefuncs.h"
#include "gnuscopeglobals.h"

int yscale,ybin,yscaleforce,ybinforce;

static GtkWidgetClass *parent_class = NULL;

int column_width;
int binsizeforce;
int binsize;
int currentmax;
int printnotdone;
long double displayused;
int plottype;
int currentrange[2];
int numspectra;

/*
 * forward declorations
 */
static void gtk_graph_class_init (GtkGraphClass *class);
static void gtk_graph_init (GtkGraph *graph);
static void gtk_graph_realize (GtkWidget *widget);
//--ddc feb11 gtk2 deprecation of gtk_widget_draw
//--ddc feb11  static void gtk_graph_draw (GtkWidget *widget, GdkRectangle *area);
static void gtk_graph_size_request (GtkWidget *widget, GtkRequisition *req);
static gint gtk_graph_expose (GtkWidget *widget, GdkEventExpose *event);
static void gtk_graph_destroy (GtkObject *object);

/*
 * gtk_graph_get_type
 * Internal class. Used to define the GtkGraph class to GTK+
 *
 */

GType gtk_graph_get_type (void)
{

  static GType graph_type = 0;
  
  /* --- If not created yet --- */
  if (!graph_type) {
    /* --- Create a graph_info object --- */

    const GTypeInfo graph_info = 
    {
      sizeof(GtkGraphClass),
      NULL,
      NULL,
      (GClassInitFunc) gtk_graph_class_init,
      NULL,
      NULL,
      sizeof(GtkGraph),
      0,
      (GInstanceInitFunc) gtk_graph_init
    };
    
    /* --- Tell GTK+ about it - get a unique identifying key --- */

    graph_type = g_type_register_static(gtk_widget_get_type(),"GtkGraph",
				 &graph_info, 0);
  }
  return graph_type;
}

/*
 * gtk_graph_class_init
 * 
 * Override any methods for the graph class that are needed for
 * the graph class to behave properly.  Here, the functions that
 * cause painting to occur are overriden.
 *
 * class - object definition class.
 */
static void gtk_graph_class_init (GtkGraphClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  /* --- get the widget class --- */
  object_class = (GtkObjectClass *) class;
  widget_class = (GtkWidgetClass *) class;
  parent_class = gtk_type_class (gtk_widget_get_type());


  /* --- Override object destroy --- */
  object_class->destroy = gtk_graph_destroy;

  /* --- Override these widget methods --- */
  widget_class->realize = gtk_graph_realize;
  //--ddc in gtk2, no draw member now (only expose)...
  //widget_class->draw = gtk_graph_draw;
  widget_class->size_request = gtk_graph_size_request;
  widget_class->expose_event = gtk_graph_expose;
}

/*
 * gtk_graph_init
 *
 * Called each time a graph item gets created.
 * This initializes fields in our structure.
 */
static void gtk_graph_init(GtkGraph *graph)
{
  GtkWidget *widget;


  widget = (GtkWidget *) graph;

  /* --- initial values --- */
  graph->values = NULL;
  graph->num_values = 0;
  graph->title = NULL;
  graph->num_segments = 0;
  graph->segments_x = NULL;
  graph->segments_y = NULL;
  graph->pixmap = NULL;
  graph->scale_type = GTK_GRAPH_SCALE_CHANNELS;
  graph->type = GTK_GRAPH_TYPE_LINEAR;
  graph->y_scale_mode = GTK_GRAPH_Y_SCALE_AUTO_ON;
}

/*
 * gtk_graph_new
 *
 * Create a new GtkGraph item
 */
GtkWidget* gtk_graph_new (void)
{
  return gtk_type_new (gtk_graph_get_type());
}

/*
 * gtk_graph_realize
 *
 * Associate the widget with an X window.
 */
static void gtk_graph_realize (GtkWidget *widget)
{
  GtkGraph *darea;
  GdkWindowAttr attributes;
  gint attributes_mask;

  /* --- Check for failures --- */
  //--ddc 24may06  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_WIDGET(widget));
  g_return_if_fail (GTK_IS_GRAPH (widget));

  darea = GTK_GRAPH (widget);
  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

  /* --- attributes to create the window --- */
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);
  attributes.event_mask = gtk_widget_get_events (widget) |
    GDK_EXPOSURE_MASK;
  
  /* --- We're passing in x, y, visiual and colormap values --- */
  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

  /* --- Create the Window --- */
  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
				  &attributes, attributes_mask);
  gdk_window_set_user_data (widget->window, darea);
  
  widget->style = gtk_style_attach (widget->style, widget->window);
  gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
}

/* gtk_graph_set_num_lines
 *
 * custom method to set the number of histograms being displayed
 */
void gtk_graph_set_num_lines(GtkGraph *graph, int num_lines) 
{
  int i;
  g_return_if_fail(graph != NULL);
  g_return_if_fail(GTK_IS_GRAPH (graph));
  graph->num_lines = num_lines;
  graph->num_values = 0;
  if (graph->values == NULL) {
    graph->values = (gint **) malloc(sizeof(gint *) * graph->num_lines);
  } else {
    graph->values = (gint **) realloc(graph->values,
				     sizeof(gint *) * graph->num_lines);
  }
  for (i = 0; i < graph->num_lines; i++) {
    graph->values[i] = NULL;
  }
  if (graph->title == NULL) {
    graph->title = (char **) malloc(sizeof(char *) * graph->num_lines);
  } else {
    graph->title = (char **) realloc(graph->title,
				     sizeof(char *) * graph->num_lines);
  }
  for (i = 0; i < graph->num_lines; i++) {
    graph->title[i] = (char *) malloc(sizeof(char) * 40);
  }
}

/* gtk_graph_size
 *
 * Custom method to set the size of the graph.
 */
void gtk_graph_size (GtkGraph*graph, int size)
{
  int i;
  g_return_if_fail (graph != NULL);
  g_return_if_fail (GTK_IS_GRAPH (graph));
  
  graph->num_values = size;
  for (i = 0; i < graph->num_lines; i++) {
    graph->values[i] = (gint *) g_realloc (graph->values[i], sizeof (gint) *size);
  }
}


/* 
 * gtk_graph_set_value
 *
 * Custom Method to set size.
 */
void gtk_graph_set_value (GtkGraph *graph,int linenum, int index, int value)
{
  g_return_if_fail (graph!= NULL);
  g_return_if_fail (GTK_IS_GRAPH(graph));
  g_return_if_fail ((linenum >= 0) && (linenum < graph->num_lines));
  g_return_if_fail ((index < graph->num_values) && (index >=0));

  graph->values[linenum][index] = value;
}

/*
 * gtk_graph_clear_segments
 *
 * clears the segments
 */
void gtk_graph_clear_segments(GtkGraph *graph)
{
  g_return_if_fail (graph!=NULL);
  g_return_if_fail (GTK_IS_GRAPH(graph));
  g_return_if_fail (graph->num_segments > 0);
  
  graph->num_segments = 0;
  free(graph->segments_x);
  free(graph->segments_y);
}


/*
 * gtk_graph_has_segments
 *
 * returns 1 if the graph has segments, 0 otherwise
 */
int gtk_graph_has_segments (GtkGraph *graph)
{
  g_return_val_if_fail (graph!=NULL,0);
  g_return_val_if_fail (GTK_IS_GRAPH(graph),0);

  return (graph->num_segments);
}

/*
 * gtk_graph_add_segment(GtkGraph *graph, GdkPoint *point)
 *
 * adds a point to segments
 */
void gtk_graph_add_segment(GtkGraph *graph, float x, float y)
{
  g_return_if_fail (graph !=NULL);
  g_return_if_fail (GTK_IS_GRAPH(graph));
  
  /* --- two cases, one if the there are no segments --- */
  if (graph->num_segments == 0) {
    graph->num_segments = 1;
    graph->segments_x = (float *) malloc(sizeof(float));
    graph->segments_y = (float *) malloc(sizeof(float));
    graph->segments_x[0] = x;
    graph->segments_y[0] = y;
  } else {
    if (graph->num_segments > 0) {
      graph->num_segments = graph->num_segments + 1;
      graph->segments_x = (float *) realloc(graph->segments_x,
					    graph->num_segments * sizeof(float));
      graph->segments_y = (float *) realloc(graph->segments_y,
					    graph->num_segments * sizeof(float));
      graph->segments_x[graph->num_segments - 1] = x;
      graph->segments_y[graph->num_segments - 1] = y;
    }
  }
}

/*
 * gtk_graph_title
 *
 * sets the title of the graph
 */
void gtk_graph_title (GtkGraph *graph, int linenum, const char *title)
{
  //--ddc newer versions of widgets handle linefeed differently..
  //      zero it.
  char *linefeed;
  g_return_if_fail (graph!=NULL);
  g_return_if_fail (GTK_IS_GRAPH (graph));
  g_return_if_fail ((linenum >= 0) && (linenum < graph->num_lines));
  sprintf(graph->title[linenum],title);
  if((linefeed=strchr(graph->title[linenum],'\n')) != NULL ) *linefeed=0;

}

/* gtk_graph_scale_set_type
 *
 * sets 2 types:
 * GTK_GRAPH_SCALE_CHANNELS = x-scale is in channels
 * GTK_GRAPH_SCALE_CALIBRATED = x-scale uses calibration (provided there is one)
 */
void gtk_graph_scale_set_type(GtkGraph *graph,enum gtk_graph_scale_type type)
{
  //--ddc 24may06  g_return_if_fail(graph != NULL);
  g_return_if_fail(GTK_IS_GRAPH(graph));
  graph->scale_type = type;
}

/* gtk_graph_set_calibration
 *
 * sets the polynomial calibration
 * a = constant
 * b = linear
 * c = quadratic
 */
void gtk_graph_set_calibration(GtkGraph *graph, float a, float b, float c)
{
  //--ddc 24may06  g_return_if_fail(graph != NULL);
  g_return_if_fail(GTK_IS_GRAPH(graph));

  graph->calibration[0] = a;
  graph->calibration[1] = b;
  graph->calibration[2] = c;
}

/* gtk_graph_get_max
 *
 * Returns the current value of max (as an integer)
 */
int gtk_graph_get_max(GtkGraph *graph)
{
  int i,j,k,max;
  int tempsum;

  g_return_val_if_fail(graph != NULL,0);
  g_return_val_if_fail(GTK_IS_GRAPH(graph),0);

  //-ddc aug11 'max' was uninitialized... havoc may ensue when compared 
  //to tempsum
  max=0;

  if (binsize > 0) {
    
    /* --- Find the max value --- */
    /* --- The max value can be modified by the user using
       --- yscaleforce (yscale) and ybinforce(ybin)
    */
    if ( yscaleforce == 1) {
      max = yscale;
    } else {
      for (j = 0; j < graph->num_lines; j++) {
        //--ddc aug11 why not all values?
	//	for (i = binsize; i < (graph->num_values - binsize); i=i + binsize) {
	for (i = binsize; i < graph->num_values ; i=i + binsize) {
	  tempsum = 0;
	  for(j = 0; j < binsize; j++){
	    tempsum = tempsum + graph->values[j][i+j];
	  }
	  if (max < tempsum) {
	    max = tempsum;
	  }
	}
      }
    }
  } else {

    max=max*gtk_graph_y_scale();
  }
  max = max * 1.1; /* to make it look pretty */
  if (max < 10) max = 10;

  return(max);
}


/* gtk_graph_set_type
 *
 * sets the type of graph
 */
void gtk_graph_set_type(GtkGraph *graph,enum gtk_graph_type type)
{
  //--ddc 24may06  g_return_if_fail(graph != NULL);
  g_return_if_fail(GTK_IS_GRAPH(graph));

  graph->type = type;
}

/*
 * gtk_graph_draw
 *
 * Draw the widget.  Draws the widget based on the
 * number of values in the bar graph.
 */

void gtk_graph_draw (GtkWidget *widget, GdkRectangle*area)
{
  GtkGraph *graph;
  int width;
  char dummystr[20];
  int numdisplay;
  int height;
  long int max[10];
  long int min[10];
  int i,j,k,z,l;
  long int bar_height, next_bar_height;
  GdkPoint points[3];
  long int tempsum,tempavg;
  int pixoff;
  long double currentscaling;
  GdkGC *gc;
  GdkColor color;
  int string_size,num_to_display,temp_channel;
  float display_interval;
  float result2;
  gint prevx;
  gboolean bool;
  float tempfloat,tempfloat2;
  long int minmaxrange[10];

  binsize = 1;

  //--ddc jul11 add pango layout and remove gdk string handling.
  int x,y;
  PangoLayout * layout;

  
  /* --- check for obvious problems --- */
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_GRAPH(widget));
  
  /* --- make sure it's a drawable widget --- */
    
    graph = GTK_GRAPH(widget);
    if (graph->num_values == 0) {
      return;
    }

    /* --- if we don't have a pixmap we need one --- */

    if (graph->pixmap != NULL) g_object_unref(graph->pixmap);
    graph->pixmap = gdk_pixmap_new(widget->window,
				   widget->allocation.width,
				   widget->allocation.height,
				   -1);




    /* --- draw a white background --- */
    gdk_draw_rectangle(graph->pixmap,
		       widget->style->white_gc,
		       TRUE,
		       0,0,
		       widget->allocation.width,
		       widget->allocation.height);


    /* --- Get height and width --- */
    width = widget->allocation.width - 101;
    height = widget->allocation.height - 31;

    //--ddc dec11 make sure there is any SPACE to draw (and prevent
    //buffer overflow problems below.
    if(width<1 || height<1) return;

    layout=gtk_widget_create_pango_layout(widget,NULL);
   
    /* --- Calculate width of the columns --- */
    column_width =width *binsize / graph->num_values;
    /* --- Something needs to be done to keep narrow columns from failing to display --- */
    while (column_width < 1){
      column_width =width *binsize / graph->num_values;
      binsize = binsize + 1;
    }

    if (binsize < binsizeforce) binsize = binsizeforce;
    column_width = (float) width * (float) binsize / (float) graph->num_values;

    /* --- Find the max value --- */
    /* --- The max value can be modified by the user using
       --- yscaleforce (yscale) and ybinforce(ybin)
    */
    /* --- also want to get the min so we can display bellow zero appropriately --- */
    for (k = 0; k < graph->num_lines; k++) {
      max[k] = 0;
      min[k] = 0;
      if ( yscaleforce == 1) {
	max[k] = yscale;
	min[k] = 0;
      } else {
	for (i = 0; i <= (graph->num_values-binsize); i += binsize) {
	  tempsum = 0;
	  tempavg = 0;
	  for(j = 0; j < binsize; j++){
	    tempsum = tempsum + graph->values[k][i+j];
	  }
	  if (max[k] < tempsum) {
	    max[k] = tempsum;
	  }
	  if (min[k] > tempsum) {
	    min[k] = tempsum;
	  }
	}
      }

      max[k]=max[k]*gtk_graph_y_scale();
      min[k]=min[k]*gtk_graph_y_scale();
      max[k] = (float) max[k] * 1.1; /* to make it look pretty */
      min[k] = (float) min[k] * 1.1;
      if (max[k] < 10) max[k] = 10;
      if (min[k] > 0) min[k] = 0;
    }

    if (graph->y_scale_mode == GTK_GRAPH_Y_SCALE_AUTO_OFF) {
      for (k = 1; k < graph->num_lines; k++) {
	max[k] = max[0];
	min[k] = min[0];
      }
    }

    /* --- for the case of graph->type as SEMILOG or SEMIROOT we need to modify the max and min --- */
    switch (graph->type) {
    case GTK_GRAPH_TYPE_SEMILOG:
      /* --- for the case of SEMILOG we cannot have values less than zero, but probably don't want to have values less than 1 --- */
      for (k = 0; k < graph->num_lines; k++) {
      if (max[k] > 1)
	max[k] = 1.1 * log(max[k]);
      else
	max[k] = 1;
      }
      break;
    case GTK_GRAPH_TYPE_SEMISQRT:
      /* --- for the case of SEMISQRT we cannot have less than zero, but can probably have one --- */
      for (k = 0; k < graph->num_lines; k++) {
	if (max[k] > 0)
	  max[k] = sqrt(max[k]);
	else
	  max[k] = 0;
	if (min[k] < 0)
	  min[k] = 0;
	else
	  min[k] = sqrt(min[k]);
      }
      break;
    default:
      break;
    }


    /* --- display data --- */
    
    currentmax = max[0];
    displayused = (long double) ((long double)( widget->allocation.width-101) /
				 (long double) graph->num_values /
				 (long double) column_width );

    /* --- draw a y-scale --- */
    points[0].x = 100;
    points[1].x = 100;
    points[0].y = height;
    points[1].y = 0;
    gdk_draw_lines(graph->pixmap,
		   widget->style->black_gc,
		   points,2);
    for (k = 0; k < graph->num_lines; k++) {
      minmaxrange[k] = max[k] - min[k];
    }

    pango_layout_set_text(layout,"1",1);
    pango_layout_get_pixel_size(layout,&x,&y);

    numdisplay = height / y/2;

    tempfloat2 = (int) ((float) pow(10,(int)log10(minmaxrange[0])) / (float) 4);
    i = 0;
    while (((minmaxrange[0] / tempfloat2) > numdisplay) && (i < 10)) {
      tempfloat2 = tempfloat2 * 2;
      i++;
    }
    /* --- this has to be done differently for each case --- */
    switch (graph->type) {
    case GTK_GRAPH_TYPE_LINEAR:
      for (i = 0; (i * tempfloat2) < minmaxrange[0]; i++) {
	points[0].x = 100;
	points[1].x = 90;
	tempfloat = tempfloat2 * i;
	  points[0].y = points[1].y = height  - ((float)height *(int) tempfloat / (float) minmaxrange[0]) ;
	  gdk_draw_lines(graph->pixmap,
			 widget->style->black_gc,
			 points,2);
	  sprintf(dummystr,"%d",(int)(tempfloat + min[0]));

	  pango_layout_set_text(layout,dummystr,strlen(dummystr));
	  pango_layout_get_pixel_size(layout,&x,&y);
	  gdk_draw_layout(graph->pixmap, widget->style->black_gc,  
			  30, points[0].y+4-y,layout);
	}
	break;
      case GTK_GRAPH_TYPE_SEMILOG:
	for (i = 0; i < 8; i++) {
	points[0].x = 100;
	points[1].x = 90;
	points[0].y = (int)( height - height*log((double)(i+1))/log(8));
	points[1].y = (int)( height - height*log((double)(i+1))/log(8));
	gdk_draw_lines(graph->pixmap,
		       widget->style->black_gc,
		       points,2);
	sprintf(dummystr,"%d",(int)exp(max[0])/8 * i);

	pango_layout_set_text(layout,dummystr,strlen(dummystr));
	pango_layout_get_pixel_size(layout,&x,&y);
	gdk_draw_layout(graph->pixmap, widget->style->black_gc,  
			30, points[0].y+4-y,layout);
	}
	break;
      case GTK_GRAPH_TYPE_SEMISQRT:
	for (i = 0; i < 8; i++) {
	  points[0].x = 100;
	  points[1].x = 90;
	  points[0].y = (int) height - height * sqrt((double)(float)i/(float)8);
	  points[1].y = (int) height - height * sqrt((double)(float)i/(float)8);
	  gdk_draw_lines(graph->pixmap,
			 widget->style->black_gc,
			 points,2);
	  sprintf(dummystr,"%d",(int)pow(max[0]/8*i,2));

	  pango_layout_set_text(layout,dummystr,strlen(dummystr));
	  pango_layout_get_pixel_size(layout,&x,&y);
	  gdk_draw_layout(graph->pixmap, widget->style->black_gc,  
			  30, points[0].y-y,layout); 
	}
	break;
      default:
	break;
    }
      
    /* --- life will be easier if we do the x-axis here too --- */
    /* --- start with displaying it inside the current graph --- */
    /* --- draw the axis --- */
    currentscaling = (long double) width / (currentrange[1] - currentrange[0]);
    points[0].x = 100;
    points[1].x = widget->allocation.width;
    points[0].y = points[1].y = widget->allocation.height - 29;
    gdk_draw_lines(graph->pixmap,
		   widget->style->black_gc,
		   points,2);


    /* --- draw the hash-marks and labels --- */
    /* --- y values are all the same here --- */
    points[0].y = widget->allocation.height - 29;
    points[1].y = widget->allocation.height - 19;
    /* --- we want at most 10 hash marks --- */

    /* --- Calculating the position of the centers of the cannels --- */

    /* --- need to determine the number of x-channel-displays which can
       --- be made in a cosmetically pleasing manner --- */

    /* --- first determine the size of a character --- */
    sprintf(dummystr," %d ",currentrange[1]);

    pango_layout_set_text(layout,dummystr,strlen(dummystr));
    pango_layout_get_pixel_size(layout,&string_size,&y);
    num_to_display = (float) width / (float) (string_size*2);
    if (num_to_display > 15) num_to_display = 15;

    //--ddc aug11 
    // to prevent 'staggering in tic marks' you really need to round off to 
    // higher integer AND recompute points to plot.  ALSO, the range is the difference PLUS ONE!
    //    display_interval = (float) (currentrange[1] - currentrange[0]) / (float) num_to_display;
    display_interval = ceil((float) abs(currentrange[1]-currentrange[0] + 1 ) / (float) num_to_display);
    num_to_display = ceil((float) abs(currentrange[1]-currentrange[0] + 1) / display_interval);
    for ( i = 0; i < num_to_display; i++) {
      /* --- need to determine the channel we are labeling --- */
      temp_channel = currentrange[0] + display_interval * i;
      /* --- need to determine the x position of the hash marks --- */
      if (binsize > 1) {
	result2 = (temp_channel - currentrange[0]) * column_width * displayused;
	points[1].x = points[0].x = (int) result2 + 101;
	gdk_draw_lines(graph->pixmap,widget->style->black_gc,points,2);
      } else {
	result2 = (temp_channel - currentrange[0] + 0.5) * column_width * displayused + 101;
	points[1].x = points[0].x = (int) result2;
      }
      gdk_draw_lines(graph->pixmap,
		     widget->style->black_gc,
		     points,2);
      /* --- now for the text labels --- */
      switch (graph->scale_type) {
      case GTK_GRAPH_SCALE_CALIBRATED:
	sprintf(dummystr," %4.0f ",(float)(graph->calibration[0] + graph->calibration[1] * (float) (temp_channel + 1) + graph->calibration[2] * pow(temp_channel + 1,2)));
	break;
      case GTK_GRAPH_SCALE_CHANNELS:
      default:
	sprintf(dummystr," %d ",temp_channel+1);
      }
      pango_layout_set_text(layout,dummystr,strlen(dummystr));
      pango_layout_get_pixel_size(layout,&x,&y);
      gdk_draw_layout(graph->pixmap, widget->style->black_gc,  
		      points[0].x-x/2, widget->allocation.height-7-y,layout);

    }

    //--ddc aug11 the LAST point is handled seperately.  You'll need the text for the label FIRST since the
    //position of the hash mark depends on it!

    //--ddc aug11 What you really want is the END of the range (and so _say_ end of range)
    //    temp_channel = currentrange[0] + display_interval * num_to_display;
    temp_channel = currentrange[1];

    switch (graph->scale_type) {
    case GTK_GRAPH_SCALE_CALIBRATED:
      sprintf(dummystr," %4.0f ",(float)(graph->calibration[0] + graph->calibration[1] * (float) (temp_channel + 1) + graph->calibration[2] * pow(temp_channel + 1,2)));
      break;
    case GTK_GRAPH_SCALE_CHANNELS:
    default:
      sprintf(dummystr," %d ",temp_channel+1);
    }

    if (binsize > 1) {
      result2 = (temp_channel - currentrange[0]) * column_width * displayused;
      points[1].x = points[0].x = (int) result2 + 101;
      gdk_draw_lines(graph->pixmap,widget->style->black_gc,points,2);
    } else {
      result2 = (temp_channel - currentrange[0] + 0.5) * column_width * displayused + 101;
      points[1].x = points[0].x = (int) result2;
    }


    pango_layout_set_text(layout,dummystr,strlen(dummystr));
    pango_layout_get_pixel_size(layout,&x,&y);
    //--ddc aug11 skip drawing the last label, if it might overlay another label
    if(x<display_interval/2){
      points[1].x = points[1].x - (int) ((float)x / 2);


      gdk_draw_layout(graph->pixmap, widget->style->black_gc,  
		    points[1].x-(float)x/2, widget->allocation.height-7-y,layout);

    }

    gdk_draw_lines(graph->pixmap,
		   widget->style->black_gc,
		   points,2);

    gc = gdk_gc_new(widget->window);
    bool = gdk_color_parse("#00F",&color);
    gdk_color_alloc(gtk_widget_get_default_colormap(),&color);
    gdk_gc_set_foreground((GdkGC *)gc,&color);

    /* --- display title and spectrum info --- */
    /* --- Display each bar graph --- */
    
    for (z = 0; z < graph->num_lines; z++) {
      switch(z) {
      case 0:
	bool = gdk_color_parse(colorstring[0],&color);
	break;
      case 1:
	bool = gdk_color_parse(colorstring[1],&color);
	break;
      case 2:
	bool = gdk_color_parse(colorstring[2],&color);
	break;
      case 3:
	bool = gdk_color_parse(colorstring[3],&color);
	break;
      case 4:
	bool = gdk_color_parse(colorstring[4],&color);
	break;
      case 5:
	bool = gdk_color_parse(colorstring[5],&color);
	break;
      case 6:
	bool = gdk_color_parse(colorstring[6],&color);
	break;
      case 7:
	bool = gdk_color_parse(colorstring[7],&color);
	break;
      case 8:
	bool = gdk_color_parse(colorstring[8],&color);
	break;
      case 9:
	bool = gdk_color_parse(colorstring[9],&color);
	break;
      default:
	bool = gdk_color_parse(colorstring[0],&color);
      };
      gdk_color_alloc(gtk_widget_get_default_colormap(),&color);
      gdk_gc_set_foreground((GdkGC *)gc,&color);
      
      pango_layout_set_text(layout,graph->title[z],strlen(graph->title[z]));
      pango_layout_get_pixel_size(layout,&x,&y);
      gdk_draw_layout(graph->pixmap, widget->style->black_gc,  
			  widget->allocation.width*7/10, 15+12*z-y, layout);

      prevx = 100;
      //--ddc aug11 hmm. why reducing the points to graph?
      //      for (i = 0; i < (graph->num_values - binsize- 1); i += binsize){
      for (i = 0; i < graph->num_values; i += binsize){
	tempsum = 0;
	for(j = 0; j < binsize; j++){
	  tempsum += graph->values[z][i+j];
	}
	/* --- funny thing, bar_height should be dependant on graph->type --- */
	switch (graph->type) {
	case GTK_GRAPH_TYPE_LINEAR:
	  bar_height = (float)(tempsum - min[z]) / (float)minmaxrange[z] * (float)height;
	  break;
	case GTK_GRAPH_TYPE_SEMILOG:
	  if (tempsum < 1) tempsum = 1;
	  bar_height = (float)log (tempsum) / (float)minmaxrange[z] * (float)height;
	  break;
	case GTK_GRAPH_TYPE_SEMISQRT:
	  if (tempsum < min[z]) tempsum = min[z];
	  bar_height = (float)sqrt (tempsum - min[z]) / (float)minmaxrange[z] * (float)height;
	  break;
	default:
	  break;
	}
	tempsum = 0;
        //--ddc debug.  If it is the last bar, there is no "next_bar_height"
        //and the tempsum goes out of bounds for the array. Add the "if"
        //in the next line, and for an else, set the 'next_bar_height'
        //to the bar_height
        //in the next line
	//--ddc aug11 hmm why fewer than points?
	//	if(i<(graph->num_values - 2*binsize- 1)){
	if(i<graph->num_values){
          for(j = 0; j < binsize; j++){
            tempsum += graph->values[z][i+j+binsize];
          }
          switch (graph->type) {
          case GTK_GRAPH_TYPE_LINEAR:
            next_bar_height = (float)(tempsum - min[z]) /(float)minmaxrange[z] * (float)height;
            break;
          case GTK_GRAPH_TYPE_SEMILOG:
            if (tempsum < 1) tempsum = 1;
            next_bar_height = (float)log (tempsum) / (float)minmaxrange[z] * (float)height;
            break;
          case GTK_GRAPH_TYPE_SEMISQRT:
            if (tempsum < min[z]) tempsum = min[z];
            next_bar_height = (float)sqrt (tempsum - min[z]) / (float)minmaxrange[z] * (float)height;
            break;
          default:
            break;
          }
        } else next_bar_height=bar_height;
           
        //--ddc ??! the points[0].x is set twice?!! (but I'll leave it).
        points[0].x = (i*column_width)*displayused + 101;
	points[0].x = prevx;
	points[0].y = height-bar_height;
	points[1].x = (i*column_width + column_width)*displayused + 101;
	points[1].y = height-bar_height;
	points[2].x = (i*column_width + column_width)*displayused + 101;
	points[2].y = height-next_bar_height;
	prevx = points[1].x;
	
	/* --- have to make sure that we are not going to flip over the y-axis --- */
	/* --- it would seem that gints are somehow different than ints --- */

	/* --- make sure that we will actually display something in the x-direction --- */
	if (points[0].x == points[1].x) points[1].x = points[2].x = points[0].x+1;
	gdk_draw_lines(graph->pixmap,
		       gc,
		       points,3);
      }
      points[0].x = 0;
      points[0].y = widget->allocation.height - 1;
      points[1].x = widget->allocation.width;
      points[1].y = widget->allocation.height - 1;
      gdk_draw_lines(graph->pixmap,
		     gc,
		     points,2);
      
      points[0].x = points[1].x = widget->allocation.width - 1;
      points[0].y = 0;
      points[1].y = widget->allocation.height;
      gdk_draw_lines(graph->pixmap,
		     gc,
		     points,2);
    }
    g_object_unref(gc);
    if (graph->num_segments) {
      /* --- draw the segments --- */    
      
      gc = gdk_gc_new(widget->window);
      bool = gdk_color_parse("#F0F",&color);
      gdk_color_alloc(gtk_widget_get_default_colormap(),&color);
      gdk_gc_set_foreground((GdkGC *)gc,&color);
      
      /* --- display the segments --- */
      for (i = 0; i < (graph->num_segments - 2); i++) {
	points[0].x = (graph->segments_x[i] + 0.5 - currentrange[0]) * displayused * column_width + 100;
	points[1].x = (graph->segments_x[i+1] + 0.5 - currentrange[0]) * displayused * column_width + 100;
	switch (graph->type) {
	case GTK_GRAPH_TYPE_LINEAR:
	  bar_height = (float)(graph->segments_y[i] - min[0]) /(float) minmaxrange[0] * (float)height;
	  next_bar_height = (float)(graph->segments_y[i+1] - min[0]) / (float)minmaxrange[0] * (float)height;
	  break;
	case GTK_GRAPH_TYPE_SEMILOG:
	  if (graph->segments_y[i] < 1) graph->segments_y[i] = 1;
	  if (graph->segments_y[i+1] < 1) graph->segments_y[i+1] = 1;
	  bar_height = (float)log(graph->segments_y[i]) / (float)minmaxrange[0] * (float)height;
	  next_bar_height = (float)log(graph->segments_y[i+1]) / (float)minmaxrange[0] * (float)height;
	  break;
	case GTK_GRAPH_TYPE_SEMISQRT:
	  if (graph->segments_y[i] < min[0]) graph->segments_y[i] = min[0];
	  if (graph->segments_y[i+1] < min[0]) graph->segments_y[i+1] = min[0];
	  bar_height = (float)sqrt(graph->segments_y[i] - min[0]) / (float)minmaxrange[0] * (float)height;
	  next_bar_height = (float)sqrt(graph->segments_y[i+1] - min[0]) / (float)minmaxrange[0] *(float) height;
	  break;
	default:
	  break;
	}
	points[0].y = height - bar_height;
	points[1].y = height - next_bar_height;
	gdk_draw_lines(graph->pixmap,
		       gc,
		       points,
		       2);
      }


      g_object_unref(gc);
    }

    g_object_unref(layout);

    gdk_draw_drawable(widget->window,
		    widget->style->fg_gc[gtk_widget_get_state(widget)],
		    graph->pixmap,
		    0,0,0,0,
		    widget->allocation.width,widget->allocation.height);

}

/*
 * gtk_graph_size_request
 *
 * How big should the widget be?
 * It can be modified.
 */
static void gtk_graph_size_request (GtkWidget *widget,
				    GtkRequisition *req)
{
  int ncols;
  int nrows;
  //--ddc apr11 ... Same problem (but done differently;) as in main.c.
  //  ncols = (numspectra - ((numspectra - 1) % 4)) / 4 + 1;
  //  if (numspectra <= 4) nrows = ((numspectra - 1) % 4) + 1;
  //  else nrows = 4;
  ncols=(numspectra-1)/4 + 1;
  if(numspectra<=4) {
    nrows=numspectra;
  } else nrows=4;


  req->width = (gdk_screen_width()-10) / ncols; 
  req->height = gdk_screen_height() / (2*nrows);

}

/*
 * gtk_graph_expose
 * 
 * The graph widget has been exposed and needs to be painted.
 */
static gint gtk_graph_expose (GtkWidget *widget, GdkEventExpose *event)
{
  GtkGraph *graph;

  /* --- Do error checking --- */
  //--ddc 24may06  g_return_val_if_fail(widget != NULL,FALSE);
  g_return_val_if_fail(GTK_IS_WIDGET(widget),FALSE);
  g_return_val_if_fail(GTK_IS_GRAPH (widget), FALSE);
  g_return_val_if_fail(event != NULL, FALSE);


  /* --- Get the graph widget --- */
  graph = GTK_GRAPH(widget);


  //--ddc apr11 debug... It seems that there is a problem with the double buffering that
  //is taken care of by 'drawing' on every expose. :(
  //gtk_graph_draw(widget,NULL);

  //ddc jun11 dbg...  This is the LAST I did to work (modify expose
  //to 'draw' EVERYTIME)
  gtk_graph_draw(widget,NULL);
  gdk_draw_drawable(widget->window,
		    widget->style->fg_gc[gtk_widget_get_state(widget)],
		    graph->pixmap,
		    event->area.x,event->area.y,
		    event->area.x,event->area.y,
		    event->area.width, event->area.height);

}

/* gtk_graph_destroy
 *
 * Destroy the widget and free up any allocated memory.
 * When done, call he parent destroy to make sure any 
 * memory allocated by the parent is freed.
 */
static void gtk_graph_destroy (GtkObject *object)
{
  GtkGraph *delgraph;
  int i;

  /* --- Check type --- */
  g_return_if_fail(object != NULL);
  g_return_if_fail(GTK_IS_GRAPH (object));

  /* --- Convert to a graph object --- */
  delgraph = GTK_GRAPH (object);

  /* --- Free memory --- */
  for (i = 0; i < delgraph->num_lines; i++) {
    g_free(delgraph->values[i]);
  }

  g_free (delgraph->segments_x);
  g_free (delgraph->segments_y);

  /* --- call parent destroy --- */
  GTK_OBJECT_CLASS(parent_class) ->destroy(object);
}

/* gtk_graph_get_pixmap
 *
 * returns a pointer to the pixmap of the graph
 */
GdkPixmap *gtk_graph_get_pixmap(GtkGraph *graph)
{
  /* --- check type --- */
  g_return_val_if_fail(graph != NULL, NULL);
  g_return_val_if_fail(GTK_IS_GRAPH(graph), NULL);
  return(graph->pixmap);
		   
}

/* gtk_graph_make_ps
 *
 * produces a ps file from the pixmap of the graph
 */
void gtk_graph_make_ps(GtkGraph *graph,const char *sFilename)
{
  FILE *outfile;
  int i,j,k,z;
  float col_width, bar_height_multiplier[10];
  float tempx1,tempx2,tempx3;
  float tempy1,tempy2,tempy3;
  float xdif,ydif;
  guint temp1,temp2;
  guint whatval;
  int numdisplay;
  float tempfloat,tempfloat2;
  int minmaxrange[10];
  int max[10],min[10];
  char dummystr[80];

  g_return_if_fail(graph != NULL);
  g_return_if_fail(GTK_IS_GRAPH(graph));
  g_return_if_fail(graph->num_values > 0);


  if ((outfile = fopen(sFilename, "w")) != NULL) {  
    printnotdone = 1;
    fprintf(outfile,"%%!PS-Adobe-2.0\n");
    fprintf(outfile,"%%%%BoundingBox: 0 0 600 800\n");
    fprintf(outfile,"%%%%DocumentData: Clean7Bit\n");
    fprintf(outfile,"%%%%Orientation: Portrait\n");
    fprintf(outfile,"%%%%Pages:1\n");
    fprintf(outfile,"%%%%Title: gtktwodplot\n");
    fprintf(outfile,"%%%%PageOrder: Ascend\n");
    fprintf(outfile,"%%%%EndComments\n");
    fprintf(outfile,"0.9 0.9 scale\n");
    fprintf(outfile,"30 70 translate\n");

    /* --- while we are at it, lets set up our text stuff --- */
    fprintf(outfile,"/Times-Roman findfont\n15 scalefont\nsetfont\n");

    /* --- calculate the width of columns --- */
    col_width = - (float) 700 / (float) graph->num_values;

    /* --- calculate the multiplicative factor for bar height --- */

    /* --- start by getting the maximum --- */
    for (z = 0; z < graph->num_lines; z++) {
      max[z] = 0;
      min[z] = 0;
    }
    /* --- also need to get the minumum --- */
    for (z = 0; z < graph->num_lines; z++) {
      for (i = 0; i < graph->num_values; i++) {
	if (max[z] < graph->values[z][i])
	  max[z] = graph->values[z][i];
	if (min[z] > graph->values[z][i])
	  min[z] = graph->values[z][i];
      }
      if (yscaleforce == 1) {
	max[z] = yscale;
	if (min[z] < 0)
	  min[z] =  - yscale;
      }

      max[z]=max[z]*gtk_graph_y_scale();
      min[z]=min[z]*gtk_graph_y_scale();
      if (max[z] < 10) max[z] = 10;
      if (min[z] > 0) min[z] = 0;
      max[z] = max[z] * 1.05;
      min[z] = min[z] * 1.05;
      /* --- let's also do this so that we can use the graph->type --- */
      switch (graph->type) {
      case GTK_GRAPH_TYPE_SEMILOG:
	if (max[z] > 1)
	  max[z] = 1.1 * log(max[z]);
	else
	  max[z] = 1;
	break;
      case GTK_GRAPH_TYPE_SEMISQRT:
	if (max[z] > 0) max[z] = sqrt(max[z]);
	else max[z] = 0;
	if (min[z] < 0) min[z] = 0; else min[z] = sqrt(min[z]);
	break;
      default:
	break;
      }
      minmaxrange[z] = max[z] - min[z];
      bar_height_multiplier[z] = (float) 570 / (float) minmaxrange[z];
    }
    
    
    /* --- display data --- */
    fprintf(outfile,"0.5 setlinewidth\n");
    for (z = 0; z < graph->num_lines; z++) {
      fprintf(outfile,"newpath\n");
      tempx3 = col_width;
      tempy1 = (graph->values[z][0] - min[z]) * bar_height_multiplier[z] + 30;
      fprintf(outfile,"%f 700 moveto 0 %f rlineto\n",
	      tempy1,col_width);
      switch (graph->type) {

      case GTK_GRAPH_TYPE_LINEAR:
	//--ddc aug11  why not all values?
	//      	for (i = 1; i < (graph->num_values - 1); i++) {
       	for (i = 1; i < graph->num_values; i++) {
	  xdif = col_width;
	  ydif = (float)((graph->values[z][i+1]) - graph->values[z][i]) * bar_height_multiplier[z];
	  fprintf(outfile,"0 %f rlineto\n",xdif);
	  fprintf(outfile,"%f 0 rlineto\n",ydif);
	}
	fprintf(outfile,"stroke\n");
	break;

      case GTK_GRAPH_TYPE_SEMILOG:
        //--ddc aug11 why not all values?
	//	for (i = 1; i < (graph->num_values - 1); i++) {
	for (i = 1; i < graph->num_values; i++) {
	  xdif = col_width;
	  if (graph->values[z][i] < 1) temp1 = 1;
	  else temp1 = graph->values[z][i];
	  if (graph->values[z][i+1] < 1) temp2 = 1;
	  else temp2 = graph->values[z][i+1];
	  ydif = (float)(log(temp2) - log(temp1)) * bar_height_multiplier[z];
	  fprintf(outfile,"0 %f rlineto\n",xdif);
	  fprintf(outfile,"%f 0 rlineto\n",ydif);
	}
	fprintf(outfile,"stroke\n");
	break;
	
      case GTK_GRAPH_TYPE_SEMISQRT:
        //--ddc aug11 why not all values?
//	for (i = 1; i < (graph->num_values - 1); i++) {
	for (i = 1; i < graph->num_values; i++) {
	  xdif = col_width;
	  if (graph->values[z][i] < 0) temp1 = 0;
	  else temp1 = graph->values[z][i];
	  if (graph->values[z][i+1] < 0) temp2 = 0;
	  else temp2 = graph->values[z][i+1];
	  ydif = (float)(sqrt(temp2) - sqrt(temp1)) * bar_height_multiplier[z];
	  fprintf(outfile,"0 %f rlineto\n",xdif);
	  fprintf(outfile,"%f 0 rlineto\n",ydif);
	}
	fprintf(outfile,"stroke\n");
	break;

      default:
	break;
      }
    }
    fprintf(outfile,"2 setlinewidth\n");

    /* --- draw the axes --- */
    fprintf(outfile,"newpath\n30 0 moveto 0 700 rlineto 570 0 rlineto 0 -700 rlineto -570 0 rlineto stroke\n");

    /* --- determine the location of the hash marks --- */
    /* --- first for the y scale --- */
    switch (graph->type) {
    case GTK_GRAPH_TYPE_LINEAR:
      numdisplay = (float) 570 / (float) 15;
      tempfloat2 = (int) ((float) pow(10,(int)log10(minmaxrange[0])) / (float) 4);
      while ((minmaxrange[0] / tempfloat2) > numdisplay)
	tempfloat2 = tempfloat2 * 2;
      for (i = 0; (i * tempfloat2) <= minmaxrange[0]; i++) {
	tempfloat = tempfloat2 * i;
	tempy1 = tempfloat * bar_height_multiplier[0] + 30;
	fprintf(outfile,"newpath\n %f 690 moveto 0 10 rlineto stroke\n",tempy1);
	fprintf(outfile,"%f 790 moveto\n -90 rotate\n (%d) show\n 90 rotate\n",
		tempy1-7,(int)(tempfloat + min[0]));
      }
      break;

    case GTK_GRAPH_TYPE_SEMILOG:
      for ( i = 1; i < 10; i++) {
	tempy1 = log(i) / log( 10) * 570 + 30;
	fprintf(outfile,"newpath\n %f 690 moveto 0 10 rlineto stroke\n",tempy1);
	fprintf(outfile,"%f 790 moveto\n -90 rotate\n (%d) show\n 90 rotate\n",
		tempy1 - 7, (int)(exp(max[0]) * log(i) /log(10)));
      }
      break;

    case GTK_GRAPH_TYPE_SEMISQRT:
      for ( i = 0; i < 10; i++) {
	tempy1 = sqrt((float)i /(float) 10) * 570 + 30;
	fprintf(outfile,"newpath\n %f 690 moveto 0 10 rlineto stroke\n",tempy1);
	fprintf(outfile,"%f 790 moveto\n -90 rotate\n (%d) show\n 90 rotate\n",
		tempy1 - 7, (int) (pow(max[0],2) * sqrt((float)i /(float) 10)));
      }
      break;

    default:
      break;
    }
    /* --- now for the x scale --- */

    numdisplay = (float) 700 / (float) 30;
    tempfloat2 = (int) ((float) pow(10,(int)log10(graph->num_values)) / (float) 4);
    while (((float)graph->num_values / (float) tempfloat2) > numdisplay)
      tempfloat2 = tempfloat2 * 2;

    for (i = 0; (i * tempfloat2) <= (graph->num_values); i++) {
      tempfloat = tempfloat2 * i;
      tempx1 = tempfloat * col_width + 700;
      fprintf(outfile,"newpath\n 30 %f moveto 10 0 rlineto stroke\n",tempx1);
      switch (graph->scale_type) {
      case GTK_GRAPH_SCALE_CALIBRATED:
	sprintf(dummystr,"%4.0f",(float)(graph->calibration[0] + graph->calibration[1] * (float) (tempfloat + currentrange[0]) + graph->calibration[2] * pow(tempfloat + currentrange[0],2)));
	break;
      case GTK_GRAPH_SCALE_CHANNELS:
      default:
	sprintf(dummystr,"%4.0f",(float)(tempfloat + currentrange[0]));
      }
      fprintf(outfile,"0 %f moveto\n -90 rotate\n(%s) stringwidth pop\n 0 exch sub 2 div\n 0 rmoveto\n (%s) show\n 90 rotate\n",
	      (tempx1),dummystr,dummystr);
    }

    fprintf(outfile,"showpage\n");
    fprintf(outfile,"%%EOF\n");
    fclose(outfile);
    printnotdone = 0;
  } else {
    GetMessageDialog("Error opening output file.\n");
  } 
}


/* gtk_graph_set_y_scale_mode
 *
 * Sets the method of determining the relative y scales in 
 * overlay mode.  The current options are 
 * GTK_GRAPH_Y_SCALE_AUTO_OFF and
 * GTK_GRAPH_Y_SCALE_AUTO_ON
 */
void gtk_graph_set_y_scale_mode(GtkGraph *graph,enum gtk_graph_y_scale_mode mode)
{
  //--ddc 23may06  g_return_if_fail(graph != NULL);
  g_return_if_fail(GTK_IS_GRAPH(graph));
  
  graph->y_scale_mode = mode;
}


//--ddc aug11 J.Pavan's prior version of a scaling function was 'distributed'
//I move it here so it can be called other places, and because it was WRONG.
//It was multivalued, returning a value of one for -1,0,and 1.  ybinforce...
//this is dubious (and that it is 'toggled' in this function), But, for
//the moment I leave it as is, BUT, I add ONE to abs(ybin), keep it
//from being multivalued!

double gtk_graph_y_scale(void){
  double max=1.0;
  
  if (ybinforce == 1){
    if (ybin > 0) {
      max = max * (ybin+1) ;
    } else {
      if (ybin < 0) {
	max = max /( -ybin+1);
      } else {
	if (ybin == 0) {
	  ybinforce = 0;
	}
      }
    }
  }

  return max;
  
}


/*
 * Progress bar
 *
 * 
 */
#include <stdio.h>
#include <gtk/gtk.h>
#include <time.h>

//--ddc aug11 gtk deprecation of gtk_progress_bar.  Replace percentage with
//      fraction globally :\

typedef struct {

  GtkWidget *progressbar;
  GtkWidget *window;
  int bProgressUp;
  float nLastFrac;
  long int startime;
} typProgressData;

typProgressData *pdata = NULL;
typProgressData *sdata = NULL;
GtkWidget *mainwindow;
int abortflag;

GtkWidget *sortlabel;

/* SortAbortCallback
 *
 * Sets the abort flag for the sort function to 1
 */
void SortAbortCallback(GtkWidget *widget, gpointer *data) 
{
  abortflag = 1;
}

/*
 * CanWindowClose
 *
 * Function that determines that if the dialog
 * window can be closed.
 */
gint CanWindowClose (GtkWidget *widget)
{
    /* --- TRUE => cannot close --- */
    /* --- FALSE => can close --- */
    return (pdata->bProgressUp);
}

/*
 * UpdateProgress
 *
 * Update the progress window to reflect the state
 * of the file that is being loaded.  
 *
 * pos - how much of the file has been loaded.
 * len - length of the file
 * (pos / len) = % file has been loaded.
 */
void UpdateProgress (long pos, long len)
{
    gfloat pvalue;
    //    int frac;
 
    /* --- Prevent divide by zero errors --- */
    if (len > 0) {


        pvalue = (gfloat) pos / (gfloat) len;
    
	//--ddc 11aug        frac = pvalue;

        if (pdata->nLastFrac != pvalue) {

            /* --- Update the displayed value --- */
	  if ((pvalue <=1.0) && (pvalue >= 0))
	    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(pdata->progressbar), 
                                         pvalue);

            /* --- Repaint any windows - like the progress bar --- */
	    while (gtk_events_pending ()) {
	      gtk_main_iteration ();
	    }
            pdata->nLastFrac = pvalue;
        }
    }
}

/*
 * UpdateSortProgress
 * 
 * same as update Progress but for sort
 */
void UpdateSortProgress (long pos, long len)
{
    gfloat pvalue;
    //    int frac;
    long int etime;
    time_t thime;
    char dummystr[80];
    long int time_left,time_used;

    /* --- Prevent divide by zero errors --- */
    if (len > 0) {

        pvalue = (gfloat) pos / (gfloat) len;
    
	//        frac = pvalue;

        if (sdata->nLastFrac != pvalue) {

            /* --- Update the displayed value --- */
	  if ((pvalue <=1.0) && (pvalue >= 0))
	    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR (sdata->progressbar), 
                                         pvalue);

	  etime = time(&thime);
	  time_used = etime - sdata->startime;
	  if (pvalue > 0) {
	    time_left = time_used / pvalue - time_used;
	  } else time_left = -1;
	  sprintf(dummystr,"%d Sec. Elapsed, %d Sec. Remaining.",time_used,time_left);
	  gtk_label_set_text(GTK_LABEL(sortlabel),dummystr);

            /* --- Repaint any windows - like the progress bar --- */
	    while (gtk_events_pending ()) {
	      gtk_main_iteration ();
	    }
            sdata->nLastFrac = pvalue;
        }
    }
}



/*
 * StartProgress 
 *
 * Create a window for the progress bar
 */
void StartProgress ()
{
    GtkWidget *label;
    GtkWidget *table;
    GtkWidget *barwindow;
    GtkAdjustment *adj;

    pdata = (typProgressData *) g_malloc (sizeof (typProgressData));
    pdata->nLastFrac = -1;
    pdata->bProgressUp = TRUE;
    /*
     * --- Create the top level window
     */
    barwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    pdata->window = barwindow;
    
    /* --- Hook up the destroy  --- */
    //--ddc aug11    gtk_signal_connect (GTK_OBJECT (barwindow), "delete_event",
    g_signal_connect (GTK_OBJECT (barwindow), "delete_event",
			G_CALLBACK (CanWindowClose), pdata);
    //--ddc aug11    
    //    gtk_container_border_width (GTK_CONTAINER (barwindow), 10);
    gtk_container_set_border_width (GTK_CONTAINER (barwindow), 10);
    
    /* --- Create a table --- */
    table = gtk_table_new (3, 2, TRUE);
    gtk_container_add (GTK_CONTAINER (barwindow), table);
    
    /* --- Add a label to the table --- */
    label = gtk_label_new ("Projecting ... ");
    gtk_table_attach_defaults (GTK_TABLE (table), label, 0,2,0,1);
    gtk_widget_show (label);
    
    /* --- Add the progress bar to the table --- */
    adj = (GtkAdjustment *) gtk_adjustment_new (0, 0, 400, 0, 0, 0);
    //--ddc aug11 gtk deprecation    
    //pdata->progressbar = gtk_progress_bar_new_with_adjustment (adj);
    pdata->progressbar = gtk_progress_bar_new();
    gtk_table_attach_defaults (GTK_TABLE (table), 
                               pdata->progressbar, 0,2,1,2);
    gtk_widget_show (pdata->progressbar);

    
    /* --- Show everything --- */
    gtk_widget_show (table);
    gtk_widget_show (barwindow);
    gtk_window_set_position((GtkWindow *) barwindow,GTK_WIN_POS_CENTER);
}

/*
 * StartSortProgress
 *
 * same as StartProgress but for sorting
 */
void StartSortProgress ()
{
    GtkWidget *table;
    GtkWidget *barwindow;
    GtkAdjustment *adj;
    time_t thime;
    GtkWidget *localbutton;
    GtkWidget *localvbox;

    sdata = g_malloc (sizeof (typProgressData));
    sdata->nLastFrac = -1;
    sdata->bProgressUp = TRUE;
    sdata->startime = time(&thime);
    
    /*
     * --- Create the top level window
     */
    barwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_modal(GTK_WINDOW(barwindow),TRUE);
    sdata->window = barwindow;
    
    /* --- create the localvbox --- */
    localvbox = gtk_vbox_new(FALSE,2);
    gtk_container_add(GTK_CONTAINER(barwindow),localvbox);

    /* --- Hook up the destroy  --- */
    //--ddc aug11    gtk_signal_connect (GTK_OBJECT (barwindow), "delete_event",
    g_signal_connect (GTK_OBJECT (barwindow), "delete_event",
			G_CALLBACK (CanWindowClose), sdata);
    
    //--ddc aug11    gtk_container_border_width (GTK_CONTAINER (barwindow), 10);
    gtk_container_set_border_width (GTK_CONTAINER (barwindow), 10);
    
    /* --- Create a table --- */
    table = gtk_table_new (3, 2, TRUE);
    gtk_container_add (GTK_CONTAINER (localvbox), table);
    
    /* --- Add a label to the table --- */
    sortlabel = gtk_label_new ("Sorting...");
    gtk_table_attach_defaults (GTK_TABLE (table), sortlabel, 0,2,0,1);
    gtk_widget_show (sortlabel);
    
    /* --- Add the progress bar to the table --- */
    adj = (GtkAdjustment *) gtk_adjustment_new (0, 0, 400, 0, 0, 0);
    //--ddc aug11 gtk deprecation
    //    sdata->progressbar = gtk_progress_bar_new_with_adjustment (adj);
    sdata->progressbar = gtk_progress_bar_new();
    gtk_table_attach_defaults (GTK_TABLE (table), 
                               sdata->progressbar, 0,2,1,2);
    gtk_widget_show (sdata->progressbar);

    /* --- create the cancel button --- */
    localbutton = gtk_button_new_with_label("Cancel");
    //--ddc aug11    gtk_signal_connect(GTK_OBJECT(localbutton),"clicked",
    g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		       G_CALLBACK(SortAbortCallback),NULL);
    gtk_container_add(GTK_CONTAINER(localvbox),localbutton);

    
    /* --- Show everything --- */
    gtk_widget_show_all (barwindow);
    gtk_window_set_position((GtkWindow *) barwindow,GTK_WIN_POS_CENTER);
}

/*
 * EndProgress
 *
 * Close down the progress bar.
 */
void EndProgress ()
 {
    /* --- Allow it to close --- */
    pdata->bProgressUp = FALSE;

    /* --- Destroy the window --- */
    gtk_widget_destroy (pdata->window);

    /* --- Free used memory. --- */
    g_free (pdata);

    pdata = NULL;
}


/*
 * EndSortProgress
 *
 * Close down the progress bar.
 */
void EndSortProgress ()
{
    /* --- Allow it to close --- */
    sdata->bProgressUp = FALSE;

    /* --- Destroy the window --- */
    gtk_widget_destroy (sdata->window);

    /* --- Free used memory. --- */
    g_free (sdata);

    sdata = NULL;
}

#include <stdio.h>
#include <gtk/gtk.h>

/* --- structures --- */

typedef struct {
  void (*func) (gchar *);
  GtkWidget *filesel;
} typFileSelectionData;

/* --- globals --- */

static char *sFilename = NULL;

/* --- function declarations --- */

void GetFilename (char *sTitle, int WriteDialog, void (*callback) (char *));
static void FileOk (GtkWidget *w, gint response, gpointer data);


/* --- functions --- */

/*
 * GetFilename
 *
 * Show a dialog box with a title and if "ok" is slected
 * call the function with the name of the file.
 *
 * Use as GetFilename("Open", Loadfile);
 */
void GetFilename (char *sTitle, int WriteDialog, void (*callback) (char *))
{
  GtkWidget *filew = NULL;
  typFileSelectionData *data;

  char* DefFolder;
  char* DefFile;

  /* --- Create a File Chooser Dialog widget --- */

  if(WriteDialog==1){
    filew=gtk_file_chooser_dialog_new(sTitle,NULL,GTK_FILE_CHOOSER_ACTION_SAVE,
               GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, 
				    GTK_RESPONSE_ACCEPT,  NULL);
  } else {
    filew=gtk_file_chooser_dialog_new(sTitle,NULL,GTK_FILE_CHOOSER_ACTION_OPEN,
	      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, 
				    GTK_RESPONSE_ACCEPT,  NULL);
  }

  data = g_malloc (sizeof (typFileSelectionData));
  data->func = callback;
  data->filesel = filew;

  if (sFilename){  
    /* --- Set the default filename --- */

    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (filew),
 				    sFilename);
  } else { //--ddc for cripes sake, give it some memory!
    sFilename = g_malloc(120);
    g_stpcpy(sFilename,"default");

  }


  /* --- connect the response  --- */

  g_signal_connect (GTK_OBJECT(GTK_FILE_CHOOSER(filew)),
	    		      "response", (GCallback) FileOk, data);

  
  /* --- show the dialogue box --- */
  gtk_widget_show_all (filew);

  /* --- grab the focus --- */
  gtk_grab_add(filew);
}

/* 
 * FileOk
 *
 * Call the function (func) to do what is needed to the file
 */
//void FileOk (GtkWidget *w, gpointer data)
void FileOk (GtkWidget *w,  gint       response_id, gpointer data)
{
  char *sTempFile;
  typFileSelectionData *localdata;
  GtkWidget *filew;

  localdata = (typFileSelectionData *) data;
  filew = localdata->filesel;

  switch(response_id) {
  case GTK_RESPONSE_OK:
  case GTK_RESPONSE_ACCEPT:
    
    /* --- Which file? --- */

    sTempFile = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER (filew));

    /* --- Free old memory --- */
    if (sFilename) g_free (sFilename);

    /* --- Duplicate the string --- */
    sFilename = g_strdup (sTempFile);

    /* --- Call the Function that does the work --- */
    (*(localdata->func)) (sFilename);

    /* --- Free the sFilename --- */
    g_free (sFilename);
    sFilename = NULL;
    break;
  default:
    printf("Humm. Response code is: %d \n", response_id);
  }

  /* --- Close the dialog box --- */
  gtk_widget_destroy (filew);

}


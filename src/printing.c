/* File: printing.c
 *
 * by: John Pavan
 *
 * Printing support for gnuscope
 */

/* --- includes --- */

#include <gtk/gtk.h>
#include "gtkgraph.h"
#include "gtktwodplot.h"
#include <stdio.h>
//--ddc 3jun08 add missing function prototypes (stdlib,string,time,unistd)
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/types.h>

//--ddc aug11 MANY gtk deprecations.  listbox is now a tree_view, with
//--ddc a liststore for the model...  Also as there is a datastore of
//--ddc the printers with the liststore, there was no reason to keep
//--ddc all the string manipulation functions

/* --- globals --- */

int printnotdone;
char printername[120];

/* --- function declarations --- */

void CreatePrintWindow(GtkWidget *widget);
void ListBoxAddItem(GtkWidget *listbox, char *sText, int printerdex);
void CloseWidget(GtkWidget *widget, gpointer data);
void PrintCallback(GtkWidget *widget, gpointer *data);

//--ddc added function
static void ListBoxSelectedCallbackDisplay(GtkTreeSelection *widget, gpointer *data);

/* --- functions --- */

/* AddListBoxItem
 *
 * Adds an item to a list box
 */
void ListBoxAddItem(GtkWidget *listbox, char *sText, int printerdex) {

  GtkListStore *localliststore;

  GtkTreeIter iter;

  localliststore = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(listbox)));

  gtk_list_store_append(localliststore, &iter);
  gtk_list_store_set(localliststore, &iter, 0, sText, -1);

  gtk_tree_view_expand_all(listbox);
}

/* CreatePrintWindow
 *
 * creates a window which should contain a list of avalible printers,
 * a print button, and a cancel button
 *
 * shoulde be smart enough to identify if the widget passed to it
 * is a graph or a twodplot
 */
void CreatePrintWindow(GtkWidget *widget) {
  char dummystr[120];
  //  char dummystr2[120];
  FILE *printcap;

  GtkWidget *localwindow;
  GtkWidget *localbutton;
  GtkWidget *localvbox;
  GtkWidget *localhbox;
  GtkWidget *locallistbox;
  int i, j;
  //--ddc jan11 gtk deprecation for gtklist
  GtkListStore *localliststore;

  GtkTreeSelection *locallistsel;
  GtkTreeViewColumn *locallistcol;
  GtkCellRenderer *locallistrenderer;

  /* --- idiot check --- */

  g_return_if_fail(widget != NULL);

  //--ddc aug11 initialize the printer name..
  printername[0] = '\0';

  /* --- make the window --- */

  localwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(localwindow), "Gnuscope Print");
  gtk_window_set_modal(GTK_WINDOW(localwindow), TRUE);

  /* --- create the localvbox --- */
  localvbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(localwindow), localvbox);

  /* --- create the list box --- */
  { // this is to replace deprecated gtk_list...

    localliststore = gtk_list_store_new(1, G_TYPE_STRING);
    locallistbox = gtk_tree_view_new_with_model(GTK_TREE_MODEL(localliststore));

    locallistrenderer = gtk_cell_renderer_text_new();
    locallistcol =
        gtk_tree_view_column_new_with_attributes("Printer", locallistrenderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(locallistbox), locallistcol);

    locallistsel = gtk_tree_view_get_selection(GTK_TREE_VIEW(locallistbox));
    gtk_tree_selection_set_mode(locallistsel, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(locallistsel), "changed", G_CALLBACK(ListBoxSelectedCallbackDisplay),
                     NULL);

    gtk_container_add(GTK_CONTAINER(localvbox), locallistbox);
  }

  /* --- open printcap and add the appropriate entries to the list box --- */
  i = 0;
  if ((printcap = fopen("/etc/printcap", "r")) != NULL) {
    while (fgets(dummystr, 120, printcap) != NULL) {
      if ((strncmp(dummystr, " ", 1) != 0) && (strncmp(dummystr, "#", 1) != 0)) {
        j = 0;
        while (((strncmp(&dummystr[j], ":", 1) != 0) && (strncmp(&dummystr[j], "|", 1) != 0)) &&
               (j < strlen(dummystr)))
          j++;
        dummystr[j] = '\0';
        ListBoxAddItem(locallistbox, dummystr, i);
        i++;
      }
    }
    fclose(printcap);
  }

  /* --- create the localhbox for the print and cancel button --- */

  localhbox = gtk_hbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(localvbox), localhbox);

  /* --- create the printbutton --- */

  localbutton = gtk_button_new_with_label("Print");
  gtk_container_add(GTK_CONTAINER(localhbox), localbutton);
  g_signal_connect(GTK_OBJECT(localbutton), "clicked", G_CALLBACK(PrintCallback),
                   (gpointer *)widget);
  g_signal_connect_swapped(GTK_OBJECT(localbutton), "clicked", G_CALLBACK(CloseWidget),
                           GTK_OBJECT(localwindow));

  /* --- create the cancel button --- */

  localbutton = gtk_button_new_with_label("Cancel");
  gtk_container_add(GTK_CONTAINER(localhbox), localbutton);
  g_signal_connect_swapped(GTK_OBJECT(localbutton), "clicked", G_CALLBACK(CloseWidget),
                           GTK_OBJECT(localwindow));

  /* --- show the window --- */
  gtk_widget_show_all(localwindow);
}

/* PrintCallback
 *
 * Prints based on what is selected in the listbox previously
 */
void PrintCallback(GtkWidget *widget, gpointer *data) {
  pid_t pid;
  char dummystr[120];

  sprintf(dummystr, "/tmp/gnuscope%d%d.ps", (int)time(NULL), getpid());
  if (GTK_IS_GRAPH(data))
    gtk_graph_make_ps(GTK_GRAPH(data), dummystr);
  if (GTK_IS_TWODPLOT(data))
    gtk_twodplot_make_ps(GTK_TWODPLOT(data), dummystr, 0);

  while (printnotdone) {
    /* --- do nothing until done printing --- */
  }

  if ((pid = fork()) < 0)
    g_error("fork error");
  else if (pid == 0) {
    if ((pid = execl("/usr/bin/lpr", "lpr", "-r", "-P", printername, dummystr, (char *)0)) == -1)
      g_error("execl error");
  }

  if (waitpid(pid, NULL, 0) < 0)
    g_error("wait error");
}

static void ListBoxSelectedCallbackDisplay(GtkTreeSelection *widget, gpointer *data) {

  GtkTreeIter iter;
  GtkTreeModel *model;
  GList *pathlist;
  char *printer;

  pathlist = gtk_tree_selection_get_selected_rows(widget, &model);

  if (pathlist != NULL) {
    while (pathlist) {
      gtk_tree_model_get_iter(model, &iter, pathlist->data);
      gtk_tree_model_get(model, &iter, 0, &printer, -1);
      sprintf(printername, "%s", printer);
      pathlist = pathlist->next;
    }
    g_list_foreach(pathlist, (GFunc)gtk_tree_path_free, NULL);
    g_list_free(pathlist);
  }
}

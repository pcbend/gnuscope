/* displaywindow.c 
* by:  John Pavan 
* 
* so you can click on the title of a graph and it will display 
*/

//--ddc jan11 MANY gtk deprecations.  listbox is now a tree_view, with
//--ddc a liststore for the model...

/* --- includes --- */

#include <gtk/gtk.h>
#include "gtkgraph.h"
#include "gtktwodplot.h"
#include <stdio.h> 
#include "gnuscopeglobals.h" 
//--ddc trouble with function prototypes... 
//#include "pgam.h" 
#include "gnuscopefuncs.h" 

/* --- structs --- */

/*--- globals --- */

//int histid[254],histsize[254],histpointer;
//int *histloc[254];
//char field1[254][40],field2[254][40],field3[254][40];
int numspectra;
int spectra;
int currentrange[2];
int xcurrentrange[2],ycurrentrange[2];
short int display_toggle[254];
short int twod_display_toggle[254];
GtkWidget *twoddisplayed[16];
short int twodspectradisplayed[16];
short int numtwoddisplayed;
int pgammax;
struct bigdata **big_data_info;
char twodtitles[254][80];

/* --- function declarations --- */

void PgamDisplaySelectionEntry(GtkWidget *widget, GtkWidget *entry); 
void CloseWidget(GtkWidget *widget, gpointer *data);
void MultipleDisplayEntry(GtkWidget *widget, GtkWidget *entry);
void CreateDisplaySelectionWindow(int mode);

void ListBoxAddItemDisplay(GtkWidget*, char*,int);
void TwodListBoxAddItemDisplay(GtkWidget *listbox, char *sText,int index);

void DisplayWindowCallback(GtkWidget *widget, gpointer *data);
void TwodDisplayWindowCallback(GtkWidget *widget, gpointer *data);
void MultipleDisplaySameRangeEntry(GtkWidget *widget, GtkWidget *entry);
void MultiInOneEntry(GtkWidget *widget, GtkWidget *entry);

//--ddc added functions
static void ListBoxSelectedCallbackDisplay(GtkTreeSelection *widget, gpointer *data);
static void TwodListBoxSelectedCallbackDisplay(GtkTreeSelection *widget, gpointer *data);


/* --- functions --- */

/* ListBoxAddItemDisplay
 *
 * Adds items to the display list box
 */

void ListBoxAddItemDisplay(GtkWidget *listbox, char *sText,int index) 
{
  //--ddc jan11 deprecation of gtk list, item
  //  GtkWidget *item;
  GtkListStore *localliststore;
  GtkTreeIter iter;
  //I think we need "index" converted to a string here...
  //--ddc jan11 gtk deprecation  item = gtk_list_item_new_with_label(sText);
  localliststore=gtk_tree_view_get_model(GTK_TREE_VIEW(listbox));
  gtk_list_store_append(localliststore,&iter);
  gtk_list_store_set(localliststore,&iter,0,index,1,sText,-1);
  gtk_tree_view_expand_all(listbox);
  
  
  /*
  g_signal_connect(GTK_OBJECT(item),"select",
		     G_SIGNAL_FUNC(ListItemSelectedCallbackDisplay),
		     (int *) index);
  g_signal_connect(GTK_OBJECT(item),"deselect",
		     G_SIGNAL_FUNC(ListItemUnselectCallbackDisplay),
		     (int *) index);
  gtk_container_add(GTK_CONTAINER(listbox),item);
  */
}

/* TwodListBoxAddItemDisplay
 * Adds a list item for the twoddisplay selection boxes
 */
void TwodListBoxAddItemDisplay(GtkWidget *listbox, char *sText, int index)
{

//--ddc jan11 deprecation of gtk list, item
//  GtkWidget *item;
  GtkListStore *localliststore;
  GtkTreeIter iter;
//I think we need "index" converted to a string here...
//--ddc jan11 gtk deprecation  item = gtk_list_item_new_with_label(sText);
  localliststore=gtk_tree_view_get_model(GTK_TREE_VIEW(listbox));
  gtk_list_store_append(localliststore,&iter);
  gtk_list_store_set(localliststore,&iter,0,index,1,sText,-1);
  gtk_tree_view_expand_all(listbox);
  
  //  item = gtk_list_item_new_with_label(sText);
  /*  item deprecation..
  g_signal_connect(GTK_OBJECT(item),"select",
		     G_SIGNAL_FUNC(TwodListItemSelectedCallbackDisplay),
		     (int *) index);
  g_signal_connect(GTK_OBJECT(item),"deselect",
		     G_SIGNAL_FUNC(TwodListItemUnselectCallbackDisplay),
		     (int *) index);
  gtk_container_add(GTK_CONTAINER(listbox),item);
  */

}

/* TwodCreateDisplaySelectionWindow
 *
 * Creates a window with a list box for the purpose of selecting what to display
 * Currently does not support modes
 */
void TwodCreateDisplaySelectionWindow()
{
  GtkWidget *localwindow;
  GtkWidget *localbutton;
  GtkWidget *localvbox;
  GtkWidget *localhbox;
  GtkWidget *locallistbox;
  //--ddc jan11 gtk deprecation of list widget 
  GtkListStore *localliststore;
  GtkTreeViewColumn *locallistcol;
  GtkTreeSelection *locallistsel;
  GtkCellRenderer *locallistrenderer;

  GtkWidget *localframe;
  //--ddc replace with scrolled window  GtkWidget *framehbox;
  GtkWidget *localscrollwindow;
  int x,y;

  GtkWidget *localentry;
  char dummystr[80],*linefeed;
  int i,j;


  /* --- idiot check --- */
  g_return_if_fail(pgammax != 0);

  /* --- Create the window and the appropriate stuff in the window --- */
  localwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(localwindow),"2D Histogram Display Selection");
  gtk_window_set_modal(GTK_WINDOW(localwindow),TRUE);

  /* --- create the localvbox --- */
  localvbox = gtk_vbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(localwindow),localvbox);
  
  /* --- create the frame --- */
  localframe = gtk_frame_new("Select 2D histograms to display");
  gtk_container_add(GTK_CONTAINER(localvbox),localframe);

  /* --- make a horizontal box inside the frame --- */

  //  framehbox = gtk_hbox_new(TRUE,0);
  //  gtk_container_add(GTK_CONTAINER(localframe),framehbox);


  localentry = gtk_entry_new();
  gtk_entry_set_max_length (GTK_ENTRY (localentry), 80);


  {//this is to replace deprecated gtk_list...
      localliststore=gtk_list_store_new(2,G_TYPE_INT,G_TYPE_STRING);

      locallistbox = gtk_tree_view_new_with_model(localliststore);

      locallistrenderer=gtk_cell_renderer_text_new();
      locallistcol = gtk_tree_view_column_new_with_attributes ("Spec #", locallistrenderer,
						   "text", 0, NULL);
      gtk_tree_view_append_column (GTK_TREE_VIEW (locallistbox), locallistcol);

      locallistcol = gtk_tree_view_column_new_with_attributes ("Name", locallistrenderer,
						   "text", 1, NULL);
      gtk_tree_view_append_column (GTK_TREE_VIEW (locallistbox), locallistcol);


      //--ddc jan11 deprecation  gtk_list_set_selection_mode(GTK_LIST(locallistbox),GTK_SELECTION_MULTIPLE);
      //--ddc jan11 need hook up signal handler >here<
      locallistsel=gtk_tree_view_get_selection(GTK_TREE_VIEW(locallistbox));
      gtk_tree_selection_set_mode(locallistsel, GTK_SELECTION_MULTIPLE);
      g_signal_connect(G_OBJECT(locallistsel), "changed", 
		       G_CALLBACK(TwodListBoxSelectedCallbackDisplay), NULL);
      //--ddc      gtk_container_add(GTK_CONTAINER(framehbox),locallistbox);


      localscrollwindow=gtk_scrolled_window_new(NULL,NULL);
      gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(localscrollwindow),
					    locallistbox);

      pango_layout_get_pixel_size(gtk_entry_get_layout(localentry),&x,&y);
      gtk_widget_set_size_request(localscrollwindow,-1,10*y);

  }

  gtk_container_add(GTK_CONTAINER(localframe),localscrollwindow);


  i = 0; // keep track of how many histograms have been listed

  while (i < pgammax) {
    /*--ddc jul11 deprecation of gtklist...
    if ((i % 10) == 0) {
      //--ddc jan11 gtk deprecation      locallistbox = gtk_list_new();
      localliststore=gtk_list_store_new(2,G_TYPE_INT,G_TYPE_STRING);
      locallistbox = gtk_tree_view_new_with_model(localliststore);
      //--ddc jan11 gtk_list_set_selection_mode(GTK_LIST(locallistbox),GTK_SELECTION_MULTIPLE);
      //--ddc      gtk_container_add(GTK_CONTAINER(framehbox),locallistbox);
    }
    */
    sprintf(dummystr,"%d: %s",i+1,twodtitles[i]);
    if((linefeed=strchr(dummystr,'\n')) != NULL ) *linefeed=0; 
    TwodListBoxAddItemDisplay(locallistbox,dummystr,i+1);
    i++;
  }
  
  /* --- create an entry for entering information by hand --- */
  //--ddc jan11 gtk2 deprecation  localentry = gtk_entry_new_with_max_length(80);
  //--ddc move this above...  localentry = gtk_entry_new();
  //--ddc  gtk_entry_set_max_length (GTK_ENTRY (localentry), 80);


  gtk_container_add(GTK_CONTAINER(localvbox),localentry);
  g_signal_connect(GTK_OBJECT(localentry),"activate",
		     G_CALLBACK(PgamDisplaySelectionEntry),localentry);
  g_signal_connect_swapped(GTK_OBJECT(localentry),"activate",
			    G_CALLBACK(CloseWidget),(GtkObject *) localwindow);  

  localhbox = gtk_hbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(localvbox),localhbox);


  localbutton = gtk_button_new_with_label("Display");
  gtk_container_add(GTK_CONTAINER(localhbox),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(TwodDisplayWindowCallback),NULL);
  g_signal_connect_swapped(GTK_OBJECT(localbutton),"clicked",
			    G_CALLBACK(CloseWidget),GTK_OBJECT(localwindow));

  localbutton = gtk_button_new_with_label("Cancel");
  gtk_container_add(GTK_CONTAINER(localhbox),localbutton);
  g_signal_connect_swapped(GTK_OBJECT(localbutton),"clicked",
			    G_CALLBACK(CloseWidget),GTK_OBJECT(localwindow));

  gtk_box_set_child_packing(localvbox,localhbox,FALSE,FALSE,0,GTK_PACK_START);
  gtk_box_set_child_packing(localvbox,localentry,FALSE,FALSE,0,GTK_PACK_START);
  

  gtk_widget_show_all(localwindow);
  gtk_widget_grab_focus(localentry);

}

/* CreateDisplaySelectionWindow
 *
 * Creates a window with a list box for the purpose of selecting what to display
 * mode: 0 = display full range, 1 = display current range, 2 = overlay in active graph
 */
void CreateDisplaySelectionWindow(int mode)
{
  GtkWidget *localwindow;
  GtkWidget *localbutton;
  GtkWidget *localvbox;
  GtkWidget *localhbox;
  GtkWidget *locallistbox;
  //--ddc jan11 gtk deprecation for gtklist
  GtkListStore *localliststore;
  GtkTreeSelection *locallistsel;
  GtkTreeViewColumn *locallistcol;
  GtkCellRenderer *locallistrenderer;
 
  GtkWidget *localframe;
  //--ddc feb11 replace with scrolled window  GtkWidget *framehbox;
  GtkWidget *localscrollwindow;
  int x,y;
  //--ddc jun14 make resizing rational, by grouping buttons and input
  //and disabling their expand and fill with gtk_box_set_child_packing


  GtkWidget *localentry;
  char dummystr[80], *linefeed;
  int i,j;

  /* --- idiot check --- */
  g_return_if_fail(histsize[0] > 0);

  /* --- make the window --- */

  localwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  switch (mode) {
  case 0:
    gtk_window_set_title(GTK_WINDOW(localwindow),"Display Full Domain");
    break;
  case 1:
    gtk_window_set_title(GTK_WINDOW(localwindow),"Display Current Domain");
    break;
  case 2:
    gtk_window_set_title(GTK_WINDOW(localwindow),"Overlay Histograms");
    break;
  }
  gtk_window_set_modal(GTK_WINDOW(localwindow),TRUE);

  /* --- create the localvbox --- */
  
  localvbox = gtk_vbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(localwindow),localvbox);
  

  /* --- create the frame --- */

  localframe = gtk_frame_new("Select histograms to Display");
  gtk_container_add(GTK_CONTAINER(localvbox),localframe);

  /* --- create the horizontal box for the frame --- */

  //--ddc feb11 mod  framehbox = gtk_hbox_new(TRUE,0);
  //--ddc feb11 gtk_container_add(GTK_CONTAINER(localframe),framehbox);

  localentry = gtk_entry_new();
  gtk_entry_set_max_length (GTK_ENTRY (localentry), 80);


  {//this is to replace deprecated gtk_list... (and add scrolling).

      localliststore=gtk_list_store_new(2,G_TYPE_INT,G_TYPE_STRING);

      locallistbox = gtk_tree_view_new_with_model(localliststore);

      locallistrenderer=gtk_cell_renderer_text_new();
      locallistcol = gtk_tree_view_column_new_with_attributes ("Spec #", locallistrenderer,
						   "text", 0, NULL);
      gtk_tree_view_append_column (GTK_TREE_VIEW (locallistbox), locallistcol);

      locallistcol = gtk_tree_view_column_new_with_attributes ("Name", locallistrenderer,
						   "text", 1, NULL);
      gtk_tree_view_append_column (GTK_TREE_VIEW (locallistbox), locallistcol);


      //--ddc jan11 deprecation  gtk_list_set_selection_mode(GTK_LIST(locallistbox),GTK_SELECTION_MULTIPLE);
      //--ddc jan11 need hook up signal handler >here<
      locallistsel=gtk_tree_view_get_selection(GTK_TREE_VIEW(locallistbox));
      gtk_tree_selection_set_mode(locallistsel, GTK_SELECTION_MULTIPLE);
      g_signal_connect(G_OBJECT(locallistsel), "changed", 
		       G_CALLBACK(ListBoxSelectedCallbackDisplay), NULL);

      localscrollwindow=gtk_scrolled_window_new(NULL,NULL);
      gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(localscrollwindow),
					locallistbox);

      pango_layout_get_pixel_size(gtk_entry_get_layout(localentry),&x,&y);
      //      gtk_widget_set_size_request(localscrollwindow,-1,10*y);
      gtk_widget_set_size_request(localscrollwindow,-1,20*y);

      //--ddc feb11 mod      gtk_container_add(GTK_CONTAINER(framehbox),locallistbox);
  }

  gtk_container_add(GTK_CONTAINER(localframe),localscrollwindow);

  i = 0; // to keep track of how many histograms have been listed;

  while (histsize[i] > 0) {

    //-ddc jan11 gtk deprecation, omitted construction of gtklists.
    sprintf(dummystr,"%d: %s",i+1,field2[i]);
    if((linefeed=strchr(dummystr,'\n')) != NULL ) *linefeed=0; 
    ListBoxAddItemDisplay(locallistbox,dummystr,i+1);
    i++;
  }
  
  /* --- create an entry for entering information by hand --- */

  //--ddc jan11 gtk2 deprecation.. the entry was created here... but I needed it get font info.
  //--ddc jan11  localentry = gtk_entry_new_with_max_length(80);
  //  localentry = gtk_entry_new();
  //  gtk_entry_set_max_length (GTK_ENTRY (localentry), 80);

  gtk_container_add(GTK_CONTAINER(localvbox),localentry);
  switch (mode) {
  case 1:
    g_signal_connect(GTK_OBJECT(localentry),"activate",
		       G_CALLBACK(MultipleDisplaySameRangeEntry),localentry);
    break;
  case 2:
    g_signal_connect(GTK_OBJECT(localentry),"activate",
		       G_CALLBACK(MultiInOneEntry),localentry);
    break;
  default:
    /*///hmm.
    g_signal_connect(GTK_OBJECT(localentry),"activate",
		       G_CALLBACK(MultipleDisplayEntry),localentry);
    */
    g_signal_connect(GTK_OBJECT(localentry),"activate",
		       G_CALLBACK(MultipleDisplayEntry),GTK_OBJECT(localentry));
  }

  //--ddc jan11 deprecated  gtk_signal_connect_object(GTK_OBJECT(localentry),"activate",
  g_signal_connect_swapped(GTK_OBJECT(localentry),"activate",
			    G_CALLBACK(CloseWidget),(GtkObject *) localwindow);

  /* --- create a localhbox for the display and cancel buttons --- */

  localhbox = gtk_hbox_new(FALSE,0);

  gtk_container_add(GTK_CONTAINER(localvbox),localhbox);

  gtk_box_set_child_packing(localvbox,localhbox,FALSE,FALSE,0,GTK_PACK_START);
  gtk_box_set_child_packing(localvbox,localentry,FALSE,FALSE,0,GTK_PACK_START);


  /* --- create the display button and cancel button --- */

  localbutton = gtk_button_new_with_label("Display");
  gtk_container_add(GTK_CONTAINER(localhbox),localbutton);
  g_signal_connect(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(DisplayWindowCallback),(int *)mode);
  g_signal_connect_swapped(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(CloseWidget),GTK_OBJECT(localwindow));

  localbutton = gtk_button_new_with_label("Cancel");
  gtk_container_add(GTK_CONTAINER(localhbox),localbutton);
  g_signal_connect_swapped(GTK_OBJECT(localbutton),"clicked",
		     G_CALLBACK(CloseWidget),GTK_OBJECT(localwindow));


  gtk_widget_show_all(localwindow);
  gtk_widget_grab_focus(localentry);

}


static void ListBoxSelectedCallbackDisplay(GtkTreeSelection *widget, gpointer *data)
{
  //--ddc jan11 this is a new callback for the selected items from a treeview widget
  int i,localdata;
  GtkTreeIter iter;
  GtkTreeModel *model;
  GtkTreePath *localpath;
  GList *pathlist;
  
  pathlist = gtk_tree_selection_get_selected_rows(widget, &model);

  if(pathlist!=NULL){
    for(i=0;i<254;i++) display_toggle[i]=0;
    while(pathlist){
      gtk_tree_model_get_iter(model,&iter,pathlist->data);
      gtk_tree_model_get(model,&iter,0,&localdata, -1);
      if(localdata>0 && localdata<255) display_toggle[(int) localdata-1] = 1;
      pathlist=pathlist->next;
    }
    g_list_foreach (pathlist, (GFunc)gtk_tree_path_free, NULL); 
    g_list_free (pathlist); 
  }
}

static void TwodListBoxSelectedCallbackDisplay(GtkTreeSelection *widget, gpointer *data)
{
  //--ddc jan11 this is a new callback for the selected items from a treeview widget
  int i,localdata;
  GtkTreeIter iter;
  GtkTreeModel *model;
  GtkTreePath *localpath;
  GList *pathlist;
  
  pathlist = gtk_tree_selection_get_selected_rows(widget, &model);

  if(pathlist!=NULL){
    for(i=0;i<254;i++) twod_display_toggle[i]=0;
    while(pathlist){
      gtk_tree_model_get_iter(model,&iter,pathlist->data);
      gtk_tree_model_get(model,&iter,0,&localdata, -1);
      if(localdata>0 && localdata<255) twod_display_toggle[(int) localdata-1] = 1;
      pathlist=pathlist->next;
    }
    g_list_foreach (pathlist, (GFunc)gtk_tree_path_free, NULL); 
    g_list_free (pathlist); 
  }
}


/* TwodDisplayWindowCallback
 *
 * When the display window is activated it looks at the twod toggle list
 * and then sets up the appropriate information before attempting a display
 */
void TwodDisplayWindowCallback(GtkWidget *widget, gpointer *data)
{
  int i,j;
  int templist[16];
  int max;
  int which;
  int xmax,ymax;

  i = 0; j = 0;
  while ((j < 16) && (i < pgammax)) {
    if (twod_display_toggle[i]) {
      templist[j] = i;
      j++;
    }
    i++;
  }
  if (j > 0) {
    /* --- have to determine the size of the x and y displays --- */
    xmax = 0; ymax = 0;
    for (i = 0; i < j; i++) {
      if (xmax < big_data_info[templist[i]]->axes[0]) xmax = big_data_info[templist[i]]->axes[0];
      if (ymax < big_data_info[templist[i]]->axes[1]) ymax = big_data_info[templist[i]]->axes[1];
    }
    xcurrentrange[0] = 0;
    ycurrentrange[0] = 0;
    xcurrentrange[1] = xmax;
    ycurrentrange[1] = ymax;
    numtwoddisplayed = j;
    for (i = 0; i < j; i++) {
      twodspectradisplayed[i] = templist[i];
    }
    TwodDisplayCurrentRange();
    for (i = 0; i < j; i++) twod_display_toggle[i] = 0;
  }
}

/* DisplayWindowCallback
 *
 * When the display window is activated it looks at the toggle list and 
 * then sets up the appropriate information before attempting to display
 */
void DisplayWindowCallback(GtkWidget *widget, gpointer *data)
{
  int i,j,k;
  int templist[16];
  int mode;
  int max;
  int which;

  mode = (int) data;

  i = 0;
  j = 0;
  while ((j < 16) && (histsize[i] > 0)) {
    if (display_toggle[i]) {
      templist[j] = i;
      j++;
    }
    i++;
  }
  if (j > 0) {
    /* --- things are handled a little differently for each of the modes --- */
    switch (mode) {
    case 0:
      /* --- this is the display full range one --- */
      /* --- need to find the maxium channels --- */
      max = 0;
      for (i = 0; i < j; i++)
	if (max < histsize[i])
	  max = histsize[i];
      if (max < 0) max = 10;
      currentrange[0] = 0;
      currentrange[1] = max - 1;
      /* --- and then we just continue to the display current range option --- */
    case 1:
      numspectra = j;
      for (i = 0; i < j; i++) {
	spectradisplayed[i][0] = templist[i];
	for (k = 1; k < 10; k++)
	  spectradisplayed[i][k] = -1;
      }
      break;
    case 2:
      /* --- this is the option where we overlay --- */
      /* --- first get the maxium number of channels --- */
      max = 0;
      for (i = 0; i < j; i++)
	if (max < histsize[i])
	  max = histsize[i];
      if (max < 0) max = 10;
      currentrange[0] = 0;
      currentrange[1] = max - 1;
      /* --- now set up the graph --- */
      //numspectra = j;
      which = 0;
      if (numspectra > 1) {
	for (i = 0; i < numspectra; i++) {
	  if (spectra = spectradisplayed[i][0]) which = i;
	}
      }
      for (i = 0; i < 10; i++) 
	spectradisplayed[which][i] = -1;
      for (i = 0; i < j; i++) {
	spectradisplayed[which][i] = templist[i];
      }
      break;
    }

    DisplayCurrentRange();
    for (i = 0; i < j; i++) display_toggle[i] = 0;
  }
}

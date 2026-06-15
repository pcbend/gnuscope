/* output.c
 * by John Pavan
 *
 * Handles the text output of gnuscope
 */

#include <gtk/gtk.h>
#include "gnuscopefuncs.h"
#include "gnuscopeglobals.h"

//--ddc 23jun06 define close for messagetext
void CloseMessageTextWidget(GtkWidget *);

/* --- structures --- */

/* --- globals --- */
/* --- should be included in gnuscopeglobals.h --- */

/* --- function declarations --- */
/* --- should be included in gnuscopefuncs.h --- */

/* --- functions --- */
// gint TextViewKeyCallback(GtkWidget *, gpointer *);

/*
 * WriteMainText
 *
 * Writes a string to the main text box
 */
void WriteMainText(const char *newtext) {
  GtkTextIter startbuffer; //--ddc
  GtkTextView *mainview;   //--ddc

  if (GTK_IS_WIDGET(maintext)) {
    mainview = GTK_TEXT_VIEW(maintext); //--ddc

    gtk_text_buffer_get_start_iter(mytextbuffer(maintext), &startbuffer);

    if (gtk_text_buffer_get_char_count(mytextbuffer(maintext)) > 0) {
      gtk_text_buffer_place_cursor(mytextbuffer(maintext), &startbuffer);
    }

    gtk_text_buffer_insert(mytextbuffer(maintext), &startbuffer, newtext, strlen(newtext));

  } else {
    printf(newtext);
  }
}

/*
 * WriteTwodText
 *
 * Writes a string to the twod text box
 */
void WriteTwodText(const char *newtext) {
  GtkTextIter startbuffer; //--ddc
  GtkTextView *mainview;   //--ddc

  //--kp 3may06  if (twodtext != NULL) {
  if (GTK_IS_WIDGET(twodtext)) {
    mainview = GTK_TEXT_VIEW(twodtext); //--ddc

    gtk_text_buffer_get_start_iter(mytextbuffer(twodtext), &startbuffer);

    if (gtk_text_buffer_get_char_count(mytextbuffer(twodtext)) > 0) {
      gtk_text_buffer_place_cursor(mytextbuffer(twodtext), &startbuffer);
    }

    gtk_text_buffer_insert(mytextbuffer(twodtext), &startbuffer, newtext, strlen(newtext));
  }
}

/*
 * WriteMessageText
 *
 * Writes to the message text window
 * if the message text window does not exist (messagetext == null)
 * then a new message text window is created
 * the message text window should be used for text which is useful for
 * more than one display type (such as file input/output and pgam sorting messages)
 */
void WriteMessageText(const char *newtext) {
  GtkTextIter startbuffer; //--ddc

  if (!GTK_IS_WIDGET(messagetext))
    CreateNewMessageTextWindow();
  g_return_if_fail(GTK_IS_WIDGET(messagetext));

  gtk_text_buffer_get_start_iter(mytextbuffer(messagetext), &startbuffer);

  if (gtk_text_buffer_get_char_count(mytextbuffer(messagetext)) > 0) {
    gtk_text_buffer_place_cursor(mytextbuffer(messagetext), &startbuffer);
  }

  gtk_text_buffer_insert(mytextbuffer(messagetext), &startbuffer, newtext, strlen(newtext));
}

/* CreateNewMessageTextWindow
 *
 * generates a new text window and assigns the "message text" to that window
 */
void CreateNewMessageTextWindow() {
  //  GtkWidget *table;
  GtkWidget *localscrollwindow;
  GtkWidget *localwindow;

  /* --- local window --- */
  localwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(localwindow), 400, 100);
  gtk_window_set_title(GTK_WINDOW(localwindow), "Gnuscope Messages");

  //--ddc 23jun06
  g_signal_connect(GTK_OBJECT(localwindow), "destroy", G_CALLBACK(CloseMessageTextWidget), NULL);

  /* --- table for text and scrollbars --- */

  localscrollwindow = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(localwindow), localscrollwindow);

  /* --- message text --- */

  messagetext = init_text_view();
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(localscrollwindow), messagetext);

  gtk_widget_show(messagetext);
  gtk_widget_show(localscrollwindow);
  gtk_widget_show(localwindow);
}

//--ddc 23jun06 add a handler for window destroy
void CloseMessageTextWidget(GtkWidget *widget) {
  gtk_widget_destroy(widget);
  messagetext = NULL;
}

//--ddc aug11 Some functions for dealing with textview widgets

//--ddc get the textbuffer from the textview widget...

GtkTextBuffer *mytextbuffer(GtkWidget *text) {

  return gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
}

// This initializes a textview for DISPLAYING TEXT!!!

GtkTextView *init_text_view(void) {

  GtkTextView *localview;

  localview = gtk_text_view_new();

  gtk_widget_set_events(GTK_WIDGET(localview), GDK_BUTTON_PRESS_MASK);

  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(localview), FALSE);

  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(localview), GTK_WRAP_WORD);

  gtk_text_view_set_editable(GTK_TEXT_VIEW(localview), FALSE);

  g_signal_connect_after(GTK_OBJECT(localview), "key_press_event", (GCallback)TextViewKeyCallback,
                         NULL);

  return localview;
}

gint TextViewKeyCallback(GtkWidget *widget, gpointer *data) {
  return 0;
}

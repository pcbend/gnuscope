#include <gtk/gtk.h>
// #include <gtk/gtkenums.h>

/* --- structures --- */

/* --- globals --- */
GtkWidget *mainwindow;
int abortflag;
int modalwindowopen = 0;

/* --- function declarations --- */

GtkTextBuffer *mytextbuffer(GtkWidget *text);

void AutoDetectFileType(char *sFilename);
void GetDialog(int type);
void GetDialog(int type);
void SortTypePrompt(GtkWidget *text);
void ChannelsPrompt(GtkWidget *text);
void BadnessPrompt(GtkWidget *text);
void DopplerCorrectionPrompt(GtkWidget *text);
void TwdorSqrPrompt(GtkWidget *text);
void SortTypeEntry(GtkWidget *widget, GtkWidget *entry);
void ChannelsEntry(GtkWidget *widget, GtkWidget *entry);
void BadnessEntry(GtkWidget *widget, GtkWidget *entry);
void DopplerCorrectionEntry(GtkWidget *widget, GtkWidget *entry);
void MultipleDisplaySameRangeEntry(GtkWidget *widget, GtkWidget *entry);
void TwdorSqrEntry(GtkWidget *widget, GtkWidget *entry);
void RetainSpectraPrompt(GtkWidget *text);
void RetainSpectraEntry(GtkWidget *widget, GtkWidget *entry);
void ManualMarkerPrompt(GtkWidget *text);
void ManualMarkerEntry(GtkWidget *widget, GtkWidget *entry);
void ExpandPlotPrompt(GtkWidget *text);
void ExpandPlotEntry(GtkWidget *widget, GtkWidget *entry);
void SetYScaleEntry(GtkWidget *widget, GtkWidget *entry);
void SetYScalePrompt(GtkWidget *text);
void MultipleDisplayEntry(GtkWidget *widget, GtkWidget *entry);
void MultipleDisplayPrompt(GtkWidget *text);
void AddSpectraEntry(GtkWidget *widget, GtkWidget *entry);
void AddSpectraPrompt(GtkWidget *text);
void SubtractSpectraEntry(GtkWidget *widget, GtkWidget *entry);
void SubtractSpectraPrompt(GtkWidget *text);
void MoveSpectraEntry(GtkWidget *widget, GtkWidget *entry);
void MoveSpectraPrompt(GtkWidget *text);
void NormalizeSpectraEntry(GtkWidget *widget, GtkWidget *entry);
void NormalizeSpectraPrompt(GtkWidget *text);
void CompressSpectraEntry(GtkWidget *widget, GtkWidget *entry);
void CompressSpectraPrompt(GtkWidget *text);
void ClearSpectraEntry(GtkWidget *widget, GtkWidget *entry);
void ClearSpectraPrompt(GtkWidget *text);
void TitleSpectraEntry(GtkWidget *widget, GtkWidget *entry);
void TitleSpectraPrompt(GtkWidget *text);
void GainshiftSpectraEntry(GtkWidget *widget, GtkWidget *entry);
void GainshiftSpectraPrompt(GtkWidget *text);
void FixedWidthGaussFitEntry(GtkWidget *widget, GtkWidget *entry);
void FixedWidthGaussFitPrompt(GtkWidget *text);
void GainshiftSpectraPrompt(GtkWidget *text);
void DoubleGaussFitEntry(GtkWidget *widget, GtkWidget *entry);
void DoubleGaussFitPrompt(GtkWidget *text);
void GetCalibrationEntry(GtkWidget *widget, GtkWidget *entry);
void GetCalibrationPrompt(GtkWidget *text);
void ManualSetBackgroundEntry(GtkWidget *widget, GtkWidget *entry);
void ManualSetBackgroundPrompt(GtkWidget *text);
void RetainSpectraEntry(GtkWidget *widget, GtkWidget *entry);
void RetainSpectraPrompt(GtkWidget *text);
void WriteFileEntry(GtkWidget *widget, GtkWidget *entry);
void WriteFilePrompt(GtkWidget *text);
void SetEfficiencyPrompt(GtkWidget *text);
void SetEfficiencyEntry(GtkWidget *widget, GtkWidget *entry);
// void FitsTauPrompt(GtkWidget *text);
// void FitsTauEntry(GtkWidget *widget, GtkWidget *entry);
void CloseWidget(GtkWidget *widget, gpointer data);
void FindPeaksEntry(GtkWidget *widget, GtkWidget *entry);
void FindPeaksPrompt(GtkWidget *text);
void CloseWidgetCancel(GtkWidget *widget, gpointer data);
void SpawnPrintEntry(GtkWidget *widget, GtkWidget *entry);
void SpawnPrintPrompt(GtkWidget *text);
void ReadBigPrompt(GtkWidget *text);
void ReadBigEntry(GtkWidget *widget, GtkWidget *entry);
void WriteBigPrompt(GtkWidget *text);
void WriteBigEntry(GtkWidget *widget, GtkWidget *entry);
void MultiInOnePrompt(GtkWidget *text);
void MultiInOneEntry(GtkWidget *widget, GtkWidget *entry);
void NumContoursPrompt(GtkWidget *text);
void NumContoursEntry(GtkWidget *widget, GtkWidget *entry);
void PgamSpawnPrintEntry(GtkWidget *widget, GtkWidget *entry);
void PgamSetTwodTitleEntry(GtkWidget *widget, GtkWidget *entry);
void PgamSetTwodTitlePrompt(GtkWidget *text);
void PgamDisplaySelectionEntry(GtkWidget *widget, GtkWidget *entry);
void PgamDisplaySelectionPrompt(GtkWidget *text);
void PgamClear2DEntry(GtkWidget *widget, GtkWidget *entry);
void PgamClear2DPrompt(GtkWidget *text);
void PgamAdd2DEntry(GtkWidget *widget, GtkWidget *entry);
void PgamAdd2DPrompt(GtkWidget *text);
void PgamSubtract2DEntry(GtkWidget *widget, GtkWidget *entry);
void PgamSubtract2DPrompt(GtkWidget *text);
void YCalibrationEntry(GtkWidget *widget, GtkWidget *entry);
void XCalibrationEntry(GtkWidget *widget, GtkWidget *entry);
void XCalibrationPrompt(GtkWidget *text);
void YCalibrationPrompt(GtkWidget *text);
void CloseModalWidget(GtkWidget *widget, gpointer data);

/* --- functions --- */

/* CloseWidgetCancel
 * Closes the widget and sends a signal for canceling the action
 */
void CloseWidgetCancel(GtkWidget *widget, gpointer data) {
  abortflag = 1;
  /*   gtk_widget_destroy(GTK_WIDGET(data)); */
  gtk_widget_destroy(widget);
  widget = NULL;
}

/* CloseModalWidget
 *
 * Closes a modal widget and sets the modalwindowopen flag to 0
 */
void CloseModalWidget(GtkWidget *widget, gpointer data) {
  /*   gtk_widget_destroy(GTK_WIDGET(data)); */
  gtk_widget_destroy(widget);
  modalwindowopen = 0;
}

/*
 * GetMessageDialog
 *
 * Creates a dialog containing a particular text
 * the window can be closed, but nothing else
 */
void GetMessageDialog(const char *message) {
  GtkWidget *dialog;
  GtkWidget *text;
  GtkWidget *ok_button;
  GtkWidget *textbox;
  GtkWidget *vscrollbar;

  /* --- make the dialog --- */
  if (!modalwindowopen) {
    dialog = gtk_dialog_new();
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    //--ddc aug11    gtk_signal_connect (GTK_OBJECT(dialog),"delete_event",
    g_signal_connect(GTK_OBJECT(dialog), "delete_event", G_CALLBACK(CloseModalWidget), NULL);
    //--ddc aug11    gtk_signal_connect (GTK_OBJECT(dialog),"destroy",
    g_signal_connect(GTK_OBJECT(dialog), "destroy", G_CALLBACK(CloseModalWidget), dialog);
    /* --- make the ok button --- */
    ok_button = gtk_button_new_with_label("OK");
    //--ddc aug11   gtk_signal_connect_object(GTK_OBJECT(ok_button),"clicked",
    g_signal_connect_swapped(GTK_OBJECT(ok_button), "clicked", G_CALLBACK(CloseModalWidget),
                             (GtkObject *)dialog);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area), ok_button);
    /* --- make the text box --- */
    textbox = gtk_hbox_new(FALSE, 0);
    //    text = gtk_text_new(NULL,NULL);
    text = gtk_text_view_new();
    //    gtk_text_set_editable(GTK_TEXT(text),FALSE);
    gtk_box_pack_start(GTK_BOX(textbox), text, GTK_FILL, GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0);
    //    vscrollbar = gtk_vscrollbar_new(GTK_TEXT(text)->vadj);
    vscrollbar = gtk_vscrollbar_new(GTK_TEXT_VIEW(text)->vadjustment);
    gtk_container_add(GTK_CONTAINER(textbox), vscrollbar);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), textbox);
    /* --- write in the text --- */
    //    gtk_text_freeze(GTK_TEXT_VIEW(text)); // must freeze widget to write it
    //    gtk_text_insert(GTK_TEXT_VIEW(text),NULL,NULL,NULL,message,strlen(message));
    gtk_text_buffer_set_text(mytextbuffer(text), message, strlen(message));
    //    gtk_text_thaw(GTK_TEXT_VIEW(text)); // must thaw the widget to display
    modalwindowopen = 1; // tell the rest of the program that a modal window is open
    gtk_widget_show_all(dialog);
    gtk_widget_grab_focus(ok_button);
  }
}

/*
 * GetSingleDialog
 *
 * Creates a dialog box with a title
 * and uses one function to get the message
 * and annother to enterpret the result
 *
 * use:  GetDialog(int type)
 * types:  0 = nothing 1 = display 2 = retain spectra prompt
 *         3 = manually set a marker 4 = Manual Expand Plot
 *         5 = set the y scale 6 = multiple display same range
 *         7 = Add spectra 8 = Subtract Spectra 9 = Move Spectra
 *         10 = Normalize Spectra 11 = Compress Spectra 12 = Clear Spectra
 *         13 = title spectra 14 = gainshift spectra
 *         15 = Fixed Width Gauss Fit 16 = double gauss fit
 *         17 = set calibration 18 = manually set the background
 *         19 = sorttype 20 = channels 21 = badness
 *         22 = Dopplershift correction 23 = twd or sqr
 *         24 = Write File 25 = efficency calibration
 *         26 = print 27 = Read 2D 28 = write 2D
 *         30 = multiple spectrum in 1 histogram
 *         31 = Set number of Contours in 2D display
 *         32 = pgamprint
 *         33 = set the title of a 2D display
 *         34 = Select which 2D to display
 *         35 = Case of destroying a 2D histogram from memory
 *         36 = Adding 2 2D histograms
 *         37 = Subtracting 2 2D histograms
 *         38 = Setting the Calibration of the X-Axis of a 2d Plot
 *         39 = Setting the Calibration of the Y-Axis of a 2D plot
 *         50 = Find Peaks
 *         100 = get tau range
 */
void GetDialog(int type) {
  int i, j, k;
  GtkWidget *dialog;
  GtkWidget *text;
  GtkWidget *entry;
  GtkWidget *cancel_button;
  GtkWidget *localvbox;
  GtkWidget *textbox;
  GtkWidget *vscrollbar;

  if (type != 0) {
    /* --- make the dialog --- */
    dialog = gtk_dialog_new();
    //  gtk_window_set_transient_for((GtkWindow *)mainwindow,(GtkWindow *)dialog);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    /* --- make the vbox --- */
    localvbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area), localvbox);
    /* --- make the entry box --- */
    entry = gtk_entry_new_with_max_length(40);
    gtk_box_pack_start(GTK_BOX(localvbox), entry, FALSE, FALSE, 0);
    /* --- make the cancel button --- */
    cancel_button = gtk_button_new_with_label("Cancel");
    //--ddc aug11 deprecation gtk_signal_connect_object(GTK_OBJECT(cancel_button),"clicked",
    g_signal_connect_swapped(GTK_OBJECT(cancel_button), "clicked", G_CALLBACK(CloseWidgetCancel),
                             (GtkObject *)dialog);
    gtk_container_add(GTK_CONTAINER(localvbox), cancel_button);
    /* --- make the text box --- */
    textbox = gtk_hbox_new(FALSE, 0);
    //  text = gtk_text_new(NULL,NULL);
    text = gtk_text_view_new();
    //  gtk_text_set_editable(GTK_TEXT(text),FALSE);
    gtk_box_pack_start(GTK_BOX(textbox), text, GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                       GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0);
    vscrollbar = gtk_vscrollbar_new(GTK_TEXT_VIEW(text)->vadjustment);
    gtk_container_add(GTK_CONTAINER(textbox), vscrollbar);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), textbox);
    /* --- let's attach the appropriate functions to the entry
       --- and input the appropriate text to the text box --- */
    //  gtk_text_freeze(GTK_TEXT_VIEW(text));  // if we don't freeze the text the program dumps
    switch (type) {
    case 1:
      /* --- case of prompting for multiple display --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(MultipleDisplayEntry), entry);
      MultipleDisplayPrompt(text);
      break;
    case 2:
      /* --- prompt to retain histograms in memory before loading the file --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(RetainSpectraEntry), entry);
      /* --- say how many histograms are in memory and if they should be retained --- */
      RetainSpectraPrompt(text);
      break;
    case 3:
      /* --- case of manually setting a marker --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(ManualMarkerEntry), entry);
      ManualMarkerPrompt(text);
      break;
    case 4:
      /* --- case of manually expanding the plot --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(ExpandPlotEntry), entry);
      ExpandPlotPrompt(text);
      break;
    case 5:
      /* --- case of setting the yscale --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(SetYScaleEntry), entry);
      SetYScalePrompt(text);
      break;
    case 6:
      /* --- case of prompting for multiple display --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(MultipleDisplaySameRangeEntry),
                       entry);
      MultipleDisplayPrompt(text);
      break;
    case 7:
      /* --- case of add spectra  --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(AddSpectraEntry), entry);
      AddSpectraPrompt(text);
      break;
    case 8:
      /* --- case of subtract spectra  --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(SubtractSpectraEntry), entry);
      SubtractSpectraPrompt(text);
      break;
    case 9:
      /* --- case of Move spectra  --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(MoveSpectraEntry), entry);
      MoveSpectraPrompt(text);
      break;
    case 10:
      /* --- case of Normalize spectra  --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(NormalizeSpectraEntry), entry);
      NormalizeSpectraPrompt(text);
      break;
    case 11:
      /* --- case of Compress spectra  --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(CompressSpectraEntry), entry);
      CompressSpectraPrompt(text);
      break;
    case 12:
      /* --- case of Clear spectra  --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(ClearSpectraEntry), entry);
      ClearSpectraPrompt(text);
      break;
    case 13:
      /* --- case of Title spectra  --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(TitleSpectraEntry), entry);
      TitleSpectraPrompt(text);
      break;
    case 14:
      /* --- case of Gainshift spectra  --- */
      //--ddc    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(GainshiftSpectraEntry), entry);
      GainshiftSpectraPrompt(text);
      break;
    case 15:
      /* --- case of Fixed Width Gauss Fit spectra  --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(FixedWidthGaussFitEntry), entry);
      FixedWidthGaussFitPrompt(text);
      break;
    case 16:
      /* --- case of Fixed Width Gauss Fit spectra  --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(DoubleGaussFitEntry), entry);
      DoubleGaussFitPrompt(text);
      break;
    case 17:
      /* --- case of setting calibrations  --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(GetCalibrationEntry), entry);
      GetCalibrationPrompt(text);
      break;
    case 18:
      /* --- case of setting background  --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(ManualSetBackgroundEntry), entry);
      ManualSetBackgroundPrompt(text);
      break;
    case 19:
      /* --- case of setting sorttype  --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(SortTypeEntry), entry);
      SortTypePrompt(text);
      break;
    case 20:
      /* --- case of particle channels  --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(ChannelsEntry), entry);
      ChannelsPrompt(text);
      break;
    case 21:
      /* --- case of setting badness  --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(BadnessEntry), entry);
      BadnessPrompt(text);
      break;
    case 22:
      /* --- case of setting DopplerCorrection  --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(DopplerCorrectionEntry), entry);
      DopplerCorrectionPrompt(text);
      break;
    case 23:
      /* --- case of setting twd or sqr  --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(TwdorSqrEntry), entry);
      TwdorSqrPrompt(text);
      break;
    case 24:
      /* --- case of setting writing spectra  --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(WriteFileEntry), entry);
      WriteFilePrompt(text);
      break;
    case 25:
      /* --- case of setting writing spectra  --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(SetEfficiencyEntry), entry);
      SetEfficiencyPrompt(text);
      break;
    case 26:
      /* --- case of printing image of the window --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(SpawnPrintEntry), entry);
      SpawnPrintPrompt(text);
      break;
    case 27:
      /* --- case of reading 2d plots --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(ReadBigEntry), entry);
      ReadBigPrompt(text);
      break;
    case 28:
      /* --- case of writing 2d plots --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(WriteBigEntry), entry);
      WriteBigPrompt(text);
      break;
    case 30:
      /* --- case of displaying multiple histograms in 1 plot --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(MultiInOneEntry), entry);
      MultiInOnePrompt(text);
      break;
    case 31:
      /* --- case of setting the number of contours to display --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(NumContoursEntry), entry);
      NumContoursPrompt(text);
      break;
    case 32:
      /* --- case of printing image of the window --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(PgamSpawnPrintEntry), entry);
      SpawnPrintPrompt(text);
      break;
    case 33:
      /* --- case of setting the 2D title--- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(PgamSetTwodTitleEntry), entry);
      PgamSetTwodTitlePrompt(text);
      break;
    case 34:
      /* --- case of selecting the 2D to display --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(PgamDisplaySelectionEntry), entry);
      PgamDisplaySelectionPrompt(text);
      break;
    case 35:
      /* --- case of deleting a 2D histogram --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(PgamClear2DEntry), entry);
      PgamClear2DPrompt(text);
      break;
    case 36:
      /* --- case of adding 2 2D histograms --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(PgamAdd2DEntry), entry);
      PgamAdd2DPrompt(text);
      break;
    case 37:
      /* --- case of Subtracting 2 2D histograms --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(PgamSubtract2DEntry), entry);
      PgamSubtract2DPrompt(text);
      break;
    case 38:
      /* --- case of Setting the calibration of the Xaxis of a 2d plot --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(XCalibrationEntry), entry);
      XCalibrationPrompt(text);
      break;
    case 39:
      /* --- case of Setting the calibration of the Xaxis of a 2d plot --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(YCalibrationEntry), entry);
      YCalibrationPrompt(text);
      break;
    case 50:
      /* --- case of the peak finding algorythm --- */
      //--ddc aug11    gtk_signal_connect(GTK_OBJECT(entry),"activate",
      g_signal_connect(GTK_OBJECT(entry), "activate", G_CALLBACK(FindPeaksEntry), entry);
      FindPeaksPrompt(text);
      break;

    default:
      /* --- do nothing --- */
      break;
    }
    //  gtk_text_thaw(GTK_TEXT_VIEW(text));  // have to unfreeze the text when we are done

    /* --- close the dialog box after info is entered --- */
    //--ddc aug11  gtk_signal_connect_object(GTK_OBJECT(entry),"activate",
    g_signal_connect_swapped(GTK_OBJECT(entry), "activate", G_CALLBACK(CloseWidget),
                             (GtkObject *)dialog);

    /* --- show the dialog --- */
    gtk_widget_show_all(dialog);
    gtk_widget_grab_focus(entry);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 200);
  }
}

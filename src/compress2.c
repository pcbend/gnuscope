/* Compress2
 *
 * by John Pavan
 *
 * Functions for converting an uncompressed .mul file to a .ev2 file
 */

/* --- format of an .ev2 file --- */

/* an .ev2 file follows the following format
 * (1byte) ADC's Fired
 * (1byte) ADC Label 1
 * (2bytes) ADC Channel 1
 * (1byte) ADC Label 2
 * (2bytes) ADC Channel 2
 * and so on
 * (1byte) echo ADCs Fired (for checking)
 * next event
 */

/* --- includes --- */

#include <stdio.h>
//--ddc 3jun08 add missing function prototypes (stdlib, and gnuscopefuncs.h)
#include <stdlib.h>
#include <sys/stat.h>

#include "gnuscopefuncs.h"

/* --- structs --- */

/* --- globals --- */

/* --- function declarations --- */

void Compress2FileFromMenu(char *sFilename);
void Compress2File(char *infilename, char *outfilename);
void Compress2FileNOGTK(char *sFilename);

/* --- functions --- */

/* Compress2FileFromMenu
 *
 * Gets a single file name, chops off the end of it
 * adds the .ev2 file extention, and passes that to
 * the Compress2File procedure
 */
void Compress2FileFromMenu(char *sFilename)
{
  char dummystr[120];
  if (strstr(sFilename,".mul") != NULL) {
    sprintf(dummystr,"");
    strncat(dummystr,sFilename,strlen(sFilename)-4);
    strcat(dummystr,".ev2");
    Compress2File(sFilename,dummystr);
  } else {
    GetMessageDialog("Error not an .mul file.\n");
  }
}


/* Compress2File
 *
 * Converts infilename (mul) to outfilename (ev2)
 */
void Compress2File(char *infilename, char *outfilename)
{
  /* --- local variables --- */
  const int num_adcs = 256;
  FILE *infile;
  FILE *outfile;
  int i,j,k;
  char *outbuffer = NULL;
  short int inbuffer[8192];
  char *adclabels = NULL;
  short int *adcchns = NULL;
  short int z,y,x;
  short int currentadc;
  char adcsfired;
  char tempchar;
  int outcounter;
  struct stat filebuffer;
  int filelen;

  //--ddc
  int eof=0;

  /* --- real code --- */
  
  outcounter = 0;

  /* --- get the length of the file --- */
  stat(infilename,&filebuffer);
  filelen = filebuffer.st_size;

  if ((infile = fopen(infilename,"r")) != NULL) {
    printf("Compressing %s.\n",infilename);
    StartSortProgress();
    if ((outfile = fopen(outfilename,"w")) != NULL) {
      /* --- now that the files are open, we need to allocate memory for stuff --- */
      adclabels = (char *) malloc (num_adcs * sizeof(char));
      adcchns = (short int *) malloc(num_adcs * sizeof(short int));
      outbuffer = (char *) malloc(8192);

      adcsfired = 0;
      currentadc = 1;

      i = 0; // a counter for the number of buffers read
      k = 0;
      /* --- try to read in the in buffer --- */

      //--ddc      while (fread(&inbuffer,sizeof(short int),8192,infile)) {
      while ((eof=fread(inbuffer,sizeof(short int),8192,infile))!=0) {
	/* --- now we need to scan through the in buffer --- */
	//--ddc	for (j = 0; j < 8192; j++) {
	for (j = 0; j < eof; j++) {
	  if (inbuffer[j] < 0) { // this means it is a new event
	    /* --- first let's write out the current event to the outbuffer --- */
	    /* --- we are going to want to "pad" the end of an outbuffer
	       --- so that events don't cross buffers --- */
	    if ((outcounter + (4 + adcsfired * 4)) >= 8192){
	      for (k = outcounter; k < 8192; k++) {
		outbuffer[k] = 0;
	      }
	      fwrite(outbuffer,1,8192,outfile);
	      outcounter = 0;
	      /* --- do the gtk-friendly stuff --- */
	      while(gtk_events_pending()) {
		gtk_main_iteration();
	      }
	      UpdateSortProgress(ftell(infile),filelen);
	    }
	    if (adcsfired > 0) {
	      *(outbuffer + outcounter) = adcsfired;
	      outcounter++;
	      for (k = 0; k < adcsfired; k++) {
		*(outbuffer + outcounter) = adclabels[k];
		outcounter++;
		tempchar = (char) (adcchns[k]);
		*(outbuffer + outcounter) = tempchar;
		outcounter++;
		tempchar = (char) (adcchns[k] >> 8);
		*(outbuffer + outcounter) = tempchar;
		outcounter++;
	      }    
	      *(outbuffer + outcounter) = adcsfired;
	      outcounter++;
	    
	    }
	    /* --- now that we (hopefully) have written out the event to the outbuffer --- */
	    /* --- we need to re-set the event --- */
	    adcsfired = 0;
	    currentadc = 1;
	  } else { // this is the case for a non-new event
	    if (inbuffer[j]  > 0) {
	      adclabels[adcsfired] = currentadc;
	      adcchns[adcsfired] = inbuffer[j];
	      adcsfired++;
	    }
	    currentadc ++;
	  }
	} // done looking through the in buffer
      }
      //--ddc  oops, probably need to put the last event in the output
      //       buffer too!!
	    if (adcsfired > 0) {
	      *(outbuffer + outcounter) = adcsfired;
	      outcounter++;
	      for (k = 0; k < adcsfired; k++) {
		*(outbuffer + outcounter) = adclabels[k];
		outcounter++;
		tempchar = (char) (adcchns[k]);
		*(outbuffer + outcounter) = tempchar;
		outcounter++;
		tempchar = (char) (adcchns[k] >> 8);
		*(outbuffer + outcounter) = tempchar;
		outcounter++;
	      }    
	      *(outbuffer + outcounter) = adcsfired;
	      outcounter++;
	    }
      //--ddc end of last event.
	/* --- make sure we write out whatever is left in the outbuffer --- */
      if (outcounter > 0) {
	fwrite(outbuffer,1,outcounter,outfile);
      }
      /* --- fread should have returned a 0 if there is no more file --- */
      free(adclabels);
      free(adcchns);
      free(outbuffer);
      EndSortProgress();
      fclose(outfile);
    }
    fclose(infile);
  }
}

/* Compress2FileNOGTK
 * 
 * Compresses a file without any of the GTK friendly stuff
 * this prevents hanging when run outside of the 
 * gtk_main loop
 */
void Compress2FileNOGTK(char *sFilename)
{

  char infilename[120];
  char outfilename[120];
  
  if (strstr(sFilename,".mul") != NULL) {
    sprintf(infilename,sFilename);
    sprintf(outfilename,"");
    strncat(outfilename,sFilename,strlen(sFilename)-4);
    strcat(outfilename,".ev2");
  } else {
    GetMessageDialog("Error not an .mul file.\n");
  }

  {  
  /* --- local variables --- */
  const int num_adcs = 256;
  FILE *infile;
  FILE *outfile;
  int i,j,k;
  char *outbuffer = NULL;
  short int inbuffer[8192];
  char *adclabels = NULL;
  short int *adcchns = NULL;
  short int z,y,x;
  short int currentadc;
  char adcsfired;
  char tempchar;
  int outcounter;
  struct stat filebuffer;
  int filelen;

  //--ddc
  int eof=0;

  /* --- real code --- */
  
  outcounter = 0;

  /* --- get the length of the file --- */
  stat(infilename,&filebuffer);
  filelen = filebuffer.st_size;

  if ((infile = fopen(infilename,"r")) != NULL) {
    //    printf("Compressing %s.\n",infilename);
    //    StartSortProgress();
    if ((outfile = fopen(outfilename,"w")) != NULL) {
      /* --- now that the files are open, we need to allocate memory for stuff --- */
      adclabels = (char *) malloc (num_adcs * sizeof(char));
      adcchns = (short int *) malloc(num_adcs * sizeof(short int));
      outbuffer = (char *) malloc(8192);

      adcsfired = 0;
      currentadc = 1;

      i = 0; // a counter for the number of buffers read
      k = 0;
      /* --- try to read in the in buffer --- */
      //--ddc      while (fread(inbuffer,sizeof(short int),8192,infile)) {
      while ((eof=fread(inbuffer,sizeof(short int),8192,infile))!=0) {
	/* --- now we need to scan through the in buffer --- */
	for (j = 0; j < 8192; j++) {
	  if (inbuffer[j] < 0) { // this means it is a new event
	    /* --- first let's write out the current event to the outbuffer --- */
	    /* --- we are going to want to "pad" the end of an outbuffer
	       --- so that events don't cross buffers --- */
	    if ((outcounter + (4 + adcsfired * 4)) >= 8192){
	      for (k = outcounter; k < 8192; k++) {
		outbuffer[k] = 0;
	      }
	      fwrite(outbuffer,1,8192,outfile);
	      outcounter = 0;
	      /* --- do the gtk-friendly stuff --- */
	      //      while(gtk_events_pending()) {
	      //	gtk_main_iteration();
	      //}
	      //UpdateSortProgress(ftell(infile),filelen);
	      if (((int)(((float) ftell(infile) / (float) filelen) * 100)
		   % 10) == 0)
		printf(".",(int)
		       (((int)(((float)ftell(infile) / 
				(float) filelen) * 100))));
	    }
	    if (adcsfired > 0) {
	      *(outbuffer + outcounter) = adcsfired;
	      outcounter++;
	      for (k = 0; k < adcsfired; k++) {
		*(outbuffer + outcounter) = adclabels[k];
		outcounter++;
		tempchar = (char) (adcchns[k]);
		*(outbuffer + outcounter) = tempchar;
		outcounter++;
		tempchar = (char) (adcchns[k] >> 8);
		*(outbuffer + outcounter) = tempchar;
		outcounter++;
	      }    
	      *(outbuffer + outcounter) = adcsfired;
	      outcounter++;
	    
	    }
	    /* --- now that we (hopefully) have written out the event to the outbuffer --- */
	    /* --- we need to re-set the event --- */
	    adcsfired = 0;
	    currentadc = 1;
	  } else { // this is the case for a non-new event
	    if (inbuffer[j]  > 0) {
	      adclabels[adcsfired] = currentadc;
	      adcchns[adcsfired] = inbuffer[j];
	      adcsfired++;
	    }
	    currentadc ++;
	  }
	} // done looking through the in buffer
      }
      //--ddc  oops, probably need to put the last event in the output
      //       buffer too!!
	    if (adcsfired > 0) {
	      *(outbuffer + outcounter) = adcsfired;
	      outcounter++;
	      for (k = 0; k < adcsfired; k++) {
		*(outbuffer + outcounter) = adclabels[k];
		outcounter++;
		tempchar = (char) (adcchns[k]);
		*(outbuffer + outcounter) = tempchar;
		outcounter++;
		tempchar = (char) (adcchns[k] >> 8);
		*(outbuffer + outcounter) = tempchar;
		outcounter++;
	      }    
	      *(outbuffer + outcounter) = adcsfired;
	      outcounter++;
	    }
      //--ddc end of last event.
	/* --- make sure we write out whatever is left in the outbuffer --- */
      if (outcounter > 0) {
	fwrite(outbuffer,1,outcounter,outfile);
      }
      /* --- fread should have returned a 0 if there is no more file --- */
      free(adclabels);
      free(adcchns);
      free(outbuffer);
      //      EndSortProgress();
      printf("done\n");
      fclose(outfile);
    }
    fclose(infile);
  }
  }
}

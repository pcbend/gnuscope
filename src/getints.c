#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getints(FILE *infile, int *ints, int num){

  int chars,count,dummy;
  char line[132],*lineptr,*lastlineptr;

  count=0;

  while( (lastlineptr=fgets(line,132,infile)) != NULL ){ 
    if(index(line,'\n') == NULL) {
      printf("Possible problem with format of input file\n  :%s\n",line);
      return(-1);
    }
    /*eat any repeated spaces, a gap must be a number*/
    while( (lineptr=index(lastlineptr,' ')) != NULL ){
      if(lineptr > lastlineptr){
	if( (sscanf(lastlineptr,"%d",ints+count)) != 1){
	  printf("Item in line that is not a number or separator!\n :%s\n",
		 line);
	  return(-1);
	}
	count++;
	lastlineptr=lineptr+1;
	if(count==num){
	  if( (sscanf(lastlineptr,"%d",&dummy)) == 1){	  
	    printf("Items are left on line that will be discarded!\n :%s\n",
		 line);
	  return(count);
	  }
	}
      } else {
	lastlineptr=lineptr+1;
      }
    }
	  
 /*unless the line ends with spaces, there should be a number
    between the last space found, and the EOL*/
    if( (sscanf(lastlineptr,"%d",ints+count)) == 1){
      count++;
    }

    if (count==num) break;

  }

  return(count);

}
	   
   


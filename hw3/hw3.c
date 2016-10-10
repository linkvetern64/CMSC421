#define _POSIX_SOURCE
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>

int B;
int T;


int main(int argc, char *argv[]){
  FILE *file;
  char buf[80];

  file = fopen("unsafetrack1.data", "r");
  //Sets the runtime and number of busses
  if(fgets(buf, 80, file) != NULL){B = atoi(buf);};
  if(fgets(buf, 80, file) != NULL){T = atoi(buf);};
  
  while(fgets(buf, 80, file) != NULL){
    //printf("%d \n", atoi(buf));
  }
  fclose(file); 
}

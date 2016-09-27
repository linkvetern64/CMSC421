#define _POSIX_SOURCE
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int numHandles = 0;

int main(int argc, char *argv[]){
  wait();
  char * name = argv[1];
  char * position = argv[2];
  
  printf("#%ld:%s (%s)\n",getpid(), name, position);
  
  return 0;
}

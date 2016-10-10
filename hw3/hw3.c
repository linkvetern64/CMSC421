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
#include <pthread.h>

int mandist(int, int, int, int);
void *print_message_function(void *ptr);
int B;
int T;

int main(int argc, char *argv[]){
  FILE *file;
  char buf[80];
  pthread_t bus[B];
  const char *message1 = "bus 1";
  
  if(argc != 2){
    printf("Need exactly 1 argument. Exiting...\n");
    exit(0);
  }
  else if(access(argv[1], F_OK) == -1){
    printf("File does not exist. Exiting...\n");
    exit(0);
  }
  
  file = fopen(argv[1], "r");
  //Sets the runtime and number of busses
  if(fgets(buf, 80, file) != NULL){B = atoi(buf);};
  if(fgets(buf, 80, file) != NULL){T = atoi(buf);};
  
  while(fgets(buf, 80, file) != NULL){
    //printf("%d \n", atoi(buf));
  }
  fclose(file); 

  for(int i = 0; i < B; i++){
    if(pthread_create(&bus[i], NULL, print_message_function, (void *)message1)){
      printf("Failure in creating thread %d\n", i);
    }
    else{
      printf("Created thread %d successfully \n", i);
    }
  }
}

int mandist(int x1, int y1, int x2, int y2){ 
  return (abs(x1 - x2) + abs(y1 - y2));
}

void *print_message_function(void *ptr){
  char *message;
  message = (char *)ptr;
  printf("%s \n", message);
}

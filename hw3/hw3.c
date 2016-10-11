#define _POSIX_SOURCE
#define _BSD_SOURCE //For usleep
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
struct station{int x, y, passengers;};

//exactly 8 stations
//Test out of coordinate problems *********************** 10,10
struct station stops[8];
//going to spawn like 100 busses
int B;
int T;



int main(int argc, char *argv[]){
  FILE *file;
  char buf[80];
  pthread_t bus[B];
  char * coord;
  char * pch;
  const char *message1 = "bus 1";

  
  if(argc != 2){
    printf("Need exactly 1 argument. Exiting...\n");
    exit(0);
  }
  else if(access(argv[1], F_OK) == -1){
    printf("File does not exist. Exiting...\n");
    exit(0);
  }
  //2 threads for all locations
  //Lock station as thread gets there.
  //How to tell if there's a race condition
  //should not be more than max amount of people
  //Do malloc for arrays
  //Multiple stations at same coordinates
  //FREE MALLOC
  // 2 dimensional array, allow coordinates and #stations there
  //Maybe no busses at all
  //Check size of bus array
  file = fopen(argv[1], "r");
  //Sets the runtime and number of busses
  if(fgets(buf, 80, file) != NULL){B = atoi(buf);};
  if(fgets(buf, 80, file) != NULL){T = atoi(buf);};
  
  for(int i = 0; i < 8; i++){
    //Not tokenized.  Needs to be tokenized
    fgets(buf, 80, file);
    
    coord = (char *)&buf;
    pch = strtok(coord, " ");
    while(pch != NULL){
      stops[i].x = atoi(pch);
      stops[i].y = atoi(pch);
      //printf("%d ", atoi(pch));
      pch = strtok(NULL, " ");
    }
 
    //printf("%s \n", buf);
    //if(fgets(buf, 80, file) != NULL){stops[i].x = atoi(buf);}
    //if(fgets(buf, 80, file) != NULL){stops[i].y = atoi(buf);}
    
    //printf("Station %d at (%d,%d)\n",i, stops[i].x, stops[i].y);
  }
  //TEST IF THERES NO PASSENGERS
  while(fgets(buf, 80, file) != NULL){
    //Fill structs here
    printf("%s \n", buf);
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
  //Need to join threads so 
  for(int j = 0; j < B; j++){
     pthread_join(bus[j], NULL);
     pthread_exit(&bus[j]);
  }
  
  return 0;
}

/**
 * function - mandist :
 * Parameter(s): int x1, y1, x2, y2 : x1, y1 and x2, y2 are coordinates
 * Pre-conditions:  None
 * Post-conditions: Manhattan distance will be returned
 *
 * Description: 
 * This function takes 4 parameters, coordinates that will be used
 * to compute the manhattan distance between bus stations.
 * Manhattan distance = |x1 - x2| + |y1 - y2|
 */
int mandist(int x1, int y1, int x2, int y2){ 
  return (abs(x1 - x2) + abs(y1 - y2));
}

int shortestRoute(){
  
  return 0;
}

void *print_message_function(void *ptr){
  usleep(1000);
  char *message;
  message = (char *)ptr;
  printf("%s \n", message);
  return 0;
}

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

void getTotalPassengers();
int travel(int, int, int, int);
void *drive(void *ptr);
void printStops();

//exactly 8 stations

//Struct containing destination and number of passengers
struct passengers{
  int destination, numPass;
  pthread_mutex_t lock;
};

//Struct containing its coordinates, total number of passengers
//and an array containing groups of people going to each destination
struct station{
  pthread_mutex_t lock;
  int x, y, passengers;
  struct passengers destinations[8];
};

//current stop
struct station stops[8];

//Test out of coordinate problems *********************** 10,10
//going to spawn like 100 busses
int B;
int T;
int totalPassengers = 0;
int passArrived;
int timer = 0;
int delivered = 0;
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
int main(int argc, char *argv[]){
  FILE *file;
  char buf[80];
  char * pch;
  int coords[2];
  int buffer[3]; 
  
  if(argc != 2 || access(argv[1], F_OK) == -1){ 
    printf("Failure: Needs 1 existing filename as argument. Exiting...\n");
    exit(0);
  }

  //BEGIN filling data -----------------------------
  file = fopen(argv[1], "r");
  //Sets the runtime and number of busses
  if(fgets(buf, 80, file) != NULL){B = atoi(buf);};
  if(fgets(buf, 80, file) != NULL){T = atoi(buf);};
  
  pthread_t bus[B];


  for(int i = 0; i < 8; i++){
    //Not tokenized.  Needs to be tokenized
    if(fgets(buf, 80, file) == NULL){
      printf("Failure getting tokenizer\n");
      exit(0);
    };
     
    pch = strtok((char *)&buf, " ");
    int k = 0;
    while(pch != NULL){
      coords[k] = atoi(pch); 
      pch = strtok(NULL, " ");
      k++;
    }
    
    //Check for out of bounds 
    if(coords[0] > 9 || coords[0] < 0 || coords[1] < 0 || coords[1] < 0){
      printf("Coordinates of stop %d are out of bounds\n", i);
    }
    else{
      stops[i].x = coords[0];
      stops[i].y = coords[1];
    }
  }
  
  while(fgets(buf, 80, file) != NULL){
   
    //Fill structs here
    pch = strtok((char *)&buf, " ");
    int k = 0;
    while(pch != NULL){
      buffer[k] = atoi(pch);
      pch = strtok(NULL, " ");
      k++;
    }
    //Fill structs 
    stops[buffer[1]].destinations[buffer[2]].numPass += buffer[0];
    stops[buffer[1]].destinations[buffer[2]].destination = buffer[2];
  }
  fclose(file); 
  //init total passengers
  getTotalPassengers();
  //End filling data ----------------------------------

    //work mutex
  for(int i = 0; i < 8; i++){
    for(int j = 0; j < 8; j++){
      if(pthread_mutex_init(&stops[i].destinations[j].lock, NULL) == 0){
	//printf("init success\n");
      }
    }
  }
  
  printStops();
  int * threadN;
  for(int i = 0; i < B; i++){
    threadN = (int *)i;
    if(pthread_create(&bus[i], NULL, drive, (void *)threadN)){
      //printf("Failure in creating thread %d\n", i);
    }
    else{
      //printf("Created thread %d successfully \n", i);
    }
  }

  //  END OF PROGRAM HERE

  while(timer != T){
    printf("@ time %d\n", timer);
    printStops();
    sleep(1);
    timer++;
  }
  //Need to join threads so 
  for(int j = 0; j < B; j++){
    pthread_join(bus[j], NULL);
    pthread_cancel(bus[j]);
  }
  return 0;
}

/**
 * function - travel :
 * Parameter(s): int x1, y1, x2, y2 : x1, y1 and x2, y2 are coordinates
 * Pre-conditions:  None
 * Post-conditions: travel time will be returned
 *
 * Description: 
 * This function takes 4 parameters, coordinates that will be used
 * to compute the manhattan distance between bus stations.
 * Manhattan distance = |x1 - x2| + |y1 - y2|
 */
int travel(int x1, int y1, int x2, int y2){ 
  return (abs(x1 - x2) + abs(y1 - y2)) * 250;
}

void *drive(void *ptr){
  int station = 0;
  int destination = 0;
  //int test = (int)ptr;
  for(;;){

    //Determine highest amount of people in station
    //Pop array.  implement as stack?
    if(timer >= T){break;}
    //if(stops[test].destinations[test].numPass > 0){}
    int biggest = 0;
    //Check immediate station to take passengers
    for(int n = 0; n < 8; n++){
      if(stops[station].destinations[n].numPass > 0){
        destination = n;
        biggest = stops[station].destinations[n].numPass;
        if(pthread_mutex_trylock(&stops[station].destinations[n].lock) == 0){
          stops[station].destinations[n].numPass = 0;
          //printf("%d Thread - Successfully got stop\n", ptr);
          usleep(travel(stops[station].x, stops[station].y, stops[n].x , stops[n].y) * 1000);
          delivered += biggest;
          station = destination;
          if(pthread_mutex_unlock(&stops[station].destinations[n].lock) == 0){
            //printf("%d Thread - unlocked stop\n",ptr);
          }
        }
      }
      else{

      }
    }
    if(biggest < 1){
      //Nothing
    }
    /*
    float bestRoute = 0;
    float currBest = 0;
    //Check all other stations for smartest route
    //Travel to smartest route
    if(biggest < 1){
      for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
          bestRoute = ((stops[i].destinations[j].numPass * 1000) / travel(stops[station].x, stops[station].y, stops[i].x, stops[i].y));
          if(stops[i].destinations[j].numPass > biggest && bestRoute > currBest){
            destination = j;
            destStation = i;
            currBest = bestRoute;
            biggest = stops[i].destinations[j].numPass;
            printf("%f best route\n", bestRoute);
          } 
        }
      }
      usleep(travel(stops[station].x, stops[station].y, stops[destination].x, stops[destination].y));
      station = destation;
      printf("Best station is %d @ (%d, %d) with %d people. \n", destination, stops[destination].x, stops[destination].y, biggest);
    }
    else{

      printf("Biggest at station %d destination %d is %d\n", station, destination, biggest);
    }*/
    //

  }  
  return 0;
}
void getTotalPassengers(){
  for(int i = 0; i < 8; i++){
    int sum = 0;
    for(int j = 0; j < 8; j++){
      sum += stops[i].destinations[j].numPass;
    }
    totalPassengers += sum;
  }
}


void printStops(){
  int total = 0;
  for(int i = 0; i < 8; i++){
    int totalPass = 0;
    for(int j = 0; j < 8; j++){
      totalPass += stops[i].destinations[j].numPass;
    }
    total += totalPass;
    printf("%d @ (%d, %d): %d passengers\n", i, stops[i].x, stops[i].y, totalPass);
  }
  printf("%d of %d delivered.\n", delivered, totalPassengers);
}

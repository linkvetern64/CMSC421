/**
 * @Author Joshua Standiford (jstand1@umbc.edu)
 * This file will locate a valid data file passed in via
 * command line.  That file is then parsed and relevant information
 * is placed into proper sections.  Then a number of busses B will
 * be generated as pthreads.  These busses will be responsible for 
 * ferrying passengers to their locations.  If there are no passengers
 * at their current location.  The bus will do a weighted search
 * for the most optimal path to pick up the most people for shortest 
 * amount of time.  Once the time T is finished, the threads will join
 * and the program will end, printing the results.
 *
 * ---- QUESTIONS ----
 * 1. A potentional race condition for printing in main is printing
 *    the amount of people who have been delivered at the same time 
 *    the value is being updated.
 * 
 * 2. A way to prevent this would be to lock the variable everytime 
 *    its being read / written to.  This would ensure that the value 
 *    is accurate everytime it's accessed.
 *
 */


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


//Prototypes of functions
void getTotalPassengers();
int travel(int, int, int, int);
void drive();
void *waiting(void *ptr);
void printStops();


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
int done = 0;
int B;
int T;
int totalPassengers = 0;
int passArrived;
int timer = 0;
int delivered = 0;
int WAITING = 1;

/**
 * function - main():
 * Pre-conditions:  File must be passed in and valid
 * Post-conditions: None
 *
 * Description: 
 * This file will locate a valid data file passed in via
 * command line.  That file is then parsed and relevant information
 * is placed into proper sections.  Then a number of busses B will
 * be generated as pthreads.  These busses will be responsible for 
 * ferrying passengers to their locations.  If there are no passengers
 * at their current location.  The bus will do a weighted search
 * for the most optimal path to pick up the most people for shortest 
 * amount of time.  Once the time T is finished, the threads will join
 * and the program will end, printing the results.
 */
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
    if(coords[0] > 9 || coords[0] < 0 || coords[1] > 9 || coords[1] < 0){
      printf("Coordinates of stop %d are out of bounds\n", i);
      printf("Exiting program.. Please put in range (0,0) - (9,9)\n");
      exit(0);
    }
    else{
      stops[i].x = coords[0];
      stops[i].y = coords[1];
    }
  }
  
  //Files the structs with relevant data
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
 
  for(int i = 0; i < 8; i++){
    for(int j = 0; j < 8; j++){
      if(pthread_mutex_init(&stops[i].destinations[j].lock, NULL) == 0){
	     //printf("init success\n");
      }
    }
  }
  
  for(int i = 0; i < B; i++){
    if(pthread_create(&bus[i], NULL, waiting, (void *)1)){
      //printf("Failure in creating thread %d\n", i);
    }
  }
 
  WAITING = -1;
  while(timer < T){
    printf("After %d seconds:\n", timer);
    printStops();
    printf("%d of %d delivered.\n", delivered, totalPassengers);
    sleep(1);
    timer++;
  }

  //Need to join threads so 
  for(int j = 0; j < B; j++){
    pthread_join(bus[j], NULL);
  }
  printf("After time %d seconds:\n", timer);
  printStops();
  printf("%d of %d delivered.\n", delivered, totalPassengers);
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

/**
 * function - waiting :
 * Parameter(s): void *pt
 * Pre-conditions:  None
 * Post-conditions: None
 *
 * Description: 
 * All threads will initialize into waiting.
 * Once the program is ready to start, the loop will break
 * and drive() will be called.
 */
void *waiting(void *ptr){
  for(;;){
    if(WAITING == -1){
      break;
    }
    usleep(100000);
  }
  drive();
  return 0;
}

/**
 * function - drive :
 * Parameter(s): None
 * Pre-conditions:  waiting must be called in thread creation
 * Post-conditions: None
 * Constraints: All busses start at station 0

 * Description: 
 * This function is the driving function for each bus thread.
 * First the bus will check the immediate station for most passengers.
 * If there are no passengers at the current station, then the bus will
 * have to travel.  Once all of the stations are exhausted, done will flagged.
 * This makes it so the threads are still spinning but aren't computing anything.
 * sleep will be called, computed by the manhattan distance between stations.
 */
void drive(){ 
  int station = 0;
  int destStation = 0;
  int destination = 0;

  //run until threads are joined
  for(;;){

    if(timer >= T){break;}
    int biggest = 0;
    //Check immediate station to take passengers
    for(int n = 0; n < 8; n++){
      if(stops[station].destinations[n].numPass > 0){
        destination = n;
        biggest = stops[station].destinations[n].numPass;
        if(pthread_mutex_trylock(&stops[station].destinations[n].lock) == 0){
          stops[station].destinations[n].numPass = 0;
          usleep(travel(stops[station].x, stops[station].y, stops[n].x , stops[n].y) * 1000);
          delivered += biggest;
          station = destination;
          if(pthread_mutex_unlock(&stops[station].destinations[n].lock) == 0){
            //printf("%d Thread - unlocked stop\n",ptr);
          }
        }
      } 
    }

    //used for weighted estimation
    float bestRoute = 0;
    float currBest = 0;
    if(biggest < 1 && !done){
      for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
          bestRoute = ((stops[i].destinations[j].numPass * 1000) / (1 + travel(stops[station].x, stops[station].y, stops[i].x, stops[i].y)));
          if(stops[i].destinations[j].numPass > biggest && bestRoute > currBest){
            destination = j;
            destStation = i;
            currBest = bestRoute;
            biggest = stops[i].destinations[j].numPass;
          }
        }
      }
      if(biggest < 1){
        done = 1;
      }
      usleep(travel(stops[station].x, stops[station].y, stops[destination].x, stops[destination].y));
      station = destStation;
    }
  }  
}
/**
 * function - getTotalPassengers :
 * Parameter(s): None
 * Pre-conditions:  None
 * Post-conditions: None
 *
 * Description: 
 * Fills global variable with total amount of passengers waiting.
 * Used for debugging and printing to screen.
 */
void getTotalPassengers(){
  for(int i = 0; i < 8; i++){
    int sum = 0;
    for(int j = 0; j < 8; j++){
      sum += stops[i].destinations[j].numPass;
    }
    totalPassengers += sum;
  }
}

/**
 * function - printStops :
 * Parameter(s): None
 * Pre-conditions:  None
 * Post-conditions: None
 *
 * Description: 
 * This function will be called by the main thread every 1 second until
 * time is greater than or equal to file defined time T.  This will check 
 * and update the current passengers waiting.  Used as a debugging tool.
 */
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
}

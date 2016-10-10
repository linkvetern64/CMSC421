/**
 * @Author Joshua Standiford (jstand1@umbc.edu)
 * This file accepts a single argument.  The program then attempts
 * to find the matching interrupt number in the /proc/interrupts/ file.
 * If the number or driver doesn't exist nothing is printed.  
 * Otherwise the driver name is printed.
 */
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
int numHandles = 0;
char * positions[4];
int randPos;
int PID[4];
char * position;
char * name;
//Function prototypes
void my_handler(int);
int getHandles(void);
int getRand(void);
//End function prototypes

/**
 * function - main():
 * Pre-conditions:  name and position passed in via arguments
 * Post-conditions: player process is created that reads and handles signals
 *
 * Description: 
 * This function contains the logic for handling signal processing and STDIN_FILENO writing.
 * First a name and position are read by this program.  Then the parent process will write 4 PID's
 * to the child processes.  Then this program will step into a loop that will listen for write calls
 * or signals.  
 * SIGUSR1 - Will increment numHandles by 1.
 * SIGUSR2 - Will pass the ball randomly until it reaches child of position 1B.  
 *           everytime the child handles the ball numHandles incrememnts by 1.
 * Read()  - Will read from STDIN_FILENO and display the player statistics
 */
int main(int argc, char *argv[]){
  sleep(1); //So players load in after menu
  name = argv[1];
  position = argv[2];
  int fd[2];
  char * buf[50];
  char * buff[8];
  char * players;
  positions[0] = "1B";
  positions[1] = "2B";
  positions[2] = "SS";
  positions[3] = "3B";
 
  dup2(fd[0], STDIN_FILENO);
  //This read loops reads the PID's from the parent process
  char * pch;
  int k = 0;
  while(1){
    sleep(1);
    if(read(STDIN_FILENO, &buf, 50) > 0){
players = (char *)&buf;
      pch = strtok(players, " ");
      while(pch != NULL){
 	PID[k] = atoi(pch);
	pch = strtok(NULL, " ");
	k++;
      }
      //Tokenize string and put in PID's
      break;
    }
  }
  printf("#%d:%s (%s)\n",(int)getpid(), name, position);

  sigset_t mask;
  sigemptyset(&mask);
  struct sigaction sa = {
    .sa_handler = my_handler,
    .sa_mask = mask,
    .sa_flags = 0
  };
  sigaction(SIGUSR1, &sa, NULL);
  sigaction(SIGUSR2, &sa, NULL);
  
  while(1){
    sleep(1);
    //Doesn't matter whats sent, expect read() bigger than 0
    if(read(STDIN_FILENO, &buff, 8) > 0){
      printf("Player %d caught ball %d times.\n",getpid(), getHandles());
    }
  }
  return 0;
}

/**
 * function - my_handler :
 * Parameter: int sig, will be the integer value of incoming signal
 * Pre-conditions:  None
 * Post-conditions: Signal handler will call appropriate action internally
 *
 * Description: 
 * This function takes in the integer value of a signal sent by the parent.
 * This number will be handled and will be either SIGUSR1 or SIGUSR2
 */
void my_handler(int sig){
  if(sig == SIGUSR1){   
    numHandles++;
    printf("%s caught the ball.\n", name);
  }
  else if(sig == SIGUSR2){
    numHandles++;
    printf("%s caught the ball\n", name);
    if(!strcmp(position, "1B")){
      //printf("arrived at first base\n");
    }
    else{
      randPos = getRand();
      do{
	randPos = getRand();
      }
      while(!strcmp(positions[randPos], position));
      printf("Throwing the ball to #%d %s\n", PID[randPos], positions[randPos]);
      kill(PID[randPos], SIGUSR2);
    }
  }
}

/**
 * function - getHandles:
 * Pre-conditions: None
 * Post-conditions: Returns numbHandles value
 *
 * Description: 
 * Returns int value of var numHandles
 */
int getHandles(){
  return numHandles;
}

/**
 * function - getRand():
 * Pre-conditions: None
 * Post-conditions: A random number of range 0 - 3 will be returned
 *
 * Description: 
 * This function reads from dev/urandom a random number and modulus 4 of the number
 * will return 0 - 3
 */
int getRand(){
  int file;
  unsigned int rand;
  int errNum;
  file = open("/dev/urandom", O_RDONLY);	  
  errNum = read(file, &rand, sizeof(rand));
  if(errNum < 0 ){
    perror("Failure");
  }
  
  close(file);
  return (rand % 4);
}

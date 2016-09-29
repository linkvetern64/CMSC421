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
#include <errno.h>
#include <stdint.h>

/**
 * function - main():
 * Pre-conditions:  4 player names
 * Post-conditions: 4 child processes will be created
 *
 * Description: 
 * This function contains the logic for first reading the command line 
 * for 4 child arguments.  Any less or more and the program exits.
 *
 * 4 child processes are then created and their PID's are written to them.
 * a menu opens up once the children processes are successfully created.
 * this menu allows interaction with the child processes in the form of baseball.
 * Menu options:
 * 1, SIGUSR1 is sent to childs PID and signal handler increments a global var by 1
 * 2, SIGUSR2 is sent to a childs PID.  If 1B then do nothing, otherwise keep passing
 *    the ball until it reaches 1st base.  Increment each number of handles each time.
 * 3, Writes a byte to each child process which alerts them to print their statistics
 * 4, The Program is killed and waitpid() is called so the child processes shut down
 */
int main(int argc, char *argv[]){
  //Exits program if too many or too little arguments
  if(argc != 5){
    printf("Error too many or too little arguments.  Need exactly 4\n");
    return 0;
  }

  char * players[4];
  char * positions[4];
  positions[0] = "1B";
  positions[1] = "2B";
  positions[2] = "SS";
  positions[3] = "3B";
  int input, playerTo;
  int pid;
  int selection = 9;
  int PID[4];
  int fd[8];
  char * buf[255];
  pipe(fd);
  pipe(fd + 2);
  pipe(fd + 4);
  pipe(fd + 6);
  
  //Creates forked processes
  for(int i = 0; i <= 3; i++){
    switch(pid = fork()){
    //Child process
    case 0:
      
      //Close Read
      if(dup2(fd[i * 2], STDIN_FILENO) == -1){
	printf("dup2 error\n");
      }
      
      close(fd[i * 2]);
      execlp("./player", "player", argv[i + 1],positions[i], NULL);
      return 0;
   
    case -1:
      //Error - do nothing
      break;

    //Parent process  
    default:
      PID[i] = pid;
      break;
    }
  }
  
  //Send PID's to child processes
  char str[10];
  for(int i = 0; i < 4; i++){
    for(int j = 0; j < 4; j++){
      sprintf(str, "%d", PID[j]);
      dprintf(fd[1 + i * 2], "%s ", str);
    }
  }
   
  if(getpid() > 0){
    while(selection > -1){
	printf("Main Menu:\n");
	printf("1. Throw ball directory to a player\n");
	printf("2. Field ball\n");
	printf("3. Show player statistics\n");
	printf("4. End game\n");
	selection = scanf("%d", &input);

	switch(input){
	  //Throwing ball
	case 1:
	  printf("Throwing ball to who?\n");
	  for(int i = 1; i < 5; i++){
	    printf("%d %s\n", i, argv[i]);
	  }
	  selection = scanf("%d", &playerTo);
	  switch(playerTo){
	  case 1:
	    kill(PID[0], SIGUSR1);
	    break;

	  case 2:
	    kill(PID[1], SIGUSR1);
	    break;

	  case 3:
	    kill(PID[2], SIGUSR1);
	    break;

	  case 4:
	    kill(PID[3], SIGUSR1);
	    break;
     
	  default:
	    printf("Illegal entry, returning to main menu\n");
	    break;
	  }
	  break;

	//Fielding ball
	case 2:
	  printf("Fielding ball to who?\n");
	  for(int i = 1; i < 5; i++){
	    printf("%d %s\n", i, argv[i]);
	  }
	  selection = scanf("%d", &playerTo);
	  switch(playerTo){
	  case 1:
	    kill(PID[0], SIGUSR2);
	    break;
	    
	  case 2:
	    kill(PID[1], SIGUSR2);
	    break;
	    
	  case 3:
	    kill(PID[2], SIGUSR2);	    
	    break;
	    
	  case 4:
	    kill(PID[3], SIGUSR2);
	    break;
     
	  default:
	    printf("Illegal entry, returning to main menu\n");
	    break;
	  }
	  break;
	  
	//prints player stats
	case 3:
	  for(int i = 0; i < 4; i++){
	    if(write(fd[1 + i * 2], "t", 8) < 1){
	      printf("failed writing to file descriptor");
	    }
	    usleep(200);
	  }
	  break;
	  
	case 4:
	  printf("Ending game...\n");
	  for(int i = 0; i < 4; i++){
	    kill(PID[i], SIGQUIT);
	  }
	  waitpid();
	  return 0;
	  break;
       
	default:
	  printf("Value of input is %d\n", input);
	  break;
	}	
      }
  }
  return 0;
}

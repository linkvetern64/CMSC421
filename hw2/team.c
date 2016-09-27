#define _POSIX_SOURCE
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
void childProcess(void); //Prototype of child process
void parentProcess(void); //Prototype of parent process

int main(int argc, char *argv[]){
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
  int input;
  int pid;
  int selection = 9;
  int PID[4];
  int fd[8];

  pipe(fd);
  pipe(fd + 2);
  pipe(fd + 4);
  pipe(fd + 6);
  
  //Creates forked processes
  for(int i = 0; i <= 3; i++){
    switch(pid = fork()){
    case 0:
      //Close Read
      dup2(fd[i * 2], 0);
      //printf("Testing %d, %d\n", (i * 2), (1 + i * 2)); 
      close(fd[i * 2]);
      close(fd[1 + i * 2]);
      
      wait();
      execlp("./player", "player", argv[i + 1],positions[i], NULL);
      return 0;
   
    case -1:
      //Error - do nothing
      break;

    //Parent process  
    default:
      close(fd[1 + i * 2]);
      PID[i] = pid;
      break;
    }
  }

  if(getpid() > 0){
    while(selection > -1){
	printf("Main Menu:\n");
	printf("1. Throw ball directory to a player\n");
	printf("2. Field ball\n");
	printf("3. Show player statistics\n");
	printf("4. End game\n");
	printf("5. Print child PIDS\n");
	printf("6. Kill PIDS\n");
	selection = scanf("%d", &input);

	switch(input){
	case 1:
	  printf("Throwing ball...\n");
	  break;
	case 2:
	  printf("Fielding ball...\n");
	  break;
	case 3:
	  printf("Showing player stats...\n");
	  break;
	case 4:
	  printf("Ending game...\n");
	  waitpid();
	  return 0;
	  break;
	//Test case
	case 5:
	  for(int l = 0; l < 4; l++){
	    printf("Child ID = %d \n", PID[l]);
	  }
	  break;
	//Test case
	case 6:
	  //Test kills the PID's -  DELETE THIS
	  for(int j = 0; j < 4; j++){
	    printf("PID for child %d = %d\n", j, PID[j]);
	    if(kill(PID[j], 0) == 0){
	      kill(PID[j], SIGKILL);
	      printf("Error:(%s)\n", strerror(errno));
	      //printf("Succeeded in killing PID\n");
	    }
	    else{
	      printf("Failed to kill PID\n");
	    }
	  }
	  break;
	case 7:
	  
	  write(fd[0], "Test", strlen("Test") + 1);
	  printf("Error:(%s)\n", strerror(errno));
	  break;
	default:
	  
	  printf("Value of input is %d\n", input);
	  break;
	}	
      }
  }
 
  
  return 0;
}

void childProcess(void){
  printf("Child Process\n");
  //close(fd[1]);
}

void parentProcess(void){
  printf("Parent Process\n");
  //close(fd[0]);
}

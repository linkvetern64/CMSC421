
//#define _POSIX_SOURCE
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
void childProcess(void); //Prototype of child process
void parentProcess(void); //Prototype of parent process

int main(int argc, char *argv[]){
  if(argc != 5){
    return 0;
  }

  int input;
  int selection = 9;

  int pid;
  int PID[4];
  int fd[2];
  int pipeRet = pipe(fd);
  if(pipeRet == 0){
    printf("Successful pipe\n");
  }
  //Creates forked processes
  
  for(int i = 0; i <= 3; i++){
    switch(pid = fork()){
    case 0:
      //Close READ
      close(fd[0]);
      //Close WRITE
      //close(fd[1]);
      //child case
      return 0;
   
    case -1:
      //Error - do nothing
      break;

    default:
      PID[i] = pid;
      close(fd[1]);
      //parentProcess();
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
	  return 0;
	  break;
	default:
	  printf("Value of input is %d\n", input);
	  break;
	}	
      }
  }
  
  //Test kills the PID's -  DELETE THIS
  for(int j = 0; j < 4; j++){
    printf("PID for child %d = %d\n", j, PID[j]);
    if(kill(PID[j]) == 0){
	printf("Succeeded in killing PID\n");
      }
      else{
	printf("Failed to kill PID\n");
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

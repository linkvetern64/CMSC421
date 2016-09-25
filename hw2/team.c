
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
  //int input;
  //int selection = 0;

  int pid;
  int PID[4];
  //int fd[2];
  //pipe(fd);
  //Creates forked processes
  
  for(int i = 0; i <= 3; i++){
    switch(pid = fork()){
    case 0:
      printf("Child process\n");
      //childProcess();
      //child process
      return 0;
   
    case -1:
      //Error - do nothing
      printf("Error in forking\n");
      break;
    default:
      PID[i] = pid;
      //parent process
      //parentProcess();
      break;
    }
  }
    
  for(int j = 0; j < 4; j++){
    printf("PID for child %d = %d\n", j, PID[j]);
    if(kill(PID[j]) == 0){
	printf("Succeeded in killing PID\n");
      }
      else{
	printf("Failed to kill PID\n");
      }
  }
  
  /*
  //Menu
  while(true){
  printf("Main Menu:\n");
  printf("1. Throw ball directory to a player\n");
  printf("2. Field ball\n");
  printf("3. Show player statistics\n");
  printf("4. End game\n");
  selection = scanf("%d", input);
  }
  //End menu
  
  FILE *file = fopen("/dev/urandom", "r");
  int rand = fscanf(file, "%[^n]");
  printf("Num - %d\n", rand);
  for(int i = 1; i < argc; i++){
    printf("Name %d - %s\n",i, argv[i]);
  }
  fclose(file);
  */
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

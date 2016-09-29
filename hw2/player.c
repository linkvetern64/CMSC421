#define _POSIX_SOURCE
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int numHandles = 0;

void my_handler(int);

int getHandles(void);

int main(int argc, char *argv[]){
  usleep(100); //So players load in after menu
  char * name = argv[1];
  char * position = argv[2];
  int fd[2];
  char * buf[50];
  char * buff[20];
  int PID[4];
  char * players;
  char * positions[4];
  positions[0] = "1B";
  positions[1] = "2B";
  positions[2] = "SS";
  positions[3] = "3B";
  
  
  //close(fd[0]);
  //close(fd[1]);
  dup2(fd, STDIN_FILENO);
  
  char * pch;
  int k = 0;
  while(1){
    sleep(1);
    if(read(STDIN_FILENO, &buf, 50) > 0){
      players = &buf;
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
  
  sigset_t mask;
  sigemptyset(&mask);
  struct sigaction sa = {
    .sa_handler = my_handler,
    .sa_mask = mask,
    .sa_flags = 0
  };
  sigaction(SIGUSR1, &sa, NULL);
  sigaction(SIGUSR2, &sa, NULL);
  
  //MAKE TWO WHILE LOOPS MAYBE? BREAK AFTER PIDS STORED
  printf("#%ld:%s (%s)\n",getpid(), name, position);
  while(1){
    sleep(1);
    //Doesn't matter whats send, expect read() bigger than 0
    if(read(STDIN_FILENO, &buff, 20) > 0){
      printf("Player %d caught ball %d times.\n",getpid(), getHandles());
    }
  }
  return 0;
}

void my_handler(int sig){
  if(sig == SIGUSR1){   
    numHandles++;
    printf("%d - Caught signal SIGUSR1 %d times.\n", getpid(), numHandles);
  }
  else if(sig == SIGUSR2){
    printf("Caught SIGUSR2\n");
  }
}

int getHandles(){
  return numHandles;
}

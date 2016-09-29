#define _POSIX_SOURCE
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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

int main(int argc, char *argv[]){
  usleep(100); //So players load in after menu
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
  
  close(fd[1]);
  dup2(fd, STDIN_FILENO);

  //This read loops reads the PID's from the parent process
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
  printf("#%ld:%s (%s)\n",getpid(), name, position);

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

int getHandles(){
  return numHandles;
}

int getRand(){
  int file;
  unsigned int rand;
  uint16_t randomNum;
  file = open("/dev/urandom", O_RDONLY);	  
  randomNum = read(file, &rand, sizeof(rand));
  close(file);
  return (rand % 4);
}

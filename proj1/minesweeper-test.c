/**
 * @Author Joshua Standiford (jstand1@umbc.edu)
 * @E-mail jstand1@umbc.edu
 * This file is used to test minesweeper.c.
 * This file will test edge cases, boundary issues,
 * concurrency issues and if the game breaks under
 * extreme conditions.
 */

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/user.h>
#include <string.h>
#include <stdio.h>
/* description over function */
void print_table(void);

/* description over function */
void rewind_fd(void);

/* function to test threads */
void *waiting(void *ptr);


char * board;
char status[80];
#define THREAD_COUNT 1000

int fd_read_ms, fd_read_ms_ctl, fd_write;

/*
 * Used for testing with fixed_mines
 */
int main(void) {
	pthread_t players[THREAD_COUNT];
	unsigned test_passed = 0;
	unsigned test_failed = 0;
	//init section
	fd_read_ms = open("/dev/ms", O_RDONLY);
	fd_read_ms_ctl = open("/dev/ms_ctl", O_RDONLY);
	fd_write = open("/dev/ms_ctl", O_WRONLY);

	if(fd_read_ms < 0 || fd_write < 0 || fd_read_ms_ctl < 0){
		printf("/dev/ms is not loaded.\n");
	}

	//mmap section
	board = mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, fd_read_ms, 0);
	if(board == MAP_FAILED){printf("test failed on %d\n", __LINE__);}
 	 
	printf("Test 1: Status Correctly Displayed\n");
	/* Test for game reset status */
	if(write(fd_write, "s\n" , 3)){/*Expected write to work*/}
	if(read(fd_read_ms_ctl, status, sizeof(status)) <= 0){
		printf("Failure to read on %d\n", __LINE__);
	}
 	if(!strcmp(status, "Game reset")){test_passed++;}
 	else{test_failed++;}
 	rewind_fd();

	/* Test for correct markings status */
	if(write(fd_write, "r10\n" , 3)){/*Expected write to work*/}
	if(read(fd_read_ms_ctl, status, sizeof(status)) <= 0){
		printf("Failure to read on %d\n", __LINE__);
	}
 	if(!strcmp(status, "0 Marked of 10")){test_passed++;}
 	else{test_failed++;}
 	rewind_fd();

	/* Test for marked status */
	if(write(fd_write, "m00\n" , 3)){/*Expected write to work*/}
	
	if(read(fd_read_ms_ctl, status, sizeof(status)) <= 0){
		printf("Failure to read on %d\n", __LINE__);
	}
 	if(!strcmp(status, "1 Marked of 10")){test_passed++;}
 	else{test_failed++;}
 	rewind_fd();

 	/* Test for You win! */
 	if(write(fd_write, "m11\n" , 3)){/*Expected write to work*/}
 	if(write(fd_write, "m22\n" , 3)){/*Expected write to work*/}
 	if(write(fd_write, "m33\n" , 3)){/*Expected write to work*/}
 	if(write(fd_write, "m44\n" , 3)){/*Expected write to work*/}
 	if(write(fd_write, "m55\n" , 3)){/*Expected write to work*/}
 	if(write(fd_write, "m66\n" , 3)){/*Expected write to work*/}
 	if(write(fd_write, "m77\n" , 3)){/*Expected write to work*/}
 	if(write(fd_write, "m88\n" , 3)){/*Expected write to work*/}
 	if(write(fd_write, "m99\n" , 3)){/*Expected write to work*/}
	if(read(fd_read_ms_ctl, status, sizeof(status)) <= 0){
		printf("Failure to read on %d\n", __LINE__);
	}
 	if(!strcmp(status, "Game won!")){test_passed++;}
 	else{test_failed++;}
 	rewind_fd();

	/* Test for You lose! status */
	if(write(fd_write, "s\n" , 3)){/*Expected write to work*/}
 	if(write(fd_write, "q\n" , 3)){/*Expected write to work*/}
	if(read(fd_read_ms_ctl, status, sizeof(status)) <= 0){
		printf("Failure to read on %d\n", __LINE__);
	}
 	if(!strcmp(status, "You lose!")){test_passed++;}
 	else{test_failed++;}
 	rewind_fd();




 	/* Test concurrency locks */
 	printf("Test 2: Concurrency Lock Testing\n");

 	for(int i = 0; i < THREAD_COUNT; i++){
    if(pthread_create(&players[i], NULL, waiting, (void *)1)){
      //printf("Failure in creating thread %d\n", i);
    	}
  	}
	  	//Need to join threads so 
	  for(int j = 0; j < THREAD_COUNT; j++){
	    pthread_join(players[j], NULL);
	  }

 	//Test results
 	printf("Tests %d of %d passed.\n", test_passed, (test_passed + test_failed));
	return 0;
}

/* @Name: print_table
 * @Return: void
 * @Desc: this function is used for debugging
 * and will print the game_board and status.
 */
void print_table(){
	printf("  0123456789\n");
	for(int i = 0; i < 10; i++){
		printf("%d ",i);
		for(int j = 0; j < 10; j++){
			printf("%c",board[10*i + j]);
		}
		printf("\n");
	}
	printf("Status : %s\n", status);
}

/* @Name: rewind
 * @Return: void
 * @Desc: This function closes the /dev/ms_ctl 
 * file descriptor and re-opens /dev/ms_ctl.
 */
void rewind_fd(){
 	close(fd_read_ms_ctl);
	fd_read_ms_ctl = open("/dev/ms_ctl", O_RDONLY);
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
	if(write(fd_write, "m10\n" , 3)){/*Expected write to work*/}
	printf("1");
	return 0;
}

/*
    ------------Testing Template-------------

	fd_read_ms = open("/dev/ms", O_RDONLY);
	fd_read_ms_ctl = open("/dev/ms_ctl", O_RDONLY);
	fd_write = open("/dev/ms_ctl", O_WRONLY);

	if(fd_read_ms < 0 || fd_write < 0 || fd_read_ms_ctl < 0){
		printf("/dev/ms is not loaded.\n");
	}

	//mmap section
	board = mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, fd_read_ms, 0);
	if(board == MAP_FAILED){printf("test failed on %d\n", __LINE__);}

	//Test section
	if(write(fd_write, "r00" , 3)){}
	if(read(fd_read_ms_ctl, status, sizeof(status)) <= 0){
		printf("Failure to read on %d\n", __LINE__);
	}
 	print_table();
 	if(!strcmp(status, "Revealing pieces")){printf("Correct!\n");}

 	rewind();
	 
	if(write(fd_write, "s" , 3)){}
	if(read(fd_read_ms_ctl, status, sizeof(status)) <= 0){
		printf("Failure to read on %d\n", __LINE__);
	}
	print_table();	
*/
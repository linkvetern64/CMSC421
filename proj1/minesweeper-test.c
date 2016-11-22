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

#define THREAD_COUNT 1000

/* description over function */
void print_table(void);

/* description over function */
void rewind_fd(void);

/* function to test threads */
void *waiting(void *ptr);

char * board;
char status[80];


int fd_read_ms, fd_read_ms_ctl, fd_write;
unsigned test_passed, test_failed;

/*
 * Used for testing with fixed_mines
 */
int main(void) {
	pthread_t players[THREAD_COUNT];
	test_passed = 0;
	test_failed = 0;
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


 	/*------------------------------------------------*/

	printf("Test 1: Status Correctly Displayed:\n");
	/* Test for game reset status */
	printf("\nGame Reset Status Displayed:\n");
	if(write(fd_write, "s\n" , 3)){/*Expected write to work*/}
	if(read(fd_read_ms_ctl, status, sizeof(status)) <= 0){
		printf("Failure to read on %d\n", __LINE__);
	}
 	if(!strcmp(status, "Game reset")){test_passed++;}
 	else{test_failed++;}
 	print_table();
 	rewind_fd();

 	/*------------------------------------------------*/

 	printf("\n0 Marked Status Displayed:\n\n");
	/* Test for correct markings status */
	if(write(fd_write, "r10\n" , 3)){/*Expected write to work*/}
	if(read(fd_read_ms_ctl, status, sizeof(status)) <= 0){
		printf("Failure to read on %d\n", __LINE__);
	}
 	if(!strcmp(status, "0 Marked of 10")){test_passed++;}
 	else{test_failed++;}
 	print_table();
 	rewind_fd();

 	/*------------------------------------------------*/
 
 	printf("\n1 Marked Status Displayed:\n");
	/* Test for marked status */
	if(write(fd_write, "m00\n" , 3)){/*Expected write to work*/}	
	if(read(fd_read_ms_ctl, status, sizeof(status)) <= 0){
		printf("Failure to read on %d\n", __LINE__);
	}
 	if(!strcmp(status, "1 Marked of 10")){test_passed++;}
 	else{test_failed++;}
 	print_table();
 	rewind_fd();

 	/*------------------------------------------------*/
 
 	printf("\nGame Won Status Displayed:\n");
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
 	print_table();
 	rewind_fd();

 	/*------------------------------------------------*/
 
	printf("\nGame Over Status Displayed:\n");
 	/* Reveal mine space and game over */
	if(write(fd_write, "s\n" , 3)){/*Expected write to work*/}
 	if(write(fd_write, "r00\n" , 3)){/*Expected write to work*/}
	if(read(fd_read_ms_ctl, status, sizeof(status)) <= 0){
		printf("Failure to read on %d\n", __LINE__);
	}
 	if(!strcmp(status, "You lose!")){test_passed++;}
 	else{test_failed++;}
 	print_table();
 	rewind_fd();

 	/*------------------------------------------------*/
 
 	printf("\nGame Over Status Displayed:\n");
	/* Test for You lose! status */
	if(write(fd_write, "s\n" , 3)){/*Expected write to work*/}
 	if(write(fd_write, "q\n" , 3)){/*Expected write to work*/}
	if(read(fd_read_ms_ctl, status, sizeof(status)) <= 0){
		printf("Failure to read on %d\n", __LINE__);
	}
 	if(!strcmp(status, "You lose!")){test_passed++;}
 	else{test_failed++;}
 	print_table();
 	rewind_fd();

 	/*------------------------------------------------*/
 



 	/* Test concurrency locks */
 	printf("Test 2: Concurrency Lock Testing\n");
 	printf("\n1000 Threads Writing to /dev/ms:\n");
	if(write(fd_write, "s\n" , 3)){/*Expected write to work*/}

 	for(int i = 0; i < THREAD_COUNT; i++){
   		if(pthread_create(&players[i], NULL, waiting, (void *)1)){
      //printf("Failure in creating thread %d\n", i);
    	}
  	}
	  	//Need to join threads so 
	for(int j = 0; j < THREAD_COUNT; j++){
	    pthread_join(players[j], NULL);
	}
	print_table();
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
 * All threads will initialize and try to write 
 * to the game board at the same time.  
 */
void *waiting(void *ptr){
	if(write(fd_write, "m10\n" , 3)){/*Expected write to work*/}
	return 0;
}
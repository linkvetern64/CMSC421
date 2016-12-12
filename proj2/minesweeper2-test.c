/**
 * @Author Joshua Standiford (jstand1@umbc.edu)
 * @E-mail jstand1@umbc.edu
 * This file is used to test minesweeper.c.
 * This file will test edge cases, boundary issues,
 * concurrency issues and if the game breaks under
 * extreme conditions.
 */
#define _BSD_SOURCE //For usleep
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
#include "cs421net.h"

#define THREAD_COUNT 1000

/* description over function */
void print_table(void);

/* description over function */
void rewind_fd(void);

/* function to test threads */
void *waiting(void *ptr);

/* description over function */
void print_stats(void);

char * board;
char * ms_stats_list;
char status[80];


int fd_read_ms, fd_read_ms_ctl, fd_write, fd_read_ms_stats;
unsigned test_passed, test_failed;

/*
 * Used for testing with fixed_mines
 */
int main(void) {
	cs421net_init();
	
	//pthread_t players[THREAD_COUNT];
	//char stress[PAGE_SIZE];
	/*for(int i = 0; i < PAGE_SIZE; i++){
		stress[i] = 'G';
	}*/

	test_passed = 0;
	test_failed = 0;

	//init section
	fd_read_ms = open("/dev/ms", O_RDONLY);
	fd_read_ms_ctl = open("/dev/ms_ctl", O_RDONLY);
	fd_write = open("/dev/ms_ctl", O_WRONLY);
	fd_read_ms_stats = open("/dev/ms_stats", O_RDONLY);

	if(fd_read_ms < 0 || fd_write < 0 || fd_read_ms_ctl < 0){
		printf("/dev/ms is not loaded.\n");
	}

	//mmap section
	board = mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, fd_read_ms, 0);
	ms_stats_list = mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, fd_read_ms_stats, 0);

	//failure to initialize section
	if(board == MAP_FAILED){printf("test failed on %d\n", __LINE__);}
	if(ms_stats_list == MAP_FAILED){printf("test failed on %d\n", __LINE__);}


 	/*------------------------------------------------*/
	//This test will test basic inputs
	printf("Test 1: Testing Network Input:\n");
	printf("\nSending Basic Inputs:\n");
	if(cs421net_send("q", 1)){/*usleep(50);*/}
 	if(board[0] == '*'){test_passed++;}
 	else{test_failed++;}
	print_table();
	if(cs421net_send("s", 1)){/*usleep(50);*/}
 	if(board[0] == '*'){test_passed++;}
 	else{test_failed++;}
	print_table();
	if(cs421net_send("m00", 3)){/*usleep(50);*/}
 	if(board[0] == '*'){test_passed++;}
 	else{test_failed++;}
	print_table();
	if(cs421net_send("r99", 3)){/*usleep(50);*/}
 	if(board[0] == '*'){test_passed++;}
 	else{test_failed++;}
	print_table();
	 /*---------------------------------------------------------*/
	 
 	

 	


 	
 	//Test results
 	printf("Tests %d of %d passed.\n", test_passed, (test_passed + test_failed));
	return 0;
}

void print_stats(){
	printf("Printing Board:\n");
	char sym;
	for(int i = 0; i < strlen(ms_stats_list); i++){
		sym = ms_stats_list[i];
		//print only numbers, spaces and newlines
		if((sym - '0' > -1 && sym - '0' <= 9) || sym == ' ' || sym == '\n'){
			printf("%c", ms_stats_list[i]);		
		}
	}
	printf("\n");
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
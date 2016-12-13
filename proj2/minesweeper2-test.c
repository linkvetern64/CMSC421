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

#define THREAD_COUNT 10
//adjust number if you keep getting race conditions
#define SLEEP_DUR 10000

/* description over subroutine */
void print_table(void);

/* description over subroutine */
void rewind_fd(void);

/* function to test threads */
void *waiting(void *ptr);

/* description over subroutine */
void print_stats(void);

/* description over subroutine */
void board_reset(void);

char * board;
char * ms_stats_list;
char status[80];
int fd_read_ms, fd_read_ms_ctl, fd_write, fd_read_ms_stats, UID, TEST_NO;
unsigned test_passed, test_failed;

/*
 * Used for testing with fixed_mines
 */
int main(void) {
	cs421net_init();
	
	//Expected 0 for root, anything else is non-root
	UID = getuid();
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
 	
 	//Reset table for clean tests
	board_reset();

 	/*------------------------------------------------*/
	//This test will test basic inputs
	printf("Test %d: Testing Basic Network Input:\n", ++TEST_NO);

	/*---------------------------------------------------------*/

	/* Testing quitting command via network */
	printf("\nSending 'q':\n");
	if(cs421net_send("q", 1)){usleep(SLEEP_DUR);}
 	if(board[0] == '*'){test_passed++;}
 	else{test_failed++;}
 	print_table();

	/*------------------------------------------------*/

	/* Testing clear board command via network */
	printf("\nSending 's':\n");
	if(cs421net_send("s", 1)){usleep(SLEEP_DUR);}
 	print_table();
 	if(!strcmp(status, "Game reset")){test_passed++;}
 	else{test_failed++;}

	/*------------------------------------------------*/

	/* Testing marking (0,0) command via network */
	printf("\nSending 'm00':\n");
	if(cs421net_send("m00", 3)){usleep(SLEEP_DUR);}
 	if(board[0] == '?'){test_passed++;}
 	else{test_failed++;}
	print_table();

	/*------------------------------------------------*/

	/* Testing reveal (9,8) command via network */
	printf("\nSending 'r98':\n");
	if(cs421net_send("r98", 3)){usleep(SLEEP_DUR);}
 	if(board[89] == '2'){test_passed++;}
 	else{test_failed++;}
	print_table();

	/*---------------------------------------------------------*/
	
	printf("Test %d: Testing Network Input Boundaries:\n", ++TEST_NO);

	/*---------------------------------------------------------*/

	/* Testing marking corners of game board */
	printf("\nMarking 4 Corners:\n");
	board_reset();
	if(cs421net_send("m00", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m09", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m90", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m99", 3)){usleep(SLEEP_DUR);}
	print_table();

	/* Checking to see if the commands successfully planted */
 	if(board[0] == '?' && board[9] == '?' && board[90] == '?' && board[99] == '?'){test_passed++;}
 	else{test_failed++;printf("Failed here\n");}
	
	printf("\nChecking Correct Status:\n");
	if(!strcmp(status, "4 Marked of 10")){test_passed++;}
	else{test_failed++;}
 	/*---------------------------------------------------------*/
 	
 	/* 
 	STRESS TESTING WAS REMOVED DUE TO BUG IN PROFESSOR TANGS cs421net_send() function
	As of 12/13/2016.  (BUG: net writes to board faster than module can handle commands)
 	 *//*
	printf("Test %d: Testing 100 Threads Writing on Network:\n", ++TEST_NO);
	board_reset();
 	for(int i = 0; i < THREAD_COUNT; i++){
   		if(pthread_create(&players[i], NULL, waiting, (void *)1)){
      //printf("Failure in creating thread %d\n", i);
    	}
  	}
	  	//Need to join threads so 
	for(int j = 0; j < THREAD_COUNT; j++){
	    pthread_join(players[j], NULL);
	}
	print_table();*/

 	/*---------------------------------------------------------*/
	printf("\nTest %d: Testing Network Cheat Permissions:\n", ++TEST_NO);
	board_reset();

	/* This condition will always hit unless user is root in local space */
	if(UID){
		printf("\n Testing Network Permissions as User:\n");
		if(cs421net_send("a69", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("r69", 3)){usleep(SLEEP_DUR);}
		if(board[96] == '-'){test_passed++;}
		else{test_failed++;}
		print_table();
		board_reset();

		if(cs421net_send("d00", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("r00", 3)){usleep(SLEEP_DUR);}
		if(board[0] == '*'){test_passed++;}
		else{test_failed++;}
		print_table();
	}	
	else{
		printf("\n Testing Network Permissions as Root:\n");
		if(cs421net_send("a69", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("r69", 3)){usleep(SLEEP_DUR);}
		if(board[96] == '-'){test_passed++;}
		else{test_failed++;}
		print_table();
		board_reset();

		if(cs421net_send("d00", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("r00", 3)){usleep(SLEEP_DUR);}
		if(board[0] == '*'){test_passed++;}
		else{test_failed++;}
		print_table();
	}
	
 	/*---------------------------------------------------------*/

	printf("\nTest %d: Testing Local Cheat Permissions:\n", ++TEST_NO);
	board_reset();

	/* This condition will always hit unless user is root in local space */
	if(UID){
		printf("\n Testing Local Permissions as User:\n");
		if(write(fd_write, "a69\n" , 4)){/*Expected write to work*/}
		if(write(fd_write, "r69\n" , 4)){/*Expected write to work*/}
		if(board[96] == '-'){test_passed++;}
		else{test_failed++;}
		print_table();
		board_reset();

		if(write(fd_write, "d00\n" , 4)){/*Expected write to work*/}
		if(write(fd_write, "r00\n" , 4)){/*Expected write to work*/}
		if(board[0] == '*'){test_passed++;}
		else{test_failed++;}
		print_table();
	}	
	else{
		printf("\n Testing Local Permissions as Root:\n");
		if(write(fd_write, "a69\n" , 4)){/*Expected write to work*/}
		if(write(fd_write, "r69\n" , 4)){/*Expected write to work*/}
		if(board[96] == '*'){test_passed++;}
		else{test_failed++;}
		print_table();
		board_reset();

		if(write(fd_write, "d00\n" , 4)){/*Expected write to work*/}
		if(write(fd_write, "r00\n" , 4)){/*Expected write to work*/}
		if(board[0] == '1'){test_passed++;}
		else{test_failed++;}
		print_table();
	}

 	/*---------------------------------------------------------*/
	printf("\nTest %d: Testing Network Win :\n", ++TEST_NO);
	board_reset();
	
	/*Marking down (0,0) - (9,9) should result in a won game*/
	if(cs421net_send("m00", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m11", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m22", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m33", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m44", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m55", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m66", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m77", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m88", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m99", 3)){usleep(SLEEP_DUR);}

	print_table();

	if(!strcmp(status, "Game won!")){test_passed++;}
	else{test_failed++;}
 	/*---------------------------------------------------------*/

	printf("\nTest %d: Testing Network Overmarking :\n", ++TEST_NO);
	board_reset();

	/*Marking down (0,0) - (9,9) with extra coordinates */
	if(cs421net_send("m00", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m11", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m22", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m33", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m44", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m47", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m55", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m66", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m77", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m88", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m89", 3)){usleep(SLEEP_DUR);}
	if(cs421net_send("m99", 3)){usleep(SLEEP_DUR);}

	print_table();

	if(!strcmp(status, "12 Marked of 10")){test_passed++;}
	else{test_failed++;}
 	/*---------------------------------------------------------*/

	if(!UID){
		printf("\nTest %d: Testing Network Win w/ Added Mines:\n", ++TEST_NO);
		board_reset();
		if(write(fd_write, "a69\n" , 4)){/*Expected write to work*/}
		if(cs421net_send("m00", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("m11", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("m22", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("m33", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("m44", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("m55", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("m66", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("m77", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("m88", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("m99", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("m69", 3)){usleep(SLEEP_DUR);}


		print_table();

		if(!strcmp(status, "Game won!")){test_passed++;}
		else{test_failed++;}
	}
 	/*---------------------------------------------------------*/
	if(!UID){
		printf("\nTest %d: Testing Network Win w/ Deleted Mines:\n", ++TEST_NO);
		board_reset();
		if(write(fd_write, "d00\n" , 4)){/*Expected write to work*/}
		if(cs421net_send("m11", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("m22", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("m33", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("m44", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("m55", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("m66", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("m77", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("m88", 3)){usleep(SLEEP_DUR);}
		if(cs421net_send("m99", 3)){usleep(SLEEP_DUR);}


		print_table();

		if(!strcmp(status, "Game won!")){test_passed++;}
		else{test_failed++;}
	}

 	/*---------------------------------------------------------*/

	if(!UID){
		printf("\nTest %d: Testing Network Loss w/ Added Mines:\n", ++TEST_NO);
		board_reset();
		if(write(fd_write, "a69\n" , 4)){/*Expected write to work*/}
		if(cs421net_send("r69", 3)){usleep(SLEEP_DUR);}

		print_table();

		if(!strcmp(status, "You lose!")){test_passed++;}
		else{test_failed++;}
	}

 	/*---------------------------------------------------------*/

	printf("\nTest %d: Testing Statistics View :\n", ++TEST_NO);
	printf("***Note: To clear stats reload minesweeper2 module ***\n");
	print_stats();

 	/*---------------------------------------------------------*/

	printf("\nTest %d: Adding & Sorting Statistics :\n", ++TEST_NO);
	printf("***Note: This will take up to 10 seconds***\n");
	board_reset();
	printf("Adding 1 second record...\n");
	sleep(1);
	if(cs421net_send("q", 1)){usleep(SLEEP_DUR);}
	board_reset();
	printf("Adding 3 second record...\n");
	sleep(3);
	if(cs421net_send("q", 1)){usleep(SLEEP_DUR);}
	board_reset();
	printf("Adding 4 second record...\n");
	sleep(4);
	if(cs421net_send("q", 1)){usleep(SLEEP_DUR);}
	board_reset();
	printf("Adding 2 second record...\n");
	sleep(2);
	if(cs421net_send("q", 1)){usleep(SLEEP_DUR);}
 	print_stats();

 	/*---------------------------------------------------------*/

 	//Test results
 	printf("\nTests %d of %d passed.\n", test_passed, (test_passed + test_failed));
	return 0;
}

void print_stats(){
	printf("Printing Statistics:\n");
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
	if(read(fd_read_ms_ctl, status, sizeof(status)) <= 0){
		printf("Failure to read on %d\n", __LINE__);
	}
	rewind_fd();
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

/* @Name: board_reset
 * @Return: void
 * @Desc: This function returns board to fresh
 * game ready state.
 */
void board_reset(){
	if(write(fd_write, "s\n" , 3)){/*Expected write to work*/}
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
	if(cs421net_send("m00", 3)){usleep(SLEEP_DUR);}
	return 0;
}
#define _POSIX_SOURCE
#define _BSD_SOURCE //For usleep
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>

/**
 * Unit test of your memory allocator implementation. This will
 * allocate and free memory regions.
 */
extern void hw4_test(void);

/**
 * Allocates memory pages and blocks
 * Run at beginning of program
 */
void mem_init();

/**
 * Checks if value in mem is printable
 * value > 0 returned if true,
 * 0 return if false;
 */
int ascii_in_range(char);

/**
 * Display information about memory allocations to standard output.
 *
 * Display to standard output the following:
 * - Memory contents, one frame per line, 16 lines total. Display the
 *   actual bytes stored in memory. If the byte is unprintable (ASCII
 *   value less than 32 or greater than 126), then display a dot
 *   instead.
 * - Current memory allocations, one line of 16 characters, where each
 *   character corresponds to a frame. Indicate reserved frames with
 *   R, free memory with f.
 */
void my_malloc_stats(void);


int BLOCK_SZ = 64;
int NUM_PAGE = 16;

struct page{ 
	char block[64]; 
	int free;
};

struct page mem_page[16];

int main(int argc, char *argv[]){
	mem_init();
	my_malloc_stats();
	//hw4_test();
  	return 0;
}

void mem_init(){
	for(int i = 0; i < NUM_PAGE; i++){
		for(int j = 0; j < BLOCK_SZ; j++){
			mem_page[i].block[j] = 0;
		}
	}
}

void my_malloc_stats(){
	for(int i = 0; i < NUM_PAGE; i++){
		for(int j = 0; j < BLOCK_SZ; j++){
			if(ascii_in_range(mem_page[i].block[j])){
				printf("%c", mem_page[i].block[j]);
			}
			else{
				printf(".");
			}
		}
		printf("\n");
	}
	//Free or reserved
	printf("Memory allocations:\n");
	for(int i = 0; i < NUM_PAGE; i++){
		if(mem_page[i].free){
			printf("R");
		}
		else{
			printf("f");
		}
	}
	printf("\n");
}


/*
** ascii_in_range
** Preconditions: val must be a char
** Postconditions: an int is returned
** Description: 
** Char value is passed in and the ascii value is returned
*/
int ascii_in_range(char val){
	if((int)val < 32 || (int)val > 126){return 0;}
	return (int)val;
}
#define _POSIX_SOURCE
#define _BSD_SOURCE //For usleep
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>
#include <stddef.h>
#define ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)

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

/**
 * Allocate and return a contiguous memory block that is within the
 * memory region.
 *
 * The size of the returned block will be at least @a size bytes,
 * rounded up to the next 64-byte increment.
 *
 * @param size Number of bytes to allocate. If @c 0, your code may do
 * whatever it wants; my_malloc() of @c 0 is "implementation defined",
 * meaning it is up to you if you want to return @c NULL, segfault,
 * whatever.
 *
 * @return Pointer to allocated memory, or @c NULL if no space could
 * be found. If out of memory, set errno to @c ENOMEM.
 */
void *my_malloc(size_t size);

/**
 * Deallocate a memory region that was returned by my_malloc() or
 * my_realloc().
 *
 * If @a ptr is not a pointer returned by my_malloc() or my_realloc(),
 * then send a SIGSEGV signal to the calling process. Likewise,
 * calling my_free() on a previously freed region results in a
 * SIGSEGV.
 *
 * @param ptr Pointer to memory region to free. If @c NULL, do
 * nothing.
 */
//void my_free(void *ptr);
char * base;
char * end;
char * mid;
char * curr_pointer;
char * last_write;

int FRAME_SZ = 64;
int NUM_FRAMES = 16;

struct page{ 
	char frame[64]; 
};

struct frame{
	void *address;
	int limit;
	int freed;
	int free;
};

struct page memory[16];

struct frame table[16];


/**
 *
 * USE PTRDIFF_T
 *
 */
int main(int argc, char *argv[]){
	mem_init();
	void *m1;
	//Make a memory (segment) table !!!!!!!!!!!!!!
	//Frame table
	for(int m = 6; m < 15; m++){
		table[m].free = 0;
	}
	table[0].free = 0;
	table[1].free = 0;
	table[2].free = 0;
	//table[6].free = 0;
	//table[7].free = 0;
	table[8].free = 0;
	m1 = my_malloc(200);
	my_malloc_stats();

	//memset(m1, 'D', 20);
	//my_malloc_stats();
	//ptrdiff_t test;
	//hw4_test();
  	return 0;
}

void *my_malloc(size_t size){
	if(!size) return NULL;

	int frame_alloc = size / 64;
	if(size % 64) frame_alloc++;
	printf("Frames to allocate %d\n", frame_alloc);
	long int i = (curr_pointer - base) / FRAME_SZ;
	printf("%d current index\n", i);
	curr_pointer += FRAME_SZ * 9;
	/*********IMPORTANT**********/
	//curr point is at 0
	i = ((curr_pointer - base) / FRAME_SZ) % NUM_FRAMES;
	//from current pointer find next available blocks
	//TEST MEM WRAPPING, START CURR POINTER AT 9
	int loop = 1;
	int counter = 0;
	int error = 0;
	while(loop){
		if(error > 15){
			printf("Out of Memory\n");
			exit(0);
		}
		i = ((curr_pointer - base) / FRAME_SZ) % NUM_FRAMES;
		for(int k = 0; k < frame_alloc; k++){
			if(table[i + k].free){
				counter++;
			}
			else{
				curr_pointer += FRAME_SZ;
				break;
			}
		}
		if(counter == frame_alloc){
			printf("Move to this pointer\n");
			loop = 0;
			break;
		}
		else{
			error++;
			counter = 0;
		}
	}


	if(table[i].free){
		printf("Row %d is free\n", i);
	}
	else{
		printf("Row %d is locked\n", i);

	}
	
	// Find the memory
	// lock the memory
	// 0 out that memory.
	// Write to the memory
	// Otherwise bail

	printf("Found free frame at %ld\n", i);
	return &memory[i].frame[0];
}
/**
 *
 *
 *
 */
void mem_init(){
	for(int i = 0; i < NUM_FRAMES; i++){
		for(int j = 0; j < FRAME_SZ; j++){
			memory[i].frame[j] = 0;
		}
		table[i].address = &memory[i].frame[0];
		table[i].free = 1;
	}
	base = & memory[0].frame[0];
	end = & memory[15].frame[63];
	curr_pointer = & memory[0].frame[0];
	//long int frame = (end - base) / FRAME_SZ;
	//printf("Test Base = %ld\n", frame );
	//memory[0].frame[0] = 'C';
	//printf("Test End = %p\n", end);
	
	//memset(mid, 'B', 14);
}

/**
 *
 *
 *
 */
void my_malloc_stats(){
	for(int i = 0; i < NUM_FRAMES; i++){
		for(int j = 0; j < FRAME_SZ; j++){
			if(ascii_in_range(memory[i].frame[j])){
				printf("%c", memory[i].frame[j]);
			}
			else{
				printf(".");
			}
		}
		printf("\n");
	}
	//Free or reserved
	printf("Memory allocations:\n");
	for(int i = 0; i < NUM_FRAMES; i++){
		if(table[i].free){
			printf("f");
		}
		else{
			printf("R");
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

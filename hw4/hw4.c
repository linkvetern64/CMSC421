#define _POSIX_SOURCE
#define _BSD_SOURCE //For usleep
#include <signal.h>
#include <errno.h>
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
void my_free(void *ptr);

/**
 * Change the size of the memory block pointed to by @a ptr.
 *
 * - If @a ptr is @c NULL, then treat this as if a call to
 *   my_malloc() for the requested size.
 * - Else if @a size is @c 0, then treat this as if a call to
 *   my_free().
 * - Else if @a ptr is not a pointer returned by my_malloc() or
 *   my_realloc(), then send a SIGSEGV signal to the calling process.
 *
 * Otherwise reallocate @a ptr as follows:
 *
 * - If @a size is smaller than the previously allocated size, then
 *   reduce the size of the memory block. Mark the excess memory as
 *   available. Memory sizes are rounded up to the next 64-byte
 *   increment.
 * - If @a size is the same size as the previously allocated size,
 *   then do nothing.
 * - If @a size is greater than the previously allocated size, then
 *   allocate a new contiguous block of at least @a size bytes,
 *   rounded up to the next 64-byte increment. Copy the contents from
 *   the old to the new block, then free the old block.
 *
 * @param ptr Pointer to memory region to reallocate.
 * @param size Number of bytes to reallocate.
 *
 * @return If allocating a new memory block or if resizing a block,
 * then pointer to allocated memory; @a ptr will become invalid. If
 * freeing a memory region or if allocation fails, return @c NULL. If
 * out of memory, set errno to @c ENOMEM.
 */
void *my_realloc(void *ptr, size_t size);

/**
 * Unit test of your memory allocator implementation. This will
 * allocate and free memory regions.
 */
extern void hw4_test(void);

/*
 * Global var init
 */
char * base;
char * end;
char * curr_pointer;
int FRAME_SZ = 64;
int NUM_FRAMES = 16;

struct page{ 
	char frame[64]; 
};

struct frame{
	void *address;
	int limit;
	int res;
	int freed;
	int free;
};

struct page memory[16];
struct frame table[16];


/**
 * Main function
 * Preconditions:
 * Postconditions:
 * Description:
 * USE PTRDIFF_T
 *
 */
int main(int argc, char *argv[]){
	mem_init();
	
	
	
	//POSSIBLY ADD CURR_POINTER += 64
	//WHERE RETURNS NULL
	// 15 to 0 is not contiguous
	/*
	void *m1, *m2;
	m1 = my_malloc(30);
	m2 = my_malloc(20);
	memset(m1, 'D', 20);
	memset(m2, 'A', 20);
	m1 = my_realloc(m1, 40);
	my_malloc_stats();
	*/
	//ptrdiff_t test;
	hw4_test();
  	return 0;
}

void *my_realloc(void *ptr, size_t size){
	int m = ((char * )ptr - base) / FRAME_SZ;
	//if already freed or doesn't exist, Segfault
		
	if(ptr == NULL){
		return my_malloc(size);
	}	
	else if(size == 0){
		printf("Freeing\n");
		my_free(ptr);
		return NULL;
	}
	else if(table[m].address != ptr){
		raise(SIGSEGV);
		return NULL;
	}
	else{
		//Memory Shrink
		if(table[m].limit > size){
			printf("Size smol:\n");
			//my_free(ptr);
			
			printf("current size = %d\n", table[m].limit);
			printf("shrinking to size = %d\n", size);

			int toF = size / 64;
			if(size % 64) toF++;

			int currF = table[m].limit / 64;
			if(table[m].limit) currF++;

			int remove = currF - toF;
			printf("%d @ current frame\n", 	((curr_pointer - base) / FRAME_SZ) % NUM_FRAMES);
			printf("current frame usage %d\n", currF);
			printf("shrinking to frame usage %d\n", toF);
			printf("Removing %d frame(s)\n", remove);

			for(int i = toF; i < currF; i++){
				printf("Marking %d free\n", (m + i));
				table[m + i].free = 1;
			}
			curr_pointer -= FRAME_SZ * 6;
			return curr_pointer;
		}
		//Memory Increase
		else if(table[m].limit < size){
			void *mem = my_malloc(size);
			int n = ((char * )mem - base) / FRAME_SZ;
			for(int i = 0; i < table[m].limit; i++){
				memory[n].frame[i] = memory[m].frame[i];
			}
			my_free(ptr);
			return mem;
		}
	}
	return NULL;
}


void my_free(void *ptr){
	if(ptr != NULL){
		int m = ((char * )ptr - base) / FRAME_SZ;
		//if already freed or doesn't exist, Segfault
		if(table[m].freed || table[m].address != ptr) raise(SIGSEGV);
		table[m].freed = 1;
		//printf("Remove %d blocks\n", table[m].res);
		for(int i = 0; i < table[m].res; i++){
			table[m + i].free = 1;
		}
	}
}

void *my_malloc(size_t size){
	if(!size) return NULL;

	int frame_alloc = size / 64;
	if(size % 64) frame_alloc++;
	long int i = (curr_pointer - base) / FRAME_SZ;
	
	i = ((curr_pointer - base) / FRAME_SZ) % NUM_FRAMES;

	int counter = 0;
	int error = 0;

	while(1){
		if(error > 15){
			//printf("Out of Memory\n");
			errno = ENOMEM;
			break;
		}
		i = ((curr_pointer - base) / FRAME_SZ);
		
		for(int k = 0; k < frame_alloc; k++){
			if((i + k) > 15){
				errno = ENOMEM;
				return NULL;
			}
			if(table[i + k].free){
				counter++;
			}
			else{
				curr_pointer += FRAME_SZ;
				if(curr_pointer > end){
					curr_pointer -= 1024;
					//return NULL;
				}
				break;
			}
		}
		//If free mem blocks exist, update frame table
		if(counter == frame_alloc){
			
			//Update everything here
			//Updates current indexes with information
			int index = ((curr_pointer - base) / FRAME_SZ);
			for(int k = 0; k < frame_alloc; k++){
				index = ((((curr_pointer - base) / FRAME_SZ) + k)%16);
				table[index].freed = 0;
				table[index].free = 0;
				table[index].limit = size;
				table[index].res = frame_alloc;
			}
			char * newP = curr_pointer;
			curr_pointer += FRAME_SZ;
			return newP;
		}
		else{
			error++;
			counter = 0;
		}
	}
	errno = ENOMEM;
	return NULL;
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
}

/**
 *
 *
 *
 */
void my_malloc_stats(){
	printf("Memory contents:\n");
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

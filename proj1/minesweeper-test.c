/* YOUR FILE-HEADER COMMENT HERE */

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/user.h>

void print_table(void);

char * board;
char status[80];

int main(void) {

	//init section
	int fd_read_ms = open("/dev/ms", O_RDONLY);
	int fd_read_ms_ctl = open("/dev/ms_ctl", O_RDONLY);
	int fd_write = open("/dev/ms_ctl", O_WRONLY);

	if(fd_read_ms < 0 || fd_write < 0 || fd_read_ms_ctl < 0){
		printf("/dev/ms is not loaded.\n");
	}

	//mmap section
	board = mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, fd_read_ms, 0);
	if(board == MAP_FAILED){printf("test failed on %d\n", __LINE__);}

	//Test section
				 

	if(write(fd_write, "s" , 3)){}
 	print_table();

	if(write(fd_write, "q" , 3)){}
				
	if(read(fd_read_ms_ctl, &status, sizeof(status)) <= 0){
		printf("Failure to read on %d\n", __LINE__);
	}
	
	print_table();
	printf("Status : %s\n", status);

	return 0;
}


void print_table(){
	printf("0123456789\n");
	for(int i = 0; i < 10; i++){
		printf("%d",i);
		for(int j = 0; j < 10; j++){
			printf("%c",board[10*i + j]);
		}
		printf("\n");
	}
}
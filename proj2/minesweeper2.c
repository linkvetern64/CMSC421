/**
 * @Author Joshua Standiford (jstand1@umbc.edu)
 * @E-mail jstand1@umbc.edu
 * This file is used to create and insert misc devices into the kernel.
 * /dev/ms will contain the Minesweeper gameboard and /dev/ms_ctl
 * handles user input and prints the game status.
 */
/*
 * This file uses kernel-doc style comments, which is similar to
 * Javadoc and Doxygen-style comments.  See
 * ~/linux/Documentation/kernel-doc-nano-HOWTO.txt for details.
 */

/*
 * Getting compilation warnings?  The Linux kernel is written against
 * C89, which means:
 *  - No // comments, and
 *  - All variables must be declared at the top of functions.
 * Read ~/linux/Documentation/CodingStyle to ensure your project
 * compiles without warnings.
 */



/*
	set flags to 0, cookie, 
	look up ISR cookies, (cookie very similar to web cookie, IRQ handler, must be a pointer)

	lower half of assignment, using a subroutine

	make subroutine and change write callback to new function
*/

#define pr_fmt(fmt) "MS: " fmt

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include "xt_cs421net.h"
#include <linux/sched.h>
#include <linux/gfp.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/time.h>

#define NUM_ROWS 10
#define NUM_COLS 10
#define BOARD_SIZE (NUM_ROWS * NUM_COLS)
#define CS421NET_IRQ 6

/*Spinlock to handle critical sections when modifying board*/
static DEFINE_SPINLOCK(lock);

/*defined as 10 by default*/
static int NUM_MINES;

/** true if using a fixed initial game board, false to randomly
    generate it */
static bool fixed_mines;

/** holds locations of mines */
static bool game_board[NUM_ROWS][NUM_COLS];

/** buffer that displays what the player can see */
static char *user_view;

/* buffer that displays statistics*/
static char *stats_view;

/** buffer that holds admin board display*/
static char * admin_board;

/** buffer that holds values passed into ms_ctl_write*/
static char *buf;

static char *tmp_stats;

/*If data is sent from network this is true.*/
static bool net_sig;

/*Holds the temporary board*/
static char *tmp;

/* Netpacket used for 421 network play*/
static const char *packet;

/** tracks number of mines that the user has marked */
static unsigned mines_marked;

/** true if user revealed a square containing a mine, false
    otherwise */
static bool game_over;

static struct timeval then;
static struct timeval now;

/**
 * String holding the current game status, generated via scnprintf().
 * Game statuses are like the following: "3 out of 10 mines
 * marked" or "Game Over".
 */
static char game_status[80];

static char to_stats[10];

static int right;
/* GLOBAL VARS GO ABOVE */ 

 
/**
 * cs421net_top() - top-half of network ISR
 * @irq: IRQ that was invoked
 * @cookie: Pointer to data that was passed into
 * request_threaded_irq() (ignored)
 *
 * If @irq is %CS421NET_IRQ, then wake up the bottom-half. Otherwise,
 * return %IRQ_NONE.
 */
static irqreturn_t cs421net_top(int irq, void *cookie);
/**
 * cs421net_bottom() - bottom-half to network ISR
 * @irq: IRQ that was invoked (ignored)
 * @cookie: Pointer that was passed into request_threaded_irq()
 * (ignored)
 *
 * Fetch the incoming packet, via cs421net_get_data(). Treat the data
 * as if it were user input, as per minesweeper_ctl_write(). Remember
 * to add appropriate spin lock calls in this function.
 *
 * Note that the incoming payload is NOT a string; you can NOT use
 * strcpy() or strlen() on it.
 *
 * Return: always %IRQ_HANDLED
 */
static irqreturn_t cs421net_bottom(int irq, void *cookie);
/**/
static void game_reveal_mines(void);
/**/
static void game_reset(void);
/**/
static ssize_t delta_mines(int, int, int, size_t);
/**/
static bool check_won(void);
/**/
static bool is_authorized(void);
/**/
static bool pos_equals_mark(int);
/**/
static void record_stats(void);
/**/
//static void sort_list(void);
/* PROTOTYPES GO ABOVE */

/* Nodes of linked list */
struct stats{
	int mines;
	int marked_right;
	int marked_wrong;
	struct list_head list;
};


struct list_head some_list;
static LIST_HEAD(mylist); 

/*
static void sort_list(){
	struct stats *node, *pos;
	list_for_each_entry(pos, &mylist, list){
		scnprintf(to_stats, 9, "%d %d %d\n", pos->mines, pos->marked_right, pos->marked_wrong);
		strcat(tmp_stats, to_stats);
	}
}*/

/* record_stats
 * desc: This function records specific stats to a linked list
 * that data is written to ms_stats and is accessible via mmap
 *
 */
static void record_stats(){
	/*Logic to determine stats here*/
	struct stats *node, *pos;
	int i, j, len, marked_correctly, loc, time;
	time = (int)(now.tv_sec - then.tv_sec);

	marked_correctly = 0;
	right = 0;
	loc = 0;
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {
			loc = 10 * i + j;
			if (game_board[j][i] && user_view[loc] == '?') {
				marked_correctly++;
			}
		}
	}
	right = marked_correctly;


	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if(!node){
		printk("Error creating node\n");
	}

	node->mines = NUM_MINES;
	node->marked_right = right;
	node->marked_wrong = mines_marked - right;

	list_add_tail(&node->list, &mylist);
	i = 0;
	while (i < PAGE_SIZE) {
		stats_view[i++] = ' ';
	}
	len = 0;
	i = 0;

	tmp_stats[0] = '\0'; 
	list_for_each_entry(pos, &mylist, list){
		len += scnprintf(to_stats, 9, "%d %d %d %d\n", time, pos->mines, pos->marked_right, pos->marked_wrong);
		strcat(tmp_stats, to_stats);
	}
	//RECORD STATS IS WORKING
	printk("New stats\n");
	for(i = 0; i < strlen(tmp_stats); i++){
		stats_view[i] = tmp_stats[i];
		printk("%c", stats_view[i]);
	}
}


 

/*
 *
 *
 *
 */
static bool pos_equals_mark(int pos){
	if(user_view[pos] == '?'){
		mines_marked--;
		user_view[pos] = '.';
		return true;
	}
	return false;
}

/*Name delta_mines
 *return: void
 *param: int op, decides if mine will be added or subtracted
 *param: int x, x coordinate for adding/removing mine
 *param: int y, y coordinate for adding/removing mine
 *param: size_t count, # of bytes to be returned if error
 *desc: will add or remove mine from gameboard if the user
 *is root.  Will check if authorized and not a net user.
 */
static ssize_t delta_mines(int ops, int x, int y, size_t count){
	int pos;

	if(!is_authorized() || net_sig){
		spin_unlock(&lock);
		return -EPERM;
	}

	//to add mine greater than 0, to remove mine less than 0
	

	pos = 10 * y + x;

	if (!game_over) {
	
		/*Check that X and Y are in range of 0 - 9 */
		if (x < 0 || x > 9 || y < 0 || y > 9) {
			spin_unlock(&lock);
			return count;
		}
		//broken when adding / subtracting mines
		if(ops > 0){
			if(!game_board[x][y] && NUM_MINES < 100){
				game_board[x][y] = true;
				NUM_MINES += 1;
			}
			else{
				//scnprintf(game_status, 80, "Can't add more mines.");
			}
		}
		else{
			if(game_board[x][y] && NUM_MINES > 1){
				game_board[x][y] = false;
				NUM_MINES += -1;
			}
			else{
				//scnprintf(game_status, 80, "Must have atleast 1 mine.");
			}
		}
		

		if(user_view[pos] == '?'){mines_marked--;}
		user_view[pos] = '.';
		if (y + 1 <= 9 && y + 1 >= 0) {pos_equals_mark(10 * (y + 1) + x);}
		if (y - 1 <= 9 && y - 1 >= 0) {pos_equals_mark(10 * (y - 1) + x);}
		if (x - 1 <= 9 && x - 1 >= 0) {pos_equals_mark(10 * (y + 0) + (x - 1));}
		if (x - 1 <= 9 && x - 1 >= 0 && y + 1 <= 9 && y + 1 >= 0) {pos_equals_mark(10 * (y + 1) + (x - 1));}
		if (x - 1 <= 9 && x - 1 >= 0 && y - 1 <= 9 && y - 1 >= 0) {pos_equals_mark(10 * (y - 1) + (x - 1));}
		if (x + 1 <= 9 && x + 1 >= 0) {pos_equals_mark(10 * (y + 0) + (x + 1));}
		if (x + 1 <= 9 && x + 1 >= 0 && y + 1 <= 9 && y + 1 >= 0) {pos_equals_mark(10 * (y + 1) + (x + 1));}
		if (x + 1 <= 9 && x + 1 >= 0 && y - 1 <= 9 && y - 1 >= 0) {pos_equals_mark(10 * (y - 1) + (x + 1));}
		
		
		if(!check_won()){
			scnprintf(game_status, 80, "%d Marked of %d", mines_marked, NUM_MINES);
		}
		
	}
	spin_unlock(&lock);
	return count;
}




/*Name: check_won
 *Return: true if game is over, false if not. val of game_over;
 *Desc: This function will check the board for completed game.
 * Will update game_status and change game_over if it is so.
 */
static bool check_won(void){
	int marked_correctly, i, j, pos;
	marked_correctly = 0;
	right = 0;
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {
			pos = 10 * i + j;
			if (game_board[j][i] && user_view[pos] == '?') {
				marked_correctly++;
			}
		}
	}
	right = marked_correctly;
	scnprintf(game_status, 80, "%d Marked of %d", mines_marked, NUM_MINES);
	if (marked_correctly == NUM_MINES && mines_marked == NUM_MINES) {
		strncpy(game_status, "Game won!\0", 80);
		do_gettimeofday(&now);
		record_stats();
		game_over = true;
	}	
	return game_over;
}



/* @Name: is_authorized
 * @Return: bool
 * @Desc: Checks if the user is a root or not.
 * Returns true if the user is root, otherwise returns false
 */
static bool is_authorized(){
	return current_uid().val == 0;
}


/* @Name: game_reveal_mines
 * @Return: void
 * @Desc: this function reveals the mines as asterisks
 * on the user_view, displayed from /dev/ms
 */
static void game_reveal_mines()
{
	int x, y, pos;
	for (y = 0; y < 10; y++) {
		for (x = 0; x < 10; x++) {
			pos = 10 * y + x;
			if (game_board[x][y]) {
				user_view[pos] = '*';
			}
		}
	}
}

/* @Name: game_reset
 * @Return: void
 * @Desc: This function resets the gameboard.
 * the game_status is reset, game_over is reset
 * random mines are set unless fixed_mines is true.
 */
static void game_reset()
{
	int i, k, j, marked, X, Y;
	char rand[8];
	do_gettimeofday(&then);
	//printk("Current time = %d\n", (int)then->tv_sec);
	//Need \0 null terminator denotes string 
	strncpy(game_status, "Game reset\0", 80);
	NUM_MINES = 2;
	i = 0;
	game_over = false;
	mines_marked = 0;
	fixed_mines = true; // TEST MODE
	/* Reset gameboard */
	for (k = 0; k < 10; k++) {
		for (j = 0; j < 10; j++) {
			game_board[k][j] = false;
		}
	}

	//Fill user_view with . up to 4096 or PAGE_SIZE
	while (i < PAGE_SIZE) {
		user_view[i++] = '.';
	}

	marked = 0;

	if (fixed_mines) {
		for (i = 0; i < NUM_MINES; i++) {
			game_board[i][i] = true;
		}
	} else {
		for (;;) {
			get_random_bytes(rand, 1);
			X = abs((int)rand[0]) % 10;
			get_random_bytes(rand, 1);
			Y = abs((int)rand[0]) % 10;
			if (!game_board[X][Y]) {
				game_board[X][Y] = true;
				marked++;
				if (marked >= NUM_MINES) {
					break;
				}
			}
		}
	}
}

/**
 * ms_read() - callback invoked when a process reads from /dev/ms
 * @filp: process's file object that is reading from this device (ignored)
 * @ubuf: destination buffer to store game board
 * @count: number of bytes in @ubuf
 * @ppos: file offset
 *
 * Copy to @ubuf the contents of @user_view, starting from the offset
 * @ppos. Copy the lesser of @count and (@BOARD_SIZE - *@ppos). Then
 * increment the value pointed to by @ppos by the number of bytes that
 * were copied.
 *
 * Return: number of bytes written to @ubuf, or negative on error
 */
static ssize_t ms_read(struct file *filp, char __user * ubuf,
		       size_t count, loff_t * ppos)
{
	int x, y, pos, len;
	size_t comp;
	 
	len = 100;
 	if (*ppos != 0) {
		return 0;
	}

	comp = (BOARD_SIZE - *ppos);
	count = min(count, comp);

	//This makes a copy of the user_view and marks it with mines
	for (y = 0; y < 10; y++) {
		for (x = 0; x < 10; x++) {
			pos = 10 * y + x;
			admin_board[pos] = user_view[pos];
			if (game_board[x][y]) {
				admin_board[pos] = '*';
			}
		}
	}
	

	if (copy_to_user(ubuf, user_view, count)) {
		return -EINVAL;
	};
	if(is_authorized() && copy_to_user(ubuf, admin_board, count)){
		return -EINVAL;
	}
	*ppos = count;

	return len;
}

/**
 * ms_mmap() - callback invoked when a process mmap()s to /dev/ms
 * @filp: process's file object that is mapping to this device (ignored)
 * @vma: virtual memory allocation object containing mmap() request
 *
 * Create a read-only mapping from kernel memory (specifically,
 * @user_view) into user space.
 *
 * Code based upon
 * <a href="http://bloggar.combitech.se/ldc/2015/01/21/mmap-memory-between-kernel-and-userspace/">http://bloggar.combitech.se/ldc/2015/01/21/mmap-memory-between-kernel-and-userspace/</a>
 *
 * You do not need to modify this function.
 *
 * Return: 0 on success, negative on error.
 */
static int ms_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long size = (unsigned long)(vma->vm_end - vma->vm_start);
	unsigned long page = vmalloc_to_pfn(user_view);
	if (size > PAGE_SIZE)
		return -EIO;
	vma->vm_pgoff = 0;
	vma->vm_page_prot = PAGE_READONLY;
	if (remap_pfn_range(vma, vma->vm_start, page, size, vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

/**
 * ms_stats_mmap() - callback invoked when a process mmap()s to /dev/ms
 * @filp: process's file object that is mapping to this device (ignored)
 * @vma: virtual memory allocation object containing mmap() request
 *
 * Create a read-only mapping from kernel memory (specifically,
 * @user_view) into user space.
 *
 * Code based upon
 * <a href="http://bloggar.combitech.se/ldc/2015/01/21/mmap-memory-between-kernel-and-userspace/">http://bloggar.combitech.se/ldc/2015/01/21/mmap-memory-between-kernel-and-userspace/</a>
 *
 * You do not need to modify this function.
 *
 * Return: 0 on success, negative on error.
 */
static int ms_stats_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long size = (unsigned long)(vma->vm_end - vma->vm_start);
	unsigned long page = vmalloc_to_pfn(stats_view);
	if (size > PAGE_SIZE)
		return -EIO;
	vma->vm_pgoff = 0;
	vma->vm_page_prot = PAGE_READONLY;
	if (remap_pfn_range(vma, vma->vm_start, page, size, vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

/**
 * ms_ctl_read() - callback invoked when a process reads from
 * /dev/ms_ctl
 * @filp: process's file object that is reading from this device (ignored)
 * @ubuf: destination buffer to store game board
 * @count: number of bytes in @ubuf
 * @ppos: file offset
 *
 * Copy to @ubuf the contents of @game_status, starting from the
 * offset @ppos. Copy the lesser of @count and (string length of
 * @game_status - *@ppos). Then increment the value pointed to by
 * @ppos by the number of bytes that were copied.
 *
 * Return: number of bytes written to @ubuf, or negative on error
 */
static ssize_t ms_ctl_read(struct file *filp, char __user * ubuf, size_t count,
			   loff_t * ppos)
{
	int len;
	size_t comp;

	len = 80;

	if (*ppos != 0) {
		return 0;
	}
	//strlength of gamestatus
	comp = (80 - *ppos);
	count = min(count, comp);

	if (copy_to_user(ubuf, game_status, count)) {
		return -EINVAL;
	};

	*ppos = count;

	return len;
}

/**
 * ms_ctl_write() - callback invoked when a process writes to
 * /dev/ms_ctl
 * @filp: process's file object that is writing to this device (ignored)
 * @ubuf: source buffer from user
 * @count: number of bytes in @ubuf
 * @ppos: file offset (ignored)
 *
 * Copy the contents of @ubuf, up to the lesser of @count and 8 bytes,
 * to a temporary variable. Then parse that character array as
 * following:
 *
 *   s   - Quit existing game and start a new one.
 *   rXY - Reveal the contents at (X, Y). X and Y must be integers
 *         from zero through nine.
 *   mXY - Toggle the marking at (X, Y) as either a bomb or not. X and
 *         Y must be integers from zero through nine.
 *   q   - Reveal all mines and quit the current game.
 *
 * If the input is none of the above, then return -EINVAL.
 *
 * Be aware that although the user using (X, Y) coordinates,
 * internally your program operates via row/column.
 *
 * WARNING: Note that @ubuf is NOT a string! You CANNOT use strcpy()
 * or strlen() on it!
 *
 * Return: @count, or negative on error
 */
static ssize_t ms_ctl_write(struct file *filp, const char __user * ubuf,
			    size_t count, loff_t * ppos)
{
	int x, y, pos, mines, l;
	char op;
	size_t comp;
	spin_lock(&lock);
	comp = 8;
	count = min(count, comp);

 	if (copy_from_user(buf, ubuf, count) && !net_sig) {
		spin_unlock(&lock);
		return count;
	}
	//-3345 identifier for net code
	if(net_sig){
		for(l = 0; l <= count; l++){
			buf[l] = ubuf[l];
		}
	}
	//ubuf is what takes the users input
	//count is size of input + 1 ?for null terminator?
	//ppos && filp are ignored for this
	//Check if entry is greater than 3, or character not in lowercase range
	if (count > 4 || !((int)buf[0] >= 97 && (int)buf[0] <= 122)) {
		spin_unlock(&lock);
		return count;
	}
  	op = buf[0];
	 
	//Converts XY to integer value                  
	x = buf[1] - '0';
	y = buf[2] - '0';
	
	//Position = 10 * row position + column #
	pos = 10 * y + x;
	
	switch (op) {
	case 's':
			if((int)buf[1] != 10){
				scnprintf(game_status, 80, "Invalid entry");
				spin_unlock(&lock);
				return count;
			}
			 
			game_reset();
			break;

	case 'r':

		if (game_over) {
			spin_unlock(&lock);
			return count;
		}
		
		if((int)buf[3] != 10){
			scnprintf(game_status, 80, "Invalid entry");
			spin_unlock(&lock);
			return count;
		}

		scnprintf(game_status, 80, "%d Marked of 10", mines_marked);
		//Checks if X & Y are non negative and 0 - 9
		if (!((x > -1 && x < 10) && (y > -1 && y < 10))) {
			spin_unlock(&lock);
			return count;
		}

		/* CHECK THAT X & Y in correct positions */
		else if (game_board[x][y]) {
			strncpy(game_status, "You lose!\0", 80);
			record_stats();
			game_reveal_mines();
			do_gettimeofday(&now);
			game_over = true;
			spin_unlock(&lock);
			return count;
		}

		mines = 0;

		//Inner boundaries X&Y no the outer edge
		//check wraparound boundaries
		if (!game_over && (x < 9 && x > 0) && (y > 0 && y < 9)) {
			if (game_board[x][y + 1]) {
				mines++;
			}
			if (game_board[x][y - 1]) {
				mines++;
			}
			if (game_board[x - 1][y]) {
				mines++;
			}
			if (game_board[x - 1][y + 1]) {
				mines++;
			}
			if (game_board[x - 1][y - 1]) {
				mines++;
			}
			if (game_board[x + 1][y]) {
				mines++;
			}
			if (game_board[x + 1][y + 1]) {
				mines++;
			}
			if (game_board[x + 1][y - 1]) {
				mines++;
			}
			if (mines > 0) {
				user_view[pos] = mines + '0';
			} else {
				user_view[pos] = '-';
			}
		} else if (!game_over) {

			mines = 0;
			/*Check that X and Y are in range of 0 - 9 */
			if (x < 0 || x > 9 || y < 0 || y > 9) {
				spin_unlock(&lock);
				return count;
			}

			if (y + 1 <= 9 && y + 1 >= 0 && game_board[x][y + 1]) {
				mines++;
			}
			if (y - 1 <= 9 && y - 1 >= 0 && game_board[x][y - 1]) {
				mines++;
			}
			if (x - 1 <= 9 && x - 1 >= 0 && game_board[x - 1][y]) {
				mines++;
			}
			if (x - 1 <= 9 && x - 1 >= 0 && y + 1 <= 9 && y + 1 >= 0
			    && game_board[x - 1][y + 1]) {
				mines++;
			}
			if (x - 1 <= 9 && x - 1 >= 0 && y - 1 <= 9 && y - 1 >= 0
			    && game_board[x - 1][y - 1]) {
				mines++;
			}
			if (x + 1 <= 9 && x + 1 >= 0 && game_board[x + 1][y]) {
				mines++;
			}
			if (x + 1 <= 9 && x + 1 >= 0 && y + 1 <= 9 && y + 1 >= 0
			    && game_board[x + 1][y + 1]) {
				mines++;
			}
			if (x + 1 <= 9 && x + 1 >= 0 && y - 1 <= 9 && y - 1 >= 0
			    && game_board[x + 1][y - 1]) {
				mines++;
			}
			if (mines > 0) {
				user_view[pos] = mines + '0';
			} else {
				user_view[pos] = '-';
			}
		}
		break;

	case 'a':
		return delta_mines(1, x, y, count);
		break;

	case 'd':
		return delta_mines(-1, x, y, count);
		break;

	case 'm':

		if (game_over) {
			spin_unlock(&lock);
			return count;
		}
 		if((int)buf[3] != 10){
			scnprintf(game_status, 80, "Invalid entry");
			spin_unlock(&lock);
			return count;
		}
		
		//Checks if X & Y are non negative and 0 - 9
		if (!((x > -1 && x < 10) && (y > -1 && y < 10))) {
			spin_unlock(&lock);
			return count;
		}

		switch (user_view[pos]) {
		case '.':
			user_view[pos] = '?';
			mines_marked++;
			break;
		case '?':
			user_view[pos] = '.';
			mines_marked--;
			break;
		default:
			/* Do nothing */
			break;
		}

		check_won();
		break;

	case 'q':

		if((int)buf[1] != 10 && !game_over){
			scnprintf(game_status, 80, "Invalid entry");
			spin_unlock(&lock);
			return count;
		}

		


		/* User quits the game */
		strncpy(game_status, "You lose!\0", 80);
		if(!game_over){
			record_stats();
		}
		do_gettimeofday(&now);
		game_over = true;
		game_reveal_mines();
		break;

	default:
		if(game_over){
			spin_unlock(&lock);
			return count;
		}
		scnprintf(game_status, 80, "Invalid entry");
		spin_unlock(&lock);
		return -EINVAL;
		break;
	}
	spin_unlock(&lock);
	return count;
}


static const struct file_operations fop_ms = {
	.owner = THIS_MODULE,
	.read = ms_read,
	.mmap = ms_mmap,
};

static const struct file_operations fop_ms_ctl = {
	.owner = THIS_MODULE,
	.read = ms_ctl_read,
	.write = ms_ctl_write,
};

static const struct file_operations fop_ms_stats = {
	.owner = THIS_MODULE,
	.mmap = ms_stats_mmap,
};


static struct miscdevice ms = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ms",
	.fops = &fop_ms,
	.mode = 0444,
};

static struct miscdevice ms_ctl = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ms_ctl",
	.fops = &fop_ms_ctl,
	.mode = 0666,
};
 
static struct miscdevice ms_stats = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ms_stats",
	.fops = &fop_ms_stats,
	.mode = 0444,
};

static irqreturn_t cs421net_top(int irq, void *cookie){
 	if(irq == CS421NET_IRQ){
		return IRQ_WAKE_THREAD;
	}
	return IRQ_NONE;
}



static irqreturn_t cs421net_bottom(int irq, void *cookie){
	//size_t * const len;
	size_t * const len;
	size_t i; 
	int counter;

	counter = 0;
	packet = cs421net_get_data((size_t * const)&len); 
	
	//parse through packet for \n
	if(packet != NULL && len > 0){
		for(i = 0; i < (size_t)len; i++){
			if((int)packet[(int)i] != 10 && (int)packet[(int)i] != 13){
				tmp[counter] = (char)packet[(int)i];
				counter++;
			
			}
		}
		tmp[counter] = '\n';
		net_sig = true;
	 	ms_ctl_write(NULL, tmp, counter, 0);
	 	net_sig = false;
 	}
	return IRQ_HANDLED;
}


/**
 * minesweeper_init() - entry point into the minesweeper kernel module
 * Return: 0 on successful initialization, negative on error
 */
static int __init minesweeper_init(void)
{
	pr_info("Initializing the game.\n");
	if (fixed_mines)
		pr_info("Using a fixed minefield.\n");
	user_view = vmalloc(PAGE_SIZE);
	buf = vmalloc(PAGE_SIZE);
	tmp = vmalloc(PAGE_SIZE);
	admin_board = vmalloc(PAGE_SIZE);
	stats_view = vzalloc(PAGE_SIZE);
	tmp_stats = vmalloc(PAGE_SIZE);
	//packet = vmalloc(PAGE_SIZE);
	

	if (!user_view) {
		pr_err("Could not allocate memory\n");
		return -ENOMEM;
	}
	/* YOUR CODE HERE */
	/*L21 is very important for this.*/
	cs421net_enable();
	if(request_threaded_irq(CS421NET_IRQ, cs421net_top, cs421net_bottom, 0, "421 HERE TEST" , NULL) == 0){
		//Worked
		printk("request IRQ worked successfully\n");
	}
	else{
		printk("request IRQ failed\n");
	}	

	misc_register(&ms);
	misc_register(&ms_ctl);
	misc_register(&ms_stats);
	game_reset();

	return 0;
}

/**
 * minesweeper_exit() - called by kernel to clean up resources
 */
static void __exit minesweeper_exit(void)
{

	struct stats *entry, *tmpP;

	pr_info("Freeing resources.\n");
	vfree(user_view);
	vfree(buf);
	vfree(tmp);
	vfree(admin_board);
	vfree(stats_view);
	vfree(tmp_stats);
	//vfree(packet);

	list_for_each_entry_safe(entry, tmpP, &mylist, list){
		kfree(entry);
	}
	INIT_LIST_HEAD(&mylist);

	/* YOUR CODE HERE */
	free_irq(CS421NET_IRQ, NULL);
	misc_deregister(&ms);
	misc_deregister(&ms_ctl);
	misc_deregister(&ms_stats);
	cs421net_disable();
}

module_init(minesweeper_init);
module_exit(minesweeper_exit);
module_param(fixed_mines, bool, 0444);

MODULE_DESCRIPTION("CS421 Minesweeper Game");
MODULE_LICENSE("GPL");
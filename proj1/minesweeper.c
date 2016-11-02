/* YOUR FILE-HEADER COMMENT HERE */

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

#define pr_fmt(fmt) "MS: " fmt

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>

#define NUM_ROWS 10
#define NUM_COLS 10
#define BOARD_SIZE (NUM_ROWS * NUM_COLS)
#define NUM_MINES 10

static const struct file_operations;

/** true if using a fixed initial game board, false to randomly
    generate it */
static bool fixed_mines;

/** holds locations of mines */
static bool game_board[NUM_ROWS][NUM_COLS];

/** buffer that displays what the player can see */
static char *user_view;

/** tracks number of mines that the user has marked */
static unsigned mines_marked;

/** true if user revealed a square containing a mine, false
    otherwise */
static bool game_over;

/**
 * String holding the current game status, generated via scnprintf().
 * Game statuses are like the following: "3 out of 10 mines
 * marked" or "Game Over".
 */
static char game_status[80];

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
	/* YOUR CODE HERE, and update the following 'return' statement */
	return -EPERM;
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
	/* YOUR CODE HERE, and update the following 'return' statement */
	return 0;
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
	/* YOUR CODE HERE */
	return count;
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
	if (!user_view) {
		pr_err("Could not allocate memory\n");
		return -ENOMEM;
	}

	/* YOUR CODE HERE */

	return 0;
}

/**
 * minesweeper_exit() - called by kernel to clean up resources
 */
static void __exit minesweeper_exit(void)
{
	pr_info("Freeing resources.\n");
	vfree(user_view);

	/* YOUR CODE HERE */
}

module_init(minesweeper_init);
module_exit(minesweeper_exit);
module_param(fixed_mines, bool, 0444);

MODULE_DESCRIPTION("CS421 Minesweeper Game");
MODULE_LICENSE("GPL");

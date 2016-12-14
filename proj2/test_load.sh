./proj2_start.sh

sudo rmmod minesweeper2

make

sudo insmod minesweeper2.ko fixed_mines=1

lsmod | grep minesweeper2

dmesg | tail


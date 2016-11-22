sudo rmmod minesweeper

make

sudo insmod minesweeper.ko fixed_mines = 1

lsmod | grep minesweeper

dmesg | tail


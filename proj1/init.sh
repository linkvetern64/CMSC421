wget http://www.csee.umbc.edu/~jtang/cs421.f16/homework/proj1/minesweeper.c

wget http://www.csee.umbc.edu/~jtang/cs421.f16/homework/proj1/minesweeper-test.c

wget http://www.csee.umbc.edu/~jtang/cs421.f16/homework/proj1/Kbuild

wget http://www.csee.umbc.edu/~jtang/cs421.f16/homework/proj1/Makefile

wget http://www.csee.umbc.edu/~jtang/cs421.f16/homework/proj1/play.sh 

chmod u+x play.sh

make

sudo insmod minesweeper.ko

lsmod | grep minesweeper

dmesg | tail

sudo rmmod minesweeper

sudo insmod minesweeper.ko fixed_mines=1

dmesg | tail

sudo rmmod minesweeper

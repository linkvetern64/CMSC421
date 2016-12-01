# Basic Makefile to pull in kernel's KBuild to build an out-of-tree
# kernel module

KDIR ?= /lib/modules/$(shell uname -r)/build

all: modules minesweeper2-test

minesweeper2-test: minesweeper2-test.o cs421net.o
	gcc --std=c99 -Wall -O2 -pthread -o $@ $^ -lm

minesweeper2-test.o: minesweeper2-test.c cs421net.h
cs421net.o: cs421net.c cs421net.h

%.o: %.c
	gcc --std=c99 -Wall -O2 -c -o $@ $<

modules:
	$(MAKE) -C $(KDIR) M=$$PWD $@

clean:
	$(MAKE) -C $(KDIR) M=$$PWD $@
	-rm minesweeper2-test

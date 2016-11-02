# Basic Makefile to pull in kernel's KBuild to build an out-of-tree
# kernel module

KDIR ?= /lib/modules/`uname -r`/build

all: modules minesweeper-test

minesweeper-test: minesweeper-test.c
	gcc --std=c99 -Wall -O2 -pthread -o $@ $^ -lm

modules:
	$(MAKE) -C $(KDIR) M=$$PWD $@

clean:
	$(MAKE) -C $(KDIR) M=$$PWD $@
	-rm minesweeper-test

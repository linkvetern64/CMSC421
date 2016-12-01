#!/bin/bash

if [ -f /dev/ms ]; then
    echo "No /dev/ms! Did you insert minesweeper.ko?"
    exit 1
fi

if [ -f /dev/ms_ctl ]; then
    echo "No /dev/ms_ctl! Did you insert minesweeper.ko?"
    exit 1
fi

ROW_SIZE=10

while /bin/true; do
    board=$(cat /dev/ms)
    echo "    0123456789"
    for i in $(seq 0 9); do
	echo "  $i ${board:$(($i * 10)):10}"
    done
    echo -n "Status: "
    cat /dev/ms_ctl
    echo
    read -e -r -p "> " cmd
    if [ -z "$cmd" ]; then
	echo "Commands: (s)tart game, (r)eveal square, (m)ark square, (q)uit."
	continue
    fi
    echo "$cmd" > /dev/ms_ctl
done

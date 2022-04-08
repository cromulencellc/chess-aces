#!/bin/bash

pid=$(ps axeo pid,tty,command | grep vsftpd | grep ? | grep -v grep | cut -f 1 -d " ")

NL=$(echo -en "\n")

if [ "$pid" != "$NL" ]
then
	sudo gdb --command=earnshaw.pov.2.sc -p $pid
fi

if [ "$pid" = "$NL" ]
then
	echo "Did not locate the pid. Is it connected?"
fi


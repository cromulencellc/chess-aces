#!/bin/bash

pid=$(ps aux | grep vsftp | grep "?" | cut -f 6 -d " ")

NL=$(echo -en "\n")

if [ "$pid" != "$NL" ]
then
	sudo gdb --command=earnshaw.sc -p $pid
fi

if [ "$pid" = "$NL" ]
then
	echo "Did not locate the pid. Is it connected?"
fi


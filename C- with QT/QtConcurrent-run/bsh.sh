#!/bin/bash

fl="AKS_main";
thrd=5;
PTH="/home/yaryna/Desktop/";
inpt_file="sec.txt";
outpt_file="HERE.txt";
argv="$*"
COUNTER=0
make
while [  $COUNTER -lt 10 ]; do
	if [ $COUNTER -eq 9 ];
		then ./$fl $thrd $PTH $inpt_file $outpt_file $COUNTER "LAST"
	
	else ./$fl $thrd $PTH $inpt_file $outpt_file $COUNTER "NOT LAST"
	fi
	let COUNTER=COUNTER+1 
done

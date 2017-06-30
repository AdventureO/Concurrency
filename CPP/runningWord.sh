#!/bin/bash

x=1
totalTime=()
countTime=()
count=0


echo -n "Enter number of threads: "
read num


while [ $x -le 5 ]
do
Temp=()
g++ main.cpp -o progRun -lpthread -O3
./progRun $num

	while read -r line
		do
	    		name="$line"
	    		Temp+=($name)

		done < "$1"

x=$(( $x + 1 ))

countTime+=(${Temp[0]})
totalTime+=(${Temp[1]})

done


minTotal=${totalTime[0]}

for i in "${totalTime[@]}"
do
    # Update min if applicable
    if [[ "$i" -lt "$minTotal" ]]; then
        minTotal="$i"
    fi
done
echo Minimum total: $minTotal


minCount=${countTime[0]}
 
for i in "${countTime[@]}"
do
    # Update min if applicable
    if [[ "$i" -lt "$minCount" ]]; then
        minCount="$i"
    fi
done
echo Minimum parallel: $minCount


echo "Number of thread: "$num >> "test.txt"
echo "Minimum parallel:" $minCount >> "test.txt"
echo "Minimum total:" $minTotal >> "test.txt"





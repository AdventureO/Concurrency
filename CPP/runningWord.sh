#!/bin/bash

x=1
Temp=()
totalTime=()
readTime=()
countTime=()
count=0
while [ $x -le 10 ]
do
g++ main.cpp -o progRun -lpthread 
	./progRun
while read -r line
	do
    		name="$line"
    		Temp+=($name)
		
	done < "$1"
x=$(( $x + 1 ))
#echo ${#Temp[@]}

while [ $count -lt ${#Temp[@]} ]
	do
	totalTime+=("${Temp[$count]}")
	readTime+=("${Temp[$count+1]}")
	readTime+=("${Temp[$count+2]}")
	#echo $count
	count=$(( $count + 1 ))
	done

#echo "${totalTime[1]}"
minTotal=${totalTime[0]}
done

# Loop through all elements in the array
for i in "${totalTime[@]}"
do
    # Update min if applicable
    if [[ "$i" -lt "$minTotal" ]]; then
        minTotal="$i"
    fi
done
echo Minimum total: $minTotal

minRead=${totalTime[0]}

# Loop through all elements in the array
for i in "${readTime[@]}"
do
    # Update min if applicable
    if [[ "$i" -lt "$minRead" ]]; then
        minRead="$i"
    fi
done
echo Minimum read: $minRead

minCount=${countTime[0]}

# Loop through all elements in the array
for i in "${countTime[@]}"
do
    # Update min if applicable
    if [[ "$i" -lt "$minCount" ]]; then
        minCount="$i"
    fi
done
echo Minimum count: $minCount

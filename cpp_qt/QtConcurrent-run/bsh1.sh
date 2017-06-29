#alternative bash script with input from terminal
#!/bin/bash

fl="AKS_main";
#thrd=5;
#PTH="/home/yaryna/Desktop/yura/";
#inpt_file="sec.txt";
#outpt_file="HERE.txt";
argv="$*"
COUNTER=0

if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ] || [ -z "$4" ]
  then
    echo "No arguments or not all arguments supplied"
    exit 1
	
fi


make

while getopts 'p:i:o:t:' opt; do #gives an ability to specify the path and the unique number starting point
  case $opt in
    p)
      echo "PATH: " $OPTARG
      PTH=$OPTARG
      echo " -p: Path chosen!" >&2
      ;;
    i)
      inpt_file=$OPTARG
      echo " -i: Input file chosen!" >&2	
      ;;
    o)
      outpt_file=$OPTARG
      echo " -o: Output file chosen!" >&2	
      ;;
    t)
      thrd=$OPTARG
      echo " -t: Thread quantity chosen!" >&2	
      ;;

    \?)
      echo "Invalid option: -$OPTARG" >&2
      ;;
  esac
done

while [  $COUNTER -lt 10 ]; do
	if [ $COUNTER -eq 9 ];
		then ./$fl $thrd $PTH $inpt_file $outpt_file $COUNTER "LAST"
	
	else ./$fl $thrd $PTH $inpt_file $outpt_file $COUNTER "NOT LAST"
	fi
	let COUNTER=COUNTER+1 
done

#!/bin/bash

#In the name of God



#Set maximum memory limit
#MaximumMemoryLimit=$((1 * 1024 * 1024))
#ulimit -v $MaximumMemoryLimit




time ./Modaber $1 $2 $3
echo "*******************************"
for (( i = 1; i < 100000; i++ )){
	echo $i
	if [ -f ${3}.$i ]; then
		Last=${3}.$i
	else
		break;
	fi
}

./validate $1 $2 $Last

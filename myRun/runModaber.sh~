#!/bin/bash

#In the name of God



#Set maximum memory limit
#MaximumMemoryLimit=$((1 * 1024 * 1024))
#ulimit -v $MaximumMemoryLimit




time timeout 30m  ./Modaber $1 $2 $3
echo "*******************************"
./validate $1 $2 $3.*

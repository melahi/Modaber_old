#!/bin/bash

#In the name of God

Algorithm="Evolutionary"
Algorithm="Simple"
NPG="1"

#Set maximum memory limit
MaximumMemoryLimit=$((3 * 1024 * 1024))
ulimit -v $MaximumMemoryLimit




time ./Modaber $1 $2 --algorithm "$Algorithm" --NPG "$NPG"
echo "*******************************"
./validate $1 $2 solution

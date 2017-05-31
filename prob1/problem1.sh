#!/bin/bash
PROGRAM=./problem1

for i in {0..100..10} 
	# Changing Delay time of process 1
	# relative to process 2
do
	echo "The delay is $i"
	for j in {1..1000} 
		# Repeat with same delay time setting
		# in order to find the race condition case
	do
		RESULT1=$($PROGRAM 0 $i)
		if [ $RESULT1 != 0 ]
		then
			echo $RESULT1
		fi
	done
done

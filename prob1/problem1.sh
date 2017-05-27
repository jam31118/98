#!/bin/bash
PROGRAM=./problem1

for i in {0..100..10} 
	# Changing Delay time of process 1
	# relative to process 2
do
	echo "The delay is $i"
	for j in {1..100} 
		# Repeat with same delay time setting
		# in order to find the race condition case
	do
		RESULT=$($PROGRAM 0 $i)
		if [ $RESULT != 0 ]
		then
			echo $RESULT
		fi
	done
done

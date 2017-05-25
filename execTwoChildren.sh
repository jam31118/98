#!/bin/bash
PROGRAM=./twoChildren

for i in {0..100..10}
do
	echo "The delay is $i"
	for j in {1..100}
	do
		#echo "turn == $j"
		RESULT=$($PROGRAM 0 $i)
		#echo $RESULT
		if [ $RESULT != 0 ]
		then
			echo $RESULT
			#echo $RESULT
		fi
	done
done

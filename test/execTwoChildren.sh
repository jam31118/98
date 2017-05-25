#!/bin/bash
PROGRAM=./twoChildren

for i in {0..0}
do
	for j in {0..10000..20}
	do
		#echo $i, $j
		RESULT=$($PROGRAM $i $j)
		#echo $RESULT
		if [ $RESULT != 0 ]
		then
			echo $RESULT
		fi
	done
done

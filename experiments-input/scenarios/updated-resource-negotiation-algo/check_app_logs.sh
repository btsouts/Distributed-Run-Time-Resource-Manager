#!/bin/bash

num_of_apps=$1 # Default, if not specified on command-line.
#echo num of apps is $num_of_apps

base_dir=$2
#echo num is $2

for (( i=0; i<$num_of_apps; i++ ))
do
	fname_num=$base_dir/$i
	fname=$fname_num.txt
	#echo $fname
	tail_out=$(tail -n 1 $fname)
	echo $i:$tail_out
done

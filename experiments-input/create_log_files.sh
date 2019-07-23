#!/bin/bash

num_of_cores=$1 # Default, if not specified on command-line.
#echo num of apps is $num_of_apps

base_dir=$2
#echo num is $2

for (( i=0; i<$num_of_cores; i++ ))
do
	fname_num=$base_dir/log_file_$i
	#fname=$fname_num.txt
	echo $fname_num
	echo '' > $fname_num
done

#!/bin/bash
base_dir=$1
#echo num is $2

for (( i=0; i<48; i++ ))
do
	fname=$base_dir/log_file_$i
	tail_out=$(tail -n 1 $fname)
	echo $i:$tail_out
done

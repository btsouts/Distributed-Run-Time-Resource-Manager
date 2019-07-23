#!/bin/bash

output_file=$1
input_dir=$2

#clear
cat $output_file | grep Init | awk 'NR % 2'
echo ""
cat $output_file | grep Init | awk 'NR % 2' | awk '{print $3}' | awk 'BEGIN { FS = "-" } ; { print $1 }'
echo ""
cat $output_file | grep Init | awk 'NR % 2' | awk '{print $3}' | awk 'BEGIN { FS = "-" } ; { print $2 }'
echo ""
cat $output_file | grep Init | awk 'NR % 2' | awk '{print $3}' | awk 'BEGIN { FS = "-" } ; { print $3 }'

cat $output_file | grep Fin
echo ""
cat $output_file | grep Fin | awk '{print $3}' | awk 'BEGIN { FS = "-" } ; { print $1 }'
echo ""
cat $output_file | grep Fin | awk '{print $3}' | awk 'BEGIN { FS = "-" } ; { print $2 }'
echo ""
cat $output_file | grep Fin | awk '{print $3}' | awk 'BEGIN { FS = "-" } ; { print $3 }'

cat $output_file | grep Mean
cat $output_file | grep Mean | awk '{print $4}'

cat $output_file | grep "message count"
cat $output_file | grep "message count" | awk '{print $5}'

cat $output_file | grep "message size"
cat $output_file | grep "message size" | awk '{print $5}'

cat $output_file | grep "distance"
cat $output_file | grep "distance" | awk '{print $4}'

echo ""
for (( i=1; i<=5; i++ ))
do
	echo apps_broken_$i
./get_times_apps_broken.py 128 $input_dir $i
done

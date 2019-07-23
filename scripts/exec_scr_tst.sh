#!/bin/bash

index_start=$1
num_of_rounds=$2 # Default, if not specified on command-line.
#echo num of apps is $num_of_apps

num_of_apps=$3

res_dir=$4
#echo $num_of_apps
#base_dir=$2
#echo num is $2

for (( i=$index_start; i<=$num_of_rounds; i++ ))
do

	##./clear_app_logs.sh 128 mul_scenaria/8x6_power/app_logs > /dev/null

	fname_power=power_$num_of_apps'_'$res_dir'_'$i.txt
	#echo "fname_power =" 
	echo $fname_power

	fname_power_output=power_output_$num_of_apps'_'$i.txt
        echo $fname_power_output

	time_init=$(date +%H-%M-%S-%N)
	echo "Init time" $time_init
	##./power_tst.sh $fname_power & #power_64_3.txt &
	##power_pid=$!

	##./brccerun -nue 48 -f rc.hosts_48 my_rtrm_power 8x6_power & #> $fname_power_output & #power_output_64_3 &

	##rcce_pid=$!

	##./watchdog.sh my_rtrm_power & #$rcce_pid &
	##watchdog_pid=$!
	
	echo "watchdog_pid " $watchdog_pid

	wait $rcce_pid
	##kill -9 $power_pid
	time_fin=$(date +%H-%M-%S-%N)
	echo "SCC exec completed successfully!"
	
	##kill -9 $watchdog_pid
	echo "Init time" $time_init
	echo "Finish time" $time_fin

	##./get_power_stats.py $fname_power #power_64_3.txt
	#./mul_scenaria/8x6_power/cp_scr.sh $num_of_apps $i
	##cd mul_scenaria/8x6_power/
	##./cp_scr.sh $num_of_apps $i $res_dir #power_rtrm/
	##cd ../../
done

#!/bin/bash

num_of_apps=$1 # Default, if not specified on command-line.
#echo num of apps is $num_of_apps

num=$2
#echo num is $2

dir=$3
#echo dir is $dir

suf=_$num
#echo suf is $suf

suffix=$num_of_apps$suf
#echo suffix is $suffix

cur_app_times=app_times_$suffix
#echo cur_app_times is $cur_app_times 

cur_times_dif=times_dif_$suffix
#echo cur_times_dif is $cur_times_dif

cur_init_ack=init_ack_$suffix

cur_times_log=times_log_$suffix
#echo cur_times_log is $cur_times_log

#cur_patch_file=app_times_patch_$suffix.patch

cur_app_logs=app_logs_$suffix
#echo cur_app_logs is $cur_app_logs

cur_log_files=log_files_$suffix
#echo cur_app_logs is $cur_app_logs

cur_screen_out=screen_out_$suffix

echo "./get_times_apps.py $num_of_apps"
./get_times_apps.py $num_of_apps &
times_pid=$!

./watchdog_pid.sh $times_pid &
watchdog_pid=$!	
echo "watchdog_pid " $watchdog_pid

kill -9 $watchdog_pid

echo "./times_dif_idag.py $num_of_apps"
./times_dif_idag.py $num_of_apps

echo "rm -rv $dir/$cur_app_logs"
rm -rv $dir/$cur_app_logs

echo "cp -r app_logs $dir/$cur_app_logs"
cp -r app_logs $dir/$cur_app_logs

#echo cp -r log_files $dir/$cur_log_files
#cp -r log_files $dir/$cur_log_files

tar czvf $cur_log_files.tar.gz log_files/
echo "mv $cur_log_files.tar.gz $dir/"
mv $cur_log_files.tar.gz $dir/

#echo creating patch
#diff -u app_times.txt $dir/$cur_app_times > $cur_patch_file
#sleep 10

echo "cp times_dif.txt $dir/$cur_times_dif"
cp times_dif.txt $dir/$cur_times_dif

#echo dif2
#diff -u app_times.txt $dir/$cur_app_times

echo "cp init_ack.txt $dir/$cur_init_ack"
cp init_ack.txt $dir/$cur_init_ack

#echo dif3
#diff -u app_times.txt $dir/$cur_app_times

echo "cp times_log.txt $dir/$cur_times_log"
cp times_log.txt $dir/$cur_times_log

echo "cat tmp_power_out > $dir/$cur_screen_out"
cat tmp_power_out > $dir/$cur_screen_out

echo "head -n 1 log_files/log_file_0 >> $dir/$cur_screen_out"
head -n 1 log_files/log_file_0 >> $dir/$cur_screen_out

echo "tail -n 11 log_files/log_file_0 >> $dir/$cur_screen_out"
tail -n 11 log_files/log_file_0 >> $dir/$cur_screen_out

sleep 30

echo "cp app_times.txt $dir/$cur_app_times"
cp -v app_times.txt $dir/$cur_app_times

echo app_times_tail
tail -n 3 app_times.txt
echo ""
#echo dif4
#diff -u app_times.txt $dir/$cur_app_times

#echo ./get_matrix_times.py $num_of_apps
#./get_matrix_times.py $num_of_apps

#echo dif5
#diff -u app_times.txt $dir/$cur_app_times


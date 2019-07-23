#!/bin/bash

index_start=$1
num_of_rounds=$2
num_of_apps=$3
exec_name=$4
scen_dir=$5
scen_name=$6
res_dir=$7
idag_conf=$8
suffix=$9
app_input_file=${10}
app_type=${11}
Selfopt_Radius=${12}
Selfopt_Rounds=${13}

suffix=$suffix'_'$Selfopt_Radius'_'$Selfopt_Rounds

fname_output=$scen_dir/$scen_name/$res_dir/$idag_conf'_'$suffix'_'output.txt
echo $fname_output

for (( i=$index_start; i<=$num_of_rounds; i++ ))
do
	date
	./clear_app_logs.sh 128 $scen_dir/$scen_name/app_logs > /dev/null
	
	fname_power=power_$num_of_apps'_'$res_dir/power'_'$idag_conf'_'$suffix'_'$i.txt
	echo $fname_power

	#fname_power_output=power_output_$num_of_apps'_'$i.txt
        #echo $fname_power_output

	sccBmc -c status

	time_init=$(date +%H-%M-%S-%N)
	echo "Init time" $time_init
	./power_tst.sh $fname_power & #power_64_3.txt &
	power_pid=$!

	./brccerun -nue 48 -f rc.hosts_48 $exec_name -d $scen_dir/ -n $scen_name/ -i $idag_conf -a $app_input_file -p paxos_0.conf -x 1 -y 1 -t $app_type -r $Selfopt_Radius -u $Selfopt_Rounds &

	rcce_pid=$!

	./watchdog.sh $exec_name & #my_rtrm_power & #$rcce_pid &
	watchdog_pid=$!
	
	echo "watchdog_pid " $watchdog_pid

	wait $rcce_pid
	kill -9 $power_pid
	time_fin=$(date +%H-%M-%S-%N)
	echo "SCC exec completed successfully!"
	
	sleep 10s

	if [ -e /proc/$watchdog_pid ] 
	then
		kill -9 $watchdog_pid
		echo "Init time" $time_init
		echo $fname_power > $scen_dir/$scen_name/tmp_power_out
		date >> $scen_dir/$scen_name/tmp_power_out
		echo "Init time" $time_init >> $scen_dir/$scen_name/tmp_power_out 
		echo "Finish time" $time_fin
		echo "Finish time" $time_fin >> $scen_dir/$scen_name/tmp_power_out

		./get_power_stats.py $fname_power >> $scen_dir/$scen_name/tmp_power_out
		#cp_scr.sh has to be in the appropriate directory to find the files to copy. FIXME probably		
		#mul_scenaria/8x6_power/cp_scr.sh $num_of_apps $i $res_dir
		
		cd $scen_dir/$scen_name/

		mkdir $res_dir/$idag_conf'_'$suffix
		./cp_scr.sh $num_of_apps $i $res_dir/$idag_conf'_'$suffix
		cd ../../../		

		##sleep 30s
		##cat current_exec_out.txt >> $fname_output
		##sleep 30s		
	fi

	sleep 10
done

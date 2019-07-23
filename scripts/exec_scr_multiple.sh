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

fname_output=$scen_dir/$scen_name/$res_dir/$idag_conf'_'$suffix'_'output.txt
echo $fname_output

for (( i=$index_start; i<=$num_of_rounds; i++ ))
do
	date
	./clear_app_logs.sh 128 $scen_dir/$scen_name/app_logs > /dev/null
	
	fname_power=power_$res_dir/$exec_name'_'power'_'$idag_conf'_'$suffix'_'$num_of_apps'_apps_'$i.txt
	echo $fname_power

	#fname_power_output=power_output_$num_of_apps'_'$i.txt
        #echo $fname_power_output

	sccBmc -c status

	time_init=$(date +%H-%M-%S-%N)
	echo "Init time" $time_init
	./power_tst.sh $fname_power & #power_64_3.txt &
	power_pid=$!

#pssh -h PSSH_HOST_FILE.30448 -t -1 -P -p 48 /shared/herc/my_rtrm_multiple_apps 48 0.533 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 -d drtrm-multiple-apps/scenaria// -n 8x6_journal// -i idag_conf_2_1 -a 10 -p paxos_0.conf -x 1 -y 1 -t 11 < /dev/nul

#./brccerun -nue 48 -f rc.hosts_48 my_rtrm_multiple_apps -d drtrm-multiple-apps/scenaria/ -n 8x6_journal/ -i idag_conf_2_1 -a app_input_idag_conf_2_1_full_higher.txt -p paxos_0.conf -x 1 -y 1 -t S

#./exec_scr_multiple.sh 1 5 128 my_rtrm_multiple_apps drtrm-multiple-apps/scenaria/ 8x6_journal/ 2_idags idag_conf_2_1 rr app_input_idag_conf_2_1_rr_higher.txt S

	echo "Selfopt radius is 6, Selfopt rounds is 3"
	./brccerun -nue 48 -f rc.hosts_48 $exec_name -d $scen_dir/ -n $scen_name/ -i $idag_conf -a $app_input_file -p paxos_0.conf -x 1 -y 1 -t $app_type -r 6 -u 3 &

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

		mkdir $res_dir/$idag_conf'_'$app_type'_'$suffix
		./cp_scr.sh $num_of_apps $i $res_dir/$idag_conf'_'$app_type'_'$suffix
		cd ../../../		

		##sleep 30s
		##cat current_exec_out.txt >> $fname_output
		##sleep 30s		
	fi

	sleep 10
done

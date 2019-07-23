#!/bin/bash

echo "sleeping...."
#sleep 150m
#date

#orig
#exec_name=$1
#scenario_folder=$2
#scen_name=$3
#idags=$4
#app_type=$5
#intensity=$6

exec_name=$1
scenario_folder=$2
scen_name=$3
idags=$4
app_type=$5
intensity=$6
N_apps=$7
Cpu_freq=$8
init_scen=$9
arrival_rate=${10}

#for intensity in wp_16_4 wp_32_8 wp_48_8 wp_64_8
#do

#for init_scen in 'rr' 'full' # 'max_min0'
#do

if [ -z "$arrival_rate" ]
then

echo './exec_scr_multiple.sh 1 5 '$N_apps' '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_1 '$init_scen'_'$intensity'_'$Cpu_freq' app_input_idag_conf_'$idags'_1_'$init_scen'_'$intensity'_'$app_type'_'$N_apps'apps.txt '$app_type
./exec_scr_multiple.sh 600 600 $N_apps $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_1' $init_scen'_'$intensity'_'$Cpu_freq app_input_idag_conf_$idags'_1_'$init_scen'_'$intensity'_'$app_type'_'$N_apps'apps.txt' $app_type

else

echo './exec_scr_multiple.sh 1 5 '$N_apps' '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_1 '$init_scen'_'$intensity'_'$Cpu_freq' app_input_idag_conf_'$idags'_1_'$init_scen'_'$intensity'_'$arrival_rate'_'$app_type'_'$N_apps'apps.txt '$app_type
./exec_scr_multiple.sh 1 5 $N_apps $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_1' $init_scen'_'$intensity'_'$Cpu_freq app_input_idag_conf_$idags'_1_'$init_scen'_'$intensity'_'$arrival_rate'_'$app_type'_'$N_apps'apps.txt' $app_type

fi

#done

#done

exit
init_scen='rr'

echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_1 '$init_scen' app_input_idag_conf_'$idags'_1_'$init_scen'_'$intensity'.txt '$app_type
./exec_scr_multiple.sh 100 100 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_1' $init_scen app_input_idag_conf_$idags'_1_'$init_scen'_'$intensity'.txt' $app_type
exit
echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_2 '$init_scen' app_input_idag_conf_'$idags'_2_'$init_scen'_higher.txt '$app_type
./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_2' $init_scen app_input_idag_conf_$idags'_2_'$init_scen'_higher.txt' $app_type

echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_3 '$init_scen' app_input_idag_conf_'$idags'_3_'$init_scen'_higher.txt '$app_type
./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_3' $init_scen app_input_idag_conf_$idags'_3_'$init_scen'_higher.txt' $app_type

echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_4 '$init_scen' app_input_idag_conf_'$idags'_4_'$init_scen'_higher.txt '$app_type
./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_4' $init_scen app_input_idag_conf_$idags'_4_'$init_scen'_higher.txt' $app_type

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_5 '$init_scen' app_input_idag_conf_'$idags'_5_'$init_scen'_higher.txt '$app_type
#./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_5' $init_scen app_input_idag_conf_$idags'_5_'$init_scen'_higher.txt' $app_type

#exit
#full
init_scen='full'

echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_1 '$init_scen' app_input_idag_conf_'$idags'_1_'$init_scen'_higher.txt '$app_type
./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_1' $init_scen app_input_idag_conf_$idags'_1_'$init_scen'_higher.txt' $app_type

echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_2 '$init_scen' app_input_idag_conf_'$idags'_2_'$init_scen'_higher.txt '$app_type
./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_2' $init_scen app_input_idag_conf_$idags'_2_'$init_scen'_higher.txt' $app_type

echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_3 '$init_scen' app_input_idag_conf_'$idags'_3_'$init_scen'_higher.txt '$app_type
./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_3' $init_scen app_input_idag_conf_$idags'_3_'$init_scen'_higher.txt' $app_type

echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_4 '$init_scen' app_input_idag_conf_'$idags'_4_'$init_scen'_higher.txt '$app_type
./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_4' $init_scen app_input_idag_conf_$idags'_4_'$init_scen'_higher.txt' $app_type

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_5 '$init_scen' app_input_idag_conf_'$idags'_5_'$init_scen'_higher.txt '$app_type
#./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_5' $init_scen app_input_idag_conf_$idags'_5_'$init_scen'_higher.txt' $app_type

#max_dist
init_scen='max_dist'

echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_1 '$init_scen' app_input_idag_conf_'$idags'_1_'$init_scen'_higher.txt '$app_type
./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_1' $init_scen app_input_idag_conf_$idags'_1_'$init_scen'_higher.txt' $app_type

echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_2 '$init_scen' app_input_idag_conf_'$idags'_2_'$init_scen'_higher.txt '$app_type
./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_2' $init_scen app_input_idag_conf_$idags'_2_'$init_scen'_higher.txt' $app_type

echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_3 '$init_scen' app_input_idag_conf_'$idags'_3_'$init_scen'_higher.txt '$app_type
./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_3' $init_scen app_input_idag_conf_$idags'_3_'$init_scen'_higher.txt' $app_type

echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_4 '$init_scen' app_input_idag_conf_'$idags'_4_'$init_scen'_higher.txt '$app_type
./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_4' $init_scen app_input_idag_conf_$idags'_4_'$init_scen'_higher.txt' $app_type

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_5 '$init_scen' app_input_idag_conf_'$idags'_5_'$init_scen'_higher.txt '$app_type
#./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_5' $init_scen app_input_idag_conf_$idags'_5_'$init_scen'_higher.txt' $app_type

#random cluster

#echo "sleeping...."
#sleep 540m
#date

#cd $scenario_folder
#echo "cp inputs/app_input_idag_conf_2_1_rc_higher.txt app_input.txt"
#cp inputs/app_input_idag_conf_2_1_rc_higher.txt app_input.txt
#cd ../../../
#./exec_scr.sh 1 5 128 my_rtrm 8x6_journal 2_idags idag_conf_2_1 rc_higher

#echo './exec_scr_multiple.sh 5 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_4 rr app_input_idag_conf_'$idags'_4_rr_higher.txt '$app_type
#./exec_scr_multiple.sh 5 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_4' rr app_input_idag_conf_$idags'_4_rr_higher.txt' $app_type


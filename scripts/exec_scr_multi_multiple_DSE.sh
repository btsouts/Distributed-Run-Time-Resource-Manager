#!/bin/bash

echo "sleeping...."
#sleep 150m
#date
exec_name=$1
scenario_folder=$2
scen_name=$3
idags=$4
app_type=$5
workld_mul=$6

init_scen='max_min0'

#Selfopt_Radius=2
#Selfopt_Rounds=3

for (( Selfopt_Radius=9; Selfopt_Radius<=9; Selfopt_Radius++ )) #10
do


for (( Selfopt_Rounds=1; Selfopt_Rounds<=3; Selfopt_Rounds++ )) #6
do
#Selfopt_Rounds=2

if [ "$Selfopt_Radius" -eq "$Selfopt_Rounds" ]
then
	continue
fi

if [ "$Selfopt_Rounds" -eq "2" ]
then
        continue
fi

#echo 'Selfopt_Radius is '$Selfopt_Radius' Selfopt_Rounds is '$Selfopt_Rounds

echo './exec_scr_multiple.sh 1 3 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_1 '$init_scen' app_input_idag_conf_'$idags'_1_'$init_scen'_higher_'$workld_mul'x.txt '$app_type' '$Selfopt_Radius' '$Selfopt_Rounds
#./exec_scr_multiple_DSE.sh 1 3 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_1' $init_scen app_input_idag_conf_$idags'_1_'$init_scen'_higher_'$workld_mul'x.txt' $app_type $Selfopt_Radius $Selfopt_Rounds

done

done

exit
#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_2 '$init_scen' app_input_idag_conf_'$idags'_2_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 11 12 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_2' $init_scen app_input_idag_conf_$idags'_2_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_3 '$init_scen' app_input_idag_conf_'$idags'_3_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 11 11 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_3' $init_scen app_input_idag_conf_$idags'_3_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_4 '$init_scen' app_input_idag_conf_'$idags'_4_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 20 20 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_4' $init_scen app_input_idag_conf_$idags'_4_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_5 '$init_scen' app_input_idag_conf_'$idags'_5_'$init_scen'_higher.txt '$app_type
#./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_5' $init_scen app_input_idag_conf_$idags'_5_'$init_scen'_higher.txt' $app_type

#exit
#full
init_scen='full2'

echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_1 '$init_scen' app_input_idag_conf_'$idags'_1_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
./exec_scr_multiple.sh 100 104 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_1' $init_scen app_input_idag_conf_$idags'_1_'$init_scen'_higher_'$workld_mul'x.txt' $app_type
#exit
echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_2 '$init_scen' app_input_idag_conf_'$idags'_2_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
./exec_scr_multiple.sh 100 104 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_2' $init_scen app_input_idag_conf_$idags'_2_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_3 '$init_scen' app_input_idag_conf_'$idags'_3_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
./exec_scr_multiple.sh 100 104 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_3' $init_scen app_input_idag_conf_$idags'_3_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_4 '$init_scen' app_input_idag_conf_'$idags'_4_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
./exec_scr_multiple.sh 100 104 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_4' $init_scen app_input_idag_conf_$idags'_4_'$init_scen'_higher_'$workld_mul'x.txt' $app_type
exit
#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_5 '$init_scen' app_input_idag_conf_'$idags'_5_'$init_scen'_higher.txt '$app_type
#./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_5' $init_scen app_input_idag_conf_$idags'_5_'$init_scen'_higher.txt' $app_type
#exit
#max_dist
init_scen='max_dist'

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_1 '$init_scen' app_input_idag_conf_'$idags'_1_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 12 12 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_1' $init_scen app_input_idag_conf_$idags'_1_'$init_scen'_higher_'$workld_mul'x.txt' $app_type
#exit
#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_2 '$init_scen' app_input_idag_conf_'$idags'_2_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 12 12 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_2' $init_scen app_input_idag_conf_$idags'_2_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_3 '$init_scen' app_input_idag_conf_'$idags'_3_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 14 14 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_3' $init_scen app_input_idag_conf_$idags'_3_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_4 '$init_scen' app_input_idag_conf_'$idags'_4_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 20 20 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_4' $init_scen app_input_idag_conf_$idags'_4_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_5 '$init_scen' app_input_idag_conf_'$idags'_5_'$init_scen'_higher.txt '$app_type
#./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_5' $init_scen app_input_idag_conf_$idags'_5_'$init_scen'_higher.txt' $app_type

init_scen='max_all'

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_1 '$init_scen' app_input_idag_conf_'$idags'_1_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 100 104 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_1' $init_scen app_input_idag_conf_$idags'_1_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_2 '$init_scen' app_input_idag_conf_'$idags'_2_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 13 13 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_2' $init_scen app_input_idag_conf_$idags'_2_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_3 '$init_scen' app_input_idag_conf_'$idags'_3_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 10 10 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_3' $init_scen app_input_idag_conf_$idags'_3_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_4 '$init_scen' app_input_idag_conf_'$idags'_4_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 10 14 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_4' $init_scen app_input_idag_conf_$idags'_4_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_5 '$init_scen' app_input_idag_conf_'$idags'_5_'$init_scen'_higher.txt '$app_type
#./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_5' $init_scen app_input_idag_conf_$idags'_5_'$init_scen'_higher.txt' $app_type

#init_scen='full_false'

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_1 '$init_scen' app_input_idag_conf_'$idags'_1_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 100 104 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_1' $init_scen app_input_idag_conf_$idags'_1_'$init_scen'_higher_'$workld_mul'x.txt' $app_type
#exit
#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_2 '$init_scen' app_input_idag_conf_'$idags'_2_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 100 104 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_2' $init_scen app_input_idag_conf_$idags'_2_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_3 '$init_scen' app_input_idag_conf_'$idags'_3_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 100 104 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_3' $init_scen app_input_idag_conf_$idags'_3_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_4 '$init_scen' app_input_idag_conf_'$idags'_4_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 100 104 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_4' $init_scen app_input_idag_conf_$idags'_4_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

init_scen='max_min'

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_1 '$init_scen' app_input_idag_conf_'$idags'_1_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 100 104 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_1' $init_scen app_input_idag_conf_$idags'_1_'$init_scen'_higher_'$workld_mul'x.txt' $app_type
#exit
#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_2 '$init_scen' app_input_idag_conf_'$idags'_2_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 100 104 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_2' $init_scen app_input_idag_conf_$idags'_2_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_3 '$init_scen' app_input_idag_conf_'$idags'_3_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 100 104 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_3' $init_scen app_input_idag_conf_$idags'_3_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_4 '$init_scen' app_input_idag_conf_'$idags'_4_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
#./exec_scr_multiple.sh 100 104 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_4' $init_scen app_input_idag_conf_$idags'_4_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

#echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_5 '$init_scen' app_input_idag_conf_'$idags'_5_'$init_scen'_higher.txt '$app_type
#./exec_scr_multiple.sh 1 5 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_5' $init_scen app_input_idag_conf_$idags'_5_'$init_scen'_higher.txt' $app_type

init_scen='max_min0'

echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_1 '$init_scen' app_input_idag_conf_'$idags'_1_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
./exec_scr_multiple.sh 100 104 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_1' $init_scen app_input_idag_conf_$idags'_1_'$init_scen'_higher_'$workld_mul'x.txt' $app_type
#exit
echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_2 '$init_scen' app_input_idag_conf_'$idags'_2_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
./exec_scr_multiple.sh 100 104 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_2' $init_scen app_input_idag_conf_$idags'_2_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_3 '$init_scen' app_input_idag_conf_'$idags'_3_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
./exec_scr_multiple.sh 100 104 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_3' $init_scen app_input_idag_conf_$idags'_3_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

echo './exec_scr_multiple.sh 1 5 128 '$exec_name' '$scenario_folder' '$scen_name $idags'_idags idag_conf_'$idags'_4 '$init_scen' app_input_idag_conf_'$idags'_4_'$init_scen'_higher_'$workld_mul'x.txt '$app_type
./exec_scr_multiple.sh 100 104 128 $exec_name $scenario_folder $scen_name $idags'_idags' idag_conf_$idags'_4' $init_scen app_input_idag_conf_$idags'_4_'$init_scen'_higher_'$workld_mul'x.txt' $app_type

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


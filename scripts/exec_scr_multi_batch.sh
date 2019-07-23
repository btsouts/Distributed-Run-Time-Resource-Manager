#!/bin/bash
echo "./exec_scr_multi_multiple_xworkload.sh my_rtrm_multiple_apps_fft drtrm-multiple-apps/scenaria/ 8x6_updated_offer_fast 4 F 0.75"
./exec_scr_multi_multiple_xworkload.sh my_rtrm_multiple_apps_fft drtrm-multiple-apps/scenaria/ 8x6_updated_offer_fast 4 F 0.75

echo "./exec_scr_multi_multiple_xworkload.sh my_rtrm_multiple_apps_fft drtrm-multiple-apps/scenaria/ 8x6_updated_offer_fast 6 F 0.75"
./exec_scr_multi_multiple_xworkload.sh my_rtrm_multiple_apps_fft drtrm-multiple-apps/scenaria/ 8x6_updated_offer_fast 6 F 0.75

echo "./exec_scr_multi_multiple_xworkload.sh my_rtrm_multiple_apps_fft drtrm-multiple-apps/scenaria/ 8x6_updated_offer_fast 8 F 0.75"
./exec_scr_multi_multiple_xworkload.sh my_rtrm_multiple_apps_fft drtrm-multiple-apps/scenaria/ 8x6_updated_offer_fast 8 F 0.75

#echo ./tst_exec_scr.sh 1 5 128 my_rtrm_power_6idags_3rdconf 6_idags/power_rtrm_3rd_idag_conf
#./tst_exec_scr.sh 1 5 128 my_rtrm_power_6idags_3rdconf 6_idags/power_rtrm_3rd_idag_conf | tee test_current_exec_out.txt

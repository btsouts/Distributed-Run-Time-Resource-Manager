#!/bin/bash

exec_name=$1

echo "Watchdog started! pid " $exec_name

sleep 8m
echo "Watchdog expired! Killing"
#./killit $exec_name
#./killit my_rtrm_power
pssh -P -h allhosts -p 48 -t -1 /shared/herc/killcorePIDs $exec_name < /dev/null
kill_pid=$!
#echo "i come here"
sleep 2
wait $kill_pid

#!/bin/bash

pid=$1

echo "Watchdog started! pid " $pid

sleep 1m
echo "Watchdog expired! Killing "$pid
kill -9 $pid


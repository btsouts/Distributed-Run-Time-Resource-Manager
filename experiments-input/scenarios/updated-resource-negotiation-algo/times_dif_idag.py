#!/usr/bin/env python

import sys, os, stat

num_of_apps = int(sys.argv[1]) 
MS = 1000000
out_filename = './times_dif.txt' 
fd_w = open(out_filename, 'w')

input_time_log = './times_log.txt' 
fd_r = open(input_time_log, 'r')

one_line = fd_r.readline().split()
init_time = one_line[0].strip('[]')
write_line = init_time  + '\n'
fd_w.write(write_line)
time_m = init_time.split(':')

for i in range(1,num_of_apps):
	
	one_line = fd_r.readline().split()
	init_node = one_line[0].strip('[]')
	
	time_m2 = init_node.split(':')
	dif = ((int(time_m2[0]) - int(time_m[0])) * 3600 * MS) + ((int(time_m2[1]) - int(time_m[1])) * 60 * MS) + ((int(time_m2[2]) - int(time_m[2]))  * MS) + ((int(time_m2[3]) - int(time_m[3])))
	dif_ms = dif / 10000;
	write_line = init_node + ' ' + str(dif) + ' ' +str(dif_ms) + '\n'
	fd_w.write(write_line)
	
fd_r.close()
fd_w.close()


#!/usr/bin/env python

import sys, os, stat, random

num_of_apps = int(sys.argv[1]) 

#app_input_idag_conf_1_1_full_higher.txt

input_filename = sys.argv[2]
fd_r = open(input_filename, 'r')

input_init_areas = sys.argv[3]
fd_ro = open(input_init_areas, 'r')

suffix = sys.argv[4]

#input_filename_parts = input_filename.split('_')
out_filename = input_filename[0:24] + suffix + '.txt'
print out_filename
 
fd_w = open(out_filename, 'w')

for i in range(0,num_of_apps):
	one_line = fd_r.readline().split()

	one_workload_value = fd_ro.readline().split()

	write_line = str(one_line[0]) + ' ' + str(one_line[1]) + ' ' + str(one_line[2]) + ' ' + str(one_workload_value[3]) + '\n'
	fd_w.write(write_line)

fd_r.close()
	
fd_w.close()
#print len(list_of_clusters)

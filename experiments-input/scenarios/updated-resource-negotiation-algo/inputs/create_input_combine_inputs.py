#!/usr/bin/env python

import sys, os, stat, random

num_of_apps = int(sys.argv[1]) 
app_type = sys.argv[2]

input_arrival = sys.argv[3]
fd_arr = open(input_arrival, 'r')

input_initials = sys.argv[4]
fd_init = open(input_initials, 'r')

input_apps = sys.argv[5]
fd_apps = open(input_apps, 'r')

input_workld = sys.argv[6]
fd_workld = open(input_workld, 'r')

out_filename = sys.argv[7]
fd_w = open(out_filename, 'w')

#tmp_str = input_filename.split('.')
#out_filename = tmp_str[0]
#for i in range(1,len(tmp_str)-1):
#	out_filename += '.' + tmp_str[i] 

#out_filename += '_' + suffix + '.txt'
#print out_filename

for i in range(0,num_of_apps):
	one_arrival = fd_arr.readline().split()

	one_initial = fd_init.readline().split()

	one_app = fd_apps.readline().split()

	one_workld = fd_workld.readline().split()

	if app_type == 'A':
		write_line = str(one_arrival[0]) + ' ' + str(one_initial[0]) + ' ' + str(one_app[2]) + ' ' + str(one_app[3]) + ' ' + str(one_workld[0]) + '\n'	
	else:
		write_line = str(one_arrival[0]) + ' ' + str(one_initial[0]) + ' ' + str(one_app[2]) + ' ' + str(one_workld[0]) + '\n'	
	
	fd_w.write(write_line)

fd_arr.close()
fd_init.close()
fd_apps.close()
fd_workld.close()

fd_w.close()
#print len(list_of_clusters)

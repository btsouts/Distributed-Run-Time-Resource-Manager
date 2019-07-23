#!/usr/bin/env python

import sys, os, stat, random

input_filename = sys.argv[1]
fd_r = open(input_filename, 'r')

#app_input_idag_conf_2_A_rr_wp32_fast_A_80apps.txt
#app_input_idag_conf_2_A_rr_wp16_fastM_128apps.txt
out_filename = input_filename[0:37] + 'M' + input_filename[38:]
print out_filename
 
fd_w = open(out_filename, 'w')

one_line = fd_r.readline().split()
while (len(one_line) != 0):
	write_line = str(one_line[0]) + ' ' + str(one_line[1]) + ' 4096  ' + str(one_line[4]) + '\n'
	fd_w.write(write_line)

	one_line = fd_r.readline().split()

fd_r.close()
	
fd_w.close()
#print len(list_of_clusters)

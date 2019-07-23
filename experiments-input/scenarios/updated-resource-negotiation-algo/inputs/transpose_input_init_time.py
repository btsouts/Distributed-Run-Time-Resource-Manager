#!/usr/bin/env python

import sys, os, stat, random

num_of_apps = int(sys.argv[1]) 

input_filename = sys.argv[2]
fd_r = open(input_filename, 'r')

input_ortrm = sys.argv[3]
fd_ro = open(input_ortrm, 'r')

suffix = sys.argv[4]

tmp_str = input_filename.split('.')

out_filename = tmp_str[0]
for i in range(1,len(tmp_str)-1):
	out_filename += '.' + tmp_str[i] 

#out_filename = input_filename.split('.')[0:1] + '_' + suffix + '.txt' 
out_filename += '_' + suffix + '.txt'
print out_filename
fd_w = open(out_filename, 'w')

for i in range(0,num_of_apps):
	one_line = fd_r.readline().split()

	one_ortrm = fd_ro.readline().split()

	write_line = str(one_ortrm[0]) + ' ' + str(one_line[1]) + ' ' + str(one_line[2]) + ' ' + str(one_line[3]) + '\n'
	fd_w.write(write_line)

fd_r.close()
	
fd_w.close()
#print len(list_of_clusters)

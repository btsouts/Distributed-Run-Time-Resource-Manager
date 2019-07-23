#!/usr/bin/env python

import sys, os, stat, random

num_of_apps = int(sys.argv[1]) 

input_filename = sys.argv[2]
fd_r = open(input_filename, 'r')

multiplier = sys.argv[3]
mult = float(multiplier)
suffix = multiplier + 'x'

out_filename = input_filename.split('.')[0] + '_' + suffix + '.txt'
print out_filename 
fd_w = open(out_filename, 'w')

for i in range(0,num_of_apps):
	one_line = fd_r.readline().split()

	write_line = str(one_line[0]) + ' ' + str(one_line[1]) + ' ' + str(one_line[2]) + ' ' + str(int(float(one_line[3])*mult)) + '\n'
	fd_w.write(write_line)

fd_r.close()	
fd_w.close()
#print len(list_of_clusters)

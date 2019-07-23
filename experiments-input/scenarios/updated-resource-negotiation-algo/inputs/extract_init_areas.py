#!/usr/bin/env python

import sys, os, stat, random

num_of_apps = int(sys.argv[1]) 

input_init_areas = sys.argv[2]
fd_init = open(input_init_areas, 'r')

output_init_areas = sys.argv[3]
fd_out = open(output_init_areas, 'w')

for i in range(0,num_of_apps):
	one_line = fd_init.readline().split()

	write_line = str(one_line[1]) + '\n'
	fd_out.write(write_line)

fd_init.close()
fd_out.close()
#print len(list_of_clusters)

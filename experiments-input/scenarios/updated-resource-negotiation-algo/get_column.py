#!/usr/bin/env python

import sys, os, stat, random

input_filename = (sys.argv[1])
fd_r = open(input_filename, 'r')

#out_filename = './app_input_big_4096_medium_cross.txt' 
#fd_w = open(out_filename, 'w')
column = int(sys.argv[2])

one_line = fd_r.readline().split()

while (len(one_line) != 0):
	print str(one_line[column])
	one_line = fd_r.readline().split()
	
fd_r.close()	
#fd_w.close()
#print len(list_of_clusters)

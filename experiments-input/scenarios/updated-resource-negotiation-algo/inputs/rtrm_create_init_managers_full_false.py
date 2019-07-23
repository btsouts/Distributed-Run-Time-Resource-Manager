#!/usr/bin/env python

import sys, os, stat, random

X_max = 6
Y_max = 8

num_of_apps = int(sys.argv[1])
idag_conf_filename = sys.argv[2]
out_filename = idag_conf_filename + '_rand_full_false_areas'
#regural grids only

fd_idags = open(idag_conf_filename, 'r')
num_idags = int(fd_idags.readline())

list_of_clusters = []
idags = []
avail_cores = X_max * Y_max - num_idags
print 'available cores = ' + str(avail_cores)
print 'out_filename = ' + out_filename

for i in range(0,num_idags):
	idags.append(int(fd_idags.readline()))

cores = []
for i in range (0,X_max*Y_max):
	if i not in idags:
		cores.append(i) 

fd_w = open(out_filename, 'w')

print cores
pos = 0
for i in range(0,num_of_apps):
	core_id = cores.pop(0)
	#print core_id

	write_line = str(core_id) + '\n'
	fd_w.write(write_line)

 	if cores == []:
		for i in range (0,X_max*Y_max):
        		if i not in idags:
                		cores.append(i)

#fd_r.close()
fd_idags.close()	
fd_w.close()

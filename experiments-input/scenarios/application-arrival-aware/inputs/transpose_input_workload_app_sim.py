#!/usr/bin/env python

import sys, os, stat, random, math, numpy

N_req_params = 3
#app_input_idag_conf_1_1_rr_max_fast_A_1apps.txt
num_of_apps = int(sys.argv[1]) 

input_filename = sys.argv[2]
fd_r = open(input_filename, 'r')

input_workld = sys.argv[3]
fd_workld = open(input_workld, 'r')
#suffix = sys.argv[4]

if len(sys.argv) < (N_req_params+2):
	suffix = '_A'
	out_filename = input_filename[0:35] + suffix + '_' + str(num_of_apps) + 'apps.txt'	
	
	#tmp_str = input_filename.split('.')
	#out_filename = tmp_str[0]
	#for i in range(1,len(tmp_str)-1):
	#	out_filename += '.' + tmp_str[i] 

	##out_filename = input_filename.split('.')[0:1] + '_' + suffix + '.txt' 
	#out_filename += '_' + suffix + '.txt'
else:
	out_filename = sys.argv[N_req_params+1]

print out_filename
fd_w = open(out_filename, 'w')

for i in range(0,num_of_apps):
	one_line = fd_r.readline().split()
	one_workld = fd_workld.readline().split()
	write_line = str(one_line[0]) + ' ' + str(one_line[1]) + ' ' + str(one_line[2]) + ' ' + str(one_line[3]) + ' ' + str(one_workld[0]) + '\n'	
	
	#print str(one_line[0]) 
	#print str(one_line[1]) 
	#print str(Var_list_perm[i])
	#print str(int(math.ceil(random.uniform(A_min,A_max)))) 
	#print str(one_workld[0])
 
	fd_w.write(write_line)
	#print write_line

fd_workld.close()
fd_r.close()
fd_w.close()

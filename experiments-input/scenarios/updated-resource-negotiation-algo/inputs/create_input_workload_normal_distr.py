#!/usr/bin/env python

import sys, os, stat, random, math

num_of_apps = int(sys.argv[1])
mu = int(sys.argv[2])
sigma = int(sys.argv[3])
 
out_filename = 'workload_wp_' + str(mu) + '_' + str(sigma) + '_' + str(num_of_apps) + 'apps.txt'
print 'out_filename is ' + out_filename
fd_w = open(out_filename, 'w')

for i in range(0,num_of_apps):
	one_workld = int(math.ceil(random.normalvariate(mu, sigma)))
	fd_w.write(str(one_workld) + '\n')

fd_w.close()

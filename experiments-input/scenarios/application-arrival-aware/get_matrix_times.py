#!/usr/bin/env python

import sys, os, stat

num_of_apps = int(sys.argv[1]) 
MS = 1000000
mean_idag = 0
mean_init = 0
mean_agent = 0

in_app_filename = './app_input.txt'
fd_app = open(in_app_filename, 'r')

times_dir_base = './matrix_times/'


for i in range(0,num_of_apps):
	input_filename = './app_logs/' + str(i) + '.txt' 
	fd_r = open(input_filename, 'r')

	one_line = fd_app.readline().split()
	array_size = one_line[2]
	workld = int(one_line[3])
	
	print str(i) + ' ' + array_size
	times_dir = times_dir_base + array_size + '/'
	#[5:13:26:477789]: I am agent 31 . Init ok!! my_cores_count = 5 array_size = 2048
	one_line_app = fd_r.readline()
	one_line_app = fd_r.readline()
	one_line_app = fd_r.readline()
	app_line = one_line_app.split()
	
	active_cores = str(int(app_line[10]) - 1)
	if app_line[13] != array_size:
		print 'error ' + array_size + ' ' + app_line[13] + '\n'
	
	if int(active_cores) == 0:
		while True:
			one_line_app = fd_r.readline() 	
			if one_line_app.find('Self opt ok') != -1:
				app_line = one_line_app.split()
				active_cores = str(int(app_line[len(app_line) - 1]) - 1)
				break
		
	while workld > 0 :
		out_filename = times_dir + active_cores + '.txt' 
		fd_w = open(out_filename, 'a')
		
		#first one will always be init	
		while True:
			one_line_app = fd_r.readline() 	
			if one_line_app.find('work to') != -1:
				app_line = one_line_app.split()
				init_work = app_line[0].strip('[]') 
				break
				
		while True:
			one_line_app = fd_r.readline() 	
			if one_line_app.find('atrix mul') != -1:
				app_line = one_line_app.split()
				finish_work = app_line[0].strip('[]') 
				break			
		
		workld -= 1;
		if workld > 0:
			#[5:13:28:341226] A matrix mul is over. Remaining workload is 3 active cores = 7
			active_cores = app_line[len(app_line) - 1]		

		#while True:
		#	one_line_app = fd_r.readline() 	
		#	if one_line_app.find('Actual finish') != -1:
		#		app_line = one_line_app.split()
		#		actual_finish_work = app_line[0].strip('[]') 
		#		break		

		#out_str = active_cores + ' -> ' + init_work + ' - ' + finish_work + ' = '
		time_m = init_work.split(':')
		time_m2 = finish_work.split(':')
		dif = ((int(time_m2[0]) - int(time_m[0])) * 3600 * MS) + ((int(time_m2[1]) - int(time_m[1])) * 60 * MS) + ((int(time_m2[2]) - int(time_m[2]))  * MS) + ((int(time_m2[3]) - int(time_m[3])))
		if dif < 0:
			print 'gamhthhke'
		#out_str += str(dif)# + '\n'
		#print out_str
		
		write_line= str(dif) + '\n'
		fd_w.write(write_line)
		fd_w.close()
			
	fd_r.close()
	#print '\n'
	
#write_line = str(mean_idag) + '\n' + str(mean_init) + '\n' + str(mean_agent) + '\n'
#fd_w.write(write_line)
#fd_w.close()
fd_app.close()

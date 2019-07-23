#!/usr/bin/env python

import sys, os, stat

num_of_apps = int(sys.argv[1]) 
MS = 1000000
mean_idag = 0
mean_init = 0
mean_agent = 0
#out_filename = './app_times.txt' 
#fd_w = open(out_filename, 'w')

work_dir = sys.argv[2]
scen_num = sys.argv[3]

input_time_log = work_dir + 'times_log_' + str(num_of_apps) + '_' + scen_num#sys.argv[2]#'./times_log.txt' 
fd_rt = open(input_time_log, 'r')

init_ack_log = work_dir + 'init_ack_' + str(num_of_apps) + '_' + scen_num#sys.argv[3]#'./init_ack.txt'
fd_r_ack = open(init_ack_log, 'r')

app_logs_dir = work_dir + 'app_logs_' + str(num_of_apps) + '_' + scen_num + '/'#sys.argv[4]

for i in range(0,num_of_apps):
	input_filename =  app_logs_dir + str(i) + '.txt' 
	fd_r = open(input_filename, 'r')

	one_line = fd_rt.readline().split()
	init_idag = one_line[0].strip('[]')
	
	one_line = fd_r_ack.readline().split()
        init_ack = one_line[0]

	one_line = fd_r.readline().split()
	init_start = one_line[4].strip('[]')

	one_line = fd_r.readline().split()
	init_fin = one_line[4].strip('[]')

	one_line = fd_r.readline().split()
	while one_line != []:

		#print one_line
		if one_line[0].find('[') != -1:
			if one_line[1].find('Agent') != -1:
				agent_start = one_line[0].strip('[]')
		 	elif one_line[1].find('App') != -1:
				agent_fin = one_line[0].strip('[]:')
	
		one_line = fd_r.readline().split()

	#write_line = init_idag + ' - ' +  init_ack + '\n'
	#fd_w.write(write_line)

	time_m = init_idag.split(':')
	time_m2 = init_ack.split(':')
	dif = ((int(time_m2[0]) - int(time_m[0])) * 3600 * MS) + ((int(time_m2[1]) - int(time_m[1])) * 60 * MS) + ((int(time_m2[2]) - int(time_m[2]))  * MS) + ((int(time_m2[3]) - int(time_m[3])))
	mean_idag += dif
	#write_line= str(dif) + '\n'
	#fd_w.write(write_line)

	#write_line = init_start + ' - ' +  init_fin + '\n'
	#fd_w.write(write_line)

	time_m = init_start.split(':')
	time_m2 = init_fin.split(':')
	dif = ((int(time_m2[0]) - int(time_m[0])) * 3600 * MS) + ((int(time_m2[1]) - int(time_m[1])) * 60 * MS) + ((int(time_m2[2]) - int(time_m[2]))  * MS) + ((int(time_m2[3]) - int(time_m[3])))
	mean_init += dif
	#write_line= str(dif) + '\n'
	#fd_w.write(write_line)

	#write_line = agent_start + ' - ' +  agent_fin + '\n'
	#fd_w.write(write_line)
	time_m = agent_start.split(':')
	time_m2 = agent_fin.split(':')
	dif = ((int(time_m2[0]) - int(time_m[0])) * 3600 * MS) + ((int(time_m2[1]) - int(time_m[1])) * 60 * MS) + ((int(time_m2[2]) - int(time_m[2]))  * MS) + ((int(time_m2[3]) - int(time_m[3])))
	mean_agent += dif
	#write_line= str(dif) + '\n\n'
	#fd_w.write(write_line)

	fd_r.close()

mean_idag = mean_idag / num_of_apps
mean_init = mean_init / num_of_apps
mean_agent = mean_agent / num_of_apps

#write_line = str(mean_idag) + '\n' + str(mean_init) + '\n' + str(mean_agent) + '\n'
#fd_w.write(write_line)
#fd_w.close()
print str(mean_idag)
print str(mean_init)
print str(mean_agent)

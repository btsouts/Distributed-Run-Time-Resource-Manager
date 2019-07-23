#!/usr/bin/env python

import sys, os, stat

app_start = int(sys.argv[1])
apps = int(sys.argv[2])
num_of_apps = int(sys.argv[3]) 
MS = 1000000
mean_agent = 0
#out_filename = './app_times.txt' 
#fd_w = open(out_filename, 'w')

work_dir = sys.argv[4]
scen_num = sys.argv[5]

workld = int(sys.argv[6])

app_logs_dir = work_dir + 'app_logs_' + str(num_of_apps) + '_' + scen_num + '/'#sys.argv[4]

worker_time_array = [0, 0, 0, 0, 0, 0, 0]
worker_num_array = [0, 0, 0, 0, 0, 0, 0]

for i in range(app_start,app_start + apps):
	input_filename =  app_logs_dir + str(i) + '.txt' 
	fd_r = open(input_filename, 'r')

	#[16:12:12:509177] A matrix mul is over. Remaining workload is 40 active cores = 2 //14
	#[16:12:12:866558] A matrix mul is over in resizing. Remaining workload is 39 active cores = 3

	exec_list = []

	one_line = fd_r.readline().split()
	iter_cores = 0
	while one_line != []:

		if one_line[1] == 'A':
			iter_fin = one_line[0].strip('[]')
			
			if iter_cores > 0: #Not first time
	
				time_m = iter_start.split(':')
				time_m2 = iter_fin.split(':')
				dif = ((int(time_m2[0]) - int(time_m[0])) * 3600 * MS) + ((int(time_m2[1]) - int(time_m[1])) * 60 * MS) + ((int(time_m2[2]) - int(time_m[2]))  * MS) + ((int(time_m2[3]) - int(time_m[3])))					

				if (dif > 0):
					exec_list.append([iter_cores,dif])

			if len(one_line) == 14:
				iter_cores = int(one_line[13])
			else:
				iter_cores = int(one_line[15])
	
			iter_start = iter_fin

		elif one_line[1] == 'Matrix':
			iter_fin = one_line[0].strip('[]')

			time_m = iter_start.split(':')
			time_m2 = iter_fin.split(':')
			dif = ((int(time_m2[0]) - int(time_m[0])) * 3600 * MS) + ((int(time_m2[1]) - int(time_m[1])) * 60 * MS) + ((int(time_m2[2]) - int(time_m[2]))  * MS) + ((int(time_m2[3]) - int(time_m[3])))					

			if (dif > 0):
				exec_list.append([iter_cores,dif])		 

		one_line = fd_r.readline().split()

	fd_r.close()

	#for i in exec_list:
	#	print str(i[0]) + ' -> ' + str(i[1])

	#print '\n'

	if workld > 1:

		prev_workers = -1
		cur_rep = 0
		for i in exec_list:
			if cur_rep == 0:
				workers = i[0]
				cur_rep = 1
				cur_time = i[1]	
			else: 			
				prev_workers = workers
				workers = i[0]

				if prev_workers == workers:
					cur_rep += 1			
					cur_time += i[1]

					if cur_rep == workld:
						worker_time_array[workers-1] += cur_time 
						worker_num_array[workers-1] += 1 #+= workld

						#prev_workers = -1
						cur_rep = 0
				else:
					cur_rep = 1			
					cur_time = i[1]
	else:
		for i in exec_list:
			workers = i[0]
			cur_time = i[1]
			worker_time_array[workers-1] += cur_time 
			worker_num_array[workers-1] += 1						 	
	
print_str = ''		
for i in range(0,7):
	if worker_num_array[i] > 0:
		avg_time = worker_time_array[i] / worker_num_array[i] #* workld)
	else:
		avg_time = 0

	print str(i+1) + ' -> ' + str(worker_num_array[i]) + ' : ' + str(worker_time_array[i]) + ' -> ' + str(avg_time)
	print_str += str(avg_time) + '\n'

print print_str
#write_line = str(mean_idag) + '\n' + str(mean_init) + '\n' + str(mean_agent) + '\n'
#fd_w.write(write_line)
#fd_w.close()

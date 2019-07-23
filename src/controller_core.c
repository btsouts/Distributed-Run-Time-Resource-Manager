#include <sys/wait.h>

#include "include/controller_core.h"
#include "include/my_rtrm.h"
#include "include/libfunctions.h"
#include "include/noc_functions.h"
#include "include/sig_aux.h"
#include "include/common_core.h"
#include "include/signal_handlers.h"
#include "include/scc_signals.h"
#include "include/idag_defs.h"
#include "include/structs.h"
#include "include/variables.h"

#ifndef PLAT_LINUX
void idle_agent_actions(char scen_directory[SCEN_DIR_SIZE], char scen_num[SCEN_NUM_SIZE]) {
#else
void idle_agent_actions(char scen_directory[SCEN_DIR_SIZE], char scen_num[SCEN_NUM_SIZE], int Selfopt_Radius, int Max_SelfOpt_Interval_MS) {
#endif
	int i, one_core; 
	pid_t p;
	core_list *tmp_cores_list;
	inter_list tmp_inter_list;
	float avg_cluster_util;
	int init_delay_sec;
	
	
	printf("I an idag with node_id = %d, pid = %d paxos_state = %s\n",node_id,getpid(),id2string(paxos_state));
	if (paxos_state == NEW_IDAG){
		fprintf(log_file,"I an idag with node_id = %d, pid = %d\n",node_id,getpid());
	}
	
	if (paxos_state != NEW_IDAG)
	{
	  
		its.it_value.tv_sec = 0;
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;
		nodes_initialised=0;
		
		index_bottom[node_id] = 0;

		#ifdef PLAT_LINUX
		for (i=(node_id * MAX_SIGNAL_LIST_LEN * LINE_SIZE); i<((node_id + 1) * MAX_SIGNAL_LIST_LEN * LINE_SIZE); i++) {
				sig_array[i] = NO_SIG;
		}

		//semaphore inits
		if (sem_init(&scc_lock[node_id], 1, 1) == -1){
			printf("I am %d error\n",node_id);
			perror("sem_init");
		}

		if (sem_init(&flag_data_written[node_id], 1, 0) == -1){
			printf("I am %d error\n",node_id);
			perror("sem_init");
		}
		#endif

		install_signal_handlers();
		#ifdef PLAT_LINUX
		sig_SEGV_enable();
		#endif

		sev.sigev_notify = SIGEV_SIGNAL;
		sev.sigev_signo = SIG_TIMER;
		sev.sigev_value.sival_ptr = &timerid;
		if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1) printf("timer_create error\n");	  
	  
		DDS_count=0;
		my_cores_count=0;
		DDS=NULL;
		my_cores=NULL;
		
		if (my_cores == NULL) {
			my_cores = (core_list *) malloc(sizeof(core_list));
			my_cores_tail = my_cores;
		}
		
		my_cores_count++;
		my_cores_tail->core_id = node_id;
		my_cores_tail->offered_to = -1;
		my_cores_tail->next = NULL;
		
		DDS = (DDS_list *) malloc(sizeof(DDS_list));
		DDS->agent_id = node_id;
		DDS->next = NULL;
		DDS_tail = DDS;
		DDS_count++;
		for (i = 0; i < NUES; i++)
			if (i != node_id && idag_mask[i] == node_id) {
				if (my_cores == NULL) {
					my_cores = (core_list *) malloc(sizeof(core_list));
					my_cores_tail = my_cores;
				} else {
					my_cores_tail->next = (core_list *) malloc(sizeof(core_list));
					my_cores_tail = my_cores_tail->next;
				}

				my_cores_count++;
				my_cores_tail->core_id = i;
				my_cores_tail->offered_to = -1;
				my_cores_tail->next = NULL;
				#ifdef PLAT_LINUX
				/* 7.12.2015 I have to create new children only if i am initial IDAG */
				p = fork();
				if (p==0){
					node_id = i;
					common_node_actions(scen_directory,scen_num,Selfopt_Radius,Max_SelfOpt_Interval_MS);
				}
					
				#endif
			}

		DDS->num_of_cores = my_cores_count;
		
		if (log_file == NULL){
			log_file = create_log_file(node_id, 0, scen_directory, scen_num);
			setbuf(log_file, NULL);
		}

		fprintf(log_file,"my pid is %d\n",getpid());
		cur_time = time(NULL);
		cur_t = localtime(&cur_time);
		fprintf(log_file, "[%d:%d:%d]: I initialized node_id=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,node_id);

		alive = (int *)malloc(X_max*Y_max*sizeof(int));
		suspected = (int *)malloc(X_max*Y_max*sizeof(int));
		
		for (i = 0; i < X_max*Y_max; i++){
			alive[i] = 1;
			suspected[i] = 0;
		}
		
		#ifdef PLAT_SCC
		RCCE_barrier(&RCCE_COMM_WORLD);
		#else
		sleep(1);
		#endif
		
		for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next) {
		  
			one_core = tmp_cores_list->core_id;
			if (core_inter_head[one_core] == NULL){
				core_inter_head[one_core] = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[one_core] = core_inter_head[one_core];
			} else {
				core_inter_tail[one_core]->next = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[one_core] = core_inter_tail[one_core]->next;
			}

			core_inter_tail[one_core]->type = INIT_CORE;
			core_inter_tail[one_core]->next = NULL;
			
			//if (paxos_state != NEW_IDAG)
			signals_disable();
			scc_kill(one_core, SIG_INIT, core_inter_head[one_core]);
			signals_enable();
		}
		
		state = IDLE_IDAG;

		while (nodes_initialised != my_cores_count-1) {
			scc_pause();
			scc_signals_check();
		}

#ifdef PLAT_SCC
		RCCE_barrier(&RCCE_COMM_WORLD);
		#else
		sleep(1);
		#endif
		#if defined(EPFD) || defined(tEPFD)
		sev.sigev_notify = SIGEV_SIGNAL;
		sev.sigev_signo = SIG_EPFD_TIMER;
		sev.sigev_value.sival_ptr = &epfd_timer;
		if (timer_create(CLOCK_REALTIME, &sev, &epfd_timer) == -1)
			printf("timer_create error\n");
		else
			fprintf(log_file,"I succesfully created epfd_timer\n");
		#endif
			
		#if defined(PFD) || defined(tPFD)
		sev.sigev_notify = SIGEV_SIGNAL;
		sev.sigev_signo = SIG_PFD_TIMER;
		sev.sigev_value.sival_ptr = &pfd_timer;
		if (timer_create(CLOCK_REALTIME, &sev, &pfd_timer) == -1)
			printf("timer_create error\n");
		else
			fprintf(log_file,"I succesfully created pfd_timer\n");
		#endif
		if (node_id < 10){
			init_delay_sec = node_id;
		}else if (node_id < 100){
			init_delay_sec = node_id%10;
		}else{
			init_delay_sec = node_id%100;
		}
		#if defined(EPFD) || defined(tEPFD)
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;
		its.it_value.tv_sec = init_delay_sec;
		its.it_value.tv_nsec = 0;
		if (timer_settime(epfd_timer, 0, &its, NULL) == -1) perror("timer_settime error9");
		#endif

		#if defined (PFD) || defined (tPFD)
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;
		its.it_value.tv_sec = init_delay_sec;
		its.it_value.tv_nsec = 0;
		if (timer_settime(pfd_timer, 0, &its, NULL) == -1) perror("timer_settime error9");
		#endif
	}
	
	if (paxos_state == NEW_IDAG){
		tmp_inter_list.type = RECOVERED;
		scc_kill(10,SIG_RECOVER,&tmp_inter_list);
		state = IDLE_IDAG;
		pending_state = NO_PENDING_STATE;
		paxos_state = NO_PENDING_STATE;
		fprintf(log_file,"state : %s\n",id2string(state));
	}
	time_t t;
	srand((unsigned) time(&t));
	
	#ifdef CONTROLLER
	/* Scenario pou peftei o controller */
	 if (timer_schedule[node_id] != 0){
		timer_schedule[node_id] = 8 + rand() % 10;
	   	sev.sigev_notify = SIGEV_SIGNAL;
		sev.sigev_signo = SIG_CTIMER;
		sev.sigev_value.sival_ptr = &controller_timer;
		if (timer_create(CLOCK_REALTIME, &sev, &controller_timer) == -1)
			 printf("timer_create error\n");
		else
			 printf("Controller Timer created succesfully!\n");
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;
		
		its.it_value.tv_sec = timer_schedule[node_id];
		its.it_value.tv_nsec = 0;
		if (timer_settime(controller_timer, 0, &its, NULL) == -1)
			 perror("controller_core.c : timer_settime error9");
		else
			 printf("%d : My timer will explode in %d seconds.\n", node_id, timer_schedule[node_id]);
	 }
	 #endif

	while (state != IDAG_ENDING)
		if (state == IDLE_IDAG) {
			scc_pause();
			if (paxos_state == NEW_IDAG){
				fprintf(log_file, "I will check for signals now!\n");
			}
			scc_signals_check();
		} else {
			printf("idle_agent.c : Uknown state node_id = %d state = %s\n",node_id,id2string(state));
			state = IDLE_IDAG;
	}
	
	tmp_cores_list = my_cores;
	my_cores = my_cores->next;
	free(tmp_cores_list);
	for (; my_cores != NULL; my_cores = my_cores->next){
		tmp_cores_list = my_cores;
		one_core = my_cores->core_id;
		if (core_inter_head[one_core] == NULL) {
			core_inter_head[one_core] = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[one_core] = core_inter_head[one_core];
		} else {
			core_inter_tail[one_core]->next = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[one_core] = core_inter_tail[one_core]->next;
			fprintf(log_file,"I am still doing smth with my node %d interaction = %d\n",one_core,core_inter_head[one_core]->type);
		}

		core_inter_tail[one_core]->type = TERMINATION_STATS;
		core_inter_tail[one_core]->next = NULL;
		//kill(pid_num[one_core], SIG_TERMINATE);
		//signals_disable();
		scc_kill(one_core, SIG_TERMINATE, core_inter_head[one_core]);
		
		/* 8.7.2016 Paxos Stats */
		tmp_inter_list.next = NULL;
		tmp_inter_list.type = PAXOS_STATS_REQ;
		scc_kill(one_core, SIG_PAXOS_STATS_REQ, &tmp_inter_list);
		paxos_node_stats.msg_count++;
		paxos_node_stats.distance += distance(node_id,one_core);
		
		//signals_enable();
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,one_core);

		free(tmp_cores_list);
	}

	while (state == IDAG_ENDING) {
		scc_pause();
		scc_signals_check();

		/* 8.7.2016 Paxos stats */
		if (paxos_stats_replied == my_cores_count -1) {
			tmp_inter_list.next = NULL;
			tmp_inter_list.type = PAXOS_STATS_REP;
			
			paxos_total_stats.msg_count += paxos_node_stats.msg_count;
			paxos_total_stats.distance += paxos_node_stats.distance;
			
			tmp_inter_list.data.paxos_stats[0] = paxos_total_stats.msg_count;
			tmp_inter_list.data.paxos_stats[1] = paxos_total_stats.fd_msg_count;
			scc_kill(idag_id_arr[0], SIG_PAXOS_STATS_REP, &tmp_inter_list);
			paxos_stats_replied++;
		}
		
		if (stats_replied == my_cores_count-1 && paxos_stats_replied == my_cores_count) {
			core_inter_head[0] = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[0] = core_inter_head[0];
		
			core_inter_tail[0]->type = REP_STATISTICS;
			core_inter_tail[0]->next = NULL;

			total_stats.msg_count += my_stats.msg_count;
			total_stats.message_size += my_stats.message_size;
			total_stats.distance += my_stats.distance; 
			total_stats.app_turnaround += my_stats.app_turnaround;
			total_stats.comp_effort += my_stats.comp_effort;
			total_stats.cores_utilized += my_stats.cores_utilized;
			total_stats.times_accessed += my_stats.times_accessed;

			avg_cluster_util = (float) my_stats.cores_utilized / (my_stats.times_accessed * (my_cores_count-1));
			printf("I am %d with cores_utilized = %d times_accessed = %d my_cores_count = %d and avg_cluster_util = %0.2f\n",
				node_id,my_stats.cores_utilized,my_stats.times_accessed,my_cores_count,avg_cluster_util);
 			fprintf(log_file,"cores_utilized = %d times_accessed = %d my_cores_count = %d and avg_cluster_util = %0.2f\n",
				my_stats.cores_utilized,my_stats.times_accessed,my_cores_count,avg_cluster_util);

			core_inter_tail[0]->data.stats = total_stats;
			signals_disable();
			scc_kill(idag_id_arr[0], SIG_TERMINATE, core_inter_head[0]);
			signals_enable();
			stats_replied++;
		}
	}
	
	#ifdef PLAT_SCC
	RCCE_flag_free(&flag_data_written);
	RCCE_free((t_vcharp) sig_array);
	RCCE_free((t_vcharp) data_array);
	#else
	for (i=0; i<my_cores_count-1; i++) 
		wait(NULL);  
	#endif

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended well\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);	
	fclose(log_file);
	exit(0);
}

#include "idle_agent.h"

//extern int *pid_num;
extern int num_idags, node_id ,my_idag;//, fd_log;
extern FILE *log_file;
extern core_states state; 	
extern inter_list **core_inter_head,**core_inter_tail;
extern app my_app;
extern app init_app;
extern app far_req_app;
extern metrics my_stats, total_stats;
extern int far_req_or_sender;//far_req_max_man_cores, far_req_max_man, far_req_max_man_count, 
extern int *idag_mask, *idag_id_arr;
extern int *Cl_x_max_arr, *Cl_y_max_arr; 
extern int DDS_count, my_cores_count;
extern DDS_list *DDS, *DDS_tail;
extern core_list *my_cores, *my_cores_tail;
extern offer_list *init_man_offers, *selfopt_man_offers;
extern offer_list *far_man_offers, *far_man_offers_tail;
extern int far_list_count, far_man_agent_count;
extern int my_agent, time_worked;
extern int debug_global;
extern time_t cur_time;
extern struct tm *cur_t;
extern struct sigevent sev;
extern struct itimerspec its, chk_timer;
extern timer_t timerid;
extern long int selfopt_time_rem;//-1 means it is not set
extern long int upper_work_bound;
extern int time_for_farman;
extern char scen_num[4];
extern int chk_rem_count, chk_rem_num, sum_rem_time;
extern float old_Speedup, my_Speedup;
extern int nodes_ended_cnt, app_terminated, stats_replied;
region far_reg;
extern int nodes_initialised;
extern int *sig_array, *data_array, NUES;
extern RCCE_FLAG flag_signals_enabled,flag_data_written;

void idle_agent_actions(int idag_num, char scen_num[4]){
	int i,j,k, Cl_x_max, Cl_y_max, one_core;
	pid_t p;
	offer_list *tmp_offer_list;
	core_list *tmp_cores_list;
	float avg_cluster_util;

	DDS_count=0; 
	my_cores_count=0;
	DDS=NULL;
	my_cores=NULL;
	my_stats.msg_count=0;
	my_stats.message_size=0;
	my_stats.distance=0;
	my_stats.app_turnaround=0;
	my_stats.comp_effort=0;
	my_stats.cores_utilized=0;
	my_stats.times_accessed=0;
	its.it_value.tv_sec = 0;
	its.it_interval.tv_sec = 0;//its.it_value.tv_sec;
	its.it_interval.tv_nsec = 0;
	nodes_initialised=0;
	//node_sem = (sem_t*) shmat (seg_id, NULL, 0);

	i = get_cluster_info(idag_num, &Cl_x_max, &Cl_y_max);
	if (i != node_id) printf("I am %d and i was %d\n",node_id,i);
	idag_id_arr = (int *) malloc(num_idags*sizeof(int));
	Cl_x_max_arr = (int *) malloc(num_idags*sizeof(int));
	Cl_y_max_arr = (int *) malloc(num_idags*sizeof(int));
	idag_mask = (int *) malloc(X_max*Y_max*sizeof(int));
	far_reg.C = -1;
	far_reg.r = -1;	

	for (i=0; i<num_idags; i++) {
		idag_id_arr[i] = get_cluster_info(i, &Cl_x_max_arr[i], &Cl_y_max_arr[i]);
		for (j=idag_id_arr[i]; j<idag_id_arr[i] + Cl_y_max_arr[i]*X_max; j+=X_max)
			for (k=0; k<Cl_x_max_arr[i]; k++) 
				idag_mask[j+k] = idag_id_arr[i];
	}
	printf("I an idag with node_id = %d, pid = %d\n",node_id,getpid());			
	
	log_file = create_log_file(node_id, scen_num);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I start initialising node_id=%d",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,node_id);
	fflush(log_file);

	install_signal_handlers();
	//sig_SEGV_enable();
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIG_TIMER;
	sev.sigev_value.sival_ptr = &timerid;
	if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1) printf("timer_create error\n");

	for (j=node_id; j<node_id+Cl_y_max*X_max; j+=X_max)
		for (k=0; k<Cl_x_max; k++)	{
			if (my_cores == NULL) {
				my_cores = (core_list *) malloc(sizeof(core_list));
				my_cores_tail = my_cores;
			} else {
				my_cores_tail->next = (core_list *) malloc(sizeof(core_list));
				my_cores_tail = my_cores_tail->next;					
			}

			my_cores_count++;				
			my_cores_tail->core_id = j+k;
			my_cores_tail->offered_to = -1;
			my_cores_tail->next = NULL;			

			if ((j+k) == node_id){ 
				DDS = (DDS_list *) malloc(sizeof(DDS_list));
				DDS->agent_id = j+k;
				DDS->num_of_cores = Cl_x_max*Cl_y_max;
				DDS->next = NULL;
				DDS_tail = DDS;
				DDS_count++;
				
				//pid_num[j+k] = getpid();
			} /*else {
				p = fork();
				if (p==0){
					node_id = j+k;
					common_node_actions(node_id,scen_num,seg_id);
				}
			}*/
		}
	
	RCCE_barrier(&RCCE_COMM_WORLD);
	//sleep(1);
	
	for (j=node_id; j<node_id+Cl_x_max*X_max; j+=X_max)
		for (k=0; k<Cl_x_max; k++) 
			if ((j+k) != node_id) {
				signals_disable();
			
				one_core = j+k;
				if (core_inter_head[one_core] == NULL){
					core_inter_head[one_core] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[one_core] = core_inter_head[one_core];
				} else {
					core_inter_tail[one_core]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[one_core] = core_inter_tail[one_core]->next;
				}

				core_inter_tail[one_core]->type = INIT_CORE;
				core_inter_tail[one_core]->next = NULL;
				signals_enable();
				
				//kill(pid_num[one_core], SIG_INIT);
				scc_kill(one_core, SIG_INIT);
				//my_stats.msg_count++;
				//my_stats.distance += distance(node_id,one_core);
			}
	
	int dummy=0;
	while (nodes_initialised != my_cores_count-1) {//pause();
		for (i=0; i<1000; i++)
			for(j=0; j<1000; j++)
				dummy++;

		scc_signals_check();
	}

	state = IDLE_IDAG;
	while (state != IDAG_ENDING)
		if (state == IDLE_IDAG || state == IDLE_FAR_MAN) { 
			//pause();
			dummy=0;
			for (i=0; i<1000; i++)
				for(j=0; j<1000; j++)
					dummy++;

			scc_signals_check();
		/*} else if (state == FAR_MAN_CHK_OFFERS) {
			signals_disable();
			printf("far check alarm went off in idag %d! far_req_or_sender = %d\n",node_id,far_req_or_sender);
			cur_time = time(NULL);	
			cur_t = localtime(&cur_time);
			fprintf(log_file, "[%d:%d:%d]: far check alarm went off in idag %d! far_req_or_sender = %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,node_id,far_req_or_sender);
			fflush(log_file);		

			tmp_offer_list = far_man_offers;
			while (tmp_offer_list != NULL){
				fprintf(log_file,"Offer by %d for %d cores\n",tmp_offer_list->sender,tmp_offer_list->off.num_of_cores);
				tmp_offer_list = tmp_offer_list->next;
			}
			fflush(log_file);

			if (core_inter_head[far_req_or_sender] == NULL){
				core_inter_head[far_req_or_sender] = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[far_req_or_sender] = core_inter_head[far_req_or_sender];
			} else {
				core_inter_tail[far_req_or_sender]->next = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[far_req_or_sender] = core_inter_tail[far_req_or_sender]->next;
			}

			core_inter_tail[far_req_or_sender]->type = FAR_REQ_OFFER;
			//core_inter_tail[far_req_or_sender]->data.my_offer = far_man_offers->off;
			if (far_man_offers != NULL) 
				core_inter_tail[far_req_or_sender]->data.my_offer = far_man_offers->off;
			else {
				fprintf(log_file,"far_man_offers is null far_list_count = %d\n",far_list_count);
				fflush(log_file);
			}
			core_inter_tail[far_req_or_sender]->next = NULL;

			//kill(pid_num[far_req_or_sender],SIG_FAR_REQ);
			if (core_inter_head[far_req_or_sender]->next == NULL) {
				kill(pid_num[far_req_or_sender],SIG_FAR_REQ);
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,far_req_or_sender);
			} else printf("first i am doing smth else with far_req_or_sender type0=%d type1=%d\n",core_inter_head[far_req_or_sender]->type,core_inter_head[far_req_or_sender]->next->type);
			
			if (selfopt_time_rem != -1) printf("selfopt timer in idag??\n");
			state = IDLE_IDAG;
			signals_enable();*/
		} else {
			printf("Uknown state node_id = %d state = %d\n",node_id,state);	
			state = IDLE_IDAG;
		}

	//printf("killing inside %d\n",getpid());
	
	tmp_cores_list = my_cores;
	my_cores = my_cores->next;
	free(tmp_cores_list);
	for (; my_cores != NULL; my_cores = my_cores->next){
		tmp_cores_list = my_cores;
	
		one_core = my_cores->core_id;
		if (core_inter_head[one_core] == NULL){
			core_inter_head[one_core] = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[one_core] = core_inter_head[one_core];
		} else {
			core_inter_tail[one_core]->next = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[one_core] = core_inter_tail[one_core]->next;
			fprintf(log_file,"I am still doing smth with my node %d interaction = %d\n",one_core,core_inter_head[one_core]->type);
			fflush(log_file);		
		}

		core_inter_tail[one_core]->type = TERMINATION_STATS;
		core_inter_tail[one_core]->next = NULL;	
		//kill(pid_num[one_core], SIG_TERMINATE);
		scc_kill(one_core, SIG_TERMINATE);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,one_core);

		free(tmp_cores_list);
	}

	while (state == IDAG_ENDING) {
		//pause();
		dummy=0;
		for (i=0; i<1000; i++)
			for(j=0; j<1000; j++)
				dummy++;

		scc_signals_check();

		if (stats_replied == my_cores_count-1) {
			//printf("I am %d and all my cores replied their stats\n",node_id);
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
			fflush(log_file);

			core_inter_tail[0]->data.stats = total_stats;
			//kill(pid_num[0], SIG_TERMINATE);
			scc_kill(0, SIG_TERMINATE);
			my_cores_count = 0;
		}
	}
	
	//for (i=0; i<Cl_x_max*Cl_y_max-1; i++) wait(NULL);
	RCCE_flag_free(&flag_signals_enabled);
        RCCE_flag_free(&flag_data_written);
        RCCE_free((t_vcharp) sig_array);
        RCCE_free((t_vcharp) data_array);
	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended well\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);	
	fclose(log_file);
	exit(0);
}

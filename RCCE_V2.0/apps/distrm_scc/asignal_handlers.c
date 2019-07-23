#include "signal_handlers.h"

//extern int *pid_num;
extern int num_idags, node_id ,my_idag;//, fd_log;
extern FILE *log_file, *app_log_file;
extern core_states state, pending_state; 	
extern inter_list **core_inter_head,**core_inter_tail,*init_pending_head, *init_pending_tail;
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
extern int nodes_ended_cnt, app_terminated, nodes_initialised, stats_replied;
extern int init_DDS_replies, selfopt_DDS_replies, init_DDS_idags, selfopt_DDS_idags, selfopt_interval, init_idags_areas_replies;//, init_near_areas_num;
extern target_list *init_targets_head, *init_targets_tail;
extern target_list *selfopt_targets_head, *selfopt_targets_tail;
extern application_states app_state;

extern int *sig_array, *data_array, NUES, idags_replied;
extern RCCE_FLAG flag_signals_enabled,flag_data_written;

/*void send_init_reqs (int sender_id) {
	int agent_id, i;
	inter_list *tmp_inter_list;
	//core_list *tmp_cores_list;
	target_list *tmp_target_list;	
	//offer_list *tmp_offer_list;	
	
	for (tmp_target_list = init_targets_head; tmp_target_list != NULL; tmp_target_list = tmp_target_list->next){
		agent_id = tmp_target_list->target;

		for (tmp_inter_list = core_inter_head[agent_id]; tmp_inter_list != NULL; tmp_inter_list=tmp_inter_list->next)
			if (tmp_inter_list->type == FAR_INIT_REQ || tmp_inter_list->type == FAR_REQ_MAN) break; 

		if (tmp_inter_list == NULL) {
			if (core_inter_head[agent_id] == NULL){
				core_inter_head[agent_id] = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[agent_id] = core_inter_head[agent_id];
			} else {
				core_inter_tail[agent_id]->next = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[agent_id] = core_inter_tail[agent_id]->next;
			}

			core_inter_tail[agent_id]->type = AGENT_REQ_CORES_PENDING;
			core_inter_tail[agent_id]->data.reg_arr.num_of_regions = tmp_target_list->num_of_regions;
			core_inter_tail[agent_id]->data.reg_arr.region_arr = (region *) malloc(tmp_target_list->num_of_regions * sizeof(region));

			if (tmp_target_list->target == node_id) printf("Why is this here node_id=%d\n",node_id);
			fprintf(log_file,"Init req target = %d, num_of_regions = %d. Αreas:",tmp_target_list->target,tmp_target_list->num_of_regions);
			for (i=0; i<tmp_target_list->num_of_regions; i++) {
				core_inter_tail[agent_id]->data.reg_arr.region_arr[i] = tmp_target_list->region_arr[i];
				fprintf(log_file," (%d,%d),",tmp_target_list->region_arr[i].C,tmp_target_list->region_arr[i].r);
			}
			fprintf(log_file,"\n");
			fflush(log_file);
			core_inter_tail[agent_id]->next = NULL;

			//kill(pid_num[agent_id], SIG_REQ_CORES);
			if (core_inter_head[agent_id]->next == NULL) {
				kill(pid_num[agent_id], SIG_REQ_CORES);
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,agent_id);
			} else if (agent_id == sender_id && core_inter_head[agent_id]->next->type == AGENT_REQ_CORES_PENDING) { //den exei fugei apo to free alla einai wra na stalei
				kill(pid_num[agent_id], SIG_REQ_CORES);
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,agent_id);
			} else printf("This init fucker is not NULL interaction=%d interaction2=%d\n",core_inter_head[agent_id]->type,core_inter_head[agent_id]->next->type);
		} else 
			printf("I am %d and i did not send local requests to %d because he is my far manager\n",node_id,agent_id);
	}

	its.it_value.tv_nsec = INIT_NODE_INTERVAL * MS;//750000000;// * MS;
	if (timer_settime(timerid, 0, &its, NULL) == -1) perror("timer_settime error412\n");
}	

void send_selfopt_reqs (int sender_id) {
	int agent_id, i;
	//inter_list *tmp_inter_list;
	//core_list *tmp_cores_list;
	target_list *tmp_target_list;	
	//offer_list *tmp_offer_list;	

	for (tmp_target_list = selfopt_targets_head; tmp_target_list != NULL; tmp_target_list = tmp_target_list->next){
		agent_id = tmp_target_list->target;
		if (core_inter_head[agent_id] == NULL){
			core_inter_head[agent_id] = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[agent_id] = core_inter_head[agent_id];
		} else {
			core_inter_tail[agent_id]->next = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[agent_id] = core_inter_tail[agent_id]->next;
		}

		core_inter_tail[agent_id]->type = SELFOPT_REQ_CORES_PENDING;
		core_inter_tail[agent_id]->data.reg_arr.num_of_regions = tmp_target_list->num_of_regions;
		core_inter_tail[agent_id]->data.reg_arr.region_arr = (region *) malloc(tmp_target_list->num_of_regions * sizeof(region));
		if (tmp_target_list->target == node_id) printf("selfopt Why is this here node_id=%d\n",node_id);
		fprintf(log_file,"Selfopt req target = %d, num_of_regions = %d. Αreas:",tmp_target_list->target,tmp_target_list->num_of_regions);
		for (i=0; i<tmp_target_list->num_of_regions; i++) {
			core_inter_tail[agent_id]->data.reg_arr.region_arr[i] = tmp_target_list->region_arr[i];
			fprintf(log_file," (%d,%d),",tmp_target_list->region_arr[i].C,tmp_target_list->region_arr[i].r);
		}
		fprintf(log_file,"\n");
		fflush(log_file);				
		core_inter_tail[agent_id]->next = NULL;

		if (core_inter_head[agent_id]->next == NULL) {
			kill(pid_num[agent_id], SIG_REQ_CORES);
			my_stats.msg_count++;
			my_stats.distance += distance(node_id,agent_id);
		} else if (agent_id == sender_id && core_inter_head[agent_id]->next->type == SELFOPT_REQ_CORES_PENDING) {
			kill(pid_num[agent_id], SIG_REQ_CORES);
			my_stats.msg_count++;
			my_stats.distance += distance(node_id,agent_id);
		} else printf("This selfopt fucker is not NULL interaction=%d interaction2=%d\n",core_inter_head[agent_id]->type,core_inter_head[agent_id]->next->type);
	}
}

void sig_SEGV_handler(int signo, siginfo_t *info, void *context){
	signals_disable();
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: Segmentation fault\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);
	fflush(log_file);
	fflush(stderr);
	fflush(stdout);
	signals_enable();
	fclose(log_file);
	exit(1);
}*/

void sig_TERMINATE_handler(int sender_id)
{
	//metrics some_stats;
	inter_list *tmp_inter_list;
	int data_array_local[LINE_SIZE];
	int i, error, str_len;
	//RCCE_FLAG_STATUS receiver_status;
	char error_str[64];	
		
	signals_disable();
	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_TERMINATE_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	
	
	if (my_idag != -1 && core_inter_head[sender_id] != NULL && core_inter_head[sender_id]->type != TERMINATION_STATS) 
		while (core_inter_head[sender_id] != NULL && core_inter_head[sender_id]->type != TERMINATION_STATS)	{ 
			fprintf(log_file,"I am still doing smth with my agent %d interaction = %d\n",sender_id,core_inter_head[sender_id]->type);	
			fflush(log_file);

			tmp_inter_list = core_inter_head[sender_id];
			core_inter_head[sender_id] = core_inter_head[sender_id]->next;
			free(tmp_inter_list);
		}

	if (core_inter_head[sender_id] == NULL){ //edw tha mpei otan exw allaksei agent
				
		core_inter_head[sender_id] = (inter_list *) malloc(sizeof(inter_list));
		core_inter_tail[sender_id] = core_inter_head[sender_id];
		
		core_inter_tail[sender_id]->type = REP_STATISTICS;
		core_inter_tail[sender_id]->next = NULL;		

		if (my_idag != -1) {		
			core_inter_tail[sender_id]->data.stats = my_stats;
			//kill(info->si_pid, SIG_TERMINATE);
			scc_kill(sender_id, SIG_TERMINATE);
		} else 
			state = IDAG_ENDING;
	
	} else if (core_inter_head[sender_id]->type == TERMINATION_STATS) {
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);		
		scc_kill(sender_id, SIG_ACK);
	
		RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);

		error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), LINE_SIZE * sizeof(int), node_id);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			fprintf(log_file,"I got an error in get data in sig_TERMINATE_handler from %d with descr %s\n",sender_id,error_str);
			fflush(log_file);
		}

		total_stats.msg_count += data_array_local[0];//some_stats.msg_count;
		total_stats.message_size += data_array_local[1];//some_stats.message_size;
		total_stats.distance += data_array_local[2];//some_stats.distance; 
		total_stats.app_turnaround += data_array_local[3];//some_stats.app_turnaround;
		total_stats.comp_effort += data_array_local[4];//some_stats.comp_effort;
		total_stats.cores_utilized += data_array_local[5];//some_stats.cores_utilized;
		total_stats.times_accessed += data_array_local[6];//some_stats.times_accessed;

		stats_replied++;
		fprintf(log_file,"My node %d replied stats stats_replied = %d my_cores_count = %d msg_count=%d\n",sender_id,stats_replied,my_cores_count,data_array_local[0]);	
		fflush(log_file);	
	
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		free(tmp_inter_list);	
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else printf("I am %d in sig_terminate and after free i am still doing smth with my agent interaction = %d\n",node_id,core_inter_head[sender_id]->type);
	} else printf("I am %d in sig_terminate and i am still doing smth with my agent %d interaction = %d\n",node_id,sender_id,core_inter_head[sender_id]->type);

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_TERMINATE_handler with sender = %d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	
	
	signals_enable(); 
}

void sig_INIT_APP_handler(int sender_id)
{
	int data_array_local[LINE_SIZE];
	int i, error, str_len;
	//RCCE_FLAG_STATUS receiver_status;
	char error_str[64];	
	signals_disable();
	inter_list *tmp_inter_list, *tmp_inter_prev;	

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_INIT_APP_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	
	
	if (node_id == 0) {
		tmp_inter_prev = NULL;
		for (tmp_inter_list = init_pending_head; tmp_inter_list != NULL; tmp_inter_list = tmp_inter_list->next) {
			if (tmp_inter_list->data.new_app.num_of_cores == sender_id) break;
			tmp_inter_prev = tmp_inter_list;
		}
				
		if (tmp_inter_list != NULL) {
			fprintf(log_file,"I am sending an aborted init_app\n");
			fflush(log_file);

			if (core_inter_head[sender_id] == NULL){
				core_inter_head[sender_id] = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[sender_id] = core_inter_head[sender_id];
			} else {
				core_inter_tail[sender_id]->next = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[sender_id] = core_inter_tail[sender_id]->next;
			}

			core_inter_tail[sender_id]->type = INIT_APP;
			core_inter_tail[sender_id]->data.new_app = tmp_inter_list->data.new_app;
			core_inter_tail[sender_id]->data.new_app.num_of_cores = 0;
			core_inter_tail[sender_id]->next = NULL;

			if (core_inter_head[sender_id]->next == NULL) {
				//kill(pid_num[sender_id],SIG_INIT_APP);
				scc_kill(sender_id,SIG_INIT_APP);  
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,sender_id);
			}

			if (tmp_inter_prev == NULL) {
				init_pending_head = init_pending_head->next;
				//free(tmp_inter_list);
			} else {
				tmp_inter_prev->next = tmp_inter_list->next;
				if (tmp_inter_prev->next == NULL) init_pending_tail = tmp_inter_prev;
			}

			free(tmp_inter_list);
		}
	} else if (state == INIT_MANAGER || state == INIT_MANAGER_SEND_OFFERS || state == IDLE_INIT_MAN || state == INIT_MAN_CHK_OFFERS || state == WORKING_NODE_IDLE_INIT) {
		//printf("I have to reject sig_INIT_APP sender_id=%d node_id=%d state=%d\n",sender_id,node_id,state);
		fprintf(log_file,"I have to reject sig_INIT_APP sender_id=%d\n",sender_id);
		fflush(log_file);

		scc_kill(sender_id, SIG_REJECT);
		//kill(info->si_pid, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	} else {
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);
    scc_kill(sender_id, SIG_ACK);
    RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);

		//scc_kill(sender_id, SIG_ACK);		
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		
		error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), LINE_SIZE * sizeof(int), node_id);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			fprintf(log_file,"I got an error in get data in sig_INIT_APP from %d with descr %s\n",sender_id,error_str);
			fflush(log_file);
		}

		init_app.id = data_array_local[0];	
		memcpy(&init_app.A,&data_array_local[1],sizeof(int));
		memcpy(&init_app.var,&data_array_local[2],sizeof(int));
		memcpy(&init_app.workld,&data_array_local[3],sizeof(int));	
		init_app.num_of_cores = data_array_local[4];
		//read(fd_r, &init_app, sizeof(app));
		
		fprintf(log_file,"My app is A = %f, var = %f, cores = %d\n",init_app.A,init_app.var,init_app.num_of_cores);
		fflush(log_file);	

		if (state == IDLE_AGENT_WAITING_OFF || state == AGENT_SELF_CHK_OFFERS || state == AGENT_ZOMBIE || state == AGENT_ENDING) 
			pending_state = INIT_MANAGER;
		else if (state == IDLE_AGENT) {
			if (timer_gettime(timerid, &chk_timer) == -1) perror("timer_gettime error init\n");
			else selfopt_time_rem = chk_timer.it_value.tv_nsec;
		
			its.it_value.tv_nsec = 0;
			if (timer_settime(timerid, 0, &its, NULL) == -1) printf("timer_settime error init\n");	 
			pending_state = IDLE_AGENT;
			state = INIT_MANAGER;
		} else if (state == WORKING_NODE || state == AGENT_SELF_OPT) {
			pending_state = state;
			state = INIT_MANAGER;
		} else state = INIT_MANAGER;
	}

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_INIT_APP_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	
	signals_enable();
}

void sig_TIMER_handler(int signo, siginfo_t *info, void *context)
{
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: Alarm went off state=%d pending_state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,state,pending_state);
	fflush(log_file);

	if (state == IDLE_INIT_MAN) state = INIT_MAN_CHK_OFFERS;
	else if (state == IDLE_FAR_MAN) state = FAR_MAN_CHK_OFFERS;
	else if (state == IDLE_AGENT_WAITING_OFF) state = AGENT_SELF_CHK_OFFERS;
	else if (state == IDLE_AGENT) state = AGENT_SELF_OPT;
	else if (state == IDLE_CHK_APP_FILE) state = CHK_APP_FILE;
	else if (state == IDLE_INIT_MAN_SELFOPT_PENDING) {
		state = INIT_MAN_CHK_OFFERS_SELFOPT_PENDING;
		//printf("I am %d and ths giwrgria ths aresei na spermangarei k na mastourbarei\n",node_id);
	}
	else if (state == IDLE_INIT_MAN_WORK_PENDING) {
		state = INIT_MAN_CHK_OFFERS_WORK_PENDING;
		//printf("I am %d and poutana thalassa\n",node_id);
	}
	else if (state == WORKING_NODE_IDLE_INIT) {
		state = INIT_MAN_CHK_OFFERS;
		pending_state = WORKING_NODE;
		//printf("I am %d and poutana thalassa\n",node_id);
	}
	else if (state == AGENT_INIT_STATE) {
		if (pending_state == IDLE_INIT_MAN) pending_state = INIT_MAN_CHK_OFFERS;
	}
	else printf("i am %d, timer went off and i don't know what to do. My state is %d\n",node_id,state);	

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: End of timer handler state=%d pending_state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,state,pending_state);
	fflush(log_file);		
}

void sig_INIT_handler(int sender_id)
{
	int data_array_local[LINE_SIZE];
	int error, str_len;
	//RCCE_FLAG_STATUS receiver_status;
	char error_str[64];	
	
	signals_disable();
	sender_id = get_id_from_pid(info->si_pid);
	//kill(info->si_pid, SIG_ACK);
	RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);		
	scc_kill(sender_id, SIG_ACK);
	
	RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);

	error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), LINE_SIZE * sizeof(int), node_id);
	if (error != RCCE_SUCCESS) {
		RCCE_error_string(error, error_str, &str_len);
		printf("I am %d and i got an error in get data in sig_INIT from %d with descr %s\n",node_id,sender_id,error_str);
	} else my_idag = data_array_local[0];

	my_stats.msg_count++; //gia to sig_ACK
	my_stats.distance += distance(node_id,my_idag);
	
	printf("I am node with id %d my idle agent is %d\n",node_id,my_idag);

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: my idle agent is %d and my pid is %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,my_idag,getpid());
	fflush(log_file);	
	
	signals_enable(); 	
}

void sig_ACK_handler(int sender_id)
{
	int clear=1, i, j, data_array_local[3 * LINE_SIZE];;//, fd_r;//fd_r, ; clear is 1 if node is to be removed
	inter_list *tmp_inter_list;
	DDS_list *tmp_DDS;//, *tmp_inter_prev=NULL;
	offer_list *tmp_offer_list;
	int error, str_len;
	//RCCE_FLAG_STATUS receiver_status;
	char error_str[64];	

	signals_disable();
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_ACK_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	

	if (core_inter_head[sender_id] == NULL) {
		fprintf(log_file,"We were fucked in sig_ACK! sender_id = %d\n",sender_id);
		fflush(log_file);
	} else {
		fprintf(log_file, "Type=%d\n",core_inter_head[sender_id]->type);
		fflush(log_file);
				
		tmp_inter_list = core_inter_head[sender_id];

		if (tmp_inter_list->type == INIT_CORE){ 
			data_array_local[0] = node_id;
			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), LINE_SIZE * sizeof(int), sender_id);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				printf("I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
			}	

			RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
			fprintf(log_file, "node_id=%d size=%d write_res=%d\n",node_id,sizeof(int),write_res);
			fflush(log_file);
			my_stats.message_size += sizeof(int);
			nodes_initialised++;
		} else if (tmp_inter_list->type == INIT_APP){
			//write_res = write(fd_w, &tmp_inter_list->data.new_app, sizeof(app));
			//fprintf(log_file, "A=%0.2f size=%d write_res=%d\n",tmp_inter_list->data.new_app.A,sizeof(app),write_res);
			//fflush(log_file);
			data_array_local[0] = tmp_inter_list->data.new_app.id;	
			memcpy(&data_array_local[1],&tmp_inter_list->data.new_app.A,sizeof(int));
			memcpy(&data_array_local[2],&tmp_inter_list->data.new_app.var,sizeof(int));
			memcpy(&data_array_local[3],&tmp_inter_list->data.new_app.workld,sizeof(int));	
			data_array_local[4] = tmp_inter_list->data.new_app.num_of_cores;

			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), LINE_SIZE * sizeof(int), sender_id);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				printf("I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
			}	

			RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);

			my_stats.message_size += sizeof(app);						
		} else if (tmp_inter_list->type == IDAG_FIND_IDAGS_PENDING 	|| tmp_inter_list->type == SELFOPT_IDAG_FIND_IDAGS_PENDING) {//I am the requesting common node
			data_array_local[0] = tmp_inter_list->data.reg.C;
			data_array_local[1] = tmp_inter_list->data.reg.r;
			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), LINE_SIZE * sizeof(int), sender_id);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				printf("I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
			}	

			RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
			
			//write_res = write(fd_w, &tmp_inter_list->data.reg, sizeof(region));
			fprintf(log_file, "C=%d r=%d\n",tmp_inter_list->data.reg.C,tmp_inter_list->data.reg.r);
			fflush(log_file);
			if (tmp_inter_list->type == IDAG_FIND_IDAGS_PENDING) tmp_inter_list->type = IDAG_FIND_IDAGS;
			else if (tmp_inter_list->type == SELFOPT_IDAG_FIND_IDAGS_PENDING) tmp_inter_list->type = SELFOPT_IDAG_FIND_IDAGS;

			my_stats.message_size += sizeof(region);
			clear = 0;
		} else if (core_inter_head[sender_id]->type == IDAG_REQ_DDS_PENDING || core_inter_head[sender_id]->type == FAR_REQ_IDAG_REQ_DDS_PENDING 
			|| core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS_PENDING || core_inter_head[sender_id]->type == DEBUG_IDAG_REQ_DDS){
			data_array_local[0] = tmp_inter_list->data.reg.C;
			data_array_local[1] = tmp_inter_list->data.reg.r;
			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), LINE_SIZE * sizeof(int), sender_id);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				printf("I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
			}	

			RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
			//write_res = write(fd_w, &tmp_inter_list->data.reg, sizeof(region));
			fprintf(log_file, "C=%d r=%d\n",tmp_inter_list->data.reg.C,tmp_inter_list->data.reg.r);
			fflush(log_file);
			my_stats.message_size += sizeof(region);

			if (core_inter_head[sender_id]->type == IDAG_REQ_DDS_PENDING) core_inter_head[sender_id]->type = IDAG_REQ_DDS;
		 	else if (core_inter_head[sender_id]->type == FAR_REQ_IDAG_REQ_DDS_PENDING) core_inter_head[sender_id]->type = FAR_REQ_IDAG_REQ_DDS;
			else if (core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS_PENDING) core_inter_head[sender_id]->type = SELFOPT_IDAG_REQ_DDS;

			clear = 0;
		} else if (tmp_inter_list->type == REP_IDAG_FIND_IDAGS) {//I am the idag
			data_array_local[0] = tmp_inter_list->data.idags_in_reg[num_idags_x*num_idags_y];
			fprintf(log_file, "num_of_idags=%d\n",tmp_inter_list->data.idags_in_reg[num_idags_x*num_idags_y]);
			fflush(log_file);
			my_stats.message_size += sizeof(int);
			
			j=1;
			for (i=0; i<num_idags_x*num_idags_y; i++)
				if (tmp_inter_list->data.idags_in_reg[i])	{			
					//write_res = write(fd_w, &idag_id_arr[i], sizeof(int)); //matching of increasing number to node_id
					data_array_local[j] = idag_id_arr[i];
					j++;
					fprintf(log_file, "idag=%d\n",idag_id_arr[i]);
					fflush(log_file);
					my_stats.message_size += sizeof(int);
				}

			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), LINE_SIZE * sizeof(int), sender_id);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				printf("I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
			}	

			RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
		} else if (tmp_inter_list->type == REP_IDAG_REQ_DDS) {//I am the idag
			if (tmp_inter_list->data.agents_in_reg == NULL) {//debugging
				fprintf(log_file, "In null rep_idag_dds with sender %d and DDS_count %d\n",sender_id,DDS_count);
				fflush(log_file);				
				data_array_local[0] = DDS_count;
				data_array_local[1] = 0;

				i=2;
				tmp_DDS = DDS;			
				while(tmp_DDS != NULL){				
					//write_res = write(fd_w, &tmp_DDS->agent_id, sizeof(int));
					//write_res = write(fd_w, &tmp_DDS->num_of_cores, sizeof(int));
					data_array_local[i++] = tmp_DDS->agent_id;
					data_array_local[i++] = tmp_DDS->num_of_cores;
					tmp_DDS = tmp_DDS->next;
				}

				error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), 2 * LINE_SIZE * sizeof(int), sender_id);
				if (error != RCCE_SUCCESS) {
					RCCE_error_string(error, error_str, &str_len);
					fprintf(log_file,"I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
					fflush(log_file);
				}
				RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
			} else {
				data_array_local[0] = tmp_inter_list->data.agents_in_reg[0];
				data_array_local[1] = 0;
				fprintf(log_file, "num_of_agents=%d\n",tmp_inter_list->data.agents_in_reg[0]);
				fflush(log_file);
				my_stats.message_size += sizeof(int);

				j=2;
				for (i=1; i<=2*tmp_inter_list->data.agents_in_reg[0]; i+=2){
					/*write_res = write(fd_w, &tmp_inter_list->data.agents_in_reg[i], sizeof(int));
					fprintf(log_file, "agent=%d size=%d write_res=%d\n",tmp_inter_list->data.agents_in_reg[i],sizeof(int),write_res);
					fflush(log_file);
					write_res = write(fd_w, &tmp_inter_list->data.agents_in_reg[i+1], sizeof(int));//offset may be different!!!!!!
					fprintf(log_file, "cores=%d size=%d write_res=%d\n",tmp_inter_list->data.agents_in_reg[i+1],sizeof(int),write_res);
					fflush(log_file);*/
					fprintf(log_file, "agent=%d cores=%d\n",tmp_inter_list->data.agents_in_reg[i],tmp_inter_list->data.agents_in_reg[i+1]);
					fflush(log_file);
					data_array_local[j++] = tmp_inter_list->data.agents_in_reg[i];
					data_array_local[j++] = tmp_inter_list->data.agents_in_reg[i+1];
					my_stats.message_size += 2 * sizeof(int);
				}
			
				error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), 2 * LINE_SIZE * sizeof(int), sender_id);
        if (error != RCCE_SUCCESS) {
          RCCE_error_string(error, error_str, &str_len);
          fprintf(log_file,"I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
          fflush(log_file);
        }
        RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
			}		
		} else if (tmp_inter_list->type == AGENT_REQ_CORES_PENDING) {
 			fprintf(log_file, "A=%0.2f\n",init_app.A);
			fflush(log_file);
			my_stats.message_size += sizeof(app);

			data_array_local[0] = init_app.id;	
			memcpy(&data_array_local[1],&init_app.A,sizeof(int));
			memcpy(&data_array_local[2],&init_app.var,sizeof(int));
			memcpy(&data_array_local[3],&init_app.workld,sizeof(int));	
			data_array_local[4] = init_app.num_of_cores;
			
			//write_res = write(fd_w, &tmp_inter_list->data.reg_arr.num_of_regions, sizeof(int));
			data_array_local[5] = tmp_inter_list->data.reg_arr.num_of_regions;
			fprintf(log_file, "num_of_regions=%d\n",tmp_inter_list->data.reg_arr.num_of_regions);
			fflush(log_file);	
			
			if (tmp_inter_list->data.reg_arr.num_of_regions > 1) {
				fprintf(log_file, "In init ack i have num_of_regions = %d\n",tmp_inter_list->data.reg_arr.num_of_regions);
				fflush(log_file);
			}	

			for (i=0; i<tmp_inter_list->data.reg_arr.num_of_regions; i++) {
				//write_res = write(fd_w, &tmp_inter_list->data.reg_arr.region_arr[i], sizeof(region));
				fprintf(log_file, "C=%d r=%d\n",tmp_inter_list->data.reg_arr.region_arr[i].C,tmp_inter_list->data.reg_arr.region_arr[i].r);
				fflush(log_file);
				data_array_local[6] = tmp_inter_list->data.reg_arr.region_arr[i].C;
				data_array_local[7] = tmp_inter_list->data.reg_arr.region_arr[i].r;
				my_stats.message_size += sizeof(region);
			}

			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]),LINE_SIZE * sizeof(int), sender_id);
      if (error != RCCE_SUCCESS) {
        RCCE_error_string(error, error_str, &str_len);
        fprintf(log_file,"I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
        fflush(log_file);
      }
      RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);

			tmp_inter_list->type = AGENT_REQ_CORES;
			free(tmp_inter_list->data.reg_arr.region_arr);
			clear = 0;
		} else if (tmp_inter_list->type == SELFOPT_REQ_CORES_PENDING) {
 			fprintf(log_file, "A=%0.2f num_of_cores = %d\n",my_app.A,my_app.num_of_cores);
			fflush(log_file);
			my_stats.message_size += sizeof(app);

			data_array_local[0] = my_app.id;	
			memcpy(&data_array_local[1],&my_app.A,sizeof(int));
			memcpy(&data_array_local[2],&my_app.var,sizeof(int));
			memcpy(&data_array_local[3],&my_app.workld,sizeof(int));	
			data_array_local[4] = my_app.num_of_cores;

			//write_res = write(fd_w, &tmp_inter_list->data.reg_arr.num_of_regions, sizeof(int));
			data_array_local[5] = tmp_inter_list->data.reg_arr.num_of_regions;			
			fprintf(log_file, "num_of_regions=%d\n",tmp_inter_list->data.reg_arr.num_of_regions);
			fflush(log_file);	

			if (tmp_inter_list->data.reg_arr.num_of_regions > 1) {
				fprintf(log_file, "In selfopt ack i have num_of_regions = %d\n",tmp_inter_list->data.reg_arr.num_of_regions);
				fflush(log_file);
			}		
			
			for (i=0; i<tmp_inter_list->data.reg_arr.num_of_regions; i++) {
				//write_res = write(fd_w, &tmp_inter_list->data.reg_arr.region_arr[i], sizeof(region));
				data_array_local[6] = tmp_inter_list->data.reg_arr.region_arr[i].C;
				data_array_local[7] = tmp_inter_list->data.reg_arr.region_arr[i].r;
				fprintf(log_file, "C=%d r=%d\n",tmp_inter_list->data.reg_arr.region_arr[i].C,tmp_inter_list->data.reg_arr.region_arr[i].r);
				fflush(log_file);
				my_stats.message_size += sizeof(region);
			}

			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]),LINE_SIZE * sizeof(int), sender_id);
      if (error != RCCE_SUCCESS) {
        RCCE_error_string(error, error_str, &str_len);
        fprintf(log_file,"I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
        fflush(log_file);
      }
      RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
			tmp_inter_list->type = SELFOPT_REQ_CORES;
			free(tmp_inter_list->data.reg_arr.region_arr);
			clear = 0;
		/*} else if (tmp_inter_list->type == FAR_REQ_CORES_PENDING) {//I am the requesting common node
			//printf("I am common node inside with sender_id %d and %s\n",sender_id,fifo_name);
			//if (tmp_inter_list->type == AGENT_REQ_CORES) 			j
			//else 
			//if (tmp_inter_list->type == FAR_REQ_CORES) 
			//else write_res = write(fd_w, &my_app, sizeof(app));
			write_res = write(fd_w, &far_req_app, sizeof(app));
			fprintf(log_file, "A=%0.2f size=%d write_res=%d\n",far_req_app.A,sizeof(app),write_res);
			fflush(log_file);
			my_stats.message_size += sizeof(app);

			i = 1;
			write_res = write(fd_w, &i, sizeof(int));
			fprintf(log_file, "num_of_regions=%d size=%d write_res=%d\n",i,sizeof(int),write_res);
			fflush(log_file);
			my_stats.message_size += sizeof(int);

			write_res = write(fd_w, &tmp_inter_list->data.reg, sizeof(region));
			fprintf(log_file, "C=%d r=%d size=%d write_res=%d\n",tmp_inter_list->data.reg.C,tmp_inter_list->data.reg.r,sizeof(region),write_res);
			fflush(log_file);
			my_stats.message_size += sizeof(region);
			tmp_inter_list->type = FAR_REQ_CORES;
			clear = 0;*/				
		} else if (tmp_inter_list->type == REP_AGENT_REQ_CORES) {//I am the agent
			//printf("I am common node inside with sender_id %d and %s\n",sender_id,fifo_name);
			write_res = write(fd_w, &tmp_inter_list->data.off_arr.num_of_offers, sizeof(int));
			fprintf(log_file, "num_of_offers=%d size=%d write_res=%d\n",tmp_inter_list->data.off_arr.num_of_offers,sizeof(int),write_res);
			fflush(log_file);	
			my_stats.message_size += sizeof(int);			
			for (j=0; j<tmp_inter_list->data.off_arr.num_of_offers; j++){
				//printf("I am %d and in ack i offer %d cores\n",node_id,tmp_inter_list->data.off_arr.offer_arr[j].num_of_cores);
				write_res = write(fd_w, &tmp_inter_list->data.off_arr.offer_arr[j].num_of_cores, sizeof(int));
				fprintf(log_file, "num_of_cores=%d size=%d write_res=%d\n",tmp_inter_list->data.off_arr.offer_arr[j].num_of_cores,sizeof(int),write_res);
				fflush(log_file);
				my_stats.message_size += sizeof(int);
				write_res = write(fd_w, &tmp_inter_list->data.off_arr.offer_arr[j].spd_loss, sizeof(float));
				fprintf(log_file, "spd_loss=%0.2f size=%d write_res=%d\n",tmp_inter_list->data.off_arr.offer_arr[j].spd_loss,sizeof(float),write_res);
				fflush(log_file);
				my_stats.message_size += sizeof(float);
				for (i=0; i<tmp_inter_list->data.off_arr.offer_arr[j].num_of_cores; i++) {
				//printf ("Offering core = %d\n",tmp_inter_list->data.my_offer.offered_cores[i]);
					write_res = write(fd_w, &tmp_inter_list->data.off_arr.offer_arr[j].offered_cores[i], sizeof(int));
					fprintf(log_file, "core=%d size=%d write_res=%d\n",tmp_inter_list->data.off_arr.offer_arr[j].offered_cores[i],sizeof(int),write_res);
					fflush(log_file);
					//printf("I am %d and in here i offeres %d\n",node_id,tmp_inter_list->data.off_arr.offer_arr[j].offered_cores[i]);
					my_stats.message_size += sizeof(int);
				}
			}

			core_inter_head[sender_id]->type = AGENT_OFFER_SENT;
			/*free(tmp_inter_list->data.my_offer.offered_cores);
			tmp_inter_list = core_inter_head[sender_id]->next;
			free(core_inter_head[sender_id]);
			core_inter_head[sender_id] = (inter_list *) malloc(sizeof(inter_list));
			core_inter_head[sender_id]->next = tmp_inter_list;
			core_inter_head[sender_id]->type = AGENT_OFFER_SENT;*///kalutera na krataw thn prosfora gia na exw tous purhnes
			//core_inter_head[sender_id]->data.offer_accepted = -1;
			clear = 0;
		} else if (tmp_inter_list->type == REP_AGENT_OFFER_SENT) {
			fprintf(log_file, "I have to reply %d for %d offers\n",sender_id,tmp_inter_list->data.offer_acc_array[0]);
			fflush(log_file);
						
			my_stats.message_size += sizeof(int);
			for (i=1; i<=tmp_inter_list->data.offer_acc_array[0]; i++){
				//printf("offer answer %d is %d\n",i,tmp_inter_list->data.offer_acc_array[i]);	
				write_res = write(fd_w, &tmp_inter_list->data.offer_acc_array[i], sizeof(int));
				fprintf(log_file, "offer_ans=%d size=%d write_res=%d\n",tmp_inter_list->data.offer_acc_array[i],sizeof(int),write_res);
				fflush(log_file);
				//fprintf(log_file, "I replied %d with %d\n",sender_id,tmp_inter_list->data.offer_acc_array[i]);
				my_stats.message_size += sizeof(int);
			}
			fflush(log_file);
			free(tmp_inter_list->data.offer_acc_array);	
			//write_res = write(fd_w, &tmp_inter_list->data.offer_accepted, sizeof(int));				
		} else if (tmp_inter_list->type == INIT_AGENT) {
				write_res = write(fd_w, &init_app, sizeof(app));
				fprintf(log_file, "A=%0.2f size=%d write_res=%d\n",init_app.A,sizeof(app),write_res);
				fflush(log_file);

				my_stats.message_size += sizeof(app);
				//printf("poutanes %d\n",init_app.num_of_cores);
				for (i=1; i<=init_app.num_of_cores; i++){
					//printf("karioles %d\n",tmp_inter_list->data.app_cores[i]);			
					write_res = write(fd_w, &tmp_inter_list->data.app_cores[i], sizeof(int));
					fprintf(log_file, "core=%d size=%d write_res=%d\n",tmp_inter_list->data.app_cores[i],sizeof(int),write_res);
					fflush(log_file);
					my_stats.message_size += sizeof(int);
				}

				init_app.A=-1.0;
				init_app.var=-1.0; 
				init_app.num_of_cores=-1;
				kill(pid_num[0], SIG_INIT_APP);
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,0);

				free(tmp_inter_list->data.app_cores);
		} else if (tmp_inter_list->type == IDAG_ADD_CORES_DDS || tmp_inter_list->type == IDAG_REM_CORES_DDS) {// || tmp_inter_list->type == REMOVE_APP
			//fprintf(log_file, "I am in add/remove/remove_app to %d with %d cores\n",sender_id,tmp_inter_list->data.app_cores[0]);			
			//fflush(log_file);	
			write_res = write(fd_w, &tmp_inter_list->data.app_cores[0], sizeof(int));
			fprintf(log_file, "app_cores=%d size=%d write_res=%d\n",tmp_inter_list->data.app_cores[0],sizeof(int),write_res);
			fflush(log_file);
			my_stats.message_size += sizeof(int);
			for (i=1; i<=tmp_inter_list->data.app_cores[0]; i++){			
				write_res = write(fd_w, &tmp_inter_list->data.app_cores[i], sizeof(int));
				fprintf(log_file, "core=%d size=%d write_res=%d\n",tmp_inter_list->data.app_cores[i],sizeof(int),write_res);
				fflush(log_file);
				my_stats.message_size += sizeof(int);
			}
			//I am an idag and i have to send to other idags my original sender
			if (my_idag == -1) {//idag_id != -1
				write_res = write(fd_w, &tmp_inter_list->data.app_cores[i], sizeof(int));
				fprintf(log_file, "orig_sender=%d size=%d write_res=%d\n",tmp_inter_list->data.app_cores[i],sizeof(int),write_res);
				fflush(log_file);
				my_stats.message_size += sizeof(int);
			}
		} else if (tmp_inter_list->type == REMOVE_APP) {
			//fprintf(log_file, "I am in add/remove/remove_app to %d with %d cores\n",sender_id,tmp_inter_list->data.app_cores[0]);			
			//fflush(log_file);	
			if (my_idag != -1) {			
				write_res = write(fd_w, &tmp_inter_list->data.app_cores[0], sizeof(int));
				fprintf(log_file, "app_cores=%d size=%d write_res=%d\n",tmp_inter_list->data.app_cores[0],sizeof(int),write_res);
				fflush(log_file);
				my_stats.message_size += sizeof(int);
				for (i=1; i<=tmp_inter_list->data.app_cores[0]; i++){			
					write_res = write(fd_w, &tmp_inter_list->data.app_cores[i], sizeof(int));
					fprintf(log_file, "core=%d size=%d write_res=%d\n",tmp_inter_list->data.app_cores[i],sizeof(int),write_res);
					fflush(log_file);
					my_stats.message_size += sizeof(int);
				}
			} else {
				//I am an idag and i have to send to other idags my original sender
				write_res = write(fd_w, &tmp_inter_list->data.agent_ended, sizeof(int));
				fprintf(log_file, "orig_sender=%d size=%d write_res=%d\n",tmp_inter_list->data.agent_ended,sizeof(int),write_res);
				fflush(log_file);
				my_stats.message_size += sizeof(int);
			}
		} else if (tmp_inter_list->type == FAR_INIT_REQ) {//I am the requesting common node
			write_res = write(fd_w, &init_app, sizeof(app));
			fprintf(log_file, "A=%0.2f size=%d write_res=%d\n",init_app.A,sizeof(app),write_res);
			fflush(log_file);
			my_stats.message_size += sizeof(app);			
			write_res = write(fd_w, &tmp_inter_list->data.reg, sizeof(region));
			fprintf(log_file, "C=%d r=%d size=%d write_res=%d\n",tmp_inter_list->data.reg.C,tmp_inter_list->data.reg.r,sizeof(region),write_res);
			fflush(log_file);
			my_stats.message_size += sizeof(region);
			clear = 0;
		} else if (tmp_inter_list->type == FAR_REQ_MAN_APPOINT) {//I am the requesting common node
			//fprintf(log_file, "I am appointing %d for far manager of %d with region (%d,%d)\n",
			//	sender_id,tmp_inter_list->data.far_req.orig_sender,tmp_inter_list->data.far_req.reg.C,tmp_inter_list->data.far_req.reg.r);
			//fflush(log_file);			
			write_res = write(fd_w, &tmp_inter_list->data.far_req.orig_sender, sizeof(int));
			fprintf(log_file, "orig_sender=%d size=%d write_res=%d\n",tmp_inter_list->data.far_req.orig_sender,sizeof(int),write_res);
			fflush(log_file);			
			my_stats.message_size += sizeof(int);			
			write_res = write(fd_w, &tmp_inter_list->data.far_req.far_app, sizeof(app));
			fprintf(log_file, "A=%0.2f size=%d write_res=%d\n",tmp_inter_list->data.far_req.far_app.A,sizeof(app),write_res);
			fflush(log_file);
			my_stats.message_size += sizeof(app);			
			write_res = write(fd_w, &tmp_inter_list->data.far_req.reg, sizeof(region));
			fprintf(log_file, "C=%d r=%d size=%d write_res=%d\n",tmp_inter_list->data.far_req.reg.C,tmp_inter_list->data.far_req.reg.r,sizeof(region),write_res);
			fflush(log_file);
			my_stats.message_size += sizeof(region);

			if (tmp_inter_list->next != NULL && tmp_inter_list->next->type == FAR_REQ_MAN_APPOINT) { //gia na thrhthei h seira k na mhn skaei to apo katw
				core_inter_head[sender_id] = tmp_inter_list->next;
				free(tmp_inter_list);
				clear = 0;
			}	
			/*far_req_app.A=-1.0;
			far_req_app.var=-1.0; 
			far_req_app.num_of_cores=-1;
			far_req_or_sender = -1;*/
			/*far_req_max_man = -1;
			far_req_max_man_cores = -1;
			far_req_max_man_count = -1;*/
		} else if (tmp_inter_list->type == REP_FAR_INIT_REQ){ 
			write_res = write(fd_w, &tmp_inter_list->data.far_req_man, sizeof(int));
			fprintf(log_file, "orig_sender=%d size=%d write_res=%d\n",tmp_inter_list->data.far_req_man,sizeof(int),write_res);
			fflush(log_file);
			my_stats.message_size += sizeof(int);
			//printf("i am %d in asdfwe and far_req_man is %d\n",node_id,tmp_inter_list->data.far_req_man);
			//if (core_inter_head[tmp_inter_list->data.far_req_man] == NULL) printf("kariolares\n");
			if (tmp_inter_list->data.far_req_man != node_id && core_inter_head[tmp_inter_list->data.far_req_man] != NULL && 
				core_inter_head[tmp_inter_list->data.far_req_man]->type == FAR_REQ_MAN_APPOINT) {//next == NULL) {
				kill(pid_num[tmp_inter_list->data.far_req_man], SIG_FAR_REQ);
				fprintf(log_file, "I sent it\n"); 
				fflush(log_file);				
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,tmp_inter_list->data.far_req_man);
			} else {
				fprintf(log_file, "far_man=%d\n",tmp_inter_list->data.far_req_man); 
				fflush(log_file);
			}
		} else if (tmp_inter_list->type == ABORT_FAR_MAN){ 
			write_res = write(fd_w, &tmp_inter_list->data.far_req_man, sizeof(int));
			fprintf(log_file, "far_req_man=%d size=%d write_res=%d\n",tmp_inter_list->data.far_req_man,sizeof(int),write_res);
			fflush(log_file);
			my_stats.message_size += sizeof(int);
			//printf("i am %d in asdfwe and far_req_man is %d\n",node_id,tmp_inter_list->data.far_req_man);
			//if (core_inter_head[tmp_inter_list->data.far_req_man] == NULL) printf("kariolares\n");		
		} else if (tmp_inter_list->type == FAR_REQ_OFFER) {//I am the agent
			//printf("far_list_count = %d\n",far_list_count);
			write_res = write(fd_w, &far_list_count, sizeof(int));
			fprintf(log_file, "far_list_count=%d size=%d write_res=%d\n",far_list_count,sizeof(int),write_res);
			fflush(log_file);	
			my_stats.message_size += sizeof(int);
			tmp_offer_list = far_man_offers;
			while (tmp_offer_list != NULL){
				write_res = write(fd_w, &tmp_offer_list->off.num_of_cores, sizeof(int));
				fprintf(log_file, "num_of_offers=%d size=%d write_res=%d\n",tmp_offer_list->off.num_of_cores,sizeof(int),write_res);
				fflush(log_file);
				my_stats.message_size += sizeof(int);
				write_res = write(fd_w, &tmp_offer_list->off.spd_loss, sizeof(float));
				fprintf(log_file, "spd_loss=%0.2f size=%d write_res=%d\n",tmp_offer_list->off.spd_loss,sizeof(float),write_res);
				fflush(log_file);
				my_stats.message_size += sizeof(float);
				for (i=0; i<tmp_offer_list->off.num_of_cores; i++) {
					//printf ("Offering core = %d\n",tmp_inter_list->data.my_offer.offered_cores[i]);
					write_res = write(fd_w, &tmp_offer_list->off.offered_cores[i], sizeof(int));
					fprintf(log_file, "core=%d size=%d write_res=%d\n",tmp_offer_list->off.offered_cores[i],sizeof(int),write_res);
					fflush(log_file);
					my_stats.message_size += sizeof(int);
				}
				tmp_offer_list = tmp_offer_list->next;
			}

			core_inter_head[sender_id]->type = FAR_REQ_OFFER_SENT;
			clear = 0;
		} else if (tmp_inter_list->type == REP_FAR_REQ_OFFER_SENT) {
			fprintf(log_file, "num_of_offers=%d\n",tmp_inter_list->data.offer_acc_array[0]);
			fflush(log_file);	
			for (i=1; i<=tmp_inter_list->data.offer_acc_array[0]; i++) {
				write_res = write(fd_w, &tmp_inter_list->data.offer_acc_array[i], sizeof(int));	
				fprintf(log_file, "offer_ans=%d size=%d write_res=%d\n",tmp_inter_list->data.offer_acc_array[i],sizeof(int),write_res);
				fflush(log_file);
				my_stats.message_size += sizeof(int);
			}
		} else if (tmp_inter_list->type == INIT_WORK_NODE) {
			if (tmp_inter_list->data.work_time != -1) {
				i=1;
				write_res = write(fd_w, &i, sizeof(int));
				fprintf(log_file, "i=%d size=%d write_res=%d\n",i,sizeof(int),write_res);
				fflush(log_file);
				write_res = write(fd_w, &node_id, sizeof(int));
				fprintf(log_file, "node_id=%d size=%d write_res=%d\n",node_id,sizeof(int),write_res);
				fflush(log_file);
				write_res = write(fd_w, &tmp_inter_list->data.work_time, sizeof(int));
				fprintf(log_file, "work_time=%d size=%d write_res=%d\n",tmp_inter_list->data.work_time,sizeof(int),write_res);
				fflush(log_file);
				my_stats.message_size += 3 * sizeof(int);
			} else {
				i=0;
				write_res = write(fd_w, &i, sizeof(int));
				fprintf(log_file, "i=%d size=%d write_res=%d\n",i,sizeof(int),write_res);
				fflush(log_file);
				my_stats.message_size += sizeof(int);
			}
		} else if (tmp_inter_list->type == APPOINT_WORK_NODE) {
			if (tmp_inter_list->data.work_time != -1) {
				i=1;
				write_res = write(fd_w, &i, sizeof(int));
				fprintf(log_file, "i=%d size=%d write_res=%d\n",i,sizeof(int),write_res);
				fflush(log_file);
				write_res = write(fd_w, &tmp_inter_list->data.work_time, sizeof(int));
				fprintf(log_file, "work_time=%d size=%d write_res=%d\n",tmp_inter_list->data.work_time,sizeof(int),write_res);
				fflush(log_file);						
				my_stats.message_size += 2 * sizeof(int);
			} else {
				i=0;
				write_res = write(fd_w, &i, sizeof(int));
				fprintf(log_file, "i=%d size=%d write_res=%d\n",i,sizeof(int),write_res);
				fflush(log_file);
				my_stats.message_size += sizeof(int);
			}
		} else if (tmp_inter_list->type == REP_CHK_REM_TIME) {
			write_res = write(fd_w, &tmp_inter_list->data.work_time, sizeof(int));
			fprintf(log_file, "work_time=%d size=%d write_res=%d\n",tmp_inter_list->data.work_time,sizeof(int),write_res);
			fflush(log_file);						
			my_stats.message_size += sizeof(int);
			//if (node_id == 28) printf("sender_id = %d a\n",sender_id); 
		} else if (tmp_inter_list->type == REP_STATISTICS) {
			write_res = write(fd_w, &tmp_inter_list->data.stats, sizeof(metrics));
			state = TERMINATED;
			//if (node_id == 28) printf("sender_id = %d a\n",sender_id);
		} else printf("We were fucked inside ACK! node_id = %d sender_id = %d\n",node_id,sender_id);

		//sem_wait(&node_sem[sender_id]);
		sem_getvalue(&node_sem_out[node_id],&i);
		fprintf(log_file, "In ack Trying to acquire semaphore. Sem value =  %d\n",i);
		fflush(log_file);
		sem_wait(&node_sem_out[node_id]);
		sem_getvalue(&node_sem_out[node_id],&i);
		fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
		fflush(log_file);
		close(fd_w);
		free(fifo_name);
		//if (node_id == 28) printf("sender_id = %d b\n",sender_id);	
		if (clear){
			core_inter_head[sender_id] = tmp_inter_list->next;
			if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
			else send_next_signal(core_inter_head[sender_id],sender_id);
			//if (node_id == 28) printf("sender_id = %d c\n",sender_id);
			free(tmp_inter_list);
		}
	} 
	//if (node_id == 28) printf("sender_id = %d d\n",sender_id);
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_ACK_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();	
}

void sig_IDAG_FIND_IDAGS_handler(int signo, siginfo_t *info, void *context)
{
	int fd_r=-1, sender_id, num_of_idags, i, one_idag;
	char *fifo_name;
	region cur_reg;
	inter_list *tmp_inter_list;

	//printf("I came through FIND_IDAGS!! idag_id = %d, node_id = %d\n",idag_id,node_id);
	signals_disable();
	
	sender_id = get_id_from_pid(info->si_pid);
	fifo_name = get_pipe_name(node_id);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_IDAG_FIND_IDAGS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	
		
	if (core_inter_head[sender_id] == NULL || core_inter_head[sender_id]->type == REP_IDAG_FIND_IDAGS){ //I am the idag

		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "In 1 Trying to acquire semaphore. Sem value =  %d\n",i);
		fflush(log_file);
		kill(info->si_pid, SIG_ACK);		
		sem_wait(&node_sem[node_id]);
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
		fflush(log_file);
	
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
							
		//printf("I am inside with sender_id %d %s\n",sender_id,fifo_name);
		//printf("open3 idag_id=%d node_id=%d\n",idag_id,node_id);
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		} /*else {
			fprintf(log_file, "I went through open\n");
			fflush(log_file);
		}	*/
					
		read(fd_r, &cur_reg, sizeof(region));
		//printf("asdI am node with id %d idag_id %d and I am to investigate region C=%d r=%d\n",node_id,idag_id,cur_reg.C,cur_reg.r);
	
		if (core_inter_head[sender_id] == NULL){
			core_inter_head[sender_id] = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[sender_id] = core_inter_head[sender_id];
		} else {
			core_inter_tail[sender_id]->next = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[sender_id] = core_inter_tail[sender_id]->next;
		}

		core_inter_tail[sender_id]->type = REP_IDAG_FIND_IDAGS;
		core_inter_tail[sender_id]->data.idags_in_reg = (int *) malloc((num_idags_x*num_idags_y+1)*sizeof(int));		
		core_inter_tail[sender_id]->next = NULL;
	
		get_reg_idags(cur_reg, core_inter_tail[sender_id]->data.idags_in_reg);
		if (core_inter_head[sender_id]->next == NULL) kill(info->si_pid, SIG_IDAG_FIND_IDAGS);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		//sem_post(&node_sem[node_id]);
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
		fflush(log_file);
		if (sem_post(&node_sem_out[sender_id]) == -1){
			printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
			perror("sem_post");
		}
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "Sem value = %d\n",i);
		fflush(log_file);
		
	} else if (core_inter_head[sender_id]->type == IDAG_FIND_IDAGS && state != IDLE_INIT_MAN && state != WORKING_NODE_IDLE_INIT) {
		//IDLE_INIT_MAN_SELFOPT_PENDING && state != IDLE_INIT_MAN_WORK_PENDING) {
		printf("I am %d and i think i cought a stray init idag_find_idags reply from %d. My current state is %d\n",node_id,sender_id,state);
		fprintf(log_file,"I am %d and i think i cought a stray init idag_find_idags reply from %d. My current state is %d\n",node_id,sender_id,state);
		fflush(log_file);

		kill(info->si_pid, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);		
		/*kill(info->si_pid, SIG_ACK);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		}		
		
		read(fd_r, &num_of_idags, sizeof(int));
		for (i=0; i<num_of_idags; i++) read(fd_r, &one_idag, sizeof(int));
		*/
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		free(tmp_inter_list);
		
		if(core_inter_head[sender_id] == NULL) {
			if (core_inter_tail[sender_id] == NULL){
				printf("Malaka gamietai to core_inter_tail\n");				
				core_inter_tail[sender_id] = NULL;
			}
		} else send_next_signal(core_inter_head[sender_id], sender_id);
	} else if (core_inter_head[sender_id]->type == SELFOPT_IDAG_FIND_IDAGS && state != IDLE_AGENT_WAITING_OFF) {
		printf("I am %d and i think i cought a stray selfopt idag_find_idags reply from %d. My current state is %d\n",node_id,sender_id,state);
		fprintf(log_file,"I am %d and i think i cought a stray selfopt idag_find_idags reply from %d. My current state is %d\n",node_id,sender_id,state);
		fflush(log_file);

		kill(info->si_pid, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		/*kill(info->si_pid, SIG_ACK);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		}		
		
		read(fd_r, &num_of_idags, sizeof(int));
		for (i=0; i<num_of_idags; i++) read(fd_r, &one_idag, sizeof(int));
		*/
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		free(tmp_inter_list);
		
		if(core_inter_head[sender_id] == NULL) {
			if (core_inter_tail[sender_id] == NULL){
				printf("Malaka gamietai to core_inter_tail\n");				
				core_inter_tail[sender_id] = NULL;
			}
		} else send_next_signal(core_inter_head[sender_id], sender_id);
	} else if (core_inter_head[sender_id]->type == IDAG_FIND_IDAGS || core_inter_head[sender_id]->type == SELFOPT_IDAG_FIND_IDAGS){ //I am the requesting common node
		//printf("qweI am inside i am node with node id %d idag_id %d and sender_id %d %s\n",node_id,idag_id,sender_id,fifo_name);		
		sem_getvalue(&node_sem[node_id],&i);		
		fprintf(log_file, "In 2 Trying to acquire semaphore. Sem value = %d\n",i);
		fflush(log_file);
		kill(info->si_pid, SIG_ACK);		
		sem_wait(&node_sem[node_id]);
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
		fflush(log_file);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		//printf("open4 idag_id=%d node_id=%d\n",idag_id,node_id);		
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		}	/*else {
			fprintf(log_file, "I went through open\n");
			fflush(log_file);
		}	*/	
		
		read(fd_r, &num_of_idags, sizeof(int));
		fprintf(log_file,"Number of agents in region %d\n",num_of_idags);
		fflush(log_file);
		//if (core_inter_head[sender_id]->type == FAR_INIT_IDAG_FIND_IDAGS) far_req_max_man_count = num_of_idags;
		//else 
		if (core_inter_head[sender_id]->type == IDAG_FIND_IDAGS) {
			init_idags_areas_replies++;			
			init_DDS_idags += num_of_idags;		
		} else if (core_inter_head[sender_id]->type == SELFOPT_IDAG_FIND_IDAGS)	selfopt_DDS_idags += num_of_idags;
		//if (node_id == 14) printf("Number of idags in region = %d\n",num_of_idags);
		for (i=0; i<num_of_idags; i++){
			read(fd_r, &one_idag, sizeof(int));
			fprintf(log_file,"In the region I have idag with id %d\n",one_idag);
			fflush(log_file);

			if (core_inter_head[one_idag] == NULL){
				core_inter_head[one_idag] = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[one_idag] = core_inter_head[one_idag];
			} else {
				for (tmp_inter_list = core_inter_head[one_idag]; tmp_inter_list != NULL; tmp_inter_list = tmp_inter_list->next)
					if (tmp_inter_list->type == FAR_REQ_MAN) break;	

				if (tmp_inter_list != NULL) {
					fprintf(log_file,"I dismissed node %d in rep_idag_find_idags\n",one_idag);
					fflush(log_file);
					init_DDS_idags--;
					continue;
				}
				
				core_inter_tail[one_idag]->next = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[one_idag] = core_inter_tail[one_idag]->next;
			}

			if (core_inter_head[sender_id]->type == IDAG_FIND_IDAGS) core_inter_tail[one_idag]->type = IDAG_REQ_DDS_PENDING;
			else if (core_inter_head[sender_id]->type == FAR_REQ_IDAG_FIND_IDAGS) core_inter_tail[one_idag]->type = FAR_REQ_IDAG_REQ_DDS_PENDING;
			else core_inter_tail[one_idag]->type = SELFOPT_IDAG_REQ_DDS_PENDING;
			
			core_inter_tail[one_idag]->data.reg.C = core_inter_head[sender_id]->data.reg.C;
			core_inter_tail[one_idag]->data.reg.r = core_inter_head[sender_id]->data.reg.r;
			core_inter_tail[one_idag]->next = NULL;

			if (core_inter_head[one_idag]->next == NULL) {
				kill(pid_num[one_idag], SIG_REQ_DDS); //newly created
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,one_idag);
			} else {
				fprintf(log_file,"I did not sent req_dds to %d with interaction = %d inter 2=%d\n",one_idag,core_inter_head[one_idag]->type,core_inter_head[one_idag]->next->type);
				fflush(log_file);
			}
		}

		//sem_post(&node_sem[node_id]);
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
		fflush(log_file);
		if (sem_post(&node_sem_out[sender_id]) == -1){
			printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
			perror("sem_post");
		}
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "Sem value = %d\n",i);
		fflush(log_file);

		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);		
		free(tmp_inter_list);		

	} else if (core_inter_head[sender_id]->type == IDAG_FIND_IDAGS_PENDING || core_inter_head[sender_id]->type == SELFOPT_IDAG_FIND_IDAGS_PENDING){
		printf("I am %d and deadlock was prevented in sig_find_idags\n",node_id);	
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		free(tmp_inter_list);
		
		if(core_inter_head[sender_id] == NULL) {
			if (core_inter_tail[sender_id] == NULL){
				printf("Malaka gamietai to core_inter_tail\n");				
				core_inter_tail[sender_id] = NULL;
			}
		} else send_next_signal(core_inter_head[sender_id], sender_id);		
	} else {
		printf("I have to reject sig_IDAG_FIND_IDAGS_handler sender_id=%d node_id=%d interaction=%d\n",sender_id,node_id,core_inter_head[sender_id]->type);
		fprintf(log_file,"I have to reject sig_IDAG_FIND_IDAGS_handler sender_id=%d interaction=%d\n",sender_id,core_inter_head[sender_id]->type);
		fflush(log_file);

		kill(info->si_pid, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	}

	if (fd_r != -1) close(fd_r);
	free(fifo_name);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_IDAG_FIND_IDAGS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

void sig_REQ_DDS_handler(int signo, siginfo_t *info, void *context)
{
	int fd_r=-1, sender_id, num_of_agents, i, agent_id, num_of_cores,j;
	char *fifo_name;
	region cur_reg;
	core_list *tmp_cores_list;
	inter_list *tmp_inter_list;
	DDS_list *tmp_DDS;
	target_list *tmp_target_list;	

	signals_disable();
	
	sender_id = get_id_from_pid(info->si_pid);
	fifo_name = get_pipe_name(node_id);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_REQ_DDS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	
	
	if (core_inter_head[sender_id] == NULL){ //I am the idag
		//printf("I am %d and %d requested DDS\n",idag_id,sender_id);
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "In 3 Trying to acquire semaphore. Sem value =  %d\n",i);
		fflush(log_file);
		kill(info->si_pid, SIG_ACK);		
		sem_wait(&node_sem[node_id]);
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
		fflush(log_file);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		//printf("open5 idag_id=%d node_id=%d\n",idag_id,node_id);		
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		}	/*else {
			fprintf(log_file, "I went through open\n");
			fflush(log_file);
		}	*/
		read(fd_r, &cur_reg, sizeof(region));
		//sem_post(&node_sem[sender_id]);
		/*while (cur_reg.C < 0) {
			printf("I am %d and i got shit region (%d,%d) in req_dds from %d\n",node_id,cur_reg.C,cur_reg.r,sender_id);
			read(fd_r, &cur_reg, sizeof(region));
		}*/

		fprintf(log_file,"I am to investigate region C=%d r=%d for %d\n",cur_reg.C,cur_reg.r,sender_id);		
		fflush(log_file);

		if (core_inter_head[sender_id] == NULL){
			core_inter_head[sender_id] = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[sender_id] = core_inter_head[sender_id];
		} else {
			printf("Ton hpiame\n");
			core_inter_tail[sender_id]->next = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[sender_id] = core_inter_tail[sender_id]->next;
		}

		core_inter_tail[sender_id]->type = REP_IDAG_REQ_DDS;
		core_inter_tail[sender_id]->next = NULL;

		if (cur_reg.C > -1) {//gia na zhtaei o node 0 dds
			core_inter_tail[sender_id]->data.agents_in_reg = (int *) malloc((2*DDS_count+1)*sizeof(int));
			core_inter_tail[sender_id]->data.agents_in_reg[0] = 0;		
			for (i=1; i<=2*DDS_count; i+=2){
				core_inter_tail[sender_id]->data.agents_in_reg[i] = -1;
				core_inter_tail[sender_id]->data.agents_in_reg[i+1] = 0;
			}
		
			tmp_cores_list = my_cores;
			while (tmp_cores_list != NULL){
				if (distance(tmp_cores_list->core_id, cur_reg.C) <= cur_reg.r){
					if (tmp_cores_list->offered_to == -1) agent_id=node_id;
					else {
						agent_id=tmp_cores_list->offered_to;	
						for (tmp_DDS = DDS->next; tmp_DDS!=NULL; tmp_DDS=tmp_DDS->next) if (tmp_DDS->agent_id == agent_id) break;
						if (tmp_DDS == NULL) agent_id = node_id;
					}

					for (i=1; i<=2*DDS_count; i+=2)
						if (core_inter_tail[sender_id]->data.agents_in_reg[i] == agent_id) break;
						else if (core_inter_tail[sender_id]->data.agents_in_reg[i] == -1){
							core_inter_tail[sender_id]->data.agents_in_reg[0]++;
							core_inter_tail[sender_id]->data.agents_in_reg[i] = agent_id;
							break;
						}
					core_inter_tail[sender_id]->data.agents_in_reg[i+1]++;
				}

				tmp_cores_list = tmp_cores_list->next;
			}
		} else core_inter_tail[sender_id]->data.agents_in_reg = NULL;
 
		kill(info->si_pid, SIG_REQ_DDS);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		//sem_post(&node_sem[node_id]);
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
		fflush(log_file);
		if (sem_post(&node_sem_out[sender_id]) == -1){
			printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
			perror("sem_post");
		}
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "Sem value = %d\n",i);
		fflush(log_file);
	} else if (core_inter_head[sender_id]->type == IDAG_REQ_DDS && state != IDLE_INIT_MAN && state != WORKING_NODE_IDLE_INIT) {
		//IDLE_INIT_MAN_SELFOPT_PENDING && state != IDLE_INIT_MAN_WORK_PENDING) {
		printf("I am %d and i think i cought a stray init IDAG_REQ_DDS reply from %d. My current state is %d\n",node_id,sender_id,state);
		fprintf(log_file,"I am %d and i think i cought a stray init IDAG_REQ_DDS reply from %d. My current state is %d\n",node_id,sender_id,state);
		fflush(log_file);

		kill(info->si_pid, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		/*kill(info->si_pid, SIG_ACK);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		}		
		
		read(fd_r, &num_of_agents, sizeof(int));
		for (i=0; i<num_of_agents; i++){
			read(fd_r, &agent_id, sizeof(int));
			read(fd_r, &num_of_cores, sizeof(int));
		}
		*/
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		free(tmp_inter_list);
		
		if(core_inter_head[sender_id] == NULL) {
			if (core_inter_tail[sender_id] == NULL){
				printf("Malaka gamietai to core_inter_tail\n");				
				core_inter_tail[sender_id] = NULL;
			}
		} else send_next_signal(core_inter_head[sender_id], sender_id);
	} else if (core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS && state != IDLE_AGENT_WAITING_OFF) {
		printf("I am %d and i think i cought a stray selfopt SELFOPT_IDAG_REQ_DDS reply from %d. My current state is %d\n",node_id,sender_id,state);
		fprintf(log_file,"I am %d and i think i cought a stray selfopt SELFOPT_IDAG_REQ_DDS reply from %d. My current state is %d\n",node_id,sender_id,state);		
		fflush(log_file);

		kill(info->si_pid, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		/*kill(info->si_pid, SIG_ACK);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		}		
		
		read(fd_r, &num_of_agents, sizeof(int));
		for (i=0; i<num_of_agents; i++){
			read(fd_r, &agent_id, sizeof(int));
			read(fd_r, &num_of_cores, sizeof(int));
		}
		*/
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		free(tmp_inter_list);
		
		if(core_inter_head[sender_id] == NULL) {
			if (core_inter_tail[sender_id] == NULL){
				printf("Malaka gamietai to core_inter_tail\n");				
				core_inter_tail[sender_id] = NULL;
			}
		} else send_next_signal(core_inter_head[sender_id], sender_id);
	} else if (core_inter_head[sender_id]->type == IDAG_REQ_DDS || core_inter_head[sender_id]->type == FAR_REQ_IDAG_REQ_DDS || core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS){ //I am the requesting common node
		//printf("vbnI am inside i am node with node id %d and sender_id %d\n",node_id,sender_id);
		sem_getvalue(&node_sem[node_id],&i);		
		fprintf(log_file, "In 4 Trying to acquire semaphore. Sem value =  %d\n",i);
		fflush(log_file);
		kill(info->si_pid, SIG_ACK);		
		sem_wait(&node_sem[node_id]);
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
		fflush(log_file);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		//printf("open5 idag_id=%d node_id=%d\n",idag_id,node_id);		
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		}	/*else {
			fprintf(log_file, "I went through open\n");
			fflush(log_file);
		}	*/	
		
		if (core_inter_head[sender_id]->type == IDAG_REQ_DDS) {
			init_DDS_replies++;
			fprintf(log_file,"init_DDS_idags = %d, init_DDS_replies = %d init_idags_areas_replies=%d\n",init_DDS_idags,init_DDS_replies,init_idags_areas_replies);
			fflush(log_file);
		} else if (core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS) {
			selfopt_DDS_replies++;
			fprintf(log_file,"selfopt_DDS_idags = %d, selfopt_DDS_replies = %d\n",selfopt_DDS_idags,selfopt_DDS_replies);
			fflush(log_file);		
		}		
		read(fd_r, &num_of_agents, sizeof(int));
		while (num_of_agents > X_max*Y_max) {
			printf("I am %d and in req_dds i got shit num_of_agents %d from %d\n",node_id,num_of_agents,sender_id);	
			read(fd_r, &num_of_agents, sizeof(int));
		}
		//if (core_inter_head[sender_id]->type == FAR_REQ_IDAG_REQ_DDS)	far_man_agent_count += num_of_agents;	
		//printf("Number of agents in region = %d\n",num_of_agents);
		//fprintf(log_file, "Number of agents in region = %d of %d init_DDS_replies = %d init_DDS_idags = %d\n",num_of_agents,sender_id,init_DDS_replies,init_DDS_idags);
		fprintf(log_file, "Number of agents in region = %d of %d reg = (%d,%d)\n",num_of_agents,sender_id,core_inter_head[sender_id]->data.reg.C,core_inter_head[sender_id]->data.reg.r);
		fflush(log_file);

		for (i=0; i<num_of_agents; i++) {
			read(fd_r, &agent_id, sizeof(int));
			read(fd_r, &num_of_cores, sizeof(int));
			//printf("zxc I am %d and In the DDS of %d there is an agent with id %d and %d cores\n",node_id,sender_id,agent_id,num_of_cores);
			fprintf(log_file, "there is an agent with id %d and %d cores\n",agent_id,num_of_cores);
			fflush(log_file);
			if (agent_id == node_id) continue;//((node_id == -1 && agent_id == idag_id) || node_id == agent_id) 

			if (core_inter_head[sender_id]->type == IDAG_REQ_DDS){
				tmp_target_list = init_targets_head;
				while (tmp_target_list != NULL && tmp_target_list->target != agent_id) 
					//if (tmp_target_list->target != agent_id) 
					tmp_target_list = tmp_target_list->next; 

				if (tmp_target_list == NULL) {
					if (init_targets_head == NULL) {
						init_targets_head = (target_list *) malloc(sizeof(target_list));
						init_targets_tail = init_targets_head;;
					} else {
						init_targets_tail->next = (target_list *) malloc(sizeof(target_list));
						init_targets_tail =	init_targets_tail->next;
					}

					//fprintf(log_file, "in here it begins a\n");
					//fflush(log_file);
					init_targets_tail->next = NULL;
					init_targets_tail->target = agent_id;
					init_targets_tail->num_of_regions = 1;
					//init_targets_tail->region_arr = (region *) malloc(INIT_AREAS_NUM * sizeof(region));	
					init_targets_tail->region_arr[0] = core_inter_head[sender_id]->data.reg;
					//fprintf(log_file, "in here it begins b\n");
					//fflush(log_file);
				} else {
					fprintf(log_file, "in here num_of_regions = %d\n",tmp_target_list->num_of_regions);
					fflush(log_file);
					for (j=0; j<tmp_target_list->num_of_regions; j++)
						if (tmp_target_list->region_arr[j].C == core_inter_head[sender_id]->data.reg.C && tmp_target_list->region_arr[j].r == core_inter_head[sender_id]->data.reg.r){
							fprintf(log_file, "fucking area allready exists\n");
							fflush(log_file);							
							break;
						}
					
					if (j == tmp_target_list->num_of_regions) {							
						tmp_target_list->region_arr[tmp_target_list->num_of_regions++] = core_inter_head[sender_id]->data.reg;
						//fprintf(log_file, "new area added\n");
						//fflush(log_file);
					}
				}
			} else if (core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS){
				tmp_target_list = selfopt_targets_head;
				while (tmp_target_list != NULL && tmp_target_list->target != agent_id)  
					tmp_target_list = tmp_target_list->next; 

				if (tmp_target_list == NULL) {
					if (selfopt_targets_head == NULL) {
						selfopt_targets_head = (target_list *) malloc(sizeof(target_list));
						selfopt_targets_tail = selfopt_targets_head;;
					} else {
						selfopt_targets_tail->next = (target_list *) malloc(sizeof(target_list));
						selfopt_targets_tail = selfopt_targets_tail->next;
					}

					selfopt_targets_tail->next = NULL;
					selfopt_targets_tail->target = agent_id;
					selfopt_targets_tail->num_of_regions = 1;
					selfopt_targets_tail->region_arr[0] = core_inter_head[sender_id]->data.reg;
				} else {
					for (j=0; j<tmp_target_list->num_of_regions; j++)
						if (tmp_target_list->region_arr[j].C == core_inter_head[sender_id]->data.reg.C && tmp_target_list->region_arr[j].r == core_inter_head[sender_id]->data.reg.r){
							break;
						}
					
					if (j == tmp_target_list->num_of_regions) 							
						tmp_target_list->region_arr[tmp_target_list->num_of_regions++] = core_inter_head[sender_id]->data.reg;

				}	//tmp_target_list->region_arr[tmp_target_list->num_of_regions++] = core_inter_head[sender_id]->data.reg;
			} else if (core_inter_head[sender_id]->type == FAR_REQ_IDAG_REQ_DDS){
				if (core_inter_head[agent_id] == NULL){
					core_inter_head[agent_id] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[agent_id] = core_inter_head[agent_id];
				} else {
					core_inter_tail[agent_id]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[agent_id] = core_inter_tail[agent_id]->next;
				}

				//if (core_inter_head[sender_id]->type == IDAG_REQ_DDS) core_inter_tail[agent_id]->type = AGENT_REQ_CORES_PENDING;
				//else if (core_inter_head[sender_id]->type == FAR_REQ_IDAG_REQ_DDS) core_inter_tail[agent_id]->type = FAR_REQ_CORES;				
				//else core_inter_tail[agent_id]->type = SELFOPT_REQ_CORES_PENDING;
	 			
				core_inter_tail[agent_id]->type = FAR_REQ_CORES_PENDING;		
				core_inter_tail[agent_id]->data.reg = core_inter_head[sender_id]->data.reg;
				core_inter_tail[agent_id]->next = NULL;
			
				if (core_inter_head[agent_id]->next == NULL) {
					kill(pid_num[agent_id], SIG_REQ_CORES); //newly created, not an idag
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,agent_id);
				}
				//else printf("It is fucked here interaction= %d\n",core_inter_head[agent_id]->type);
			} /*else if (far_req_max_man_cores == -1 || far_req_max_man_cores<num_of_cores){
						far_req_max_man_cores = num_of_cores;
						far_req_max_man = agent_id;
			}*/
		}
		//sem_post(&node_sem[sender_id]);	
		/*if (core_inter_head[sender_id]->type == FAR_INIT_IDAG_REQ_DDS)
			if (--far_req_max_man_count == 0){//all idags have replied
				printf("Agent with max cores in region is %d\n",far_req_max_man);

				if (far_req_max_man == node_id)// || (node_id == -1 && far_req_max_man == idag_id)) //I am the one
					printf("I am the one!!\n");
					//completely inclomplete apparently
				else {
					if (core_inter_head[far_req_max_man] == NULL){
						core_inter_head[far_req_max_man] = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[far_req_max_man] = core_inter_head[far_req_max_man];
					} else {
						core_inter_tail[far_req_max_man]->next = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[far_req_max_man] = core_inter_tail[far_req_max_man]->next;
					}

					core_inter_tail[far_req_max_man]->type = FAR_REQ_MAN_APPOINT;
					core_inter_tail[far_req_max_man]->data.reg = core_inter_head[sender_id]->data.reg;
					core_inter_tail[far_req_max_man]->next = NULL;
					
					//dont't kill here for sychronisation reasons, original manager should be informed for the change in his far manager
					//if (core_inter_head[far_req_max_man]->next == NULL) kill(pid_num[far_req_max_man], SIG_FAR_REQ);

					if (core_inter_head[far_req_or_sender] == NULL){
						core_inter_head[far_req_or_sender] = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[far_req_or_sender] = core_inter_head[far_req_or_sender];
					} else {
						core_inter_tail[far_req_or_sender]->next = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[far_req_or_sender] = core_inter_tail[far_req_or_sender]->next;
					}

					core_inter_tail[far_req_or_sender]->type = REP_FAR_INIT_REQ;
					core_inter_tail[far_req_or_sender]->data.far_req_man = far_req_max_man;
					core_inter_tail[far_req_or_sender]->next = NULL;

					if (core_inter_head[far_req_or_sender]->next == NULL) {
						kill(pid_num[far_req_or_sender], SIG_INIT_FAR_REQ);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,far_req_or_sender);
					}
				}
			}*/

		if (core_inter_head[sender_id]->type == IDAG_REQ_DDS && init_DDS_replies == init_DDS_idags && init_idags_areas_replies == INIT_AREAS_NUM) {//INIT_AREAS_NUM
			for (tmp_target_list = init_targets_head; tmp_target_list != NULL; tmp_target_list = tmp_target_list->next){
				agent_id = tmp_target_list->target;

				for (tmp_inter_list = core_inter_head[agent_id]; tmp_inter_list != NULL; tmp_inter_list=tmp_inter_list->next)
					if (tmp_inter_list->type == FAR_INIT_REQ || tmp_inter_list->type == FAR_REQ_MAN) break; 

				if (tmp_inter_list == NULL) {
					if (core_inter_head[agent_id] == NULL){
						core_inter_head[agent_id] = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[agent_id] = core_inter_head[agent_id];
					} else {
						core_inter_tail[agent_id]->next = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[agent_id] = core_inter_tail[agent_id]->next;
					}
	
					core_inter_tail[agent_id]->type = AGENT_REQ_CORES_PENDING;
					core_inter_tail[agent_id]->data.reg_arr.num_of_regions = tmp_target_list->num_of_regions;
					core_inter_tail[agent_id]->data.reg_arr.region_arr = (region *) malloc(tmp_target_list->num_of_regions * sizeof(region));
		
					if (tmp_target_list->target == node_id) printf("Why is this here node_id=%d\n",node_id);
					fprintf(log_file,"Init req target = %d, num_of_regions = %d. Αreas:",tmp_target_list->target,tmp_target_list->num_of_regions);
					for (i=0; i<tmp_target_list->num_of_regions; i++) {
						core_inter_tail[agent_id]->data.reg_arr.region_arr[i] = tmp_target_list->region_arr[i];
						fprintf(log_file," (%d,%d),",tmp_target_list->region_arr[i].C,tmp_target_list->region_arr[i].r);
					}
					fprintf(log_file,"\n");
					fflush(log_file);
					core_inter_tail[agent_id]->next = NULL;

					//kill(pid_num[agent_id], SIG_REQ_CORES);
					if (core_inter_head[agent_id]->next == NULL) {
						kill(pid_num[agent_id], SIG_REQ_CORES);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,agent_id);
					} else if (agent_id == sender_id && core_inter_head[agent_id]->next->type == AGENT_REQ_CORES_PENDING) { //den exei fugei apo to free alla einai wra na stalei
						kill(pid_num[agent_id], SIG_REQ_CORES);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,agent_id);
					} else printf("This init fucker is not NULL interaction=%d interaction2=%d\n",core_inter_head[agent_id]->type,core_inter_head[agent_id]->next->type);
				} else 
					printf("I am %d and i did not send local requests to %d because he is my far manager\n",node_id,agent_id);
			}

			its.it_value.tv_nsec = INIT_NODE_INTERVAL * MS;//750000000;// * MS;
			if (timer_settime(timerid, 0, &its, NULL) == -1) printf("timer_settime error4\n");	 
		} else if (core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS && selfopt_DDS_replies == selfopt_DDS_idags) {
			for (tmp_target_list = selfopt_targets_head; tmp_target_list != NULL; tmp_target_list = tmp_target_list->next){
				agent_id = tmp_target_list->target;
				if (core_inter_head[agent_id] == NULL){
					core_inter_head[agent_id] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[agent_id] = core_inter_head[agent_id];
				} else {
					core_inter_tail[agent_id]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[agent_id] = core_inter_tail[agent_id]->next;
				}
	
				core_inter_tail[agent_id]->type = SELFOPT_REQ_CORES_PENDING;
				core_inter_tail[agent_id]->data.reg_arr.num_of_regions = tmp_target_list->num_of_regions;
				core_inter_tail[agent_id]->data.reg_arr.region_arr = (region *) malloc(tmp_target_list->num_of_regions * sizeof(region));
				if (tmp_target_list->target == node_id) printf("selfopt Why is this here node_id=%d\n",node_id);
				fprintf(log_file,"Selfopt req target = %d, num_of_regions = %d. Αreas:",tmp_target_list->target,tmp_target_list->num_of_regions);
				for (i=0; i<tmp_target_list->num_of_regions; i++) {
					core_inter_tail[agent_id]->data.reg_arr.region_arr[i] = tmp_target_list->region_arr[i];
					fprintf(log_file," (%d,%d),",tmp_target_list->region_arr[i].C,tmp_target_list->region_arr[i].r);
				}
				fprintf(log_file,"\n");
				fflush(log_file);				
				core_inter_tail[agent_id]->next = NULL;

				if (core_inter_head[agent_id]->next == NULL) {
					kill(pid_num[agent_id], SIG_REQ_CORES);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,agent_id);
				} else if (agent_id == sender_id && core_inter_head[agent_id]->next->type == SELFOPT_REQ_CORES_PENDING) {
					kill(pid_num[agent_id], SIG_REQ_CORES);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,agent_id);
				} else printf("This selfopt fucker is not NULL interaction=%d interaction2=%d\n",core_inter_head[agent_id]->type,core_inter_head[agent_id]->next->type);
			}

			its.it_value.tv_nsec = 500 * MS;// 750000000;
			selfopt_time_rem = its.it_value.tv_nsec;
			if (timer_settime(timerid, 0, &its, NULL) == -1) printf("timer_settime error7\n");
				
			if (state != IDLE_AGENT_WAITING_OFF) {
				printf("I am %d and about to set my alarm for selfopt check and my state before that was %d\n",node_id,state);
				state = IDLE_AGENT_WAITING_OFF;
			}
		}	
		//printf("I come here\n");
		//sem_post(&node_sem[node_id]);
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
		fflush(log_file);
		if (sem_post(&node_sem_out[sender_id]) == -1){
			printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
			perror("sem_post");
		}
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "Sem value = %d\n",i);
		fflush(log_file);
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else if (core_inter_head[sender_id]->type != AGENT_REQ_CORES_PENDING && core_inter_head[sender_id]->type != SELFOPT_REQ_CORES_PENDING)//far_req_max_man != sender_id && 
			send_next_signal(core_inter_head[sender_id], sender_id);
		free(tmp_inter_list);		
		//printf("I come here as well\n");
	} else if (core_inter_head[sender_id]->type == DEBUG_IDAG_REQ_DDS){ //I am the requesting common node
		//printf("I am inside i am node with node id %d and sender_id %d %s\n",node_id,sender_id,fifo_name);		
		sem_getvalue(&node_sem[node_id],&i);		
		fprintf(log_file, "In DEBUG_IDAG_REQ_DDS Trying to acquire semaphore. Sem value =  %d\n",i);
		fflush(log_file);
		kill(info->si_pid, SIG_ACK);		
		sem_wait(&node_sem[node_id]);
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
		fflush(log_file);		
		//kill(info->si_pid, SIG_ACK);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		//printf("open6 idag_id=%d node_id=%d\n",idag_id,node_id);		
		fd_r = open(fifo_name, O_RDONLY);
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		}	/*else {
			fprintf(log_file, "I went through open\n");
			fflush(log_file);
		}	*/	
		
		read(fd_r, &num_of_agents, sizeof(int));
		printf("\n");
		printf("Number of agents in region = %d\n",num_of_agents);
		for (i=0; i<num_of_agents; i++){
			read(fd_r, &agent_id, sizeof(int));
			read(fd_r, &num_of_cores, sizeof(int));

			printf("Agent no %d is %d with %d cores\n",i,agent_id,num_of_cores);	
		}

		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
		fflush(log_file);
		if (sem_post(&node_sem_out[sender_id]) == -1){
			printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
			perror("sem_post");
		}
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "Sem value = %d\n",i);
		fflush(log_file);

		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);		
		free(tmp_inter_list);
	} else if (core_inter_head[sender_id]->type == IDAG_REQ_DDS_PENDING || core_inter_head[sender_id]->type == FAR_REQ_IDAG_REQ_DDS_PENDING 
		|| core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS_PENDING){

		printf("I am %d and deadlock was prevented in sig_req_dds\n",node_id);	
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		free(tmp_inter_list);
		
		if(core_inter_head[sender_id] == NULL) {
			if (core_inter_tail[sender_id] == NULL){
				printf("Malaka gamietai to core_inter_tail\n");				
				core_inter_tail[sender_id] = NULL;
			}
		} else send_next_signal(core_inter_head[sender_id], sender_id);		
	} else {
		//printf("I am %d and We were fucked in sig_REQ_DDS_handler from %d interaction is = %d\n",node_id,sender_id,core_inter_head[sender_id]->type);
		printf("I am %d and i have to reject req_dds from %d. Our interaction is %d\n",node_id,sender_id,core_inter_head[sender_id]->type);
		fprintf(log_file,"I am %d and i have to reject req_dds from %d. Our interaction is %d\n",node_id,sender_id,core_inter_head[sender_id]->type);
		fflush(log_file);

		/*kill(info->si_pid, SIG_ACK);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		}		
		
		read(fd_r, &num_of_agents, sizeof(int));
		for (i=0; i<num_of_agents; i++){
			read(fd_r, &agent_id, sizeof(int));
			read(fd_r, &num_of_cores, sizeof(int));
		}*/

		kill(info->si_pid, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	}

	if (fd_r != -1) close(fd_r);

	free(fifo_name);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_REQ_DDS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

void sig_REQ_CORES_handler(int signo, siginfo_t *info, void *context)
{
	int fd_r=-1, sender_id, i, tmp_int, num_of_offers, j;//num_of_idags, i, one_idag;
	char *fifo_name;
	float req_gain;
	region cur_reg;
	app req_app;
	offer one_offer;
	offer_list *tmp_offer_list, *tmp_offer_prev = NULL, *tmp_head, *chosen_node;
	inter_list *tmp_inter_list;	
	//printf("I came through REQ!CORES! idag_id = %d, node_id = %d\n",idag_id,node_id);
	signals_disable();
	
	sender_id = get_id_from_pid(info->si_pid);
	fifo_name = get_pipe_name(node_id);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_REQ_CORES_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	
		
	if (core_inter_head[sender_id] == NULL && my_cores != NULL){ //I am the agent && (my_idag == -1 || (my_idag != -1 && my_app.num_of_cores != -1))
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "In 5 Trying to acquire semaphore. Sem value =  %d\n",i);
		fflush(log_file);
		kill(info->si_pid, SIG_ACK);		
		sem_wait(&node_sem[node_id]);
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
		fflush(log_file);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);	
		//printf("I am inside with sender_id %d %s\n",sender_id,fifo_name);
		//printf("open7 idag_id=%d node_id=%d\n",idag_id,node_id);
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		} /*else {
			fprintf(log_file, "I went through open\n");
			fflush(log_file);
		}	*/
					
		read(fd_r, &req_app, sizeof(app));
		//printf("I am node with id %d idag id %d with sender id %d and I am to investigate region C=%d r=%d and app %f\n",node_id,idag_id,sender_id,cur_reg.C,cur_reg.r,req_app.A);
	
		if (core_inter_head[sender_id] == NULL){
			core_inter_head[sender_id] = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[sender_id] = core_inter_head[sender_id];
		} else {
			core_inter_tail[sender_id]->next = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[sender_id] = core_inter_tail[sender_id]->next;
		}

		core_inter_tail[sender_id]->type = REP_AGENT_REQ_CORES;
		read(fd_r, &core_inter_tail[sender_id]->data.off_arr.num_of_offers, sizeof(int));
		while (core_inter_tail[sender_id]->data.off_arr.num_of_offers > INIT_AREAS_NUM) {
			printf("I am %d kai fagame skoupidia apo ton %d kai einai %d\n",node_id,sender_id,core_inter_tail[sender_id]->data.off_arr.num_of_offers);
			read(fd_r, &core_inter_tail[sender_id]->data.off_arr.num_of_offers, sizeof(int));
		}
		core_inter_tail[sender_id]->data.off_arr.offer_arr = (offer *) malloc(core_inter_tail[sender_id]->data.off_arr.num_of_offers * sizeof(offer));
		core_inter_tail[sender_id]->next = NULL;

		//if (node_id == 14) printf("Sender is %d and num_of_offers = %d\n",sender_id,core_inter_tail[sender_id]->data.off_arr.num_of_offers);	

		for (i=0; i<core_inter_tail[sender_id]->data.off_arr.num_of_offers; i++){
			read(fd_r, &cur_reg, sizeof(region));
			
			if (my_idag == -1) {
				core_inter_tail[sender_id]->data.off_arr.offer_arr[i].offered_cores = (int *) malloc(my_cores_count*sizeof(int));		
			
				tmp_int = offer_cores(my_cores, req_app, cur_reg, core_inter_tail[sender_id]->data.off_arr.offer_arr[i].offered_cores, sender_id);
				core_inter_tail[sender_id]->data.off_arr.offer_arr[i].num_of_cores = tmp_int;
				my_stats.comp_effort++;

				core_inter_tail[sender_id]->data.off_arr.offer_arr[i].spd_loss = 0.0;
			} else if (my_cores != NULL && my_cores_count>2) {
				core_inter_tail[sender_id]->data.off_arr.offer_arr[i].offered_cores = (int *) malloc(my_cores_count*sizeof(int));		
			
				tmp_int = offer_cores(my_cores, req_app, cur_reg, core_inter_tail[sender_id]->data.off_arr.offer_arr[i].offered_cores, sender_id);
				core_inter_tail[sender_id]->data.off_arr.offer_arr[i].num_of_cores = tmp_int;
				my_stats.comp_effort++;
				//printf("I am %d in i = %d and i offer %d cores\n",node_id,i,tmp_int);

				core_inter_tail[sender_id]->data.off_arr.offer_arr[i].spd_loss = Speedup(my_app, my_cores_count) - Speedup(my_app, my_cores_count-tmp_int);
				req_gain = Speedup(req_app,req_app.num_of_cores+tmp_int) - Speedup(req_app,req_app.num_of_cores);
				if (tmp_int > 0) {
					fprintf(log_file,"I offered %d cores with spd_loss = %0.2f and %0.2f gain for the req_app\n",tmp_int,core_inter_tail[sender_id]->data.off_arr.offer_arr[i].spd_loss,req_gain);
					fflush(log_file);
					fprintf(app_log_file,"I offered %d cores with spd_loss = %0.2f and %0.2f gain for the req_app\n"
						,tmp_int,core_inter_tail[sender_id]->data.off_arr.offer_arr[i].spd_loss,req_gain);
					fflush(app_log_file);
				}			
			} else {
				core_inter_tail[sender_id]->data.off_arr.offer_arr[i].num_of_cores = 0;
				core_inter_tail[sender_id]->data.off_arr.offer_arr[i].spd_loss = 0.0;
			} 

			fprintf(log_file, "I offered %d %d cores: ",sender_id,core_inter_tail[sender_id]->data.off_arr.offer_arr[i].num_of_cores);
			for (j=0; j<core_inter_tail[sender_id]->data.off_arr.offer_arr[i].num_of_cores; j++)
				fprintf(log_file, "%d, ",core_inter_tail[sender_id]->data.off_arr.offer_arr[i].offered_cores[j]);
			fprintf(log_file, "\n");
			fflush(log_file);
		}
		//sem_post(&node_sem[sender_id]);

		//get_reg_idags(cur_reg, core_inter_tail[sender_id]->data.idags_in_reg);
		if (core_inter_head[sender_id]->next == NULL) {
			kill(info->si_pid, SIG_REQ_CORES);
			my_stats.msg_count++;
			my_stats.distance += distance(node_id,sender_id);
		} else printf("Apparently not null interaction=%d\n",core_inter_head[sender_id]->type);
		//sem_post(&node_sem[node_id]);
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
		fflush(log_file);
		if (sem_post(&node_sem_out[sender_id]) == -1){
			printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
			perror("sem_post");
		}
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "Sem value = %d\n",i);
		fflush(log_file);
	} else if (core_inter_head[sender_id] == NULL) {
		printf("I am %d and i have to reject req_cores from %d with null interaction\n",node_id,sender_id);
		fprintf(log_file,"i have to reject req_cores from %d. with null interaction\n",sender_id);
		fflush(log_file);

		kill(info->si_pid, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	} else if (core_inter_head[sender_id]->type == AGENT_REQ_CORES && state != IDLE_INIT_MAN && state != WORKING_NODE_IDLE_INIT) {
		//IDLE_INIT_MAN_SELFOPT_PENDING && state != IDLE_INIT_MAN_WORK_PENDING) {
		printf("I am %d and i think i cought a stray init AGENT_REQ_CORES reply from %d. My current state is %d\n",node_id,sender_id,state);
		fprintf(log_file,"I am %d and i think i cought a stray init AGENT_REQ_CORES reply from %d. My current state is %d\n",node_id,sender_id,state);
		fflush(log_file);

		kill(info->si_pid, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		/*kill(info->si_pid, SIG_ACK);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		}		
		
		read(fd_r, &num_of_offers, sizeof(int));
		core_inter_head[sender_id]->data.offer_acc_array = (int *) malloc((num_of_offers+1)*sizeof(int));	
		core_inter_head[sender_id]->data.offer_acc_array[0] = num_of_offers;			
		
		for (j=1; j<=num_of_offers; j++){ 
			read(fd_r, &one_offer.num_of_cores, sizeof(int));
			read(fd_r, &one_offer.spd_loss, sizeof(float));

			for (i=0; i<one_offer.num_of_cores; i++)
				read(fd_r, &tmp_int, sizeof(int));
				
			core_inter_head[sender_id]->data.offer_acc_array[j] = 0;
		}

		core_inter_head[sender_id]->type = REP_AGENT_OFFER_SENT;
		kill(info->si_pid, SIG_REP_OFFERS);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);*/
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		free(tmp_inter_list);
		
		if(core_inter_head[sender_id] == NULL) {
			if (core_inter_tail[sender_id] == NULL){
				printf("Malaka gamietai to core_inter_tail\n");				
				core_inter_tail[sender_id] = NULL;
			}
		} else send_next_signal(core_inter_head[sender_id], sender_id);	
	} else if (core_inter_head[sender_id]->type == SELFOPT_REQ_CORES && state != IDLE_AGENT_WAITING_OFF) {
		printf("I am %d and i think i cought a stray selfopt SELFOPT_REQ_CORES reply from %d. My current state is %d\n",node_id,sender_id,state);
		fprintf(log_file,"I am %d and i think i cought a stray selfopt SELFOPT_REQ_CORES reply from %d. My current state is %d\n",node_id,sender_id,state);
		fflush(log_file);

		kill(info->si_pid, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		/*kill(info->si_pid, SIG_ACK);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		}		
		
		read(fd_r, &num_of_offers, sizeof(int));
		core_inter_head[sender_id]->data.offer_acc_array = (int *) malloc((num_of_offers+1)*sizeof(int));	
		core_inter_head[sender_id]->data.offer_acc_array[0] = num_of_offers;			
		
		for (j=1; j<=num_of_offers; j++){ 
			read(fd_r, &one_offer.num_of_cores, sizeof(int));
			read(fd_r, &one_offer.spd_loss, sizeof(float));

			for (i=0; i<one_offer.num_of_cores; i++)
				read(fd_r, &tmp_int, sizeof(int));
				
			core_inter_head[sender_id]->data.offer_acc_array[j] = 0;
		}

		core_inter_head[sender_id]->type = REP_AGENT_OFFER_SENT;
		kill(info->si_pid, SIG_REP_OFFERS);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);*/
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		free(tmp_inter_list);
		
		if(core_inter_head[sender_id] == NULL) {
			if (core_inter_tail[sender_id] == NULL){
				printf("Malaka gamietai to core_inter_tail\n");				
				core_inter_tail[sender_id] = NULL;
			}
		} else send_next_signal(core_inter_head[sender_id], sender_id);
	} else if (core_inter_head[sender_id]->type == FAR_REQ_CORES && (state == IDLE_IDAG || (node_id == 0 && time_for_farman == -1))) {
		printf("I am %d and i think i cought a stray far_REQ_CORES reply from %d. My current state is %d\n",node_id,sender_id,state);
		fprintf(log_file,"I am %d and i think i cought a stray far_REQ_CORES reply from %d. My current state is %d\n",node_id,sender_id,state);
		fflush(log_file);

		kill(info->si_pid, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		/*kill(info->si_pid, SIG_ACK);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		}		
		
		read(fd_r, &num_of_offers, sizeof(int));
		core_inter_head[sender_id]->data.offer_acc_array = (int *) malloc((num_of_offers+1)*sizeof(int));	
		core_inter_head[sender_id]->data.offer_acc_array[0] = num_of_offers;			
		
		for (j=1; j<=num_of_offers; j++){ 
			read(fd_r, &one_offer.num_of_cores, sizeof(int));
			read(fd_r, &one_offer.spd_loss, sizeof(float));

			for (i=0; i<one_offer.num_of_cores; i++)
				read(fd_r, &tmp_int, sizeof(int));
				
			core_inter_head[sender_id]->data.offer_acc_array[j] = 0;
		}

		core_inter_head[sender_id]->type = REP_AGENT_OFFER_SENT;
		kill(info->si_pid, SIG_REP_OFFERS);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);*/
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		free(tmp_inter_list);
		
		if(core_inter_head[sender_id] == NULL) {
			if (core_inter_tail[sender_id] == NULL){
				printf("Malaka gamietai to core_inter_tail\n");				
				core_inter_tail[sender_id] = NULL;
			}
		} else send_next_signal(core_inter_head[sender_id], sender_id);
	} else if (core_inter_head[sender_id]->type == AGENT_REQ_CORES || core_inter_head[sender_id]->type == FAR_REQ_CORES || core_inter_head[sender_id]->type == SELFOPT_REQ_CORES){ //I am the requesting common node
		//printf("fghjfgI am inside i am node with node id %d idag_id %d and sender_id %d\n",node_id,idag_id,sender_id);		
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "In 6 Trying to acquire semaphore. Sem value =  %d\n",i);
		fflush(log_file);
		kill(info->si_pid, SIG_ACK);		
		sem_wait(&node_sem[node_id]);
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
		fflush(log_file);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		//printf("open8 idag_id=%d node_id=%d\n",idag_id,node_id);		
		fd_r = open(fifo_name, O_RDONLY);
		
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		} /*else {
			fprintf(log_file, "I went through open\n");
			fflush(log_file);
		}	*/		
		
		if (core_inter_head[sender_id]->type == AGENT_REQ_CORES || core_inter_head[sender_id]->type == SELFOPT_REQ_CORES) {//den erxontai ta far edw
			read(fd_r, &num_of_offers, sizeof(int));
			core_inter_head[sender_id]->data.offer_acc_array = (int *) malloc((num_of_offers+1)*sizeof(int));	
			core_inter_head[sender_id]->data.offer_acc_array[0] = num_of_offers;			
			//core_inter_head[sender_id]->data.offer_accepted = -1;
			//one_offer.offered_cores = NULL;

			for (j=1; j<=num_of_offers; j++){
				read(fd_r, &one_offer.num_of_cores, sizeof(int));
				read(fd_r, &one_offer.spd_loss, sizeof(float));
				//if (one_offer.offered_cores != NULL) free(one_offer.offered_cores);
				/*one_offer.offered_cores = (int *) malloc(one_offer.num_of_cores*sizeof(int));
				for (i=0; i<one_offer.num_of_cores; i++)
					read(fd_r, &one_offer.offered_cores[i], sizeof(int));
				printf("asdasNode %d is offering %d cores: ",sender_id,one_offer.num_of_cores);
				for (i=0; i<one_offer.num_of_cores; i++)
					printf(" %d,",one_offer.offered_cores[i]);
				printf("\n");*/	
				
				if (core_inter_head[sender_id]->type == AGENT_REQ_CORES && init_man_offers == NULL){
					init_man_offers = (offer_list *) malloc(sizeof(offer_list));
					chosen_node = init_man_offers;
					/*init_man_offers->off = one_offer;
					init_man_offers->sender = sender_id;
					core_inter_head[sender_id]->data.offer_acc_array[j] = -1; 
					init_man_offers->answer = &core_inter_head[sender_id]->data.offer_acc_array[j];
					*/					
					init_man_offers->next = NULL;
				} else if (core_inter_head[sender_id]->type == SELFOPT_REQ_CORES && selfopt_man_offers == NULL){
					selfopt_man_offers = (offer_list *) malloc(sizeof(offer_list));
					chosen_node = selfopt_man_offers;
					/*					
					selfopt_man_offers->off = one_offer;
					selfopt_man_offers->sender = sender_id;
					core_inter_head[sender_id]->data.offer_acc_array[j] = -1; 
					selfopt_man_offers->answer = &core_inter_head[sender_id]->data.offer_acc_array[j];
					*/					
					selfopt_man_offers->next = NULL;	
				} else {
					if (core_inter_head[sender_id]->type == AGENT_REQ_CORES) tmp_offer_list = init_man_offers;
					else tmp_offer_list = selfopt_man_offers;
					//if (node_id == 12) printf("aaa\n");

					tmp_offer_prev = NULL;
					while (tmp_offer_list != NULL && tmp_offer_list->off.num_of_cores >= one_offer.num_of_cores){
						tmp_offer_prev = tmp_offer_list;			
						tmp_offer_list = tmp_offer_list->next;
					}
					//if (node_id == 12) printf("aa\n");
					if (tmp_offer_list == NULL) { //prepei na mpei teleutaio
						//if (node_id == 12) printf("a\n");						
						tmp_offer_prev->next = (offer_list *) malloc(sizeof(offer_list));
						tmp_offer_list = tmp_offer_prev->next;
						tmp_offer_list->next = NULL;
						chosen_node = tmp_offer_list;
												
						/*tmp_offer_list->off = one_offer;
						tmp_offer_list->sender = sender_id;
						core_inter_head[sender_id]->data.offer_acc_array[j] = -1;
						tmp_offer_list->answer = &core_inter_head[sender_id]->data.offer_acc_array[j];
						*/
					} else if (tmp_offer_prev == NULL) { //prepei na mpei prwto
						//if (node_id == 12) printf("b\n");						
						if (core_inter_head[sender_id]->type == AGENT_REQ_CORES) {
							init_man_offers = (offer_list *) malloc(sizeof(offer_list));
							tmp_head = init_man_offers;
						} else {
							selfopt_man_offers = (offer_list *) malloc(sizeof(offer_list));
							tmp_head = selfopt_man_offers;
						}
						
						chosen_node = tmp_head;
						/*tmp_head->off = one_offer;
						tmp_head->sender = sender_id;
						core_inter_head[sender_id]->data.offer_acc_array[j] = -1;
						tmp_head->answer = &core_inter_head[sender_id]->data.offer_acc_array[j];*/
						tmp_head->next = tmp_offer_list;
					} else {
						//if (node_id == 12) printf("c\n");						
						tmp_offer_prev->next = (offer_list *) malloc(sizeof(offer_list));
						tmp_offer_prev = tmp_offer_prev->next;
						chosen_node = tmp_offer_prev;						
						/*tmp_offer_prev->off = one_offer;
						tmp_offer_prev->sender = sender_id;
						core_inter_head[sender_id]->data.offer_acc_array[j] = -1;
						tmp_offer_prev->answer = &core_inter_head[sender_id]->data.offer_acc_array[j];
						*/
												
						tmp_offer_prev->next = tmp_offer_list;
					}

					//for(tmp_offer_list=init_man_offers; tmp_offer_list!=NULL; tmp_offer_list=tmp_offer_list->next) 
					//	printf("kariolares node_id=%d sender_id=%d\n",node_id,tmp_offer_list->sender); 
				}
	
				chosen_node->off.num_of_cores = one_offer.num_of_cores;
				chosen_node->off.spd_loss = one_offer.spd_loss;					
				chosen_node->off.offered_cores = (int *) malloc(one_offer.num_of_cores*sizeof(int));
				for (i=0; i<one_offer.num_of_cores; i++)
					read(fd_r, &chosen_node->off.offered_cores[i], sizeof(int));
				/*printf("asdasNode %d is offering %d cores: ",sender_id,chosen_node->off.num_of_cores);
				for (i=0; i<chosen_node->off.num_of_cores; i++)
					printf(" %d,",chosen_node->off.offered_cores[i]);
				printf("\n");*/
				chosen_node->sender = sender_id;
				core_inter_head[sender_id]->data.offer_acc_array[j] = -1;
				chosen_node->answer = &core_inter_head[sender_id]->data.offer_acc_array[j];

				cur_time = time(NULL);	
				cur_t = localtime(&cur_time);
				fprintf(log_file, "[%d:%d:%d]: One node successfully added in list type=%d sender_id=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,core_inter_head[sender_id]->type,sender_id);
				fflush(log_file);
			}
		} else {
			read(fd_r, &num_of_offers, sizeof(int));
			//if (num_of_offers > 1) printf("I am %d in req cores in far and i am getting %d offers from %d\n",node_id,num_of_offers,sender_id);
			core_inter_head[sender_id]->data.offer_acc_array = (int *) malloc((num_of_offers+1)*sizeof(int));	
			core_inter_head[sender_id]->data.offer_acc_array[0] = num_of_offers;			
			
			read(fd_r, &one_offer.num_of_cores, sizeof(int));
			read(fd_r, &one_offer.spd_loss, sizeof(float));
			one_offer.offered_cores = (int *) malloc(one_offer.num_of_cores*sizeof(int));
			for (i=0; i<one_offer.num_of_cores; i++)
				read(fd_r, &one_offer.offered_cores[i], sizeof(int));
			//printf("asdasNode %d is offering %d cores with speedup loss %f\n",sender_id,one_offer.num_of_cores,one_offer.spd_loss);
			//core_inter_head[sender_id]->data.offer_accepted = -1;
			core_inter_head[sender_id]->data.offer_acc_array[1] = -1;

			if (far_man_offers == NULL){
				far_man_offers = (offer_list *) malloc(sizeof(offer_list));
				far_man_offers_tail = far_man_offers;
			} else {
				far_man_offers_tail->next = (offer_list *) malloc(sizeof(offer_list));
				far_man_offers_tail = far_man_offers_tail->next;
			}

			far_man_offers_tail->off = one_offer;		
			far_man_offers_tail->sender = sender_id;
			far_man_offers_tail->answer = &core_inter_head[sender_id]->data.offer_acc_array[1];//offer_accepted;	
			far_man_offers_tail->next = NULL;	
			far_list_count++;

			cur_time = time(NULL);	
			cur_t = localtime(&cur_time);
			fprintf(log_file, "[%d:%d:%d]: One node successfully added in far_man_offers list %d\n",
				cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,core_inter_head[sender_id]->data.offer_acc_array[0]);
			fflush(log_file);			
		}
		core_inter_head[sender_id]->type = REP_AGENT_OFFER_PENDING;
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
		fflush(log_file);
		if (sem_post(&node_sem_out[sender_id]) == -1){
			printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
			perror("sem_post");
		}
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "Sem value = %d\n",i);
		fflush(log_file);		
		//sem_post(&node_sem[node_id]);
		////sem_post(&node_sem[sender_id]);
		/*tmp_inter_list = core_inter_head[sender_id]->next;
		free(core_inter_head[sender_id]);
		core_inter_head[sender_id] = (inter_list *) malloc(sizeof(inter_list));
		core_inter_head[sender_id]->next = tmp_inter_list;
		core_inter_head[sender_id]->type = REP_AGENT_OFFER_SENT;
		core_inter_head[sender_id]->data.offer_accepted = -1;*/

		/*if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);		
		free(tmp_inter_list);*/		
	} else if (core_inter_head[sender_id]->type == AGENT_REQ_CORES_PENDING || core_inter_head[sender_id]->type == SELFOPT_REQ_CORES_PENDING 
		|| core_inter_head[sender_id]->type == FAR_REQ_CORES_PENDING){
		printf("I am %d and deadlock was prevented in req_cores by %d\n",node_id,sender_id);	
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		free(tmp_inter_list);
		
		if(core_inter_head[sender_id] == NULL) {
			if (core_inter_tail[sender_id] == NULL){
				printf("Malaka gamietai to core_inter_tail\n");				
				core_inter_tail[sender_id] = NULL;
			}
		} else send_next_signal(core_inter_head[sender_id], sender_id);				
	} else {
		//printf("I am %d and We were fucked in sig_REQ_DDS_handler from %d interaction is = %d\n",node_id,sender_id,core_inter_head[sender_id]->type);
		printf("I am %d and i have to reject req_cores from %d. Our interaction is %d\n",node_id,sender_id,core_inter_head[sender_id]->type);
		fprintf(log_file,"I am %d and i have to reject req_cores from %d. Our interaction is %d\n",node_id,sender_id,core_inter_head[sender_id]->type);
		fflush(log_file);

		/*kill(info->si_pid, SIG_ACK);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		}		
		
		read(fd_r, &req_app, sizeof(app));
		//printf("I am node with id %d idag id %d with sender id %d and I am to investigate region C=%d r=%d and app %f\n",node_id,idag_id,sender_id,cur_reg.C,cur_reg.r,req_app.A);
	
		read(fd_r, &num_of_offers, sizeof(int));
		for (i=0; i<num_of_offers; i++)
			read(fd_r, &cur_reg, sizeof(region));
		*/
		kill(info->si_pid, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	}

	//printf("We were fucked in sig_REQ_CORES_handler node_id = %d sender_id = %d interaction=%d\n",node_id,sender_id,core_inter_head[sender_id]->type);

	if (fd_r != -1) close(fd_r);
	free(fifo_name);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_REQ_CORES_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

void sig_REP_OFFERS_handler(int signo, siginfo_t *info, void *context)
{	
	int fd_r, sender_id, offer_ans, i, one_core, j, old_cores_cnt;//num_of_idags, i, one_idag;
	core_list *tmp_cores, *tmp_cores_prev, *tmp_cores_list;
	inter_list *tmp_inter_list, *tmp_inter_prev;
	offer_list *tmp_offer_list;
	char *fifo_name;
	
	signals_disable();
	
	sender_id = get_id_from_pid(info->si_pid);
	fifo_name = get_pipe_name(node_id);	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_REP_OFFERS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	
	if (core_inter_head[sender_id] == NULL)
		printf("fail!\n");
	else if (core_inter_head[sender_id]->type == AGENT_OFFER_SENT) {
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "In 7 Trying to acquire semaphore. Sem value =  %d\n",i);
		fflush(log_file);
		kill(info->si_pid, SIG_ACK);		
		sem_wait(&node_sem[node_id]);
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
		fflush(log_file);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);	
		//printf("open9 idag_id=%d node_id=%d fifo_name=%s\n",idag_id,node_id,fifo_name);		
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		} /*else {
			fprintf(log_file, "I went through open\n");
			fflush(log_file);
		}	*/

		fprintf(log_file, "Num of offers is  =  %d\n",core_inter_head[sender_id]->data.off_arr.num_of_offers);
		fflush(log_file);
		//even if i am in a far req offer, my answer will be the first
		for (j=0; j<core_inter_head[sender_id]->data.off_arr.num_of_offers; j++) {
			read(fd_r, &offer_ans, sizeof(int));
			fprintf(log_file, "offer_ans = %d\n",offer_ans);
			fflush(log_file);
			//printf("I am node %d and my offer answer is %d interaction=%d\n",node_id,offer_ans,core_inter_head[sender_id]->type);
			//printf("I am %d and i am getting bizarre answer = %d from %d with num of offers = %d\n",node_id,offer_ans,sender_id,core_inter_head[sender_id]->data.off_arr.num_of_offers);			
			while (offer_ans != 0 && offer_ans != 1){
				printf("I am %d and i am getting bizarre answer = %d from %d\n",node_id,offer_ans,sender_id);
				read(fd_r, &offer_ans, sizeof(int));
			}

			if (offer_ans == 0 && my_cores != NULL) { // && my_cores != NULL
				for (i=0; i<core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores; i++)
					for (tmp_cores = my_cores->next; tmp_cores != NULL; tmp_cores = tmp_cores->next)
						if (tmp_cores->core_id == core_inter_head[sender_id]->data.off_arr.offer_arr[j].offered_cores[i]) {
							fprintf(log_file,"core id = %d offered_to = %d\n",tmp_cores->core_id,tmp_cores->offered_to);
							fflush(log_file);
							if (tmp_cores->offered_to == sender_id) tmp_cores->offered_to = -1;
							break;
						}

				if (app_state == APP_TERMINATED) {//app_terminated
					for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next)
					if (tmp_cores_list->offered_to != -1) break;

					//if (tmp_cores_list != NULL && state ) state = AGENT_ZOMBIE;
					//else state = AGENT_ENDING;
					if (tmp_cores_list == NULL && state == AGENT_ZOMBIE) state = AGENT_ENDING;
				}	
				/*tmp_cores = my_cores;
				while (tmp_cores != NULL){
					if (tmp_cores->offered_to == sender_id) tmp_cores->offered_to = -1;
					tmp_cores = tmp_cores->next;
				}*/
			} else if (offer_ans == 1) {//&& (!app_terminated || my_idag == -1) my_cores != NULL inform my idag if I have. If i am an idag i just reduce my DDS core num. I keep my core.
				if (my_idag == -1){ //I am an idag 
					//if (node_id == 0) printf("qwer asdf %d\n",DDS->num_of_cores);					
					//if (DDS->agent_id	!= node_id) printf("Fuck i am not first in my DDS list!!\n");
					DDS->num_of_cores -= core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores;
					//if (node_id == 0) printf("kariolares asdf %d\n",core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores);
					//printf("I did REM\n");			
				} else {//I am common node
					//printf("I start REM here!!!!!\n");				
					/*first i must get the remaining time from my cores, before i change my core list*/
					
					if (app_state != APP_TERMINATED) { //!app_terminated	
						old_Speedup = my_Speedup;
						old_cores_cnt = my_cores_count;
						my_cores_count -= core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores;
						my_app.num_of_cores = my_cores_count;				
						my_Speedup = Speedup(my_app, my_cores_count);			
	
						fprintf(log_file,"Initialising removal new_cores_count = %d app_state = %d\n",my_cores_count,app_state);
						fflush(log_file);
						fprintf(app_log_file,"Initialising removal new_cores_count = %d app_state = %d\n",my_cores_count,app_state);
						fflush(app_log_file);

						if (app_state == RUNNING) {
							chk_rem_num = old_cores_cnt-1;
							chk_rem_count = 0;
							sum_rem_time = 0;
							app_state = RESIZING;

							for(tmp_cores_list=my_cores->next; tmp_cores_list!=NULL; tmp_cores_list=tmp_cores_list->next) {//first is myself
								one_core = tmp_cores_list->core_id;
								if (core_inter_head[one_core] == NULL){
									core_inter_head[one_core] = (inter_list *) malloc(sizeof(inter_list));
									core_inter_tail[one_core] = core_inter_head[one_core];
								} else {
									core_inter_tail[one_core]->next = (inter_list *) malloc(sizeof(inter_list));
									core_inter_tail[one_core] = core_inter_tail[one_core]->next;
								}

								core_inter_tail[one_core]->type = APPOINT_WORK_NODE_PENDING;//CHK_REM_TIME;
								core_inter_tail[one_core]->next = NULL;
		
								if (core_inter_head[one_core]->next == NULL) {
									kill(pid_num[one_core], SIG_CHECK_REM_TIME);
									my_stats.msg_count++;
									my_stats.distance += distance(node_id,one_core);
								} else printf("I am %d and I am doing smth else with my working node %d in send SIG_CHECK_REM_TIME in rep offers type = %d\n",
										node_id,one_core,core_inter_head[one_core]->type);
							}
						} //else {		
					} else {
						my_cores_count -= core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores;
						my_app.num_of_cores = my_cores_count;	

						fprintf(log_file,"Initialising removal app finished new_cores_count = %d\n",my_cores_count);
						fflush(log_file);
						fprintf(app_log_file,"Initialising removal app finished new_cores_count = %d\n",my_cores_count);
						fflush(app_log_file);
					}

					if (core_inter_head[my_idag] == NULL){
						core_inter_head[my_idag] = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[my_idag] = core_inter_head[my_idag];
					} else {
						core_inter_tail[my_idag]->next = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[my_idag] = core_inter_tail[my_idag]->next;
					}

					core_inter_tail[my_idag]->type = IDAG_REM_CORES_DDS;
					core_inter_tail[my_idag]->data.app_cores = (int *)malloc((core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores+1)*sizeof(int));
					core_inter_tail[my_idag]->data.app_cores[0] = core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores;
					for(i=1; i<=core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores; i++) {
						one_core = core_inter_head[sender_id]->data.off_arr.offer_arr[j].offered_cores[i-1];
					
						if (app_state != APP_TERMINATED) {
							tmp_inter_prev = core_inter_head[one_core];
							tmp_inter_list = core_inter_head[one_core]->next;
						} else {
							tmp_inter_prev = NULL;
							tmp_inter_list = core_inter_head[one_core];
						}
						//for (tmp_inter_list = core_inter_head[one_core]; tmp_inter_list != NULL; tmp_inter_list=tmp_inter_list->next)
						while (tmp_inter_list != NULL)
							if (tmp_inter_list->type == INIT_WORK_NODE || tmp_inter_list->type == APPOINT_WORK_NODE || tmp_inter_list->type == INIT_WORK_NODE_PENDING 
								|| tmp_inter_list->type == APPOINT_WORK_NODE_PENDING) {							
								//|| (tmp_inter_list->type == APPOINT_WORK_NODE_PENDING && tmp_inter_prev != NULL)) {
							
								fprintf(log_file, "Removing in rem offers one node of %d with inter = %d\n",one_core,tmp_inter_list->type);
								fflush(log_file);

						 		if (tmp_inter_prev == NULL) {
									core_inter_head[one_core] = core_inter_head[one_core]->next;
									free(tmp_inter_list);
									tmp_inter_list = core_inter_head[one_core];
								} else {
									tmp_inter_prev->next = tmp_inter_list->next;
									if (tmp_inter_prev->next == NULL) core_inter_tail[one_core] = tmp_inter_prev;
									free(tmp_inter_list);
									tmp_inter_list = tmp_inter_prev->next;
								} 	
							} else {
								tmp_inter_prev = tmp_inter_list;
								tmp_inter_list = tmp_inter_list->next;
							} 
					
						if (app_state != APP_TERMINATED) {
							if (core_inter_head[one_core] == NULL) {
								fprintf(log_file,"No interaction with %d. Theoritically impossible\n",one_core);
								fflush(log_file);
							} else if (core_inter_head[one_core]->type == INIT_WORK_NODE_PENDING) {
								fprintf(log_file,"I offered my new core %d. I will clear the interaction\n",one_core);
								fflush(log_file);
								tmp_inter_list = core_inter_head[one_core];
								core_inter_head[one_core] = core_inter_head[one_core]->next;
								if (core_inter_head[one_core] == NULL) core_inter_tail[one_core] = NULL;
								else send_next_signal(core_inter_head[one_core], one_core);	
								free(tmp_inter_list);
							} else if (core_inter_head[one_core]->type == INIT_WORK_NODE || core_inter_head[one_core]->type == APPOINT_WORK_NODE) {
								fprintf(log_file,"Invalidating %d. Interaction is %d\n",one_core,core_inter_head[one_core]->type);
								fflush(log_file);

								core_inter_head[one_core]->data.work_time = -1;
								core_inter_tail[one_core]->next = (inter_list *) malloc(sizeof(inter_list));
								core_inter_tail[one_core] = core_inter_tail[one_core]->next;
							
								core_inter_tail[one_core]->type = REMOVED_NODE_REM_TIME;//APPOINT_WORK_NODE_PENDING;//CHK_REM_TIME;
								core_inter_tail[one_core]->next = NULL;
							} else if (core_inter_head[one_core]->type == APPOINT_WORK_NODE_PENDING) {
								fprintf(log_file,"Everything ok %d.\n",one_core);
								fflush(log_file);
								core_inter_head[one_core]->type = REMOVED_NODE_REM_TIME;
							} else {
								fprintf(log_file,"Another interaction with %d. Interaction is %d\n",one_core,core_inter_head[one_core]->type);
								fflush(log_file);

								core_inter_tail[one_core]->next = (inter_list *) malloc(sizeof(inter_list));
								core_inter_tail[one_core] = core_inter_tail[one_core]->next;
							
								core_inter_tail[one_core]->type = REMOVED_NODE_REM_TIME;//APPOINT_WORK_NODE_PENDING;//CHK_REM_TIME;
								core_inter_tail[one_core]->next = NULL;
							}
						}
						tmp_cores = my_cores;
						tmp_cores_prev = NULL;
						while (tmp_cores != NULL && tmp_cores->core_id != core_inter_head[sender_id]->data.off_arr.offer_arr[j].offered_cores[i-1]){
							tmp_cores_prev = tmp_cores;			
							tmp_cores = tmp_cores->next; 
						}

						if (tmp_cores == NULL) printf("strangely offered core is not in my_cores list\n");
						else {
							if (tmp_cores_prev == NULL) {
								printf("i fucking offered my agent core!!!\n");
								my_cores = my_cores->next;
							} else if (tmp_cores == my_cores_tail){
								my_cores_tail = tmp_cores_prev;
								my_cores_tail->next = NULL;
							} else tmp_cores_prev->next = tmp_cores->next;
		
							free(tmp_cores);
						}
	
						core_inter_tail[my_idag]->data.app_cores[i] = core_inter_head[sender_id]->data.off_arr.offer_arr[j].offered_cores[i-1];
					}				
					core_inter_tail[my_idag]->next = NULL;

					if (core_inter_head[my_idag]->next == NULL) {
						kill(pid_num[my_idag], SIG_REM_CORES_DDS);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,my_idag);
					} else printf("I did not send rem signal!\n");
					//printf("At least i am coming here!!!!!\n");
					
					cur_time = time(NULL);	
					cur_t = localtime(&cur_time);					
					fprintf(app_log_file, "[%d:%d:%d]: Removal ok. my_cores_count = %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,my_cores_count);
					fprintf(app_log_file, "my cores are:");

					for (tmp_cores_list=my_cores; tmp_cores_list!=NULL; tmp_cores_list=tmp_cores_list->next) {
						//fprintf(log_file, " %d", tmp_cores_list->core_id);
						fprintf(app_log_file, " %d", tmp_cores_list->core_id);					
						//printf(" %d",tmp_cores_list->core_id);
					}

					//printf("\n");
					//fprintf(log_file, "\n");
					//fflush(log_file);
					fprintf(app_log_file, "\n");
					fflush(app_log_file);

					if (app_state == APP_TERMINATED) {//app_terminated
						for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next)
							if (tmp_cores_list->offered_to != -1) break;
						
						if (tmp_cores_list == NULL && state == AGENT_ZOMBIE) state = AGENT_ENDING;
					}
				}
			/*} else if (offer_ans == 1 && app_terminated) {
				my_cores_count -= core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores;				
				fprintf(log_file,"Initialising removal 2 new_cores_count = %d\n",my_cores_count);
				fflush(log_file);

				for(i=1; i<=core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores; i++) {
					tmp_cores = my_cores;
					tmp_cores_prev = NULL;
					while (tmp_cores != NULL && tmp_cores->core_id != core_inter_head[sender_id]->data.off_arr.offer_arr[j].offered_cores[i-1]){
						tmp_cores_prev = tmp_cores;			
						tmp_cores = tmp_cores->next; 
					}

					if (tmp_cores == NULL) printf("strangely offered core is not in my_cores list\n");
					else {
						if (tmp_cores_prev == NULL) {
							printf("i fucking offered my agent core!!!\n");
							my_cores = my_cores->next;
						} else if (tmp_cores == my_cores_tail){
							my_cores_tail = tmp_cores_prev;
							my_cores_tail->next = NULL;
						} else tmp_cores_prev->next = tmp_cores->next;
	
						free(tmp_cores);
					}

				}

				for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next)
					if (tmp_cores_list->offered_to != -1) break;

				//if (tmp_cores_list != NULL) state = AGENT_ZOMBIE;
				//else state = AGENT_ENDING;
				if (tmp_cores_list == NULL && state == AGENT_ZOMBIE) state = AGENT_ENDING;*/
			} else printf("I am %d and my Answer from %d different than 0 or 1 and is %d!!\n",node_id,sender_id,offer_ans);
		}
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
		fflush(log_file);
		if (sem_post(&node_sem_out[sender_id]) == -1){
			printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
			perror("sem_post");
		}
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "Sem value = %d\n",i);
		fflush(log_file);
		//sem_post(&node_sem[node_id]);
		//sem_post(&node_sem[sender_id]);
	}	else if (core_inter_head[sender_id]->type == FAR_REQ_OFFER_SENT){
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "In 8 Trying to acquire semaphore. Sem value =  %d\n",i);
		fflush(log_file);
		kill(info->si_pid, SIG_ACK);		
		sem_wait(&node_sem[node_id]);
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
		fflush(log_file);
		//printf("open9 idag_id=%d node_id=%d fifo_name=%s\n",idag_id,node_id,fifo_name);		
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		} else {
			fprintf(log_file, "I went through open\n");
			fflush(log_file);
		}	
		//even if i am in a far req offer, my answer will be the first
		read(fd_r, &offer_ans, sizeof(int));
		printf("I am node %d and my offer answer is %d interaction=%d\n",node_id,offer_ans,core_inter_head[sender_id]->type);

		if (offer_ans == 0) {
			tmp_cores = my_cores;
			while (tmp_cores != NULL){
				if (tmp_cores->offered_to == sender_id) tmp_cores->offered_to = -1;
				tmp_cores = tmp_cores->next;
			}
		} else if (offer_ans == 1) {//inform my idag if I have. If i am an idag i just reduce my DDS core num. I keep my core.
			if (DDS->agent_id	!= node_id) printf("Fuck i am not first in my DDS list!!\n");
			DDS->num_of_cores -= core_inter_head[sender_id]->data.my_offer.num_of_cores;
		} else printf("I am %d Answer different than 0 or 1 in far from %d!!\n",node_id,sender_id);
		//if (core_inter_head[sender_id]->type == FAR_REQ_OFFER_SENT) {//FAR_REQ_OFFER
			/*printf("poutanares\n");
			if (far_man_offers == NULL) printf("kai poytanes kai karioles\n");
			else printf("gamw ths papias\n");*/
		tmp_offer_list = far_man_offers;
		far_man_offers = far_man_offers->next;
		free(tmp_offer_list);

		while (far_man_offers != NULL){
			//printf("kai edw ftamw\n");
			read(fd_r, &offer_ans, sizeof(int));
			printf("I am node %d and far offer answer for node %d is %d\n",node_id,far_man_offers->sender,offer_ans);

			if (offer_ans == 0 || offer_ans == 1){
				*far_man_offers->answer = offer_ans;
				if (core_inter_head[far_man_offers->sender]->type == REP_AGENT_OFFER_PENDING){
					kill(pid_num[far_man_offers->sender],SIG_REP_OFFERS);
					core_inter_head[far_man_offers->sender]->type = REP_AGENT_OFFER_SENT;
				} else printf("Apparently lists are poutana ola\n");
			} else printf("Far Answer different than 0 or 1!!\n");
				
			tmp_offer_list = far_man_offers;
			far_man_offers = far_man_offers->next;
			free(tmp_offer_list);
		}

		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
		fflush(log_file);
		if (sem_post(&node_sem_out[sender_id]) == -1){
			printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
			perror("sem_post");
		}
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "Sem value = %d\n",i);
		fflush(log_file);
		//sem_post(&node_sem[node_id]);
		//sem_post(&node_sem[sender_id]);
		far_req_app.A=-1.0;
		far_req_app.var=-1.0; 
		far_req_app.num_of_cores=-1;
		far_req_or_sender = -1;					
	} else printf("I am %d and fail 2\n",node_id);

	tmp_inter_list = core_inter_head[sender_id];
	core_inter_head[sender_id] = core_inter_head[sender_id]->next;
	if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
	else send_next_signal(core_inter_head[sender_id], sender_id);	
	free(tmp_inter_list);

	close(fd_r);
	free(fifo_name);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_REP_OFFERS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

void sig_INIT_AGENT_handler(int signo, siginfo_t *info, void *context)
{
	int fd_r, i, tmp, sender_id; 
	char *fifo_name;
	core_list *tmp_core;

	signals_disable();
	sender_id = get_id_from_pid(info->si_pid);
	fifo_name = get_pipe_name(node_id);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_INIT_AGENT_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);

	sem_getvalue(&node_sem[node_id],&i);
	fprintf(log_file, "In 9 Trying to acquire semaphore. Sem value =  %d\n",i);
	fflush(log_file);
	kill(info->si_pid, SIG_ACK);		
	sem_wait(&node_sem[node_id]);
	sem_getvalue(&node_sem[node_id],&i);
	fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
	fflush(log_file);
	my_stats.msg_count++;
	my_stats.distance += distance(node_id,get_id_from_pid(info->si_pid));	
	//printf("open10 idag_id=%d node_id=%d\n",idag_id,node_id);	
	fd_r = open(fifo_name, O_RDONLY);

	if (fd_r == -1) {
		perror("opening reading pipe"); 
		signals_enable();		
		return;
	} else {
		fprintf(log_file, "I went through open\n");
		fflush(log_file);
	}	
	
	if (my_app.num_of_cores != -1) { //Very very very important!!!
		printf ("I am already managing an app!!! Fail!!!!\n");
		while (my_cores != NULL){
			tmp_core = my_cores;
			my_cores = my_cores->next;
			free(tmp_core);
		}
		my_cores_tail = NULL;
		my_cores_count = 0;
	} 

	read(fd_r, &my_app, sizeof(app));
	my_cores_count = my_app.num_of_cores;//+1;
	if (my_cores == NULL) {
		my_cores = (core_list *) malloc(sizeof(core_list));
		my_cores_tail = my_cores;
	} else {
		printf("My cores still not fucking null!!\n");
		my_cores_tail->next = (core_list *) malloc(sizeof(core_list));
		my_cores_tail = my_cores_tail->next;
	}

	my_cores_tail->core_id = node_id;
	my_cores_tail->offered_to = -1;
	my_cores_tail->next = NULL;

	//I want myself to be first in my_cores list
	for (i=0; i<my_app.num_of_cores; i++){
		read(fd_r, &tmp, sizeof(int));
		if (tmp != node_id){
			my_cores_tail->next = (core_list *) malloc(sizeof(core_list));
			my_cores_tail = my_cores_tail->next;

			//read(fd_r, &my_cores_tail->core_id, sizeof(int));
			my_cores_tail->core_id = tmp;
			my_cores_tail->offered_to = -1;
			my_cores_tail->next = NULL;
		}
	}

	////sem_post(&node_sem[sender_id]);

	printf("I am new agent with id %d and app with A=%.2f, var=%.2f and %d my_cores_count = %d cores: \n"
		,node_id,my_app.A,my_app.var,my_app.num_of_cores,my_cores_count);
	my_Speedup = Speedup(my_app, my_app.num_of_cores);

	if (core_inter_head[my_idag] == NULL){
		core_inter_head[my_idag] = (inter_list *) malloc(sizeof(inter_list));
		core_inter_tail[my_idag] = core_inter_head[my_idag];
	} else {
		core_inter_tail[my_idag]->next = (inter_list *) malloc(sizeof(inter_list));
		core_inter_tail[my_idag] = core_inter_tail[my_idag]->next;
	}

	core_inter_tail[my_idag]->type = IDAG_ADD_CORES_DDS;
	//core_inter_tail[my_idag]->data.my_offer = core_inter_head[sender_id]->data.my_offer;
	core_inter_tail[my_idag]->data.app_cores = (int *)malloc((my_cores_count+1)*sizeof(int));
	core_inter_tail[my_idag]->data.app_cores[0] = my_cores_count;
	//for(i=1; i<=my_cores_count; i++)
	//	core_inter_tail[my_idag]->data.app_cores[i] = init_man_offers->off.offered_cores[i-1];		
	tmp_core = my_cores;
	i=1;
	while (tmp_core != NULL){
		core_inter_tail[my_idag]->data.app_cores[i] = tmp_core->core_id;
		tmp_core = tmp_core->next;
		i++;
	}

	core_inter_tail[my_idag]->next = NULL;

	if (core_inter_head[my_idag]->next == NULL) {
		kill(pid_num[my_idag], SIG_ADD_CORES_DDS);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,my_idag);
	} else printf("I am %d and i didn't call add!! with interaction %d\n",node_id,core_inter_head[my_idag]->type);

	if (my_agent != -1) {
		printf("I am %d and i do this agent switch\n",node_id);

		//if (state != IDLE_CORE) {
		if (state == WORKING_NODE || state == WORKING_NODE_IDLE_INIT) {
			if (core_inter_head[my_agent] == NULL){
				core_inter_head[my_agent] = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[my_agent] = core_inter_head[my_agent];
			} else {
				core_inter_tail[my_agent]->next = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[my_agent] = core_inter_tail[my_agent]->next;
			}

			core_inter_tail[my_agent]->type = REP_CHK_REM_TIME;
			core_inter_tail[my_agent]->data.work_time = upper_work_bound - time_worked;
			core_inter_tail[my_agent]->next = NULL;
		}	
	
		upper_work_bound = 0;
		my_agent = -1;
	}

	//sem_post(&node_sem[node_id]);
	sem_getvalue(&node_sem_out[sender_id],&i);
	fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
	fflush(log_file);
	if (sem_post(&node_sem_out[sender_id]) == -1){
		printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
		perror("sem_post");
	}
	sem_getvalue(&node_sem_out[sender_id],&i);
	fprintf(log_file, "Sem value = %d\n",i);
	fflush(log_file);

	printf("Init ok!! my_cores_count = %d app_cores=%d app_id=%d\n",my_cores_count,my_app.num_of_cores,my_app.id);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: Init ok!! my_cores_count = %d app_id=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,my_cores_count,my_app.id);
	fprintf(log_file, "my cores are:");
	for (tmp_core=my_cores; tmp_core!=NULL; tmp_core=tmp_core->next) fprintf(log_file, " %d", tmp_core->core_id);
	fprintf(log_file, "\n");

	printf("I am %d and about to do an selfopt agent with my state before change = %d\n",node_id,state);	
	if (state == IDLE_INIT_MAN || state == INIT_MANAGER || state == INIT_MANAGER_SEND_OFFERS || state == INIT_MAN_CHK_OFFERS || state == WORKING_NODE_IDLE_INIT) {
		fprintf(log_file,"I have pending init\n");
		//state = AGENT_INIT_STATE_INIT_INTERRUPTED;
		pending_state = state;
		state = AGENT_INIT_STATE;	
	} else state = AGENT_INIT_STATE;

	close(fd_r);
	free(fifo_name);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_INIT_AGENT_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

//an mou erthei to shma apo allon idag prepei apla na valw, an mou erthei apo common node tote prepei na upologisw
void sig_ADD_CORES_DDS_handler(int signo, siginfo_t *info, void *context)
{
	int fd_r, i, is_sender_idag, sender_id, j, new_agent_id; 
	int *nodes_to_process, nodes_cnt=0, processed_cnt=0;
	int *tmp_list, tmp_cnt, tmp_idag;	
	char *fifo_name;
	core_list *tmp_cores;//, *tmp_cores_list;
	DDS_list *tmp_DDS;

	signals_disable();
	sender_id = get_id_from_pid(info->si_pid);
	fifo_name = get_pipe_name(node_id);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_ADD_CORES_DDS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	

	sem_getvalue(&node_sem[node_id],&i);
	fprintf(log_file, "In 10 Trying to acquire semaphore. Sem value =  %d\n",i);
	fflush(log_file);
	kill(info->si_pid, SIG_ACK);		
	sem_wait(&node_sem[node_id]);	
	sem_getvalue(&node_sem[node_id],&i);
	fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
	fflush(log_file);
	my_stats.msg_count++;
	my_stats.distance += distance(node_id,sender_id);
	//printf("open11 idag_id=%d node_id=%d\n",idag_id,node_id);
	fd_r = open(fifo_name, O_RDONLY);
	
	if (fd_r == -1) {
		perror("opening reading pipe"); 
		signals_enable();		
		return;
	} else {
		fprintf(log_file, "I went through open\n");
		fflush(log_file);
	}	

	is_sender_idag = 0;
	for (i=0; i<num_idags; i++)
		if (idag_id_arr[i] == sender_id){
			is_sender_idag = 1;
			break;
		}

	//printf("I am in add with is_sender_idag = %d\n",is_sender_idag);
	if (is_sender_idag == 0){
		read(fd_r, &nodes_cnt, sizeof(int));
		while (nodes_cnt <=0){
			printf("i am %d and i read %d in nodes_cnt from %d\n",node_id,nodes_cnt,sender_id);
			read(fd_r, &nodes_cnt, sizeof(int));
		}
		nodes_to_process = (int *) malloc(nodes_cnt * sizeof(int));
		for (i=0; i<nodes_cnt; i++)
			read(fd_r, &nodes_to_process[i], sizeof(int));

		////sem_post(&node_sem[sender_id]);
		tmp_list = (int *) malloc(nodes_cnt * sizeof(int));

		printf("I am %d in add cores with sender %d and nodes_cnt = %d\n",node_id,sender_id,nodes_cnt);	

		while (processed_cnt < nodes_cnt){
			
			tmp_cnt = 0;
			tmp_idag = -1;

			for (i=0; i<nodes_cnt; i++){
				if (processed_cnt == nodes_cnt) break;
				if (nodes_to_process[i] == -1) continue;
				else {
					if (tmp_idag == -1) tmp_idag = idag_mask[nodes_to_process[i]];

					if (idag_mask[nodes_to_process[i]] != tmp_idag) continue;
					else {
						tmp_list[tmp_cnt++] = nodes_to_process[i];
						nodes_to_process[i] = -1;
						processed_cnt++;
					}
				}
			}

			if (tmp_idag != node_id){
				if (core_inter_head[tmp_idag] == NULL){
					core_inter_head[tmp_idag] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[tmp_idag] = core_inter_head[tmp_idag];
				} else {
					printf("Adding to DDS not in high priority! Shouldn't I high prioritize? tmp = %d idag = %d\n",tmp_idag,node_id);
					core_inter_tail[tmp_idag]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[tmp_idag] = core_inter_tail[tmp_idag]->next;
				}

				printf("In ADD tmp_idag = %d tmp_cnt = %d sender_id = %d node_id = %d\n",tmp_idag,tmp_cnt,sender_id,node_id);
				core_inter_tail[tmp_idag]->type = IDAG_ADD_CORES_DDS;
				core_inter_tail[tmp_idag]->data.app_cores = (int *)malloc((tmp_cnt+2)*sizeof(int));
				core_inter_tail[tmp_idag]->data.app_cores[0] = tmp_cnt;//+1;
				core_inter_tail[tmp_idag]->data.app_cores[tmp_cnt+1] = sender_id;

				for (j=1; j<= tmp_cnt; j++)
					core_inter_tail[tmp_idag]->data.app_cores[j] = tmp_list[j-1];
		
				core_inter_tail[tmp_idag]->next = NULL;

				if (core_inter_head[tmp_idag]->next == NULL) {
					kill(pid_num[tmp_idag], SIG_ADD_CORES_DDS);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,tmp_idag);
				}
			} else {
				printf("In ADD same tmp_idag = %d tmp_cnt = %d nodes_cnt = %d sender_id = %d node_id = %d\n",tmp_idag,tmp_cnt,nodes_cnt,sender_id,node_id);
				new_agent_id = sender_id;
				for (i=0; i<tmp_cnt; i++){//nodes_cnt
					fprintf(log_file,"I am importing node %d\n",tmp_list[i]);
					fflush(log_file);
					tmp_cores = my_cores;
			
					while (tmp_cores->core_id != tmp_list[i])//nodes_to_process[i]
						tmp_cores = tmp_cores->next;
					
					if (tmp_cores->offered_to == -1) {
						fprintf(log_file,"Node %d was offered to nobody\n",tmp_list[i]);
						fflush(log_file);
						DDS->num_of_cores--;
						tmp_cores->offered_to = new_agent_id;
					} else if (tmp_cores->offered_to != new_agent_id) {
						fprintf(log_file,"Node %d is offered to %d\n",tmp_list[i],tmp_cores->offered_to);
						fflush(log_file);
						tmp_cores->offered_to = new_agent_id;
					}
				}

				tmp_DDS = DDS;
				while (tmp_DDS != NULL && tmp_DDS->agent_id != new_agent_id)
					tmp_DDS = tmp_DDS->next;

				if (tmp_DDS != NULL)
					tmp_DDS->num_of_cores += nodes_cnt;
				else {
					//printf("I am in here DDSing nodes_cnt=%d\n",nodes_cnt);
					DDS_tail->next = (DDS_list *) malloc(sizeof(DDS_list));
					DDS_tail = DDS_tail->next;
					DDS_tail->agent_id = new_agent_id;
					DDS_tail->num_of_cores = nodes_cnt;
					DDS_tail->next = NULL;
					DDS_count++;
				}	
			}						
		}
	}	else {
		read(fd_r, &nodes_cnt, sizeof(int));
		nodes_to_process = (int *) malloc(nodes_cnt * sizeof(int));
		for (i=0; i<nodes_cnt; i++)
			read(fd_r, &nodes_to_process[i], sizeof(int));

		read(fd_r, &new_agent_id, sizeof(int));
		////sem_post(&node_sem[sender_id]);
		//printf("new agent id = %d\n",new_agent_id);		

		for (i=0; i<nodes_cnt; i++){
			//printf("Node %d is %d\n",i,nodes_to_process[i]);
			fprintf(log_file,"I am importing node %d\n",nodes_to_process[i]);
			fflush(log_file);
			tmp_cores = my_cores;

			while (tmp_cores->core_id != nodes_to_process[i])
				tmp_cores = tmp_cores->next;
		

			if (tmp_cores->offered_to == -1) {
				fprintf(log_file,"Node %d was offered to nobody\n",nodes_to_process[i]);
				fflush(log_file);
				DDS->num_of_cores--;
				tmp_cores->offered_to = new_agent_id;
			} else if (tmp_cores->offered_to != new_agent_id) {
				fprintf(log_file,"Node %d is offered to %d\n",nodes_to_process[i],tmp_cores->offered_to);
				fflush(log_file);
				tmp_cores->offered_to = new_agent_id;
			}
			//tmp_cores->offered_to = new_agent_id;
		}

		tmp_DDS = DDS;
		while (tmp_DDS != NULL && tmp_DDS->agent_id != new_agent_id)
			tmp_DDS = tmp_DDS->next;

		if (tmp_DDS != NULL)
			tmp_DDS->num_of_cores += nodes_cnt;
		else {
			if (DDS_count == 1 && DDS != DDS_tail) printf("I am %d kai ta hpiame asxhma!\n",node_id);
			DDS_tail->next = (DDS_list *) malloc(sizeof(DDS_list));
			DDS_tail = DDS_tail->next;
			DDS_tail->agent_id = new_agent_id;
			DDS_tail->num_of_cores = nodes_cnt;
			DDS_tail->next = NULL;
			DDS_count++;
			//printf("I did this with node_id = %d, new_agent_id = %d\n",node_id,new_agent_id);
		}
	}					

	//sem_post(&node_sem[node_id]);
	sem_getvalue(&node_sem_out[sender_id],&i);
	fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
	fflush(log_file);
	if (sem_post(&node_sem_out[sender_id]) == -1){
		printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
		perror("sem_post");
	}
	sem_getvalue(&node_sem_out[sender_id],&i);
	fprintf(log_file, "Sem value = %d\n",i);
	fflush(log_file);

	/*for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next)
		if (tmp_cores_list->offered_to != -1) my_stats.cores_utilized++;
	
	printf("I am %d Adding ended well with sender_id=%d!\n",node_id,sender_id);
	printf("Number of agents in region = %d\n",DDS_count);	
	i=0;
	for (tmp_DDS = DDS; tmp_DDS != NULL; tmp_DDS = tmp_DDS->next){
		printf("Agent no %d is %d with %d cores\n",i,tmp_DDS->agent_id,tmp_DDS->num_of_cores);	
		i++;
	}*/

	my_stats.times_accessed++;
	printf("I am %d Adding ended well with sender_id=%d!\n",node_id,sender_id);
	printf("Number of agents in region = %d\n",DDS_count);	
	printf("Agent no 0 is %d with %d cores\n",DDS->agent_id,DDS->num_of_cores);	
	i=1;
	for (tmp_DDS = DDS->next; tmp_DDS != NULL; tmp_DDS = tmp_DDS->next){
		printf("Agent no %d is %d with %d cores\n",i,tmp_DDS->agent_id,tmp_DDS->num_of_cores);
		my_stats.cores_utilized += tmp_DDS->num_of_cores;
		i++;
	}
	
	close(fd_r);
	free(fifo_name);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_ADD_CORES_DDS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

void sig_REM_CORES_DDS_handler(int signo, siginfo_t *info, void *context)
{
	int fd_r, i, is_sender_idag, sender_id, j, new_agent_id; 
	int *nodes_to_process, nodes_cnt=0, processed_cnt=0;
	int *tmp_list, tmp_cnt, tmp_idag;	
	char *fifo_name;
	DDS_list *tmp_DDS,*tmp_DDS_prev;

	signals_disable();
	sender_id = get_id_from_pid(info->si_pid);
	fifo_name = get_pipe_name(node_id);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_REM_CORES_DDS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	

	sem_getvalue(&node_sem[node_id],&i);
	fprintf(log_file, "In 11 Trying to acquire semaphore. Sem value =  %d\n",i);
	fflush(log_file);
	kill(info->si_pid, SIG_ACK);		
	sem_wait(&node_sem[node_id]);
	sem_getvalue(&node_sem[node_id],&i);
	fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
	fflush(log_file);
	my_stats.msg_count++;
	my_stats.distance += distance(node_id,sender_id);
	//printf("open11 idag_id=%d node_id=%d\n",idag_id,node_id);
	fd_r = open(fifo_name, O_RDONLY);
	
	if (fd_r == -1) {
		perror("opening reading pipe"); 
		signals_enable();		
		return;
	} else {
		fprintf(log_file, "I went through open\n");
		fflush(log_file);
	}	

	is_sender_idag = 0;
	for (i=0; i<num_idags; i++)
		if (idag_id_arr[i] == sender_id){
			is_sender_idag = 1;
			break;
		}

	//printf("I am in rem with is_sender_idag = %d\n",is_sender_idag);
	if (is_sender_idag == 0){
		read(fd_r, &nodes_cnt, sizeof(int));
		nodes_to_process = (int *) malloc(nodes_cnt * sizeof(int));
		for (i=0; i<nodes_cnt; i++)
			read(fd_r, &nodes_to_process[i], sizeof(int));

		////sem_post(&node_sem[sender_id]);
		tmp_list = (int *) malloc(nodes_cnt * sizeof(int));

		while (processed_cnt < nodes_cnt){
			
			tmp_cnt = 0;
			tmp_idag = -1;

			for (i=0; i<nodes_cnt; i++){
				if (processed_cnt == nodes_cnt) break;
				if (nodes_to_process[i] == -1) continue;
				else {
					if (tmp_idag == -1) tmp_idag = idag_mask[nodes_to_process[i]];

					if (idag_mask[nodes_to_process[i]] != tmp_idag) continue;
					else {
						tmp_list[tmp_cnt++] = nodes_to_process[i];
						nodes_to_process[i] = -1;
						processed_cnt++;
					}
				}
			}

			if (tmp_idag != node_id){
				if (core_inter_head[tmp_idag] == NULL){
					core_inter_head[tmp_idag] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[tmp_idag] = core_inter_head[tmp_idag];
				} else {
					printf("Removing from DDS not in high priority! Shouldn't I high prioritize? tmp = %d idag = %d\n",tmp_idag,node_id);
					core_inter_tail[tmp_idag]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[tmp_idag] = core_inter_tail[tmp_idag]->next;
				}

				//printf("In REM tmp_idag = %d tmp_cnt = %d sender_id = %d idag_id = %d\n",tmp_idag,tmp_cnt,sender_id,idag_id);
				core_inter_tail[tmp_idag]->type = IDAG_REM_CORES_DDS;
				core_inter_tail[tmp_idag]->data.app_cores = (int *)malloc((tmp_cnt+2)*sizeof(int));
				core_inter_tail[tmp_idag]->data.app_cores[0] = tmp_cnt;//+1;
				core_inter_tail[tmp_idag]->data.app_cores[tmp_cnt+1] = sender_id;

				for (j=1; j<= tmp_cnt; j++)
					core_inter_tail[tmp_idag]->data.app_cores[j] = tmp_list[j-1];
		
				core_inter_tail[tmp_idag]->next = NULL;

				if (core_inter_head[tmp_idag]->next == NULL) {
					kill(pid_num[tmp_idag], SIG_REM_CORES_DDS);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,tmp_idag);
				}
			} else {
				printf("In REM node_id = %d same tmp_idag = %d tmp_cnt = %d nodes_cnt = %d sender_id = %d \n",node_id,tmp_idag,tmp_cnt,nodes_cnt,sender_id);
				new_agent_id = sender_id;
				
				tmp_DDS = DDS;
				tmp_DDS_prev = NULL;	
				while (tmp_DDS != NULL && tmp_DDS->agent_id != new_agent_id){
					tmp_DDS_prev = tmp_DDS;
					tmp_DDS = tmp_DDS->next;
				}

				if (tmp_DDS == NULL) printf("Agent does not exist in my DDS\n");
				else if (tmp_DDS == DDS) printf("I am removing from myself in REM?\n");
				else {
					tmp_DDS->num_of_cores -= nodes_cnt;
					if (tmp_DDS->num_of_cores == 0){
						DDS_count--;
						if (tmp_DDS == DDS_tail){
							DDS_tail = tmp_DDS_prev;
							DDS_tail->next = NULL;
						} else tmp_DDS_prev->next = tmp_DDS->next;

						free(tmp_DDS);
					}
				}	
			}						
		}
	}	else { //den afairw tous purhnes apo thn core list giati mporei na exoun hdh ginei add
		read(fd_r, &nodes_cnt, sizeof(int));
		nodes_to_process = (int *) malloc(nodes_cnt * sizeof(int));
		for (i=0; i<nodes_cnt; i++)
			read(fd_r, &nodes_to_process[i], sizeof(int));

		read(fd_r, &new_agent_id, sizeof(int));
		////sem_post(&node_sem[sender_id]);		
		//printf("I am in the second rem with new agent id = %d and nodes_cnt = %d\n",new_agent_id,nodes_cnt);		

		tmp_DDS = DDS;
		tmp_DDS_prev = NULL;	
		while (tmp_DDS != NULL && tmp_DDS->agent_id != new_agent_id){
			tmp_DDS_prev = tmp_DDS;
			tmp_DDS = tmp_DDS->next;
		}

		if (tmp_DDS == NULL) printf("Agent does not exist in my DDS\n");
		else if (tmp_DDS == DDS) printf("I am removing from myself in REM?\n");
		else {
			tmp_DDS->num_of_cores -= nodes_cnt;

			if (tmp_DDS->num_of_cores == 0){
				DDS_count--;
				if (tmp_DDS == DDS_tail){
					DDS_tail = tmp_DDS_prev;
					DDS_tail->next = NULL;
				} else tmp_DDS_prev->next = tmp_DDS->next;

				free(tmp_DDS);
			}
		}	
	}					

	//sem_post(&node_sem[node_id]);
	sem_getvalue(&node_sem_out[sender_id],&i);
	fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
	fflush(log_file);
	if (sem_post(&node_sem_out[sender_id]) == -1){
		printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
		perror("sem_post");
	}
	sem_getvalue(&node_sem_out[sender_id],&i);
	fprintf(log_file, "Sem value = %d\n",i);
	fflush(log_file);
	printf("I am %d Removing ended well! with sender_id=%d!\n",node_id,sender_id);
	printf("Number of agents in region = %d\n",DDS_count);	
	i=0;
	for (tmp_DDS = DDS; tmp_DDS != NULL; tmp_DDS = tmp_DDS->next){
		printf("Agent no %d is %d with %d cores\n",i,tmp_DDS->agent_id,tmp_DDS->num_of_cores);	
		i++;
	}	


	close(fd_r);
	free(fifo_name);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_REM_CORES_DDS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

void sig_FAR_REQ_handler(int signo, siginfo_t *info, void *context)
{
	int fd_r=-1, sender_id, i, one_idag, *idags_in_reg, agent_id, num_of_offers, j, far_req_man, one_far_sender;//num_of_idags, 
	char *fifo_name;
	region cur_reg;
	offer one_offer;
	inter_list *tmp_inter_list, *tmp_inter_prev;
	DDS_list *tmp_DDS;
	offer_list *tmp_offer_list, *tmp_offer_prev;
	app one_app;

	signals_disable();
	
	sender_id = get_id_from_pid(info->si_pid);
	fifo_name = get_pipe_name(node_id);	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_FAR_REQ_handler with sender=%d and signo=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,signo,state);
	fflush(log_file);
	//I am the node that is far. 
	//I keep that as NULL because this must be the way, or else why is he sending me smth if we are allready communicating?
	if (core_inter_head[sender_id] == NULL && signo == SIG_FAR_REQ && my_idag != -1) {	
		printf("I am %d and i think i cought a stray far_manager reply from %d. My current state is %d\n",node_id,sender_id,state);
		fprintf(log_file,"I am %d and i think i cought a stray far_manager reply from %d. My current state is %d\n",node_id,sender_id,state);
		fflush(log_file);

		kill(info->si_pid, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	} else if (core_inter_head[sender_id] == NULL && (signo == SIG_INIT_FAR_REQ || (signo == SIG_FAR_REQ && my_idag == -1))) {
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "In 12 Trying to acquire semaphore. Sem value =  %d\n",i);
		fflush(log_file);
		kill(info->si_pid, SIG_ACK);		
		sem_wait(&node_sem[node_id]);
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
		fflush(log_file);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);	
		//printf("I am inside with sender_id %d %s\n",sender_id,fifo_name);
		//printf("open12 idag_id=%d node_id=%d\n",idag_id,node_id);
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		} /*else {
			fprintf(log_file, "I went through open\n");
			fflush(log_file);
		}	*/
		
		if (signo == SIG_INIT_FAR_REQ) {
			//far_req_or_sender = sender_id;
			one_far_sender = sender_id;
	
			//read(fd_r, &far_req_app, sizeof(app));
			read(fd_r, &one_app, sizeof(app));				
			read(fd_r, &cur_reg, sizeof(region));

			if (core_inter_head[one_far_sender] == NULL){
				core_inter_head[one_far_sender] = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[one_far_sender] = core_inter_head[one_far_sender];
			} else {
				core_inter_tail[one_far_sender]->next = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[one_far_sender] = core_inter_tail[one_far_sender]->next;
			}

			core_inter_tail[one_far_sender]->type = REP_FAR_INIT_REQ;
			if (my_idag != -1) {
				far_req_man	= my_idag;
				if (core_inter_head[far_req_man] == NULL){
					core_inter_head[far_req_man] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[far_req_man] = core_inter_head[far_req_man];
				} else {
					core_inter_tail[far_req_man]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[far_req_man] = core_inter_tail[far_req_man]->next;
				}

				//far_req_or_sender = one_far_sender;
				//far_req_app = one_app;
				core_inter_tail[far_req_man]->type = FAR_REQ_MAN_APPOINT;
				core_inter_tail[far_req_man]->data.far_req.orig_sender = one_far_sender;				
				core_inter_tail[far_req_man]->data.far_req.far_app = one_app;				
				core_inter_tail[far_req_man]->data.far_req.reg = cur_reg;
				core_inter_tail[far_req_man]->next = NULL;
				//core_inter_tail[far_req_or_sender]->data.far_req_man = my_idag;
			} else {
				far_req_man = node_id;
				if (far_req_or_sender == -1) {
					far_req_or_sender = one_far_sender;
					far_req_app = one_app;
				}
			}
				//core_inter_tail[one_far_sender]->data.far_req_man = node_id;
			core_inter_tail[one_far_sender]->data.far_req_man = far_req_man; 
			core_inter_tail[one_far_sender]->next = NULL;

			if (core_inter_head[one_far_sender]->next == NULL) {
				kill(pid_num[one_far_sender], SIG_INIT_FAR_REQ);
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,one_far_sender);
			} else printf("I am %d and did not send %d one_far_sender init_far_req in here. interaction = %d\n",node_id,one_far_sender,core_inter_head[one_far_sender]->type);
		} else { //SIG_FAR_REQ
			//read(fd_r, &one_far_sender, sizeof(int));
			//read(fd_r, &far_req_app, sizeof(app));
			read(fd_r, &one_far_sender, sizeof(int));
			read(fd_r, &one_app, sizeof(app));
			if (far_req_or_sender == -1) {
				far_req_or_sender = one_far_sender;
				far_req_app = one_app;
				//printf("I am %d and i come here far_req_or_sender = %d\n",node_id,far_req_or_sender);
			}	//else printf("I am %d and karioles far_req_or_sender = %d one_far_sender = %d\n",node_id,far_req_or_sender,one_far_sender);			
			read(fd_r, &cur_reg, sizeof(region));
			while (cur_reg.C > X_max * Y_max){
				printf("I am %d kai mas taizei malakies o %d\n",node_id,sender_id);
				read(fd_r, &cur_reg, sizeof(region));
			}
			fprintf(log_file,"one_far_sender=%d far_req_or_sender = %d\n",one_far_sender,far_req_or_sender);
			fflush(log_file);
		}

		if (signo == SIG_FAR_REQ || (signo == SIG_INIT_FAR_REQ && far_req_man == node_id)) {
			if (far_req_or_sender == one_far_sender && far_man_offers == NULL) {
				printf("I am node with id %d I am to far check-manage region C=%d r=%d for %d\n",node_id,cur_reg.C,cur_reg.r,far_req_or_sender);
				fprintf(log_file,"I am node with id %d I am to far check-manage region C=%d r=%d for %d\n",node_id,cur_reg.C,cur_reg.r,far_req_or_sender);
				fflush(log_file);				
				//my offer
				if (far_man_offers == NULL){
					far_list_count = 1;
					far_man_offers = (offer_list *) malloc(sizeof(offer_list));
					far_man_offers_tail = far_man_offers;
				} else printf ("I am %d Far man offers list not NULL far_req_or_sender = %d one_far_sender = %d\n",node_id,far_req_or_sender,one_far_sender);

				far_man_offers_tail->off.offered_cores = (int *) malloc(my_cores_count*sizeof(int));		
				far_man_offers_tail->off.num_of_cores = offer_cores(my_cores, far_req_app, cur_reg, far_man_offers_tail->off.offered_cores, far_req_or_sender);
				my_stats.comp_effort++;
				//if (my_idag == -1) 
				far_man_offers_tail->off.spd_loss = 0.0;
				//else far_man_offers_tail->off.spd_loss = Speedup(my_app, my_cores_count) - Speedup(my_app, my_cores_count-far_man_offers_tail->off.num_of_cores);
			
				far_man_offers_tail->sender = node_id;
				far_man_offers_tail->next = NULL;
				//far_list_count++;

				cur_time = time(NULL);	
				cur_t = localtime(&cur_time);
				fprintf(log_file, "[%d:%d:%d]: I added my offer in far_man_offers list\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);
				fflush(log_file);

				idags_in_reg = (int *) malloc((num_idags+1)*sizeof(int));
				get_reg_idags(cur_reg, idags_in_reg);

				for (i=0; i<num_idags; i++)
					if (idags_in_reg[i] && idag_id_arr[i] != node_id){				
						one_idag = idag_id_arr[i];
					
						if (core_inter_head[one_idag] == NULL){
							core_inter_head[one_idag] = (inter_list *) malloc(sizeof(inter_list));
							core_inter_tail[one_idag] = core_inter_head[one_idag];
						} else {
							core_inter_tail[one_idag]->next = (inter_list *) malloc(sizeof(inter_list));
							core_inter_tail[one_idag] = core_inter_tail[one_idag]->next;
						}

						//if (signo == SIG_FAR_REQ) 
						//else core_inter_tail[one_idag]->type = FAR_INIT_IDAG_REQ_DDS;
						core_inter_tail[one_idag]->type = FAR_REQ_IDAG_REQ_DDS_PENDING;
						core_inter_tail[one_idag]->data.reg = cur_reg;
						core_inter_tail[one_idag]->next = NULL;

						if (core_inter_head[one_idag]->next == NULL) {
							kill(pid_num[one_idag], SIG_REQ_DDS); //newly created
							my_stats.msg_count += 2;
							my_stats.distance += 2 * distance(node_id,one_idag);
						} else printf("I am %d and i want far dds from %d but interaction is %d\n",node_id,one_idag,core_inter_head[one_idag]->type);
					}

				//printf("uiou 0 qw %d num_of_idags = %d idag_id=%d\n",idags_in_reg[idag_id],idags_in_reg[num_idags],idag_id);
				//for (i=0; i<num_idags; i++)
					//printf("idag i=%d node_num = %d idags_in_reg = %d\n",i,idag_id_arr[i],idags_in_reg[i]);
			
				//asking offers from other agents in my DDS
				tmp_DDS = DDS->next;
				if (tmp_DDS != NULL) {
					while (tmp_DDS != NULL){					
						agent_id = tmp_DDS->agent_id;						
						if (core_inter_head[agent_id] == NULL){
							core_inter_head[agent_id] = (inter_list *) malloc(sizeof(inter_list));
							core_inter_tail[agent_id] = core_inter_head[agent_id];
						
							core_inter_tail[agent_id]->type = FAR_REQ_CORES_PENDING;
							core_inter_tail[agent_id]->data.reg = cur_reg;//core_inter_head[sender_id]->data.reg;
							core_inter_tail[agent_id]->next = NULL;
	
							//if (core_inter_head[agent_id]->next == NULL) {
							kill(pid_num[agent_id], SIG_REQ_CORES);
							my_stats.msg_count++;
							my_stats.distance += distance(node_id,agent_id);
							//}	else printf("!!!!! I am %d and in sending local-far offer to %d i have interaction %d\n",node_id,agent_id,core_inter_head[agent_id]->type);
						} else {
							printf("I did not send I am %d and in sending local-far offer to %d i have interaction %d\n",node_id,agent_id,core_inter_head[agent_id]->type);
							//core_inter_tail[agent_id]->next = (inter_list *) malloc(sizeof(inter_list));
							//core_inter_tail[agent_id] = core_inter_tail[agent_id]->next;
						}
					
						tmp_DDS = tmp_DDS->next;
	 				}
				
					if (state != IDLE_CHK_APP_FILE) {
						its.it_value.tv_nsec = 250 * MS;
						if (timer_settime(timerid, 0, &its, NULL) == -1) printf("timer_settime error2\n");	
						state = IDLE_FAR_MAN;
					} else time_for_farman = 25;		
				} else if (idags_in_reg[num_idags] == 1){//i am the only idag in region with no agents
					if (core_inter_head[far_req_or_sender] == NULL){
						core_inter_head[far_req_or_sender] = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[far_req_or_sender] = core_inter_head[far_req_or_sender];
					} else {
						core_inter_tail[far_req_or_sender]->next = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[far_req_or_sender] = core_inter_tail[far_req_or_sender]->next;
					}

					core_inter_tail[far_req_or_sender]->type = FAR_REQ_OFFER;
					core_inter_tail[far_req_or_sender]->data.my_offer = far_man_offers->off;
					core_inter_tail[far_req_or_sender]->next = NULL;

					if (core_inter_head[far_req_or_sender]->next == NULL) {
						kill(pid_num[far_req_or_sender],SIG_FAR_REQ);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,far_req_or_sender);
					} else printf("I am %d and i am doing smth else with far_req_or_sender %d interaction %d\n",node_id,far_req_or_sender,core_inter_head[far_req_or_sender]->type);
				} else {
					if (state != IDLE_CHK_APP_FILE) {
						its.it_value.tv_nsec = 250 * MS;
						if (timer_settime(timerid, 0, &its, NULL) == -1) printf("timer_settime error3\n");	
						state = IDLE_FAR_MAN;
					} else time_for_farman = 25;		
				}
			} else if (far_req_or_sender != one_far_sender) {
				fprintf(log_file,"I am node with id %d and I am allready far managing %d so i am going to send %d only my offer\n",node_id,far_req_or_sender,one_far_sender);
				fflush(log_file);

				if (core_inter_head[one_far_sender] == NULL){
					core_inter_head[one_far_sender] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[one_far_sender] = core_inter_head[one_far_sender];
				
					core_inter_tail[one_far_sender]->type = REP_AGENT_REQ_CORES;
				
					core_inter_tail[one_far_sender]->data.off_arr.num_of_offers = 1;
					core_inter_tail[one_far_sender]->data.off_arr.offer_arr = (offer *) malloc(core_inter_tail[one_far_sender]->data.off_arr.num_of_offers * sizeof(offer));
					core_inter_tail[one_far_sender]->next = NULL;

					core_inter_tail[one_far_sender]->data.off_arr.offer_arr[0].offered_cores = (int *) malloc(my_cores_count*sizeof(int));		
					j = offer_cores(my_cores, one_app, cur_reg, core_inter_tail[one_far_sender]->data.off_arr.offer_arr[0].offered_cores, one_far_sender);
					core_inter_tail[one_far_sender]->data.off_arr.offer_arr[0].num_of_cores = j;
					my_stats.comp_effort++;
					core_inter_tail[one_far_sender]->data.off_arr.offer_arr[0].spd_loss = 0.0;
				
					i=0;
					fprintf(log_file, "I offered in far only %d %d cores: ",one_far_sender,core_inter_tail[one_far_sender]->data.off_arr.offer_arr[i].num_of_cores);
					for (j=0; j<core_inter_tail[one_far_sender]->data.off_arr.offer_arr[i].num_of_cores; j++)
						fprintf(log_file, "%d, ",core_inter_tail[one_far_sender]->data.off_arr.offer_arr[i].offered_cores[j]);
					fprintf(log_file, "\n");
					fflush(log_file);
			
					//get_reg_idags(cur_reg, core_inter_tail[one_far_sender]->data.idags_in_reg);
					//if (core_inter_head[one_far_sender]->next == NULL) {
					kill(pid_num[one_far_sender], SIG_FAR_REQ);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,one_far_sender);
					//} else printf("i am %d and Apparently not null interaction in far only=%d\n",node_id,core_inter_head[one_far_sender]->type);
				} else {
					fprintf(log_file,"den to evala I am %d with one_far_sender %d in only my offer and interaction is %d\n",node_id,one_far_sender,core_inter_head[one_far_sender]->type);
					fflush(log_file);	
					//core_inter_tail[one_far_sender]->next = (inter_list *) malloc(sizeof(inter_list));
					//core_inter_tail[one_far_sender] = core_inter_tail[one_far_sender]->next;
				}

				//far_man_agent_count = DDS_count;
				//printf("uioy 1 far_list_count = %d far_man_agent_count=%d\n",far_list_count,far_man_agent_count);
			} else printf("I am node with id %d and I am allready far managing %d so i am going to abort re managing him\n",node_id,far_req_or_sender);
		} else printf("I am node with id %d and I am to far check region C=%d r=%d with sender %d\n",node_id,cur_reg.C,cur_reg.r,sender_id);
	
	
		//sem_post(&node_sem[node_id]);
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
		fflush(log_file);
		if (sem_post(&node_sem_out[sender_id]) == -1){
			printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
			perror("sem_post");
		}
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "Sem value = %d\n",i);
		fflush(log_file);
	} else if (core_inter_head[sender_id]->type == FAR_INIT_REQ && signo == SIG_INIT_FAR_REQ) {//in original manager change who the far manager is
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "In 13 Trying to acquire semaphore. Sem value =  %d\n",i);
		fflush(log_file);
		kill(info->si_pid, SIG_ACK);		
		sem_wait(&node_sem[node_id]);
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
		fflush(log_file);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);	
		//printf("I am inside with sender_id %d %s\n",sender_id,fifo_name);
		//printf("open13 idag_id=%d node_id=%d\n",idag_id,node_id);
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		} /*else {
			fprintf(log_file, "I went through open\n");
			fflush(log_file);
		}	*/

		read(fd_r, &agent_id, sizeof(int));
		////sem_post(&node_sem[sender_id]);

		for (tmp_inter_list = core_inter_head[agent_id]; tmp_inter_list != NULL; tmp_inter_list=tmp_inter_list->next)
			if (tmp_inter_list->type == FAR_REQ_MAN) break; 
				
		if (tmp_inter_list == NULL) {
			if (core_inter_head[agent_id] == NULL){
				core_inter_head[agent_id] = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[agent_id] = core_inter_head[agent_id];
			} else {
				core_inter_tail[agent_id]->next = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[agent_id] = core_inter_tail[agent_id]->next;
			}

			core_inter_tail[agent_id]->type = FAR_REQ_MAN;
			core_inter_tail[agent_id]->data.far_req_man = sender_id;
			core_inter_tail[agent_id]->next = NULL;

			printf("I am node with id %d and my far request manager is %d\n",node_id,agent_id);
			fprintf(log_file,"I am node with id %d and my far request manager is %d\n",node_id,agent_id);
			fflush(log_file);

		} else printf("I am node with id %d my far request manager is %d and i am aborting him being appointed again by sender=%d\n",node_id,agent_id,sender_id);

		/*if (agent_id == sender_id) {
			tmp_inter_prev = NULL;
			tmp_inter_list = core_inter_head[agent_id];//core_inter_head[agent_id]->next;
		} else {
			tmp_inter_prev = core_inter_head[agent_id];
			tmp_inter_list = core_inter_head[agent_id]->next;
		}*/

		tmp_inter_prev = core_inter_head[agent_id];
		tmp_inter_list = core_inter_head[agent_id]->next;		
		j = 0;

		while (tmp_inter_list != NULL) {
			if (tmp_inter_list->type == FAR_INIT_REQ || tmp_inter_list->type == IDAG_REQ_DDS_PENDING) {

				if (tmp_inter_list->type == IDAG_REQ_DDS_PENDING){
					init_DDS_replies++;
					if (init_DDS_idags == init_DDS_replies && init_idags_areas_replies == INIT_AREAS_NUM) j=1; //INIT_AREAS_NUM send_init_reqs(sender_id);
				}

				fprintf(log_file, "I dismissed type = %d of sender = %d\n",tmp_inter_list->type,agent_id);
				fflush(log_file);

				if (tmp_inter_prev == NULL) {
					core_inter_head[agent_id] = core_inter_head[agent_id]->next;
					free(tmp_inter_list);
					tmp_inter_list = core_inter_head[agent_id];
				} else {						
					tmp_inter_prev->next = tmp_inter_list->next;					
					if (tmp_inter_prev->next == NULL) core_inter_tail[agent_id] = tmp_inter_prev;

					free(tmp_inter_list);
					tmp_inter_list = tmp_inter_prev->next;
				}
			}
			else tmp_inter_list = tmp_inter_list->next;
		}

		if (j) send_init_reqs(sender_id);

		//sem_post(&node_sem[node_id]);
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
		fflush(log_file);
		if (sem_post(&node_sem_out[sender_id]) == -1){
			printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
			perror("sem_post");
		}
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "Sem value = %d\n",i);
		fflush(log_file);
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);
		free(tmp_inter_list);
	} else if (core_inter_head[sender_id]->type == FAR_REQ_MAN && signo == SIG_FAR_REQ 
			&& state != IDLE_INIT_MAN && state != IDLE_INIT_MAN_SELFOPT_PENDING && state != IDLE_INIT_MAN_WORK_PENDING) {
		printf("I am %d and i think i cought a stray init far_REQ_CORES reply from %d. My current state is %d\n",node_id,sender_id,state);
		fprintf(log_file,"I am %d and i think i cought a stray init far_REQ_CORES reply from %d. My current state is %d\n",node_id,sender_id,state);
		fflush(log_file);

		kill(info->si_pid, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		/*kill(info->si_pid, SIG_ACK);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		}		
		
		read(fd_r, &num_of_offers, sizeof(int));
		core_inter_head[sender_id]->data.offer_acc_array = (int *) malloc((num_of_offers+1)*sizeof(int));	
		core_inter_head[sender_id]->data.offer_acc_array[0] = num_of_offers;			
		
		for (j=1; j<=num_of_offers; j++){ 
			read(fd_r, &one_offer.num_of_cores, sizeof(int));
			read(fd_r, &one_offer.spd_loss, sizeof(float));

			for (i=0; i<one_offer.num_of_cores; i++)
				read(fd_r, &tmp_int, sizeof(int));
				
			core_inter_head[sender_id]->data.offer_acc_array[j] = 0;
		}

		core_inter_head[sender_id]->type = REP_FAR_REQ_OFFER_SENT;
		kill(info->si_pid, SIG_REP_OFFERS);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);	*/
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		free(tmp_inter_list);
		
		if(core_inter_head[sender_id] == NULL) {
			if (core_inter_tail[sender_id] == NULL){
				printf("Malaka gamietai to core_inter_tail\n");				
				core_inter_tail[sender_id] = NULL;
			}
		} else send_next_signal(core_inter_head[sender_id], sender_id);
	} else if (core_inter_head[sender_id]->type == FAR_REQ_MAN && signo == SIG_FAR_REQ) {//in original manager get far offers
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "In 14 Trying to acquire semaphore. Sem value =  %d\n",i);
		fflush(log_file);
		kill(info->si_pid, SIG_ACK);		
		sem_wait(&node_sem[node_id]);
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
		fflush(log_file);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);	
		//printf("I am inside with sender_id %d %s\n",sender_id,fifo_name);
		//printf("open14 idag_id=%d node_id=%d\n",idag_id,node_id);
		fd_r = open(fifo_name, O_RDONLY);
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		} /*else {
			fprintf(log_file, "I went through open\n");
			fflush(log_file);
		}	*/

		read(fd_r, &num_of_offers, sizeof(int));
		fprintf(log_file,"num of offers = %d\n",num_of_offers);
		fflush(log_file);
		core_inter_head[sender_id]->type = REP_FAR_REQ_OFFER_PENDING;
		core_inter_head[sender_id]->data.offer_acc_array = (int *) malloc((num_of_offers+1)*sizeof(int));
		core_inter_head[sender_id]->data.offer_acc_array[0] = num_of_offers;
		//printf("qweNode %d num_of_offers %d \n",sender_id,num_of_offers);

		for (i=1; i<=num_of_offers; i++){
			read(fd_r, &one_offer.num_of_cores, sizeof(int));
			read(fd_r, &one_offer.spd_loss, sizeof(float));
			one_offer.offered_cores = (int *) malloc(one_offer.num_of_cores*sizeof(int));
			for (j=0; j<one_offer.num_of_cores; j++)
				read(fd_r, &one_offer.offered_cores[j], sizeof(int));
			//printf("qweNode %d is offering %d cores with speedup loss %f\n",sender_id,one_offer.num_of_cores,one_offer.spd_loss);
			
			core_inter_head[sender_id]->data.offer_acc_array[i] = -1;
			if (init_man_offers == NULL) {
				init_man_offers = (offer_list *) malloc(sizeof(offer_list));
				init_man_offers->off = one_offer;
				init_man_offers->sender = sender_id;
				init_man_offers->answer = &core_inter_head[sender_id]->data.offer_acc_array[i];
				init_man_offers->next = NULL;	
			} else {
				tmp_offer_prev = NULL;
				tmp_offer_list = init_man_offers;
				while (tmp_offer_list != NULL && tmp_offer_list->off.num_of_cores >= one_offer.num_of_cores){
					tmp_offer_prev = tmp_offer_list;			
					tmp_offer_list = tmp_offer_list->next; 
				}

				if (tmp_offer_list == NULL) { //prepei na mpei teleutaio
					tmp_offer_prev->next = (offer_list *) malloc(sizeof(offer_list));
					tmp_offer_list = tmp_offer_prev->next;
					tmp_offer_list->next = NULL;
					tmp_offer_list->off = one_offer;
					tmp_offer_list->sender = sender_id;
					tmp_offer_list->answer = &core_inter_head[sender_id]->data.offer_acc_array[i];
				} else if (tmp_offer_prev == NULL) { //prepei na mpei prwto
					init_man_offers = (offer_list *) malloc(sizeof(offer_list));
					init_man_offers->off = one_offer;
					init_man_offers->sender = sender_id;
					init_man_offers->answer = &core_inter_head[sender_id]->data.offer_acc_array[i];
					init_man_offers->next = tmp_offer_list;
				} else {
					tmp_offer_prev->next = (offer_list *) malloc(sizeof(offer_list));
					tmp_offer_prev = tmp_offer_prev->next;
					tmp_offer_prev->off = one_offer;
					tmp_offer_prev->sender = sender_id;
					tmp_offer_prev->answer = &core_inter_head[sender_id]->data.offer_acc_array[i];
					tmp_offer_prev->next = tmp_offer_list;
				}
			}
			cur_time = time(NULL);	
			cur_t = localtime(&cur_time);
			fprintf(log_file, "[%d:%d:%d]: One node added in init_man_offers in sig_FAR_REQ\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);
			fflush(log_file);
		}
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
		fflush(log_file);
		if (sem_post(&node_sem_out[sender_id]) == -1){
			printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
			perror("sem_post");
		}
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "Sem value = %d\n",i);
		fflush(log_file);
		//sem_post(&node_sem[node_id]);
		////sem_post(&node_sem[sender_id]);
	} else {
		//printf("I am %d and We were fucked in sig_REQ_DDS_handler from %d interaction is = %d\n",node_id,sender_id,core_inter_head[sender_id]->type);
		printf("I am %d and i have to reject sig_far_req from %d. Our interaction is %d sig=%d\n",node_id,sender_id,core_inter_head[sender_id]->type,signo);
		fprintf(log_file,"I am %d and i have to reject sig_far_req from %d. Our interaction is %d sig=%d\n",node_id,sender_id,core_inter_head[sender_id]->type,signo);
		fflush(log_file);

		kill(info->si_pid, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	}
	//printf("IIIII I am %d in sig_far_req with sender %d and interaction = %d sig=%d\n",node_id,sender_id,core_inter_head[sender_id]->type,signo);
	
	//sem_getvalue(&node_sem[node_id],&i);
	//fprintf(log_file, "Sem value =  %d\n",i);
	//fflush(log_file);
	if (fd_r != -1) close(fd_r);
	free(fifo_name);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_FAR_REQ_handler with sender=%d and signo=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,signo,state);
	fflush(log_file);

	signals_enable();
}

void sig_APPOINT_WORK_handler(int signo, siginfo_t *info, void *context)
{
	int fd_r, sender_id, i, valid=0; 
	char *fifo_name;
//	inter_list *tmp_inter_list; 
	
	signals_disable();
	
	sender_id = get_id_from_pid(info->si_pid);
	fifo_name = get_pipe_name(node_id);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_APPOINT_WORK_handler with sender=%d and state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	
	
	if (core_inter_head[sender_id] != NULL) {
	//else {
		printf("I am %d and i in am doing smth else with my agent %d in sig_APPOINT_WORK_handler interaction is %d\n",node_id,sender_id,core_inter_head[sender_id]->type);
		
		/*tmp_inter_list = core_inter_head[sender_id];
		while (tmp_inter_list != NULL) {
			core_inter_head[sender_id] = core_inter_head[sender_id]->next;
			if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;

			fprintf(log_file, "I dismissed type = %d of sender = %d\n",tmp_inter_list->type,sender_id);
			fflush(log_file);

			free(tmp_inter_list);
			tmp_inter_list = core_inter_head[sender_id];
		}*/
	}

	sem_getvalue(&node_sem[node_id],&i);
	fprintf(log_file, "In 15 Trying to acquire semaphore. Sem value =  %d\n",i);
	fflush(log_file);
	kill(info->si_pid, SIG_ACK);		
	sem_wait(&node_sem[node_id]);
	sem_getvalue(&node_sem[node_id],&i);
	fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
	fflush(log_file);
	my_stats.msg_count++;
	my_stats.distance += distance(node_id,sender_id);	
	fd_r = open(fifo_name, O_RDONLY);

	if (fd_r == -1) {
		perror("opening reading pipe"); 
		signals_enable();		
		return;
	} else {
		fprintf(log_file, "I went through open\n");
		fflush(log_file);
	}	
	
	read(fd_r, &valid, sizeof(int));

	fprintf(log_file, "Validity of message = %d\n",valid);
	fflush(log_file);

	if (valid == 1) {
		if (my_agent == -1) read(fd_r, &my_agent, sizeof(int));// || my_agent != sender_id
		else if (my_agent != sender_id) {
			printf("I am %d and i do in appoint work switch of agent with upper_work_bound = %ld\n",node_id,upper_work_bound);
			fprintf(log_file,"I do in appoint work switch of agent with upper_work_bound = %ld\n",upper_work_bound);
			fflush(log_file);

			//if (upper_work_bound > 0) {//case where my former agent has not collected my remaining work time
			if (state == WORKING_NODE || state == WORKING_NODE_IDLE_INIT) {		
				if (core_inter_head[my_agent] == NULL){
					core_inter_head[my_agent] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[my_agent] = core_inter_head[my_agent];
				} else {
					core_inter_tail[my_agent]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[my_agent] = core_inter_tail[my_agent]->next;
				}

				core_inter_tail[my_agent]->type = REP_CHK_REM_TIME;
				core_inter_tail[my_agent]->data.work_time = upper_work_bound - time_worked;
				core_inter_tail[my_agent]->next = NULL;
			}
			read(fd_r, &my_agent, sizeof(int));
		}	
		//else if (state != WORKING_NODE) printf("I have been initialied but i am not working\n");

		read(fd_r, &upper_work_bound, sizeof(long int));
		time_worked=0;
		////sem_post(&node_sem[sender_id]);		
		printf("I am node %d with agent %d I am going to start working for %ld\n",node_id,my_agent,upper_work_bound);

		if (upper_work_bound > 0) {
			if (state == IDLE_INIT_MAN) state = WORKING_NODE_IDLE_INIT;
			else if (state == INIT_MANAGER || state == INIT_MAN_CHK_OFFERS || state == INIT_MANAGER_SEND_OFFERS) {
				//state = IDLE_INIT_MAN_WORK_PENDING;
				pending_state = WORKING_NODE;
				printf("I am node %d and sou milaw gia mallia\n",node_id);
			} else if (state != WORKING_NODE_IDLE_INIT) state = WORKING_NODE;
		} else {
			kill(info->si_pid, SIG_FINISH);
			my_stats.msg_count++;
			my_stats.distance += distance(node_id,sender_id);
		}
	}

	//sem_post(&node_sem[node_id]);
	sem_getvalue(&node_sem_out[sender_id],&i);
	fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
	fflush(log_file);
	if (sem_post(&node_sem_out[sender_id]) == -1){
		printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
		perror("sem_post");
	}
	sem_getvalue(&node_sem_out[sender_id],&i);
	fprintf(log_file, "Sem value = %d\n",i);
	fflush(log_file);
	//} else printf("I am %d and i in am doing smth else with my agent %d in sig_APPOINT_WORK_handler interaction is %d\n",node_id,sender_id,core_inter_head[sender_id]->type);

	close(fd_r);
	free(fifo_name);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_APPOINT_WORK_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

void sig_CHECK_REM_TIME_handler(int signo, siginfo_t *info, void *context)
{
	int fd_r=-1, sender_id, tmp_rem_time, i; 
	char *fifo_name=NULL;
	int time_per_node, time_left, time_to_work, one_core;
	core_list *tmp_cores_list;
	inter_list *tmp_inter_list;		
	float rem_workld;
	//inter_list tmp_inter_list;
	
	signals_disable();
	sender_id = get_id_from_pid(info->si_pid);
	fifo_name = get_pipe_name(node_id);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_CHECK_REM_TIME_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	
	
	if (core_inter_head[sender_id] == NULL && sender_id == my_agent){ //edw tha mpei otan exw allaksei agent
		fprintf(log_file, "I sig_CHECK_REM_TIME_handler a\n");
		fflush(log_file);	
				
		if (core_inter_head[sender_id] == NULL){
			core_inter_head[sender_id] = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[sender_id] = core_inter_head[sender_id];
		} else {
			core_inter_tail[sender_id]->next = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[sender_id] = core_inter_tail[sender_id]->next;
		}

		//fprintf(log_file, "I sig_CHECK_REM_TIME_handler a1\n");
		//fflush(log_file);	

		core_inter_tail[sender_id]->type = REP_CHK_REM_TIME;
		core_inter_tail[sender_id]->data.work_time = upper_work_bound - time_worked;
		if (core_inter_tail[sender_id]->data.work_time < 0) core_inter_tail[sender_id]->data.work_time = 0;
		core_inter_tail[sender_id]->next = NULL;		

		//fprintf(log_file, "I sig_CHECK_REM_TIME_handler a2\n");
		//fflush(log_file);	
		upper_work_bound = 0;
		if (state == WORKING_NODE) state = IDLE_CORE;
		kill(info->si_pid, SIG_CHECK_REM_TIME);
		//fprintf(log_file, "I sig_CHECK_REM_TIME_handler a3\n");
		//fflush(log_file);	
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		//fprintf(log_file, "I sig_CHECK_REM_TIME_handler a4\n");
		//fflush(log_file);	
	} else if (core_inter_head[sender_id] == NULL && sender_id != my_agent) {
		//I hope that this is where i end up when my former agent asks me for my remaining work but my original upper_work_bound was 0
		fprintf(log_file, "I sig_CHECK_REM_TIME_handler c\n");
		fflush(log_file);	
				
		if (core_inter_head[sender_id] == NULL){
			core_inter_head[sender_id] = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[sender_id] = core_inter_head[sender_id];
		} else {
			core_inter_tail[sender_id]->next = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[sender_id] = core_inter_tail[sender_id]->next;
		}

		core_inter_tail[sender_id]->type = REP_CHK_REM_TIME;
		core_inter_tail[sender_id]->data.work_time = 0;
		core_inter_tail[sender_id]->next = NULL;		

		kill(info->si_pid, SIG_CHECK_REM_TIME);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	} else if (core_inter_head[sender_id]->type == REP_CHK_REM_TIME) {
		fprintf(log_file, "I sig_CHECK_REM_TIME_handler b\n");
		fflush(log_file);
		kill(info->si_pid, SIG_CHECK_REM_TIME);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);		 
	} else if (core_inter_head[sender_id]->type == APPOINT_WORK_NODE_PENDING || core_inter_head[sender_id]->type == REMOVED_NODE_REM_TIME) {//CHK_REM_TIME
		//printf("I an in here node_id=%d\n",node_id);
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "In 16 Trying to acquire semaphore. Sem value =  %d\n",i);
		fflush(log_file);
		kill(info->si_pid, SIG_ACK);		
		sem_wait(&node_sem[node_id]);
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
		fflush(log_file);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);	
		fd_r = open(fifo_name, O_RDONLY);
		//printf("I come through here\n");
	
		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		} /*else {
			fprintf(log_file, "I went through open\n");
			fflush(log_file);
		}	*/
			
		if (app_state != RESIZING) {
			printf("I am %d in check rem and app_state = %d app_id = %d\n",node_id,app_state,my_app.id);	
			fprintf(log_file,"I am in check rem and app_state = %d app_id = %d\n",app_state,my_app.id);
			fflush(log_file);
		}

		read(fd_r, &tmp_rem_time, sizeof(int));
		fprintf(app_log_file,"tmp_rem_time = %d sender_id = %d\n",tmp_rem_time,sender_id);
		fflush(app_log_file);
		////sem_post(&node_sem[sender_id]);
		my_stats.app_turnaround -= tmp_rem_time; 
		sum_rem_time += tmp_rem_time;
	
		if (++chk_rem_count == chk_rem_num){
			nodes_ended_cnt = 0;			
			rem_workld = sum_rem_time * old_Speedup;
			time_to_work = (int) roundf(rem_workld / my_Speedup);
			if (my_cores_count > 1) {
				time_per_node = time_to_work / (my_cores_count-1);
				time_left = time_to_work % (my_cores_count-1);

				printf("i am %d in check_rem. sum_rem_time =%d rem_workld = %0.2f time_to_work =%d time_per_node = %d time_left = %d, my_cores_count = %d\n",
					node_id,sum_rem_time,rem_workld, time_to_work,time_per_node,time_left,my_cores_count);
				fprintf(app_log_file,"i am in check_rem. sum_rem_time =%d rem_workld = %0.2f time_to_work =%d time_per_node = %d time_left = %d, my_cores_count = %d\n",
					sum_rem_time,rem_workld, time_to_work,time_per_node,time_left,my_cores_count);
				fflush(app_log_file);

				if (rem_workld > 0.0) {
					if (time_to_work == 0) time_left++;
					tmp_cores_list = my_cores->next;
					while (tmp_cores_list != NULL) {
						one_core = tmp_cores_list->core_id;
						
						if (core_inter_head[one_core]->type != APPOINT_WORK_NODE_PENDING && core_inter_head[one_core]->type != INIT_WORK_NODE_PENDING) {
							printf("Wrong fucking interaction core = %d interaction = %d\n",one_core,core_inter_head[one_core]->type);
							tmp_inter_list = core_inter_head[one_core];
							while (tmp_inter_list != NULL) {
								if (tmp_inter_list->type == APPOINT_WORK_NODE_PENDING || tmp_inter_list->type == INIT_WORK_NODE_PENDING) break;
								tmp_inter_list = tmp_inter_list->next;
							}

							if (tmp_inter_list == NULL) printf("I am %d and mas ksekwliase o %d\n",node_id,one_core);
						} else tmp_inter_list = core_inter_head[one_core];
			
						if (tmp_inter_list->type == APPOINT_WORK_NODE_PENDING) tmp_inter_list->type = APPOINT_WORK_NODE;
						else tmp_inter_list->type = INIT_WORK_NODE;

						tmp_inter_list->data.work_time = time_per_node;
						if (time_left > 0) {
							tmp_inter_list->data.work_time++;
							time_left--;
						}
				
						if (tmp_inter_list->data.work_time > 0) {
							my_stats.app_turnaround += tmp_inter_list->data.work_time;
						//printf("sending core with id=%d\n",one_core);
							if (core_inter_head[one_core]->type == APPOINT_WORK_NODE || core_inter_head[one_core]->type == INIT_WORK_NODE) {//||next == NULL
								kill(pid_num[one_core], SIG_APPOINT_WORK);
								my_stats.msg_count++;
								my_stats.distance += distance(node_id,one_core);
							} else printf("I am %d doing smth else with my working node %d in re appointing inter1 = %d inter2 = %d\n",node_id,one_core,core_inter_head[one_core]->type
								,core_inter_head[one_core]->next->type);
						} else nodes_ended_cnt++;
						/*core_inter_head[one_core]->data.work_time = time_per_node;
						if (time_left > 0) {
							core_inter_head[one_core]->data.work_time++;
							time_left--;
						}
				
						if (core_inter_head[one_core]->data.work_time > 0) {
							my_stats.app_turnaround += core_inter_head[one_core]->data.work_time;
						//printf("sending core with id=%d\n",one_core);
							if (core_inter_head[one_core]->next == NULL) {
								kill(pid_num[one_core], SIG_APPOINT_WORK);
								my_stats.msg_count++;
								my_stats.distance += distance(node_id,one_core);
							} else printf("I am %d doing smth else with my working node %d in re appointing inter1 = %d inter2 = %d\n",node_id,one_core,core_inter_head[one_core]->type
								,core_inter_head[one_core]->next->type);
						} else nodes_ended_cnt++;*/
						
						tmp_cores_list = tmp_cores_list->next;
					}
					app_state = RUNNING;
				} else {
					//app_terminated = 1;
					app_state = APP_TERMINATED;					
					//state = AGENT_ENDING;
				}
			} else {
				printf("I am %d and i am so stupid that i offered my only working core\n",node_id);
				fprintf(app_log_file,"I have only one core state = %d and pending_state = %d\n",state,pending_state);
				fflush(app_log_file);				

				if (selfopt_time_rem == -1 && state == IDLE_AGENT) state = AGENT_REWIND_FILE;
				else if (pending_state == IDLE_AGENT) pending_state = AGENT_REWIND_FILE;
			}
			/*if (app_terminated) {
				for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next)
					if (core_inter_head[tmp_cores_list->core_id]->type == APPOINT_WORK_NODE_PENDING || core_inter_head[tmp_cores_list->core_id]->type == INIT_WORK_NODE_PENDING){
						tmp_inter_list = core_inter_head[tmp_cores_list->core_id];
						core_inter_head[tmp_cores_list->core_id] = core_inter_head[tmp_cores_list->core_id]->next;
						free(tmp_inter_list);
						printf("I am %d and i pre freed node %d\n",node_id,tmp_cores_list->core_id);
					} else printf("I am %d and i tried to pre free but my interaction with my working node %d is %d\n",
							node_id,tmp_cores_list->core_id,core_inter_head[tmp_cores_list->core_id]->type);
			} else {*/
			if (app_state != APP_TERMINATED) {//!app_terminated
				if (pending_state == INIT_MANAGER) {
						state = INIT_MANAGER;
						pending_state = IDLE_AGENT;
				} else {
					if (selfopt_interval > 0){
						its.it_value.tv_nsec = selfopt_interval * MS;
						selfopt_time_rem = selfopt_interval;

						if (timer_settime(timerid, 0, &its, NULL) == -1) {
							perror("timer_settime error8");
							printf("I am %d timer_settime error8 selfopt_interval = %d\n",node_id,selfopt_interval);
						}	
					} else selfopt_time_rem = -1;
				} 
			}
		}			
		
		if (core_inter_head[sender_id]->type == REMOVED_NODE_REM_TIME) {
			fprintf(log_file,"I am with sender_id = %d and a\n",sender_id);
			fflush(log_file);			
			tmp_inter_list = core_inter_head[sender_id];
			core_inter_head[sender_id] = core_inter_head[sender_id]->next;
			if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
			else send_next_signal(core_inter_head[sender_id], sender_id);		
			free(tmp_inter_list);	
			fprintf(log_file,"I am with sender_id = %d and b\n",sender_id);
			fflush(log_file);
		}
		//printf("I am node %d my agent is %d and I am going to start working\n",node_id,my_agent);
	//sem_post(&node_sem[node_id]);
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
		fflush(log_file);
		if (sem_post(&node_sem_out[sender_id]) == -1){
			printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
			perror("sem_post");
		}
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "Sem value = %d\n",i);
		fflush(log_file);	
	} else if (sender_id == my_agent){
		printf("I am %d and i am doing smth else with my agent %d in sig_CHECK_REM_TIME_handler interaction=%d\n",node_id,sender_id,core_inter_head[sender_id]->type);
		fprintf(log_file,"I am doing smth else with my agent %d in sig_CHECK_REM_TIME_handler interaction=%d\n",sender_id,core_inter_head[sender_id]->type);
		fflush(log_file);

		core_inter_tail[sender_id]->next = (inter_list *) malloc(sizeof(inter_list));
		core_inter_tail[sender_id] = core_inter_tail[sender_id]->next;

		core_inter_tail[sender_id]->type = REP_CHK_REM_TIME;
		core_inter_tail[sender_id]->data.work_time = upper_work_bound - time_worked;
		if (core_inter_tail[sender_id]->data.work_time < 0) core_inter_tail[sender_id]->data.work_time = 0;
		core_inter_tail[sender_id]->next = NULL;		

		//fprintf(log_file, "I sig_CHECK_REM_TIME_handler a2\n");
		//fflush(log_file);	
		upper_work_bound = 0;
		if (state == WORKING_NODE) state = IDLE_CORE;
	} else {
		printf("I am %d and i am doing smth else with random node %d in sig_CHECK_REM_TIME_handler interaction=%d\n",node_id,sender_id,core_inter_head[sender_id]->type);
		fprintf(log_file,"I am doing smth else with random node %d in sig_CHECK_REM_TIME_handler interaction=%d\n",sender_id,core_inter_head[sender_id]->type);
		fflush(log_file);

		kill(info->si_pid, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	}
	//fprintf(log_file, "I sig_CHECK_REM_TIME_handler a5\n");
	//fflush(log_file);
	if (fd_r != -1) close(fd_r);
	//fprintf(log_file, "I sig_CHECK_REM_TIME_handler a6\n");
	//fflush(log_file);
	free(fifo_name);
	//fprintf(log_file, "I sig_CHECK_REM_TIME_handler a7\n");
	//fflush(log_file);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_CHECK_REM_TIME_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

void sig_FINISH_handler(int signo, siginfo_t *info, void *context)
{
	int fd_r, i, is_sender_idag, sender_id, new_agent_id; 
	int *nodes_to_process, nodes_cnt=0, processed_cnt=0;
	int *tmp_list, tmp_cnt, tmp_idag;	
	char *fifo_name;
	core_list *tmp_cores_list;//tmp_cores, 
	DDS_list *tmp_DDS,*prev_DDS;
	inter_list *tmp_inter_list;	

	sender_id = get_id_from_pid(info->si_pid);
	fifo_name = get_pipe_name(node_id);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_FINISH_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	
	
	//if i am an idag, sig_finish should be proceed instantly, so i am dropping. 
	//On the other hand if an common node doesn't set its agent to -1 its not a big deal. (Hopefully)	
	if (core_inter_head[sender_id] != NULL && my_idag == -1) { 
		//else {
		printf("I am %d in sig finish with sender %d and i don't know what to do with interaction %d\n",node_id,sender_id,core_inter_head[sender_id]->type);		
		
		if (my_idag == -1) {
			is_sender_idag = 0;
			for (i=0; i<num_idags; i++)
				if (idag_id_arr[i] == sender_id){
					is_sender_idag = 1;
					break;
				}
		
			if (!is_sender_idag) {
				tmp_inter_list = core_inter_head[sender_id];
				while (tmp_inter_list != NULL) {
					core_inter_head[sender_id] = core_inter_head[sender_id]->next;
					if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;

					fprintf(log_file, "I dismissed type = %d of sender = %d\n",tmp_inter_list->type,sender_id);
					fflush(log_file);

					free(tmp_inter_list);
					tmp_inter_list = core_inter_head[sender_id];
				}
			}
		}
	} 
		
	if (my_idag != -1){
		if (my_agent != -1 && my_agent == sender_id) my_agent = -1;//working node
		if (++nodes_ended_cnt == (my_cores_count - 1)) app_state = APP_TERMINATED;//app_terminated = 1;
	} else {
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "In 17 Trying to acquire semaphore. Sem value =  %d\n",i);
		fflush(log_file);
		kill(info->si_pid, SIG_ACK);		
		sem_wait(&node_sem[node_id]);
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
		fflush(log_file);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		fd_r = open(fifo_name, O_RDONLY);

		if (fd_r == -1) {
			perror("opening reading pipe"); 
			signals_enable();		
			return;
		} /*else {
			fprintf(log_file, "I went through open\n");
			fflush(log_file);
		}	*/

		is_sender_idag = 0;
		for (i=0; i<num_idags; i++)
			if (idag_id_arr[i] == sender_id){
				is_sender_idag = 1;
				break;
			}

		printf("I am %d in sig finish with is_sender_idag = %d\n",node_id,is_sender_idag);
		if (is_sender_idag == 0){
			read(fd_r, &nodes_cnt, sizeof(int));
			nodes_to_process = (int *) malloc(nodes_cnt * sizeof(int));
			for (i=0; i<nodes_cnt; i++)
				read(fd_r, &nodes_to_process[i], sizeof(int));

			////sem_post(&node_sem[sender_id]);
			tmp_list = (int *) malloc(nodes_cnt * sizeof(int));
			printf("I am node_id=%d Cores to be processed: ",node_id);				
			for (i=0; i<nodes_cnt; i++)
				printf(" %d",nodes_to_process[i]);
			printf("\n");

			printf("I am node_id=%d nodes_cnt = %d\n",node_id,nodes_cnt);
			while (processed_cnt < nodes_cnt){
				tmp_cnt = 0;
				tmp_idag = -1;

				for (i=0; i<nodes_cnt; i++){
					if (processed_cnt == nodes_cnt) break;
					if (nodes_to_process[i] == -1) continue;
					else {
						if (tmp_idag == -1) tmp_idag = idag_mask[nodes_to_process[i]];

						if (idag_mask[nodes_to_process[i]] != tmp_idag) continue;
						else {
							tmp_list[tmp_cnt++] = nodes_to_process[i];
							nodes_to_process[i] = -1;
							processed_cnt++;
						}
					}
				}

				if (tmp_idag != node_id){
					if (core_inter_head[tmp_idag] == NULL){
						core_inter_head[tmp_idag] = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[tmp_idag] = core_inter_head[tmp_idag];
					} else {
						printf("finish agent DDS not in high priority! Shouldn't I high prioritize? tmp = %d idag = %d\n",tmp_idag,node_id);
						core_inter_tail[tmp_idag]->next = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[tmp_idag] = core_inter_tail[tmp_idag]->next;
					}

					printf("node_id = %d In finish tmp_idag = %d tmp_cnt = %d sender_id = %d is_sender_idag=%d tmp_list[0]=%d\n",node_id,tmp_idag,tmp_cnt,sender_id,is_sender_idag,tmp_list[0]);
					core_inter_tail[tmp_idag]->type = REMOVE_APP;
					core_inter_tail[tmp_idag]->data.agent_ended = sender_id;
					/*core_inter_tail[tmp_idag]->data.app_cores = (int *)malloc((tmp_cnt+2)*sizeof(int));
					core_inter_tail[tmp_idag]->data.app_cores[0] = tmp_cnt;//+1;
					core_inter_tail[tmp_idag]->data.app_cores[tmp_cnt+1] = sender_id;

					for (j=1; j<= tmp_cnt; j++)
						core_inter_tail[tmp_idag]->data.app_cores[j] = tmp_list[j-1];
					*/
					core_inter_tail[tmp_idag]->next = NULL;
				
					//if (tmp_idag != 0) why???
					if (core_inter_head[tmp_idag]->next == NULL) {
						kill(pid_num[tmp_idag], SIG_FINISH);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,tmp_idag);
					}
				} else {
					printf("In FINISH same tmp_idag = %d tmp_cnt = %d nodes_cnt = %d sender_id = %d node_id = %d\n",tmp_idag,tmp_cnt,nodes_cnt,sender_id,node_id);
					new_agent_id = sender_id;
					/*for (i=0; i<tmp_cnt; i++){
						tmp_cores = my_cores;
	
						while (tmp_cores->core_id != tmp_list[i])//nodes_to_process[i]
							tmp_cores = tmp_cores->next;

						if (tmp_cores->offered_to == new_agent_id) tmp_cores->offered_to = -1;
					}

					DDS->num_of_cores += tmp_cnt;*/
					for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next) 
						if (tmp_cores_list->offered_to == new_agent_id) {
							tmp_cores_list->offered_to = -1;
							DDS->num_of_cores++;
						}

					prev_DDS = DDS;
					tmp_DDS = DDS->next;
					while (tmp_DDS != NULL && tmp_DDS->agent_id != new_agent_id){
						prev_DDS = tmp_DDS;
						tmp_DDS = tmp_DDS->next;
					}

					if (tmp_DDS != NULL){
						DDS_count--;
						prev_DDS->next = tmp_DDS->next;
						if (tmp_DDS->next == NULL) DDS_tail = prev_DDS;
						free(tmp_DDS);
					} else printf("Fuck agent that finished is not in my DDS\n");	

					printf("My removal of agent complete node_id = %d sender_id=%d\n",node_id,sender_id);
					printf("Number of agents in region = %d\n",DDS_count);	
					i=0;
					for (tmp_DDS = DDS; tmp_DDS != NULL; tmp_DDS = tmp_DDS->next){
						printf("Agent no %d is %d with %d cores\n",i,tmp_DDS->agent_id,tmp_DDS->num_of_cores);	
						i++;
					}
				}															
			}

			free(nodes_to_process);
			free(tmp_list);
		} else {
			/*read(fd_r, &nodes_cnt, sizeof(int));
			printf("I am %d and secondary node cnt by %d is %d\n",node_id,sender_id,nodes_cnt);

			nodes_to_process = (int *) malloc(nodes_cnt * sizeof(int));
			for (i=0; i<nodes_cnt; i++)
				read(fd_r, &nodes_to_process[i], sizeof(int));

			read(fd_r, &new_agent_id, sizeof(int));
			////sem_post(&node_sem[sender_id]);
			printf("Secondary Cores to be processed of %d by %d: ",new_agent_id,node_id);				
			for (i=0; i<nodes_cnt; i++)
				printf(" %d",nodes_to_process[i]);
			printf("\n");

			for (i=0; i<nodes_cnt; i++){
				tmp_cores = my_cores;

				while (tmp_cores->core_id != nodes_to_process[i])//nodes_to_process[i]
					tmp_cores = tmp_cores->next;

				if (tmp_cores->offered_to == new_agent_id) tmp_cores->offered_to = -1;
			}

			DDS->num_of_cores += nodes_cnt;
			*/
			read(fd_r, &new_agent_id, sizeof(int));
			printf("I am %d in Secondary sig_finish for %d\n",node_id,new_agent_id);

			for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next) 
				if (tmp_cores_list->offered_to == new_agent_id) {
					tmp_cores_list->offered_to = -1;
					DDS->num_of_cores++;
				}

			prev_DDS = DDS;
			tmp_DDS = DDS->next;
			while (tmp_DDS != NULL && tmp_DDS->agent_id != new_agent_id){
				prev_DDS = tmp_DDS;
				tmp_DDS = tmp_DDS->next;
			}

			if (tmp_DDS != NULL){
				DDS_count--;
				prev_DDS->next = tmp_DDS->next;
				if (tmp_DDS->next == NULL) DDS_tail = prev_DDS;
				free(tmp_DDS);
			} else printf("Fuck agent that finished is not in my DDS\n");
			//free(nodes_to_process);
			printf("Secondary removal of agent complete node_id =%d sender_id=%d\n",node_id,sender_id);
			printf("Number of agents in region = %d\n",DDS_count);	
			i=0;
			for (tmp_DDS = DDS; tmp_DDS != NULL; tmp_DDS = tmp_DDS->next){
				printf("Agent no %d is %d with %d cores\n",i,tmp_DDS->agent_id,tmp_DDS->num_of_cores);	
				i++;
			}	
		}

		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
		fflush(log_file);
		if (sem_post(&node_sem_out[sender_id]) == -1){
			printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
			perror("sem_post");
		}
		sem_getvalue(&node_sem_out[sender_id],&i);
		fprintf(log_file, "Sem value = %d\n",i);
		fflush(log_file);

		close(fd_r);
	}
		//sem_post(&node_sem[node_id]);	
	//} else printf("I am %d in sig finish with sender %d and i don't know what to do with interaction %d\n",node_id,sender_id,core_inter_head[sender_id]->type);

	//free(fifo_name);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_FINISH_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

void sig_REJECT_handler(int signo, siginfo_t *info, void *context)
{
	int sender_id, agent_id, i, j;//, crit_app_id;//fd_r,  
	//char *fifo_name;
	inter_list *tmp_inter_list;
	core_list *tmp_cores_list;
	target_list *tmp_target_list;	
	offer_list *tmp_offer_list;	

	signals_disable();
	//inter_list tmp_inter_list;
	
	sender_id = get_id_from_pid(info->si_pid);
	//fifo_name = get_pipe_name(node_id);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_REJECT_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	
	
	if (core_inter_head[sender_id] != NULL){
		fprintf(log_file, "[%d:%d:%d]: Interaction with sender=%d is %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,core_inter_head[sender_id]->type);
		fflush(log_file);
	}

	if (core_inter_head[sender_id] == NULL){ //edw tha mpei otan exw allaksei agent
		printf("I am %d in sig_reject and i have null interaction with sender %d\n",node_id,sender_id);
	} else if (core_inter_head[sender_id]->type == IDAG_REQ_DDS || core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS || core_inter_head[sender_id]->type == FAR_REQ_IDAG_REQ_DDS
			|| core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS_PENDING || core_inter_head[sender_id]->type == IDAG_REQ_DDS_PENDING) {
		if (core_inter_head[sender_id]->type == IDAG_REQ_DDS || core_inter_head[sender_id]->type == IDAG_REQ_DDS_PENDING) {
			init_DDS_replies++;
			fprintf(log_file, "One init_req_dds has been rejected by %d!\n",sender_id);
			fflush(log_file);
		} else if (core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS || core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS_PENDING) {
			selfopt_DDS_replies++;
			fprintf(log_file, "One selfopt_req_dds has been rejected by %d!\n",sender_id);
			fflush(log_file);
		}
			//printf("selfopt_DDS_idags = %d, selfopt_DDS_replies = %d\n",selfopt_DDS_idags,selfopt_DDS_replies);		
				
		if ((core_inter_head[sender_id]->type == IDAG_REQ_DDS || core_inter_head[sender_id]->type == IDAG_REQ_DDS_PENDING) 
			&& init_DDS_replies == init_DDS_idags && init_idags_areas_replies == INIT_AREAS_NUM)//INIT_AREAS_NUM
			for (tmp_target_list = init_targets_head; tmp_target_list != NULL; tmp_target_list = tmp_target_list->next){
				agent_id = tmp_target_list->target;

				for (tmp_inter_list = core_inter_head[agent_id]; tmp_inter_list != NULL; tmp_inter_list=tmp_inter_list->next)
					if (tmp_inter_list->type == FAR_INIT_REQ || tmp_inter_list->type == FAR_REQ_MAN) break; 

				if (tmp_inter_list == NULL) {
					if (core_inter_head[agent_id] == NULL){
						core_inter_head[agent_id] = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[agent_id] = core_inter_head[agent_id];
					} else {
						core_inter_tail[agent_id]->next = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[agent_id] = core_inter_tail[agent_id]->next;
					}
	
					core_inter_tail[agent_id]->type = AGENT_REQ_CORES_PENDING;
					core_inter_tail[agent_id]->data.reg_arr.num_of_regions = tmp_target_list->num_of_regions;
					core_inter_tail[agent_id]->data.reg_arr.region_arr = (region *) malloc(tmp_target_list->num_of_regions * sizeof(region));
		
					if (tmp_target_list->target == node_id) printf("Why is this here node_id=%d\n",node_id);
					fprintf(log_file,"Init req target = %d, num_of_regions = %d. Αreas:",tmp_target_list->target,tmp_target_list->num_of_regions);
					for (i=0; i<tmp_target_list->num_of_regions; i++) {
						core_inter_tail[agent_id]->data.reg_arr.region_arr[i] = tmp_target_list->region_arr[i];
						fprintf(log_file," (%d,%d),",tmp_target_list->region_arr[i].C,tmp_target_list->region_arr[i].r);
					}
					fprintf(log_file,"\n");
					fflush(log_file);
					core_inter_tail[agent_id]->next = NULL;

					//kill(pid_num[agent_id], SIG_REQ_CORES);
					if (core_inter_head[agent_id]->next == NULL) {
						kill(pid_num[agent_id], SIG_REQ_CORES);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,agent_id);
					} else if (agent_id == sender_id && core_inter_head[agent_id]->next->type == AGENT_REQ_CORES_PENDING) { //den exei fugei apo to free alla einai wra na stalei
						kill(pid_num[agent_id], SIG_REQ_CORES);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,agent_id);
					} else printf("This init fucker is not NULL interaction=%d interaction2=%d\n",core_inter_head[agent_id]->type,core_inter_head[agent_id]->next->type);
				} else 
					printf("I am %d and i did not send local requests to %d because he is my far manager\n",node_id,agent_id);
			}		
		else if ((core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS || core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS_PENDING) && selfopt_DDS_replies == selfopt_DDS_idags)
			for (tmp_target_list = selfopt_targets_head; tmp_target_list != NULL; tmp_target_list = tmp_target_list->next){
				agent_id = tmp_target_list->target;
				if (core_inter_head[agent_id] == NULL){
					core_inter_head[agent_id] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[agent_id] = core_inter_head[agent_id];
				} else {
					core_inter_tail[agent_id]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[agent_id] = core_inter_tail[agent_id]->next;
				}

				core_inter_tail[agent_id]->type = SELFOPT_REQ_CORES_PENDING;
				core_inter_tail[agent_id]->data.reg_arr.num_of_regions = tmp_target_list->num_of_regions;
				core_inter_tail[agent_id]->data.reg_arr.region_arr = (region *) malloc(tmp_target_list->num_of_regions * sizeof(region));
				if (tmp_target_list->target == node_id) printf("selfopt Why is this here node_id=%d\n",node_id);
				fprintf(log_file,"Selfopt req target = %d, num_of_regions = %d. Αreas:",tmp_target_list->target,tmp_target_list->num_of_regions);
				for (i=0; i<tmp_target_list->num_of_regions; i++) {
					core_inter_tail[agent_id]->data.reg_arr.region_arr[i] = tmp_target_list->region_arr[i];
					fprintf(log_file," (%d,%d),",tmp_target_list->region_arr[i].C,tmp_target_list->region_arr[i].r);
				}
				fprintf(log_file,"\n");
				fflush(log_file);				
				core_inter_tail[agent_id]->next = NULL;

				if (core_inter_head[agent_id]->next == NULL) {
					kill(pid_num[agent_id], SIG_REQ_CORES);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,agent_id);
				} else if (agent_id == sender_id && core_inter_head[agent_id]->next->type == SELFOPT_REQ_CORES_PENDING) {
					kill(pid_num[agent_id], SIG_REQ_CORES);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,agent_id);
				} else printf("This selfopt fucker is not NULL interaction=%d interaction2=%d\n",core_inter_head[agent_id]->type,core_inter_head[agent_id]->next->type);
			}

		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else if (core_inter_head[sender_id]->type != AGENT_REQ_CORES_PENDING && core_inter_head[sender_id]->type != SELFOPT_REQ_CORES_PENDING)//far_req_max_man != sender_id && 
			send_next_signal(core_inter_head[sender_id], sender_id);
		free(tmp_inter_list);
	} else if (core_inter_head[sender_id]->type == FAR_INIT_IDAG_REQ_DDS || core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS || core_inter_head[sender_id]->type == FAR_REQ_CORES
			|| core_inter_head[sender_id]->type == AGENT_REQ_CORES_PENDING || core_inter_head[sender_id]->type == FAR_REQ_IDAG_REQ_DDS_PENDING 
			|| core_inter_head[sender_id]->type == IDAG_FIND_IDAGS_PENDING || core_inter_head[sender_id]->type == SELFOPT_IDAG_FIND_IDAGS_PENDING 
			|| core_inter_head[sender_id]->type == SELFOPT_REQ_CORES_PENDING || core_inter_head[sender_id]->type == FAR_REQ_CORES_PENDING 
			|| core_inter_head[sender_id]->type == REP_FAR_INIT_REQ) {
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);
		free(tmp_inter_list);
	}	else if (core_inter_head[sender_id]->type == FAR_REQ_OFFER) {//FAR_REQ_OFFER_SENT
		//fprintf(log_file, "a\n");
		//fflush(log_file);
		tmp_cores_list = my_cores;
		while (tmp_cores_list != NULL){
			if (tmp_cores_list->offered_to == sender_id) tmp_cores_list->offered_to = -1;
			tmp_cores_list = tmp_cores_list->next;
		}
		
		//fprintf(log_file, "b\n");
		//fflush(log_file);
		
		tmp_offer_list = far_man_offers;
		far_man_offers = far_man_offers->next;
		free(tmp_offer_list);

		//fprintf(log_file, "c\n");
		//fflush(log_file);
		
		//if (far_man_offers == NULL) {
		//	fprintf(log_file, "karioles\n");
		//	fflush(log_file);
		//}

		while (far_man_offers != NULL){
			printf("I am node %d in reject and far offer answer for node %d is 0\n",node_id,far_man_offers->sender);

			*far_man_offers->answer = 0;
			if (core_inter_head[far_man_offers->sender]->type == REP_AGENT_OFFER_PENDING){
				kill(pid_num[far_man_offers->sender],SIG_REP_OFFERS);
				core_inter_head[far_man_offers->sender]->type = REP_AGENT_OFFER_SENT;
			} else printf("I am %d and Apparently lists are poutana ola\n",node_id);
				
			tmp_offer_list = far_man_offers;
			far_man_offers = far_man_offers->next;
			free(tmp_offer_list);
		}

		far_req_app.A=-1.0;
		far_req_app.var=-1.0; 
		far_req_app.num_of_cores=-1;
		far_req_or_sender = -1;

		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);
		free(tmp_inter_list);

	}	else if (core_inter_head[sender_id]->type == REP_IDAG_FIND_IDAGS) {	
		free(core_inter_head[sender_id]->data.idags_in_reg);		
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);
		free(tmp_inter_list);
	}	else if (core_inter_head[sender_id]->type == REP_IDAG_REQ_DDS) {
		free(core_inter_head[sender_id]->data.agents_in_reg);		
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);
		free(tmp_inter_list);
	}	else if (core_inter_head[sender_id]->type == FAR_REQ_MAN_APPOINT) {
		agent_id = core_inter_head[sender_id]->data.far_req.orig_sender;			
	
		if (core_inter_head[agent_id] == NULL){
			core_inter_head[agent_id] = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[agent_id] = core_inter_head[agent_id];
		} else {
			core_inter_tail[agent_id]->next = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[agent_id] = core_inter_tail[agent_id]->next;
		}

		core_inter_tail[agent_id]->type = ABORT_FAR_MAN;
		core_inter_tail[agent_id]->data.far_req_man = sender_id;
		core_inter_tail[agent_id]->next = NULL;

		if (core_inter_head[agent_id]->next == NULL) {
			kill(pid_num[agent_id], SIG_REMOVE_FAR_MAN);
			my_stats.msg_count++;
			my_stats.distance += distance(node_id,agent_id);
		}			

		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);
		free(tmp_inter_list);	
	} else if (core_inter_head[sender_id]->type == REP_AGENT_REQ_CORES) {
		
		for (j=0; j<core_inter_head[sender_id]->data.off_arr.num_of_offers; j++) 			
			if (my_cores != NULL) 
				for (i=0; i<core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores; i++)
					for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next)
						if (tmp_cores_list->core_id == core_inter_head[sender_id]->data.off_arr.offer_arr[j].offered_cores[i]) {
							tmp_cores_list->offered_to = -1;
							break;
						}

		free(core_inter_head[sender_id]->data.off_arr.offer_arr);
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);
		free(tmp_inter_list);
	} else if (core_inter_head[sender_id]->type == INIT_APP) {
		printf("I am 0 and %d rejected my init_app req\n",sender_id);
		fprintf(log_file,"I am 0 and %d rejected my init_app req\n",sender_id);
		fflush(log_file);

		if (init_pending_head == NULL){
			init_pending_head = (inter_list *) malloc(sizeof(inter_list));
			init_pending_tail = init_pending_head;
		} else {
			init_pending_tail->next = (inter_list *) malloc(sizeof(inter_list));
			init_pending_tail = init_pending_tail->next;
		} 

		init_pending_tail->type = INIT_APP;
		init_pending_tail->data.new_app = core_inter_head[sender_id]->data.new_app;
		init_pending_tail->data.new_app.num_of_cores = sender_id;
		init_pending_tail->next = NULL;
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);
		free(tmp_inter_list);
	} else if (core_inter_head[sender_id]->type == REP_CHK_REM_TIME) {
		printf("I am %d and %d rejected my REP_CHK_REM_TIME with work_time = %d\n",node_id,sender_id,core_inter_head[sender_id]->data.work_time);
		fprintf(log_file,"I am and %d rejected my REP_CHK_REM_TIME with work_time = %d\n",sender_id,core_inter_head[sender_id]->data.work_time);
		fflush(log_file);

		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);
		free(tmp_inter_list);
	} else printf("I am %d in sig_reject and i have interaction with sender %d interaction = %d\n",node_id,sender_id,core_inter_head[sender_id]->type);

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_REJECT_handler with sender = %d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	
	//fprintf(log_file, "[%ld:%ld:%ld:%ld]: my idle agent is %d and my pid is %d\n",tv.tv_sec/3600,tv.tv_sec%3600/60,tv.tv_sec%3600%60,(long int) tv.tv_usec,my_idag,getpid());	
	//close(fd_r);
	
	//free(fifo_name);
	signals_enable(); 
	/*signals_disable();
	state = TERMINATED;
	signals_enable();*/
}

void sig_REMOVE_FAR_MAN_handler(int signo, siginfo_t *info, void *context)
{
	int fd_r, sender_id, i, my_agent; 
	char *fifo_name;
	inter_list *tmp_inter_list, *tmp_inter_prev; 
	
	signals_disable();
	
	sender_id = get_id_from_pid(info->si_pid);
	fifo_name = get_pipe_name(node_id);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_REMOVE_FAR_MAN_handler with sender=%d and state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	
	
	if (core_inter_head[sender_id] != NULL) {
		printf("I am %d and i in am doing smth else with my agent %d in sig_REMOVE_FAR_MAN_handler interaction is %d\n",node_id,sender_id,core_inter_head[sender_id]->type);
		
		/*tmp_inter_list = core_inter_head[sender_id];
		while (tmp_inter_list != NULL) {
			core_inter_head[sender_id] = core_inter_head[sender_id]->next;
			if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;

			fprintf(log_file, "I dismissed type = %d of sender = %d\n",tmp_inter_list->type,sender_id);
			fflush(log_file);

			free(tmp_inter_list);
			tmp_inter_list = core_inter_head[sender_id];
		}*/
	}

	sem_getvalue(&node_sem[node_id],&i);
	fprintf(log_file, "In REMOVE_FAR_MAN Trying to acquire semaphore. Sem value =  %d\n",i);
	fflush(log_file);
	kill(info->si_pid, SIG_ACK);		
	sem_wait(&node_sem[node_id]);
	sem_getvalue(&node_sem[node_id],&i);
	fprintf(log_file, "Semaphore acquired successfully. Sem value = %d\n",i);
	fflush(log_file);
	my_stats.msg_count++;
	my_stats.distance += distance(node_id,sender_id);	
	fd_r = open(fifo_name, O_RDONLY);

	if (fd_r == -1) {
		perror("opening reading pipe"); 
		signals_enable();		
		return;
	} /*else {
		fprintf(log_file, "I went through open\n");
		fflush(log_file);
	}*/	
	
	read(fd_r, &my_agent, sizeof(int));// || my_agent != sender_id
	tmp_inter_prev = NULL;
	tmp_inter_list = core_inter_head[my_agent];
	while (tmp_inter_list != NULL) {
		//the far manager being clear had to be the one tha has been appointed by sender
		if (tmp_inter_list->type == FAR_REQ_MAN && tmp_inter_list->data.far_req_man == sender_id) {
			fprintf(log_file, "I dismissed type = %d of node %d sender = %d\n",tmp_inter_list->type,my_agent,sender_id);
			fflush(log_file);

			if (tmp_inter_prev == NULL) {
				core_inter_head[my_agent] = core_inter_head[my_agent]->next;
				if (core_inter_head[my_agent] == NULL) core_inter_tail[my_agent] = NULL;
				else send_next_signal(core_inter_head[my_agent], my_agent);
			} else {
				tmp_inter_prev->next = tmp_inter_list->next;					
				if (tmp_inter_prev->next == NULL) core_inter_tail[my_agent] = tmp_inter_prev;
			}

			free(tmp_inter_list);
			break;
			//tmp_inter_list = tmp_inter_prev->next;
		}
		else tmp_inter_list = tmp_inter_list->next;
	}
	//sem_post(&node_sem[node_id]);
	sem_getvalue(&node_sem_out[sender_id],&i);
	fprintf(log_file, "sender_id = %d 1Sem value = %d\n",sender_id,i);
	fflush(log_file);
	if (sem_post(&node_sem_out[sender_id]) == -1){
		printf("i am %d ta hpiame pali sender_id = %d\n",node_id,sender_id);
		perror("sem_post");
	}
	sem_getvalue(&node_sem_out[sender_id],&i);
	fprintf(log_file, "Sem value = %d\n",i);
	fflush(log_file);
	//} else printf("I am %d and i in am doing smth else with my agent %d in sig_APPOINT_WORK_handler interaction is %d\n",node_id,sender_id,core_inter_head[sender_id]->type);

	close(fd_r);
	free(fifo_name);
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended REMOVE_FAR_MAN with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

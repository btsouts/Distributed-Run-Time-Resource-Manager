#include "signal_handlers.h"

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
extern int DDS_count, my_cores_count, max_cores_count;
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
extern int init_DDS_replies, selfopt_DDS_replies, init_DDS_idags, selfopt_DDS_idags, selfopt_interval, init_idags_areas_replies, init_areas_num;
extern target_list *init_targets_head, *init_targets_tail;
extern target_list *selfopt_targets_head, *selfopt_targets_tail;
extern application_states app_state;
extern region far_reg;
extern my_time_stamp init_app_times[2], my_app_times[2];

extern int *sig_array, *data_array, NUES, idags_replied;
extern RCCE_FLAG flag_signals_enabled,flag_data_written;

/*void send_init_reqs (int sender_id) {
	int agent_id, i;
	inter_list *tmp_inter_list;
	target_list *tmp_target_list;	
	
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
	target_list *tmp_target_list;	
	
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
*/
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

	if (core_inter_head[sender_id] == NULL) {
				
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
		//kill(info->si_pid, SIG_ACK);		
		//scc_kill(sender_id, SIG_ACK);

		//printf("I come through here\n");
			
		//read(fd_r, &some_stats, sizeof(metrics));
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);		
		scc_kill(sender_id, SIG_ACK);
	
		RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);

		error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), LINE_SIZE * sizeof(int), node_id);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			printf("I am %d and i got an error in get data in sig_TERMINATE_handler from %d with descr %s\n",node_id,sender_id,error_str);
		}

		total_stats.msg_count += data_array_local[0];//some_stats.msg_count;
		total_stats.message_size += data_array_local[1];//some_stats.message_size;
		total_stats.distance += data_array_local[2];//some_stats.distance; 
		total_stats.app_turnaround += data_array_local[3];//some_stats.app_turnaround;
		total_stats.comp_effort += data_array_local[4];//some_stats.comp_effort;
		total_stats.cores_utilized += data_array_local[5];//some_stats.cores_utilized;
		total_stats.times_accessed += data_array_local[6];//some_stats.times_accessed;
		
		stats_replied++;
		fprintf(log_file,"I am %d and my node %d replied stats stats_replied = %d my_cores_count = %d msg_count=%d\n",node_id,sender_id,stats_replied,my_cores_count,data_array_local[0]);	
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
	fprintf(log_file, "[%d:%d:%d]: I entered sig_INIT_APP_handler with sender=%d state=%d pending_state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state,pending_state);
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
		printf("I have to reject sig_INIT_APP sender_id=%d node_id=%d state=%d\n",sender_id,node_id,state);
		fprintf(log_file,"I have to reject sig_INIT_APP sender_id=%d\n",sender_id);
		fflush(log_file);

		//kill(info->si_pid, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	} else {
		//kill(info->si_pid, SIG_ACK);
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);
    scc_kill(sender_id, SIG_ACK);

    RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);

		//scc_kill(sender_id, SIG_ACK);		
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		

		error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), LINE_SIZE * sizeof(int), node_id);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			printf("I am %d and i got an error in get data in sig_INIT_APP from %d with descr %s\n",node_id,sender_id,error_str);
		}

		init_app.id = data_array_local[0];	
		memcpy(&init_app.A,&data_array_local[1],sizeof(int));
		memcpy(&init_app.var,&data_array_local[2],sizeof(int));
		memcpy(&init_app.workld,&data_array_local[3],sizeof(int));	
		init_app.num_of_cores = data_array_local[4];
		//read(fd_r, &init_app, sizeof(app));
		
		printf("I am node with id %d and my app is A = %f, var = %f, cores = %d\n",node_id,init_app.A,init_app.var,init_app.num_of_cores);
	
		cur_time = time(NULL);	
		cur_t = localtime(&cur_time);
		init_app_times[0].tm_sec = cur_t->tm_sec;
		init_app_times[0].tm_min = cur_t->tm_min;
  	init_app_times[0].tm_hour = cur_t->tm_hour;

		printf("I am %d and about to do an init app with my state before change = %d\n",node_id,state);

		/*if (state == IDLE_AGENT_WAITING_OFF || state == AGENT_SELF_CHK_OFFERS || state == AGENT_ZOMBIE || state == AGENT_ENDING) 
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
		} else state = INIT_MANAGER;*/
		state = INIT_MANAGER;
	}

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_INIT_APP_handler with sender=%d state=%d pending_state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state,pending_state);
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
	else if (state == IDLE_INIT_MAN_SELFOPT_PENDING) state = INIT_MAN_CHK_OFFERS_SELFOPT_PENDING;
	else if (state == IDLE_INIT_MAN_WORK_PENDING) state = INIT_MAN_CHK_OFFERS_WORK_PENDING;
	else if (state == WORKING_NODE_IDLE_INIT) {
		state = INIT_MAN_CHK_OFFERS;
		pending_state = WORKING_NODE;
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
	int clear=1, i, j, data_array_local[3 * LINE_SIZE];//, fd_r;//fd_r, ; clear is 1 if node is to be removed
	inter_list *tmp_inter_list;
	DDS_list *tmp_DDS;//, *tmp_inter_prev=NULL;
	offer_list *tmp_offer_list;
	int error, str_len;
	//RCCE_FLAG_STATUS receiver_status;
	char error_str[64];	

	signals_disable();
	
	if (core_inter_head[sender_id] == NULL){
		printf("We were fucked in sig_ACK! sender_id = %d node_id = %d\n",sender_id,node_id);
	} else {
		
		cur_time = time(NULL);	
		cur_t = localtime(&cur_time);
		fprintf(log_file, "[%d:%d:%d]: I entered sig_ACK_handler with sender=%d type=%d state=%d\n",
			cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,core_inter_head[sender_id]->type,state);
		fflush(log_file);
				
		tmp_inter_list = core_inter_head[sender_id];

		if (tmp_inter_list->type == INIT_CORE){ 
			//write_res = write(fd_w, &node_id, sizeof(int));

			data_array_local[0] = node_id;
			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), LINE_SIZE * sizeof(int), sender_id);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				printf("I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
			}	

			RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);

			fprintf(log_file, "node_id=%d\n",node_id);
			fflush(log_file);
			my_stats.message_size += sizeof(int);
			nodes_initialised++;
		} else if (tmp_inter_list->type == INIT_APP){
			//write_res = write(fd_w, &tmp_inter_list->data.new_app, sizeof(app));
			//float tmpf;
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

			/*memcpy(&tmpf,&data_array_local[1],sizeof(int));	
			fprintf(log_file, "A=%0.2f\n",tmpf);
			fflush(log_file);
			memcpy(&tmpf,&data_array_local[2],sizeof(int));                          
      fprintf(log_file, "var=%0.2f\n",tmpf);
      fflush(log_file);
			memcpy(&tmpf,&data_array_local[3],sizeof(int));                          
      fprintf(log_file, "workld=%0.2f\n",tmpf);
      fflush(log_file);*/

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
			//write_res = write(fd_w, &tmp_inter_list->data.idags_in_reg[num_idags_x*num_idags_y], sizeof(int)); //number of idags in region
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
			if (tmp_inter_list->data.agents_in_reg == NULL){//debugging
				fprintf(log_file, "In null rep_idag_dds with sender %d and DDS_count %d\n",sender_id,DDS_count);
				fflush(log_file);				
				//write_res = write(fd_w, &DDS_count, sizeof(int)); //number of idags in region	
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
				//write_res = write(fd_w, &tmp_inter_list->data.agents_in_reg[0], sizeof(int));
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
		} else if (tmp_inter_list->type == AGENT_REQ_CORES_PENDING){
			//write_res = write(fd_w, &init_app, sizeof(app));
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
 			//write_res = write(fd_w, &my_app, sizeof(app));
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
			//write_res = write(fd_w, &tmp_inter_list->data.off_arr.num_of_offers, sizeof(int));
			data_array_local[0] = tmp_inter_list->data.off_arr.num_of_offers;
			fprintf(log_file, "num_of_offers=%d\n",tmp_inter_list->data.off_arr.num_of_offers);
			fflush(log_file);	
			my_stats.message_size += sizeof(int);	

			if (tmp_inter_list->data.off_arr.num_of_offers > 0) {		
				for (j=0; j<tmp_inter_list->data.off_arr.num_of_offers; j++){
					//write_res = write(fd_w, &tmp_inter_list->data.off_arr.offer_arr[j].num_of_cores, sizeof(int));
					data_array_local[1] = tmp_inter_list->data.off_arr.offer_arr[j].num_of_cores;
					fprintf(log_file, "num_of_cores=%d\n",tmp_inter_list->data.off_arr.offer_arr[j].num_of_cores);
					fflush(log_file);
					my_stats.message_size += sizeof(int);
					//write_res = write(fd_w, &tmp_inter_list->data.off_arr.offer_arr[j].spd_loss, sizeof(float));
					memcpy(&data_array_local[2],&tmp_inter_list->data.off_arr.offer_arr[j].spd_loss,sizeof(int));
					fprintf(log_file, "spd_loss=%0.2f\n",tmp_inter_list->data.off_arr.offer_arr[j].spd_loss);
					fflush(log_file);
					my_stats.message_size += sizeof(float);
					for (i=0; i<tmp_inter_list->data.off_arr.offer_arr[j].num_of_cores; i++) {
						data_array_local[i+LINE_SIZE] = tmp_inter_list->data.off_arr.offer_arr[j].offered_cores[i];
						//write_res = write(fd_w, &tmp_inter_list->data.off_arr.offer_arr[j].offered_cores[i], sizeof(int));
						fprintf(log_file, "core=%d\n",tmp_inter_list->data.off_arr.offer_arr[j].offered_cores[i]);
						fflush(log_file);
						my_stats.message_size += sizeof(int);
					}
				}

				core_inter_head[sender_id]->type = AGENT_OFFER_SENT;
				clear = 0;
			}
			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), 2 * LINE_SIZE * sizeof(int), sender_id);
  		if (error != RCCE_SUCCESS) {
    			RCCE_error_string(error, error_str, &str_len);
    			fprintf(log_file,"I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
    			fflush(log_file);
  		}
  		RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
		} else if (tmp_inter_list->type == REP_AGENT_OFFER_SENT) {
			fprintf(log_file, "I have to reply %d for %d offers\n",sender_id,tmp_inter_list->data.offer_acc_array[0]);
			fflush(log_file);
						
			my_stats.message_size += sizeof(int);
			for (i=1; i<=tmp_inter_list->data.offer_acc_array[0]; i++){	
				//write_res = write(fd_w, &tmp_inter_list->data.offer_acc_array[i], sizeof(int));
				data_array_local[i-1] = tmp_inter_list->data.offer_acc_array[i];
				fprintf(log_file, "offer_ans=%d\n",tmp_inter_list->data.offer_acc_array[i]);
				fflush(log_file);
				my_stats.message_size += sizeof(int);
			}

			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), LINE_SIZE * sizeof(int), sender_id);
      if (error != RCCE_SUCCESS) {
        RCCE_error_string(error, error_str, &str_len);
        fprintf(log_file,"I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
        fflush(log_file);
      }
      RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);

			fflush(log_file);
			free(tmp_inter_list->data.offer_acc_array);	
		} else if (tmp_inter_list->type == INIT_AGENT) {
			//write_res = write(fd_w, &init_app, sizeof(app));
			data_array_local[0] = init_app.id;	
			memcpy(&data_array_local[1],&init_app.A,sizeof(int));
			memcpy(&data_array_local[2],&init_app.var,sizeof(int));
			memcpy(&data_array_local[3],&init_app.workld,sizeof(int));	
			data_array_local[4] = init_app.num_of_cores;
			fprintf(log_file, "A=%0.2f\n",init_app.A);
			fflush(log_file);

			my_stats.message_size += sizeof(app);
			for (i=1; i<=init_app.num_of_cores; i++){
				//write_res = write(fd_w, &tmp_inter_list->data.app_cores[i], sizeof(int));
				data_array_local[LINE_SIZE+i-1] = tmp_inter_list->data.app_cores[i];
				fprintf(log_file, "core=%d\n",tmp_inter_list->data.app_cores[i]);
				fflush(log_file);
				my_stats.message_size += sizeof(int);
			}

			data_array_local[2*LINE_SIZE] = init_app_times[0].tm_sec;
			data_array_local[2*LINE_SIZE+1] = init_app_times[0].tm_min;
			data_array_local[2*LINE_SIZE+2] = init_app_times[0].tm_hour;
			data_array_local[2*LINE_SIZE+3] = init_app_times[1].tm_sec;
			data_array_local[2*LINE_SIZE+4] = init_app_times[1].tm_min;
			data_array_local[2*LINE_SIZE+5] = init_app_times[1].tm_hour;

			//write_res = write(fd_w, &init_app_times[0], sizeof(my_time_stamp));
			fprintf(log_file, "sec=%d min=%d hours=%d\n",init_app_times[0].tm_sec,init_app_times[0].tm_min,init_app_times[0].tm_hour);
			fflush(log_file);
			//write_res = write(fd_w, &init_app_times[1], sizeof(my_time_stamp));
			fprintf(log_file, "sec=%d min=%d hours=%d\n",init_app_times[1].tm_sec,init_app_times[1].tm_min,init_app_times[1].tm_hour);
			fflush(log_file);

			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]),3 * LINE_SIZE * sizeof(int), sender_id);
      if (error != RCCE_SUCCESS) {
        RCCE_error_string(error, error_str, &str_len);
        fprintf(log_file,"I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
        fflush(log_file);
      }
      RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);

			init_app.A=-1.0;
			init_app.var=-1.0; 
			init_app.num_of_cores=-1;
			init_app_times[0].tm_sec = 0;
			init_app_times[0].tm_min = 0;
			init_app_times[0].tm_hour = 0;
			init_app_times[1].tm_sec = 0;
			init_app_times[1].tm_min = 0;
			init_app_times[1].tm_hour = 0;

			//kill(pid_num[0], SIG_INIT_APP);
			//my_stats.msg_count++;
			//my_stats.distance += distance(node_id,0);

			free(tmp_inter_list->data.app_cores);
		} else if (tmp_inter_list->type == IDAG_ADD_CORES_DDS || tmp_inter_list->type == IDAG_REM_CORES_DDS) {// || tmp_inter_list->type == REMOVE_APP
			//fprintf(log_file, "I am in add/remove/remove_app to %d with %d cores\n",sender_id,tmp_inter_list->data.app_cores[0]);			
			//fflush(log_file);	
			//write_res = write(fd_w, &tmp_inter_list->data.app_cores[0], sizeof(int));
			data_array_local[0] = tmp_inter_list->data.app_cores[0];
			fprintf(log_file, "app_cores=%d\n",tmp_inter_list->data.app_cores[0]);
			fflush(log_file);
			my_stats.message_size += sizeof(int);
			for (i=1; i<=tmp_inter_list->data.app_cores[0]; i++){			
				//write_res = write(fd_w, &tmp_inter_list->data.app_cores[i], sizeof(int));
				data_array_local[LINE_SIZE+i-1] = tmp_inter_list->data.app_cores[i];
				fprintf(log_file, "core=%d\n",tmp_inter_list->data.app_cores[i]);
				fflush(log_file);
				my_stats.message_size += sizeof(int);
			}
			//I am an idag and i have to send to other idags my original sender
			if (my_idag == -1) {//idag_id != -1
				//write_res = write(fd_w, &tmp_inter_list->data.app_cores[i], sizeof(int));
				data_array_local[1] = tmp_inter_list->data.app_cores[i];
				fprintf(log_file, "orig_sender=%d\n",tmp_inter_list->data.app_cores[i]);
				fflush(log_file);
				my_stats.message_size += sizeof(int);
			}
		
			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]),2 * LINE_SIZE * sizeof(int), sender_id);
      if (error != RCCE_SUCCESS) {
        RCCE_error_string(error, error_str, &str_len);
        fprintf(log_file,"I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
        fflush(log_file);
      }
      RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
		} else if (tmp_inter_list->type == REMOVE_APP) {
			//fprintf(log_file, "I am in add/remove/remove_app to %d with %d cores\n",sender_id,tmp_inter_list->data.app_cores[0]);			
			//fflush(log_file);	
			if (my_idag != -1) {			
				//write_res = write(fd_w, &tmp_inter_list->data.app_cores[0], sizeof(int));
				data_array_local[0] = tmp_inter_list->data.app_cores[0];
				fprintf(log_file, "app_cores=%d\n",tmp_inter_list->data.app_cores[0]);
				fflush(log_file);
				my_stats.message_size += sizeof(int);
				for (i=1; i<=tmp_inter_list->data.app_cores[0]; i++){			
					//write_res = write(fd_w, &tmp_inter_list->data.app_cores[i], sizeof(int));
					data_array_local[LINE_SIZE+i-1] = tmp_inter_list->data.app_cores[i];
					fprintf(log_file, "core=%d\n",tmp_inter_list->data.app_cores[i]);
					fflush(log_file);
					my_stats.message_size += sizeof(int);
				}
			} else {
				//I am an idag and i have to send to other idags my original sender
				//write_res = write(fd_w, &tmp_inter_list->data.agent_ended, sizeof(int));
				data_array_local[0] = tmp_inter_list->data.agent_ended;
				fprintf(log_file, "orig_sender=%d\n",tmp_inter_list->data.agent_ended);
				fflush(log_file);
				my_stats.message_size += sizeof(int);
			}

			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), 2 * LINE_SIZE * sizeof(int), sender_id);
      if (error != RCCE_SUCCESS) {
        RCCE_error_string(error, error_str, &str_len);
        fprintf(log_file,"I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
        fflush(log_file);
      }
      RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
		/*} else if (tmp_inter_list->type == FAR_INIT_REQ) {//I am the requesting common node
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

			if (tmp_inter_list->next != NULL && tmp_inter_list->next->type == FAR_REQ_MAN_APPOINT_PENDING) { //gia na thrhthei h seira k na mhn skaei to apo katw
				fprintf(log_file, "I enter this case\n");
				fflush(log_file);	
				core_inter_head[sender_id] = tmp_inter_list->next;
				free(tmp_inter_list);
				for (tmp_inter_list = core_inter_head[sender_id]; tmp_inter_list != NULL; tmp_inter_list = tmp_inter_list->next)
					fprintf (log_file, "Interaction is %d\n",tmp_inter_list->type);
				fflush(log_file);				
				clear = 0;
			}	
		} else if (tmp_inter_list->type == REP_FAR_INIT_REQ){ 
			write_res = write(fd_w, &tmp_inter_list->data.far_req_man, sizeof(int));
			fprintf(log_file, "far manager=%d size=%d write_res=%d\n",tmp_inter_list->data.far_req_man,sizeof(int),write_res);
			fflush(log_file);
			my_stats.message_size += sizeof(int);
			//printf("i am %d in asdfwe and far_req_man is %d\n",node_id,tmp_inter_list->data.far_req_man);
			//if (core_inter_head[tmp_inter_list->data.far_req_man] == NULL) printf("kariolares\n");
			if (tmp_inter_list->data.far_req_man != node_id && core_inter_head[tmp_inter_list->data.far_req_man] != NULL && 
				core_inter_head[tmp_inter_list->data.far_req_man]->type == FAR_REQ_MAN_APPOINT_PENDING) {//next == NULL) {
				kill(pid_num[tmp_inter_list->data.far_req_man], SIG_FAR_REQ);
				core_inter_head[tmp_inter_list->data.far_req_man]->type = FAR_REQ_MAN_APPOINT;
				fprintf(log_file, "I sent it\n"); 
				fflush(log_file);				
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,tmp_inter_list->data.far_req_man);
			} else {
				fprintf(log_file, "far_man=%d\n",tmp_inter_list->data.far_req_man);
				if (core_inter_head[tmp_inter_list->data.far_req_man] != NULL) fprintf(log_file, "interaction is =%d\n",core_inter_head[tmp_inter_list->data.far_req_man]->type);
				else fprintf(log_file, "no interaction\n");   
				fflush(log_file);
			}
		} else if (tmp_inter_list->type == ABORT_FAR_MAN) { 
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

			if (far_list_count > 0) {
				tmp_offer_list = far_man_offers;
				while (tmp_offer_list != NULL){
					write_res = write(fd_w, &tmp_offer_list->off.num_of_cores, sizeof(int));
					fprintf(log_file, "num_of_cores=%d size=%d write_res=%d\n",tmp_offer_list->off.num_of_cores,sizeof(int),write_res);
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
			} else {
				far_req_app.A=-1.0;
				far_req_app.var=-1.0; 
				far_req_app.num_of_cores=-1;
				far_req_or_sender = -1;
				far_reg.C = -1;
				far_reg.r = -1;
			}
		} else if (tmp_inter_list->type == REP_FAR_REQ_OFFER_SENT) {
			fprintf(log_file, "num_of_offers=%d\n",tmp_inter_list->data.offer_acc_array[0]);
			fflush(log_file);	
			for (i=1; i<=tmp_inter_list->data.offer_acc_array[0]; i++) {
				write_res = write(fd_w, &tmp_inter_list->data.offer_acc_array[i], sizeof(int));	
				fprintf(log_file, "offer_ans=%d size=%d write_res=%d\n",tmp_inter_list->data.offer_acc_array[i],sizeof(int),write_res);
				fflush(log_file);
				my_stats.message_size += sizeof(int);
			}*/
		} else if (tmp_inter_list->type == INIT_WORK_NODE) {
			if (tmp_inter_list->data.work_time != -1) {
				//i=1;
				//write_res = write(fd_w, &i, sizeof(int));
				data_array_local[0] = 1;
				fprintf(log_file, "i=%d\n",i);
				fflush(log_file);
				//write_res = write(fd_w, &node_id, sizeof(int));
				data_array_local[1] = node_id;
				fprintf(log_file, "node_id=%d\n",node_id);
				fflush(log_file);
				//write_res = write(fd_w, &tmp_inter_list->data.work_time, sizeof(int));
				data_array_local[2] = tmp_inter_list->data.work_time;	
				fprintf(log_file, "work_time=%d\n",tmp_inter_list->data.work_time);
				fflush(log_file);
				my_stats.message_size += 3 * sizeof(int);
			} else {
				//i=0;
				//write_res = write(fd_w, &i, sizeof(int));
				data_array_local[0] = 0;
				fprintf(log_file, "i=%d\n",i);
				fflush(log_file);
				my_stats.message_size += sizeof(int);
			}

			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]),2 * LINE_SIZE * sizeof(int), sender_id);
      if (error != RCCE_SUCCESS) {
        RCCE_error_string(error, error_str, &str_len);
        fprintf(log_file,"I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
        fflush(log_file);
      }
      RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
		} else if (tmp_inter_list->type == APPOINT_WORK_NODE) {
			if (tmp_inter_list->data.work_time != -1) {
				/*i=1;
				write_res = write(fd_w, &i, sizeof(int));
				fprintf(log_file, "i=%d size=%d write_res=%d\n",i,sizeof(int),write_res);
				fflush(log_file);
				write_res = write(fd_w, &tmp_inter_list->data.work_time, sizeof(int));
				fprintf(log_file, "work_time=%d size=%d write_res=%d\n",tmp_inter_list->data.work_time,sizeof(int),write_res);
				fflush(log_file);*/
				data_array_local[0] = 1;
				fprintf(log_file, "i=%d\n",i);
				fflush(log_file);
				//write_res = write(fd_w, &node_id, sizeof(int));
				data_array_local[1] = node_id;
				fprintf(log_file, "node_id=%d\n",node_id);
				fflush(log_file);						
				my_stats.message_size += 2 * sizeof(int);
			} else {
				/*i=0;
				write_res = write(fd_w, &i, sizeof(int));
				fprintf(log_file, "i=%d size=%d write_res=%d\n",i,sizeof(int),write_res);
				fflush(log_file);
				my_stats.message_size += sizeof(int);*/
				data_array_local[0] = 0;
				fprintf(log_file, "i=%d\n",i);
				fflush(log_file);
				my_stats.message_size += sizeof(int);
			}
			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]),LINE_SIZE * sizeof(int), sender_id);
      if (error != RCCE_SUCCESS) {
        RCCE_error_string(error, error_str, &str_len);
        fprintf(log_file,"I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
        fflush(log_file);
      }
      RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
		} else if (tmp_inter_list->type == REP_CHK_REM_TIME) {
			//write_res = write(fd_w, &tmp_inter_list->data.work_time, sizeof(int));
			data_array_local[0] = tmp_inter_list->data.work_time;
			fprintf(log_file, "work_time=%d\n",tmp_inter_list->data.work_time);
			fflush(log_file);						
			my_stats.message_size += sizeof(int);

			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]),LINE_SIZE * sizeof(int), sender_id);
      if (error != RCCE_SUCCESS) {
        RCCE_error_string(error, error_str, &str_len);
        fprintf(log_file,"I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
        fflush(log_file);
      }
      RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id); 
		} else if (tmp_inter_list->type == REP_STATISTICS) {
			//write_res = write(fd_w, &tmp_inter_list->data.stats, sizeof(metrics));
			data_array_local[0] = tmp_inter_list->data.stats.msg_count;
			data_array_local[1] = tmp_inter_list->data.stats.message_size;
			data_array_local[2] = tmp_inter_list->data.stats.distance; 
			data_array_local[3] = tmp_inter_list->data.stats.app_turnaround;
			data_array_local[4] = tmp_inter_list->data.stats.comp_effort;
			data_array_local[5] = tmp_inter_list->data.stats.cores_utilized;
			data_array_local[6] = tmp_inter_list->data.stats.times_accessed;
			
			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), LINE_SIZE * sizeof(int), sender_id);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				printf("I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
			}	

			RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
			state = TERMINATED;		
		} else printf("We were fucked inside ACK! node_id = %d sender_id = %d\n",node_id,sender_id);

		if (clear){
			core_inter_head[sender_id] = tmp_inter_list->next;
			if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
			else send_next_signal(core_inter_head[sender_id],sender_id);
			free(tmp_inter_list);
		}
	} 
	
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_ACK_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();	
}

void sig_IDAG_FIND_IDAGS_handler(int sender_id)
{
	int num_of_idags, i, one_idag, error, str_len;//, idags_read=0;
	region cur_reg;
	inter_list *tmp_inter_list;
	int data_array_local[LINE_SIZE];
	//RCCE_FLAG_STATUS receiver_status;
	char error_str[64];	

	signals_disable();
	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_IDAG_FIND_IDAGS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	
		
	if (core_inter_head[sender_id] == NULL || core_inter_head[sender_id]->type == REP_IDAG_FIND_IDAGS){ //I am the idag

		//kill(info->si_pid, SIG_ACK);		
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);		
		scc_kill(sender_id, SIG_ACK);
	
		RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);		
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
												
		//read(fd_r, &cur_reg, sizeof(region));
		error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), LINE_SIZE * sizeof(int), node_id);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			printf("I am %d and i got an error in get data in sig_IDAG_FIND_IDAGS_handler from %d with descr %s\n",node_id,sender_id,error_str);
		}

		cur_reg.C = data_array_local[0];
		cur_reg.r = data_array_local[1];
		
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
		if (core_inter_head[sender_id]->next == NULL) {
			//kill(info->si_pid, SIG_IDAG_FIND_IDAGS);
			scc_kill(sender_id, SIG_IDAG_FIND_IDAGS);
			my_stats.msg_count++;
			my_stats.distance += distance(node_id,sender_id);
		}		
	} else if (core_inter_head[sender_id]->type == IDAG_FIND_IDAGS && state != IDLE_INIT_MAN && state != WORKING_NODE_IDLE_INIT) {
		//IDLE_INIT_MAN_SELFOPT_PENDING && state != IDLE_INIT_MAN_WORK_PENDING) {
		printf("I am %d and i think i cought a stray init idag_find_idags reply from %d. My current state is %d\n",node_id,sender_id,state);
		fprintf(log_file,"I am %d and i think i cought a stray init idag_find_idags reply from %d. My current state is %d\n",node_id,sender_id,state);
		fflush(log_file);

		//kill(info->si_pid, SIG_REJECT);
		scc_kill(sender_id, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);		
		
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

		//kill(info->si_pid, SIG_REJECT);
		scc_kill(sender_id, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		free(tmp_inter_list);
		
		if(core_inter_head[sender_id] == NULL) {
			if (core_inter_tail[sender_id] == NULL){
				printf("Malaka gamietai to core_inter_tail\n");				
				core_inter_tail[sender_id] = NULL;
			}
		} else send_next_signal(core_inter_head[sender_id], sender_id);
	} else if (core_inter_head[sender_id]->type == IDAG_FIND_IDAGS || core_inter_head[sender_id]->type == SELFOPT_IDAG_FIND_IDAGS) { //I am the requesting common node
		
		//kill(info->si_pid, SIG_ACK);
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);		
		scc_kill(sender_id, SIG_ACK);
	
		RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);			
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
				
		//read(fd_r, &num_of_idags, sizeof(int));
		error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), LINE_SIZE * sizeof(int), node_id);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			printf("I am %d and i got an error in get data in sig_IDAG_FIND_IDAGS_handler from %d with descr %s\n",node_id,sender_id,error_str);
		}

		num_of_idags = data_array_local[0];
		fprintf(log_file,"Number of agents in region %d\n",num_of_idags);
		fflush(log_file);
		//if (core_inter_head[sender_id]->type == FAR_INIT_IDAG_FIND_IDAGS) far_req_max_man_count = num_of_idags;

		if (core_inter_head[sender_id]->type == IDAG_FIND_IDAGS) {
			init_idags_areas_replies++;			
			init_DDS_idags += num_of_idags;		
		} else if (core_inter_head[sender_id]->type == SELFOPT_IDAG_FIND_IDAGS)	selfopt_DDS_idags += num_of_idags;
		
		for (i=0; i<num_of_idags; i++){ //max 4 idags
			//read(fd_r, &one_idag, sizeof(int));
			one_idag = data_array_local[i];
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
				//kill(pid_num[one_idag], SIG_REQ_DDS); //newly created
				scc_kill(one_idag, SIG_REQ_DDS);				
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,one_idag);
			} else {
				fprintf(log_file,"I did not sent req_dds to %d with interaction = %d inter 2=%d\n",one_idag,core_inter_head[one_idag]->type,core_inter_head[one_idag]->next->type);
				fflush(log_file);
			}
		}

		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);		
		free(tmp_inter_list);		
	} else if (core_inter_head[sender_id]->type == IDAG_FIND_IDAGS_PENDING || core_inter_head[sender_id]->type == SELFOPT_IDAG_FIND_IDAGS_PENDING) {
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

		//kill(info->si_pid, SIG_REJECT);
		scc_kill(sender_id, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	}

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_IDAG_FIND_IDAGS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

void sig_REQ_DDS_handler(int sender_id)
{
	int num_of_agents, i, agent_id, num_of_cores, j, agents_read=0;
	region cur_reg;
	core_list *tmp_cores_list;
	inter_list *tmp_inter_list;
	DDS_list *tmp_DDS;
	target_list *tmp_target_list;	
	int data_array_local[LINE_SIZE], error, str_len;
	//RCCE_FLAG_STATUS receiver_status;
	char error_str[64];	

	signals_disable();
	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_REQ_DDS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	
	
	if (core_inter_head[sender_id] == NULL){ //I am the idag
		//kill(info->si_pid, SIG_ACK);		
		
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);		
		scc_kill(sender_id, SIG_ACK);
	
		RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);		
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
												
		//read(fd_r, &cur_reg, sizeof(region));
		error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), LINE_SIZE * sizeof(int), node_id);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			printf("I am %d and i got an error in get data in sig_REQ_DDS_handler from %d with descr %s\n",node_id,sender_id,error_str);
		}

		cur_reg.C = data_array_local[0];
		cur_reg.r = data_array_local[1];
		//read(fd_r, &cur_reg, sizeof(region));

		fprintf(log_file,"I am to investigate region C=%d r=%d for %d\n",cur_reg.C,cur_reg.r,sender_id);		
		fflush(log_file);

		if (core_inter_head[sender_id] == NULL){
			core_inter_head[sender_id] = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[sender_id] = core_inter_head[sender_id];
		} else {
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
 
		//kill(info->si_pid, SIG_REQ_DDS);
		scc_kill(sender_id, SIG_REQ_DDS);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	} else if (core_inter_head[sender_id]->type == IDAG_REQ_DDS && state != IDLE_INIT_MAN && state != WORKING_NODE_IDLE_INIT) {
		//IDLE_INIT_MAN_SELFOPT_PENDING && state != IDLE_INIT_MAN_WORK_PENDING) {
		printf("I am %d and i think i cought a stray init IDAG_REQ_DDS reply from %d. My current state is %d\n",node_id,sender_id,state);
		fprintf(log_file,"I am %d and i think i cought a stray init IDAG_REQ_DDS reply from %d. My current state is %d\n",node_id,sender_id,state);
		fflush(log_file);

		//kill(info->si_pid, SIG_REJECT);
		scc_kill(sender_id, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		
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

		//kill(info->si_pid, SIG_REJECT);
		scc_kill(sender_id, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		
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
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);		
		scc_kill(sender_id, SIG_ACK);
	
		RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);			
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
				
		//read(fd_r, &num_of_idags, sizeof(int));
		error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), LINE_SIZE * sizeof(int), node_id);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			printf("I am %d and i got an error in get data in sig_REQ_DDS_handler from %d with descr %s\n",node_id,sender_id,error_str);
		}

		if (core_inter_head[sender_id]->type == IDAG_REQ_DDS) {
			init_DDS_replies++;
			fprintf(log_file,"init_DDS_idags = %d, init_DDS_replies = %d init_idags_areas_replies=%d\n",init_DDS_idags,init_DDS_replies,init_idags_areas_replies);
			fflush(log_file);
		} else if (core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS) {
			selfopt_DDS_replies++;
			fprintf(log_file,"selfopt_DDS_idags = %d, selfopt_DDS_replies = %d\n",selfopt_DDS_idags,selfopt_DDS_replies);
			fflush(log_file);		
		}

		//read(fd_r, &num_of_agents, sizeof(int));
		num_of_agents = data_array_local[0];	
		while (num_of_agents > X_max*Y_max) {
			printf("I am %d and in req_dds i got shit num_of_agents %d from %d\n",node_id,num_of_agents,sender_id);	
			//read(fd_r, &num_of_agents, sizeof(int));
		}
		
		fprintf(log_file, "Number of agents in region = %d of %d reg = (%d,%d)\n",num_of_agents,sender_id,core_inter_head[sender_id]->data.reg.C,core_inter_head[sender_id]->data.reg.r);
		fflush(log_file);

		//for (i=0; i<num_of_agents; i++) 
		i = 2; //bypass second element of array to produce cleaner code
		while (agents_read < num_of_agents) {
			//read(fd_r, &agent_id, sizeof(int));
			//read(fd_r, &num_of_cores, sizeof(int));
			if (i > LINE_SIZE - 1) {
				error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[LINE_SIZE]), LINE_SIZE * sizeof(int), node_id);
				if (error != RCCE_SUCCESS) {
					RCCE_error_string(error, error_str, &str_len);
					printf("I am %d and i got an error in get data in sig_IDAG_FIND_IDAGS_handler from %d with descr %s\n",node_id,sender_id,error_str);
				}
			
				i=0;
			}
			agent_id = data_array_local[i++];
			num_of_cores = data_array_local[i++];
			agents_read++;

			fprintf(log_file, "there is an agent with id %d and %d cores\n",agent_id,num_of_cores);
			fflush(log_file);
			if (agent_id == node_id) continue;//((node_id == -1 && agent_id == idag_id) || node_id == agent_id) 

			if (core_inter_head[sender_id]->type == IDAG_REQ_DDS) {
				tmp_target_list = init_targets_head;
				while (tmp_target_list != NULL && tmp_target_list->target != agent_id) 
					tmp_target_list = tmp_target_list->next; 

				if (tmp_target_list == NULL) {
					if (init_targets_head == NULL) {
						init_targets_head = (target_list *) malloc(sizeof(target_list));
						init_targets_tail = init_targets_head;;
					} else {
						init_targets_tail->next = (target_list *) malloc(sizeof(target_list));
						init_targets_tail =	init_targets_tail->next;
					}

					init_targets_tail->next = NULL;
					init_targets_tail->target = agent_id;
					init_targets_tail->num_of_regions = 1;
					init_targets_tail->region_arr[0] = core_inter_head[sender_id]->data.reg;
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
				}	
			} else if (core_inter_head[sender_id]->type == FAR_REQ_IDAG_REQ_DDS) {
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
					//kill(pid_num[agent_id], SIG_REQ_CORES); //newly created, not an idag
					scc_kill(agent_id, SIG_REQ_CORES);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,agent_id);
				}
			} 
		}
		
		if (core_inter_head[sender_id]->type == IDAG_REQ_DDS && init_DDS_replies == init_DDS_idags && init_idags_areas_replies == init_areas_num) {
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

					if (core_inter_head[agent_id]->next == NULL) {
						//kill(pid_num[agent_id], SIG_REQ_CORES);
						scc_kill(agent_id, SIG_REQ_CORES);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,agent_id);
					} else if (agent_id == sender_id && core_inter_head[agent_id]->next->type == AGENT_REQ_CORES_PENDING) { //den exei fugei apo to free alla einai wra na stalei
						//kill(pid_num[agent_id], SIG_REQ_CORES);
						scc_kill(agent_id, SIG_REQ_CORES);
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
					//kill(pid_num[agent_id], SIG_REQ_CORES);
					scc_kill(agent_id, SIG_REQ_CORES);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,agent_id);
				} else if (agent_id == sender_id && core_inter_head[agent_id]->next->type == SELFOPT_REQ_CORES_PENDING) {
					//kill(pid_num[agent_id], SIG_REQ_CORES);
					scc_kill(agent_id, SIG_REQ_CORES);
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
		
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else if (core_inter_head[sender_id]->type != AGENT_REQ_CORES_PENDING && core_inter_head[sender_id]->type != SELFOPT_REQ_CORES_PENDING)//far_req_max_man != sender_id && 
			send_next_signal(core_inter_head[sender_id], sender_id);
		free(tmp_inter_list);		
	} else if (core_inter_head[sender_id]->type == DEBUG_IDAG_REQ_DDS){ //I am the requesting common node
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);		
		scc_kill(sender_id, SIG_ACK);
	
		RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);			
						
		//read(fd_r, &num_of_idags, sizeof(int));
		error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), LINE_SIZE * sizeof(int), node_id);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			fprintf(log_file,"I am %d and i got an error in get data in sig_REQ_DDS_handler from %d with descr %s\n",node_id,sender_id,error_str);
			fflush(log_file);	
		}
		//read(fd_r, &num_of_agents, sizeof(int));
		num_of_agents = data_array_local[0];	
		printf("\nNumber of agents in region = %d\n",num_of_agents);

		fprintf(log_file,"I come here a\n");
                fflush(log_file);	
		//for (i=0; i<num_of_agents; i++) 
		i = 2; //bypass second element of array to produce cleaner code
		while (agents_read < num_of_agents) {
			//read(fd_r, &agent_id, sizeof(int));
			//read(fd_r, &num_of_cores, sizeof(int));
			if (i > LINE_SIZE - 1) {
				error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[LINE_SIZE]), LINE_SIZE * sizeof(int), node_id);
				if (error != RCCE_SUCCESS) {
					RCCE_error_string(error, error_str, &str_len);
					printf("I am %d and i got an error in get data in sig_IDAG_FIND_IDAGS_handler from %d with descr %s\n",node_id,sender_id,error_str);
				}
			
				i=0;
			}
			agent_id = data_array_local[i++];
			num_of_cores = data_array_local[i++];
			agents_read++;

			printf("Agent no %d is %d with %d cores\n",i,agent_id,num_of_cores);	
		}
		
		fprintf(log_file,"I come here b\n");
                fflush(log_file);

		idags_replied++;
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

		//kill(info->si_pid, SIG_REJECT);
		scc_kill(sender_id, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	}

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_REQ_DDS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

void sig_REQ_CORES_handler(int sender_id)
{
	int i, tmp_int, num_of_offers, j, non_zero_offers=0, cores_util;//num_of_idags, i, one_idag;
	float req_gain;
	region cur_reg;
	app req_app;
	offer one_offer;
	offer_list *tmp_offer_list, *tmp_offer_prev = NULL, *tmp_head, *chosen_node;
	inter_list *tmp_inter_list;	
	offer_array off_arr;
	DDS_list *tmp_DDS;
	int data_array_local[2 * LINE_SIZE], error, str_len;
	//RCCE_FLAG_STATUS receiver_status;
	char error_str[64];

	signals_disable();
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_REQ_CORES_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	
		
	if (core_inter_head[sender_id] == NULL && my_cores != NULL) { 
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);
    scc_kill(sender_id, SIG_ACK);

    RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);
	
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);

		error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), LINE_SIZE * sizeof(int), node_id);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			printf("I am %d and i got an error in get data in sig_REQ_CORES_handler from %d with descr %s\n",node_id,sender_id,error_str);
		}

		req_app.id = data_array_local[0];	
		memcpy(&req_app.A,&data_array_local[1],sizeof(int));
		memcpy(&req_app.var,&data_array_local[2],sizeof(int));
		memcpy(&req_app.workld,&data_array_local[3],sizeof(int));	
		req_app.num_of_cores = data_array_local[4];
						
		//read(fd_r, &req_app, sizeof(app));
			
		if (core_inter_head[sender_id] == NULL) {
			core_inter_head[sender_id] = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[sender_id] = core_inter_head[sender_id];
		} else {
			core_inter_tail[sender_id]->next = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[sender_id] = core_inter_tail[sender_id]->next;
		}

		core_inter_tail[sender_id]->type = REP_AGENT_REQ_CORES;
		off_arr.num_of_offers = data_array_local[5];
		//read(fd_r, &off_arr.num_of_offers, sizeof(int));
		/*while (off_arr.num_of_offers > OLD_INIT_AREAS_NUM) {
			printf("I am %d kai fagame skoupidia apo ton %d kai einai %d\n",node_id,sender_id,off_arr.num_of_offers);
			read(fd_r, &off_arr.num_of_offers, sizeof(int));
		}*/
		if (off_arr.num_of_offers > 1) {
			fprintf(log_file,"I got num of offers greater than %d\n",off_arr.num_of_offers);
			fflush(log_file);
		}

		off_arr.offer_arr = (offer *) malloc(off_arr.num_of_offers * sizeof(offer));
		core_inter_tail[sender_id]->next = NULL;

		for (i=0; i<off_arr.num_of_offers; i++) {
			//read(fd_r, &cur_reg, sizeof(region));
			cur_reg.C = data_array_local[6];
			cur_reg.r = data_array_local[7];			

			if (my_idag == -1) {
				off_arr.offer_arr[i].offered_cores = (int *) malloc(my_cores_count*sizeof(int));		
			
				tmp_int = offer_cores(my_cores, req_app, cur_reg, off_arr.offer_arr[i].offered_cores, sender_id);
				off_arr.offer_arr[i].num_of_cores = tmp_int;
				my_stats.comp_effort++;
				if (tmp_int > 0) non_zero_offers++;

				cores_util = 0;
				for (tmp_DDS = DDS->next; tmp_DDS != NULL; tmp_DDS = tmp_DDS->next)
					cores_util += tmp_DDS->num_of_cores;

				if (cores_util == 0) off_arr.offer_arr[i].spd_loss = -2.0;
				else off_arr.offer_arr[i].spd_loss = (float) (-1 * cores_util) / (my_cores_count-1);//0.0;
			} else if (my_cores != NULL && my_cores_count>2) {
				off_arr.offer_arr[i].offered_cores = (int *) malloc(my_cores_count*sizeof(int));		
			
				tmp_int = offer_cores(my_cores, req_app, cur_reg, off_arr.offer_arr[i].offered_cores, sender_id);
				off_arr.offer_arr[i].num_of_cores = tmp_int;
				my_stats.comp_effort++;
				//printf("I am %d in i = %d and i offer %d cores\n",node_id,i,tmp_int);

				off_arr.offer_arr[i].spd_loss = Speedup(my_app, my_cores_count) - Speedup(my_app, my_cores_count-tmp_int);
				req_gain = Speedup(req_app,req_app.num_of_cores+tmp_int) - Speedup(req_app,req_app.num_of_cores);
				if (tmp_int > 0) {
					non_zero_offers++;
					fprintf(log_file,"I offered %d cores with spd_loss = %0.2f and %0.2f gain for the req_app\n",tmp_int,off_arr.offer_arr[i].spd_loss,req_gain);
					fflush(log_file);
					fprintf(app_log_file,"I offered %d cores with spd_loss = %0.2f and %0.2f gain for the req_app\n"
						,tmp_int,off_arr.offer_arr[i].spd_loss,req_gain);
					fflush(app_log_file);
				}			
			} else {
				off_arr.offer_arr[i].num_of_cores = 0;
				off_arr.offer_arr[i].spd_loss = 0.0;
			} 

			fprintf(log_file, "I offered %d %d cores: ",sender_id,off_arr.offer_arr[i].num_of_cores);
			for (j=0; j<off_arr.offer_arr[i].num_of_cores; j++)
				fprintf(log_file, "%d, ",off_arr.offer_arr[i].offered_cores[j]);
			fprintf(log_file, "\n");
			fflush(log_file);
		}
		
		fprintf(log_file,"non_zero_offers = %d\n",non_zero_offers);
		core_inter_tail[sender_id]->data.off_arr.num_of_offers = non_zero_offers;
		if (non_zero_offers > 0) core_inter_tail[sender_id]->data.off_arr.offer_arr = (offer *) malloc(non_zero_offers * sizeof(offer));
		else core_inter_tail[sender_id]->data.off_arr.offer_arr = NULL;

		j = 0;
		for (i=0; i<off_arr.num_of_offers; i++) 
			if (off_arr.offer_arr[i].num_of_cores > 0) {
				core_inter_tail[sender_id]->data.off_arr.offer_arr[j].num_of_cores = off_arr.offer_arr[i].num_of_cores;
				core_inter_tail[sender_id]->data.off_arr.offer_arr[j].spd_loss = off_arr.offer_arr[i].spd_loss;
				core_inter_tail[sender_id]->data.off_arr.offer_arr[j].offered_cores = (int *) malloc(off_arr.offer_arr[i].num_of_cores * sizeof(int));
				for (tmp_int=0; tmp_int<off_arr.offer_arr[i].num_of_cores; tmp_int++)
					core_inter_tail[sender_id]->data.off_arr.offer_arr[j].offered_cores[tmp_int] = off_arr.offer_arr[i].offered_cores[tmp_int];
				j++;
			}

		if (core_inter_head[sender_id]->next == NULL) {
			//kill(info->si_pid, SIG_REQ_CORES);
			scc_kill(sender_id, SIG_REQ_CORES);
			my_stats.msg_count++;
			my_stats.distance += distance(node_id,sender_id);
		} else printf("Apparently not null interaction=%d\n",core_inter_head[sender_id]->type);
		
	} else if (core_inter_head[sender_id] == NULL) {
		printf("I am %d and i have to reject req_cores from %d with null interaction\n",node_id,sender_id);
		fprintf(log_file,"i have to reject req_cores from %d. with null interaction\n",sender_id);
		fflush(log_file);

		scc_kill(sender_id, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	} else if (core_inter_head[sender_id]->type == AGENT_REQ_CORES && state != IDLE_INIT_MAN && state != WORKING_NODE_IDLE_INIT) {
		//IDLE_INIT_MAN_SELFOPT_PENDING && state != IDLE_INIT_MAN_WORK_PENDING) {
		printf("I am %d and i think i cought a stray init AGENT_REQ_CORES reply from %d. My current state is %d\n",node_id,sender_id,state);
		fprintf(log_file,"I am %d and i think i cought a stray init AGENT_REQ_CORES reply from %d. My current state is %d\n",node_id,sender_id,state);
		fflush(log_file);

		scc_kill(sender_id, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		
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

		scc_kill(sender_id, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		free(tmp_inter_list);
		
		if(core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);
	} else if (core_inter_head[sender_id]->type == FAR_REQ_CORES && (state == IDLE_IDAG || (node_id == 0 && time_for_farman == -1))) {
		printf("I am %d and i think i cought a stray far_REQ_CORES reply from %d. My current state is %d\n",node_id,sender_id,state);
		fprintf(log_file,"I am %d and i think i cought a stray far_REQ_CORES reply from %d. My current state is %d\n",node_id,sender_id,state);
		fflush(log_file);

		scc_kill(sender_id, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		free(tmp_inter_list);
		
		if(core_inter_head[sender_id] == NULL) {
			if (core_inter_tail[sender_id] == NULL){
				printf("Malaka gamietai to core_inter_tail\n");				
				core_inter_tail[sender_id] = NULL;
			}
		} else send_next_signal(core_inter_head[sender_id], sender_id);
	} else if (core_inter_head[sender_id]->type == AGENT_REQ_CORES || core_inter_head[sender_id]->type == FAR_REQ_CORES || core_inter_head[sender_id]->type == SELFOPT_REQ_CORES){ 
		//I am the requesting common node
		
		//kill(info->si_pid, SIG_ACK);
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);
    scc_kill(sender_id, SIG_ACK);

    RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);		
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		
		error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), 2 * LINE_SIZE * sizeof(int), node_id);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			printf("I am %d and i got an error in get data in sig_REQ_CORES_handler from %d with descr %s\n",node_id,sender_id,error_str);
		}
		
		if (core_inter_head[sender_id]->type == AGENT_REQ_CORES || core_inter_head[sender_id]->type == SELFOPT_REQ_CORES) {//den erxontai ta far edw
			//read(fd_r, &num_of_offers, sizeof(int));
			num_of_offers = data_array_local[0];
			fprintf(log_file, "num_of_offers = %d\n",num_of_offers);
			fflush(log_file);

			if (off_arr.num_of_offers > 1) {
				fprintf(log_file,"I got offered offers greater than 1 = %d\n",off_arr.num_of_offers);
				fflush(log_file);
			}

			if (num_of_offers > 0) {
				core_inter_head[sender_id]->data.offer_acc_array = (int *) malloc((num_of_offers+1)*sizeof(int));	
				core_inter_head[sender_id]->data.offer_acc_array[0] = num_of_offers;			
				//core_inter_head[sender_id]->data.offer_accepted = -1;
				//one_offer.offered_cores = NULL;

				for (j=1; j<=num_of_offers; j++){
					//read(fd_r, &one_offer.num_of_cores, sizeof(int));
					//read(fd_r, &one_offer.spd_loss, sizeof(float));
					one_offer.num_of_cores = data_array_local[1];
					memcpy(&one_offer.spd_loss,&data_array_local[2],sizeof(int));					
	
					if (core_inter_head[sender_id]->type == AGENT_REQ_CORES && init_man_offers == NULL) {
						init_man_offers = (offer_list *) malloc(sizeof(offer_list));
						chosen_node = init_man_offers;
						init_man_offers->next = NULL;
					} else if (core_inter_head[sender_id]->type == SELFOPT_REQ_CORES && selfopt_man_offers == NULL){
						selfopt_man_offers = (offer_list *) malloc(sizeof(offer_list));
						chosen_node = selfopt_man_offers;
						selfopt_man_offers->next = NULL;	
					} else {
						tmp_offer_prev = NULL;

						if (core_inter_head[sender_id]->type == AGENT_REQ_CORES) {
							tmp_offer_list = init_man_offers;

							if (one_offer.spd_loss < 0.0) {
								while (tmp_offer_list != NULL && tmp_offer_list->off.num_of_cores >= one_offer.num_of_cores && tmp_offer_list->off.spd_loss < 0.0){
									if (tmp_offer_list->off.num_of_cores > one_offer.num_of_cores) {									
										tmp_offer_prev = tmp_offer_list;			
										tmp_offer_list = tmp_offer_list->next;
									} else if (tmp_offer_list->off.spd_loss < one_offer.spd_loss) {
										tmp_offer_prev = tmp_offer_list;			
										tmp_offer_list = tmp_offer_list->next;
									} else break;
								}

								/*while (tmp_offer_list != NULL && tmp_offer_list->off.spd_loss < one_offer.spd_loss && tmp_offer_list->off.spd_loss < 0.0){
									tmp_offer_prev = tmp_offer_list;			
									tmp_offer_list = tmp_offer_list->next;
								}*/
							} else {
								while (tmp_offer_list != NULL && (tmp_offer_list->off.spd_loss < 0.0 || tmp_offer_list->off.num_of_cores >= one_offer.num_of_cores)){
									tmp_offer_prev = tmp_offer_list;			
									tmp_offer_list = tmp_offer_list->next;
								}
							}
						} else {
							tmp_offer_list = selfopt_man_offers;						

							while (tmp_offer_list != NULL && tmp_offer_list->off.num_of_cores >= one_offer.num_of_cores){
								tmp_offer_prev = tmp_offer_list;			
								tmp_offer_list = tmp_offer_list->next;
							}
						}

						if (tmp_offer_list == NULL) { //prepei na mpei teleutaio
							tmp_offer_prev->next = (offer_list *) malloc(sizeof(offer_list));
							tmp_offer_list = tmp_offer_prev->next;
							tmp_offer_list->next = NULL;
							chosen_node = tmp_offer_list;
						} else if (tmp_offer_prev == NULL) { //prepei na mpei prwto
							if (core_inter_head[sender_id]->type == AGENT_REQ_CORES) {
								init_man_offers = (offer_list *) malloc(sizeof(offer_list));
								tmp_head = init_man_offers;
							} else {
								selfopt_man_offers = (offer_list *) malloc(sizeof(offer_list));
								tmp_head = selfopt_man_offers;
							}
						
							chosen_node = tmp_head;
							tmp_head->next = tmp_offer_list;
						} else {
							tmp_offer_prev->next = (offer_list *) malloc(sizeof(offer_list));
							tmp_offer_prev = tmp_offer_prev->next;
							chosen_node = tmp_offer_prev;						
							tmp_offer_prev->next = tmp_offer_list;
						}
					}
	
					chosen_node->off.num_of_cores = one_offer.num_of_cores;
					chosen_node->off.spd_loss = one_offer.spd_loss;					
					chosen_node->off.offered_cores = (int *) malloc(one_offer.num_of_cores*sizeof(int));
					for (i=0; i<one_offer.num_of_cores; i++)
						//read(fd_r, &chosen_node->off.offered_cores[i], sizeof(int));
						chosen_node->off.offered_cores[i] = data_array_local[i+LINE_SIZE];
					/*printf("asdasNode %d is offering %d cores: ",sender_id,chosen_node->off.num_of_cores);
					for (i=0; i<chosen_node->off.num_of_cores; i++)
						printf(" %d,",chosen_node->off.offered_cores[i]);
					printf("\n");*/
					chosen_node->sender = sender_id;
					core_inter_head[sender_id]->data.offer_acc_array[j] = -1;
					chosen_node->answer = &core_inter_head[sender_id]->data.offer_acc_array[j];

					cur_time = time(NULL);	
					cur_t = localtime(&cur_time);
					fprintf(log_file, "[%d:%d:%d]: One node successfully added in list type=%d sender_id=%d\n",
						cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,core_inter_head[sender_id]->type,sender_id);
					fflush(log_file);
				}
				core_inter_head[sender_id]->type = REP_AGENT_OFFER_PENDING;
			} else { //no answer is required
				tmp_inter_list = core_inter_head[sender_id];
				core_inter_head[sender_id] = core_inter_head[sender_id]->next;
				free(tmp_inter_list);
		
				if(core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
				else send_next_signal(core_inter_head[sender_id], sender_id);
			}
		} else {
			fprintf(log_file, "I somehow got into a far request\n");
			fflush(log_file);
			/*read(fd_r, &num_of_offers, sizeof(int));
			fprintf(log_file, "num_of_offers is %d\n",num_of_offers);
			fflush(log_file);
			if (num_of_offers > 0) {
				core_inter_head[sender_id]->data.offer_acc_array = (int *) malloc((num_of_offers+1)*sizeof(int));	
				core_inter_head[sender_id]->data.offer_acc_array[0] = num_of_offers;			
				core_inter_head[sender_id]->type = REP_AGENT_OFFER_PENDING;

				read(fd_r, &one_offer.num_of_cores, sizeof(int));
				read(fd_r, &one_offer.spd_loss, sizeof(float));
				one_offer.offered_cores = (int *) malloc(one_offer.num_of_cores*sizeof(int));
				for (i=0; i<one_offer.num_of_cores; i++)
					read(fd_r, &one_offer.offered_cores[i], sizeof(int));
				fprintf(log_file, "%d is offering %d cores with speedup loss %f\n",sender_id,one_offer.num_of_cores,one_offer.spd_loss);
				fflush(log_file);	
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
			}	else { //no answer is required
				tmp_inter_list = core_inter_head[sender_id];
				core_inter_head[sender_id] = core_inter_head[sender_id]->next;
				free(tmp_inter_list);
		
				if(core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
				else send_next_signal(core_inter_head[sender_id], sender_id);
			}*/		
		}
		
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
		printf("I am %d and i have to reject req_cores from %d. Our interaction is %d\n",node_id,sender_id,core_inter_head[sender_id]->type);
		fprintf(log_file,"I am %d and i have to reject req_cores from %d. Our interaction is %d\n",node_id,sender_id,core_inter_head[sender_id]->type);
		fflush(log_file);

		scc_kill(sender_id, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	}

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_REQ_CORES_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

void sig_REP_OFFERS_handler(int sender_id)
{	
	int offer_ans, i, one_core, j, old_cores_cnt;//num_of_idags, i, one_idag;
	core_list *tmp_cores, *tmp_cores_prev, *tmp_cores_list;
	inter_list *tmp_inter_list, *tmp_inter_prev;
	offer_list *tmp_offer_list;
	int data_array_local[2 * LINE_SIZE], error, str_len;
	//RCCE_FLAG_STATUS receiver_status;
	char error_str[64];	
	
	signals_disable();
	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_REP_OFFERS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	
	if (core_inter_head[sender_id] == NULL) printf("fail!\n");
	else if (core_inter_head[sender_id]->type == AGENT_OFFER_SENT) {
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);
    scc_kill(sender_id, SIG_ACK);

    RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);		
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		
		error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), 2 * LINE_SIZE * sizeof(int), node_id);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			printf("I am %d and i got an error in get data in sig_REP_OFFERS_handler from %d with descr %s\n",node_id,sender_id,error_str);
		}	
		
		fprintf(log_file, "Num of offers is  =  %d\n",core_inter_head[sender_id]->data.off_arr.num_of_offers);
		fflush(log_file);
		
		//even if i am in a far req offer, my answer will be the first
		for (j=0; j<core_inter_head[sender_id]->data.off_arr.num_of_offers; j++) {
			//read(fd_r, &offer_ans, sizeof(int));
			offer_ans = data_array_local[0];
			fprintf(log_file, "offer_ans = %d\n",offer_ans);
			fflush(log_file);
					
			while (offer_ans != 0 && offer_ans != 1){
				printf("I am %d and i am getting bizarre answer = %d from %d\n",node_id,offer_ans,sender_id);
				//read(fd_r, &offer_ans, sizeof(int));
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
					DDS->num_of_cores -= core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores;			
				} else {//I am common node				
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
									//kill(pid_num[one_core], SIG_CHECK_REM_TIME);
									scc_kill(one_core, SIG_CHECK_REM_TIME);
									my_stats.msg_count++;
									my_stats.distance += distance(node_id,one_core);
								} else printf("I am %d and I am doing smth else with my working node %d in send SIG_CHECK_REM_TIME in rep offers type = %d\n",
										node_id,one_core,core_inter_head[one_core]->type);
							}
						} 	
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
						//kill(pid_num[my_idag], SIG_REM_CORES_DDS);
						scc_kill(my_idag, SIG_REM_CORES_DDS);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,my_idag);
					} else printf("I did not send rem signal!\n");
					
					cur_time = time(NULL);	
					cur_t = localtime(&cur_time);					
					fprintf(app_log_file, "[%d:%d:%d]: Removal ok. my_cores_count = %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,my_cores_count);
					fprintf(app_log_file, "my cores are:");

					for (tmp_cores_list=my_cores; tmp_cores_list!=NULL; tmp_cores_list=tmp_cores_list->next) {
						//fprintf(log_file, " %d", tmp_cores_list->core_id);
						fprintf(app_log_file, " %d", tmp_cores_list->core_id);					
						//printf(" %d",tmp_cores_list->core_id);
					}

					fprintf(app_log_file, "\n");
					fflush(app_log_file);

					if (app_state == APP_TERMINATED) {//app_terminated
						for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next)
							if (tmp_cores_list->offered_to != -1) break;
						
						if (tmp_cores_list == NULL && state == AGENT_ZOMBIE) state = AGENT_ENDING;
					}
				}
			} else printf("I am %d and my Answer from %d different than 0 or 1 and is %d!!\n",node_id,sender_id,offer_ans);
		}
		
	/*}	else if (core_inter_head[sender_id]->type == FAR_REQ_OFFER_SENT) {
		sem_getvalue(&node_sem[node_id],&i);
		fprintf(log_file, "In 8 Trying to acquire semaphore. Sem value =  %d\n",i);
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
		} else {
			fprintf(log_file, "I went through open\n");
			fflush(log_file);
		}	

		if (far_man_offers->sender == node_id) {		
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
				
			tmp_offer_list = far_man_offers;
			far_man_offers = far_man_offers->next;
			free(tmp_offer_list);
		}

		while (far_man_offers != NULL){
			//printf("kai edw ftamw\n");
			read(fd_r, &offer_ans, sizeof(int));
			printf("I am node %d and far offer answer for node %d is %d\n",node_id,far_man_offers->sender,offer_ans);

			if (offer_ans == 0 || offer_ans == 1){
				*far_man_offers->answer = offer_ans;
				if (core_inter_head[far_man_offers->sender]->type == REP_AGENT_OFFER_PENDING){
					kill(pid_num[far_man_offers->sender],SIG_REP_OFFERS);
					core_inter_head[far_man_offers->sender]->type = REP_AGENT_OFFER_SENT;
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,far_man_offers->sender);
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
		far_reg.C = -1;
		far_reg.r = -1;	*/			
	} else printf("I am %d and fail 2\n",node_id);

	tmp_inter_list = core_inter_head[sender_id];
	core_inter_head[sender_id] = core_inter_head[sender_id]->next;
	if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
	else send_next_signal(core_inter_head[sender_id], sender_id);	
	free(tmp_inter_list);

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_REP_OFFERS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

void sig_INIT_AGENT_handler(int sender_id)
{
	int i, tmp; 
	core_list *tmp_core;
	int data_array_local[3 * LINE_SIZE], error, str_len;
	//RCCE_FLAG_STATUS receiver_status;
	char error_str[64];

	signals_disable();
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_INIT_AGENT_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);

	RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);
  scc_kill(sender_id, SIG_ACK);

  RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);		
	my_stats.msg_count++;
	my_stats.distance += distance(node_id,sender_id);
	
	error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), 3 * LINE_SIZE * sizeof(int), node_id);
	if (error != RCCE_SUCCESS) {
		RCCE_error_string(error, error_str, &str_len);
		printf("I am %d and i got an error in get data in sig_INIT_AGENT_handler from %d with descr %s\n",node_id,sender_id,error_str);
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

	//read(fd_r, &my_app, sizeof(app));
	my_app.id = data_array_local[0];	
	memcpy(&my_app.A,&data_array_local[1],sizeof(int));
	memcpy(&my_app.var,&data_array_local[2],sizeof(int));
	memcpy(&my_app.workld,&data_array_local[3],sizeof(int));	
	my_app.num_of_cores = data_array_local[4];
	
	my_cores_count = my_app.num_of_cores;//+1;
	if (my_cores == NULL) {
		my_cores = (core_list *) malloc(sizeof(core_list));
		my_cores_tail = my_cores;
	} else {
		my_cores_tail->next = (core_list *) malloc(sizeof(core_list));
		my_cores_tail = my_cores_tail->next;
	}

	my_cores_tail->core_id = node_id;
	my_cores_tail->offered_to = -1;
	my_cores_tail->next = NULL;

	//I want myself to be first in my_cores list
	for (i=0; i<my_app.num_of_cores; i++){
		//read(fd_r, &tmp, sizeof(int));
		tmp = data_array_local[i+LINE_SIZE];
		if (tmp != node_id){
			my_cores_tail->next = (core_list *) malloc(sizeof(core_list));
			my_cores_tail = my_cores_tail->next;

			//read(fd_r, &my_cores_tail->core_id, sizeof(int));
			my_cores_tail->core_id = tmp;
			my_cores_tail->offered_to = -1;
			my_cores_tail->next = NULL;
		}
	}

	//read(fd_r, &my_app_times[0], sizeof(my_time_stamp));
	//read(fd_r, &my_app_times[1], sizeof(my_time_stamp));
	my_app_times[0].tm_sec = data_array_local[2*LINE_SIZE];
	my_app_times[0].tm_min = data_array_local[2*LINE_SIZE+1];
	my_app_times[0].tm_hour = data_array_local[2*LINE_SIZE+2];
	my_app_times[1].tm_sec = data_array_local[2*LINE_SIZE+3];
	my_app_times[1].tm_min = data_array_local[2*LINE_SIZE+4];
	my_app_times[1].tm_hour = data_array_local[2*LINE_SIZE+5];

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
	core_inter_tail[my_idag]->data.app_cores = (int *)malloc((my_cores_count+1)*sizeof(int));
	core_inter_tail[my_idag]->data.app_cores[0] = my_cores_count;
			
	tmp_core = my_cores;
	i=1;
	while (tmp_core != NULL){
		core_inter_tail[my_idag]->data.app_cores[i] = tmp_core->core_id;
		tmp_core = tmp_core->next;
		i++;
	}

	core_inter_tail[my_idag]->next = NULL;

	if (core_inter_head[my_idag]->next == NULL) {
		//kill(pid_num[my_idag], SIG_ADD_CORES_DDS);
		scc_kill(my_idag, SIG_ADD_CORES_DDS);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,my_idag);
	} else printf("I am %d and i didn't call add!! with interaction %d\n",node_id,core_inter_head[my_idag]->type);
	
	if (my_agent != -1) {
		printf("I am %d and i do this agent switch\n",node_id);

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
		pending_state = state;
		state = AGENT_INIT_STATE;	
	} else state = AGENT_INIT_STATE;

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_INIT_AGENT_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

//an mou erthei to shma apo allon idag prepei apla na valw, an mou erthei apo common node tote prepei na upologisw
void sig_ADD_CORES_DDS_handler(int sender_id)
{
	int i, is_sender_idag, j, new_agent_id; 
	int *nodes_to_process, nodes_cnt=0, processed_cnt=0;
	int *tmp_list, tmp_cnt, tmp_idag;	
	char *fifo_name;
	core_list *tmp_cores;//, *tmp_cores_list;
	DDS_list *tmp_DDS;
	int data_array_local[2 * LINE_SIZE], error, str_len;
	//RCCE_FLAG_STATUS receiver_status;
	char error_str[64];

	signals_disable();
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_ADD_CORES_DDS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	

	RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);
  scc_kill(sender_id, SIG_ACK);

  RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);		
	my_stats.msg_count++;
	my_stats.distance += distance(node_id,sender_id);
	
	error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), 2 * LINE_SIZE * sizeof(int), node_id);
	if (error != RCCE_SUCCESS) {
		RCCE_error_string(error, error_str, &str_len);
		printf("I am %d and i got an error in get data in sig_ADD_CORES_DDS_handler from %d with descr %s\n",node_id,sender_id,error_str);
	}			

	is_sender_idag = 0;
	for (i=0; i<num_idags; i++)
		if (idag_id_arr[i] == sender_id){
			is_sender_idag = 1;
			break;
		}

	if (is_sender_idag == 0) {
		//read(fd_r, &nodes_cnt, sizeof(int));
		nodes_cnt = data_array_local[0];
		while (nodes_cnt <=0){
			fprintf(log_file,"i am %d and i read %d in nodes_cnt from %d\n",node_id,nodes_cnt,sender_id);
			fflush(log_file);
			//read(fd_r, &nodes_cnt, sizeof(int));
		}
		nodes_to_process = (int *) malloc(nodes_cnt * sizeof(int));
		for (i=0; i<nodes_cnt; i++)
			//read(fd_r, &nodes_to_process[i], sizeof(int));
			nodes_to_process[i] = data_array_local[i+LINE_SIZE];

		tmp_list = (int *) malloc(nodes_cnt * sizeof(int));

		//printf("I am %d in add cores with sender %d and nodes_cnt = %d\n",node_id,sender_id,nodes_cnt);	

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
					//kill(pid_num[tmp_idag], SIG_ADD_CORES_DDS);
					scc_kill(tmp_idag, SIG_ADD_CORES_DDS);
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
		//read(fd_r, &nodes_cnt, sizeof(int));
		nodes_cnt = data_array_local[0];
		new_agent_id = data_array_local[1];
		nodes_to_process = (int *) malloc(nodes_cnt * sizeof(int));
		for (i=0; i<nodes_cnt; i++)
			//read(fd_r, &nodes_to_process[i], sizeof(int));
			nodes_to_process[i] = data_array_local[i+LINE_SIZE];

		//read(fd_r, &new_agent_id, sizeof(int));	

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
	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_ADD_CORES_DDS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

void sig_REM_CORES_DDS_handler(int sender_id)
{
	int i, is_sender_idag, j, new_agent_id; 
	int *nodes_to_process, nodes_cnt=0, processed_cnt=0;
	int *tmp_list, tmp_cnt, tmp_idag;	
	char *fifo_name;
	DDS_list *tmp_DDS,*tmp_DDS_prev;
	int data_array_local[2 * LINE_SIZE], error, str_len;
	//RCCE_FLAG_STATUS receiver_status;
	char error_str[64];

	signals_disable();
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_REM_CORES_DDS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);	
	
	RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);
  scc_kill(sender_id, SIG_ACK);

  RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);		
	my_stats.msg_count++;
	my_stats.distance += distance(node_id,sender_id);
	
	error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), 2 * LINE_SIZE * sizeof(int), node_id);
	if (error != RCCE_SUCCESS) {
		RCCE_error_string(error, error_str, &str_len);
		printf("I am %d and i got an error in get data in sig_REM_CORES_DDS_handler from %d with descr %s\n",node_id,sender_id,error_str);
	}
	
	is_sender_idag = 0;
	for (i=0; i<num_idags; i++)
		if (idag_id_arr[i] == sender_id){
			is_sender_idag = 1;
			break;
		}

	//printf("I am in rem with is_sender_idag = %d\n",is_sender_idag);
	if (is_sender_idag == 0){
		nodes_cnt = data_array_local[0];
		
		//read(fd_r, &nodes_cnt, sizeof(int));
		nodes_to_process = (int *) malloc(nodes_cnt * sizeof(int));
		for (i=0; i<nodes_cnt; i++)
			//read(fd_r, &nodes_to_process[i], sizeof(int));
			nodes_to_process[i] = data_array_local[i+LINE_SIZE];
		
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
					//kill(pid_num[tmp_idag], SIG_REM_CORES_DDS);
					scc_kill(tmp_idag, SIG_REM_CORES_DDS);
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
		nodes_cnt = data_array_local[0];
		new_agent_id = data_array_local[1];
		nodes_to_process = (int *) malloc(nodes_cnt * sizeof(int));
		for (i=0; i<nodes_cnt; i++)
			//read(fd_r, &nodes_to_process[i], sizeof(int));
			nodes_to_process[i] = data_array_local[i+LINE_SIZE];

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

	printf("I am %d Removing ended well! with sender_id=%d!\n",node_id,sender_id);
	printf("Number of agents in region = %d\n",DDS_count);	
	i=0;
	for (tmp_DDS = DDS; tmp_DDS != NULL; tmp_DDS = tmp_DDS->next){
		printf("Agent no %d is %d with %d cores\n",i,tmp_DDS->agent_id,tmp_DDS->num_of_cores);	
		i++;
	}	

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_REM_CORES_DDS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

void sig_APPOINT_WORK_handler(int sender_id)
{
	int i, valid=0;
	int data_array_local[LINE_SIZE], error, str_len;
	//RCCE_FLAG_STATUS receiver_status;
	char error_str[64]; 
	
	signals_disable();
	
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

	RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);
  scc_kill(sender_id, SIG_ACK);

  RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);		
	my_stats.msg_count++;
	my_stats.distance += distance(node_id,sender_id);		
	
	//read(fd_r, &valid, sizeof(int));
	data_array_local[0] = valid;
	fprintf(log_file, "Validity of message = %d\n",valid);
	fflush(log_file);

	if (valid == 1) {
		if (my_agent == -1) my_agent = data_array_local[1];//read(fd_r, &my_agent, sizeof(int));// || my_agent != sender_id
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
			//read(fd_r, &my_agent, sizeof(int));
			my_agent = data_array_local[1];
		}	
		//else if (state != WORKING_NODE) printf("I have been initialied but i am not working\n");

		//read(fd_r, &upper_work_bound, sizeof(long int));
		upper_work_bound = data_array_local[2];
		time_worked=0;
			
		printf("I am node %d with agent %d I am going to start working for %ld\n",node_id,my_agent,upper_work_bound);
		fprintf(log_file,"I am with agent %d I am going to start working for %ld\n",my_agent,upper_work_bound);
		fflush(log_file);

		if (upper_work_bound > 0) {
			if (state == IDLE_INIT_MAN) state = WORKING_NODE_IDLE_INIT;
			else if (state == INIT_MANAGER || state == INIT_MAN_CHK_OFFERS || state == INIT_MANAGER_SEND_OFFERS) {
				pending_state = WORKING_NODE;
				//printf("I am node %d and sou milaw gia mallia\n",node_id);
			} else if (state != WORKING_NODE_IDLE_INIT) state = WORKING_NODE;
		} else {
			//kill(info->si_pid, SIG_FINISH);
			scc_kill(sender_id, SIG_FINISH);
			my_stats.msg_count++;
			my_stats.distance += distance(node_id,sender_id);
		}
	}
	//} else printf("I am %d and i in am doing smth else with my agent %d in sig_APPOINT_WORK_handler interaction is %d\n",node_id,sender_id,core_inter_head[sender_id]->type);

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_APPOINT_WORK_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

void sig_CHECK_REM_TIME_handler(int sender_id)
{
	int tmp_rem_time, i, time_per_node, time_left, time_to_work, one_core; 
	core_list *tmp_cores_list;
	inter_list *tmp_inter_list;		
	float rem_workld;
	int data_array_local[LINE_SIZE], error, str_len;
	//RCCE_FLAG_STATUS receiver_status;
	char error_str[64]; 
	
	signals_disable();
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
		//kill(info->si_pid, SIG_CHECK_REM_TIME);
		scc_kill(sender_id, SIG_CHECK_REM_TIME);
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

		//kill(info->si_pid, SIG_CHECK_REM_TIME);
		scc_kill(sender_id, SIG_CHECK_REM_TIME);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	} else if (core_inter_head[sender_id]->type == REP_CHK_REM_TIME) {
		fprintf(log_file, "I sig_CHECK_REM_TIME_handler b\n");
		fflush(log_file);
		//kill(info->si_pid, SIG_CHECK_REM_TIME);
		scc_kill(sender_id, SIG_CHECK_REM_TIME);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);		 
	} else if (core_inter_head[sender_id]->type == APPOINT_WORK_NODE_PENDING || core_inter_head[sender_id]->type == REMOVED_NODE_REM_TIME) {//CHK_REM_TIME
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);
		scc_kill(sender_id, SIG_ACK);

		RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);		
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);		
		
		if (app_state != RESIZING) {
			printf("I am %d in check rem and app_state = %d app_id = %d\n",node_id,app_state,my_app.id);	
			fprintf(log_file,"I am in check rem and app_state = %d app_id = %d\n",app_state,my_app.id);
			fflush(log_file);
		}

		tmp_rem_time = data_array_local[0];
		//read(fd_r, &tmp_rem_time, sizeof(int));
		fprintf(app_log_file,"tmp_rem_time = %d sender_id = %d\n",tmp_rem_time,sender_id);
		fflush(app_log_file);
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
						
							if (core_inter_head[one_core]->type == APPOINT_WORK_NODE || core_inter_head[one_core]->type == INIT_WORK_NODE) {//||next == NULL
								//kill(pid_num[one_core], SIG_APPOINT_WORK);
								scc_kill(one_core, SIG_APPOINT_WORK);
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
					fprintf(app_log_file, "I enter here with app_state = %d.\n",app_state);
					fflush(app_log_file);

					if (selfopt_interval > 0) {
						if (my_cores_count == max_cores_count) {
							fprintf(app_log_file, "I have maximum cores count. I don't initiate selfopt process.\n");
							fflush(app_log_file);
							selfopt_time_rem = -1;
						} else if (time_per_node <= (selfopt_interval / 2) && my_cores_count > 1) { 
							fprintf(app_log_file, "I have little working time left time_per_node=%d selfopt_interval=%d.\n",time_per_node,selfopt_interval);
							fflush(app_log_file);
							selfopt_time_rem = -1;
						} else {
							its.it_value.tv_nsec = selfopt_interval * MS;
							selfopt_time_rem = selfopt_interval;

							if (timer_settime(timerid, 0, &its, NULL) == -1) {
								perror("timer_settime error8");
								printf("I am %d timer_settime error8 selfopt_interval = %d\n",node_id,selfopt_interval);
							}
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

		//kill(info->si_pid, SIG_REJECT);
		scc_kill(sender_id, SIG_REJECT);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	}
	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_CHECK_REM_TIME_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

void sig_FINISH_handler(int sender_id)
{
	int i, is_sender_idag, new_agent_id; 
	int *nodes_to_process, nodes_cnt=0, processed_cnt=0;
	int *tmp_list, tmp_cnt, tmp_idag;	
	core_list *tmp_cores_list;//tmp_cores, 
	DDS_list *tmp_DDS,*prev_DDS;
	inter_list *tmp_inter_list;	
	int data_array_local[2 * LINE_SIZE], error, str_len;
	//RCCE_FLAG_STATUS receiver_status;
	char error_str[64];
	
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
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);
		scc_kill(sender_id, SIG_ACK);

		RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);		
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		
		is_sender_idag = 0;
		for (i=0; i<num_idags; i++)
			if (idag_id_arr[i] == sender_id){
				is_sender_idag = 1;
				break;
			}

		printf("I am %d in sig finish with is_sender_idag = %d\n",node_id,is_sender_idag);
		if (is_sender_idag == 0){
			//read(fd_r, &nodes_cnt, sizeof(int));
			nodes_cnt = data_array_local[0];
			nodes_to_process = (int *) malloc(nodes_cnt * sizeof(int));
			for (i=0; i<nodes_cnt; i++)
				//read(fd_r, &nodes_to_process[i], sizeof(int));
				nodes_to_process[i] = data_array_local[i+LINE_SIZE];

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
						//kill(pid_num[tmp_idag], SIG_FINISH);
						scc_kill(tmp_idag, SIG_FINISH);
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
			//read(fd_r, &new_agent_id, sizeof(int));
			new_agent_id = data_array_local[0];
			
			fprintf(log_file,"I am %d in Secondary sig_finish for %d\n",node_id,new_agent_id);
			fflush(log_file);

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

			/*my_stats.times_accessed++;
			printf("I am %d Adding ended well with sender_id=%d!\n",node_id,sender_id);
			printf("Number of agents in region = %d\n",DDS_count);	
			printf("Agent no 0 is %d with %d cores\n",DDS->agent_id,DDS->num_of_cores);	
			i=1;
			for (tmp_DDS = DDS->next; tmp_DDS != NULL; tmp_DDS = tmp_DDS->next){
				printf("Agent no %d is %d with %d cores\n",i,tmp_DDS->agent_id,tmp_DDS->num_of_cores);
				my_stats.cores_utilized += tmp_DDS->num_of_cores;
				i++;
			}*/
			
			printf("Secondary removal of agent complete node_id =%d sender_id=%d\n",node_id,sender_id);
			printf("Number of agents in region = %d\n",DDS_count);	
			i=0;
			for (tmp_DDS = DDS; tmp_DDS != NULL; tmp_DDS = tmp_DDS->next){
				printf("Agent no %d is %d with %d cores\n",i,tmp_DDS->agent_id,tmp_DDS->num_of_cores);	
				i++;
			}	
		}
	}

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_FINISH_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable();
}

void sig_REJECT_handler(int sender_id)
{
	int agent_id, i, j;  
	inter_list *tmp_inter_list;
	core_list *tmp_cores_list;
	target_list *tmp_target_list;	
	offer_list *tmp_offer_list;	

	signals_disable();
	
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

			if (init_areas_num == 1) {
				fprintf(log_file, "My only init area was rejected. my state is %d my pending_state = %d\n",state,pending_state);
				fflush(log_file);
				state = INIT_MANAGER_SEND_OFFERS;
			}

		} else if (core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS || core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS_PENDING) {
			selfopt_DDS_replies++;
			fprintf(log_file, "One selfopt_req_dds has been rejected by %d!\n",sender_id);
			fflush(log_file);
		}
				
		if ((core_inter_head[sender_id]->type == IDAG_REQ_DDS || core_inter_head[sender_id]->type == IDAG_REQ_DDS_PENDING) 
			&& init_DDS_replies == init_DDS_idags && init_idags_areas_replies == init_areas_num)
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
						//kill(pid_num[agent_id], SIG_REQ_CORES);
						scc_kill(agent_id, SIG_REQ_CORES);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,agent_id);
					} else if (agent_id == sender_id && core_inter_head[agent_id]->next->type == AGENT_REQ_CORES_PENDING) { //den exei fugei apo to free alla einai wra na stalei
						scc_kill(agent_id, SIG_REQ_CORES);						
						//kill(pid_num[agent_id], SIG_REQ_CORES);
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
					//kill(pid_num[agent_id], SIG_REQ_CORES);
					scc_kill(agent_id, SIG_REQ_CORES);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,agent_id);
				} else if (agent_id == sender_id && core_inter_head[agent_id]->next->type == SELFOPT_REQ_CORES_PENDING) {
					//kill(pid_num[agent_id], SIG_REQ_CORES);
					scc_kill(agent_id, SIG_REQ_CORES);
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
	/*}	else if (core_inter_head[sender_id]->type == FAR_REQ_OFFER) {//FAR_REQ_OFFER_SENT

		if (far_man_offers != NULL && far_man_offers->sender == node_id) {		
			tmp_cores_list = my_cores;
			while (tmp_cores_list != NULL){
				if (tmp_cores_list->offered_to == sender_id) tmp_cores_list->offered_to = -1;
				tmp_cores_list = tmp_cores_list->next;
			}
		
			tmp_offer_list = far_man_offers;
			far_man_offers = far_man_offers->next;
			free(tmp_offer_list);
		}

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
		far_reg.C = -1;
		far_reg.r = -1;

		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);
		free(tmp_inter_list);
	*/
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
	/*}	else if (core_inter_head[sender_id]->type == FAR_REQ_MAN_APPOINT) {
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
		free(tmp_inter_list);	*/
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
	} else {
		printf("I am %d in sig_reject and i have interaction with sender %d interaction = %d\n",node_id,sender_id,core_inter_head[sender_id]->type);
		fprintf(log_file,"I am in sig_reject and i have interaction with sender interaction = %d\n",core_inter_head[sender_id]->type);
		fflush(log_file);
	}
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_REJECT_handler with sender = %d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);
	signals_enable(); 
}

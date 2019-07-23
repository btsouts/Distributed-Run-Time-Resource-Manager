#include "include/signal_handlers.h"
#include "include/libfunctions.h"
#include "include/noc_functions.h"
#include "include/sig_aux.h"
#include "include/scc_signals.h"
#include "include/controller_core.h"
#include "include/common_core.h"
#include "include/idag_defs.h"
#include "include/apps.h"
#include "include/macros.h"
#include "include/resource_negotiation.h"
#include "include/variables.h"

int init_DDS_replies;
int init_DDS_idags;
int selfopt_DDS_replies;
int selfopt_DDS_idags;
int base_offset;
int old_cores_cnt;
int active_working_cores;

int sig_read_ar[2 * LINE_SIZE];

my_time_stamp init_app_times[2];
my_time_stamp my_app_times[2];

application_states app_state;

target_list *init_targets_head;
target_list *init_targets_tail;
target_list *selfopt_targets_head;
target_list *selfopt_targets_tail;

agent_info pending_agent;

void send_next_signal(inter_list *head, int node_num);

#ifdef PLAT_LINUX
void new_RCCE_get(int *target, int *src, int index, int num_of_ints, int ID) {
	int mem_offset = 0, i;
	
	fprintf(log_file,"Inside new_RCCE_get and index = %d and ",index);
	if (src == data_array){
		fprintf(log_file,"src == data_array\n");
		mem_offset = ID * MAX_DATA_LIST_LEN * LINE_SIZE;
	}else if (src == sig_array){
		fprintf(log_file,"src == sig_array\n");
		mem_offset = ID * MAX_SIGNAL_LIST_LEN * LINE_SIZE;
	}else if (src > data_array && src < (data_array + (NUES * MAX_DATA_LIST_LEN * LINE_SIZE) -1)){
		fprintf(log_file,"src > data_array\n");
		mem_offset = ID * MAX_DATA_LIST_LEN * LINE_SIZE;
	}else if (src > sig_array && src < (sig_array + (NUES * MAX_SIGNAL_LIST_LEN * LINE_SIZE) -1)){
		fprintf(log_file,"src > sig_array\n");
		mem_offset = ID * MAX_SIGNAL_LIST_LEN * LINE_SIZE;
	}else {
		fprintf(log_file, "Uknown MPB array in my_RCCE_get. src = 0x%d, data_array = 0x%d, sig_array = 0x%d\n", *src, *data_array, *sig_array);
		fflush(log_file);
	}
	
	for (i = 8; i < 2*num_of_ints; i++){
		if (index == MAX_SIGNAL_LIST_LEN-1){
			target[i] = src[mem_offset + i - 8];
		}else{
			target[i] = src[mem_offset + (index * LINE_SIZE) + i];
		}
	}
	
	/*if (index == 63){
		fprintf(log_file,"\t\tindex_top[%d]=%d.\n\t\tI read from position %d till %d\n",node_id,index,mem_offset,mem_offset+7);
	}else{
		fprintf(log_file,"\t\tindex_top[%d]=%d.\n\t\tI read from position %d till %d\n",node_id,index,mem_offset+index*LINE_SIZE,mem_offset+index*LINE_SIZE+7);
	}*/
}
void my_RCCE_get(int *target, int *src, int num_of_ints, int ID) {
	int mem_offset=0, i;

	if (src == data_array)
		mem_offset = ID * MAX_DATA_LIST_LEN * LINE_SIZE;
	else if (src == sig_array)
		mem_offset = ID * MAX_SIGNAL_LIST_LEN * LINE_SIZE;
	else if (src > data_array && src < (data_array + (NUES * MAX_DATA_LIST_LEN * LINE_SIZE) -1))
		mem_offset = ID * MAX_DATA_LIST_LEN * LINE_SIZE;
	else if (src > sig_array && src < (sig_array + (NUES * MAX_SIGNAL_LIST_LEN * LINE_SIZE) -1))
		mem_offset = ID * MAX_SIGNAL_LIST_LEN * LINE_SIZE;
	else {
		fprintf(log_file, "Uknown MPB array in my_RCCE_get. src = 0x%d, data_array = 0x%d, sig_array = 0x%d\n", *src, *data_array, *sig_array);
		fflush(log_file);
	}

	for (i=0; i<num_of_ints; i++)
		 target[i] = src[mem_offset + i];
}
#endif

/* PAXOS */
extern int proposal_number_personal;
extern timer_t timerid, controller_timer, epfd_timer, pfd_timer;
/* END */

void trigger_shit(int failed_core){
	int i, agent_id;
	target_list *tmp_target_list;
	
	fprintf(log_file,"\t\tI entered trigger_shit!!\n");
	if (core_inter_head[failed_core]->type == IDAG_REQ_DDS || core_inter_head[failed_core]->type == IDAG_REQ_DDS_PENDING){
		init_DDS_replies++;
		if (init_DDS_replies == init_DDS_idags){
			for (tmp_target_list = init_targets_head; tmp_target_list != NULL; tmp_target_list = tmp_target_list->next) {
				agent_id = tmp_target_list->target;
				
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
				core_inter_tail[agent_id]->next = NULL;

				if (core_inter_head[agent_id]->next == NULL) {
					scc_kill(agent_id, SIG_REQ_CORES, core_inter_head[agent_id]);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,agent_id);
				} else {
					fprintf(log_file,"This init is not NULL interaction=%d interaction2=%d\n",core_inter_head[agent_id]->type,core_inter_head[agent_id]->next->type);
					fflush(log_file);
				}
			}
			my_settimer(INIT_NODE_INTERVAL);
			fprintf(log_file, "triggered init REQ_SEND\n");
		}
	}else if (core_inter_head[failed_core]->type == SELFOPT_IDAG_REQ_DDS || core_inter_head[failed_core]->type == SELFOPT_IDAG_REQ_DDS_PENDING){
		selfopt_DDS_replies++;
		if (selfopt_DDS_replies == selfopt_DDS_idags){
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
				
				core_inter_tail[agent_id]->next = NULL;

				if (core_inter_head[agent_id]->next == NULL) {
					scc_kill(agent_id, SIG_REQ_CORES, core_inter_head[agent_id]);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,agent_id);
				} else {
					fprintf(log_file,"Reject selfopt is not NULL interaction=%d interaction2=%d\n",core_inter_head[agent_id]->type,core_inter_head[agent_id]->next->type);
					fflush(log_file);
				}
			}
		
		my_settimer(INIT_NODE_INTERVAL);
		}
	}
}

void send_next_signal(inter_list *head, int node_num){
  
	inter_list *tmp_inter_list = NULL;

	/* signals_disable(); */ /* Commented out on 5.5.2017 */
	
	if (head->type == IDAG_FIND_IDAGS ||
	    head->type == SELFOPT_IDAG_FIND_IDAGS ||
	    head->type == REP_IDAG_FIND_IDAGS ||
	    head->type == SELFOPT_IDAG_FIND_IDAGS_PENDING || 
	    head->type == IDAG_FIND_IDAGS_PENDING){
	  
			scc_kill(node_num, SIG_IDAG_FIND_IDAGS, head);
			
	}else if (head->type == IDAG_REQ_DDS ||
		  head->type == SELFOPT_IDAG_REQ_DDS ||
		  head->type == DEBUG_IDAG_REQ_DDS ||
		  head->type == SELFOPT_IDAG_REQ_DDS_PENDING ||
		  head->type == IDAG_REQ_DDS_PENDING){
	  
			scc_kill(node_num, SIG_REQ_DDS, head);
			
	}else if (head->type == AGENT_REQ_CORES ||
		  head->type == SELFOPT_REQ_CORES ||
		  head->type == AGENT_REQ_CORES_PENDING ||
		  head->type == SELFOPT_REQ_CORES_PENDING){
	  
			scc_kill(node_num, SIG_REQ_CORES, head);
	}else if (head->type == IDAG_ADD_CORES_DDS){
	  
			scc_kill(node_num, SIG_ADD_CORES_DDS, head);
			
	}else if (head->type == IDAG_REM_CORES_DDS){
	  
			scc_kill(node_num, SIG_REM_CORES_DDS, head);
			
	}else if (head->type == INIT_WORK_NODE ||
		  head->type == APPOINT_WORK_NODE){
	  
			scc_kill(node_num, SIG_APPOINT_WORK, head);
			
	}else if (head->type == REMOVE_APP){
	  
			scc_kill(node_num, SIG_FINISH, head);
			
	}else if (head->type == INIT_APP){
	  
			scc_kill(node_num, SIG_INIT_APP, head);
			
	}else if (head->type == REP_AGENT_REQ_CORES){
	  
			scc_kill(node_num, SIG_REQ_CORES, head);
			
	}else if (head->type == INIT_AGENT){
	  
			scc_kill(node_num, SIG_INIT_AGENT, head);
	
	}else if (head->type == APPOINT_WORK_NODE_PENDING){

			fprintf(log_file,"\nI have unpredictable interaction with node %d with interaction = %d\n",node_num,head->type);
			
			tmp_inter_list = core_inter_head[node_num];
			core_inter_head[node_num] = core_inter_head[node_num]->next;
			
			if (core_inter_head[node_num] == NULL){
				core_inter_tail[node_num] = NULL;
			}else{
				send_next_signal(core_inter_head[node_num], node_num);
			}
			free(tmp_inter_list);
			
	}else if (head->type == DECLARE_INIT_AVAILABILITY) {
		scc_kill(node_num, SIG_INIT_APP,head);
		tmp_inter_list = core_inter_head[node_num];
		core_inter_head[node_num] = core_inter_head[node_num]->next;
		if (core_inter_head[node_num] == NULL) core_inter_tail[node_num] = NULL;
		else send_next_signal(core_inter_head[node_num], node_num);
		free(tmp_inter_list);
	}
	my_stats.msg_count++;
	my_stats.distance += distance(node_id,node_num);
	
	/* signals_enable(); */ /* Commented out on 5.5.2017 */
}

void sig_TERMINATE_handler(int sender_id){

	inter_list *tmp_inter_list;
	int data_array_local[LINE_SIZE];
	#ifdef PLAT_SCC	
	int error, str_len;
	char error_str[64];
	#endif
	
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "\n\n[%d:%d:%d]: I entered sig_TERMINATE_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	
	if (my_idag != -1 && core_inter_head[sender_id] != NULL && core_inter_head[sender_id]->type != TERMINATION_STATS) 
		while (core_inter_head[sender_id] != NULL && core_inter_head[sender_id]->type != TERMINATION_STATS){ 
			fprintf(log_file,"\t\tI am still doing smth with my agent %d interaction = %d\n",sender_id,core_inter_head[sender_id]->type);	

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
			scc_kill(sender_id, SIG_TERMINATE, core_inter_tail[sender_id]);
		} else 
			state = IDAG_ENDING;
	
	} else if (core_inter_head[sender_id]->type == TERMINATION_STATS) {
		#ifdef PLAT_SCC
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);
		scc_kill(sender_id, SIG_ACK, core_inter_tail[sender_id]);
		RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);

		error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), LINE_SIZE * sizeof(int), node_id);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			printf("I am %d and i got an error in get data in sig_TERMINATE_handler from %d with descr %s\n",node_id,sender_id,error_str);
		}
		#else
		scc_kill(sender_id, SIG_ACK, core_inter_tail[sender_id]);
		
		sem_wait(&flag_data_written[node_id]);
		//fprintf(log_file,"I acquire flag_data_written lock %d\n", node_id);

		my_RCCE_get(&data_array_local[0], &data_array[0], LINE_SIZE, node_id);	
		#endif

		total_stats.msg_count += data_array_local[0];//some_stats.msg_count;
		total_stats.message_size += data_array_local[1];//some_stats.message_size;
		total_stats.distance += data_array_local[2];//some_stats.distance; 
		total_stats.app_turnaround += data_array_local[3];//some_stats.app_turnaround;
		total_stats.comp_effort += data_array_local[4];//some_stats.comp_effort;
		total_stats.cores_utilized += data_array_local[5];//some_stats.cores_utilized;
		total_stats.times_accessed += data_array_local[6];//some_stats.times_accessed;
		
		stats_replied++;
		fprintf(log_file,"\t\tI am %d and my node %d replied stats stats_replied = %d my_cores_count = %d msg_count=%d\n",node_id,sender_id,stats_replied,my_cores_count,data_array_local[0]);	

		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		free(tmp_inter_list);
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else printf("I am %d in sig_terminate and after free i am still doing smth with my agent interaction = %d\n",node_id,core_inter_head[sender_id]->type);
	} else printf("I am %d in sig_terminate and i am still doing smth with my agent %d interaction = %d\n",node_id,sender_id,core_inter_head[sender_id]->type);

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_TERMINATE_handler with sender = %d state=%s\n\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));

	return;
}

void sig_INIT_APP_handler(int sender_id){
	inter_list *tmp_inter_list, *tmp_inter_prev;	
	#ifdef PLAT_SCC
	int error, str_len;
	char error_str[64];
	#endif	

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "\n\n[%d:%d:%d]: I entered sig_INIT_APP_handler with sender=%d state=%s pending_state=%s my_cores_count=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state),id2string(pending_state),my_cores_count);
	
	if (node_id == idag_id_arr[0]) {
		tmp_inter_prev = NULL;
		for (tmp_inter_list = init_pending_head; tmp_inter_list != NULL; tmp_inter_list = tmp_inter_list->next) {
			if (tmp_inter_list->data.new_app.num_of_cores == sender_id) break;
			tmp_inter_prev = tmp_inter_list;
		}
				
		if (tmp_inter_list != NULL) {
			fprintf(log_file,"\t\tI am sending an aborted init_app with id %d\n",tmp_inter_list->data.new_app.id);

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
				scc_kill(sender_id, SIG_INIT_APP, core_inter_head[sender_id]); 
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,sender_id);
			}

			if (tmp_inter_prev == NULL) {
				init_pending_head = init_pending_head->next;
			} else {
				tmp_inter_prev->next = tmp_inter_list->next;
				if (tmp_inter_prev->next == NULL) init_pending_tail = tmp_inter_prev;
			}

			free(tmp_inter_list);
		}
	} else if (state == INIT_MANAGER || state == INIT_MANAGER_SEND_OFFERS || state == IDLE_INIT_MAN || state == INIT_MAN_CHK_OFFERS || pending_state == INIT_MANAGER 
		|| pending_state == INIT_MANAGER_SEND_OFFERS || pending_state == INIT_MAN_CHK_OFFERS || pending_state == AGENT_INIT_CHK_OFFERS || pending_state == IDLE_INIT_MAN
		|| pending_state == IDLE_INIT_IDLE_AGENT || pending_state == IDLE_INIT_AGENT_SELFOPT || pending_state == INIT_CHK_OFFERS_IDLE_AGENT || pending_state == INIT_CHK_OFFERS_SELFOPT) {
		fprintf(log_file,"\t\tI have to reject sig_INIT_APP sender_id=%d\n",sender_id);

		scc_kill(sender_id, SIG_REJECT, NULL);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	} else if (sender_id == my_idag && core_inter_head[sender_id] != NULL && (core_inter_head[sender_id]->type == IDAG_ADD_CORES_DDS || core_inter_head[sender_id]->type == IDAG_REM_CORES_DDS || core_inter_head[sender_id]->type == REMOVE_APP)) {
		fprintf(log_file,"\t\tI have to reject sig_INIT_APP to prevent deadlock\n");

		if (core_inter_head[sender_id]->type == REMOVE_APP) {
			if (core_inter_head[sender_id] == NULL){
				core_inter_head[sender_id] = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[sender_id] = core_inter_head[sender_id];
			} else {
				core_inter_tail[sender_id]->next = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[sender_id] = core_inter_tail[sender_id]->next;
			}

			core_inter_tail[sender_id]->type = DECLARE_INIT_AVAILABILITY;
			core_inter_tail[sender_id]->next = NULL;
		}

		scc_kill(sender_id, SIG_REJECT, NULL);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	} else {
		if (core_inter_head[sender_id] != NULL) {
			fprintf(log_file, "\t\tI have interaction %d\n",core_inter_head[sender_id]->type);
		}

		scc_kill(sender_id, SIG_ACK, NULL);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
				
		init_app.id = sig_read_ar[2];
		init_app.app_type = sig_read_ar[3];		
		init_app.workld = sig_read_ar[4];
		init_app.num_of_cores = sig_read_ar[5];
#ifndef ARTIFICIAL_APPS_SIM		
		init_app.array_size = sig_read_ar[6];
		fprintf(log_file,"\t\tMy app is id = %d, array_size = %d, cores = %d, app_type = %d\n",init_app.id,init_app.array_size,init_app.num_of_cores,init_app.app_type);
#else
		memcpy(&init_app.var,&sig_read_ar[6],sizeof(int));
		memcpy(&init_app.A,&sig_read_ar[7],sizeof(int));

		/* printing order maintained for python scripts compatibility */
		fprintf(log_file,"\t\tMy app is id = %d, var = %f, cores = %d, app_type = %d, A = %f\n",init_app.id,init_app.var,init_app.num_of_cores,init_app.app_type,init_app.A);
#endif

		gettimeofday(&time_val, NULL);
		cur_t = localtime(&time_val.tv_sec);
		init_app_times[0].tm_sec = cur_t->tm_sec;
		init_app_times[0].tm_min = cur_t->tm_min;
		init_app_times[0].tm_hour = cur_t->tm_hour;
		init_app_times[0].tm_usec = time_val.tv_usec;

		if (state == IDLE_AGENT_WAITING_OFF || state == AGENT_SELF_CHK_OFFERS || state == AGENT_ZOMBIE || state == AGENT_ENDING || state == AGENT_INIT_STATE) 
			pending_state = INIT_MANAGER;
		else if (state == IDLE_AGENT) {
			if (my_cores_count > 1) {

				if (selfopt_time_rem != -1) {
					selfopt_time_rem = my_gettimer();
					my_settimer(0);
				}	 
				pending_state = IDLE_AGENT;
				state = INIT_MANAGER;
			} else pending_state = INIT_MANAGER;
		} else if (pending_state == AGENT_INIT_STATE) {
			pending_state = AGENT_INIT_APP_INIT;
		} else if (state == WORKING_NODE) {
			pending_state = state;
			state = INIT_MANAGER;
		} else if (state == AGENT_SELF_OPT) {
			if (my_cores_count > 1) {
      	pending_state = state;
      	state = INIT_MANAGER;
			} else pending_state = INIT_MANAGER;
		} else state = INIT_MANAGER;
	}

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_INIT_APP_handler with sender = %d || state = %s || pending_state = %s\n\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state),id2string(pending_state));

	return;
}

void sig_TIMER_handler(int signo, siginfo_t *info, void *context){
	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "\n\n[%d:%d:%d]: Alarm went off || state = %s || pending_state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,id2string(state),id2string(pending_state));

	if (state == IDLE_INIT_MAN) state = INIT_MAN_CHK_OFFERS;
	else if (state == IDLE_AGENT_WAITING_OFF) state = AGENT_SELF_CHK_OFFERS;
	else if (state == IDLE_AGENT) state = AGENT_SELF_OPT;
	else if (state == IDLE_CHK_APP_FILE) state = CHK_APP_FILE;
	else if (state == AGENT_INIT_STATE) {
		if (pending_state == IDLE_INIT_MAN) pending_state = INIT_MAN_CHK_OFFERS;
	}
	else if (state == AGENT_ENDING) pending_state = INIT_MAN_CHK_OFFERS;
	else if (state == AGENT_ZOMBIE) {
		state = INIT_MAN_CHK_OFFERS;
		pending_state = AGENT_ZOMBIE;
	}
	else if (node_id == idag_id_arr[0] && state == IDLE_IDAG)
		state = IDLE_IDAG_INIT_SEND;
	else if (state == WORKING_NODE) {
		if (pending_state == IDLE_INIT_MAN)
			pending_state = INIT_MAN_CHK_OFFERS;
		else if (pending_state == AGENT_INIT_IDLE_INIT)
			pending_state = AGENT_INIT_CHK_OFFERS;
		else if (pending_state == IDLE_INIT_AGENT_SELFOPT)
			pending_state = INIT_CHK_OFFERS_SELFOPT;
		else if (pending_state == IDLE_INIT_IDLE_AGENT)
			pending_state = INIT_CHK_OFFERS_IDLE_AGENT;
		else			
			printf("\n\ni am %d, timer went off and state working node. pending_agent is %s\n\n",node_id,id2string(pending_state));
	} else if (pending_state == AGENT_INIT_IDLE_INIT)
		pending_state = AGENT_INIT_CHK_OFFERS;	
	else printf("\n\ni am %d, timer went off and i don't know what to do. My state is %s\n\n",node_id,id2string(state));
	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: End of timer handler || state = %s || pending_state = %s\n\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,id2string(state),id2string(pending_state));

	return;
}

void sig_INIT_handler(int sender_id){
	
	int data_array_local[LINE_SIZE];
	int init_delay_sec;
	#ifdef PLAT_SCC
	int error, str_len;
	char error_str[64];
	#endif

	
	#ifdef PLAT_SCC
	RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);		
	scc_kill(sender_id, SIG_ACK, NULL);
	RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);

	error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), LINE_SIZE * sizeof(int), node_id);
	if (error != RCCE_SUCCESS) {
		RCCE_error_string(error, error_str, &str_len);
		printf("I am %d and i got an error in get data in sig_INIT from %d with descr %s\n",node_id,sender_id,error_str);
	} 
	#else

		scc_kill(sender_id, SIG_ACK, NULL);
		sem_wait(&flag_data_written[node_id]);

		my_RCCE_get(&data_array_local[0], &data_array[0], LINE_SIZE, node_id);
	#endif
	/* Initialize my timers */
	#if defined(EPFD) || defined(tEPFD)
		sev.sigev_notify = SIGEV_SIGNAL;
		sev.sigev_signo = SIG_EPFD_TIMER;
		sev.sigev_value.sival_ptr = &epfd_timer;
		if (timer_create(CLOCK_REALTIME, &sev, &epfd_timer) == -1){
			fprintf("Unable to create my timer!\n");
			printf("timer_create error\n");
		}else{
			fprintf(log_file,"I succesfully created epfd_timer\n");
		}
	#endif
		
	#if defined(PFD) || defined(tPFD)
		sev.sigev_notify = SIGEV_SIGNAL;
		sev.sigev_signo = SIG_PFD_TIMER;
		sev.sigev_value.sival_ptr = &pfd_timer;
		if (timer_create(CLOCK_REALTIME, &sev, &pfd_timer) == -1){
			fprintf(log_file,"Unable to create my timer!\n");
			printf("timer_create error\n");
		}else{
			fprintf(log_file,"I succesfully created pfd_timer\n");
		}
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
	if (timer_settime(pfd_timer, 0, &its, NULL) == -1){
		fprintf(log_file,"Unable to set timer\n");
		perror("timer_settime error9");
	}
	#endif
	/****************************/
	my_idag = data_array_local[0];

	my_stats.msg_count++; //gia to sig_ACK
	my_stats.distance += distance(node_id,my_idag);	

	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: my idle agent is %d and my pid is %d\n\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,my_idag,getpid());

	return;
}

/* Finalize max length of signals:
 * SIG_ADD_CORES_DDS : sender + signal + orig_sender + core_cnt + 8 cores = 13 ints
 * SIG_REM_CORES_DDS : sender + signal + orig_sender + new_owner + core_cnt + 8 cores = 14 ints
 * SIG_FINISH : sender + signal + orig_sender + core_cnt + 8 cores = 13 ints
 */
void sig_ACK_handler(int sender_id){
	
	int clear=1, i, j, data_array_local[6 * LINE_SIZE];//
	inter_list *tmp_inter_list;
	DDS_list *tmp_DDS;//, *tmp_inter_prev=NULL;
	struct timeval time_val_ack;
	struct tm *cur_t_ack;
	#ifdef PLAT_SCC
	int error, str_len;
	char error_str[64];
	#else
	int mem_offset=0;
	#endif

	
 	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "\n\n[%d:%d:%d]: I entered sig_ACK_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));

	/* FIXME data_array_local init ??? */

	if (core_inter_head[sender_id] == NULL) {
		fprintf(log_file,"\t\tInteraction inside sig_ACK is NULL! sender_id = %d\n",sender_id);
	} else {
		
		fprintf(log_file, "\t\tType=%s\n",inter2string(core_inter_head[sender_id]->type));
				
		tmp_inter_list = core_inter_head[sender_id];

		if (tmp_inter_list->type == INIT_CORE) { 
			data_array_local[0] = node_id;
			#ifdef PLAT_SCC
			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), LINE_SIZE * sizeof(int), sender_id);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				printf("I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
			}	

			RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
			#else
			mem_offset = sender_id * MAX_DATA_LIST_LEN * LINE_SIZE;
			for (i=0; i<LINE_SIZE; i++)
				 data_array[mem_offset + i] = data_array_local[i];

			sem_post(&flag_data_written[sender_id]);
			//fprintf(log_file,"I free flag_data_written lock %d\n", sender_id);
			#endif
	
			fprintf(log_file, "\t\tnode_id=%d\n",node_id);
			my_stats.message_size += sizeof(int);
			nodes_initialised++;
			fprintf(log_file, "\t\tnodes_initialised=%d\n",nodes_initialised);
		} else if (tmp_inter_list->type == INIT_APP) {
			gettimeofday(&time_val_ack, NULL);
			cur_t_ack = localtime(&time_val_ack.tv_sec);
			fprintf(init_ack_file, "%d:%d:%d:%ld %d\n",cur_t_ack->tm_hour,cur_t_ack->tm_min,cur_t_ack->tm_sec,time_val_ack.tv_usec,tmp_inter_list->data.new_app.id);
			my_stats.message_size += sizeof(app);
		} else if (tmp_inter_list->type == IDAG_FIND_IDAGS_PENDING || tmp_inter_list->type == SELFOPT_IDAG_FIND_IDAGS_PENDING) {//I am the requesting common node
			if (tmp_inter_list->type == IDAG_FIND_IDAGS_PENDING) {
				tmp_inter_list->type = IDAG_FIND_IDAGS;
			} else if (tmp_inter_list->type == SELFOPT_IDAG_FIND_IDAGS_PENDING) {
				tmp_inter_list->type = SELFOPT_IDAG_FIND_IDAGS;
			}

			my_stats.message_size += sizeof(region);
			clear = 0;
		} else if (core_inter_head[sender_id]->type == IDAG_REQ_DDS_PENDING || core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS_PENDING || core_inter_head[sender_id]->type == DEBUG_IDAG_REQ_DDS){
			my_stats.message_size += sizeof(region);

			if (core_inter_head[sender_id]->type == IDAG_REQ_DDS_PENDING) {
				core_inter_head[sender_id]->type = IDAG_REQ_DDS;
		 	} else if (core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS_PENDING) {
				core_inter_head[sender_id]->type = SELFOPT_IDAG_REQ_DDS;
			}

			clear = 0;
		} else if (tmp_inter_list->type == REP_IDAG_FIND_IDAGS) {//I am the idag
			fprintf(log_file, "\t\tnum_of_idags=%d\n",tmp_inter_list->data.idags_in_reg[num_idags]);
			my_stats.message_size += sizeof(int);
			my_stats.message_size += tmp_inter_list->data.idags_in_reg[num_idags]*sizeof(int);
		} else if (tmp_inter_list->type == REP_IDAG_REQ_DDS) {//I am the idag
			if (tmp_inter_list->data.agents_in_reg == NULL) {//debugging
				fprintf(log_file, "\t\tIn null rep_idag_dds with sender %d and DDS_count %d\n",sender_id,DDS_count);

				i=0;
				tmp_DDS = DDS;			
				while(tmp_DDS != NULL) {				
					data_array_local[i++] = tmp_DDS->agent_id;
					data_array_local[i++] = tmp_DDS->num_of_cores;
					tmp_DDS = tmp_DDS->next;
				}
		
				j = (2 * DDS_count) / LINE_SIZE;
				if ((2 * DDS_count) % LINE_SIZE != 0)
					j++;	
				
				#ifdef PLAT_SCC
				error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), j * LINE_SIZE * sizeof(int), sender_id);
				if (error != RCCE_SUCCESS) {
					RCCE_error_string(error, error_str, &str_len);
					fprintf(log_file,"I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
					fflush(log_file);
				}
				RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
				#else
				mem_offset = sender_id * MAX_DATA_LIST_LEN * LINE_SIZE;
				for (i=0; i<j *LINE_SIZE; i++)
					 data_array[mem_offset + i] = data_array_local[i];

				sem_post(&flag_data_written[sender_id]);
				//fprintf(log_file,"I free flag_data_written lock %d\n", sender_id);
				#endif

			} else {
				fprintf(log_file, "\t\tnum_of_agents=%d\n",tmp_inter_list->data.agents_in_reg[0]);
				my_stats.message_size += sizeof(int);

				j=0;
				for (i=1; i<=2*tmp_inter_list->data.agents_in_reg[0]; i+=2){
					fprintf(log_file, "\t\tagent=%d cores=%d\n",tmp_inter_list->data.agents_in_reg[i],tmp_inter_list->data.agents_in_reg[i+1]);
					data_array_local[j++] = tmp_inter_list->data.agents_in_reg[i];
					data_array_local[j++] = tmp_inter_list->data.agents_in_reg[i+1];
					my_stats.message_size += 2 * sizeof(int);
				}
			
				j = (2 * tmp_inter_list->data.agents_in_reg[0]) / LINE_SIZE;
				if ((2 * tmp_inter_list->data.agents_in_reg[0]) % LINE_SIZE != 0)
					j++;
				
				#ifdef PLAT_SCC		
				error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), j * LINE_SIZE * sizeof(int), sender_id);
				if (error != RCCE_SUCCESS) {
					RCCE_error_string(error, error_str, &str_len);
					fprintf(log_file,"I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
					fflush(log_file);
				}
				RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
				#else
				mem_offset = sender_id * MAX_DATA_LIST_LEN * LINE_SIZE;
				for (i=0; i<j * LINE_SIZE; i++)
					 data_array[mem_offset + i] = data_array_local[i];

				sem_post(&flag_data_written[sender_id]);
				//fprintf(log_file,"I free flag_data_written lock %d\n", sender_id);
				#endif
			}		
		} else if (tmp_inter_list->type == AGENT_REQ_CORES_PENDING) {
			tmp_inter_list->type = AGENT_REQ_CORES;

			/* Was added to avoid increasing counter inside signal handler, because it is not the same when asking for an offer or when getting a reply */
			data_array_local[0] = tmp_inter_list->data.reg_arr.region_arr[0].C;
			data_array_local[1] = tmp_inter_list->data.reg_arr.region_arr[0].r;
			#ifdef PLAT_SCC
			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), LINE_SIZE * sizeof(int), sender_id);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				printf("I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
			}	

			RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
			#else
			mem_offset = sender_id * MAX_DATA_LIST_LEN * LINE_SIZE;
			for (i=0; i<LINE_SIZE; i++)
				 data_array[mem_offset + i] = data_array_local[i];

			sem_post(&flag_data_written[sender_id]);
			//fprintf(log_file,"I free flag_data_written lock %d\n", sender_id);
			#endif

			free(tmp_inter_list->data.reg_arr.region_arr);
			my_stats.message_size += sizeof(region);
			my_stats.message_size += sizeof(app);
			clear = 0;
		} else if (tmp_inter_list->type == SELFOPT_REQ_CORES_PENDING) {
			my_stats.message_size += sizeof(app);
			my_stats.message_size += sizeof(region);

			 /* Was added to avoid increasing counter inside signal handler, because it is not the same when asking for an offer or when getting a reply */
			data_array_local[0] = tmp_inter_list->data.reg_arr.region_arr[0].C;
			data_array_local[1] = tmp_inter_list->data.reg_arr.region_arr[0].r;
			#ifdef PLAT_SCC
			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), LINE_SIZE * sizeof(int), sender_id);
			if (error != RCCE_SUCCESS) {
							RCCE_error_string(error, error_str, &str_len);
							printf("I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
			}       

			RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
			#else
			mem_offset = sender_id * MAX_DATA_LIST_LEN * LINE_SIZE;
			for (i=0; i<LINE_SIZE; i++)
								data_array[mem_offset + i] = data_array_local[i];

			sem_post(&flag_data_written[sender_id]);
			//fprintf(log_file,"I free flag_data_written lock %d\n", sender_id);
			#endif

			tmp_inter_list->type = SELFOPT_REQ_CORES;
			free(tmp_inter_list->data.reg_arr.region_arr);
			clear = 0;
		} else if (tmp_inter_list->type == REP_AGENT_REQ_CORES) {//I am the agent
			my_stats.message_size += sizeof(int);	

			if (tmp_inter_list->data.off_arr.num_of_offers > 0) {		
				for (j=0; j<tmp_inter_list->data.off_arr.num_of_offers; j++){
					my_stats.message_size += sizeof(int);
					my_stats.message_size += sizeof(float);
					for (i=0; i<tmp_inter_list->data.off_arr.offer_arr[j].num_of_cores; i++) {
						//data_array_local[i+LINE_SIZE] = tmp_inter_list->data.off_arr.offer_arr[j].offered_cores[i];
						data_array_local[i] = tmp_inter_list->data.off_arr.offer_arr[j].offered_cores[i];
						fprintf(log_file, "\t\tcore=%d\n",tmp_inter_list->data.off_arr.offer_arr[j].offered_cores[i]);
						my_stats.message_size += sizeof(int);
					}
				}

				core_inter_head[sender_id]->type = AGENT_OFFER_SENT;
				clear = 0;
			
				//fprintf(log_file,"Cores: ");
				//for (i=0; i<LINE_SIZE; i++)
				//        fprintf(log_file,"%d, ",data_array_local[i]);
				//fprintf(log_file,"\n");

				#ifdef PLAT_SCC
				error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), LINE_SIZE * sizeof(int), sender_id);
				if (error != RCCE_SUCCESS) {
					RCCE_error_string(error, error_str, &str_len);
					fprintf(log_file,"I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
					fflush(log_file);
				}
				RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
				#else
				mem_offset = sender_id * MAX_DATA_LIST_LEN * LINE_SIZE;
				for (i=0; i<LINE_SIZE; i++)
					 data_array[mem_offset + i] = data_array_local[i];

				sem_post(&flag_data_written[sender_id]);
				//fprintf(log_file,"I free flag_data_written lock %d\n", sender_id);
				#endif	
			}
		} else if (tmp_inter_list->type == REP_AGENT_OFFER_SENT) {
			my_stats.message_size += sizeof(int);
			free(tmp_inter_list->data.offer_acc_array);	
		} else if (tmp_inter_list->type == INIT_AGENT) {
			my_stats.message_size += sizeof(app);
			
			for (i=0; i<tmp_inter_list->data.one_app.new_app.num_of_cores; i++){
				data_array_local[0*LINE_SIZE+i] = tmp_inter_list->data.one_app.new_app_cores[i];

				fprintf(log_file, "\t\tcore=%d\n",tmp_inter_list->data.one_app.new_app_cores[i]);
				my_stats.message_size += sizeof(int);
			}
			data_array_local[1*LINE_SIZE] = tmp_inter_list->data.one_app.new_app_times[0].tm_sec;
			data_array_local[1*LINE_SIZE+1] = tmp_inter_list->data.one_app.new_app_times[0].tm_min;
			data_array_local[1*LINE_SIZE+2] = tmp_inter_list->data.one_app.new_app_times[0].tm_hour;
			data_array_local[1*LINE_SIZE+3] = tmp_inter_list->data.one_app.new_app_times[0].tm_usec;
			data_array_local[1*LINE_SIZE+4] = tmp_inter_list->data.one_app.new_app_times[1].tm_sec;
			data_array_local[1*LINE_SIZE+5] = tmp_inter_list->data.one_app.new_app_times[1].tm_min;
			data_array_local[1*LINE_SIZE+6] = tmp_inter_list->data.one_app.new_app_times[1].tm_hour;
			data_array_local[1*LINE_SIZE+7] = tmp_inter_list->data.one_app.new_app_times[1].tm_usec;

			fprintf(log_file, "\t\tsec=%d min=%d hours=%d\n",init_app_times[0].tm_sec,init_app_times[0].tm_min,init_app_times[0].tm_hour);
			fprintf(log_file, "\t\tsec=%d min=%d hours=%d\n",init_app_times[1].tm_sec,init_app_times[1].tm_min,init_app_times[1].tm_hour);

			#ifdef PLAT_SCC
			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]),2 * LINE_SIZE * sizeof(int), sender_id);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				fprintf(log_file,"I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
				fflush(log_file);
			}
			RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
			#else
			mem_offset = sender_id * MAX_DATA_LIST_LEN * LINE_SIZE;
			for (i=0; i<2 * LINE_SIZE; i++)
				 data_array[mem_offset + i] = data_array_local[i];

			sem_post(&flag_data_written[sender_id]);
			//fprintf(log_file,"I free flag_data_written lock %d\n",sender_id);
			#endif
			
			if (init_app.id == tmp_inter_list->data.one_app.new_app.id) {
				init_app.num_of_cores=-1;
				init_app_times[0].tm_sec = 0;
				init_app_times[0].tm_min = 0;
				init_app_times[0].tm_hour = 0;
				init_app_times[0].tm_usec = 0;
				init_app_times[1].tm_sec = 0;
				init_app_times[1].tm_min = 0;
				init_app_times[1].tm_hour = 0;
				init_app_times[1].tm_usec = 0;
			} else {
				fprintf(log_file,"Got that!!!! init = %d old = %d\n",init_app.id,tmp_inter_list->data.one_app.new_app.id);
				fflush(log_file);
			}

			/* Commented out by billtsou because target_ID was -1 */  
			/*scc_kill(idag_id_arr[0], SIG_INIT_APP, NULL);*/
			scc_kill(0, SIG_INIT_APP, NULL);
			my_stats.msg_count++;
			my_stats.distance += distance(node_id,idag_id_arr[0]);
		} else if (tmp_inter_list->type == IDAG_ADD_CORES_DDS) {
			fprintf(log_file, "\t\tapp_cores=%d\n",tmp_inter_list->data.app_cores[0]);
			my_stats.message_size += sizeof(int);
			
			for (i=1; i<=tmp_inter_list->data.app_cores[0]; i++){
				fprintf(log_file, "\t\tcore=%d\n",tmp_inter_list->data.app_cores[i]);
				my_stats.message_size += sizeof(int);
			}
			//I am an idag and i have to send to other idags my original sender
			if (my_idag == -1) {//idag_id != -1
				fprintf(log_file, "\t\torig_sender=%d\n",tmp_inter_list->data.app_cores[i]);
				my_stats.message_size += sizeof(int);
			}

		} else if (tmp_inter_list->type == IDAG_REM_CORES_DDS) {
			fprintf(log_file, "\t\tapp_cores=%d\n",tmp_inter_list->data.app_cores[0]);
			my_stats.message_size += sizeof(int);
			
			for (i=1; i<=tmp_inter_list->data.app_cores[0]; i++){			
				fprintf(log_file, "\t\tcore=%d\n",tmp_inter_list->data.app_cores[i]);
				my_stats.message_size += sizeof(int);
			}
			
			//I am an idag and i have to send to other idags my original sender
			if (my_idag == -1) {//idag_id != -1
				fprintf(log_file, "\t\torig_sender=%d\n",tmp_inter_list->data.app_cores[i]);
				fprintf(log_file, "\t\tnew_owner=%d\n",tmp_inter_list->data.app_cores[i+1]);
				my_stats.message_size += 2 * sizeof(int);
			} else {//idag_id != -1
				fprintf(log_file, "\t\tnew_owner=%d\n",tmp_inter_list->data.app_cores[i]);
				my_stats.message_size += sizeof(int);
			}
		
		} else if (tmp_inter_list->type == REMOVE_APP) {
			fprintf(log_file, "\t\tapp_cores=%d\n",tmp_inter_list->data.app_cores[0]);
			my_stats.message_size += (tmp_inter_list->data.app_cores[0] + 1) * sizeof(int); //cores plus num of cores
			
			if (my_idag != -1) {			
				if (tmp_inter_list->data.app_cores[0] > 5) {
					for (i=0; i<(tmp_inter_list->data.app_cores[0] - 5); i++){			
						fprintf(log_file, "\t\tcore=%d\n",tmp_inter_list->data.app_cores[i+6]);
					}
				}
			} else {
				//I am an idag and i have to send to other idags my original sender
				my_stats.message_size += sizeof(int); //for original sender
				if (tmp_inter_list->data.app_cores[0] > 4) {
					for (i=0; i<(tmp_inter_list->data.app_cores[0]-4); i++){			
						fprintf(log_file, "\t\tcore=%d\n",tmp_inter_list->data.app_cores[i+6]);
					}
				}
			}	
		} else if (tmp_inter_list->type == INIT_WORK_NODE) {
			if (idag_mask[sender_id] != sender_id){
				if (tmp_inter_list->data.work_bounds[0] != -1) {
					gettimeofday(&time_val, NULL);
					cur_t = localtime(&time_val.tv_sec);
					fprintf(app_log_file,"[%d:%d:%d:%ld] I init work to %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,time_val.tv_usec,sender_id);
				}
				my_stats.message_size += 5 * sizeof(int);
			}
		} else if (tmp_inter_list->type == APPOINT_WORK_NODE) {
			if (idag_mask[sender_id] != sender_id){

				if (tmp_inter_list->data.work_bounds[0] != -1) {
					gettimeofday(&time_val, NULL);
					cur_t = localtime(&time_val.tv_sec);
					fprintf(app_log_file,"[%d:%d:%d:%ld] I appoint work to %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,time_val.tv_usec,sender_id);
				}
				my_stats.message_size += 3 * sizeof(int);
			}
		} else if (tmp_inter_list->type == REP_STATISTICS) {
			data_array_local[0] = tmp_inter_list->data.stats.msg_count;
			data_array_local[1] = tmp_inter_list->data.stats.message_size;
			data_array_local[2] = tmp_inter_list->data.stats.distance; 
			data_array_local[3] = tmp_inter_list->data.stats.app_turnaround;
			data_array_local[4] = tmp_inter_list->data.stats.comp_effort;
			data_array_local[5] = tmp_inter_list->data.stats.cores_utilized;
			data_array_local[6] = tmp_inter_list->data.stats.times_accessed;
			
			#ifdef PLAT_SCC
			error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), LINE_SIZE * sizeof(int), sender_id);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				printf("I am %d and i got an error in put data to %d with descr %s\n",node_id,sender_id,error_str);
			}	

			RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, sender_id);
			#else
			mem_offset = sender_id * MAX_DATA_LIST_LEN * LINE_SIZE;
			for (i=0; i<LINE_SIZE; i++)
				data_array[mem_offset + i] = data_array_local[i];

			sem_post(&flag_data_written[sender_id]);
			//fprintf(log_file,"I free flag_data_written lock %d\n",sender_id);
			#endif	

			state = TERMINATED;		
		} else printf("Unreachable point inside ACK! node_id = %d sender_id = %d\n",node_id,sender_id);

		if (clear){
			core_inter_head[sender_id] = tmp_inter_list->next;
			if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
			else send_next_signal(core_inter_head[sender_id],sender_id);
			free(tmp_inter_list);
		}
	} 
	
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_ACK_handler with sender = %d state = %s\n\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));

	return;
}

void sig_IDAG_FIND_IDAGS_handler(int sender_id, int *inc_cnt, int cur_index_top){
  
	int num_of_idags, i, one_idag;
	region cur_reg;
	inter_list *tmp_inter_list;
	#ifdef PLAT_SCC
	char error_str[64];	
	int error, str_len;
	#endif
	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "\n\n[%d:%d:%d]: I entered sig_IDAG_FIND_IDAGS_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
		
	if (core_inter_head[sender_id] == NULL || core_inter_head[sender_id]->type == REP_IDAG_FIND_IDAGS || core_inter_head[sender_id]->type == INIT_APP) {
		fprintf(log_file,"1st case\n");
		scc_kill(sender_id, SIG_ACK, NULL);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		cur_reg.C = sig_read_ar[2];
		cur_reg.r = sig_read_ar[3];
		
		if (core_inter_head[sender_id] == NULL){
			core_inter_head[sender_id] = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[sender_id] = core_inter_head[sender_id];
		} else {
			core_inter_tail[sender_id]->next = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[sender_id] = core_inter_tail[sender_id]->next;
		}

		core_inter_tail[sender_id]->type = REP_IDAG_FIND_IDAGS;
		core_inter_tail[sender_id]->data.idags_in_reg = (int *) malloc((num_idags+1)*sizeof(int));		
		core_inter_tail[sender_id]->next = NULL;
	
		get_reg_idags(cur_reg, core_inter_tail[sender_id]->data.idags_in_reg);
		fprintf(log_file,"Agents in region are %d: ",core_inter_tail[sender_id]->data.idags_in_reg[num_idags]);
		for (i=0; i<core_inter_tail[sender_id]->data.idags_in_reg[num_idags]; i++) {
			fprintf(log_file, "%d ",core_inter_tail[sender_id]->data.idags_in_reg[i]);
		}
		fprintf(log_file,"\n");
		
		if (core_inter_head[sender_id]->next == NULL) {
			scc_kill(sender_id, SIG_IDAG_FIND_IDAGS, core_inter_head[sender_id]);
			my_stats.msg_count++;
			my_stats.distance += distance(node_id,sender_id);
		}		
	} else if (core_inter_head[sender_id]->type == IDAG_FIND_IDAGS && state != IDLE_INIT_MAN && pending_state != IDLE_INIT_MAN 
		&& pending_state != AGENT_INIT_IDLE_INIT && pending_state != IDLE_INIT_IDLE_AGENT && pending_state != IDLE_INIT_AGENT_SELFOPT) {
		fprintf(log_file,"\t\tI am %d and i think i cought a stray init idag_find_idags reply from %d. My current state is %s\n",node_id,sender_id,id2string(state));

		scc_kill(sender_id, SIG_REJECT, NULL);
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
		fprintf(log_file,"\t\tI am %d and i think i cought a stray selfopt idag_find_idags reply from %d. My current state is %s\n",node_id,sender_id,id2string(state));

		scc_kill(sender_id, SIG_REJECT, NULL);
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
		fprintf(log_file,"4th case\n");
		scc_kill(sender_id, SIG_ACK, core_inter_head[sender_id]);
		num_of_idags = sig_read_ar[2];
		if (num_of_idags > 5){
			*inc_cnt = *inc_cnt + 1;
			
			#ifdef PLAT_SCC
				error = RCCE_get((t_vcharp)(&sig_read_ar[LINE_SIZE]), (t_vcharp)(&sig_array[(cur_index_top+1)*LINE_SIZE]), LINE_SIZE * sizeof(int), node_id);
				if (error != RCCE_SUCCESS) {
					RCCE_error_string(error, error_str, &str_len);
					fprintf(log_file,"I got an error in get data in sig_ADD_CORES_DDS_handler from %d with descr %s\n",sender_id,error_str);
					fflush(log_file);
				}
			#else
				new_RCCE_get(sig_read_ar, sig_array, cur_index_top, LINE_SIZE, node_id);
			#endif
		}
		fprintf(log_file,"Number of agents in region %d\n",num_of_idags);

		if (core_inter_head[sender_id]->type == IDAG_FIND_IDAGS) init_DDS_idags += num_of_idags;		
		else if (core_inter_head[sender_id]->type == SELFOPT_IDAG_FIND_IDAGS)	selfopt_DDS_idags += num_of_idags;
		
		for (i=0; i<num_of_idags; i++) { //max 8 idags
			one_idag = sig_read_ar[i+3];
			fprintf(log_file,"In the region I have idag with id %d\n",one_idag);

			if (core_inter_head[one_idag] == NULL){
				core_inter_head[one_idag] = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[one_idag] = core_inter_head[one_idag];
			} else {
				
				core_inter_tail[one_idag]->next = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[one_idag] = core_inter_tail[one_idag]->next;
			}

			if (core_inter_head[sender_id]->type == IDAG_FIND_IDAGS) core_inter_tail[one_idag]->type = IDAG_REQ_DDS_PENDING;
			else core_inter_tail[one_idag]->type = SELFOPT_IDAG_REQ_DDS_PENDING;
			
			core_inter_tail[one_idag]->data.reg.C = core_inter_head[sender_id]->data.reg.C;
			core_inter_tail[one_idag]->data.reg.r = core_inter_head[sender_id]->data.reg.r;
			core_inter_tail[one_idag]->next = NULL;

			if (core_inter_head[one_idag]->next == NULL) {
				scc_kill(one_idag, SIG_REQ_DDS, core_inter_head[one_idag]);				
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,one_idag);
			} else {
				fprintf(log_file,"I did not sent req_dds to %d with interaction = %s inter 2=%s\n",one_idag,inter2string(core_inter_head[one_idag]->type),inter2string(core_inter_head[one_idag]->next->type));
			}
		}
		
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);
		free(tmp_inter_list);
	} else if (core_inter_head[sender_id]->type == IDAG_FIND_IDAGS_PENDING || core_inter_head[sender_id]->type == SELFOPT_IDAG_FIND_IDAGS_PENDING) {
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
		fprintf(log_file,"I have to reject sig_IDAG_FIND_IDAGS_handler sender_id=%d interaction=%d\n",sender_id,core_inter_head[sender_id]->type);

		scc_kill(sender_id, SIG_REJECT, NULL);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	}

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_IDAG_FIND_IDAGS_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));

	return;
}

void sig_REQ_DDS_handler(int sender_id){
	
	int num_of_agents, i, agent_id, num_of_cores, j, agents_read=0, data_array_local[6 * LINE_SIZE];
	region cur_reg;
	core_list *tmp_cores_list;
	inter_list *tmp_inter_list;
	DDS_list *tmp_DDS;
	target_list *tmp_target_list;	
	#ifdef PLAT_SCC	
	int error, str_len;
	char error_str[64];	
	#endif
	#ifdef ADAM_SIM
	int cur_max_agent, cur_max_cores, cur_max_occurences, tmp_integer_matrix[NUES];	
	#endif
	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_REQ_DDS_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
	if (core_inter_head[sender_id] == NULL){ //I am the idag

		scc_kill(sender_id, SIG_ACK, NULL);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		cur_reg.C = sig_read_ar[2];
		cur_reg.r = sig_read_ar[3];
		
		fprintf(log_file,"I am to investigate region C=%d r=%d for %d\n",cur_reg.C,cur_reg.r,sender_id);		

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
			while (tmp_cores_list != NULL) {
				if (distance(tmp_cores_list->core_id, cur_reg.C) <= cur_reg.r) { /* Search inside specific region to identify Managers */
					if (tmp_cores_list->offered_to == -1) {
						agent_id=node_id;
					} else {
						agent_id=tmp_cores_list->offered_to;	
						for (tmp_DDS = DDS->next; tmp_DDS!=NULL; tmp_DDS=tmp_DDS->next) {
							if (tmp_DDS->agent_id == agent_id) {
								break;
							}
						}

						if (tmp_DDS == NULL) {
							agent_id = node_id;
						}
					}

					for (i=1; i<=2*DDS_count; i+=2) {
						if (core_inter_tail[sender_id]->data.agents_in_reg[i] == agent_id) break;
						else if (core_inter_tail[sender_id]->data.agents_in_reg[i] == -1) {
							core_inter_tail[sender_id]->data.agents_in_reg[0]++;
							core_inter_tail[sender_id]->data.agents_in_reg[i] = agent_id;
							break;
						}
					}
					core_inter_tail[sender_id]->data.agents_in_reg[i+1]++;
				}

				tmp_cores_list = tmp_cores_list->next;
			}
		} else core_inter_tail[sender_id]->data.agents_in_reg = NULL;
 
		#ifdef ADAM_SIM
		tmp_inter_list = core_inter_tail[sender_id];
		fprintf(log_file, "\t\tnum_of_agents=%d\n",tmp_inter_list->data.agents_in_reg[0]);
		
		cur_max_agent = -1;
		cur_max_cores = -1;
		cur_max_occurences = 0; /* Get all agents with maximum cores */
		
		for (i=1; i<=2*tmp_inter_list->data.agents_in_reg[0]; i+=2){
			fprintf(log_file, "\t\tagent=%d cores=%d\n",tmp_inter_list->data.agents_in_reg[i],tmp_inter_list->data.agents_in_reg[i+1]);
			 
			if (tmp_inter_list->data.agents_in_reg[i+1] > cur_max_cores) {
				cur_max_cores = tmp_inter_list->data.agents_in_reg[i+1];
				cur_max_agent = tmp_inter_list->data.agents_in_reg[i];
			  	cur_max_occurences = 1;
				tmp_integer_matrix[cur_max_occurences-1] = tmp_inter_list->data.agents_in_reg[i];
			} else if (tmp_inter_list->data.agents_in_reg[i+1] == cur_max_cores) {
				cur_max_occurences++;
				tmp_integer_matrix[cur_max_occurences-1] = tmp_inter_list->data.agents_in_reg[i];	
			} else {
			}
		}
	
		if (cur_max_occurences >= 1) {
			tmp_inter_list->data.agents_in_reg[0] = cur_max_occurences;
			for (i=0; i<cur_max_occurences; i++) {
				tmp_inter_list->data.agents_in_reg[2*i+1] = tmp_integer_matrix[i];
				tmp_inter_list->data.agents_in_reg[2*i+2] = cur_max_cores;			
			} 			
		} else {
			fprintf(log_file,"Error: cur_max_occurences = %d\n",cur_max_occurences);	
		}	
		#endif

		scc_kill(sender_id, SIG_REQ_DDS, core_inter_head[sender_id]);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	} else if (core_inter_head[sender_id]->type == IDAG_REQ_DDS && state != IDLE_INIT_MAN && pending_state != IDLE_INIT_MAN
		&& pending_state != AGENT_INIT_IDLE_INIT && pending_state != IDLE_INIT_IDLE_AGENT && pending_state != IDLE_INIT_AGENT_SELFOPT) {
		fprintf(log_file,"I am %d and i think i cought a stray init IDAG_REQ_DDS reply from %d. My current state is %s\n",node_id,sender_id,id2string(state));

		scc_kill(sender_id, SIG_REJECT, NULL);
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
		fprintf(log_file,"I am %d and i think i cought a stray selfopt SELFOPT_IDAG_REQ_DDS reply from %d. My current state is %s\n",node_id,sender_id,id2string(state));		

		scc_kill(sender_id, SIG_REJECT, NULL);
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
	} else if (core_inter_head[sender_id]->type == IDAG_REQ_DDS || core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS) { //I am the requesting common node
		
		if (core_inter_head[sender_id]->type == IDAG_REQ_DDS) {
			init_DDS_replies++;
			fprintf(log_file,"init_DDS_idags = %d, init_DDS_replies = %d\n",init_DDS_idags,init_DDS_replies);
		} else if (core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS) {
			selfopt_DDS_replies++;
			fprintf(log_file,"selfopt_DDS_idags = %d, selfopt_DDS_replies = %d\n",selfopt_DDS_idags,selfopt_DDS_replies);
		}

		num_of_agents = sig_read_ar[2];	
		if (num_of_agents > X_max*Y_max) {
			printf("I am %d and in req_dds i got shit num_of_agents %d from %d\n",node_id,num_of_agents,sender_id);	
			fflush(stdout);
		}
		
		fprintf(log_file, "Number of agents in region = %d of %d reg = (%d,%d)\n",num_of_agents,sender_id,core_inter_head[sender_id]->data.reg.C,core_inter_head[sender_id]->data.reg.r);

		i = (2 * num_of_agents) / LINE_SIZE;
		if ((2 * num_of_agents) % LINE_SIZE != 0)
			i++;

		#ifdef PLAT_SCC
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);
		scc_kill(sender_id, SIG_ACK, core_inter_head[sender_id]);
	
		RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
				
		//read(fd_r, &num_of_idags, sizeof(int));
		error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), i * LINE_SIZE * sizeof(int), node_id);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			printf("I am %d and i got an error in get data in sig_REQ_DDS_handler from %d with descr %s\n",node_id,sender_id,error_str);
		}
		#else
		scc_kill(sender_id, SIG_ACK, core_inter_head[sender_id]);
		/* FIXME */
		sem_wait(&flag_data_written[node_id]);
		//fprintf(log_file,"I acquire flag_data_written lock %d\n",node_id);

		my_RCCE_get(&data_array_local[0], &data_array[0], i * LINE_SIZE, node_id);	
		#endif
	
		for (i=0; i<2*num_of_agents; i+=2) {
			//read(fd_r, &agent_id, sizeof(int));
			//read(fd_r, &num_of_cores, sizeof(int));
			
			agent_id = data_array_local[i];
			num_of_cores = data_array_local[i+1];
			agents_read++;

			fprintf(log_file, "there is an agent with id %d and %d cores\n",agent_id,num_of_cores);
			if (agent_id == node_id) {
				continue;
			}
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
					
					for (j=0; j<tmp_target_list->num_of_regions; j++)
						if (tmp_target_list->region_arr[j].C == core_inter_head[sender_id]->data.reg.C && tmp_target_list->region_arr[j].r == core_inter_head[sender_id]->data.reg.r){
							fprintf(log_file, "Area allready exists\n");
							break;
						}
					
					if (j == tmp_target_list->num_of_regions) {							
						tmp_target_list->region_arr[tmp_target_list->num_of_regions++] = core_inter_head[sender_id]->data.reg;
					}
				}
			} else if (core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS) {
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
			}
		}
		
		if (core_inter_head[sender_id]->type == IDAG_REQ_DDS && init_DDS_replies == init_DDS_idags) {
			for (tmp_target_list = init_targets_head; tmp_target_list != NULL; tmp_target_list = tmp_target_list->next) {
				agent_id = tmp_target_list->target;
				
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
				core_inter_tail[agent_id]->next = NULL;

				if (core_inter_head[agent_id]->next == NULL) {
					//kill(pid_num[agent_id], SIG_REQ_CORES);
					scc_kill(agent_id, SIG_REQ_CORES, core_inter_head[agent_id]);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,agent_id);
				/*} else if (agent_id == sender_id && core_inter_head[agent_id]->next->type == AGENT_REQ_CORES_PENDING) { 
					fprintf(log_file, "I send here awkard head\n");
					fflush(log_file);
					scc_kill(agent_id, SIG_REQ_CORES, core_inter_head[agent_id]->next);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,agent_id);*/
				} else {
					fprintf(log_file,"Init interaction is not NULL interaction=%d interaction2=%d\n",core_inter_head[agent_id]->type,core_inter_head[agent_id]->next->type);
					fflush(log_file);
				}
			}

			fprintf(log_file,"Here i set init timer\n");
			my_settimer(INIT_NODE_INTERVAL);	 
		} else if (core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS && selfopt_DDS_replies == selfopt_DDS_idags) {
		  	//TODO dimos reset timer here
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
				core_inter_tail[agent_id]->next = NULL;

				if (core_inter_head[agent_id]->next == NULL) {
					scc_kill(agent_id, SIG_REQ_CORES, core_inter_head[agent_id]);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,agent_id);

				} else {
					fprintf(log_file,"Selfopt interaction is not NULL interaction=%d interaction2=%d\n",core_inter_head[agent_id]->type,core_inter_head[agent_id]->next->type);
					fflush(log_file);
         }
			}

			fprintf(log_file,"This is where i changed selfopt_time_rem\n");
			my_settimer(INIT_NODE_INTERVAL);
				
			if (state != IDLE_AGENT_WAITING_OFF) {
				fprintf(log_file,"I am about to set my alarm for selfopt check and my state before that was %s\n",id2string(state));
				state = IDLE_AGENT_WAITING_OFF;
			}
		}	
		
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) {
			core_inter_tail[sender_id] = NULL;
		} else {
			send_next_signal(core_inter_head[sender_id], sender_id);
		}
		
		free(tmp_inter_list);		
	} else if (core_inter_head[sender_id]->type == DEBUG_IDAG_REQ_DDS) { //I am the requesting common node
		//read(fd_r, &num_of_agents, sizeof(int));
		num_of_agents = sig_read_ar[2];	
		printf("\nNumber of agents in region = %d\n",num_of_agents);

		i = (2 * num_of_agents) / LINE_SIZE;
		if ((2 * num_of_agents) % LINE_SIZE != 0)
			i++;

		#ifdef PLAT_SCC
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);		
		scc_kill(sender_id, SIG_ACK, core_inter_head[sender_id]);
		RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);			

		error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), i * LINE_SIZE * sizeof(int), node_id);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			printf("I am %d and i got an error in get data in sig_REQ_DDS_handler from %d with descr %s\n",node_id,sender_id,error_str);
		}
		#else
		scc_kill(sender_id, SIG_ACK, core_inter_head[sender_id]);	
		sem_wait(&flag_data_written[node_id]);
		//fprintf(log_file,"I acquire flag_data_written lock %d\n", node_id);

		my_RCCE_get(&data_array_local[0], &data_array[0], i * LINE_SIZE, node_id);	
		#endif
 
		for (i=0; i<2*num_of_agents; i+=2) {
			agent_id = data_array_local[i];
			num_of_cores = data_array_local[i+1];
			printf("Agent no %d is %d with %d cores\n",agents_read,agent_id,num_of_cores);
			agents_read++;
		}
		
		idags_replied++;
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);		
		free(tmp_inter_list);
	} else if (core_inter_head[sender_id]->type == IDAG_REQ_DDS_PENDING || core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS_PENDING) {
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
		fprintf(log_file,"I am %d and i have to reject req_dds from %d. Our interaction is %d\n",node_id,sender_id,core_inter_head[sender_id]->type);

		scc_kill(sender_id, SIG_REJECT, NULL);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	}

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_REQ_DDS_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
	return;
}

void sig_REQ_CORES_handler(int sender_id, int *inc_cnt, int cur_index_top) {
	
	int i, tmp_int, num_of_offers, j, non_zero_offers=0, cores_util, data_array_local[2 * LINE_SIZE];
	float req_gain;
	region cur_reg;
	app req_app;
	offer one_offer;
	offer_list *tmp_offer_list, *tmp_offer_prev = NULL, *tmp_head, *chosen_node;
	inter_list *tmp_inter_list;	
	offer_array off_arr;
	DDS_list *tmp_DDS;
	#ifdef PLAT_SCC
	int error, str_len;
	char error_str[64];
	#endif

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_REQ_CORES_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
		
	if (core_inter_head[sender_id] == NULL && my_cores != NULL) {
		/* *inc_cnt = *inc_cnt + 1; */
		
		/* 12.7.2017 really major change compared to TECS 8x6 to add an extra readline in REQ_CORES for artificial apps */
		/* 31.7.2017 Change to data array in order to avoid inc_cnt */

		#ifdef PLAT_SCC
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);
		scc_kill(sender_id, SIG_ACK, core_inter_head[sender_id]);

		RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);		
						
		error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), LINE_SIZE * sizeof(int), node_id);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			printf("I am %d and i got an error in get data in sig_REQ_CORES_handler from %d with descr %s\n",node_id,sender_id,error_str);
		}
		 
		#else
		scc_kill(sender_id, SIG_ACK, core_inter_head[sender_id]);	
		sem_wait(&flag_data_written[node_id]);
		my_RCCE_get(&data_array_local[0], &data_array[0], LINE_SIZE, node_id);	
		#endif

		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		req_app.id = sig_read_ar[2];
		req_app.app_type = sig_read_ar[3];
		req_app.workld = sig_read_ar[4];
		req_app.num_of_cores = sig_read_ar[5];
#ifndef ARTIFICIAL_APPS_SIM
		req_app.array_size = sig_read_ar[6];

		fprintf(log_file,"id = %d size = %d workld = %d cores = %d\n",req_app.id,req_app.array_size,req_app.workld,req_app.num_of_cores);
#else
		memcpy(&req_app.var,&sig_read_ar[6],sizeof(int));
		memcpy(&req_app.A,&sig_read_ar[7],sizeof(int));

		/* FIXME printing queue maintainted for python scripts compatibility */
		fprintf(log_file,"id = %d var = %f workld = %d cores = %d A = %f\n",req_app.id,req_app.var,req_app.workld,req_app.num_of_cores,req_app.A);
#endif
		if (req_app.num_of_cores < 0) {
			fprintf(log_file,"Num of cores is %d. Changing to 0\n",req_app.num_of_cores);
			req_app.num_of_cores = 0;
		}

		if (core_inter_head[sender_id] == NULL) {
			core_inter_head[sender_id] = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[sender_id] = core_inter_head[sender_id];
		} else {
			core_inter_tail[sender_id]->next = (inter_list *) malloc(sizeof(inter_list));
			core_inter_tail[sender_id] = core_inter_tail[sender_id]->next;
		}

		core_inter_tail[sender_id]->type = REP_AGENT_REQ_CORES;
		
		off_arr.num_of_offers = 1;
		off_arr.offer_arr = (offer *) malloc(off_arr.num_of_offers * sizeof(offer));
		core_inter_tail[sender_id]->next = NULL;

		cur_reg.C = data_array_local[0];
		cur_reg.r = data_array_local[1];
		fprintf(log_file,"Searching in C = %d r = %d\n",cur_reg.C,cur_reg.r);
	
		i = 0;
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
		} else if (my_cores != NULL && my_cores_count > 2 && app_state != APP_TERMINATED) {
			off_arr.offer_arr[i].offered_cores = (int *) malloc(my_cores_count*sizeof(int));
		
			tmp_int = offer_cores(my_cores, req_app, cur_reg, off_arr.offer_arr[i].offered_cores, sender_id);
			off_arr.offer_arr[i].num_of_cores = tmp_int;
			my_stats.comp_effort++;
			
			off_arr.offer_arr[i].spd_loss = Speedup(my_app, my_cores_count) - Speedup(my_app, my_cores_count-tmp_int);
			req_gain = Speedup(req_app,req_app.num_of_cores+tmp_int) - Speedup(req_app,req_app.num_of_cores);
			if (tmp_int > 0) {
				non_zero_offers++;
				fprintf(log_file,"I offered %d cores with spd_loss = %0.2f and %0.2f gain for the req_app\n",tmp_int,off_arr.offer_arr[i].spd_loss,req_gain);
				if (app_log_file != NULL) { /* Added on 5.5.2017 */
					fprintf(app_log_file,"I offered %d cores with spd_loss = %0.2f and %0.2f gain for the req_app\n"
						,tmp_int,off_arr.offer_arr[i].spd_loss,req_gain);
					fflush(app_log_file);
				} else {
					fprintf(log_file,"App log file is NULL\n");
                                	fflush(log_file);
				}
			}
		} else {
			off_arr.offer_arr[i].num_of_cores = 0;
			off_arr.offer_arr[i].spd_loss = 0.0;
		}

		fprintf(log_file, "I offered %d %d cores: ",sender_id,off_arr.offer_arr[i].num_of_cores);
		for (j=0; j<off_arr.offer_arr[i].num_of_cores; j++) {
			fprintf(log_file, "%d, ",off_arr.offer_arr[i].offered_cores[j]);
		}
		fprintf(log_file, "\n");
		
		fprintf(log_file,"non_zero_offers = %d\n",non_zero_offers);
		core_inter_tail[sender_id]->data.off_arr.num_of_offers = non_zero_offers;
		if (non_zero_offers > 0) {
			core_inter_tail[sender_id]->data.off_arr.offer_arr = (offer *) malloc(non_zero_offers * sizeof(offer));
		} else {
			core_inter_tail[sender_id]->data.off_arr.offer_arr = NULL;
		}

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
			scc_kill(sender_id, SIG_REQ_CORES, core_inter_head[sender_id]);
			my_stats.msg_count++;
			my_stats.distance += distance(node_id,sender_id);
		} else {
			fprintf(log_file,"Apparently not null interaction=%d\n",core_inter_head[sender_id]->type);
		}
		
	} else if (core_inter_head[sender_id] == NULL) {
		fprintf(log_file,"i have to reject req_cores from %d. with null interaction\n",sender_id);

		scc_kill(sender_id, SIG_REJECT, NULL);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	} else if (core_inter_head[sender_id]->type == AGENT_REQ_CORES && state != IDLE_INIT_MAN && pending_state != IDLE_INIT_MAN
		&& pending_state != AGENT_INIT_IDLE_INIT && pending_state != IDLE_INIT_IDLE_AGENT && pending_state != IDLE_INIT_AGENT_SELFOPT) {
		fprintf(log_file,"I am %d and i think i cought a stray init AGENT_REQ_CORES reply from %d. My current state is %s\n",node_id,sender_id,id2string(state));

		scc_kill(sender_id, SIG_REJECT, NULL);
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
		fprintf(log_file,"I am %d and i think i cought a stray selfopt SELFOPT_REQ_CORES reply from %d. My current state is %s\n",node_id,sender_id,id2string(state));

		scc_kill(sender_id, SIG_REJECT, NULL);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		free(tmp_inter_list);
		
		if(core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);
	} else if (core_inter_head[sender_id]->type == AGENT_REQ_CORES || core_inter_head[sender_id]->type == SELFOPT_REQ_CORES) { 
		//I am the requesting common node
		num_of_offers = sig_read_ar[2];
		fprintf(log_file, "num_of_offers = %d\n",num_of_offers);

		if (num_of_offers > 0) {
			#ifdef PLAT_SCC
			RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);
			scc_kill(sender_id, SIG_ACK, core_inter_head[sender_id]);

			RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);		
						
			error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), LINE_SIZE * sizeof(int), node_id);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				printf("I am %d and i got an error in get data in sig_REQ_CORES_handler from %d with descr %s\n",node_id,sender_id,error_str);
			}
		 
			#else
			scc_kill(sender_id, SIG_ACK, core_inter_head[sender_id]);	
			sem_wait(&flag_data_written[node_id]);
			//fprintf(log_file,"I acquire flag_data_written lock %d\n", node_id);

			my_RCCE_get(&data_array_local[0], &data_array[0], LINE_SIZE, node_id);	
			#endif

			my_stats.msg_count++;
			my_stats.distance += distance(node_id,sender_id);

			//fprintf(log_file,"Num of cores = %d. Cores:",sig_read_ar[3]);
			//for (i=0; i<LINE_SIZE; i++)
			//	fprintf(log_file,"%d, ",data_array_local[i]);
			//fprintf(log_file,"\n");
 
			core_inter_head[sender_id]->data.offer_acc_array = (int *) malloc((num_of_offers+1)*sizeof(int));	
			core_inter_head[sender_id]->data.offer_acc_array[0] = num_of_offers;			
			
			for (j=1; j<=num_of_offers; j++){
				one_offer.num_of_cores = sig_read_ar[3];
				memcpy(&one_offer.spd_loss,&sig_read_ar[4],sizeof(int));					

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

					if (tmp_offer_list == NULL) { 
						tmp_offer_prev->next = (offer_list *) malloc(sizeof(offer_list));
						tmp_offer_list = tmp_offer_prev->next;
						tmp_offer_list->next = NULL;
						chosen_node = tmp_offer_list;
					} else if (tmp_offer_prev == NULL) { 
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
					chosen_node->off.offered_cores[i] = data_array_local[i];
				
				chosen_node->sender = sender_id;
				core_inter_head[sender_id]->data.offer_acc_array[j] = -1;
				chosen_node->answer = &core_inter_head[sender_id]->data.offer_acc_array[j];

				cur_time = time(NULL);	
				cur_t = localtime(&cur_time);
				fprintf(log_file, "[%d:%d:%d]: One node successfully added in list type=%d sender_id=%d\n",
					cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,core_inter_head[sender_id]->type,sender_id);
			}
			core_inter_head[sender_id]->type = REP_AGENT_OFFER_PENDING;
		} else { //no answer is required
			scc_kill(sender_id, SIG_ACK, core_inter_head[sender_id]);		
			my_stats.msg_count++;
			my_stats.distance += distance(node_id,sender_id);
		  
			tmp_inter_list = core_inter_head[sender_id];
			core_inter_head[sender_id] = core_inter_head[sender_id]->next;
			free(tmp_inter_list);
	
			if(core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
			else send_next_signal(core_inter_head[sender_id], sender_id);
		}	
	} else if (core_inter_head[sender_id]->type == AGENT_REQ_CORES_PENDING || core_inter_head[sender_id]->type == SELFOPT_REQ_CORES_PENDING){
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
		fprintf(log_file,"I am %d and i have to reject req_cores from %d. Our interaction is %d\n",node_id,sender_id,core_inter_head[sender_id]->type);

		scc_kill(sender_id, SIG_REJECT, NULL);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
	}

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_REQ_CORES_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));

	return;
}

void sig_REP_OFFERS_handler(int sender_id){
	
	int offer_ans, i, one_core, j;
	core_list *tmp_cores, *tmp_cores_prev, *tmp_cores_list;
	inter_list *tmp_inter_list; //*tmp_inter_prev;
	#ifdef PLAT_SCC	
	int error, str_len;
	char error_str[64];
	#endif	
		
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_REP_OFFERS_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
	if (core_inter_head[sender_id] == NULL){
		fprintf(log_file,"\t\tcore_inter_head[%d] is NULL - FAIL!\n",sender_id);
	}else if (core_inter_head[sender_id]->type == AGENT_OFFER_SENT) {
		scc_kill(sender_id, SIG_ACK, core_inter_head[sender_id]);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		fprintf(log_file, "Num of offers is  =  %d\n",core_inter_head[sender_id]->data.off_arr.num_of_offers);
		
		//even if i am in a far req offer, my answer will be the first
		for (j=0; j<core_inter_head[sender_id]->data.off_arr.num_of_offers; j++) {
			offer_ans = sig_read_ar[2];//data_array_local[0];
			fprintf(log_file, "offer_ans = %d\n",offer_ans);
					
			while (offer_ans != 0 && offer_ans != 1){
				printf("I am %d and i am getting bizarre answer = %d from %d\n",node_id,offer_ans,sender_id);
			}

			if (offer_ans == 0 && my_cores != NULL) { 
				for (i=0; i<core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores; i++)
					for (tmp_cores = my_cores->next; tmp_cores != NULL; tmp_cores = tmp_cores->next)
						if (tmp_cores->core_id == core_inter_head[sender_id]->data.off_arr.offer_arr[j].offered_cores[i]) {
							fprintf(log_file,"core id = %d offered_to = %d\n",tmp_cores->core_id,tmp_cores->offered_to);
							
							if (tmp_cores->offered_to == sender_id) tmp_cores->offered_to = -1;
							break;
						}

				if (app_state == APP_TERMINATED) { //app_terminated
					for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next)
					if (tmp_cores_list->offered_to != -1) break;

					if (tmp_cores_list == NULL && state == AGENT_ZOMBIE) state = AGENT_ENDING;
				}
				
			} else if (offer_ans == 1) {
				if (my_idag == -1) { //I am an idag 
					DDS->num_of_cores -= core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores;			
				} else {//I am common node				
					/*first i must get the remaining time from my cores, before i change my core list*/
					
					if (app_state != APP_TERMINATED) { //!app_terminated	
						if (app_state != RESIZING) {						
							old_Speedup = my_Speedup;
							old_cores_cnt = my_cores_count;
						}
						my_cores_count -= core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores;
						my_app.num_of_cores = my_cores_count;				
						my_Speedup = Speedup(my_app, my_cores_count);			
	
						fprintf(log_file,"Initialising removal new_cores_count = %d app_state = %d\n",my_cores_count,app_state);
						fprintf(app_log_file,"Initialising removal new_cores_count = %d app_state = %d\n",my_cores_count,app_state);
						fflush(app_log_file);
					} else {
						my_cores_count -= core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores;
						my_app.num_of_cores = my_cores_count;	

						fprintf(log_file,"Initialising removal app finished new_cores_count = %d\n",my_cores_count);
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
					core_inter_tail[my_idag]->data.app_cores = (int *)malloc((core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores+2)*sizeof(int));
					core_inter_tail[my_idag]->data.app_cores[0] = core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores;
					for(i=1; i<=core_inter_head[sender_id]->data.off_arr.offer_arr[j].num_of_cores; i++) {
						one_core = core_inter_head[sender_id]->data.off_arr.offer_arr[j].offered_cores[i-1];
					
						if (app_state != APP_TERMINATED) {
							if (core_inter_head[one_core] != NULL) { 
								tmp_inter_list = core_inter_head[one_core]->next;
							} else {
								tmp_inter_list = NULL;
							}
						} else {
							tmp_inter_list = core_inter_head[one_core];
						}
						
						while (tmp_inter_list != NULL)
							if (tmp_inter_list->type == INIT_WORK_NODE || tmp_inter_list->type == APPOINT_WORK_NODE || tmp_inter_list->type == INIT_WORK_NODE_PENDING 
								|| tmp_inter_list->type == APPOINT_WORK_NODE_PENDING) {							
							
								fprintf(log_file, "Removing in rem offers one node of %d with inter = %d\n",one_core,tmp_inter_list->type);

								tmp_inter_list = tmp_inter_list->next; 	
							} else {
								//tmp_inter_prev = tmp_inter_list;
								tmp_inter_list = tmp_inter_list->next;
							} 
					
						if (app_state != APP_TERMINATED) {
							if (core_inter_head[one_core] == NULL) {
								fprintf(log_file,"No interaction with %d. Theoritically impossible\n",one_core);
							} else if (core_inter_head[one_core]->type == INIT_WORK_NODE_PENDING) {
								fprintf(log_file,"I offered my new core %d. I will clear the interaction\n",one_core);
							} else if (core_inter_head[one_core]->type == INIT_WORK_NODE || core_inter_head[one_core]->type == APPOINT_WORK_NODE) {
								fprintf(log_file,"Invalidating %d. Interaction is %d\n",one_core,core_inter_head[one_core]->type);
							} else if (core_inter_head[one_core]->type == APPOINT_WORK_NODE_PENDING) {
								fprintf(log_file,"Everything ok %d.\n",one_core);
								fflush(log_file);
							} else {
								fprintf(log_file,"Another interaction with %d. Interaction is %d\n",one_core,core_inter_head[one_core]->type);
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
								printf("I offered my agent core!!!\n");
								my_cores = my_cores->next;
							} else if (tmp_cores == my_cores_tail){
								my_cores_tail = tmp_cores_prev;
								my_cores_tail->next = NULL;
							} else tmp_cores_prev->next = tmp_cores->next;
		
							free(tmp_cores);
						}
	
						core_inter_tail[my_idag]->data.app_cores[i] = core_inter_head[sender_id]->data.off_arr.offer_arr[j].offered_cores[i-1];
					}
					core_inter_tail[my_idag]->data.app_cores[i] = sender_id; //new owner, or init manager				
					core_inter_tail[my_idag]->next = NULL;

					if (core_inter_head[my_idag]->next == NULL) {
						scc_kill(my_idag, SIG_REM_CORES_DDS, core_inter_head[my_idag]);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,my_idag);
					} else {
						fprintf(log_file,"I did not send rem signal! with interaction=%d\n",core_inter_head[my_idag]->type);
					}
					
					cur_time = time(NULL);	
					cur_t = localtime(&cur_time);	
					fprintf(app_log_file, "[%d:%d:%d]: Removal ok. my_cores_count = %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,my_cores_count);
					fprintf(app_log_file, "--%d-- my cores are:", node_id);

					for (tmp_cores_list=my_cores; tmp_cores_list!=NULL; tmp_cores_list=tmp_cores_list->next) {
						fprintf(app_log_file, " %d", tmp_cores_list->core_id);
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
	} else printf("I am %d and fail 2\n",node_id);

	tmp_inter_list = core_inter_head[sender_id];
	core_inter_head[sender_id] = core_inter_head[sender_id]->next;
	if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
	else send_next_signal(core_inter_head[sender_id], sender_id);	
	free(tmp_inter_list);

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_REP_OFFERS_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));

	return;
}

void sig_INIT_AGENT_handler(int sender_id){
	
	int i, tmp; 
	core_list *tmp_core;
	int data_array_local[3 * LINE_SIZE];
	#ifdef PLAT_SCC	
	int error, str_len;
	char error_str[64];
	#endif
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_INIT_AGENT_handler with sender = %d state = %s pending = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state),id2string(pending_state));

	#ifdef PLAT_SCC
	RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);
	scc_kill(sender_id, SIG_ACK, NULL);

	RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);		
		
	error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), 2 * LINE_SIZE * sizeof(int), node_id);
	if (error != RCCE_SUCCESS) {
		RCCE_error_string(error, error_str, &str_len);
		printf("I am %d and i got an error in get data in sig_INIT_AGENT_handler from %d with descr %s\n",node_id,sender_id,error_str);
	}		
	#else
	scc_kill(sender_id, SIG_ACK, NULL);	
	sem_wait(&flag_data_written[node_id]);
	//fprintf(log_file,"I acquire flag_data_written lock %d\n",node_id);

	my_RCCE_get(&data_array_local[0], &data_array[0], 2 * LINE_SIZE, node_id);	
	#endif	

	my_stats.msg_count++;
	my_stats.distance += distance(node_id,sender_id);

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

	my_app.id = sig_read_ar[2];
	my_app.app_type = sig_read_ar[3];
	my_app.workld = sig_read_ar[4];
	my_app.num_of_cores = sig_read_ar[5];
#ifndef ARTIFICIAL_APPS_SIM
	my_app.array_size = sig_read_ar[6];
	fprintf(log_file, "id=%d array_size=%d workld=%d num_of_cores=%d app_type=%d\n",my_app.id,my_app.array_size,my_app.workld,my_app.num_of_cores,my_app.app_type);
#else
	memcpy(&my_app.var,&sig_read_ar[6],sizeof(int));
	memcpy(&my_app.A,&sig_read_ar[7],sizeof(int));

	/* FIXME printing order maintained for python scripts compatibility */
	fprintf(log_file, "id=%d var=%f workld=%d num_of_cores=%d app_type=%d A=%f\n",my_app.id,my_app.var,my_app.workld,my_app.num_of_cores,my_app.app_type,my_app.A);
#endif		
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
		tmp = data_array_local[i+0*LINE_SIZE];
		if (tmp != node_id){
			my_cores_tail->next = (core_list *) malloc(sizeof(core_list));
			my_cores_tail = my_cores_tail->next;

			my_cores_tail->core_id = tmp;
			my_cores_tail->offered_to = -1;
			my_cores_tail->next = NULL;
		}
	}

	my_app_times[0].tm_sec = data_array_local[1*LINE_SIZE];
	my_app_times[0].tm_min = data_array_local[1*LINE_SIZE+1];
	my_app_times[0].tm_hour = data_array_local[1*LINE_SIZE+2];
	my_app_times[0].tm_usec = data_array_local[1*LINE_SIZE+3];
	my_app_times[1].tm_sec = data_array_local[1*LINE_SIZE+4];
	my_app_times[1].tm_min = data_array_local[1*LINE_SIZE+5];
	my_app_times[1].tm_hour = data_array_local[1*LINE_SIZE+6];
	my_app_times[1].tm_usec = data_array_local[1*LINE_SIZE+7];

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
		scc_kill(my_idag, SIG_ADD_CORES_DDS, core_inter_head[my_idag]);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,my_idag);
	}
	
	if (cur_agent.my_agent != -1) {
		fprintf(log_file,"I am beginning as agent with old agent %d\n",cur_agent.my_agent);
	}

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: Init ok!! my_cores_count = %d app_id=%d my_Speedup= %.2f\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,my_cores_count,my_app.id,my_Speedup);
	fprintf(log_file, "my cores are:");
	for (tmp_core=my_cores; tmp_core!=NULL; tmp_core=tmp_core->next) fprintf(log_file, " %d", tmp_core->core_id);
	fprintf(log_file, "\n");

	if (state == IDLE_INIT_MAN || state == INIT_MANAGER || state == INIT_MANAGER_SEND_OFFERS || state == INIT_MAN_CHK_OFFERS) {
		if (pending_state != NO_PENDING_STATE) {
			printf("\n!!!! I am %d and I change my pending state in agent_init from random previous pending_state = %s!!!!\n\n",node_id,id2string(pending_state));
			fflush(stdout);
			fprintf(log_file,"\n!!!! I change pending_state in agent init from random previous pending_state = %s!!!!\n\n",id2string(pending_state));
			fflush(log_file);
		}
		pending_state = state;
		state = AGENT_INIT_STATE;
	} else if (state == IDLE_CORE && cur_agent.my_agent != -1) {
		fprintf(log_file,"I am beginning as agent with old agent %d and state = %s\n",cur_agent.my_agent,id2string(state));
		pending_state = NO_PENDING_STATE;
		state = AGENT_INIT_STATE;
	} else if (state == WORKING_NODE) {
		fprintf(log_file,"I am currently working with agent %d\n",cur_agent.my_agent);
		
		if (pending_state == INIT_MAN_CHK_OFFERS)
			pending_state = AGENT_INIT_CHK_OFFERS;
		else if (pending_state == IDLE_INIT_MAN)
			pending_state = AGENT_INIT_IDLE_INIT;
		else	
			pending_state = AGENT_INIT_STATE;	
	} else state = AGENT_INIT_STATE;

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_INIT_AGENT_handler with sender=%d || state = %s || pending = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state),id2string(pending_state));
	
	return;
}

void sig_ADD_CORES_DDS_handler(int sender_id, int *inc_cnt, int cur_index_top){
	
	int i, is_sender_idag, j, new_agent_id; 
	int *nodes_to_process, nodes_cnt=0, processed_cnt=0;
	int *tmp_list, tmp_cnt, tmp_idag;	
	core_list *tmp_cores;//, *tmp_cores_list;
	DDS_list *tmp_DDS;
	
	#ifdef PLAT_SCC	
	int error, str_len;
	char error_str[64];
	#endif
	#ifdef ADAM_SIM
	DDS_list *tmp_DDS2;
	#endif

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_ADD_CORES_DDS_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));

	scc_kill(sender_id, SIG_ACK, NULL);
	my_stats.msg_count++;
	my_stats.distance += distance(node_id,sender_id);
	
	is_sender_idag = is_core_idag(sender_id);

	if (is_sender_idag == 0) {
		nodes_cnt = sig_read_ar[2];
		fprintf(log_file,"\t\tSender %d is not controller and nodes_cnt=%d\n",sender_id,nodes_cnt);

		nodes_to_process = (int *) malloc(nodes_cnt * sizeof(int));

		if (nodes_cnt > 5) {
			*inc_cnt = *inc_cnt + 1;

			#ifdef PLAT_SCC
			error = RCCE_get((t_vcharp)(&sig_read_ar[LINE_SIZE]), (t_vcharp)(&sig_array[(cur_index_top+1)*LINE_SIZE]), LINE_SIZE * sizeof(int), node_id);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				fprintf(log_file,"I got an error in get data in sig_ADD_CORES_DDS_handler from %d with descr %s\n",sender_id,error_str);
				fflush(log_file);
			}
			#else
			new_RCCE_get(sig_read_ar, sig_array, cur_index_top, LINE_SIZE, node_id);
			#endif
		
		}	

		for (i = 0; i < nodes_cnt; i++){
			fprintf(log_file,"\t\tnodes_to_process[%d]=%d\n",i,sig_read_ar[3+i]);
			nodes_to_process[i] = sig_read_ar[3+i];
		}
		tmp_list = (int *) malloc(nodes_cnt * sizeof(int));

		while (processed_cnt < nodes_cnt){
			
			tmp_cnt = 0;
			tmp_idag = -1;

			for (i = 0; i < nodes_cnt; i++){
				if (processed_cnt == nodes_cnt) break;
				if (nodes_to_process[i] == -1){
					continue;
				}else{
					if (tmp_idag == -1) tmp_idag = idag_mask[nodes_to_process[i]];

					if (idag_mask[nodes_to_process[i]] != tmp_idag) continue;
					else {
						tmp_list[tmp_cnt++] = nodes_to_process[i];
						nodes_to_process[i] = -1;
						processed_cnt++;
					}
				}
			}

			if (tmp_idag != node_id) {
				if (core_inter_head[tmp_idag] == NULL){
					core_inter_head[tmp_idag] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[tmp_idag] = core_inter_head[tmp_idag];
				} else {
					core_inter_tail[tmp_idag]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[tmp_idag] = core_inter_tail[tmp_idag]->next;
				}

				core_inter_tail[tmp_idag]->type = IDAG_ADD_CORES_DDS;
				core_inter_tail[tmp_idag]->data.app_cores = (int *)malloc((tmp_cnt+2)*sizeof(int));
				core_inter_tail[tmp_idag]->data.app_cores[0] = tmp_cnt;//+1;
				core_inter_tail[tmp_idag]->data.app_cores[tmp_cnt+1] = sender_id;

				for (j=1; j<= tmp_cnt; j++)
					core_inter_tail[tmp_idag]->data.app_cores[j] = tmp_list[j-1];
		
				core_inter_tail[tmp_idag]->next = NULL;

				if (core_inter_head[tmp_idag]->next == NULL) {
					fprintf(log_file,"megalo poutso\n");
					scc_kill(tmp_idag, SIG_ADD_CORES_DDS, core_inter_head[tmp_idag]);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,tmp_idag);
				} else {
					fprintf(log_file,"I did not send add_cores_dds to %d with inter=%d\n",tmp_idag,core_inter_head[tmp_idag]->type);
				}
			} else {
				new_agent_id = sender_id;
				for (i=0; i<tmp_cnt; i++){//nodes_cnt
					fprintf(log_file,"I am importing node %d\n",tmp_list[i]);
					tmp_cores = my_cores;
			
					while (tmp_cores->core_id != tmp_list[i])
						tmp_cores = tmp_cores->next;
					
					if (tmp_cores->offered_to == -1) {
						fprintf(log_file,"Node %d was offered to nobody\n",tmp_list[i]);
						
						DDS->num_of_cores--;
						tmp_cores->offered_to = new_agent_id;
					} else if (tmp_cores->offered_to != new_agent_id) {
						fprintf(log_file,"Node %d is offered to %d\n",tmp_list[i],tmp_cores->offered_to);
						
						tmp_cores->offered_to = new_agent_id;
					}
				}

				tmp_DDS = DDS;
				while (tmp_DDS != NULL && tmp_DDS->agent_id != new_agent_id) {
					tmp_DDS = tmp_DDS->next;
				}

#ifndef ADAM_SIM
				if (tmp_DDS != NULL) {
					tmp_DDS->num_of_cores += nodes_cnt;
				} else {
					//printf("I am in here DDSing nodes_cnt=%d\n",nodes_cnt);
					DDS_tail->next = (DDS_list *) malloc(sizeof(DDS_list));
					DDS_tail = DDS_tail->next;
					DDS_tail->agent_id = new_agent_id;
					DDS_tail->num_of_cores = nodes_cnt;
					DDS_tail->next = NULL;
					DDS_count++;
				}
#else
				if (tmp_DDS != NULL) {
					tmp_DDS->num_of_cores += nodes_cnt;
					
					/* tmp_DDS out */
					tmp_DDS->prev->next = tmp_DDS->next; /* Remove the link in the list */
					tmp_DDS->next->prev = tmp_DDS->prev;
				} else { /* Create new node */
					tmp_DDS = (DDS_list *) malloc(sizeof(DDS_list));
					tmp_DDS->agent_id = new_agent_id;
					tmp_DDS->num_of_cores = nodes_cnt;
					DDS_count++;
				}
				
				/* Insert sorted */
				if (tmp_DDS->num_of_cores > DDS->num_of_cores) { /* Check if it must be placed first */
					tmp_DDS2 = DDS;
					DDS = tmp_DDS;
					DDS->next = tmp_DDS2;
					DDS->prev = NULL;
					tmp_DDS2->prev = DDS;
				} else if (tmp_DDS->num_of_cores < DDS_tail->num_of_cores) { /* Check if it must be placed last */
					DDS_tail->next = tmp_DDS;
					tmp_DDS->prev = DDS_tail;
					DDS_tail = DDS_tail->next;
					DDS_tail->next = NULL;
				} else {
					tmp_DDS2 = DDS;
					while ((tmp_DDS2->next != NULL) && (tmp_DDS->num_of_cores <= tmp_DDS2->next->num_of_cores)) {		
						/* tmp_DDS in between tmp_DDS2 and tmp_DDS2->next */
						tmp_DDS->next = tmp_DDS2->next;
						tmp_DDS2->next->prev = tmp_DDS;
						
						tmp_DDS2->next = tmp_DDS;
						tmp_DDS->prev = tmp_DDS2;	
					}						
				}
#endif		
			}
		}
	} else {
		nodes_cnt = sig_read_ar[2];
		new_agent_id = sig_read_ar[3];
		nodes_to_process = (int *) malloc(nodes_cnt * sizeof(int));
		fprintf(log_file,"poutso2 nodes_cnt=%d\n",nodes_cnt);
		if (nodes_cnt > 4) {
			*inc_cnt = *inc_cnt + 1;

			#ifdef PLAT_SCC
			error = RCCE_get((t_vcharp)(&sig_read_ar[LINE_SIZE]), (t_vcharp)(&sig_array[(cur_index_top+1)*LINE_SIZE]), LINE_SIZE * sizeof(int), node_id);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				fprintf(log_file,"I got an error in get data in sig_ADD_CORES_DDS_handler from %d with descr %s\n",sender_id,error_str);
				fflush(log_file);
			}
			#else
			fprintf(log_file,"before new_RCCE_get with cur_index_top=%d\n",cur_index_top);
			new_RCCE_get(sig_read_ar, sig_array, cur_index_top, LINE_SIZE, node_id);
			#endif
		}
		
		for (i=0; i<nodes_cnt; i++)
			nodes_to_process[i] = sig_read_ar[i+4];

		for (i=0; i<nodes_cnt; i++) {
			fprintf(log_file,"I am importing node %d\n",nodes_to_process[i]);
			tmp_cores = my_cores;

			while (tmp_cores->core_id != nodes_to_process[i])
				tmp_cores = tmp_cores->next;
		
			if (tmp_cores->offered_to == -1) {
				fprintf(log_file,"Node %d was offered to nobody\n",nodes_to_process[i]);
				
				DDS->num_of_cores--;
				tmp_cores->offered_to = new_agent_id;
			} else if (tmp_cores->offered_to != new_agent_id) {
				fprintf(log_file,"Node %d is offered to %d\n",nodes_to_process[i],tmp_cores->offered_to);
				
				tmp_cores->offered_to = new_agent_id;
			}
		}

		tmp_DDS = DDS;
		while (tmp_DDS != NULL && tmp_DDS->agent_id != new_agent_id)
			tmp_DDS = tmp_DDS->next;

		if (tmp_DDS != NULL)
			tmp_DDS->num_of_cores += nodes_cnt;
		else {
			if (DDS_count == 1 && DDS != DDS_tail) printf("I am %d and things are bad!\n",node_id);
			DDS_tail->next = (DDS_list *) malloc(sizeof(DDS_list));
			DDS_tail = DDS_tail->next;
			DDS_tail->agent_id = new_agent_id;
			DDS_tail->num_of_cores = nodes_cnt;
			DDS_tail->next = NULL;
			DDS_count++;
		}
	}		
	
	my_stats.times_accessed++;
	fprintf(log_file,"I am %d Adding ended well with sender_id=%d!\n",node_id,sender_id);
	fprintf(log_file,"Number of agents in region = %d\n",DDS_count);	
	fprintf(log_file,"Agent no 0 is %d with %d cores\n",DDS->agent_id,DDS->num_of_cores);	
	i=1;
	for (tmp_DDS = DDS->next; tmp_DDS != NULL; tmp_DDS = tmp_DDS->next){
		fprintf(log_file,"Agent no %d is %d with %d cores\n",i,tmp_DDS->agent_id,tmp_DDS->num_of_cores);
		my_stats.cores_utilized += tmp_DDS->num_of_cores;
		i++;
	}

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_ADD_CORES_DDS_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));

	return;
}

void sig_REM_CORES_DDS_handler(int sender_id, int *inc_cnt, int cur_index_top){
  
	int i, is_sender_idag, j, new_agent_id, old_agent_id; 
	int *nodes_to_process, nodes_cnt=0, processed_cnt=0;
	int *tmp_list, tmp_cnt, tmp_idag;	
	DDS_list *tmp_DDS,*tmp_DDS_prev;
	core_list *tmp_cores;	
	#ifdef PLAT_SCC
	int error, str_len;
	char error_str[64];
	#endif	
	#ifdef ADAM_SIM
	DDS_list *tmp_DDS2;
	#endif	

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_REM_CORES_DDS_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
	scc_kill(sender_id, SIG_ACK, NULL);
	my_stats.msg_count++;
	my_stats.distance += distance(node_id,sender_id);
	
	is_sender_idag = is_core_idag(sender_id);

	if (is_sender_idag == 0) {
		nodes_cnt = sig_read_ar[2];
		new_agent_id = sig_read_ar[3];
		
		if (nodes_cnt > 4) {
			*inc_cnt = *inc_cnt + 1;

			#ifdef PLAT_SCC
			error = RCCE_get((t_vcharp)(&sig_read_ar[LINE_SIZE]), (t_vcharp)(&sig_array[(cur_index_top+1)*LINE_SIZE]), LINE_SIZE * sizeof(int), node_id);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				fprintf(log_file,"I got an error in get data in sig_ADD_CORES_DDS_handler from %d with descr %s\n",sender_id,error_str);
			}
			#else
			new_RCCE_get(sig_read_ar, sig_array, cur_index_top, LINE_SIZE, node_id);
			#endif
		}	
		
		nodes_to_process = (int *) malloc(nodes_cnt * sizeof(int));
		for (i=0; i<nodes_cnt; i++)
			nodes_to_process[i] = sig_read_ar[i+4];

		tmp_list = (int *) malloc(nodes_cnt * sizeof(int));

		while (processed_cnt < nodes_cnt) {
			
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
					core_inter_tail[tmp_idag]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[tmp_idag] = core_inter_tail[tmp_idag]->next;
				}

				core_inter_tail[tmp_idag]->type = IDAG_REM_CORES_DDS;
				core_inter_tail[tmp_idag]->data.app_cores = (int *)malloc((tmp_cnt+3)*sizeof(int));
				core_inter_tail[tmp_idag]->data.app_cores[0] = tmp_cnt;//+1;
				core_inter_tail[tmp_idag]->data.app_cores[tmp_cnt+1] = sender_id;
				core_inter_tail[tmp_idag]->data.app_cores[tmp_cnt+2] = new_agent_id;

				for (j=1; j<= tmp_cnt; j++)
					core_inter_tail[tmp_idag]->data.app_cores[j] = tmp_list[j-1];
		
				core_inter_tail[tmp_idag]->next = NULL;

				if (core_inter_head[tmp_idag]->next == NULL) {
					scc_kill(tmp_idag, SIG_REM_CORES_DDS, core_inter_head[tmp_idag]);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,tmp_idag);
				} else {
					fprintf(log_file,"i did not send sig_rem_dds to %d with interaction %d\n",tmp_idag,core_inter_head[tmp_idag]->type);
				}
			} else {
				fprintf(log_file,"In REM same tmp_idag = %d tmp_cnt = %d nodes_cnt = %d sender_id = %d \n",tmp_idag,tmp_cnt,nodes_cnt,sender_id);
				
				old_agent_id = sender_id;
				
				for (i=0; i<tmp_cnt; i++) {//nodes_cnt
					fprintf(log_file,"I am reassigning node %d\n",tmp_list[i]);
					tmp_cores = my_cores;
			
					while (tmp_cores->core_id != tmp_list[i])
						tmp_cores = tmp_cores->next;
					
					if (tmp_cores->offered_to == -1) {
						fprintf(log_file,"Node was offered to nobody\n");
						tmp_cores->offered_to = new_agent_id;
					} else if (tmp_cores->offered_to == old_agent_id) {
						fprintf(log_file,"Node is offered to old_agent\n");
						tmp_cores->offered_to = -2;//new_agent_id; indicating it is not mine but someone has it
					} else {
						fprintf(log_file,"Node is offered to %d\n",tmp_cores->offered_to);
					}
				}

				tmp_DDS = DDS;
				tmp_DDS_prev = NULL;	
				while (tmp_DDS != NULL && tmp_DDS->agent_id != old_agent_id){
					tmp_DDS_prev = tmp_DDS;
					tmp_DDS = tmp_DDS->next;
				}

				if (tmp_DDS == NULL) printf("Agent does not exist in my DDS\n");
				else if (tmp_DDS == DDS) printf("I am removing from myself in REM?\n");
				else {
					tmp_DDS->num_of_cores -= tmp_cnt; /* was nodes_cnt but this seems totally wrong. 5.5.2017 */
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
	} else { //Do not remove cores since they may already be added
		nodes_cnt = sig_read_ar[2];
		old_agent_id = sig_read_ar[3];
		new_agent_id = sig_read_ar[4];

		nodes_to_process = (int *) malloc(nodes_cnt * sizeof(int));

		if (nodes_cnt > 3) {
			*inc_cnt = *inc_cnt + 1;

			#ifdef PLAT_SCC			
			error = RCCE_get((t_vcharp)(&sig_read_ar[LINE_SIZE]), (t_vcharp)(&sig_array[(cur_index_top+1)*LINE_SIZE]), LINE_SIZE * sizeof(int), node_id);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				fprintf(log_file,"I got an error in get data in sig_ADD_CORES_DDS_handler from %d with descr %s\n",sender_id,error_str);
				fflush(log_file);
			}
			#else
			new_RCCE_get(sig_read_ar, sig_array, cur_index_top, LINE_SIZE, node_id);
			#endif
		}

		for (i=0; i<nodes_cnt; i++)
			nodes_to_process[i] = sig_read_ar[i+5];
		
		fprintf(log_file,"old=%d new=%d\n",old_agent_id,new_agent_id);
		
		for (i=0; i<nodes_cnt; i++) {
			fprintf(log_file,"I am reassigning node %d\n",nodes_to_process[i]);
			tmp_cores = my_cores;

			while (tmp_cores->core_id != nodes_to_process[i]){
				tmp_cores = tmp_cores->next;
			}

			if (tmp_cores->offered_to == -1) {
				fprintf(log_file,"Node was offered to nobody\n");
				tmp_cores->offered_to = new_agent_id;
			} else if (tmp_cores->offered_to == old_agent_id) {
				fprintf(log_file,"Node is offered to old_agent\n");
				tmp_cores->offered_to = -2;//new_agent_id; indicating it is not mine but someone has it
			} else {
				fprintf(log_file,"Node is offered to %d\n",tmp_cores->offered_to);
			}
		}

		tmp_DDS = DDS;
		tmp_DDS_prev = NULL;	
		while (tmp_DDS != NULL && tmp_DDS->agent_id != old_agent_id){
			tmp_DDS_prev = tmp_DDS;
			tmp_DDS = tmp_DDS->next;
		}

		if (tmp_DDS == NULL) {
			fprintf(log_file,"Agent does not exist in my DDS\n");
		} else if (tmp_DDS == DDS) {
			fprintf(log_file,"I am removing from myself in REM?\n");
		} else {
			tmp_DDS->num_of_cores -= nodes_cnt;

			if (tmp_DDS->num_of_cores == 0) {
				DDS_count--;
				if (tmp_DDS == DDS_tail){
					DDS_tail = tmp_DDS_prev;
					DDS_tail->next = NULL;
				} else {
					tmp_DDS_prev->next = tmp_DDS->next;
#ifdef ADAM_SIM
					tmp_DDS->next->prev = tmp_DDS_prev;
#endif			
				}
				free(tmp_DDS);
			} 
#ifdef ADAM_SIM
			else {
				/* tmp_DDS out */
				tmp_DDS->prev->next = tmp_DDS->next; /* Remove the link in the list */
				tmp_DDS->next->prev = tmp_DDS->prev;
					
				/* Insert sorted */
				if (tmp_DDS->num_of_cores > DDS->num_of_cores) { /* Check if it must be placed first */
					tmp_DDS2 = DDS;
					DDS = tmp_DDS;
					DDS->next = tmp_DDS2;
					DDS->prev = NULL;
					tmp_DDS2->prev = DDS;
				} else if (tmp_DDS->num_of_cores < DDS_tail->num_of_cores) { /* Check if it must be placed last */
					DDS_tail->next = tmp_DDS;
					tmp_DDS->prev = DDS_tail;
					DDS_tail = DDS_tail->next;
					DDS_tail->next = NULL;
				} else {
					tmp_DDS2 = DDS;
					while ((tmp_DDS2->next != NULL) && (tmp_DDS->num_of_cores <= tmp_DDS2->next->num_of_cores)) {		
						/* tmp_DDS in between tmp_DDS2 and tmp_DDS2->next */
						tmp_DDS->next = tmp_DDS2->next;
						tmp_DDS2->next->prev = tmp_DDS;
							
						tmp_DDS2->next = tmp_DDS;
						tmp_DDS->prev = tmp_DDS2;	
					}						
				}
			}
#endif
		}	
	}					

	fprintf(log_file,"I am %d Removing ended well! with sender_id=%d!\n",node_id,sender_id);
	fprintf(log_file,"Number of agents in region = %d\n",DDS_count);	
	i=0;
	for (tmp_DDS = DDS; tmp_DDS != NULL; tmp_DDS = tmp_DDS->next){
		fprintf(log_file,"Agent no %d is %d with %d cores\n",i,tmp_DDS->agent_id,tmp_DDS->num_of_cores);	
		i++;
	}	

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_REM_CORES_DDS_handler with sender=%d state=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,state);
	fflush(log_file);

	return;
}

void sig_APPOINT_WORK_handler(int sender_id, int *inc_cnt, int cur_index_top){
	
	int valid=-1;
	int num_of_coworkers;
	coworkers_list *tmp_coworkers,*tmp_cowork_list;
	#ifdef PLAT_SCC
	int error, str_len;
	char error_str[64];
	#endif
		
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_APPOINT_WORK_handler with sender = %d || state = %s || pending_state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state),id2string(pending_state));
	fflush(log_file);
	
	if (my_idag == -1) {
		fprintf(log_file,"I am a controller core and was appointed workload. Aborting...\n");
		valid = -2; //i do not send reject because workload is already in pending_workload
	}  
	
 	if (state == IDLE_AGENT_WAITING_OFF || state == IDLE_AGENT || state == AGENT_ENDING ||  
	    state == AGENT_SELF_OPT || state == AGENT_SELF_CHK_OFFERS || state == AGENT_ZOMBIE) {
	    
	    fprintf(log_file,"\n!!!! I reject work appoint with state = %s and pending_state = %s!!!!\n\n",id2string(state),id2string(pending_state));
	    valid = -3;
	}  
	   
	if (valid == -1){ 
		scc_kill(sender_id, SIG_ACK, NULL);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		worker_app_id = sig_read_ar[2];
		valid = sig_read_ar[2];// data_array_local[0];
		//fprintf(log_file, "Validity of message = %d\n",valid);
		//fflush(log_file);
	  
	  
		if (valid >= 0) {

			if (cur_agent.my_agent == -1) {
				cur_agent.my_agent = sig_read_ar[3];
				cur_agent.app_type = sig_read_ar[4];
				cur_agent.work_bounds[0] = sig_read_ar[5];
				cur_agent.work_bounds[1] = sig_read_ar[6];
#ifndef ARTIFICIAL_APPS_SIM
				cur_agent.array_size = sig_read_ar[7];
#endif
				#ifdef VERBOSE_WORKER
				printf("I am %d My agent is %d. I have to multiply lines %d through %d\n",node_id,cur_agent.my_agent,cur_agent.work_bounds[0],cur_agent.work_bounds[1]);
				fflush(stdout);
				#endif
				fprintf(log_file,"My agent is %d. I have to multiply lines %d through %d\n",cur_agent.my_agent,cur_agent.work_bounds[0],cur_agent.work_bounds[1]);
			} else if (cur_agent.my_agent != sender_id && state != IDLE_CORE) {
				pending_agent.my_agent = sig_read_ar[3];
				pending_agent.app_type = sig_read_ar[4];
				pending_agent.work_bounds[0] = sig_read_ar[5];
				pending_agent.work_bounds[1] = sig_read_ar[6];
#ifndef ARTIFICIAL_APPS_SIM
				pending_agent.array_size = sig_read_ar[7];
#endif
				#ifdef VERBOSE_WORKER
					printf("I am %d My old agent is %d. New agent is %d I have to multiply lines %d through %d\n"
						,node_id,cur_agent.my_agent,pending_agent.my_agent,pending_agent.work_bounds[0],pending_agent.work_bounds[1]);
					fflush(stdout);
				#endif
				fprintf(log_file,"My old agent is %d. New agent is %d I have to multiply lines %d through %d\n"
					,cur_agent.my_agent,pending_agent.my_agent,pending_agent.work_bounds[0],pending_agent.work_bounds[1]);
			} else if (cur_agent.my_agent != sender_id) {				
				cur_agent.my_agent = sig_read_ar[3];
				cur_agent.app_type = sig_read_ar[4];
				cur_agent.work_bounds[0] = sig_read_ar[5];
				cur_agent.work_bounds[1] = sig_read_ar[6];
#ifndef ARTIFICIAL_APPS_SIM
				cur_agent.array_size = sig_read_ar[7];
#endif
				base_offset = -1;
				#ifdef VERBOSE_WORKER
				printf("I am %d My new agent is %d. I have to multiply lines %d through %d\n",node_id,cur_agent.my_agent,cur_agent.work_bounds[0],cur_agent.work_bounds[1]);
				fflush(stdout);
				#endif
				fprintf(log_file,"My new agent is %d. I have to multiply lines %d through %d\n",cur_agent.my_agent,cur_agent.work_bounds[0],cur_agent.work_bounds[1]);
			} else {
				/* 12.7.2017 Changed the order to be consistent with INIT_WORK_NODE */
				cur_agent.app_type = sig_read_ar[4];
				cur_agent.work_bounds[0] = sig_read_ar[5];
				cur_agent.work_bounds[1] = sig_read_ar[6];
		
				#ifdef VERBOSE_WORKER
				printf("I am %d My agent is still %d. I have to multiply lines %d through %d\n",node_id,cur_agent.my_agent,cur_agent.work_bounds[0],cur_agent.work_bounds[1]);
				fflush(stdout);
				#endif
				fprintf(log_file,"My agent is still %d. I have to multiply lines %d through %d\n",cur_agent.my_agent,cur_agent.work_bounds[0],cur_agent.work_bounds[1]);
			}
		
			if (state == IDLE_INIT_MAN) {
				state = WORKING_NODE;
								
				if (pending_state == AGENT_SELF_OPT){
					printf("!!!!!!\n\nsignal_handlers: IDLE_INIT_AGENT_SELFOPT\n\n!!!!!!!\n");
					fprintf(log_file,"!!!!!!\n\nsignal_handlers: IDLE_INIT_AGENT_SELFOPT\n\n!!!!!!!\n");
					pending_state = IDLE_INIT_AGENT_SELFOPT;
				}else if (pending_state == IDLE_AGENT){
					pending_state = IDLE_INIT_IDLE_AGENT;
					printf("!!!!!!\n\nsignal_handlers: IDLE_INIT_IDLE_AGENT\n\n!!!!!!!\n");
					fprintf(log_file,"!!!!!!\n\nsignal_handlers: IDLE_INIT_IDLE_AGENT\n\n!!!!!!!\n");
				}else if (pending_state != NO_PENDING_STATE) {
					printf("\n!!!! I am %d and I init change my pending state from random previous pending_state = %s!!!!\n\n",node_id,id2string(pending_state));
					fflush(stdout);
					fprintf(log_file,"\n!!!! I init change pending_state from random previous pending_state = %s!!!!\n\n",id2string(pending_state));
					fflush(log_file);
				} else pending_state = IDLE_INIT_MAN;
			/*Assumptions made is that i have not entered AGENT_INIT_STATE in common_node. Plus i give time to me and my cores to finish
			*their work from previous agent*/	
			} else if (state == AGENT_INIT_STATE) {	
				if (pending_state == IDLE_INIT_MAN) {
					fprintf(log_file,"Pending is idle_init. Switching to AGENT_INIT_IDLE_INIT pending and starting work\n");
					pending_state = AGENT_INIT_IDLE_INIT;
				} else if (pending_state != NO_PENDING_STATE) {
					printf("\n!!!! I am %d and I achange my pending state from random previous pending_state = %s!!!!\n\n",node_id,id2string(pending_state));
					fflush(stdout);
					fprintf(log_file,"\n!!!! I achange pending_state from random previous pending_state = %s!!!!\n\n",id2string(pending_state));
					fflush(log_file);
					pending_state = AGENT_INIT_STATE;  
				} else pending_state = AGENT_INIT_STATE;
				
				state = WORKING_NODE;
			} else if (state == AGENT_SELF_OPT) {
				if (pending_state != NO_PENDING_STATE) {
					printf("\n!!!! I am %d and I bchange my pending state from random previous pending_state = %s!!!!\n\n",node_id,id2string(pending_state));
					fflush(stdout);
					fprintf(log_file,"\n!!!! I bchange pending_state from random previous pending_state = %s!!!!\n\n",id2string(pending_state));
					fflush(log_file);
				}
				
				pending_state = AGENT_SELF_OPT;
				state = WORKING_NODE;
			} else if (state == INIT_MANAGER || state == INIT_MAN_CHK_OFFERS || state == INIT_MANAGER_SEND_OFFERS) {
				if (pending_state != NO_PENDING_STATE) {
					printf("\n!!!! I am %d and I change my pending state from random previous pending_state = %s!!!!\n\n",node_id,id2string(pending_state));
					fflush(stdout);
					fprintf(log_file,"\n!!!! I change pending_state from random previous pending_state = %s!!!!\n\n",id2string(pending_state));
					fflush(log_file);
				}
				pending_state = WORKING_NODE;
			} else {//if (state != WORKING_NODE_IDLE_INIT) {
				if (state != IDLE_CORE && state != WORKING_NODE) {
					printf("\n!!!! I am %d and I change to working state from random previous state = %s!!!!\n\n",node_id,id2string(state));
					fflush(stdout);
					fprintf(log_file,"\n!!!! I change to working state from random previous state = %s!!!!\n\n",id2string(state));
					fflush(log_file);
				}	
				state = WORKING_NODE;
			}
		} 
		
	} else if (valid == -3)	{
		scc_kill(sender_id, SIG_REJECT, NULL);
	}  
	
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_APPOINT_WORK_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));

	return;
}

void sig_CHECK_REM_TIME_handler(int sender_id){
	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_CHECK_REM_TIME_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_CHECK_REM_TIME_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
	return;
}

void sig_FINISH_handler(int sender_id, int *inc_cnt, int cur_index_top){
  
	int i, j, is_sender_idag, old_agent_id; 
	int *nodes_to_process, nodes_cnt=0, processed_cnt=0, row_reached, rows_left, per_core_rows, one_core;
	int *tmp_list, tmp_cnt, tmp_idag;	
	core_list *tmp_cores_list;//tmp_cores, 
	DDS_list *tmp_DDS,*prev_DDS;
	inter_list *tmp_inter_list;	
	#ifdef PLAT_SCC	
	int error, str_len;
	char error_str[64];
	#endif
	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_FINISH_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
	//if i am an idag, sig_finish should be proceed instantly, so i am dropping. 
	//On the other hand if an common node doesn't set its agent to -1 its not a big deal. (Hopefully)	
	if (core_inter_head[sender_id] != NULL && my_idag == -1) {
		fprintf(log_file,"I am in sig finish i don't know what to do with interaction %d\n",core_inter_head[sender_id]->type);
		
		if (my_idag == -1) {
			is_sender_idag = is_core_idag(sender_id);
		
			if (!is_sender_idag) {
				tmp_inter_list = core_inter_head[sender_id];
				while (tmp_inter_list != NULL) {
					if (tmp_inter_list->type == INIT_APP) {
						if (init_pending_head == NULL) {
							init_pending_head = (inter_list *) malloc(sizeof(inter_list));
							init_pending_tail = init_pending_head;
						} else {
							init_pending_tail->next = (inter_list *) malloc(sizeof(inter_list));
							init_pending_tail = init_pending_tail->next;
						}

						init_pending_tail->type = INIT_APP;
						init_pending_tail->data.new_app = core_inter_head[sender_id]->data.new_app;
						init_pending_tail->next = NULL;
					}
					
					core_inter_head[sender_id] = core_inter_head[sender_id]->next;
					if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
					
					fprintf(log_file, "I dismissed type = %d of sender = %d\n",tmp_inter_list->type,sender_id);

					free(tmp_inter_list);
					tmp_inter_list = core_inter_head[sender_id];
				}
			}
		}
	} 
		
	if (my_idag != -1) {
	      
		fprintf(log_file,"In here my_cores_cnt = %d app_state = %d nodes_ended_cnt = %d old_cores_cnt = %d cur_agent = %d active_working = %d\n",
			my_cores_count,app_state,nodes_ended_cnt,old_cores_cnt,cur_agent.my_agent,active_working_cores);

		/* I am worker and the application has finished */
		if (cur_agent.my_agent != -1 && cur_agent.my_agent == sender_id) {
			base_offset = -1;
			cur_agent.my_agent = -1;
			cur_agent.array_size = -1;
			cur_agent.work_bounds[0] = 0;
			cur_agent.work_bounds[1] = 0;
		} else {
			/*My worker finished his workload so i change the workload matrix to -1*/
			tmp_cores_list = my_cores;
			while (tmp_cores_list != NULL){
				if (tmp_cores_list->core_id == sender_id){
					one_core = tmp_cores_list->core_id;
					tmp_cores_list->workload[0] = -1;
					tmp_cores_list->workload[1] = -1;
					break;
				}
				tmp_cores_list = tmp_cores_list->next;
			}
			if (pending_workload[0] != -1 || pending_workload[1] != -1){
				fprintf(log_file,"\t\tsignal_handlers.c : I have pending workload %d | %d\n",pending_workload[0],pending_workload[1]);
				fprintf(log_file,"\t\tsignal_handlers.c : I am assigning workload to %d\n",one_core);
				if (core_inter_head[one_core] == NULL){
					core_inter_head[one_core] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[one_core] = core_inter_head[one_core];
				} else {
					core_inter_tail[one_core]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[one_core] = core_inter_tail[one_core]->next;
				}
				
				core_inter_tail[one_core]->type = APPOINT_WORK_NODE;
				core_inter_tail[one_core]->data.work_bounds[0] = pending_workload[0];
				core_inter_tail[one_core]->data.work_bounds[1] = pending_workload[1];
				fprintf(app_log_file,"%d (%d, %d), ",one_core,core_inter_tail[one_core]->data.work_bounds[0],core_inter_tail[one_core]->data.work_bounds[1]);
				core_inter_tail[one_core]->next = NULL;
				if (core_inter_head[one_core]->next == NULL) {
					scc_kill(one_core, SIG_APPOINT_WORK, core_inter_head[one_core]);
				} else {
					fprintf(log_file,"I am doing smth else with my working node %d in init inter1 = %d inter2 = %d\n",one_core,core_inter_head[one_core]->type,core_inter_head[one_core]->next->type);
				}
				pending_workload[0] = -1;
				pending_workload[1] = -1;
			}
			
			fprintf(log_file,"I enter here with app_state = %s\n",app_state_2_string(app_state));
			fprintf(log_file,"Some info:\n");
			fprintf(log_file,"Active working cores: %d\n",active_working_cores);
			fprintf(log_file,"Nodes ended count: %d\n",nodes_ended_cnt);
			
			if (my_app.workld <= 0) {
				fprintf(log_file,"In strange finish with agent %d and workld=%d\n",cur_agent.my_agent,my_app.workld);
			} else {
				if ((++nodes_ended_cnt) == active_working_cores) {
					if (--my_app.workld == 0) {
						gettimeofday(&time_val, NULL);
						cur_t = localtime(&time_val.tv_sec);
						fprintf(log_file,"[%d:%d:%d:%ld] Matrix mul is over in resizing.\n",
							cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,time_val.tv_usec);
						fprintf(app_log_file,"[%d:%d:%d:%ld] Matrix mul is over in resizing.\n",
							cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,time_val.tv_usec);

						app_state = APP_TERMINATED;
					} else {
						fprintf(log_file,"A matrix mul is over in resizing. Remaining workload is %d\n",my_app.workld);
						
						active_working_cores = 0;

						for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next) {
							one_core = tmp_cores_list->core_id;
							if (core_inter_head[one_core] == NULL){
								active_working_cores++;
							} else if (core_inter_head[one_core]->type == INIT_WORK_NODE_PENDING) {
								active_working_cores++;
							} else {
								fprintf(log_file,"I am doing smth else with my work node %d in resize appoint inter = %d\n",
								  one_core,core_inter_head[one_core]->type);
							}
						}

						if (active_working_cores > 0) {
							row_reached = 0;
#ifndef ARTIFICIAL_APPS_SIM
							per_core_rows = my_app.array_size / active_working_cores;
							rows_left = my_app.array_size % active_working_cores;
#else
							per_core_rows = (int) Speedup(my_app,active_working_cores+1); /* FIXME cutting off floating points -- +1 is because in Speedup calc it is -1*/
#endif
						} else if (my_cores_count == 2) { //I have only one working core, I know(?) that i wiil not give him up
							fprintf(log_file,"Case of only one core\n");
							active_working_cores = 1;
							one_core = my_cores->next->core_id;
							core_inter_tail[one_core]->next = (inter_list *) malloc(sizeof(inter_list));
							core_inter_tail[one_core] = core_inter_tail[one_core]->next;
							core_inter_tail[one_core]->type = APPOINT_WORK_NODE;

							/* FIXME I maintained that on 11.7.2017 - It seems to be useless because in a few lines it is re-appointed */
							core_inter_tail[one_core]->data.work_bounds[0] = 0;
#ifndef ARTIFICIAL_APPS_SIM
							core_inter_tail[one_core]->data.work_bounds[1] = my_app.array_size-1;
#else
							core_inter_tail[one_core]->data.work_bounds[1] = 1;
#endif
							core_inter_tail[one_core]->next = NULL;
						}
				
						gettimeofday(&time_val, NULL);
						cur_t = localtime(&time_val.tv_sec);
						fprintf(app_log_file,"[%d:%d:%d:%ld] A matrix mul is over in resizing. Remaining workload is %d active cores = %d\n",
							cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,time_val.tv_usec,my_app.workld,active_working_cores);
						
						tmp_cores_list = my_cores->next;
						while (tmp_cores_list != NULL) {
							one_core = tmp_cores_list->core_id;
							if (core_inter_head[one_core] == NULL){
								core_inter_head[one_core] = (inter_list *) malloc(sizeof(inter_list));
								core_inter_tail[one_core] = core_inter_head[one_core];
								core_inter_tail[one_core]->type = APPOINT_WORK_NODE;
							} else if (core_inter_head[one_core]->type == INIT_WORK_NODE_PENDING) {//{
								core_inter_tail[one_core]->type = INIT_WORK_NODE;
							} else {
								fprintf(log_file,"I am doing smth else with my work node %d in resize appoint inter = %d\n",
                     		one_core,core_inter_head[one_core]->type);
								
							}

							/* FIXME why is it core_inter_here */
							if (core_inter_tail[one_core]->type == APPOINT_WORK_NODE || core_inter_tail[one_core]->type == INIT_WORK_NODE) {
								/* FIXME have row_reached etc been defined if my_cores_count = 2 */
#ifndef ARTIFICIAL_APPS_SIM
								core_inter_tail[one_core]->data.work_bounds[0] = row_reached;
								tmp_cores_list->workload[0] = row_reached; /* 27.6.2016 dimos. If worker fails i have to know the workload given in order to reappoint */
								row_reached += per_core_rows;
								if (rows_left > 0) {
									row_reached++;
									rows_left--;
								}
								core_inter_tail[one_core]->data.work_bounds[1] = row_reached-1;
								tmp_cores_list->workload[1] = row_reached-1; /* 27.6.2016 dimos. If worker fails i have to know the workload given in order to reappoint */
#else								
								/* The workload[1] is practically the Speedup of the app. Internally in execution
								 * of the app this is translated to the correct time delay */
								core_inter_tail[one_core]->data.work_bounds[0] = 0;
								tmp_cores_list->workload[0] = 0;

								core_inter_tail[one_core]->data.work_bounds[1] = per_core_rows;
								tmp_cores_list->workload[1] = per_core_rows;
#endif								
								core_inter_tail[one_core]->next = NULL;

								if (core_inter_head[one_core]->next == NULL) {
									scc_kill(one_core, SIG_APPOINT_WORK, core_inter_head[one_core]);
									my_stats.msg_count++;
									my_stats.distance += distance(node_id,one_core);
								} else {
									fprintf(log_file,"I am doing smth else with my work node %d in resize appoint inter1 = %d inter2 = %d\n",
										one_core,core_inter_head[one_core]->type,core_inter_head[one_core]->next->type);
								}
							}

							tmp_cores_list = tmp_cores_list->next;
						}

						app_state = RUNNING;
						old_cores_cnt = 0;
						nodes_ended_cnt = 0;
					}
				} else {
					fprintf(log_file,"Just a plain node finish\n");
				}
			}
		}
	} else {
		nodes_cnt = sig_read_ar[2];
	  
		is_sender_idag = is_core_idag(sender_id);
		scc_kill(sender_id, SIG_ACK, NULL);
		my_stats.msg_count++;
		my_stats.distance += distance(node_id,sender_id);
		
		if (is_sender_idag == 0) {
			if (node_id == idag_id_arr[0]) {
				num_apps_terminated++;
				fprintf(log_file,"app_terminated = %d sender_id = %d\n",num_apps_terminated,sender_id);
				printf("app_terminated = %d sender_id = %d\n",num_apps_terminated,sender_id);
				fflush(stdout);
			}

			
			nodes_to_process = (int *) malloc(nodes_cnt * sizeof(int));
			if (nodes_cnt > 5) {
				*inc_cnt = *inc_cnt + 1;

				#ifdef PLAT_SCC
				error = RCCE_get((t_vcharp)(&sig_read_ar[LINE_SIZE]), (t_vcharp)(&sig_array[(cur_index_top+1)*LINE_SIZE]), LINE_SIZE * sizeof(int), node_id);
				if (error != RCCE_SUCCESS) {
					RCCE_error_string(error, error_str, &str_len);
					fprintf(log_file,"I got an error in get data in sig_ADD_CORES_DDS_handler from %d with descr %s\n",sender_id,error_str);
					fflush(log_file);
				}
				#else
				new_RCCE_get(sig_read_ar, sig_array, cur_index_top, LINE_SIZE, node_id);
				#endif
			}
			
			for(i=0; i<nodes_cnt; i++)
				nodes_to_process[i] = sig_read_ar[i+3];
		
			tmp_list = (int *) malloc(nodes_cnt * sizeof(int));
			fprintf(log_file,"nodes_cnt = %d\n",nodes_cnt);
			fprintf(log_file,"I am node_id=%d Cores to be processed: ",node_id);				
			for (i=0; i<nodes_cnt; i++)
				fprintf(log_file," %d",nodes_to_process[i]);
			fprintf(log_file,"\n");

			for (i=0; i<num_idags; i++)
				if (idag_id_arr[i] != node_id) {
					tmp_cnt = 0;
					tmp_idag = idag_id_arr[i];
					
					for (j=0; j<nodes_cnt; j++) {
						if (nodes_to_process[j] != -1 && idag_mask[nodes_to_process[j]] == tmp_idag) {
							tmp_list[tmp_cnt++] = nodes_to_process[j];
							nodes_to_process[j] = -1;
							processed_cnt++;
						}
					}
					
					if (tmp_cnt > 0) {
						if (core_inter_head[tmp_idag] == NULL){
							core_inter_head[tmp_idag] = (inter_list *) malloc(sizeof(inter_list));
							core_inter_tail[tmp_idag] = core_inter_head[tmp_idag];
						} else {
							core_inter_tail[tmp_idag]->next = (inter_list *) malloc(sizeof(inter_list));
							core_inter_tail[tmp_idag] = core_inter_tail[tmp_idag]->next;
						}

						core_inter_tail[tmp_idag]->type = REMOVE_APP;
						core_inter_tail[tmp_idag]->data.app_cores = (int *)malloc((tmp_cnt+2)*sizeof(int));
						core_inter_tail[tmp_idag]->data.app_cores[0] = tmp_cnt;
						core_inter_tail[tmp_idag]->data.app_cores[1] = sender_id;
						for (j=0; j<tmp_cnt; j++)
							core_inter_tail[tmp_idag]->data.app_cores[j+2] = tmp_list[j]; 
						core_inter_tail[tmp_idag]->next = NULL;
					
						if (core_inter_head[tmp_idag]->next == NULL) {
							scc_kill(tmp_idag, SIG_FINISH, core_inter_head[tmp_idag]);
							my_stats.msg_count++;
							my_stats.distance += distance(node_id,tmp_idag);
						} else {
							fprintf(log_file,"i did not send sig finish to %d with interaction = %d\n",tmp_idag,core_inter_head[tmp_idag]->type);
						}
					}    
				}
				      
			tmp_cnt = 0;
			tmp_idag = node_id;
			old_agent_id = sender_id;
			
			for (j=0; j<nodes_cnt; j++) {
				if (nodes_to_process[j] != -1 && idag_mask[nodes_to_process[j]] == tmp_idag) {
					tmp_list[tmp_cnt++] = nodes_to_process[j];
					nodes_to_process[j] = -1;
					processed_cnt++;
				}
			}
			
			fprintf(log_file,"nodes_cnt = %d processed_cnt = %d\n",nodes_cnt,processed_cnt);
			free(nodes_to_process);
		} else {
			tmp_cnt = nodes_cnt;//data_array_local[1];
			old_agent_id = sig_read_ar[3];//data_array_local[0];
			tmp_list = (int *) malloc(tmp_cnt * sizeof(int));

			if (nodes_cnt > 4) {
				*inc_cnt = *inc_cnt + 1;

				#ifdef PLAT_SCC
				error = RCCE_get((t_vcharp)(&sig_read_ar[LINE_SIZE]), (t_vcharp)(&sig_array[(cur_index_top+1)*LINE_SIZE]), LINE_SIZE * sizeof(int), node_id);
				if (error != RCCE_SUCCESS) {
					RCCE_error_string(error, error_str, &str_len);
					fprintf(log_file,"I got an error in get data in sig_ADD_CORES_DDS_handler from %d with descr %s\n",sender_id,error_str);
					fflush(log_file);
				}
				#else
				new_RCCE_get(sig_read_ar, sig_array, cur_index_top, LINE_SIZE, node_id);
				#endif
			}

			for (i=0; i<tmp_cnt; i++)
				tmp_list[i] = sig_read_ar[i+4];
			
			fprintf(log_file,"I am in Secondary sig_finish for %d and tmp_cnt=%d\n",old_agent_id,tmp_cnt);
		}
		
		fprintf(log_file,"My cores to be removed: ");
		for (i=0; i<tmp_cnt; i++)
			fprintf(log_file," %d",tmp_list[i]);
		fprintf(log_file,"\n");
#ifndef ADAM_SIM	
		/*Actual removal her */
		for (i=0; i<tmp_cnt; i++) {
			for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next) 
				if (tmp_cores_list->offered_to == old_agent_id && tmp_cores_list->core_id == tmp_list[i]) {
					tmp_cores_list->offered_to = -1;
					DDS->num_of_cores++;
					break;
				}
		}

		prev_DDS = DDS;
		tmp_DDS = DDS->next;
		while (tmp_DDS != NULL && tmp_DDS->agent_id != old_agent_id) {
			prev_DDS = tmp_DDS;
			tmp_DDS = tmp_DDS->next;
		}

		if (tmp_DDS != NULL) {
			DDS_count--;
			prev_DDS->next = tmp_DDS->next;
			if (tmp_DDS->next == NULL) DDS_tail = prev_DDS;
			free(tmp_DDS);
		} else {
			printf("--%d-- Agent %d that finished is not in my DDS\n", node_id, old_agent_id);
			
			fprintf(log_file,"Agent %d that finished is not in my DDS\n", old_agent_id);
			fprintf(log_file,"My current DDS list:\n");
			tmp_DDS = DDS;
			while (tmp_DDS != NULL){
				fprintf(log_file,"Agend_id : %d | Cores in my cluster : %d\n",tmp_DDS->agent_id,tmp_DDS->num_of_cores);
				tmp_DDS = tmp_DDS->next;
			}
		}
#else
		prev_DDS = DDS;
		tmp_DDS = DDS->next;
		while (tmp_DDS != NULL && tmp_DDS->agent_id != old_agent_id) {
			prev_DDS = tmp_DDS;
			tmp_DDS = tmp_DDS->next;
		}
	                
		/* tmp_DDS out */
		tmp_DDS->prev->next = tmp_DDS->next; /* Remove the link in the list */
		tmp_DDS->next->prev = tmp_DDS->prev;

		if (tmp_DDS != NULL) {
			DDS_count--;
			prev_DDS->next = tmp_DDS->next;
			if (tmp_DDS->next == NULL) DDS_tail = prev_DDS;
			free(tmp_DDS);
		} else {
			printf("--%d-- Agent %d that finished is not in my DDS\n", node_id, old_agent_id);

			fprintf(log_file,"Agent %d that finished is not in my DDS\n", old_agent_id);
			fprintf(log_file,"My current DDS list:\n");
			tmp_DDS = DDS;
			while (tmp_DDS != NULL){
				fprintf(log_file,"Agend_id : %d | Cores in my cluster : %d\n",tmp_DDS->agent_id,tmp_DDS->num_of_cores);
				tmp_DDS = tmp_DDS->next;
			}
		}

		 /* Insert sorted */
		if (tmp_DDS->num_of_cores > DDS->num_of_cores) { /* Check if it must be placed first */
			tmp_DDS2 = DDS;
			DDS = tmp_DDS;
			DDS->next = tmp_DDS2;
			DDS->prev = NULL;
			tmp_DDS2->prev = DDS;
		} else if (tmp_DDS->num_of_cores < DDS_tail->num_of_cores) { /* Check if it must be placed last */
			DDS_tail->next = tmp_DDS;
			tmp_DDS->prev = DDS_tail;
			DDS_tail = DDS_tail->next;
			DDS_tail->next = NULL;
		} else {
			tmp_DDS2 = DDS;
			while ((tmp_DDS2->next != NULL) && (tmp_DDS->num_of_cores <= tmp_DDS2->next->num_of_cores)) {
				/* tmp_DDS in between tmp_DDS2 and tmp_DDS2->next */
				tmp_DDS->next = tmp_DDS2->next;
				tmp_DDS2->next->prev = tmp_DDS;

				tmp_DDS2->next = tmp_DDS;
				tmp_DDS->prev = tmp_DDS2;
			}
		}

#endif		
		free(tmp_list);  
		fprintf(log_file,"My removal of agent complete node_id = %d sender_id=%d\n",node_id,sender_id);
		fprintf(log_file,"Number of agents in region = %d\n",DDS_count);	
		i=0;
		for (tmp_DDS = DDS; tmp_DDS != NULL; tmp_DDS = tmp_DDS->next) {
			fprintf(log_file,"Agent no %d is %d with %d cores\n",i,tmp_DDS->agent_id,tmp_DDS->num_of_cores);	
			i++;
		}
	}

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_FINISH_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
	return;
}

void sig_REJECT_handler(int sender_id){
  
	int agent_id, i, j;  
	inter_list *tmp_inter_list;
	core_list *tmp_cores_list, *tmp_core_list;
	target_list *tmp_target_list;

	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I entered sig_REJECT_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
	if (core_inter_head[sender_id] != NULL){
		fprintf(log_file, "[%d:%d:%d]: Interaction with sender=%d is %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,core_inter_head[sender_id]->type);
	}

	if (core_inter_head[sender_id] == NULL) { //When agent has changed
		fprintf(log_file,"I have null interaction with sender %d\n",sender_id);
	} else if (core_inter_head[sender_id]->type == IDAG_REQ_DDS_PENDING || core_inter_head[sender_id]->type == IDAG_REQ_DDS) {
		init_DDS_replies++;
		fprintf(log_file, "One init_req_dds has been rejected by %d!\n",sender_id);
		
		if (init_DDS_replies == init_DDS_idags) {
			for (tmp_target_list = init_targets_head; tmp_target_list != NULL; tmp_target_list = tmp_target_list->next) {
				agent_id = tmp_target_list->target;

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
				core_inter_tail[agent_id]->next = NULL;

				if (core_inter_head[agent_id]->next == NULL) {
					scc_kill(agent_id, SIG_REQ_CORES, core_inter_head[agent_id]);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,agent_id);
				} else {
					fprintf(log_file,"This reject init is not NULL interaction=%d interaction2=%d\n",core_inter_head[agent_id]->type,core_inter_head[agent_id]->next->type);
				}
			}
			
			my_settimer(INIT_NODE_INTERVAL);		
		} 

		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) {
			core_inter_tail[sender_id] = NULL;
		} else {
			send_next_signal(core_inter_head[sender_id], sender_id);
		}	
		free(tmp_inter_list);	
	} else if (core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS || core_inter_head[sender_id]->type == SELFOPT_IDAG_REQ_DDS_PENDING) {
		selfopt_DDS_replies++;
		fprintf(log_file, "One selfopt_req_dds has been rejected by %d!\n",sender_id);
	
		if (selfopt_DDS_replies == selfopt_DDS_idags) {
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
				core_inter_tail[agent_id]->next = NULL;

				if (core_inter_head[agent_id]->next == NULL) {
					//kill(pid_num[agent_id], SIG_REQ_CORES);
					scc_kill(agent_id, SIG_REQ_CORES, core_inter_head[agent_id]);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,agent_id);
				/*} else if (agent_id == sender_id && core_inter_head[agent_id]->next->type == SELFOPT_REQ_CORES_PENDING) {
					//kill(pid_num[agent_id], SIG_REQ_CORES);
					fprintf(log_file, "I send here awkard head\n");
					scc_kill(agent_id, SIG_REQ_CORES, core_inter_head[agent_id]->next);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,agent_id);*/
				} else {
					fprintf(log_file,"This reject selfopt is not NULL interaction=%d interaction2=%d\n",core_inter_head[agent_id]->type,core_inter_head[agent_id]->next->type);
				}
			}
			
			my_settimer(INIT_NODE_INTERVAL);
		}
		
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else //if (core_inter_head[sender_id]->type != AGENT_REQ_CORES_PENDING && core_inter_head[sender_id]->type != SELFOPT_REQ_CORES_PENDING)//far_req_max_man != sender_id && 
			send_next_signal(core_inter_head[sender_id], sender_id);
		free(tmp_inter_list);
	} else if (core_inter_head[sender_id]->type == AGENT_REQ_CORES_PENDING || core_inter_head[sender_id]->type == IDAG_FIND_IDAGS_PENDING 
		|| core_inter_head[sender_id]->type == SELFOPT_IDAG_FIND_IDAGS_PENDING || core_inter_head[sender_id]->type == SELFOPT_REQ_CORES_PENDING) {
	  
		if (core_inter_head[sender_id]->type == SELFOPT_IDAG_FIND_IDAGS_PENDING) {
			fprintf(log_file, "My only selfopt area was rejected. my state is %s my pending_state = %s\n",id2string(state),id2string(pending_state));
			if (pending_state == INIT_MANAGER) {
				state = INIT_MANAGER;
				pending_state = IDLE_AGENT;
			} else 
				state = AGENT_SELF_OPT;
		} else if (core_inter_head[sender_id]->type == IDAG_FIND_IDAGS_PENDING) {
			fprintf(log_file, "My only init area was rejected. my state is %s my pending_state = %s\n",id2string(state),id2string(pending_state));
			state = INIT_MANAGER_SEND_OFFERS;
		}
		
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);
		free(tmp_inter_list);
	} else if (core_inter_head[sender_id]->type == REP_IDAG_FIND_IDAGS) {
		free(core_inter_head[sender_id]->data.idags_in_reg);
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);
		free(tmp_inter_list);
	} else if (core_inter_head[sender_id]->type == REP_IDAG_REQ_DDS) {
		free(core_inter_head[sender_id]->data.agents_in_reg);
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
						
		if (app_state == APP_TERMINATED) {//app_terminated
			for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next)
							if (tmp_cores_list->offered_to != -1) break;

			if (tmp_cores_list == NULL && state == AGENT_ZOMBIE) state = AGENT_ENDING;
			else if (tmp_cores_list == NULL && pending_state == AGENT_ZOMBIE) pending_state = AGENT_ENDING;
		}

		free(core_inter_head[sender_id]->data.off_arr.offer_arr);
		tmp_inter_list = core_inter_head[sender_id];
		core_inter_head[sender_id] = core_inter_head[sender_id]->next;
		if (core_inter_head[sender_id] == NULL) core_inter_tail[sender_id] = NULL;
		else send_next_signal(core_inter_head[sender_id], sender_id);
		free(tmp_inter_list);
	} else if (core_inter_head[sender_id]->type == INIT_APP) {
		fprintf(log_file,"%d rejected my init_app req\n",sender_id);

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
	} else if (core_inter_head[sender_id]->type == APPOINT_WORK_NODE || core_inter_head[sender_id]->type == APPOINT_WORK_NODE_PENDING ||
		   core_inter_head[sender_id]->type == INIT_WORK_NODE || core_inter_head[sender_id]->type == INIT_WORK_NODE_PENDING){
		 FOR_MY_CORES_LIST{
			if (tmp_core_list->core_id == sender_id){
				pending_workload[0] = tmp_core_list->workload[0];
				pending_workload[1] = tmp_core_list->workload[1];
			}
		}
	} else {
		printf("I am %d in sig_reject and i have interaction with sender %d interaction = %d\n",node_id,sender_id,core_inter_head[sender_id]->type);
		fprintf(log_file,"I am in sig_reject and i have interaction with sender interaction = %d\n",core_inter_head[sender_id]->type);
	}
	
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_REJECT_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
	return;
}


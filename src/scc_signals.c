#include "include/scc_signals.h"
#include "include/my_rtrm.h"
#include "include/signal_handlers.h"
#include "include/paxos_signal_handlers.h"
#include "include/sig_aux.h"
#include "include/libfunctions.h"
#include "include/variables.h"
#include "include/controller_core.h"

int sig_read_ar[2 * LINE_SIZE];
int R = 0;
#ifdef IDAG_SLEEP
#include <time.h>
#define SLEEP_ROUND_DURATION_NSEC 50000000 //100000000
extern int my_idag;
#endif 


int find_sender_id(int SID){
	
	return SID % NUES;
}

int scc_kill(int target_ID, int sig, inter_list *ref_inter_list) {
	int sig_array_local[2 * LINE_SIZE], old_value, new_value, i, increase_cnt=1;
	int num_of_coworkers, counter;
	core_list *tmp_core_list;
	#ifdef PLAT_SCC
	int str_len;
	char error_str[64];
	#endif
	int error = 0;
		
	signals_disable();
	
	if (ref_inter_list == NULL)
	  fprintf(log_file,"target_ID %d and sig %s ref_inter_list is NULL\n",target_ID,sig2string(sig));

	//19.12.2015 Initial node sends SIG_INIT_APP when an app has terminated and receives no reply.
	//Thus we dont initialize our counter. Same for SIG_ACK and SIG_FINISH

	#if defined(tPFD) || defined(tEPFD)
	if (
	   (sig != SIG_INIT_APP) && (sig != SIG_ACK) && (sig != SIG_ADD_TO_DDS) && (sig != SIG_INIT)
	   && (sig != SIG_PREPARE_REQUEST) && (sig != SIG_PREPARE_ACCEPT_NO_PREVIOUS) && (sig != SIG_PREPARE_ACCEPT)
	   && (sig != SIG_ACCEPT_REQUEST) && (sig != SIG_ACCEPTED) && (sig != SIG_LEARN) && (sig != SIG_LEARN_ACK)
	   && (sig != SIG_LEARN_ACK_CONTR) && (sig != SIG_REINIT_APP) && (sig != SIG_CONTR_TO) && (sig != SIG_REMOVE_FROM_DDS)
	   && (sig != SIG_FAIL) && (sig != SIG_FINISH)
	   ){
		//suspected[target_ID]++;
		//alive[target_ID] = 0;
	}
	#endif
	
	for (i=0; i<LINE_SIZE; i++) 
		sig_array_local[i] = sig;
	if (sig != SIG_HEARTBEAT_REP && sig != SIG_HEARTBEAT_REQ){
		sig_array_local[1] = R++ * NUES + node_id;
	}else{
		sig_array_local[1] = node_id;
	}

	if (ref_inter_list != NULL) { //for a signal not associated with no inter_list interaction like SIG_FINISH
		if (strcmp(sig2string(sig),"SIG_HEARTBEAT_REQ") != 0 && strcmp(sig2string(sig), "SIG_HEARTBEAT_REP") != 0)
			fprintf(log_file,"\t\tI enter here with target_ID %d and sig %s {%d} type = %s\n",target_ID,sig2string(sig), sig_array_local[1], inter2string(ref_inter_list->type));
		
		if (ref_inter_list->type == INIT_APP) {
			sig_array_local[2] = ref_inter_list->data.new_app.id;
			sig_array_local[3] = ref_inter_list->data.new_app.app_type;
			sig_array_local[4] = ref_inter_list->data.new_app.workld;
			sig_array_local[5] = ref_inter_list->data.new_app.num_of_cores;
#ifndef ARTIFICIAL_APPS_SIM
			sig_array_local[6] = ref_inter_list->data.new_app.array_size;
#else
			memcpy(&sig_array_local[6],&ref_inter_list->data.new_app.var,sizeof(int));
			memcpy(&sig_array_local[7],&ref_inter_list->data.new_app.A,sizeof(int));
#endif
			//my_stats.message_size += sizeof(app);
			//clear = 1;
		} else if (ref_inter_list->type == IDAG_FIND_IDAGS_PENDING || ref_inter_list->type == SELFOPT_IDAG_FIND_IDAGS_PENDING) {//I am the requesting common node
			sig_array_local[2] = ref_inter_list->data.reg.C;
			sig_array_local[3] = ref_inter_list->data.reg.r;
			
			fprintf(log_file, "a C=%d r=%d\n",ref_inter_list->data.reg.C,ref_inter_list->data.reg.r);
			
			//my_stats.message_size += sizeof(region);
		} else if (ref_inter_list->type == IDAG_REQ_DDS_PENDING || ref_inter_list->type == SELFOPT_IDAG_REQ_DDS_PENDING 
			|| ref_inter_list->type == DEBUG_IDAG_REQ_DDS) {
		  
			sig_array_local[2] = ref_inter_list->data.reg.C;
			sig_array_local[3] = ref_inter_list->data.reg.r;
			
			fprintf(log_file, "a C=%d r=%d\n",ref_inter_list->data.reg.C,ref_inter_list->data.reg.r);
			//my_stats.message_size += sizeof(region);
		} else if (ref_inter_list->type == REP_IDAG_FIND_IDAGS) {//I am the idag
			sig_array_local[2] = ref_inter_list->data.idags_in_reg[num_idags];
			if (sig_array_local[2] > 5){
				increase_cnt = 2;
			}
			counter=3;
			for (i=0; i < num_idags; i++) {
				if (ref_inter_list->data.idags_in_reg[i]) {
					sig_array_local[counter++] = idag_id_arr[i];
					fprintf(log_file, "\t\tidag=%d\n",idag_id_arr[i]);
					my_stats.message_size += sizeof(int);
				}
			}

		} else if (ref_inter_list->type == REP_IDAG_REQ_DDS) {
			if (ref_inter_list->data.agents_in_reg == NULL)
				sig_array_local[2] = DDS_count;
			else	
				sig_array_local[2] = ref_inter_list->data.agents_in_reg[0];
		} else if (ref_inter_list->type == AGENT_REQ_CORES_PENDING) {
			sig_array_local[2] = init_app.id;
			sig_array_local[3] = init_app.app_type;
			sig_array_local[4] = init_app.workld;
			sig_array_local[5] = init_app.num_of_cores;
#ifndef ARTIFICIAL_APPS_SIM
			sig_array_local[6] = init_app.array_size;
#else
			memcpy(&sig_array_local[6],&init_app.var,sizeof(int));
			memcpy(&sig_array_local[7],&init_app.A,sizeof(int));
#endif			
			//fprintf(log_file, "Cores=%d r=%d\n",sig_array_local[5],ref_inter_list->data.reg_arr.region_arr[0].r);
			//free(ref_inter_list->data.reg_arr.region_arr);
		} else if (ref_inter_list->type == SELFOPT_REQ_CORES_PENDING) {
			sig_array_local[2] = my_app.id;
			sig_array_local[3] = my_app.app_type;
			sig_array_local[4] = my_app.workld;
			sig_array_local[5] = my_app.num_of_cores;
#ifndef ARTIFICIAL_APPS_SIM
			sig_array_local[6] = my_app.array_size;
#else
			memcpy(&sig_array_local[6],&my_app.var,sizeof(int));
			memcpy(&sig_array_local[7],&my_app.A,sizeof(int));
#endif
			//fprintf(log_file, "Cores=%d r=%d\n",sig_array_local[5],ref_inter_list->data.reg_arr.region_arr[0].r);
			//free(ref_inter_list->data.reg_arr.region_arr);
		} else if (ref_inter_list->type == REP_AGENT_REQ_CORES) {//I am the agent
			/* pre write these info and keep data_array only for cores. Besides, a great amount of offers will be zero */
			sig_array_local[2] = ref_inter_list->data.off_arr.num_of_offers;
			fprintf(log_file, "num_of_offers=%d\n",ref_inter_list->data.off_arr.num_of_offers);
			
			if (ref_inter_list->data.off_arr.num_of_offers > 0) {		
				sig_array_local[3] = ref_inter_list->data.off_arr.offer_arr[0].num_of_cores;
				fprintf(log_file, "num_of_cores=%d\n",ref_inter_list->data.off_arr.offer_arr[0].num_of_cores);
				
				memcpy(&sig_array_local[4],&ref_inter_list->data.off_arr.offer_arr[0].spd_loss,sizeof(int));
				fprintf(log_file, "spd_loss=%0.2f\n",ref_inter_list->data.off_arr.offer_arr[0].spd_loss);
			}	
		} else if (ref_inter_list->type == INIT_WORK_NODE) {
			if (ref_inter_list->data.work_bounds[0] != -1) {
				gettimeofday(&time_val, NULL);
				cur_t = localtime(&time_val.tv_sec);
				//fprintf(app_log_file,"[%d:%d:%d:%ld] I init work to %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,time_val.tv_usec,target_ID);
			
				/* 2.7.2016 - Changed by dimos - Instead of valid i send the app_id */
				sig_array_local[2] = my_app.id;
				sig_array_local[3] = node_id;
				sig_array_local[4] = my_app.app_type;
				sig_array_local[5] = ref_inter_list->data.work_bounds[0];
				sig_array_local[6] = ref_inter_list->data.work_bounds[1];
#ifndef ARTIFICIAL_APPS_SIM
				sig_array_local[7] = my_app.array_size;
#endif
				/********/
				fprintf(log_file, "work_time1=%d work_time2=%d\n",ref_inter_list->data.work_bounds[0],ref_inter_list->data.work_bounds[1]);
				//my_stats.message_size += 5 * sizeof(int);
			} else {
				sig_array_local[2] = -1;
				fprintf(log_file, "i=%d\n",sig_array_local[0]);
				//my_stats.message_size += sizeof(int);
			}
			//clear = 1;
		} else if (ref_inter_list->type == APPOINT_WORK_NODE) {
			if (ref_inter_list->data.work_bounds[0] != -1) {
				gettimeofday(&time_val, NULL);
				cur_t = localtime(&time_val.tv_sec);
				//fprintf(app_log_file,"[%d:%d:%d:%ld] I appoint work to %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,time_val.tv_usec,target_ID);
				
				/* 2.7.2016 - Changed by dimos - Instead of valid i send the app_id */
				sig_array_local[2] = my_app.id;
				/* 12.7.2017 Changed the order to be consistent with INIT_WORK_NODE */
				sig_array_local[4] = my_app.app_type;	
				sig_array_local[5] = ref_inter_list->data.work_bounds[0];
				sig_array_local[6] = ref_inter_list->data.work_bounds[1];
				
				fprintf(log_file, "work_time1=%d work_time2=%d\n",ref_inter_list->data.work_bounds[0],ref_inter_list->data.work_bounds[1]);
				//my_stats.message_size += 3 * sizeof(int);
			} else {
				sig_array_local[2] = -1;
				//my_stats.message_size += sizeof(int);
			}
			//clear = 1;
		} else if (ref_inter_list->type == REP_AGENT_OFFER_SENT) {
			fprintf(log_file, "I have to reply %d for %d offers\n",target_ID,ref_inter_list->data.offer_acc_array[0]);
						
			sig_array_local[2] = ref_inter_list->data.offer_acc_array[1];
			fprintf(log_file, "offer_ans=%d\n",ref_inter_list->data.offer_acc_array[1]);
			//free(ref_inter_list->data.offer_acc_array);
			//clear = 1;
		} else if (ref_inter_list->type == IDAG_ADD_CORES_DDS) {
			sig_array_local[2] = ref_inter_list->data.app_cores[0];
			fprintf(log_file, "app_cores=%d\n",ref_inter_list->data.app_cores[0]);
			
			/* FIXME change position of orig_sender and new_owner in the creation of the list */
			//8 elements available,  3 allready in use
			if (my_idag != -1) {
				if (ref_inter_list->data.app_cores[0] > 5){
					increase_cnt=2;
				}
			  
				for (i=1; i<=ref_inter_list->data.app_cores[0]; i++){
					sig_array_local[i+2] = ref_inter_list->data.app_cores[i];
					fprintf(log_file, "core=%d\n",ref_inter_list->data.app_cores[i]);
				}
			} else {
				//I am an idag and i have to send to other idags my original sender		
				sig_array_local[3] = ref_inter_list->data.app_cores[ref_inter_list->data.app_cores[0]+1];
				fprintf(log_file, "orig_sender=%d\n",ref_inter_list->data.app_cores[ref_inter_list->data.app_cores[0]+1]);
				
				if (ref_inter_list->data.app_cores[0] > 4)
					increase_cnt++;
				
				for (i=1; i<=ref_inter_list->data.app_cores[0]; i++){
					sig_array_local[i+3] = ref_inter_list->data.app_cores[i];//LINE_SIZE+i-1
					fprintf(log_file, "core=%d\n",ref_inter_list->data.app_cores[i]);
				}
			}
		} else if (ref_inter_list->type == IDAG_REM_CORES_DDS) {
			//fprintf(log_file, "I am in add/remove/remove_app to %d with %d cores\n",sender_id,tmp_inter_list->data.app_cores[0]);			
			sig_array_local[2] = ref_inter_list->data.app_cores[0];
			fprintf(log_file, "app_cores=%d\n",ref_inter_list->data.app_cores[0]);
			
			/* FIXME change position of orig_sender and new_owner in the creation of the list */ 
			//8 elements available,  3 allready in use
			if (my_idag != -1) {
				sig_array_local[3] = ref_inter_list->data.app_cores[ref_inter_list->data.app_cores[0]+1];
				fprintf(log_file, "new_owner=%d\n",ref_inter_list->data.app_cores[ref_inter_list->data.app_cores[0]+1]);
			  
				if (ref_inter_list->data.app_cores[0] > 4)
					increase_cnt++;
			  
				for (i=1; i<=ref_inter_list->data.app_cores[0]; i++) {			
					sig_array_local[i+3] = ref_inter_list->data.app_cores[i];//LINE_SIZE+i-1
					fprintf(log_file, "core=%d\n",ref_inter_list->data.app_cores[i]);
				}	
			} else {
				//I am an idag and i have to send to other idags my original sender
				sig_array_local[3] = ref_inter_list->data.app_cores[ref_inter_list->data.app_cores[0]+1];
				fprintf(log_file, "orig_sender=%d\n",ref_inter_list->data.app_cores[ref_inter_list->data.app_cores[0]+1]);
				sig_array_local[4] = ref_inter_list->data.app_cores[ref_inter_list->data.app_cores[0]+2];
				fprintf(log_file, "new_owner=%d\n",ref_inter_list->data.app_cores[ref_inter_list->data.app_cores[0]+2]);
			  
				if (ref_inter_list->data.app_cores[0] > 3)
					increase_cnt++;
			  
				for (i=1; i<=ref_inter_list->data.app_cores[0]; i++) {			
					sig_array_local[i+4] = ref_inter_list->data.app_cores[i];//LINE_SIZE+i-1
					fprintf(log_file, "core=%d\n",ref_inter_list->data.app_cores[i]);
				}
			}
		} else if (ref_inter_list->type == REMOVE_APP) {
			//fprintf(log_file, "I am in add/remove/remove_app to %d with %d cores\n",sender_id,tmp_inter_list->data.app_cores[0]);			
			sig_array_local[2] = ref_inter_list->data.app_cores[0];
			fprintf(log_file, "app_cores=%d\n",ref_inter_list->data.app_cores[0]);
			
			//8 elements available,  3 already in use
			if (my_idag != -1) {
				if (ref_inter_list->data.app_cores[0] > 5)
					increase_cnt++;
			  
				for (i=1; i<=ref_inter_list->data.app_cores[0]; i++){			
					sig_array_local[i+2] = ref_inter_list->data.app_cores[i];//LINE_SIZE+i-1
					fprintf(log_file, "core=%d\n",ref_inter_list->data.app_cores[i]);
				}
			} else {
				//I am an idag and i have to send to other idags my original sender
				sig_array_local[3] = ref_inter_list->data.app_cores[1];
				fprintf(log_file, "or_sender=%d\n",ref_inter_list->data.app_cores[0]);
				
				if (ref_inter_list->data.app_cores[0] > 4)
					increase_cnt++;
				
				for (i=2; i<=(ref_inter_list->data.app_cores[0]+1); i++){			
					sig_array_local[i+2] = ref_inter_list->data.app_cores[i];//LINE_SIZE+i-1
					fprintf(log_file, "core=%d\n",ref_inter_list->data.app_cores[i]);
				}
			}
		/* PAXOS INTERACTIONS */
		} else if (ref_inter_list->type == PREPARE_REQUEST){
			paxos_node_stats.msg_count++;
			sig_array_local[2] = ref_inter_list->data.proposal_number; // proposal_number
		} else if (ref_inter_list->type == PREPARE_ACCEPT){
			paxos_node_stats.msg_count++;
			sig_array_local[2] = ref_inter_list->data.accepted_values[0]; // highest accepted proposal_number
			sig_array_local[3] = ref_inter_list->data.accepted_values[1]; // highest accepted value
			sig_array_local[4] = ref_inter_list->data.accepted_values[2]; // state
		} else if (ref_inter_list->type == PREPARE_ACCEPT_NO_PREVIOUS){
			paxos_node_stats.msg_count++;
			sig_array_local[2] = ref_inter_list->data.accepted_values[0]; // highest accepted proposal_number
			sig_array_local[3] = ref_inter_list->data.accepted_values[1]; // highest accepted value
			sig_array_local[4] = ref_inter_list->data.accepted_values[2]; // state
		} else if (ref_inter_list->type == ACCEPT_REQUEST){
			paxos_node_stats.msg_count++;
			sig_array_local[2] = ref_inter_list->data.accepted_values[0];
			sig_array_local[3] = ref_inter_list->data.accepted_values[1];
		} else if (ref_inter_list->type == ACCEPTED){
			paxos_node_stats.msg_count++;
			sig_array_local[2] = ref_inter_list->data.accepted_values[0];
			sig_array_local[3] = ref_inter_list->data.accepted_values[1];
		} else if (ref_inter_list->type == LEARN){
			paxos_node_stats.msg_count++;
			sig_array_local[2] = ref_inter_list->data.accepted_values[0];
			sig_array_local[3] = ref_inter_list->data.accepted_values[1];
		} else if (ref_inter_list->type == LEARN_ACK_CONTR){
			paxos_node_stats.msg_count++;
			sig_array_local[2] = ref_inter_list->data.controller_index;
		} else if (ref_inter_list-> type == REINIT_APP){
			paxos_node_stats.msg_count++;
			sig_array_local[2] = ref_inter_list->data.reappointed_app.id;
			sig_array_local[3] = ref_inter_list->data.reappointed_app.workld;
			sig_array_local[4] = ref_inter_list->data.reappointed_app.num_of_cores;
#ifndef ARTIFICIAL_APPS_SIM
			sig_array_local[5] = ref_inter_list->data.reappointed_app.array_size;	
#endif
		} else if (ref_inter_list->type == ADD_TO_DDS){
			paxos_node_stats.msg_count++;
			sig_array_local[2] = ref_inter_list->data.workers_info[0];
			counter = ref_inter_list->data.workers_info[0];
			if (counter > 5){
				fprintf(log_file,"\t\tI am sending more than 5 workers. I have to use two lines!\n");
				increase_cnt = 2;
			}
			while (counter > 0){
				sig_array_local[2+counter] = ref_inter_list->data.workers_info[counter];
				fprintf(log_file,"\t\tsig_array_local[%d] = %d\n",2+counter,sig_array_local[2+counter]);
				counter--;
			 }
			 fprintf(log_file,"MY COUNTER = %d\n",sig_array_local[2]);
		} else if (ref_inter_list->type == REMOVE_FROM_DDS){
			paxos_node_stats.msg_count++;
		} else if (ref_inter_list->type == PAXOS_STATS_REQ){
			paxos_node_stats.msg_count++;
		} else if (ref_inter_list->type == PAXOS_STATS_REP){
			paxos_node_stats.msg_count++;
			sig_array_local[2] = ref_inter_list->data.paxos_stats[0];
			sig_array_local[3] = ref_inter_list->data.paxos_stats[1];
		} else if (ref_inter_list->type == HEARTBEAT_REQ){
			  paxos_node_stats.fd_msg_count++;
		} else if (ref_inter_list->type == HEARTBEAT_REP){
			  paxos_node_stats.fd_msg_count++;
		} else if (ref_inter_list->type == INIT_AGENT){
			sig_array_local[2] = ref_inter_list->data.one_app.new_app.id;
			sig_array_local[3] = ref_inter_list->data.one_app.new_app.app_type;
			sig_array_local[4] = ref_inter_list->data.one_app.new_app.workld;
			sig_array_local[5] = ref_inter_list->data.one_app.new_app.num_of_cores;
#ifndef ARTIFICIAL_APPS_SIM
			sig_array_local[6] = ref_inter_list->data.one_app.new_app.array_size;
#else
			memcpy(&sig_array_local[6],&ref_inter_list->data.one_app.new_app.var,sizeof(int));
			memcpy(&sig_array_local[7],&ref_inter_list->data.one_app.new_app.A,sizeof(int));
#endif
		}
		/* END*/
		
	}
	
	#ifdef PLAT_SCC
	if (strcmp(sig2string(sig),"SIG_HEARTBEAT_REQ") != 0 && strcmp(sig2string(sig), "SIG_HEARTBEAT_REP") != 0){
		fprintf(log_file,"Trying to acquire lock %d\n",target_ID);
	}
	RCCE_acquire_lock(target_ID);
	//fprintf(log_file,"I successfully acquired lock %d\n",target_ID);
	RCCE_shflush();
	old_value = index_bottom[target_ID];
	if (strcmp(sig2string(sig),"SIG_HEARTBEAT_REQ") != 0 && strcmp(sig2string(sig), "SIG_HEARTBEAT_REP") != 0){
		fprintf(log_file,"I read bottom index %d increase_cnt=%d and target_ID %d\n",old_value,increase_cnt,target_ID);
	}

	error = RCCE_put((t_vcharp)(&sig_array[old_value*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), increase_cnt * LINE_SIZE * sizeof(int), target_ID);
	if (error != RCCE_SUCCESS) {
		RCCE_error_string(error, error_str, &str_len);
		fprintf(log_file,"I got an error in put with descr %s\n",error_str);
		fflush(log_file);	
	}
	
	new_value = (old_value + increase_cnt) % MAX_SIGNAL_LIST_LEN;
	index_bottom[target_ID] = new_value;
	//RCCE_shflush();	
	RCCE_release_lock(target_ID);

	if (strcmp(sig2string(sig),"SIG_HEARTBEAT_REQ") != 0 && strcmp(sig2string(sig), "SIG_HEARTBEAT_REP") != 0){
		fprintf(log_file,"I leave\n");
	}
	#else
	
	sem_wait(&scc_lock[target_ID]);
	old_value = index_bottom[target_ID];
	int mem_offset = target_ID * MAX_SIGNAL_LIST_LEN * LINE_SIZE;
	mem_offset += old_value * LINE_SIZE;
	if (strcmp(sig2string(sig),"SIG_HEARTBEAT_REQ") != 0 && strcmp(sig2string(sig), "SIG_HEARTBEAT_REP") != 0){
		fprintf(log_file,"I read bottom index %d increase_cnt=%d\n",old_value,increase_cnt);
	}
	for (i=0; i<increase_cnt * LINE_SIZE; i++){
		if (old_value == MAX_SIGNAL_LIST_LEN-1 && increase_cnt == 2){
			if (i <= 7){
				sig_array[mem_offset + i] = sig_array_local[i];
			}else{
				sig_array[(target_ID * MAX_SIGNAL_LIST_LEN * LINE_SIZE) + i - LINE_SIZE] = sig_array_local[i];
			}
		}else{
			sig_array[mem_offset + i] = sig_array_local[i];
		}
	}
		
	new_value = (old_value + increase_cnt) % MAX_SIGNAL_LIST_LEN;
	index_bottom[target_ID] = new_value;
	
	sem_post(&scc_lock[target_ID]);
	
	#endif
	
	signals_enable();

	return error;
}

void scc_signals_check(void) {
	int sender_id, tmp_bottom, i, mem_offset, increase_cnt=1;
	char *sig_buf, *st_buf;
	//int sig_read_ar[LINE_SIZE]
	#ifdef PLAT_SCC
	int error, str_len; 
	#endif
	char error_str[64];
	
	signals_disable();
	#ifdef PLAT_SCC
	RCCE_acquire_lock(node_id);
	#else
	sem_wait(&scc_lock[node_id]);
	#endif

	/* Overflow check */
	tmp_bottom = index_bottom[node_id];

	//last_index_bottom = tmp_bottom;

	while (index_top != tmp_bottom) {

		#ifdef EXTRA_DELAY
		scc_pause;
		#endif
	
		#ifdef PLAT_SCC
		error = RCCE_get((t_vcharp)(&sig_read_ar[0]), (t_vcharp)(&sig_array[index_top*LINE_SIZE]), LINE_SIZE * sizeof(int), node_id);
		RCCE_release_lock(node_id);

		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			fprintf(log_file,"I got an error in get from %d with descr %s\n",sender_id,error_str);
			fflush(log_file);
		#else
		mem_offset = (node_id * MAX_SIGNAL_LIST_LEN * LINE_SIZE) + (index_top * LINE_SIZE); //node offset
		for (i = 0; i < LINE_SIZE; i++)
			sig_read_ar[i] = sig_array[mem_offset + i];
		
		sem_post(&scc_lock[node_id]);
		error_str[0] = '0';

		if (error_str[0] == '1') {
			printf("??\n");
		#endif
		} else {
			sender_id = find_sender_id(sig_read_ar[1]);
		  	if (sig_read_ar[0] != NO_SIG){
				st_buf = id2string(state);
				sig_buf = sig2string(sig_read_ar[0]);
				if (strcmp(sig2string(sig_read_ar[0]),"Unknown Sig") == 0){
						fprintf(log_file,"I read Unknown sig and its number is %d\n",sig_read_ar[0]);
				}else if (strcmp(sig2string(sig_read_ar[0]),"SIG_HEARTBEAT_REQ") != 0 && strcmp(sig2string(sig_read_ar[0]), "SIG_HEARTBEAT_REP") != 0){
					fprintf(log_file,"\t\tmy index_top = %d and index_bottom = %d\n",index_top,tmp_bottom);
					fprintf(log_file,"\t\tI read sig %s {%d} - %d from sender_id %d. Current state = %s\n",sig_buf, sig_read_ar[1], sig_read_ar[0], sender_id, st_buf);
					//fprintf(log_file,"\t\tindex_top[%d]=%d.\n\t\tI read from position %d till %d\n",node_id,index_top,mem_offset,mem_offset+7);
				}
			}
			
			
			/* Failure Detector */
			#if defined(tPFD) || defined(tEPFD)
			alive[sender_id] = 1;
			suspected[sender_id] = 0;
			#endif
			/********************/
			
			if (paxos_state == FAILED_CORE && sig_read_ar[0] == SIG_RECOVER){
				fail_flag = 1;
			} else if (paxos_state != FAILED_CORE) {
				if (sig_read_ar[0] == SIG_INIT) {
					sig_INIT_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_ACK) {
					sig_ACK_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_TERMINATE) {
					sig_TERMINATE_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_INIT_APP) {
					sig_INIT_APP_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_IDAG_FIND_IDAGS) {
					sig_IDAG_FIND_IDAGS_handler(sender_id, &increase_cnt, index_top);
				} else if (sig_read_ar[0] == SIG_REQ_DDS) {
					sig_REQ_DDS_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_REQ_CORES) {
					sig_REQ_CORES_handler(sender_id, &increase_cnt, index_top); /* increase_cnt was put like this because in response there is no need for increase */ 
				} else if (sig_read_ar[0] == SIG_REP_OFFERS) {
					sig_REP_OFFERS_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_INIT_AGENT) {
					sig_INIT_AGENT_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_ADD_CORES_DDS) {
					sig_ADD_CORES_DDS_handler(sender_id, &increase_cnt, index_top);
				} else if (sig_read_ar[0] == SIG_REM_CORES_DDS) {
					sig_REM_CORES_DDS_handler(sender_id, &increase_cnt, index_top);
				} else if (sig_read_ar[0] == SIG_APPOINT_WORK) {
					sig_APPOINT_WORK_handler(sender_id, &increase_cnt, index_top);
				} else if (sig_read_ar[0] == SIG_FINISH) {
					sig_FINISH_handler(sender_id, &increase_cnt, index_top);
				} else if (sig_read_ar[0] == SIG_REJECT) {
					sig_REJECT_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_APP_TERMINATED) {
					num_apps_terminated++;
					fprintf(log_file,"app_terminated = %d sender_id = %d\n",num_apps_terminated,sender_id);
					printf("app_terminated = %d sender_id = %d\n",num_apps_terminated,sender_id);
				/* PAXOS SIGNALS HANDLING */
				} else if (sig_read_ar[0] == SIG_PREPARE_REQUEST) {
					sig_PREPARE_REQUEST_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_PREPARE_ACCEPT_NO_PREVIOUS) {
					sig_PREPARE_ACCEPT_NO_PREVIOUS_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_PREPARE_ACCEPT) {
					sig_PREPARE_ACCEPT_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_ACCEPT_REQUEST) {
					sig_ACCEPT_REQUEST_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_ACCEPTED) {
					sig_ACCEPTED_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_LEARN)	{
					sig_LEARN_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_REINIT_APP) {
					sig_REINIT_APP_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_CONTR_TO) {
					sig_CONTR_TO_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_REMOVE_FROM_DDS) {
					sig_REMOVE_FROM_DDS_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_ADD_TO_DDS) {
					sig_ADD_TO_DDS_handler(sender_id,&increase_cnt,index_top);
				} else if (sig_read_ar[0] == SIG_HEARTBEAT_REQ) {
					sig_HEARTBEAT_REQ_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_HEARTBEAT_REP) {
					sig_HEARTBEAT_REP_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_FAIL) {
					sig_FAIL_handler();
				} else if (sig_read_ar[0] == SIG_PAXOS_STATS_REQ) {
					sig_PAXOS_STATS_REQ_handler(sender_id);
				} else if (sig_read_ar[0] == SIG_PAXOS_STATS_REP) {
					sig_PAXOS_STATS_REP_handler(sender_id);
				/* END OF PAXOS SIGNAL HANDLING */
				} else if (sig_read_ar[0] != NO_SIG) {
					fprintf(log_file,"I read smth different than no_sig which is %d from %d\n",sig_read_ar[0],sender_id);
					fflush(log_file);
				}
			}

			if (sig_read_ar[0] != NO_SIG)
				if (strcmp(sig2string(sig_read_ar[0]),"SIG_HEARTBEAT_REQ") != 0 && strcmp(sig2string(sig_read_ar[0]), "SIG_HEARTBEAT_REP") != 0)
					fprintf(log_file,"I invalidated sender_ids %d signals increase_cnt=%d\n",sender_id,increase_cnt);

			index_top = (index_top + increase_cnt) % MAX_SIGNAL_LIST_LEN;
			increase_cnt = 1;

			#ifdef PLAT_SCC
			RCCE_acquire_lock(node_id);
			#else
			sem_wait(&scc_lock[node_id]);
			#endif

			tmp_bottom = index_bottom[node_id];
			
			if (paxos_state ==  NEW_IDAG){
				fprintf(log_file,"\t\tmy index_top = %d and index_bottom = %d\n",index_top,tmp_bottom);
				break;
			}
			

		}
	}

	#ifdef PLAT_SCC
	RCCE_release_lock(node_id);
	#else
	sem_post(&scc_lock[node_id]);
	#endif
	if (paxos_state == NEW_AGENT){
		//paxos_state = PAXOS_ACTIVE;
		//common_node_actions(local_scen_directory, local_scen_num);
	}
	
	//fprintf(log_file,"paxos_state : %s", id2string(paxos_state));
	if (paxos_state == NEW_IDAG){
		fprintf(log_file, "My state is %s %s\n", id2string(state), id2string(paxos_state));
		rollback();
		fprintf(log_file, "My state 2 is %s %s\n", id2string(state), id2string(paxos_state));
		signals_enable();
		fprintf(log_file, "My state 3 is %s %s\n", id2string(state), id2string(paxos_state));
		#ifndef PLAT_LINUX
		idle_agent_actions("", "");
		#else
		idle_agent_actions("", "",0,0); /* FIXME Change to correct last two arguments!!! */
		#endif
	}
	signals_enable();
}

void scc_pause(void) {
	int dummy=0, i;
	
	#ifdef IDAG_SLEEP
	struct timespec ts;

	if (my_idag == -1) { /* I am a Controller core */
		/* sleep(1); */
		ts.tv_sec = 0;
		ts.tv_nsec = SLEEP_ROUND_DURATION_NSEC;
		nanosleep(&ts, NULL);
	}
	#endif	
	
	#ifdef LOW_VOLTAGE_0
	if ((node_id >= 0 && node_id <= 3) || (node_id >= 12 && node_id <= 15))
		for (i=0; i<125; i++) //667
			dummy++;
	else
		for (i=0; i<1000; i++)
			dummy++; 
	#else
	for (i=0; i<1000; i++)
		//for(j=0; j<1000; j++)
			dummy++;
	#endif	
}	

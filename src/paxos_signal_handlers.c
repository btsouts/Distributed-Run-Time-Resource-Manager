#include "include/paxos_signal_handlers.h"

#include "include/my_rtrm.h"
#include "include/libfunctions.h"
#include "include/sig_aux.h"
#include "include/scc_signals.h"
#include "include/controller_core.h"
#include "include/common_core.h"
#include "include/idag_defs.h"
#include "include/signal_handlers.h"
#include "include/variables.h"
#include "include/macros.h"
#include "include/structs.h"

int faulty_core = -1;
int first_time = 0;
int pending_workload[2] = {-1,-1};
int proposal_number_personal;

core_states paxos_state;

acceptor_var acceptor_vars = {-1,-1,-1};

proposer_var proposer_vars = {-1,-1,0,0,NULL};

int fail_flag = 0;

struct timeval fail_time_val;


static char local_scen_directory[SCEN_DIR_SIZE], local_scen_num[SCEN_NUM_SIZE];

#ifdef PLAT_SCC
char error_str[64];
int error, str_len, sig_array_local[LINE_SIZE];
#endif

int leader_preference(){
  
	coworkers_list *tmp_cowork_list;
	int num_of_coworkers = 0;
	
	if (state == IDLE_CORE){

		return 60;
	}
	else if (state == INIT_MANAGER ||
		 state == INIT_MANAGER_SEND_OFFERS ||
		 state == IDLE_INIT_MAN ||
		 state == INIT_MAN_CHK_OFFERS){
		
		return 50;
	}
	else if (state == WORKING_NODE && pending_state == NO_PENDING_STATE){
	  
		if (coworkers != NULL)
			FOR_MY_COWORKERS_LIST num_of_coworkers++;
		
		if (num_of_coworkers == 1)
			return 1;
		else
			return (40 + num_of_coworkers);
	}
	else if (state == WORKING_NODE && (
		 pending_state == INIT_MANAGER ||
		 pending_state == INIT_MANAGER_SEND_OFFERS ||
		 pending_state == IDLE_INIT_MAN ||
		 pending_state == INIT_MAN_CHK_OFFERS)){
		
		if (coworkers != NULL)
			FOR_MY_COWORKERS_LIST num_of_coworkers++;
		
		if (num_of_coworkers == 1)
			return 1;
		else{
			return (30+num_of_coworkers);
		}
	 }
	 else if (state == WORKING_NODE && (
		pending_state == IDLE_AGENT ||
		pending_state == IDLE_AGENT_WAITING_OFF ||
		pending_state == AGENT_INIT_STATE ||
		pending_state == AGENT_SELF_OPT ||
		pending_state == AGENT_SELF_CHK_OFFERS ||
		pending_state == AGENT_ENDING ||
		pending_state == IDAG_ENDING ||
		pending_state == AGENT_ZOMBIE ||
		pending_state == AGENT_INIT_APP_INIT ||
		pending_state == AGENT_INIT_CHK_OFFERS ||
		pending_state == AGENT_INIT_IDLE_INIT ||
		pending_state == IDLE_INIT_IDLE_AGENT ||
		pending_state == IDLE_INIT_AGENT_SELFOPT ||
		pending_state == INIT_CHK_OFFERS_IDLE_AGENT ||
		pending_state == INIT_CHK_OFFERS_SELFOPT)){
	
		if (coworkers != NULL)
			FOR_MY_COWORKERS_LIST num_of_coworkers++;
		
		if (num_of_coworkers == 1)
			return 1;
		else{
			return (20+num_of_coworkers);
		}
	}
	else if (state == IDLE_AGENT ||
		state == IDLE_AGENT_WAITING_OFF ||
		state == AGENT_INIT_STATE ||
		state == AGENT_SELF_OPT ||
		state == AGENT_SELF_CHK_OFFERS ||
		state == AGENT_ENDING ||
		state == IDAG_ENDING ||
		state == AGENT_ZOMBIE){
	
		return 10;
	}
	return 0;
}
void initialize_PAXOS_data (char scen_directory[SCEN_DIR_SIZE], char scen_num[SCEN_NUM_SIZE]) {
	//printf("Initializing Paxos data...\n");
	strcpy(local_scen_directory, scen_directory);
	strcpy(local_scen_num, scen_num);
}


void sig_PAXOS_INIT_handler(){
  
  
	 int i;
	 int num_of_coworkers = 0;
	 
	 inter_list tmp_inter_list;
	 
	 coworkers_list *tmp_cowork_list;
	
	 handler_Enter(-1, "sig_PAXOS_INIT_handler");
	 
	 
	 paxos_state = PAXOS_ACTIVE;
	
	 #ifdef PLAT_SCC
	 RCCE_wait_until(proposal_number_lock,RCCE_FLAG_UNSET);
	 RCCE_flag_write(&proposal_number_lock,RCCE_FLAG_SET,node_id);
	 proposal_number_personal = *proposal_number_global;
	 *proposal_number_global += 1;
	 RCCE_flag_write(&proposal_number_lock,RCCE_FLAG_UNSET,node_id);
	 #else
	 /* Pick up my personal proposal number */
	 sem_wait(proposal_number_lock);
	 proposal_number_personal = *proposal_number_global;
	 *proposal_number_global += 1;
	 sem_post(proposal_number_lock);
	 #endif
	 
	 fprintf(log_file, "\t\tI am %d, and my proposal number is %d\n",node_id,proposal_number_personal);
	 
	 tmp_inter_list.next = NULL;
	 
	 proposer_vars.core_states = (int *)malloc((X_max*Y_max)*sizeof(int));
	 for (i = 0; i < X_max*Y_max; i++){
		proposer_vars.core_states[i] = -1;
	 }

	/* Case where controller fails */
	if (idag_mask[faulty_core] == faulty_core){
		for (i = 0; i < X_max*Y_max; i++){
			if ((i != my_idag) && (idag_mask[i] == my_idag)){
				tmp_inter_list.data.proposal_number = proposal_number_personal;
				tmp_inter_list.type = PREPARE_REQUEST;
				scc_kill(i,SIG_PREPARE_REQUEST,&tmp_inter_list);
			}
		}
	}
	
	/* Case where manager fails */
	else{
		fprintf(log_file,"\t\tcoworkers List: ");
		FOR_MY_COWORKERS_LIST{
			fprintf(log_file,"%d --> ", tmp_cowork_list->core_id);
		}
		printf("\n");
		FOR_MY_COWORKERS_LIST{
			num_of_coworkers++;
			tmp_inter_list.data.proposal_number = proposal_number_personal;
			tmp_inter_list.type = PREPARE_REQUEST;
			scc_kill(tmp_cowork_list->core_id, SIG_PREPARE_REQUEST, &tmp_inter_list);
		}
		fprintf(log_file,"num_of_coworkers=%d\n",num_of_coworkers);
		printf("num_of_coworkers=%d\n",num_of_coworkers);
	} 
	 
	 handler_Exit(-1, "sig_PAXOS_INIT_handler");
	 return;
}


/************* If a core doesn't receive a signal for a period of time it assumes the
************** controller has died  and sends PREPARE_REQUEST signal
*************/
void sig_PREPARE_REQUEST_handler(int sender_id){
	 
 	int received_proposal_number = sig_read_ar[2];
	inter_list tmp_inter_list;
	int score;
	
	handler_Enter(sender_id,"sig_PREPARE_REQUEST_handler");
	
	fprintf(log_file, "\t\tReceived SIG_PREPARE_REQUEST from %d with Proposal Number %d\n", sender_id, received_proposal_number);

	/* The acceptor has seen a higher proposal number */
 	/* Prepare Request Rejected */ 
 	if (acceptor_vars.highest_proposed_n > received_proposal_number){
 		fprintf(log_file, "\t\tI have seen a higher proposal number request --> REJECTED!\n");
 		return;
 	}
 
 	/* The acceptor has not seen another prepare request before so it promises never to accept a prepare request with proposal number
 	lower than this one. Replies with PREPARE_ACCEPT_NO_PREVIOUS */
	
	/* UPDATE 13.10.2016 -- Also send my state in order to elect core with minimum utilization */
 	else if (acceptor_vars.highest_proposed_n == -1){
		score = leader_preference();
		printf("My score is %d and my state %s\n",score, id2string(state));
 		fprintf(log_file, "\t\tI haven't accepted any values yet!\n");
 		acceptor_vars.highest_proposed_n = received_proposal_number;
 		fprintf(log_file, "\t\tUpdated: acceptor_vars.highest_proposed_n = %d\n", acceptor_vars.highest_proposed_n);
		tmp_inter_list.next = NULL;
		tmp_inter_list.type = PREPARE_ACCEPT_NO_PREVIOUS;
		tmp_inter_list.data.accepted_values[0] = -1;
		tmp_inter_list.data.accepted_values[1] = -1;
		tmp_inter_list.data.accepted_values[2] = score;
		scc_kill(sender_id,SIG_PREPARE_ACCEPT_NO_PREVIOUS,&tmp_inter_list);
 	}

 	/* Proposal number received > Highest proposal number seen */
 	/* Acceptor replies with highest proposal number seen and its value if any and updates the values*/
 	else if (acceptor_vars.highest_proposed_n < received_proposal_number){
 		fprintf(log_file, "\t\treceived_proposal_number higher than highest_proposed_n\n");
 		acceptor_vars.highest_proposed_n = received_proposal_number;
 		fprintf(log_file, "\t\tUpdated: acceptor_vars.highest_proposed_n = %d\n", acceptor_vars.highest_proposed_n);
 		/*If the core has accepted any value it sends that back along with the proposal number of this value*/
 		if (acceptor_vars.highest_acc_value == -1){
 			fprintf(log_file, "\t\tI haven't accepted any values yet!\n");
			tmp_inter_list.next = NULL;
			tmp_inter_list.type = PREPARE_ACCEPT_NO_PREVIOUS;
			tmp_inter_list.data.accepted_values[0] = -1;
			tmp_inter_list.data.accepted_values[1] = -1;
			tmp_inter_list.data.accepted_values[2] = score;
			scc_kill(sender_id,SIG_PREPARE_ACCEPT_NO_PREVIOUS,&tmp_inter_list);
			
 		}else{
			fprintf(log_file, "\t\tI have already accepted the value %d!\n", acceptor_vars.highest_acc_value);
			tmp_inter_list.next = NULL;
			tmp_inter_list.type = PREPARE_ACCEPT;
			tmp_inter_list.data.accepted_values[0] = acceptor_vars.highest_acc_n;
			tmp_inter_list.data.accepted_values[1] = acceptor_vars.highest_acc_value;
			tmp_inter_list.data.accepted_values[2] = score; /* FIXME This seems to be uninitialised */
			scc_kill(sender_id, SIG_PREPARE_ACCEPT, &tmp_inter_list);
 		}
 	}
 	
 	handler_Exit(sender_id,"sig_PREPARE_REQUEST_handler");
	
	return;
}

void sig_PREPARE_ACCEPT_NO_PREVIOUS_handler(int sender_id){

	int k,i;
	int num_of_coworkers;
	int replied_score = sig_read_ar[4];
	int max_score,index;
	
	inter_list tmp_inter_list;
	
	coworkers_list *tmp_cowork_list;
	
	handler_Enter(sender_id,"sig_PREPARE_ACCEPT_NO_PREVIOUS_handler");
	
	proposer_vars.core_states[sender_id] = replied_score;
	proposer_vars.cores_promised++;
	fprintf(log_file, "\t\t Updated state of %d to %s\n", sender_id, id2string(proposer_vars.core_states[sender_id]));
		
	tmp_inter_list.next = NULL;
	/* Case where controller fails */
	if (idag_mask[faulty_core] == faulty_core){
		/*Received reply from majority */
		if (proposer_vars.cores_promised >= majority(CLUSTER_SIZE)){
			if (PREPARE_ACCEPT_SENT == 0){
				for (i=0; i < X_max*Y_max;i++){
					if (proposer_vars.core_states[i] > max_score){
						max_score = proposer_vars.core_states[i];
						index = i;
					}
				}
				PREPARE_ACCEPT_SENT = 1;
				
				fprintf(log_file,"\t\t-------- LAST KNOWN CORE STATES --------\n");
				FOR_NUES{
					fprintf(log_file,"\t\t%d --> %s\n",k,id2string(proposer_vars.core_states[k]));
				}
				fprintf(log_file,"\t\t----------------------------------------\n");
				fprintf(log_file,"\n");
				
				//TODO Decide proposing core based on core states
				
				fprintf(log_file, "\t\tCONTROLLER CASE ; RECEIVED ACCEPT FROM MAJORITY!\n");
				for (k = 0; k < X_max*Y_max; k++){
					/* I send SIG_ACCEPT_REQUEST to cores inside my cluster */
					if ((k != my_idag) && (idag_mask[k] == my_idag)){
						tmp_inter_list.type = ACCEPT_REQUEST;
						tmp_inter_list.data.accepted_values[PROP_NW] = proposal_number_personal;
						 /*The acceptor hasn't received any reply with accepted value so it will propose itself for leader*/
						if (proposer_vars.highest_replied_value == -1){
							/*if (im_manager() != 1){
								fprintf(log_file,"\t\tI am not a manager. I propose myself as the new controller\n");
								proposer_vars.highest_replied_value = node_id;
								tmp_inter_list.data.accepted_values[VALUE_W] = proposer_vars.highest_replied_value;
							}else{
								fprintf(log_file,"\t\tI am a manager. I propose a worker of mine as the new controller.That is %d\n",my_cores->next->core_id);
								tmp_inter_list.data.accepted_values[VALUE_W] = my_cores->next->core_id;
							}*/
							
							/* RANDOM PAXOS */
							//proposer_vars.highest_replied_value = node_id;
							//tmp_inter_list.data.accepted_values[VALUE_W] = node_id;
							/****************/
							/* MODIFIED PAXOS */
							proposer_vars.highest_replied_value = index;
							tmp_inter_list.data.accepted_values[VALUE_W] = index;
							/****************/
						}else{
						tmp_inter_list.data.accepted_values[VALUE_W] = proposer_vars.highest_replied_value;
						}
						scc_kill(k,SIG_ACCEPT_REQUEST,&tmp_inter_list);
					}
				}
			}
		}
	/* Case where manager fails */
	}else{
		num_of_coworkers = 0;
		FOR_MY_COWORKERS_LIST num_of_coworkers++;

		fprintf(log_file,"num_of_coworkers=%d\n",num_of_coworkers);
		if (proposer_vars.cores_promised > majority(num_of_coworkers)){
			if (PREPARE_ACCEPT_SENT == 0){
				PREPARE_ACCEPT_SENT = 1;
				fprintf(log_file, "\t\tMANAGER CASE ; RECEIVED ACCEPT FROM MAJORITY!\n");
				FOR_MY_COWORKERS_LIST{
					tmp_inter_list.type = ACCEPT_REQUEST;
					tmp_inter_list.data.accepted_values[PROP_NW] = proposal_number_personal;
					if (proposer_vars.highest_replied_value == -1)
						proposer_vars.highest_replied_value = node_id;
					tmp_inter_list.data.accepted_values[VALUE_W] = proposer_vars.highest_replied_value;
					scc_kill(tmp_cowork_list->core_id,SIG_ACCEPT_REQUEST,&tmp_inter_list);
				}
			}
		}
	}
 	
 	handler_Exit(sender_id,"sig_PREPARE_ACCEPT_NO_PREVIOUS_handler");
	
 	return;
}

/************* The other cores would either accept its proposal if they haven't seen
************** a higher proposal number
*************/
void sig_PREPARE_ACCEPT_handler(int sender_id){
	
 	int replied_proposal_number = sig_read_ar[PROP_NR];
	int replied_value = sig_read_ar[VALUE_R];
 	int k;
	int num_of_coworkers;
	
	inter_list tmp_inter_list;
	
	coworkers_list *tmp_cowork_list;
	
	handler_Enter(sender_id, "sig_PREPARE_ACCEPT_handler");
	
	proposer_vars.core_states[sender_id] = sig_read_ar[4];
	proposer_vars.cores_promised++;
	fprintf(log_file, "\t\t Updated state of %d to %s\n", sender_id, id2string(proposer_vars.core_states[sender_id]));
	
	tmp_inter_list.next = NULL;
 	/*Save the values if proposal number is higher than the highest replied so far*/
 	if (replied_proposal_number > proposer_vars.highest_replied_n){
 		proposer_vars.highest_replied_n = replied_proposal_number;
 		proposer_vars.highest_replied_value = replied_value;
 		fprintf(log_file, "\t\t Updated proposer_vars.highest_replied_n = %d\n", proposer_vars.highest_replied_n);
 		fprintf(log_file, "\t\t Updated proposer_vars.highest_replied_value = %d\n", proposer_vars.highest_replied_value);
 	}

	/* Case where controller fails */
	if (idag_mask[faulty_core] == faulty_core){
		if (proposer_vars.cores_promised >= majority(CLUSTER_SIZE)){
			if (PREPARE_ACCEPT_SENT == 0){
				PREPARE_ACCEPT_SENT = 1;
				
				fprintf(log_file,"\t\t-------- LAST KNOWN CORE STATES --------\n");
				FOR_NUES{
					fprintf(log_file,"\t\t%d --> %s\n",k,id2string(proposer_vars.core_states[k]));
				}
				fprintf(log_file,"\n");
				fprintf(log_file,"\t\t----------------------------------------\n");
				
				fprintf(log_file, "\t\tRECEIVED ACCEPT FROM MAJORITY!\n");
				for (k = 0; k < X_max*Y_max; k++){
					if ((k != my_idag) && (idag_mask[k] == my_idag)){
						tmp_inter_list.type = ACCEPT_REQUEST;
						tmp_inter_list.data.accepted_values[PROP_NW] = proposal_number_personal;
						tmp_inter_list.data.accepted_values[VALUE_W] = proposer_vars.highest_replied_value;
						scc_kill(k,SIG_ACCEPT_REQUEST,&tmp_inter_list);

					}
				}
			}
		}
	/* Case where manager fails */
	}else{
		num_of_coworkers = 0;
		FOR_MY_COWORKERS_LIST num_of_coworkers++;
		fprintf(log_file,"num_of_coworkers=%d\n",num_of_coworkers);
		if (proposer_vars.cores_promised >= majority(num_of_coworkers)){
			if (PREPARE_ACCEPT_SENT == 0){
				PREPARE_ACCEPT_SENT = 1;
				fprintf(log_file, "\t\tRECEIVED ACCEPT FROM MAJORITY!\n");
				FOR_MY_COWORKERS_LIST{
					tmp_inter_list.type = ACCEPT_REQUEST;
					tmp_inter_list.data.accepted_values[PROP_NW] = proposal_number_personal;
					tmp_inter_list.data.accepted_values[VALUE_W] = proposer_vars.highest_replied_value;
					scc_kill(tmp_cowork_list->core_id,SIG_ACCEPT_REQUEST,&tmp_inter_list);
				}
			}
		}
	}
	
	handler_Exit(sender_id, "sig_PREPARE_ACCEPT_handler");

	return;
}


void sig_ACCEPT_REQUEST_handler(int sender_id){

 	int proposer_proposal_number = sig_read_ar[PROP_NR];
	inter_list tmp_inter_list;
	
	handler_Enter(sender_id,"sig_ACCEPT_REQUEST_handler");
	
	tmp_inter_list.next = NULL;
	
	if (proposer_proposal_number < acceptor_vars.highest_proposed_n){
		fprintf(log_file, "\t\t ACCEPT_REQUEST proposal number lower than highest_proposed_n %d -> REJECTED\n", acceptor_vars.highest_proposed_n);
	}else{
		acceptor_vars.highest_acc_n = proposer_proposal_number;
		fprintf(log_file, "\t\t Updated acceptor_vars.highest_acc_n = %d\n", acceptor_vars.highest_acc_n);
		acceptor_vars.highest_acc_value = sig_read_ar[VALUE_R];
		fprintf(log_file, "\t\t Updated acceptor_vars.highest_acc_value = %d\n", acceptor_vars.highest_acc_value);
		acceptor_vars.highest_proposed_n = proposer_proposal_number;
		fprintf(log_file, "\t\t Updated acceptor_vars.highest_proposed_n = %d\n", acceptor_vars.highest_proposed_n);
		
		tmp_inter_list.type = ACCEPTED;
		tmp_inter_list.data.accepted_values[VALUE_W] = acceptor_vars.highest_acc_value;
		scc_kill(sender_id, SIG_ACCEPTED, &tmp_inter_list);
	}
	
	handler_Exit(sender_id,"sig_ACCEPT_REQUEST_handler");
	
	return;
}

void sig_ACCEPTED_handler(int sender_id){
 	
	int k;
	int received_value = sig_read_ar[VALUE_R];
	int num_of_coworkers;
	
	inter_list tmp_inter_list;
	
	coworkers_list *tmp_cowork_list;
	
	handler_Enter(sender_id,"sig_ACCEPTED_handler");
	
	proposer_vars.cores_accepted++;
	
	tmp_inter_list.next = NULL;
	/****************************************************************/
	/***************** Case where controller failed *****************/
	/****************************************************************/
	
	if (idag_mask[faulty_core] == faulty_core && idag_mask[faulty_core] != -1){
		if ((proposer_vars.cores_accepted >= majority(CLUSTER_SIZE)) && (SIG_LEARN_SENT == 0)){
			fprintf(log_file, "\t\tRECEIVED ACCEPTED FROM MAJORITY!\n");
			SIG_LEARN_SENT = 1;
			tmp_inter_list.type = LEARN;
			tmp_inter_list.data.learn_ack_info[VALUE_W] = received_value;
			tmp_inter_list.data.learn_ack_info[PREV_CW] = faulty_core;
			scc_kill(node_id,SIG_LEARN,&tmp_inter_list);
			for (k = 0; k < X_max*Y_max; k++){
				if ((k != my_idag) && (k != node_id)){
					tmp_inter_list.type = LEARN;
					tmp_inter_list.data.learn_ack_info[VALUE_W] = received_value;
					tmp_inter_list.data.learn_ack_info[PREV_CW] = faulty_core;
					scc_kill(k,SIG_LEARN,&tmp_inter_list);
				}
			}
			
		}

	/****************************************************************/
	/******************* Case where manager failed ******************/
	/****************************************************************/
	
	}else{
		num_of_coworkers=0;
		FOR_MY_COWORKERS_LIST num_of_coworkers++;
		printf("num_of_coworkers=%d\n",num_of_coworkers);
		if ((proposer_vars.cores_accepted >= majority(num_of_coworkers)) && (SIG_LEARN_SENT == 0)){
			fprintf(log_file, "\t\t MANAGER CASE ; RECEIVED ACCEPTED FROM MAJORITY!\n");
			SIG_LEARN_SENT = 1;
			tmp_inter_list.type = LEARN;
			tmp_inter_list.data.learn_ack_info[VALUE_W] = received_value;
			tmp_inter_list.data.learn_ack_info[PREV_CW] = faulty_core;
			scc_kill(node_id,SIG_LEARN,&tmp_inter_list);
			for (k = 0; k < X_max*Y_max; k++){
				if (k != node_id && k != faulty_core){
					tmp_inter_list.type = LEARN;
					tmp_inter_list.data.learn_ack_info[VALUE_W] = received_value;
					tmp_inter_list.data.learn_ack_info[PREV_CW] = faulty_core;
					scc_kill(k,SIG_LEARN,&tmp_inter_list);
				}
			}
		}
	}
	
	handler_Exit(sender_id,"sig_ACCEPTED_handler");
	
	return;
}

void sig_LEARN_handler(int sender_id){
  
	int received_value = sig_read_ar[VALUE_R];
	int failed_core = sig_read_ar[PREV_CR];
	int k;
	int i;
	int cluster_idag;
	int counter;
	int selfopt_r;
	int failed_interaction = 0; /* 0 nothing, 1 init_search, 2 manager_search */
	int one_core;
	
	core_list *tmp_core_list;
	core_list *tmp_core_list_prev;
	
	DDS_list *tmp_dds;
	DDS_list *tmp_prev_dds;
	
	inter_list tmp_inter_list;
	
	coworkers_list *tmp_cowork_list;
	
	inter_list *tmp_inter;
	
	core_states new_state = NO_PENDING_STATE;
	
	handler_Enter(sender_id,"sig_LEARN_handler");
	
	fprintf(log_file,"\t\t Received_value = %d and failed_core=%d\n",received_value,failed_core);
	suspected[received_value] = -1;
	
	//faulty_core = failed_core;

	/****************************************************************/
	/***************** Case where controller failed *****************/
	/****************************************************************/
	
	i = 0;
	
	/* FIXED IDs */
	for (i=0; i < X_max*Y_max;i++)
		if (idag_mask[i] == failed_core && i != failed_core){
			if (i == node_id)
				printf("I am the new controller and my current state is: %s\n\n",id2string(state));
			break;
		}
	if (idag_mask[node_id] == idag_mask[failed_core])
		printf("%d : %s\n",node_id,id2string(state));
	exit(0);
	
	/* I am checking the interactions i had with the new controller */
	if (failed_core != node_id && idag_mask[failed_core] != -1){
		if (core_inter_head[failed_core] == NULL){
			fprintf(log_file,"\t\tI had no interactions with failed core %d\n", failed_core);
		}else{
			fprintf(log_file, "\t\tMy interactions with failed core %d were:\n",failed_core);
			for (tmp_inter = core_inter_head[failed_core]; tmp_inter != NULL; tmp_inter = tmp_inter->next){
				fprintf(log_file,"\t\t\t%d. %s\n",i,inter2string(tmp_inter->type));
				if (tmp_inter->type == IDAG_FIND_IDAGS || 
				    tmp_inter->type == IDAG_FIND_IDAGS_PENDING){
					
					failed_interaction = 1;
					
				}
				
				if (tmp_inter->type == SELFOPT_IDAG_FIND_IDAGS || 
				    tmp_inter->type == SELFOPT_IDAG_FIND_IDAGS_PENDING){
				
					failed_interaction = 2;
					
				}
				i++;
			}
		}
	}
	
	if (core_inter_head[failed_core] != NULL &&
	   (core_inter_head[failed_core]->type == IDAG_REQ_DDS || 
	    core_inter_head[failed_core]->type == IDAG_REQ_DDS_PENDING ||
	    core_inter_head[failed_core]->type == SELFOPT_IDAG_REQ_DDS || 
	    core_inter_head[failed_core]->type == SELFOPT_IDAG_REQ_DDS_PENDING)){
		
		trigger_shit(failed_core);
		
	}
	
	/* Controller Failure and First Time i receive SIG_LEARN */
	if (idag_mask[failed_core] == failed_core && idag_mask[failed_core] != -1){
		
		/* Update idag_mask and idag_id_arr in any subcase */
		fprintf(log_file,"\t\tUpdating idag_mask and idag_id_arr with new controller %d... ",received_value);
		for (k = 0; k < X_max*Y_max; k++){
			if (idag_mask[k] == failed_core)
				idag_mask[k] = received_value;
		}
		
		for (k = 0; k < num_idags; k++){
			if (idag_id_arr[k] == failed_core)
				idag_id_arr[k] = received_value;
		}
		idag_mask[failed_core] = -1;
		fprintf(log_file,"DONE\n\n");
		

		fprintf(log_file,"\t\tMy interactions with the new controller are:\n");
		i = 0;

		tmp_inter = core_inter_head[received_value];
		while (tmp_inter != NULL && node_id != received_value)
		{
			fprintf(log_file, "\t\t\t%d. %s...\n",i,inter2string(tmp_inter->type));
			tmp_inter = tmp_inter->next;
			i++;
		}
		
		
		/***** I am a newly elected controller *****/
		coworkers_list *tmp_cowork_list;
		int num_of_coworkers;
		if (node_id == received_value){
		  	if (tmp_cowork_list != NULL)
				FOR_MY_COWORKERS_LIST num_of_coworkers++;
			printf("I am the new controller : %d -- Current state : %s - %d!\n", received_value,id2string(state),num_of_coworkers);
			fprintf(log_file, "\t\tI am the new controller : %d -- Current state : %s!\n", received_value,id2string(state));
		
			
			my_idag = -1;
			
			/***** Create my cores list *****/
			if (my_cores != NULL){
				printf("my_cores list is not NULL...\n");
				fprintf(log_file,"\t\tmy_cores list is not NULL...\n");
				for (tmp_core_list = my_cores->next; tmp_core_list != NULL; tmp_core_list=tmp_core_list->next){
					printf("\t\tCore_id : %d | Offered_to : %d ... %sREMOVED%s\n",my_cores->core_id,my_cores->offered_to,KRED,KNRM);
					fprintf(log_file,"\t\t\tCore_id : %d | Offered_to : %d ... %sREMOVED%s\n",my_cores->core_id,my_cores->offered_to,KRED,KNRM);
					free(my_cores);
					my_cores = tmp_core_list;
				}
				printf("\t\tCore_id : %d | Offered_to : %d ... %sREMOVED%s\n",my_cores->core_id,my_cores->offered_to,KRED,KNRM);
				fprintf(log_file,"\t\t\tCore_id : %d | Offered_to : %d ... %sREMOVED%s\n",my_cores->core_id,my_cores->offered_to,KRED,KNRM);
				free(my_cores);
				my_cores = NULL;
			}
			
			if (my_cores == NULL){
				printf("\t\tCreating my_cores list... ");
				fprintf(log_file,"\t\tCreating my_cores list... ");
				my_cores = (core_list *) malloc(sizeof(core_list));
				my_cores_count = 0;
				if (my_cores != NULL){
					printf("%sSuccess!%s\n",KGRN,KNRM);
					my_cores_tail = my_cores;
					my_cores_count++;
					my_cores_tail->core_id = node_id;
					my_cores_tail->offered_to = -1;
					my_cores_tail->next = NULL;
					my_cores_tail->workload[0] = -1;
					my_cores_tail->workload[1] = -1;
					printf("\t\t\tAdded Core_id : %d | Offered_to : %d\n",my_cores_tail->core_id,my_cores_tail->offered_to);
					fprintf(log_file,"\t\t\tAdded Core_id : %d | Offered_to : %d\n",my_cores_tail->core_id,my_cores_tail->offered_to);
					for (i = 0; i < X_max*Y_max; i++){
						if (idag_mask[i] == node_id && i != node_id){
							my_cores_tail->next = (core_list *) malloc(sizeof(core_list));
							if (my_cores_tail->next != NULL){
								my_cores_tail = my_cores_tail->next;
								my_cores_tail->next = NULL;
								my_cores_tail->core_id = i;
								my_cores_tail->offered_to = -1;
								printf("\t\t\tAdded Core_id : %d | Offered_to : %d\n",my_cores_tail->core_id,my_cores_tail->offered_to);
								fprintf(log_file,"\t\t\tAdded Core_id : %d | Offered_to : %d\n",my_cores_tail->core_id,my_cores_tail->offered_to);
								my_cores_count++;
							}else printf("--%d-- error allocating memory for my_cores\n",node_id);
						}
					}
				}else{
					printf("--%d-- error allocating memory for my_cores\n",node_id);
					fprintf(log_file, "--%d-- error allocating memory for my_cores\n",node_id);
				}
			}
			
			/***** Create my DDS List *****/
			if (DDS == DDS_tail && DDS != NULL){
				printf("\t\tDDS = DDS_tail with value: \n\t\t\tAgent_id : %d | Cores in cluster : %d\n",DDS->agent_id,DDS->num_of_cores);
				printf("\t\tReconfiguring DDS...\n");
				fprintf(log_file,"\t\tDDS = DDS_tail with value: \n\t\t\tAgent_id : %d | Cores in cluster : %d\n",DDS->agent_id,DDS->num_of_cores);
				fprintf(log_file,"\t\tReconfiguring DDS...\n");
				free(DDS);
				DDS = NULL;
				DDS_count = 0;
			}
			
			if (DDS == NULL){
				printf("\t\tCreating DDS list... ");
				fflush(stdout);
				DDS = (DDS_list *) malloc(sizeof(DDS_list));
				DDS_count = 0;
				if (DDS != NULL){
					printf("%sSuccess!%s\n",KGRN,KNRM);
					fflush(stdout);
					DDS->agent_id = node_id;
					DDS->next = NULL;
					DDS_tail = DDS;
					DDS_count++;
					DDS->num_of_cores = my_cores_count;
					printf("\t\t\tAdded Agent_id : %d | Cores in cluster : %d\n\n",DDS->agent_id, DDS->num_of_cores);
					fprintf(log_file,"\t\t\tAdded Agent_id : %d | Cores in cluster : %d\n\n",DDS->agent_id, DDS->num_of_cores);
				}else{
					printf("--%d-- error allocating memory for my_cores\n",node_id);
					fprintf(log_file,"--%d-- error allocating memory for my_cores\n",node_id);
				}
			}else{
				DDS_list *tmp_dds;
				printf("\t\tDDS list of %d:\n",node_id);
				fprintf(log_file,"\t\tDDS list of %d:\n",node_id);
				
				tmp_dds = DDS;
				while (tmp_dds != NULL){
					printf("\t\t\tAgent_id : %d | Cores in cluster : %d\n",tmp_dds->agent_id, tmp_dds->num_of_cores);
					fprintf(log_file,"\t\t\tAgent_id : %d | Cores in cluster : %d\n",tmp_dds->agent_id, tmp_dds->num_of_cores);
					tmp_dds = tmp_dds->next;
				}
				printf("\n\n");
			}
			
			paxos_state = NEW_IDAG;
			fprintf(log_file,"Changed Paxos State to %s\n", id2string(paxos_state));

		}
		/***** I am in the same cluster as the new controller *****/
		else if ((my_idag != -1) && (my_idag == failed_core)){
			fprintf(log_file, "\t\t I am in the same cluster as %d. My new controller is %d\n", sender_id, received_value);
			my_idag = received_value;
			
			if (failed_interaction == 1) {
				fprintf(log_file,"I have to resend signal SIG_IDAG_FIND_IDAGS to new controller\n");
				if (core_inter_head[my_idag] == NULL){
					core_inter_head[my_idag] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[my_idag] = core_inter_head[my_idag];
				} else {
					core_inter_tail[my_idag]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[my_idag] = core_inter_tail[my_idag]->next;
				}
				selfopt_r = (int) (1.5 * (X_max / num_idags_x));
				core_inter_tail[my_idag]->type = IDAG_FIND_IDAGS_PENDING;
				core_inter_tail[my_idag]->data.reg.C = node_id-1;
				core_inter_tail[my_idag]->data.reg.r = selfopt_r;
				core_inter_tail[my_idag]->next = NULL;

				if (core_inter_head[my_idag]->next == NULL) {
					paxos_node_stats.msg_count++;
					scc_kill(my_idag, SIG_IDAG_FIND_IDAGS, core_inter_head[my_idag]);
				} else {
					fprintf(log_file, "paxos_signal_handlers.c: Did not send idag_find_idags with interaction %s no2 %s\n",inter2string(core_inter_head[my_idag]->type),inter2string(core_inter_head[my_idag]->next->type));
				}
				if (selfopt_time_rem != -1) { 
					selfopt_time_rem = my_gettimer();

					if (selfopt_time_rem > 0) 
						my_settimer(0);
				}

				if (pending_state == WORKING_NODE) {
					fprintf(log_file, "I change to working idle init\n");
					state = WORKING_NODE;
					pending_state = IDLE_INIT_MAN;
				}else{
					state = IDLE_INIT_MAN;
				}
			}
			
			if (failed_interaction == 2){
				if (core_inter_head[my_idag] == NULL){
					core_inter_head[my_idag] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[my_idag] = core_inter_head[my_idag];
				} else {
					core_inter_tail[my_idag]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[my_idag] = core_inter_tail[my_idag]->next;
				}
				selfopt_r = (int) (1.5 * (X_max / num_idags_x));
				core_inter_tail[my_idag]->type = SELFOPT_IDAG_FIND_IDAGS_PENDING;
				core_inter_tail[my_idag]->data.reg.C = node_id;
				core_inter_tail[my_idag]->data.reg.r = selfopt_r;
				core_inter_tail[my_idag]->next = NULL;

				if (core_inter_head[my_idag]->next == NULL){
					paxos_node_stats.msg_count++;
					scc_kill(my_idag, SIG_IDAG_FIND_IDAGS, core_inter_head[my_idag]);
				} else {
					fprintf(log_file,"paxos_signal_handlers.c: Did not send sig_find_idags with inter1 = %s, inter2 = %s\n",inter2string(core_inter_head[my_idag]->type),inter2string(core_inter_head[my_idag]->next->type));
				}
				
				if (selfopt_interval != MAX_SELF_OPT_INTERVAL_MS){
					selfopt_interval = 2 * selfopt_interval;
				}else{
					selfopt_interval = -1;
				}

				new_state = IDLE_AGENT_WAITING_OFF;
			}
		}
		
		/***** I am a controller so i have to update idag_id_arr and reply with LEARN_ACK_CONTR *****/
		/*else if (im_controller() == 1){
			for (k = 0; k < num_idags; k++)
				if (idag_id_arr[k] == failed_core)
					idag_id_arr[k] = received_value;
				else if (idag_id_arr[k] == node_id)
					tmp_inter_list.data.controller_index = k;
				
			fprintf(log_file, "\t\tI am a Controller. Changed controller %d with %d\n",failed_core,sender_id);
			
			tmp_inter_list.next = NULL;
			tmp_inter_list.type = LEARN_ACK_CONTR;
			scc_kill(received_value,SIG_LEARN_ACK_CONTR,&tmp_inter_list);
			
			fprintf(log_file, "\t\tUpdated idag_id_arr\n");
		}*/
		
		/* I am a manager so i have to reply with ADD_AGENT_TO_DDS. In addition if the new controller was a worker of mine i remove him from my_cores list */
		if (im_manager() == 1){
			if (new_state == IDLE_AGENT_WAITING_OFF) state = IDLE_AGENT_WAITING_OFF;
			counter = 0;
			FOR_MY_CORES_LIST{
				cluster_idag = idag_mask[tmp_core_list->core_id];
				if (cluster_idag == received_value){
					fprintf(log_file,"\t\tI am manager %d and my core %d utilizes in cluster with idag %d\n", node_id, tmp_core_list->core_id,cluster_idag);
					/* if the new controller was a worker of mine i do not send him */
					if (tmp_core_list->core_id != received_value)
						tmp_inter_list.data.workers_info[++counter] = tmp_core_list->core_id;
				}
			}
			
			if (counter > 0){
				tmp_inter_list.next = NULL;
				tmp_inter_list.type = ADD_TO_DDS;
				tmp_inter_list.data.workers_info[0] = counter;
				fprintf(log_file,"\t\tNUMBER OF WORKERS: %d\n", counter);
				scc_kill(received_value,SIG_ADD_TO_DDS,&tmp_inter_list);
			}
			
			/***** If the new controller was a worker of mine i remove him from my_cores list and reappoing workload *****/
			tmp_core_list = my_cores->next;
			tmp_core_list_prev = my_cores;
			while (tmp_core_list != NULL){
				if (tmp_core_list->core_id == received_value){
					fprintf(log_file,"\t\tNew controller was a worker of mine! I remove him from my cores_list!\n");
					my_cores_count--;
					tmp_core_list_prev->next = tmp_core_list->next;
					pending_workload[0] = tmp_core_list->workload[0];
					pending_workload[1] = tmp_core_list->workload[1];
					fprintf(log_file,"Pending workload of new controller was: %d %d\n", pending_workload[0], pending_workload[1]);
					free(tmp_core_list);
					
					if (pending_workload[0] != -1 || pending_workload[1] != -1){
						//active_working_cores--;
						tmp_core_list = my_cores->next;
						fprintf(log_file,"\t\tI am reassigning the workload!\n");
						fprintf(log_file,"\t\t-------- CURRENT WORKLOADS --------\n");
						while (tmp_core_list != NULL){
							fprintf(log_file,"\t\t%d\t|\t%d\t|\t%d\n", tmp_core_list->core_id, tmp_core_list->workload[0], tmp_core_list->workload[1]);
							printf("\t\t%d\t|\t%d\t|\t%d\n", tmp_core_list->core_id, tmp_core_list->workload[0], tmp_core_list->workload[1]);
							tmp_core_list = tmp_core_list->next;
						}
						fprintf(log_file,"\t\t-----------------------------------\n");
						tmp_core_list = my_cores->next;
						while (tmp_core_list != NULL){
							if ((tmp_core_list->workload[0] == -1) && (tmp_core_list->workload[1] == -1)){
								one_core = tmp_core_list->core_id;
								fprintf(log_file,"\t\tpaxos_signal_handlers.c : I have pending workload %d | %d\n",pending_workload[0],pending_workload[1]);
								fprintf(log_file,"\t\tpaxos_signal_handlers.c : I am assigning workload to %d\n",one_core);
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
									paxos_node_stats.msg_count++;
									scc_kill(one_core, SIG_APPOINT_WORK, core_inter_head[one_core]);
								} else {
									fprintf(log_file,"I am doing smth else with my working node %d in init inter1 = %d inter2 = %d\n",one_core,core_inter_head[one_core]->type,core_inter_head[one_core]->next->type);
									printf("ASDASDASDASDASDAS\n");
								}
								pending_workload[0] = -1;
								pending_workload[1] = -1;
								break;
							}
							tmp_core_list = tmp_core_list->next;
						}
						break;
					}
				}else{
					tmp_core_list = tmp_core_list->next;
					tmp_core_list_prev = tmp_core_list_prev->next;
				}
			}
		}
	/****************************************************************/
	/******************* Case where manager failed ******************/
	/****************************************************************/
	}
	else{
		/***** I am the newly elected manager *****/
		if (node_id == received_value && idag_mask[failed_core] != -1){
			idag_mask[failed_core] = -1;
			printf("I am the new manager : %d -- Current state : %s!\n", received_value,id2string(state));
			
			if (my_cores != NULL){
				printf("my_cores list is not NULL...\n");
				for (tmp_core_list = my_cores->next; tmp_core_list != NULL; tmp_core_list=tmp_core_list->next){
					printf("\t\tCore_id : %d | Offered_to : %d ... %sREMOVED%s\n",my_cores->core_id,my_cores->offered_to,KRED,KNRM);
					free(my_cores);
					my_cores = tmp_core_list;
				}
				printf("\t\tCore_id : %d | Offered_to : %d ... %sREMOVED%s\n",my_cores->core_id,my_cores->offered_to,KRED,KNRM);
				free(my_cores);
				my_cores = NULL;
			}
			
			if (my_cores == NULL){
				printf("\t\tCreating my_cores list... ");
				fflush(stdout);
				my_cores = (core_list *) malloc(sizeof(core_list));
				my_cores_count = 0;
				if (my_cores != NULL){
					printf("%sSuccess!%s\n",KGRN,KNRM);
					my_cores_tail = my_cores;
					my_cores_tail->core_id = node_id;
					my_cores_tail->offered_to = -1;
					my_cores_tail->workload[0] = -1;
					my_cores_tail->workload[1] = -1;
					my_cores_tail->next = NULL;
					my_cores_count++;
					printf("\t\t\tAdded Core_id : %d | Offered_to : %d\n",my_cores_tail->core_id,my_cores_tail->offered_to);
					
					FOR_MY_COWORKERS_LIST{
						my_cores_tail->next = (core_list *) malloc(sizeof(core_list));
						if (my_cores_tail->next != NULL){
							my_cores_tail = my_cores_tail->next;
							my_cores_tail->next = NULL;
							my_cores_tail->core_id = tmp_cowork_list->core_id;
							my_cores_tail->offered_to = -1;
							my_cores_tail->workload[0] = -1;
							my_cores_tail->workload[1] = -1;
							printf("\t\t\tAdded Core_id : %d | Offered_to : %d\n",my_cores_tail->core_id,my_cores_tail->offered_to);
							my_cores_count++;
						}else printf("--%d-- error allocating memory for my_cores\n",node_id);
					}
					
				}else printf("--%d-- error allocating memory for my_cores\n",node_id);
			}
			
			/**** I have to send SIG_ADD_TO_DDS to the controllers of my workers ****/
			for (i = 0; i < X_max*Y_max; i++){
				if (idag_mask[i] == i){
					counter = 0;
					cluster_idag = idag_mask[i];
					FOR_MY_CORES_LIST{
						if (cluster_idag == idag_mask[tmp_core_list->core_id]){
							fprintf(log_file,"\t\tI am manager %d and my core %d utilizes in cluster with idag %d\n", node_id, tmp_core_list->core_id,cluster_idag);
							tmp_inter_list.data.workers_info[++counter] = tmp_core_list->core_id;
						}
					}
					
					if (counter > 0){
						tmp_inter_list.next = NULL;
						tmp_inter_list.type = ADD_TO_DDS;
						tmp_inter_list.data.workers_info[0] = counter;
						fprintf(log_file,"\t\tNUMBER OF WORKERS: %d\n", counter);
						scc_kill(i,SIG_ADD_TO_DDS,&tmp_inter_list);
					}
				}
			}
				
			state = AGENT_INIT_STATE;
			paxos_state = NEW_AGENT;
			printf("I was working for app: %d\n", worker_app_id);
			my_app.id = worker_app_id;
			my_app.num_of_cores = my_cores_count-1;
			find_app_info();
#ifndef ARTIFICIAL_APPS_SIM
			printf("Found array size = %d\n", my_app.array_size);
			fprintf(log_file,"Found array size = %d\n", my_app.array_size);
#endif
			printf("Found remaining workload = %d\n", my_app.workld);
			fprintf(log_file,"Found remaining workload = %d\n", my_app.workld);
			printf("App number of cores = %d\n", my_app.num_of_cores);
			fprintf(log_file,"App number of cores = %d\n", my_app.num_of_cores);
			FOR_MY_CORES_LIST{
				fprintf(log_file,"\t\t\tWorker_id : %d | Workload : %d %d\n", tmp_core_list->core_id, tmp_core_list->workload[0], tmp_core_list->workload[1]);
				printf("\t\t\tWorker_id : %d | Workload : %d %d\n", tmp_core_list->core_id, tmp_core_list->workload[0], tmp_core_list->workload[1]);
			}
				
		/***** I am controller i have to remove the failed_core from my DDS and cores list *****/
		}else if (my_idag == -1){
			if (idag_mask[failed_core] == node_id){
				printf("--%d-- I received SIG_LEARN from %d\n",node_id,sender_id);
				tmp_core_list = my_cores->next;
				tmp_core_list_prev = my_cores;
				while (tmp_core_list != NULL){
					if (tmp_core_list->core_id == failed_core){
						my_cores_count--;
						tmp_core_list_prev->next = tmp_core_list->next;
						free(tmp_core_list);
						break;
					}
					tmp_core_list = tmp_core_list->next;
					tmp_core_list_prev = tmp_core_list_prev->next;
				}
	
				printf("\t\tUpdated my_cores list:\n");
				fprintf(log_file,"\t\tUpdated my_cores list:\n");
	
				counter = 0;
				FOR_MY_CORES_LIST{
					if (tmp_core_list->offered_to == failed_core){
						 tmp_core_list->offered_to = -1;
						 counter++;
					}
					printf("\t\t\tCore_id : %d | Offered_to : %d\n",tmp_core_list->core_id,tmp_core_list->offered_to);
					fprintf(log_file,"\t\t\tCore_id : %d | Offered_to : %d\n",tmp_core_list->core_id,tmp_core_list->offered_to);
				}
				printf("\t\t\tmy_cores_count = %d\n",my_cores_count);
				
				tmp_dds = DDS->next;
				tmp_prev_dds = DDS;
				while (tmp_dds != NULL){
					if (tmp_dds->agent_id == failed_core){
						fprintf(log_file,"\t\t Removed failed core %d from DDS\n",tmp_dds->agent_id);
						tmp_prev_dds->next = tmp_dds->next;
						if (tmp_dds->next == NULL){
						      DDS_tail = tmp_prev_dds;
						}
						DDS_count--;
						free(tmp_dds);
						DDS->num_of_cores = DDS->num_of_cores + counter;
						break;
					}else{
						tmp_prev_dds = tmp_dds;
						tmp_dds = tmp_dds->next;
					}
				}
				
				printf("\t\tUpdated DDS list:\n");
				fprintf(log_file,"\t\tUpdated DDS list:\n");
				FOR_MY_DDS_LIST{
					printf("\t\t\tAgent_id : %d | Cores in cluster : %d\n",tmp_dds->agent_id,tmp_dds->num_of_cores);
					fprintf(log_file,"\t\t\tAgent_id : %d | Cores in cluster : %d\n",tmp_dds->agent_id,tmp_dds->num_of_cores);
				}
				printf("\t\t\tDDS_count = %d\n",DDS_count);
			}
		
		/***** I was working for the failed manager *****/  
		}else if (cur_agent.my_agent == failed_core){
			  cur_agent.my_agent = -1;
		}
	}
	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_LEARN_handler with sender = %d state = %s\n\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	//exit(0);
	return;
}

void sig_REINIT_APP_handler(int sender_id){
	
	inter_list *tmp_inter_list;
	
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "\n\n[%d:%d:%d]: I entered sig_REINIT_APP_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
	printf("Received SIG_REINIT_APP from %d\n", sender_id);
	//int i, data_array_local[LINE_SIZE];
	for (tmp_inter_list = core_inter_head[0]; tmp_inter_list != NULL; tmp_inter_list = tmp_inter_list->next)
		 if (tmp_inter_list->type == INIT_APP) {
			printf("i not null\n");
			break;
		 }
	
	if (core_inter_head[12] == NULL){
		core_inter_head[12] = (inter_list *)malloc(sizeof(inter_list));
		core_inter_tail[12] = core_inter_head[12];
	}else{
		core_inter_tail[12]->next = (inter_list *)malloc(sizeof(inter_list));
		core_inter_tail[12] = core_inter_tail[12]->next;
	}
	core_inter_tail[12]->next = NULL;
	core_inter_tail[12]->type = INIT_APP;
	core_inter_tail[12]->data.new_app.id = sig_read_ar[2];
	core_inter_tail[12]->data.new_app.num_of_cores = sig_read_ar[3];
	core_inter_tail[12]->data.new_app.workld = sig_read_ar[4];
#ifndef ARTIFICIAL_APPS_SIM
	core_inter_tail[12]->data.new_app.array_size = sig_read_ar[5];
#endif
	if (core_inter_head[12]->next == NULL){
		scc_kill(12, SIG_INIT_APP, core_inter_head[12]);
	}
	
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_REINIT_APP_handler with sender = %d state=%s\n\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));

	return;
}

void find_app_info(){
	//int sz;
	char app_log_file_name[64];
	char buffer[64];
	int temp;
		strcpy(app_log_file_name, "../");
		strcat(app_log_file_name,local_scen_directory);
		strcat(app_log_file_name, "/");
		strcat(app_log_file_name,local_scen_num);
		strcat(app_log_file_name,"/app_logs/");
		strcat(app_log_file_name, itoa(worker_app_id));
		strcat(app_log_file_name, ".txt");

		printf("Trying to open file %s... ", app_log_file_name);
		if ((app_log_file = fopen(app_log_file_name, "r")) == NULL){
			printf("%sError%s\n",KRED,KNRM);
			printf("paxos_signal_handlers.c : Cannot open input file with file path = %s ",app_log_file_name);
			perror("open app_log_file");
		}else{
			printf("%sSuccess%s\n",KGRN,KNRM);
		}
		
		while (fscanf(app_log_file,"%s", buffer) != EOF){
#ifndef ARTIFICIAL_APPS_SIM
			if (strcmp(buffer,"array_size") == 0){
				fscanf(app_log_file,"%s",buffer);
				fscanf(app_log_file,"%d", &temp);
				my_app.array_size = temp;
			}
#else
			/* FIXME must locate var and A */
#endif			
			if (strcmp(buffer,"workload") == 0){
				fscanf(app_log_file,"%s",buffer);
				fscanf(app_log_file,"%d",&temp);
				my_app.workld = temp;
			}
		}
		fclose(app_log_file);
}

void rollback(){
	 
	 offer_list *tmp_offer_list;
	 inter_list tmp_inter_list;
	 core_list *tmp_core_list;
	 int core_idag;
	 
	 cur_time = time(NULL);
	 cur_t = localtime(&cur_time);
	 
	 
	 tmp_inter_list.next = NULL;
	 
	 fprintf(log_file, "Rolling back... %s\n", id2string(state));
	 //If i am the new controller and i have an app to initialize i send SIG_REINIT_APP to 0.
	 if ((paxos_state == NEW_IDAG) && (state == INIT_MANAGER || state == INIT_MANAGER_SEND_OFFERS || state == IDLE_INIT_MAN || state == INIT_MAN_CHK_OFFERS || pending_state == INIT_MANAGER 
		|| pending_state == INIT_MANAGER_SEND_OFFERS || pending_state == INIT_MAN_CHK_OFFERS || pending_state == AGENT_INIT_CHK_OFFERS || pending_state == IDLE_INIT_MAN
		|| pending_state == IDLE_INIT_IDLE_AGENT || pending_state == IDLE_INIT_AGENT_SELFOPT || pending_state == INIT_CHK_OFFERS_IDLE_AGENT || pending_state == INIT_CHK_OFFERS_SELFOPT)){
		
		printf("i am the new controller and i have an app to initialize i send SIG_REINIT_APP to 0\n");
		if (init_man_offers != NULL){
			tmp_offer_list = init_man_offers;
			while (tmp_offer_list != NULL){
				*tmp_offer_list->answer = 0;
				tmp_offer_list = tmp_offer_list->next;
			}
		}
			
		while (init_man_offers != NULL){
			if (core_inter_head[init_man_offers->sender] != NULL) {
				if (core_inter_head[init_man_offers->sender]->type == REP_AGENT_OFFER_PENDING){
					core_inter_head[init_man_offers->sender]->type = REP_AGENT_OFFER_SENT;
					//kill(pid_num[init_man_offers->sender], SIG_REP_OFFERS);
					paxos_node_stats.msg_count++;
					scc_kill(init_man_offers->sender, SIG_REP_OFFERS, core_inter_head[init_man_offers->sender]);
					//my_stats.msg_count++;
					//my_stats.distance += distance(node_id,init_man_offers->sender);	
				} else {
					printf("gamietai b = %d",init_man_offers->sender);
					fprintf(log_file,"gamietai b = %d",init_man_offers->sender);
				}
				tmp_offer_list = init_man_offers;
				init_man_offers = init_man_offers->next;
				free(tmp_offer_list);
			}
		}
		fprintf(log_file, "Replied to all my offers negatively\n");

		tmp_inter_list.type = REINIT_APP;
		tmp_inter_list.data.reappointed_app = init_app;
		scc_kill(0,SIG_REINIT_APP,&tmp_inter_list);
	}
	/* If i were a manager i have to inform that i am no longer manager and also add other managers to my DDS */
	else if (im_manager() == 1){
	  
		printf("New controller was a manager before paxos! Remove him from dds lists and create his dds list...\n");
		//TODO remove from dds and add managers to dds
		tmp_core_list = my_cores;
		while (tmp_core_list != NULL){
			//idag_mask[tmp_core_list->core_id] -> idag id
		  
			core_idag = idag_mask[tmp_core_list->core_id];
			tmp_inter_list.next = NULL;
			tmp_inter_list.type = REMOVE_FROM_DDS;
			scc_kill(core_idag, SIG_REMOVE_FROM_DDS, &tmp_inter_list);

			
			tmp_core_list = tmp_core_list->next;
		}
	}else
		//printf("New controller was an idle core before paxos! Just create his dds list...\n");
		//TODO add managers to dds
		//my_cores = NULL;
	return;
}

void sig_ADD_TO_DDS_handler(int sender_id, int *inc_cnt, int cur_index_top){
  
	DDS_list *tmp_dds = NULL;
	core_list *tmp_core_list;
	int num_of_workers = 0, flag = 0, current = 0;
	
	handler_Enter(sender_id,"sig_ADD_TO_DDS_handler");
	
	num_of_workers = sig_read_ar[2];

	fprintf(log_file,"--%d-- [%d:%d:%d]:I received SIG_ADD_TO_DDS from %d with num_of_workers = %d\n",node_id,cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,num_of_workers);
	
	if (num_of_workers > 5){
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
	
	if (DDS == NULL){
		DDS_count=0;
		DDS = (DDS_list *) malloc(sizeof(DDS_list));
		DDS->agent_id = node_id;
		DDS->next = NULL;
		DDS_tail = DDS;
		DDS_count++;
		flag = 0;
	}else{
		FOR_MY_DDS_LIST{
			if (tmp_dds->agent_id == sender_id){
				fprintf(log_file,"\t\t%d is already in my DDS. %d->num_of_cores++ && DDS->num_of_cores--\n",sender_id,sender_id);
				DDS->num_of_cores--;
				tmp_dds->num_of_cores++;
				break;
				flag = 1;
			}
		}
	}
	
	if (flag == 0){ /* Sender was not in my DDS */
		fprintf(log_file,"\t\t%d is not in my DDS. DDS_count++\n",sender_id);
		DDS_tail->next = (DDS_list *)malloc(sizeof(DDS_list));
		DDS_tail = DDS_tail->next;
		
		DDS_tail->next = NULL;
		DDS_tail->agent_id = sender_id;
		DDS_tail->num_of_cores = num_of_workers;
		DDS->num_of_cores = DDS->num_of_cores - num_of_workers;
		DDS_count++;
	}
	
	fprintf(log_file, "\t\t%d utilizes %d cores in my cluster\n",sender_id, num_of_workers);
	while (num_of_workers > 0){
		current = sig_read_ar[2+num_of_workers];
		FOR_MY_CORES_LIST{
			if (tmp_core_list->core_id == current){
				fprintf(log_file,"\t\tChanged %d->offered_to = %d\n",tmp_core_list->core_id,sender_id);
				tmp_core_list->offered_to = sender_id;
			}
		}
		num_of_workers--;
	}
	
	printf("\t\tUpdated my_cores list:\n");
	fprintf(log_file,"\t\tUpdated my_cores list:\n");
	
	FOR_MY_CORES_LIST{
		printf("\t\t\tCore_id : %d | Offered_to : %d\n",tmp_core_list->core_id,tmp_core_list->offered_to);
		fprintf(log_file,"\t\t\tCore_id : %d | Offered_to : %d\n",tmp_core_list->core_id,tmp_core_list->offered_to);
	}
	
	printf("\t\tUpdated DDS list:\n");
	fprintf(log_file,"\t\tUpdated DDS list:\n");
	FOR_MY_DDS_LIST{
		printf("\t\t\tAgent_id : %d | Cores in cluster : %d\n",tmp_dds->agent_id,tmp_dds->num_of_cores);
		fprintf(log_file,"\t\t\tAgent_id : %d | Cores in cluster : %d\n",tmp_dds->agent_id,tmp_dds->num_of_cores);
	}

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "\n[%d:%d:%d]: I ended sig_ADD_TO_DDS_handler with sender = %d state = %s\n\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));

	return;
}

void sig_REMOVE_FROM_DDS_handler(int sender_id){
  
	core_list *tmp_core_list;
	DDS_list *tmp_dds, *tmp_prev_dds;
	
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "\n\n[%d:%d:%d]: I entered sig_REMOVE_FROM_DDS_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
	tmp_core_list = my_cores;
	while (tmp_core_list != NULL){
		if (tmp_core_list->offered_to == sender_id){
			fprintf(log_file,"\t\tChanged %d->offered_to to -1\n",tmp_core_list->core_id);
			tmp_core_list->offered_to = -1;
		}
		tmp_core_list = tmp_core_list->next;
	}
	
	tmp_dds = DDS->next;
	tmp_prev_dds = DDS;
	while (tmp_dds != NULL){
		if (tmp_dds->agent_id == sender_id){
			fprintf(log_file,"\t\t Removed %d from DDS\n",tmp_dds->agent_id);
			tmp_prev_dds->next = tmp_dds->next;
			if (tmp_dds->next == NULL){
				DDS_tail = tmp_prev_dds;
			}
			DDS_count--;
			free(tmp_dds);
			break;
		}else{
			tmp_prev_dds = tmp_dds;
			tmp_dds = tmp_dds->next;
		}
	}
	
	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_REMOVE_FROM_DDS_handler with sender = %d state = %s\n\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));

	return;
}

void sig_CONTR_TO_handler(int sender_id){
	
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "\n\n[%d:%d:%d]: I entered sig_CONTR_TO_handler with sender=%d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
	faulty_core = my_idag;
	if (paxos_state != PAXOS_ACTIVE && paxos_state != NEW_AGENT && paxos_state != NEW_IDAG)
		sig_PAXOS_INIT_handler();
	
	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_CONTR_TO_handler with sender = %d state = %s\n\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));

	return;
}

void sig_HEARTBEAT_REQ_handler(int sender_id){
	
	inter_list tmp_inter_list;
	
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	//fprintf(log_file, "[%d:%d:%d]: I entered sig_HEARTBEAT_REQ_handler with sender=%d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
	
	tmp_inter_list.next = NULL;
	tmp_inter_list.type = HEARTBEAT_REP;
	scc_kill(sender_id,SIG_HEARTBEAT_REP,&tmp_inter_list);
	
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);

	return;
}

void sig_HEARTBEAT_REP_handler(int sender_id){

	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	//fprintf(log_file, "[%d:%d:%d]: I entered sig_HEARTBEAT_REP_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
	alive[sender_id] = 1;
	suspected[sender_id] = 0;
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	//fprintf(log_file, "[%d:%d:%d]: I ended sig_TERMINATE_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
	return;
}

void sig_PFD_TIMER_handler(int signo, siginfo_t *info, void *context){
	int i, j, failed_core, pending_workload[2], one_core;
	DDS_list *tmp_dds, *tmp_dds_prev;
	inter_list tmp_inter_list;
	core_list *tmp_core_list, *tmp_core_list_prev;
	
	if (first_time == 0){
		printf("%d oh yeah\n",node_id);
		first_time = 1;
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;
		its.it_value.tv_sec = 2;
		its.it_value.tv_nsec = 0;//100000000;
		if (timer_settime(pfd_timer, 0, &its, NULL) == -1){
			printf("-- %d --", node_id);
			fflush(stdout);
			perror("paxos_signal_handlers.c : timer_settime error9");
		}else {
			fprintf(log_file,"Updated timer!\n");
		}
		return;
	}
	
	signals_disable();
	 
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "\n\n[%d:%d:%d]: I entered sig_PFD_TIMER_handler state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,id2string(state));
	//printf("[%d:%d:%d]: -%d- I entered sig_PFD_TIMER_handler state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,node_id, id2string(state));
	for (i = 0; i < X_max*Y_max; i++){
		if (alive[i] == 0){
			suspected[i]++;
		}
		#if defined(PFD) && defined(BASIC_PAXOS)
		/* The Perfect Failure Detectors sends a SIG_HEARTBEAT_REQ each time the timer explodes and waits for a SIG_HEARTBEAT_REP*/
		/* If he doesn't receive a reply until the timer reexplodes then the node is detected as faulty */
		if (alive[i] != 1 && alive[i] != -1 && faulty_core == -1 && i != node_id && idag_mask[i] == idag_mask[node_id])
		#elif defined(tPFD) && defined(BASIC_PAXOS) //tPFD
		/* The tweaked Perfect Failure Detectors only suspects a core if he sends him a signal and doesn't receive a reply in some period of time*/
		if (alive[i] == 0 && suspected[i] == 2 && i != node_id && idag_mask[i] == idag_mask[node_id])
		#else
		if (alive[i] == -5)
		#endif
		{
			cur_time = time(NULL);
			cur_t = localtime(&cur_time);
			printf("-- %d -- I detected %d as faulty at [%d:%d:%d]!!\n", node_id, i,cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);
			fprintf(log_file, "-- %d -- I detected %d as faulty at [%d:%d:%d]!!\n", node_id, i,cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);
			fprintf(log_file, "-- %d -- I detected %d as faulty!!\n", node_id, i);
			failed_core = i;
			faulty_core = i;
			suspected[failed_core] = -1;
			alive[failed_core] = -1;
			
			#if defined(CONTROLLER) && defined(PLAT_LINUX)
			int semvalue = 0;
		  
			sem_getvalue(&flag_data_written[failed_core],&semvalue);
			if (semvalue == 0){
				/*I am locked*/
				printf("--%d-- I unlocked semaphore for node %d\n",node_id,failed_core);
				sem_post(&flag_data_written[failed_core]);
				sem_getvalue(&flag_data_written[failed_core],&semvalue);
			}
			#endif
			/*My controller timed out */
			if (failed_core == my_idag){
				sig_PAXOS_INIT_handler();
			/* Controller in an other cluster timed out */
			}else if (idag_mask[failed_core] == failed_core){
				printf("--%d-- Other controller TIMED OUT\n", node_id);
				for (j = 0; j < X_max*Y_max; j++){
					if (idag_mask[j] == failed_core && j != failed_core){
						if (state == IDLE_AGENT_WAITING_OFF) state = IDLE_AGENT;
						tmp_inter_list.next = NULL;
						tmp_inter_list.type = CONTR_TO;
						scc_kill(j,SIG_CONTR_TO,&tmp_inter_list);
					}
					  
				}
			/* Worker or manager timed out */
			}else{
				/*I am controller.
				 *If failed node is inside my cluster
				 *i have to remove the failed node from my cores and DDS list */
				if ((idag_mask[node_id] == node_id) && (idag_mask[failed_core] == node_id)){
					tmp_core_list = my_cores->next;
					tmp_core_list_prev = my_cores;
					while (tmp_core_list != NULL){
						if (tmp_core_list->core_id == failed_core){
							my_cores_count--;
							tmp_core_list_prev->next = tmp_core_list->next;
							free(tmp_core_list);
							break;
						}else{
							tmp_core_list_prev = tmp_core_list;
							tmp_core_list = tmp_core_list->next;
						}
					}
					
					tmp_dds = DDS->next;
					tmp_dds_prev = DDS;
					while (tmp_dds != NULL){
						if (tmp_dds->agent_id == failed_core){
							tmp_dds_prev->next = tmp_dds->next;
							free(tmp_dds);
							break;
						}else{
							tmp_dds_prev = tmp_dds;
							tmp_dds = tmp_dds->next;
						}
					}
					
				}
				/*I am manager.
				 *I have to check if the failed node is my worker
				*If yes i have to appoint work to a new node.*/
				if (im_manager()){
				
					printf("-- %d -- I am manager of an application.\n",node_id);
					tmp_core_list = my_cores->next;
					tmp_core_list_prev = my_cores;
					while (tmp_core_list != NULL){
						/* I am the manager of the failed worker. I reappoint the work to another core. */
						if (tmp_core_list->core_id == failed_core){
							my_cores_count--;
						  
							/* I am the manager of the failed worker so i remove him from my core list */
							tmp_core_list_prev->next = tmp_core_list->next;
							
							fprintf(log_file,"I am the manager of the failed worker %d! I removed him from my cores list\n",tmp_core_list->core_id);
							printf("-- %d --I am the manager of the failed worker %d! I removed him from my cores list\n",node_id, tmp_core_list->core_id);
							//one_core = tmp_core_list->core_id;
							pending_workload[0] = tmp_core_list->workload[0];
							pending_workload[1] = tmp_core_list->workload[1];
							printf("Pending workload of faulty core: %d %d\n", pending_workload[0], pending_workload[1]);
							/*else{
								reappoint = FALSE;
							}*/
							free(tmp_core_list);
							
							
							tmp_core_list = my_cores->next;
							fprintf(log_file,"I am reassigning the workload!\n");
							fprintf(log_file,"-------- CURRENT WORKLOADS --------\n");
							printf("-------- CURRENT WORKLOADS --------\n");
							while (tmp_core_list != NULL){
								fprintf(log_file,"%d\t|\t%d\t|\t%d\n", tmp_core_list->core_id, tmp_core_list->workload[0], tmp_core_list->workload[1]);
								printf("%d\t|\t%d\t|\t%d\n", tmp_core_list->core_id, tmp_core_list->workload[0], tmp_core_list->workload[1]);
								tmp_core_list = tmp_core_list->next;
							}
							printf("-----------------------------------\n");
							fprintf(log_file,"-----------------------------------\n");
							
							tmp_core_list = my_cores->next;
							while (tmp_core_list != NULL){
								if ((tmp_core_list->workload[0] == -1) && (tmp_core_list->workload[1] == -1)){
									one_core = tmp_core_list->core_id;
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
										paxos_node_stats.msg_count++;
										scc_kill(one_core, SIG_APPOINT_WORK, core_inter_head[one_core]);
									} else {
										fprintf(log_file,"I am doing smth else with my working node %d in init inter1 = %d inter2 = %d\n",one_core,core_inter_head[one_core]->type,core_inter_head[one_core]->next->type);
									}
									break;
								}
								tmp_core_list = tmp_core_list->next;
							}
							break;
						}else {
							tmp_core_list_prev = tmp_core_list;
							tmp_core_list = tmp_core_list->next;
						}
					}
				}else{
				/* My manager timed out */
					if (cur_agent.my_agent == failed_core){
						printf("--%d-- %d is my manager! I initiate a Paxos instance\n",node_id, failed_core);
						paxos_state = PAXOS_ACTIVE;
						sig_PAXOS_INIT_handler();
					}
				}
			}
		}
		#ifdef PFD
		else{
			if (alive[i] != -1 && i != node_id && idag_mask[i] == idag_mask[node_id]){
				alive[i] = 0;
				tmp_inter_list.type = HEARTBEAT_REQ;
				tmp_inter_list.next = NULL;
				scc_kill(i,SIG_HEARTBEAT_REQ,&tmp_inter_list);
			}
		}
		#else
		else{
			if (alive[i] == 0 && i != node_id && idag_mask[i] == idag_mask[node_id]){
				tmp_inter_list.type = HEARTBEAT_REQ;
				tmp_inter_list.next = NULL;
				scc_kill(i,SIG_HEARTBEAT_REQ,&tmp_inter_list);
			}
		}
		#endif
	}
	fprintf(log_file,"\t\tNodes in my cluster: ");
	for (i = 0; i < X_max*Y_max; i++){
		if (idag_mask[i] == idag_mask[node_id] && i != node_id) fprintf(log_file,"%d, ",i);
	}
	fprintf(log_file,"\n");
	
	fprintf(log_file,"\t\tI have received a signal from: ");
	for (i = 0; i < X_max*Y_max; i++){
		if (alive[i] == 1) fprintf(log_file,"%d, ",i);
	}
	fprintf(log_file,"\n");
	
	fprintf(log_file,"\t\tSent HEARTBEAT_REQ to:");
	for (i = 0; i < X_max*Y_max; i++){
		if (alive[i] == 0 && i != node_id && idag_mask[i] == idag_mask[node_id]){
			fprintf(log_file,"%d, ",i);
		}	  
		alive[i] = 0;
	}
	
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = 2;
	its.it_value.tv_nsec = 0;//100000000;
	if (timer_settime(pfd_timer, 0, &its, NULL) == -1){
		printf("-- %d --", node_id);
		fflush(stdout);
		perror("paxos_signal_handlers.c : timer_settime error9");
	}else {
		fprintf(log_file,"Updated timer!\n");
	}

	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_PFD_TIMER_handler state = %s\n\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,id2string(state));
	
	signals_enable();
}

void sig_EPFD_TIMER_handler(int signo, siginfo_t *info, void *context)
{
	core_list *tmp_core_list, *tmp_core_list_prev;
	DDS_list *tmp_dds, *tmp_dds_prev;
	int one_core,failed_core, pending_workload[2]/*,reappoint = TRUE*/;
	
	signals_disable();
	 
	
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "\n\n[%d:%d:%d]: I entered sig_EPFD_TIMER_handler state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,id2string(state));
    
	int i, j;//, disjoint = 1;
	inter_list tmp_inter_list;
	
	for (i = 0; i < X_max*Y_max; i++){
		if (alive[i] == suspected[i]){
			fprintf(log_file,"\t\tNew Delay: %d\n",delay);
			delay *= 2;
			break;
			//disjoint = 0;
		}
	}
	
	fprintf(log_file,"\t\t------ FAILURE DETECTION ------\n");
	for (i = 0; i < X_max*Y_max; i++){
		if (alive[i] == 0 && suspected[i] != -1){
			suspected[i]++;
			fprintf(log_file,"\t\t%d -> SUSPECTED with suspected[%d] = %d\n",i,i,suspected[i]);
			if ((suspected[i] > 2) && (faulty_core != i)){
				cur_time = time(NULL);
				cur_t = localtime(&cur_time);
				failed_core = i;
				faulty_core = i;
				suspected[failed_core] = -1;
				alive[failed_core] = -1;
				
				#if defined(CONTROLLER) && defined(PLAT_LINUX)
				int semvalue = 0;
			  
				sem_getvalue(&flag_data_written[failed_core],&semvalue);
				if (semvalue == 0){
					/*I am locked*/
					printf("--%d-- I unlocked semaphore for node %d\n",node_id,failed_core);
					sem_post(&flag_data_written[failed_core]);
					sem_getvalue(&flag_data_written[failed_core],&semvalue);
				}
				#endif
				/*My controller timed out */
				if (failed_core == my_idag){
					sig_PAXOS_INIT_handler();
				/* Controller in an other cluster timed out */
				}else if (idag_mask[failed_core] == failed_core){
					printf("--%d-- Other controller TIMED OUT\n", node_id);
					for (j = 0; j < X_max*Y_max; j++){
						if (idag_mask[j] == failed_core && j != failed_core){
							if (state == IDLE_AGENT_WAITING_OFF) state = IDLE_AGENT;
							tmp_inter_list.next = NULL;
							tmp_inter_list.type = CONTR_TO;
							scc_kill(j,SIG_CONTR_TO,&tmp_inter_list);
						}
						  
					}
				/* Worker or manager timed out */
				}else{
					/*I am controller.
					*If failed node is inside my cluster
					*i have to remove the failed node from my cores and DDS list */
					if ((idag_mask[node_id] == node_id) && (idag_mask[failed_core] == node_id)){
						tmp_core_list = my_cores->next;
						tmp_core_list_prev = my_cores;
						while (tmp_core_list != NULL){
							if (tmp_core_list->core_id == failed_core){
								my_cores_count--;
								tmp_core_list_prev->next = tmp_core_list->next;
								free(tmp_core_list);
								break;
							}else{
								tmp_core_list_prev = tmp_core_list;
								tmp_core_list = tmp_core_list->next;
							}
						}
						
						tmp_dds = DDS->next;
						tmp_dds_prev = DDS;
						while (tmp_dds != NULL){
							if (tmp_dds->agent_id == failed_core){
								tmp_dds_prev->next = tmp_dds->next;
								free(tmp_dds);
								break;
							}else{
								tmp_dds_prev = tmp_dds;
								tmp_dds = tmp_dds->next;
							}
						}
						
					}
					/*I am manager.
					*I have to check if the failed node is my worker
					*If yes i have to appoint work to a new node.*/
					if (im_manager()){
					
						printf("-- %d -- I am manager of an application.\n",node_id);
						tmp_core_list = my_cores->next;
						tmp_core_list_prev = my_cores;
						while (tmp_core_list != NULL){
							/* I am the manager of the failed worker. I reappoint the work to another core. */
							if (tmp_core_list->core_id == failed_core){
								my_cores_count--;
							  
								/* I am the manager of the failed worker so i remove him from my core list */
								tmp_core_list_prev->next = tmp_core_list->next;
								
								fprintf(log_file,"I am the manager of the failed worker %d! I removed him from my cores list\n",tmp_core_list->core_id);
								printf("-- %d --I am the manager of the failed worker %d! I removed him from my cores list\n",node_id, tmp_core_list->core_id);
								//one_core = tmp_core_list->core_id;
								pending_workload[0] = tmp_core_list->workload[0];
								pending_workload[1] = tmp_core_list->workload[1];
								printf("Pending workload of faulty core: %d %d\n", pending_workload[0], pending_workload[1]);
								/*else{
									reappoint = FALSE;
								}*/
								free(tmp_core_list);
								
								
								tmp_core_list = my_cores->next;
								fprintf(log_file,"I am reassigning the workload!\n");
								fprintf(log_file,"-------- CURRENT WORKLOADS --------\n");
								printf("-------- CURRENT WORKLOADS --------\n");
								while (tmp_core_list != NULL){
									fprintf(log_file,"%d\t|\t%d\t|\t%d\n", tmp_core_list->core_id, tmp_core_list->workload[0], tmp_core_list->workload[1]);
									printf("%d\t|\t%d\t|\t%d\n", tmp_core_list->core_id, tmp_core_list->workload[0], tmp_core_list->workload[1]);
									tmp_core_list = tmp_core_list->next;
								}
								printf("-----------------------------------\n");
								fprintf(log_file,"-----------------------------------\n");
								
								tmp_core_list = my_cores->next;
								while (tmp_core_list != NULL){
									if ((tmp_core_list->workload[0] == -1) && (tmp_core_list->workload[1] == -1)){
										one_core = tmp_core_list->core_id;
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
											paxos_node_stats.msg_count++;
											scc_kill(one_core, SIG_APPOINT_WORK, core_inter_head[one_core]);
										} else {
											fprintf(log_file,"I am doing smth else with my working node %d in init inter1 = %d inter2 = %d\n",one_core,core_inter_head[one_core]->type,core_inter_head[one_core]->next->type);
										}
										break;
									}
									tmp_core_list = tmp_core_list->next;
								}
								break;
							}else {
								tmp_core_list_prev = tmp_core_list;
								tmp_core_list = tmp_core_list->next;
							}
						}
					}else{
					/* My manager timed out */
						if (cur_agent.my_agent == failed_core){
							printf("--%d-- %d is my manager! I initiate a Paxos instance\n",node_id, failed_core);
							paxos_state = PAXOS_ACTIVE;
							sig_PAXOS_INIT_handler();
						}
					}
				}
			}
			#ifdef tEPFD
			else if (suspected[i] == 2){
				tmp_inter_list.next = NULL;
				tmp_inter_list.type = HEARTBEAT_REQ;
				scc_kill(i,SIG_HEARTBEAT_REQ,&tmp_inter_list);
			}
			#else
			else {
				if (i == 10)
					printf("suspected[%d]=%d and alive[%d]=%d\n",i,suspected[i],i,alive[i]);
				tmp_inter_list.next = NULL;
				tmp_inter_list.type = HEARTBEAT_REQ;
				scc_kill(i,SIG_HEARTBEAT_REQ,&tmp_inter_list);
			}
			#endif
		  
		}else if (alive[i] == 1){
			suspected[i] = 0;
			//fprintf(log_file,"\t\t%d -> ALIVE\n", i);
		}
		alive[i] = 0;
	}
	
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = delay;
	its.it_value.tv_nsec = 0;
	if (timer_settime(epfd_timer, 0, &its, NULL) == -1){
		printf("-- %d --", node_id);
		fflush(stdout);
		perror("paxos_signal_handlers.c : timer_settime error9");
	}else {
		fprintf(log_file,"Updated timer!\n");
	}

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended sig_EPFD_TIMER_handler state = %s\n\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,id2string(state));
	
	 
	signals_enable();
	return;
}

void sig_CTIMER_handler(int signo, siginfo_t *info, void *context)
{
  
	struct tm cur_t_1;
	
	signals_disable();

	struct timeval recover_time_val;
	#ifdef CONTROLLER
	DDS_list *tmp_dds;
	printf("--%d-- CTIMER_handler : Controller %sTimed out!%s\n",node_id,KRED,KNRM);
	printf("DDS list before time out:\n");
	
	for (tmp_dds = DDS; tmp_dds != NULL; tmp_dds = tmp_dds->next)
		printf("\t\t\tAgent_id : %d | Cores in cluster : %d\n",tmp_dds->agent_id, tmp_dds->num_of_cores);

	#elif WORKER
	printf("--%d-- CTIMER_handler : Worker %sTimed out!%s\n",node_id,KRED,KNRM);
	printf("Worker state before timeout: %s\n",id2string(state));
	#elif MANAGER
	core_list *tmp_core_list;
	printf("--%d-- CTIMER_handler : Manager %sTimed out!%s\n",node_id,KRED,KNRM);
	printf("Manager state before timeout: %s\n",id2string(state));
	for (tmp_core_list = my_cores; tmp_core_list != NULL; tmp_core_list = tmp_core_list->next)
		printf("\t\t\tWorker_id : %d | Workload : %d %d\n", tmp_core_list->core_id, tmp_core_list->workload[0], tmp_core_list->workload[1]);
	#endif
	
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	printf("\n\nI timed out at [%d:%d:%d]\n\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);
	
	/* FIXED IDs */
	int k;
	inter_list tmp_inter_list;
	for (k = 0; k < X_max*Y_max; k++){
		if (k != node_id && k != faulty_core){
			tmp_inter_list.type = LEARN;
			tmp_inter_list.data.learn_ack_info[VALUE_W] = node_id+1;
			tmp_inter_list.data.learn_ack_info[PREV_CW] = node_id;
			scc_kill(k,SIG_LEARN,&tmp_inter_list);
		}
	}
	exit(0);
	paxos_state = FAILED_CORE;
	
	gettimeofday(&time_val,NULL);
        cur_t = localtime(&time_val.tv_sec);
        fail_time_val = time_val;
        cur_t_1 = *cur_t;
		
	while (fail_flag == 0){
		scc_pause();
		scc_signals_check();
	}
	
	gettimeofday(&time_val, NULL);
	cur_t = localtime(&time_val.tv_sec);
	printf("\n\n\n\n\n\n\n[%d:%d:%d:%ld]: gettimeofday_1\n",cur_t_1.tm_hour,cur_t_1.tm_min,cur_t_1.tm_sec,fail_time_val.tv_usec);
	printf("[%d:%d:%d:%ld]: gettimeofday_2\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,time_val.tv_usec);
	long int dif = ((cur_t->tm_sec * 1000000) + time_val.tv_usec) - ((cur_t_1.tm_sec * 1000000) + fail_time_val.tv_usec);
  printf("Difference in us is: %ld\n\n\n\n\n\n\n",dif);
  fflush(stdout);
	
	exit(0);
	signals_enable();
}

void sig_ITIMER_handler(int signo, siginfo_t *info, void *context)
{
	int i;
	inter_list tmp_inter_list;
	
	signals_disable();
	 
	
  	printf("--%d-- i have to decide what to do here!!\n", node_id);
	fflush(stdout);
	for (i = 0; i < X_max*Y_max; i++){
		if (idag_mask[i] == 10 && i != 10){
			if (state == IDLE_AGENT_WAITING_OFF) state = IDLE_AGENT;
			tmp_inter_list.next = NULL;
			tmp_inter_list.type = CONTR_TO;
			scc_kill(i,SIG_CONTR_TO,&tmp_inter_list);
		}
	}
	signals_enable();
}
/* END */
void sig_FAIL_handler(){
	#ifdef WORKER
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIG_CTIMER;
	sev.sigev_value.sival_ptr = &controller_timer;
	if (timer_create(CLOCK_REALTIME, &sev, &controller_timer) == -1)
		 printf("timer_create error\n");
	else
		 printf("Worker Timer created succesfully!\n");
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = 2;
	its.it_value.tv_nsec = 0;
	if (timer_settime(controller_timer, 0, &its, NULL) == -1)
		 perror("controller_core.c : timer_settime error9");
	else
		 printf("%d : My timer will explode in %d seconds.\n", node_id, 10);
	return;
	#endif
	#ifdef MANAGER
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIG_CTIMER;
	sev.sigev_value.sival_ptr = &controller_timer;
	if (timer_create(CLOCK_REALTIME, &sev, &controller_timer) == -1)
		 printf("timer_create error\n");
	else
		 printf("Manager Timer created succesfully!\n");
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = 10;
	its.it_value.tv_nsec = 0;
	if (timer_settime(controller_timer, 0, &its, NULL) == -1)
		 perror("controller_core.c : timer_settime error9");
	else
		 printf("%d : My timer will explode in %d seconds.\n", node_id, 10);
	return;
	#endif
}

void sig_PAXOS_STATS_REQ_handler(int sender_id){
  
	inter_list tmp_inter_list;
	
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "\n\n[%d:%d:%d]: I entered sig_PAXOS_STATS_REQ_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
	tmp_inter_list.next = NULL;
	tmp_inter_list.type = PAXOS_STATS_REP;
	tmp_inter_list.data.paxos_stats[0] = paxos_node_stats.msg_count;
	tmp_inter_list.data.paxos_stats[1] = paxos_node_stats.fd_msg_count;
	fprintf(log_file,"\t\tI send %d my paxos stats %lld , %d\n",sender_id,paxos_node_stats.msg_count,paxos_node_stats.fd_msg_count);
	scc_kill(sender_id,SIG_PAXOS_STATS_REP,&tmp_inter_list);
	
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "\n\n[%d:%d:%d]: I ended sig_PAXOS_STATS_REQ_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
}

void sig_PAXOS_STATS_REP_handler(int sender_id){
  
	long long int paxos_replied_stats = sig_read_ar[2];
	long long int fd_replied_stats = sig_read_ar[3];
	
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "\n\n[%d:%d:%d]: I entered sig_PAXOS_STATS_REP_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
	fprintf(log_file,"\t\t%d has replied with msg_count = %lld and fd_msg_count = %lld\n",sender_id,paxos_replied_stats,fd_replied_stats);
	paxos_total_stats.msg_count += paxos_replied_stats;
	paxos_total_stats.fd_msg_count += fd_replied_stats;
	paxos_stats_replied++;
	fprintf(log_file,"\t\tI have updated my stats. New message count = %lld\n",paxos_total_stats.msg_count);
	fprintf(log_file,"\t\tCores replied: %d | My cores count: %d\n",paxos_stats_replied,my_cores_count);
	
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	fprintf(log_file, "\n\n[%d:%d:%d]: I ended sig_PAXOS_STATS_REP_handler with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,sender_id,id2string(state));
	
  
}

#include "scc_signals.h"

extern RCCE_FLAG flag_signals_enabled,flag_data_written;
extern int *sig_array, *data_array, NUES;
extern int node_id, num_apps_terminated;
extern FILE *log_file;

int scc_kill(int target_ID, int sig) {
	int sig_array_local[LINE_SIZE], error, str_len, i;
	RCCE_FLAG_STATUS receiver_status;
	char error_str[64];

	//printf("I am %d and i enter here with target_ID %d\n",node_id,target_ID);

	for (i=0; i<LINE_SIZE; i++) 
		sig_array_local[i] = sig;	

	RCCE_flag_read(flag_signals_enabled, &receiver_status, target_ID);
	//if (receiver_status == RCCE_FLAG_UNSET) printf("I am %d kai signals of 0 are disabled\n",ME);	
	while (receiver_status == RCCE_FLAG_UNSET) 
		RCCE_flag_read(flag_signals_enabled, &receiver_status, target_ID);
		
	error = RCCE_put((t_vcharp)(&sig_array[node_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), target_ID);
	if (error != RCCE_SUCCESS) {
		RCCE_error_string(error, error_str, &str_len);
		printf("I am %d and i got an error in put with descr %s\n",node_id,error_str);
	}

	return error;
}

void scc_signals_check(void) {
	int sig_read_ar[LINE_SIZE], sig_array_local[LINE_SIZE], error, str_len, sender_id, i;
	RCCE_FLAG_STATUS receiver_status;
	char error_str[64];

	for (i=0; i<LINE_SIZE; i++) sig_array_local[i] = NO_SIG;	

	//disable signals here
	RCCE_flag_write(&flag_signals_enabled, RCCE_FLAG_UNSET, node_id);

	for (sender_id=0; sender_id<NUES; sender_id++) {
		if (sender_id == node_id) continue;

		error = RCCE_get((t_vcharp)(&sig_read_ar[0]), (t_vcharp)(&sig_array[sender_id*LINE_SIZE]), LINE_SIZE * sizeof(int), node_id);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			printf("I am %d and i got an error in get from %d with descr %s\n",node_id,sender_id,error_str);
		} else {
			if (sig_read_ar[0] == SIG_INIT) {
				sig_INIT_handler(sender_id);
				error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_ACK) {
				sig_ACK_handler(sender_id);
				error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);	
			} else if (sig_read_ar[0] == SIG_TERMINATE) {
				sig_TERMINATE_handler(sender_id);
				error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_INIT_APP) {
				sig_INIT_APP_handler(sender_id);
				error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_IDAG_FIND_IDAGS) {
				sig_IDAG_FIND_IDAGS_handler(sender_id);
				error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_REQ_DDS) {
				sig_REQ_DDS_handler(sender_id);
				error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_REQ_CORES) {
				sig_REQ_CORES_handler(sender_id);
				error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_REP_OFFERS) {
				sig_REP_OFFERS_handler(sender_id);
				error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_INIT_AGENT) {
				sig_INIT_AGENT_handler(sender_id);
				error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_REJECT) {
				sig_REJECT_handler(sender_id);
				error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_APP_TERMINATED) {
				num_apps_terminated++;
				fprintf(log_file,"app_terminated = %d sender_id = %d\n",num_apps_terminated,sender_id);
				fflush(log_file);
				error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] != NO_SIG) printf("I am %d and i read smth different than no_sig which is %d from %d\n",node_id,sig_read_ar[0],sender_id);
		}
	
	}

	RCCE_flag_write(&flag_signals_enabled, RCCE_FLAG_SET, node_id);
		
}		

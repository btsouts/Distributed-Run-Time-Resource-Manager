#include "scc_signals.h"

extern RCCE_FLAG flag_signals_enabled,flag_data_written;
extern int *sig_array, *data_array, NUES;
extern int node_id, num_apps_terminated;
extern FILE *log_file;

int scc_kill(int target_ID, int sig) {
	int sig_array_local[LINE_SIZE], error, str_len, i, sig_read_ar[LINE_SIZE];
	RCCE_FLAG_STATUS receiver_status;
	char error_str[64];

	fprintf(log_file,"I enter here with target_ID %d and sig %d\n",target_ID,sig);
	fflush(log_file);

	for (i=0; i<LINE_SIZE; i++) 
		sig_array_local[i] = sig;	

	RCCE_flag_read(flag_signals_enabled, &receiver_status, target_ID);
	//if (receiver_status == RCCE_FLAG_UNSET) printf("I am %d kai signals of 0 are disabled\n",ME);	
	while (receiver_status == RCCE_FLAG_UNSET) 
		RCCE_flag_read(flag_signals_enabled, &receiver_status, target_ID);
	
	if (sig == SIG_REJECT || sig == SIG_FINISH || sig == SIG_APP_TERMINATED || (sig == SIG_INIT_APP && target_ID == 0)) {
		error = RCCE_get((t_vcharp)(&sig_read_ar[0]), (t_vcharp)(&sig_array[node_id*LINE_SIZE]), LINE_SIZE * sizeof(int), target_ID);	
		if (sig_read_ar[0] != NO_SIG && sig_read_ar[0] != SIG_ACK) {
			fprintf(log_file,"I have allready sent sig %d\n",sig_read_ar[0]);
			fflush(log_file);

			sig_array_local[0] = sig_read_ar[0];
		}
	}

	error = RCCE_put((t_vcharp)(&sig_array[node_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), target_ID);
	if (error != RCCE_SUCCESS) {
		RCCE_error_string(error, error_str, &str_len);
		fprintf(log_file,"I got an error in put with descr %s\n",error_str);
		fflush(log_file);	
	}

	fprintf(log_file,"I leave\n");
	fflush(log_file);

	return error;
}

void scc_signals_check(void) {
	int sig_read_ar[LINE_SIZE], sig_array_local[LINE_SIZE], error, str_len, sender_id, i, sig;
	RCCE_FLAG_STATUS receiver_status;
	char error_str[64];

	for (i=0; i<LINE_SIZE; i++) sig_array_local[i] = NO_SIG;	

	//disable signals here
	//RCCE_flag_write(&flag_signals_enabled, RCCE_FLAG_UNSET, node_id);
	//rintf(log_file,"I enter\n");

	for (sender_id=0; sender_id<36; sender_id++) {
		if (sender_id == node_id) continue;

		RCCE_flag_write(&flag_signals_enabled, RCCE_FLAG_UNSET, node_id);
		error = RCCE_get((t_vcharp)(&sig_read_ar[0]), (t_vcharp)(&sig_array[sender_id*LINE_SIZE]), LINE_SIZE * sizeof(int), node_id);
		RCCE_flag_write(&flag_signals_enabled, RCCE_FLAG_SET, node_id);

		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			fprintf(log_file,"I got an error in get from %d with descr %s\n",sender_id,error_str);
			fflush(log_file);	
		} else {
			if (sig_read_ar[0] == SIG_INIT) {
				sig_INIT_handler(sender_id);
				//error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_ACK) {
				sig_ACK_handler(sender_id);
				//error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);	
			} else if (sig_read_ar[0] == SIG_TERMINATE) {
				sig_TERMINATE_handler(sender_id);
				//error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_INIT_APP) {
				sig_INIT_APP_handler(sender_id);
				//error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_IDAG_FIND_IDAGS) {
				sig_IDAG_FIND_IDAGS_handler(sender_id);
				//error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_REQ_DDS) {
				sig_REQ_DDS_handler(sender_id);
				//error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_REQ_CORES) {
				sig_REQ_CORES_handler(sender_id);
				//error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_REP_OFFERS) {
				sig_REP_OFFERS_handler(sender_id);
				//error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_INIT_AGENT) {
				sig_INIT_AGENT_handler(sender_id);
				//error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_ADD_CORES_DDS) {
				sig_ADD_CORES_DDS_handler(sender_id);
				//error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_REM_CORES_DDS) {
				sig_REM_CORES_DDS_handler(sender_id);
				//error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_APPOINT_WORK) {
				sig_APPOINT_WORK_handler(sender_id);
				//error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_CHECK_REM_TIME) {
				sig_CHECK_REM_TIME_handler(sender_id);
				//error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_FINISH) {
				sig_FINISH_handler(sender_id);
				//error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_REJECT) {
				sig_REJECT_handler(sender_id);
				//error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] == SIG_APP_TERMINATED) {
				num_apps_terminated++;
				fprintf(log_file,"app_terminated = %d sender_id = %d\n",num_apps_terminated,sender_id);
				fflush(log_file);
				//error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			} else if (sig_read_ar[0] != NO_SIG) {
				fprintf(log_file,"I read smth different than no_sig which is %d from %d\n",sig_read_ar[0],sender_id);
				fflush(log_file);
			}

			sig = sig_read_ar[0];
			RCCE_flag_write(&flag_signals_enabled, RCCE_FLAG_UNSET, node_id);
                	error = RCCE_get((t_vcharp)(&sig_read_ar[0]), (t_vcharp)(&sig_array[sender_id*LINE_SIZE]), LINE_SIZE * sizeof(int), node_id);
                	RCCE_flag_write(&flag_signals_enabled, RCCE_FLAG_SET, node_id);

			if (sig_read_ar[1] != sig_read_ar[0]) {
				fprintf(log_file,"I have a second signal %d first sig was %d not it is %d\n",sig_read_ar[1],sig,sig_read_ar[0]);
				fflush(log_file);

				/*if (sig_read_ar[1] == SIG_INIT) {
					sig_INIT_handler(sender_id);
				} else if (sig_read_ar[1] == SIG_ACK) {
					sig_ACK_handler(sender_id);
				} else if (sig_read_ar[1] == SIG_TERMINATE) {
					sig_TERMINATE_handler(sender_id);
				} else if (sig_read_ar[1] == SIG_IDAG_FIND_IDAGS) {
					sig_IDAG_FIND_IDAGS_handler(sender_id);
				} else if (sig_read_ar[1] == SIG_REQ_DDS) {
					sig_REQ_DDS_handler(sender_id);
				} else if (sig_read_ar[1] == SIG_REQ_CORES) {
					sig_REQ_CORES_handler(sender_id);
				} else if (sig_read_ar[1] == SIG_REP_OFFERS) {
					sig_REP_OFFERS_handler(sender_id);
				} else if (sig_read_ar[1] == SIG_INIT_AGENT) {
					sig_INIT_AGENT_handler(sender_id);
				} else if (sig_read_ar[1] == SIG_ADD_CORES_DDS) {
					sig_ADD_CORES_DDS_handler(sender_id);
				} else if (sig_read_ar[1] == SIG_REM_CORES_DDS) {
					sig_REM_CORES_DDS_handler(sender_id);
				} else if (sig_read_ar[1] == SIG_APPOINT_WORK) {
					sig_APPOINT_WORK_handler(sender_id);
				} else if (sig_read_ar[1] == SIG_CHECK_REM_TIME) {
					sig_CHECK_REM_TIME_handler(sender_id);
				} else */
				if (sig_read_ar[1] == SIG_INIT_APP) {
					sig_INIT_APP_handler(sender_id);
				} else if (sig_read_ar[1] == SIG_FINISH) {
					sig_FINISH_handler(sender_id);
				} else if (sig_read_ar[1] == SIG_REJECT) {
					sig_REJECT_handler(sender_id);
				} else if (sig_read_ar[1] == SIG_APP_TERMINATED) {
					num_apps_terminated++;
					fprintf(log_file,"app_terminated = %d sender_id = %d\n",num_apps_terminated,sender_id);
					fflush(log_file);
				} else if (sig_read_ar[1] != NO_SIG) {
					fprintf(log_file,"I read smth different than no_sig which is %d from %d\n",sig_read_ar[1],sender_id);
					fflush(log_file);
				}
			}

			if (sig_read_ar[0] != NO_SIG) {
				//sig = sig_read_ar[0];
				RCCE_flag_write(&flag_signals_enabled, RCCE_FLAG_UNSET, node_id);
				error = RCCE_get((t_vcharp)(&sig_read_ar[0]), (t_vcharp)(&sig_array[sender_id*LINE_SIZE]), LINE_SIZE * sizeof(int), node_id);
				if (sig == sig_read_ar[0]) {					
					error = RCCE_put((t_vcharp)(&sig_array[sender_id*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
					fprintf(log_file,"I invalidated sender_ids %d signals\n",sender_id);
					fflush(log_file);
				}
				RCCE_flag_write(&flag_signals_enabled, RCCE_FLAG_SET, node_id);
			}

		}
	}

	//RCCE_flag_write(&flag_signals_enabled, RCCE_FLAG_SET, node_id);
	//fprintf(log_file,"I leave\n");
	//fflush(log_file);
		
}		

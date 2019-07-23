#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "include/libfunctions.h"
#include "include/my_rtrm.h"

void handler_Enter(int sender_id, char *handler){
  
	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	
	if (sender_id == -1){
		fprintf(log_file, "\n\n[%d:%d:%d]: I entered %s with state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,handler,id2string(state));
	}else{
		fprintf(log_file, "\n\n[%d:%d:%d]: I entered %s with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,handler,sender_id,id2string(state));
	}
	return;
}

void handler_Exit(int sender_id, char *handler){

	cur_time = time(NULL);
	cur_t = localtime(&cur_time);
	
	if (sender_id == -1){
		fprintf(log_file, "\n\n[%d:%d:%d]: I leave %s with state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,handler,id2string(state));
	}else{
		fprintf(log_file, "\n\n[%d:%d:%d]: I leave %s with sender = %d state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,handler,sender_id,id2string(state));
	}
	return;
}
/* pl4tinum 11.10.2016 */
char * app_state_2_string(application_states state_id){
	if (state_id == 0)
		return "NO_APP";
	else if (state_id == 1)
		return "APP_TERMINATED";
	else if (state_id == 2)
		return "RUNNING";
	else
		return "RESIZING";
}

/* pl4tinum 11.10.2016 */
char * id2string(core_states state_id){
	if (state_id == 0)
		return "IDLE_CORE";
	else if (state_id == 1)
		return "WORKING_NODE";
	else if (state_id == 2)
		return "TERMINATED";
	else if (state_id == 3)
	/* Controller States */
		return "IDLE_IDAG";
	else if (state_id == 4)
		return "IDLE_IDAG_INIT_SEND";
	else if (state_id == 5)
		return "IDLE_CHK_APP_FILE";
	else if (state_id == 6)
		return "CHK_APP_FILE";
	else if (state_id == 7)
		return "USER_INPUT";
	/* Initial core States */
	else if (state_id == 8)
		return "INIT_MANAGER";
	else if (state_id == 9)
		return "INIT_MANAGER_SEND_OFFERS";
	else if (state_id == 10)
		return "IDLE_INIT_MAN";
	else if (state_id == 11)
		return "INIT_MAN_CHK_OFFERS";
	/* Manager States */
	else if (state_id == 12)
		return "IDLE_AGENT";
	else if (state_id == 13)
		return "IDLE_AGENT_WAITING_OFF";
	else if (state_id == 14)
		return "AGENT_INIT_STATE";
	else if (state_id == 15)
		return "AGENT_SELF_OPT";
	else if (state_id == 16)
		return "AGENT_SELF_CHK_OFFERS";
	else if (state_id == 17)
		return "AGENT_ENDING";
	else if (state_id == 18)
		return "IDAG_ENDING";
	else if (state_id == 19)
		return "NO_PENDING_STATE";
	else if (state_id == 20)
		return "AGENT_ZOMBIE";
	/* Multiple Pending States */
	else if (state_id == 21)
		return "AGENT_INIT_APP_INIT";
	else if (state_id == 22)
		return "AGENT_INIT_CHK_OFFERS";
	else if (state_id == 23)
		return "AGENT_INIT_IDLE_INIT";
	else if (state_id == 24)
		return "IDLE_INIT_IDLE_AGENT";
	else if (state_id == 25)
		return "IDLE_INIT_AGENT_SELFOPT";
	else if (state_id == 26)
		return "INIT_CHK_OFFERS_IDLE_AGENT";
	else if (state_id == 27)
		return "INIT_CHK_OFFERS_SELFOPT";
	else if (state_id == 28)
		return "PAXOS_ACTIVE";
	else if (state_id == 29)
		return "NEW_IDAG";
	else if (state_id == 30)
		return "NEW_AGENT";
	else
		return "error";
}

/* pl4tinum 11.10.2016 */
char * inter2string(inter_types interaction){
	if (interaction == 0)
		return "INIT_CORE";
	else if (interaction == 1)
		return "REMOVE_APP";
	else if (interaction == 2)
		return "INIT_APP";
	else if (interaction == 3)
		return "DECLARE_INIT_AVAILABILITY";
	else if (interaction == 4)
		return "DEBUG_IDAG_REQ_DDS";
	else if (interaction == 5)
		return "IDAG_FIND_IDAGS_PENDING";
	else if (interaction == 6)
		return "IDAG_FIND_IDAGS";
	else if (interaction == 7)
		return "IDAG_REQ_DDS_PENDING";
	else if (interaction == 8)
		return "IDAG_REQ_DDS";
	else if (interaction == 9)
		return "REP_IDAG_FIND_IDAGS";
	else if (interaction == 10)
		return "REP_IDAG_REQ_DDS";
	else if (interaction == 11)
		return "AGENT_REQ_CORES";
	else if (interaction == 12)
		return "AGENT_REQ_CORES_PENDING";
	else if (interaction == 13)
		return "REP_AGENT_REQ_CORES";
	else if (interaction == 14)
		return "AGENT_OFFER_SENT";
	else if (interaction == 15)
		return "REP_AGENT_OFFER_SENT";
	else if (interaction == 16)
		return "REP_AGENT_OFFER_PENDING";
	else if (interaction == 17)
		return "SELFOPT_IDAG_FIND_IDAGS_PENDING";
	else if (interaction == 18)
		return "SELFOPT_IDAG_FIND_IDAGS";
	else if (interaction == 19)
		return "SELFOPT_IDAG_REQ_DDS_PENDING";
	else if (interaction == 20)
		return "SELFOPT_IDAG_REQ_DDS";
	else if (interaction == 21)
		return "SELFOPT_REQ_CORES_PENDING";
	else if (interaction == 22)
		return "SELFOPT_REQ_CORES";
	else if (interaction == 23)
		return "IDAG_ADD_CORES_DDS";
	else if (interaction == 24)
		return "IDAG_REM_CORES_DDS";
	else if (interaction == 25)
		return "INIT_AGENT";
	else if (interaction == 26)
		return "INIT_WORK_NODE_PENDING";
	else if (interaction == 27)
		return "APPOINT_WORK_NODE_PENDING";
	else if (interaction == 28)
		return "INIT_WORK_NODE";
	else if (interaction == 29)
		return "APPOINT_WORK_NODE";
	else if (interaction == 30)
		return "TERMINATION_STATS";
	else if (interaction == 31)
		return "REP_STATISTICS";
	/* PAXOS Interactions */
	else if (interaction == 32)
		return "PAXOS_INIT";
	else if (interaction == 33)
		return "PREPARE_REQUEST";
	else if (interaction == 34)
		return "PREPARE_ACCEPT_NO_PREVIOUS";
	else if (interaction == 35)
		return "PREPARE_ACCEPT";
	else if (interaction == 36)
		return "ACCEPT_REQUEST";
	else if (interaction == 37)
		return "ACCEPTED";
	else if (interaction == 38)
		return "LEARN";
	else if (interaction == 39)
		return "LEARN_ACK";
	else if (interaction == 40)
		return "LEARN_ACK_CONTR";
	else if (interaction == 41)
		return "REINIT_APP";
	else if (interaction == 42)
		return "CONTR_TO";
	else if (interaction == 43)
		return "REMOVE_FROM_DDS";
	else if (interaction == 44)
		return "ADD_TO_DDS";
	else if (interaction == 45)
		return "HEARTBEAT_REQ";
	else if (interaction == 46)
		return "HEARTBEAT_REP";
	else if (interaction == 47)
		return "PAXOS_STATS_REQ";
	else if (interaction == 48)
		return "PAXOS_STATS_REP";
		
	return "error";
}

char * sig2string(int sig_id){
	 if (sig_id == 0)
		return "NO_SIG";
	 else if (sig_id == SIG_BASE_NUM)
		return "SIG_ACK";
	 else if (sig_id == SIG_BASE_NUM + 1)
		return "SIG_INIT";
	 else if (sig_id == SIG_BASE_NUM + 2)
		return "SIG_TERMINATE";
	 else if (sig_id == SIG_BASE_NUM + 3)
		return "SIG_INIT_APP";
	 else if (sig_id == SIG_BASE_NUM + 4)
		return "SIG_IDAG_FIND_IDAGS";
	 else if (sig_id == SIG_BASE_NUM + 5)
		return "SIG_REQ_DDS";
	 else if (sig_id == SIG_BASE_NUM + 6)
		return "SIG_REQ_CORES";
	 else if (sig_id == SIG_BASE_NUM + 7)
		return "SIG_REP_OFFERS";
	 else if (sig_id == SIG_BASE_NUM + 8)
		return "SIG_INIT_AGENT";
	 else if (sig_id == SIG_BASE_NUM + 9)
		return "SIG_ADD_CORES_DDS";
	 else if (sig_id == SIG_BASE_NUM + 10)
		return "SIG_REM_CORES_DDS";
	 else if (sig_id == SIG_BASE_NUM + 11)
		return "SIG_APPOINT_WORK";
	 else if (sig_id == SIG_BASE_NUM + 12)
		return "SIG_FINISH";
	 else if (sig_id == SIG_BASE_NUM + 13)
		return "SIG_REJECT";
	 else if (sig_id == SIG_BASE_NUM + 14)
		return "SIG_APP_TERMINATED";
	 /* PAXOS SIGNALS */
	 else if (sig_id == SIG_BASE_NUM + 15)
		return "SIG_PREPARE_REQUEST";
	 else if (sig_id == SIG_BASE_NUM + 16)
		return "SIG_PREPARE_ACCEPT_NO_PREVIOUS";
	 else if (sig_id == SIG_BASE_NUM + 17)
		return "SIG_PREPARE_ACCEPT";
	 else if (sig_id == SIG_BASE_NUM + 18)
		return "SIG_ACCEPT_REQUEST";
	 else if (sig_id == SIG_BASE_NUM + 19)
		return "SIG_ACCEPTED";
	 else if (sig_id == SIG_BASE_NUM + 20)
		return "SIG_LEARN";
	 else if (sig_id == SIG_BASE_NUM + 21)
		return "SIG_LEARN_ACK";
	 else if (sig_id == SIG_BASE_NUM + 22)
		return "SIG_LEARN_ACK_CONTR";
	 else if (sig_id == SIG_BASE_NUM + 23)
		return "SIG_REINIT_APP";
	 else if (sig_id == SIG_BASE_NUM + 24)
		return  "SIG_CONTR_TO";
	 else if (sig_id == SIG_BASE_NUM + 25)
		return "SIG_REMOVE_FROM_DDS";
	 else if (sig_id == SIG_BASE_NUM + 26)
		return "SIG_ADD_TO_DDS";
	 else if (sig_id == SIG_BASE_NUM + 27)
		return "SIG_HEARTBEAT_REQ";
	 else if (sig_id == SIG_BASE_NUM + 28)
		return "SIG_HEARTBEAT_REP";
	 else if (sig_id == SIG_BASE_NUM + 29)
		return "SIG_FAIL";
	 else if (sig_id == SIG_BASE_NUM + 30)
		return "SIG_PAXOS_STATS_REQ";
	 else if (sig_id == SIG_BASE_NUM + 31)
		return "SIG_PAXOS_STATS_REP";
	else
		return "Unknown Sig";
}

char * itoa(int value){
	char *tmp;
	int size=1, i, threshold=10;

	while (value >= threshold) {
		size++;
		threshold *= 10;
	}
	
	tmp = (char *) malloc((size+1)*sizeof(char));
	
	for (i=size-1; i>=0; i--){
		tmp[i] = (char) 48 + (value % 10);
		value = value / 10;
	}

	tmp[size] = '\0';
	return tmp;
}

FILE * create_log_file(int node_id, int log, char scen_directory[SCEN_DIR_SIZE], char scen_num[SCEN_NUM_SIZE]){
	char log_file_name[MAX_STR_NAME_SIZE], *name;
	FILE *log_file;

#ifdef PLAT_SCC
	strcpy(log_file_name, "/shared/herc/");
#else 	
	strcpy(log_file_name, "../");
#endif
	strcat(log_file_name, scen_directory);
	strcat(log_file_name, "/");
	strcat(log_file_name, scen_num);
	if (log == 0)
		strcat(log_file_name,"/log_files/log_file_");
	else{
		strcat(log_file_name,"/paxos_log_files/log_file_");
	}
	name = itoa(node_id);
	strcat(log_file_name,name);
	free(name);

	//if ((fd = open(log_file_name, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0) {
	if ((log_file = fopen(log_file_name, "w")) == NULL){
		printf("Open log error my id is %d path: %s \n ",node_id, log_file_name);
		fflush(stdout);
		perror("open log");
	}
	//free(log_file_name);
	return log_file;
}

int majority(int cores){
	return cores/2 + 1;
}

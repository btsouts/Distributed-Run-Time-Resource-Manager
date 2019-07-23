#include "include/idag_defs.h"
#include "include/variables.h"

void global_idag_defs(void) {
	#ifdef NO_ISLANDS
	return;
	#endif

	#ifdef LOW_VOLTAGE_0
	low_voltage_core[0] = 1;
	low_voltage_core[1] = 1;
	low_voltage_core[2] = 1;
	low_voltage_core[3] = 1;
	low_voltage_core[12] = 1;
	low_voltage_core[13] = 1;
	low_voltage_core[14] = 1;
	low_voltage_core[15] = 1;
	#endif
	
	#ifdef LOW_VOLTAGE_1
	low_voltage_core[4] = 1;
	low_voltage_core[5] = 1;
	low_voltage_core[6] = 1;
	low_voltage_core[7] = 1;
	low_voltage_core[16] = 1;
	low_voltage_core[17] = 1;
	low_voltage_core[18] = 1;
	low_voltage_core[19] = 1;
	#endif

	#ifdef LOW_VOLTAGE_2
	low_voltage_core[8] = 1;
	low_voltage_core[9] = 1;
	low_voltage_core[10] = 1;
	low_voltage_core[11] = 1;
	low_voltage_core[20] = 1;
	low_voltage_core[21] = 1;
	low_voltage_core[22] = 1;
	low_voltage_core[23] = 1;
	#endif

	#ifdef LOW_VOLTAGE_3
	low_voltage_core[24] = 1;
	low_voltage_core[25] = 1;
	low_voltage_core[26] = 1;
	low_voltage_core[27] = 1;
	low_voltage_core[36] = 1;
	low_voltage_core[37] = 1;
	low_voltage_core[38] = 1;
	low_voltage_core[39] = 1;
	#endif

	#ifdef LOW_VOLTAGE_4
	low_voltage_core[28] = 1;
	low_voltage_core[29] = 1;
	low_voltage_core[30] = 1;
	low_voltage_core[31] = 1;
	low_voltage_core[40] = 1;
	low_voltage_core[41] = 1;
	low_voltage_core[42] = 1;
	low_voltage_core[43] = 1;
	#endif

	#ifdef LOW_VOLTAGE_5
	low_voltage_core[32] = 1;
	low_voltage_core[33] = 1;
	low_voltage_core[34] = 1;
	low_voltage_core[35] = 1;
	low_voltage_core[44] = 1;
	low_voltage_core[45] = 1;
	low_voltage_core[46] = 1;
	low_voltage_core[47] = 1;
	#endif
}

void read_idag_defs(char scen_directory[SCEN_DIR_SIZE], char scen_num[SCEN_NUM_SIZE], char idag_defs_file_name[SCEN_NUM_SIZE], char paxos_scen[PAXOS_SCEN_SIZE]) {
	 int i, j, cnt, core_index, time;
	 char paxos_path[MAX_STR_NAME_SIZE];
	 FILE *paxos_conf_file;
	 
/* Simulating centralized approach. Controller core is 0 */    
#ifdef SINGLE_IDAG
	num_idags = 1;
	idag_id_arr = (int *) malloc(num_idags * sizeof(int));
	idag_id_arr[0] = 0;

	for (i=0; i<X_max*Y_max; i++)
		idag_mask[i] = 0;

	num_idags_x = 1;
#else  
	char idag_defs_path[MAX_STR_NAME_SIZE]; 
	FILE *idag_defs_file;

#ifdef PLAT_SCC
	strcpy(idag_defs_path, "/shared/herc/");
#else
	strcpy(idag_defs_path, "../");
#endif
	strcat(idag_defs_path, scen_directory);
	strcat(idag_defs_path, "/");
	strcat(idag_defs_path, scen_num);
	strcat(idag_defs_path, "/idag_defs/");
	strcat(idag_defs_path, idag_defs_file_name);

	if (node_id == 0){
		printf("Idag_defs_path = %s\n",idag_defs_path);
		fflush(stdout); 
	}

	if ((idag_defs_file = fopen(idag_defs_path, "r")) == NULL){
		printf("Cannot open input file with file path = %s ",idag_defs_path);
		perror("open idag_defs_path");
	}

	fscanf(idag_defs_file, "%d", &num_idags);

	idag_id_arr = (int *) malloc(num_idags * sizeof(int));

	for (i=0; i<num_idags; i++)
		fscanf(idag_defs_file, "%d", &idag_id_arr[i]);

	for (i = X_max*Y_max - 2*X_max; i >= 0; i -= 2*X_max) {
		for (j = i; j < i+2*X_max-1; j+=2){
			//printf("%d | ",j+1);
			fscanf(idag_defs_file, "%d", &idag_mask[j+1]);
		}
		//printf("\n");
		for (j = i; j < i+2*X_max-1; j+=2){
			//printf("%d | ",j);
			fscanf(idag_defs_file, "%d", &idag_mask[j]);
		}
		//printf("\n");
	}
	for (i=0;i <X_max*Y_max; i++){
		idag_mask[i] = idag_id_arr[idag_mask[i]];
	 }

	num_idags_x = num_idags / 2; /* FIXME deprecated, keeping for time being only for compatibility */
#endif  

	 /*dimos*/
	 //fclose(idag_defs_file);
#ifdef PLAT_SCC
	 strcpy(paxos_path, "/shared/herc/");
#else 
	 strcpy(paxos_path, "../");
#endif
	 strcat(paxos_path, scen_directory);
	 strcat(paxos_path, "/paxos/");
	 strcat(paxos_path, paxos_scen);
	 
	 if ((paxos_conf_file = fopen(paxos_path, "r")) == NULL){
	   printf("Cannot open input file with file path = %s\n ",paxos_path);
	   perror("open paxos_path");
	 }
	 
	 fscanf(paxos_conf_file,"%d", &cnt);
	 
	 for (i = 0; i < cnt; i++){
		fscanf(paxos_conf_file,"%d", &core_index);
		fscanf(paxos_conf_file,"%d", &time);
		timer_schedule[idag_id_arr[core_index]] = time;
	 }
	 
	 fclose(paxos_conf_file);
    
}

void print_grid(){
	int k,i,j;
	for (i = 0; i < 4*X_max; i++) printf("%s=",KMAG);
	printf("%s GRID ",KMAG);
	for (i = 0; i < 4*X_max; i++) printf("%s=",KMAG);
	printf("\n");
	for (i = X_max*Y_max - 2*X_max; i >= 0; i -= 2*X_max){
	     for (j = i; j < i+2*X_max-1; j+=2){
		      for (k = 0; k < num_idags; k++){
			      if (idag_mask[j+1] == idag_id_arr[k]){
				      if (k == 0){
					      printf("%s%d\t",KRED,j+1);
					      fflush(stdout);
				      }
				      else if (k == 1){
					      printf("%s%d\t",KGRN,j+1);
					      fflush(stdout);
				      }
				      else if (k == 2){
					      printf("%s%d\t",KBLU,j+1);
					      fflush(stdout);
				      }
				      else if (k == 3){
					      printf("%s%d\t",KYEL,j+1);
					      fflush(stdout);
				      }
				      else if (k == 4){
					      printf("%s%d\t",KMAG,j+1);
					      fflush(stdout);
				      }
				      else if (k == 5){
					      printf("%s%d\t",KCYN,j+1);
					      fflush(stdout);
				      }
				      else if (k == 6){
					      printf("%s%d\t",KWHT,j+1);
					      fflush(stdout);
				      }
				      else if (k == 7){
					      printf("%s%d\t",KBLK,j+1);
					      fflush(stdout);
				      }
			      }
		      }
	      }
	      printf("\n");
	      for (j = i; j < i+2*X_max-1; j+=2){
		      for (k = 0; k < num_idags; k++){
			      if (idag_mask[j] == idag_id_arr[k]){
				      if (k == 0){
					      printf("%s%d\t",KRED,j);
					      fflush(stdout);
				      }
				      else if (k == 1){
					      printf("%s%d\t",KGRN,j);
					      fflush(stdout);
				      }
				      else if (k == 2){
					      printf("%s%d\t",KBLU,j);
					      fflush(stdout);
				      }
				      else if (k == 3){
					      printf("%s%d\t",KYEL,j);
					      fflush(stdout);
				      }
				      else if (k == 4){
					      printf("%s%d\t",KMAG,j+1);
					      fflush(stdout);
				      }
				      else if (k == 5){
					      printf("%s%d\t",KCYN,j+1);
					      fflush(stdout);
				      }
				      else if (k == 6){
					      printf("%s%d\t",KWHT,j+1);
					      fflush(stdout);
				      }
				      else if (k == 7){
					      printf("%s%d\t",KBLK,j+1);
					      fflush(stdout);
				      }				      
			      }
		      }
	      }		
	      printf("%s\n",KNRM);
	}
	for (i = 0; i < 4*X_max; i++) printf("%s=",KMAG);
	printf("%s GRID ",KMAG);
	for (i = 0; i < 4*X_max; i++) printf("%s=",KMAG);
	printf("%s\n",KNRM);
}


int is_core_idag (int core_id) {
    int i, is_idag = 0;
    
    for (i=0; i<num_idags; i++)
      if (idag_id_arr[i] == core_id) {
	      is_idag = 1;
	      break;
      }
      
    return is_idag;  
}

int im_manager(){
	if (state == IDLE_AGENT ||
	    state == IDLE_AGENT_WAITING_OFF ||
	    state == AGENT_INIT_STATE ||
	    state == AGENT_ENDING ||
	    state == AGENT_INIT_APP_INIT ||
	    state == AGENT_INIT_CHK_OFFERS || 
	    state == AGENT_SELF_OPT ||
	    state == AGENT_SELF_CHK_OFFERS ||
	    state == AGENT_ZOMBIE ||
	    state == AGENT_INIT_APP_INIT ||
	    state == AGENT_INIT_IDLE_INIT ||
	    state == AGENT_ZOMBIE ||
	    pending_state == IDLE_AGENT || 
	    pending_state == IDLE_AGENT_WAITING_OFF || 
	    pending_state == AGENT_INIT_STATE || 
	    pending_state == AGENT_ENDING || 
	    pending_state == AGENT_INIT_APP_INIT || 
	    pending_state == AGENT_INIT_CHK_OFFERS || 
	    pending_state == AGENT_SELF_OPT || 
	    pending_state == AGENT_SELF_CHK_OFFERS || 
	    pending_state == AGENT_ZOMBIE || 
	    pending_state == AGENT_INIT_APP_INIT || 
	    pending_state == AGENT_INIT_IDLE_INIT)
	{
		return 1;
	}else{
		return 0;
	}
}

int im_worker(){
	if (cur_agent.my_agent == -1)
		return 0;
	else
		return 1;
}

int im_controller(){
	if (my_idag == -1)
		return 1;
	else
		return 0;
}

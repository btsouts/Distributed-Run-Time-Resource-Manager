#include "my_rtrm.h"
#include "libfunctions.h"
#include "noc_functions.h"
#include "sig_aux.h"
#include "idle_agent.h"
#include "common_node.h"
#include "signal_handlers.h"
#include "scc_signals.h"

//int *pid_num;
int num_idags, node_id=-1,my_idag=-1;//, fd_log;
FILE *log_file;
core_states state; 	
inter_list **core_inter_head,**core_inter_tail, *init_pending_head=NULL, *init_pending_tail;
/*app my_app = {.A=-1, .var=-1, .num_of_cores=-1};
app init_app = {.A=-1, .var=-1, .num_of_cores=-1};
app far_req_app = {.A=-1, .var=-1, .num_of_cores=-1};
metrics my_stats = {.msg_count=0, .message_size=0, .distance=0, .app_turnaround=0, .comp_effort=0, .cores_utilized=0, .times_accessed=0};
metrics total_stats = {.msg_count=0, .message_size=0, .distance=0, .app_turnaround=0, .comp_effort=0, .cores_utilized=0, .times_accessed=0};
region far_reg = {.C = -1, .r = -1};*/
app my_app, init_app;
metrics my_stats, total_stats;
int far_req_or_sender=-1;//far_req_max_man_cores=-1, far_req_max_man=-1, far_req_max_man_count=0, 
int *idag_mask, *idag_id_arr;
int *Cl_x_max_arr, *Cl_y_max_arr; 
int DDS_count=0, my_cores_count=0;
DDS_list *DDS=NULL, *DDS_tail;
core_list *my_cores=NULL, *my_cores_tail;
offer_list *init_man_offers=NULL, *selfopt_man_offers=NULL;
offer_list *far_man_offers=NULL, *far_man_offers_tail=NULL;
int far_list_count=0, far_man_agent_count=0;
int my_agent = -1, time_worked=0;
int debug_global=0;
time_t cur_time;
struct tm *cur_t;
struct sigevent sev;
struct itimerspec its, chk_timer;
timer_t timerid;
long int selfopt_time_rem=-1;//-1 means it is not set
long int upper_work_bound=-1;
int time_for_farman = -1;
char scen_num[4];
int chk_rem_count=0, chk_rem_num=0, sum_rem_time=0;
float old_Speedup, my_Speedup;
int nodes_ended_cnt=0, app_terminated=0;
int nodes_initialised=0, stats_replied=0, num_apps_terminated=0,num_apps=0, idags_replied=0;

int *sig_array, *data_array, NUES;
RCCE_FLAG flag_signals_enabled,flag_data_written;

int get_max_cores_count(app cur_app){
	
	if (cur_app.var < 1.0)
		return (int) ceilf(2.0*cur_app.A - 1);
	else
		return (int) ceilf(cur_app.A + cur_app.A*cur_app.var - cur_app.var);
}

float Speedup(app cur_app, int num_of_cores){
	float res=0;
	
	if (num_of_cores == 0) return 0;

	if (cur_app.var < 1.0)
		if (num_of_cores == 1) res = 1;//000000;//0;//cur_app.A;
		else if (num_of_cores > 1 && num_of_cores <= cur_app.A)
			res = (num_of_cores*cur_app.A) / (cur_app.A + (cur_app.var / (2.0*(num_of_cores-1))));			
		else if (num_of_cores >= cur_app.A && num_of_cores <= 2.0*cur_app.A - 1)
			res = (num_of_cores*cur_app.A) / (cur_app.var*(cur_app.A -0.5) + num_of_cores*(1.0 - 0.5*cur_app.var));
		else res = cur_app.A;
	else
		if (num_of_cores >= 1 && num_of_cores <= cur_app.A + cur_app.A*cur_app.var - cur_app.var)
			res = (num_of_cores*cur_app.A*(cur_app.var + 1)) / (cur_app.A + cur_app.var*(num_of_cores-1 + cur_app.var));
		else res = cur_app.A;

	return res;
}

int offer_cores(core_list *cores, app req_app, region req_reg, int *Offered_cores, int req_id){
	int Of_cores_num=0, min_dist, cur_dist;
	float gain_total=0.1,base_receiver,base_giver,gain_receiver,loss_giver,share_giver,new_gain;
	int Cores_receiver = req_app.num_of_cores, Cores_giver = my_app.num_of_cores;
	core_list *tmp, *GreedyChoice;

	tmp = cores;
	while (tmp != NULL){
		if (distance(req_reg.C, tmp->core_id) <= req_reg.r) share_giver++;
		tmp = tmp->next;
	}
	share_giver = share_giver / (float) region_count(req_reg);

	while (gain_total > 0.0){
		gain_total = 0.0;
		GreedyChoice = NULL;//-1;
		min_dist = -1;
		base_receiver = Speedup(req_app, Cores_receiver + Of_cores_num);
		if (my_idag == -1) base_giver = 0; 		
		else base_giver = Speedup(my_app, Cores_giver - Of_cores_num);
	
		//tmp = cores->next;
		if (my_idag == -1) tmp = cores->next;//very important!!! that way i avoid giving up my agent core
		else tmp = cores->next->next;//very important!!! that way i avoid giving up my only working core
 
		while (tmp != NULL){

			cur_dist = distance(req_reg.C, tmp->core_id);
			if (tmp->offered_to == -1 && cur_dist <= req_reg.r){
				if (Cores_receiver == 0 && Of_cores_num == 1 && req_app.var < 1.0) gain_receiver = 1000000;
				else gain_receiver = share_giver * (Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver);			
				
				if (my_idag == -1) loss_giver = 0;
				else loss_giver = base_giver - Speedup(my_app, Cores_giver - Of_cores_num - 1);
				
				new_gain = gain_receiver - loss_giver;
				if (new_gain > gain_total){
					gain_total = new_gain;
					min_dist = cur_dist;
					GreedyChoice = tmp;//->core_id;
					//tmp->offered_to = req_id;
				} else if (new_gain == gain_total && cur_dist < min_dist) {
					//printf("I am %d and i change offer to %d with cores %d->%d with distances %d->%d\n",node_id,req_id,GreedyChoice->core_id,tmp->core_id,min_dist,cur_dist);
					min_dist = cur_dist;
					GreedyChoice = tmp;
				}
			}

			tmp = tmp->next;
		}

		if (gain_total > 0.0) {
			Offered_cores[Of_cores_num++] = GreedyChoice->core_id;
			GreedyChoice->offered_to = req_id;//tmp
		}
	}

	return Of_cores_num;
}

void send_next_signal(inter_list *head, int node_num){
	inter_list *tmp_inter_list=NULL;

	if (head->type == IDAG_FIND_IDAGS || head->type == SELFOPT_IDAG_FIND_IDAGS || head->type == REP_IDAG_FIND_IDAGS 
		|| head->type == SELFOPT_IDAG_FIND_IDAGS_PENDING || head->type == IDAG_FIND_IDAGS_PENDING)
		scc_kill(node_num, SIG_IDAG_FIND_IDAGS);
	else if (head->type == IDAG_REQ_DDS || head->type == FAR_REQ_IDAG_REQ_DDS || head->type == SELFOPT_IDAG_REQ_DDS 
		|| head->type == SELFOPT_IDAG_REQ_DDS_PENDING || head->type == FAR_REQ_IDAG_REQ_DDS_PENDING || head->type == IDAG_REQ_DDS_PENDING)
		scc_kill(node_num, SIG_REQ_DDS);
	else if (head->type == AGENT_REQ_CORES || head->type == FAR_REQ_CORES || head->type == SELFOPT_REQ_CORES 
		|| head->type == AGENT_REQ_CORES_PENDING || head->type == SELFOPT_REQ_CORES_PENDING || head->type == FAR_REQ_CORES_PENDING)
		scc_kill(node_num, SIG_REQ_CORES);
	else if (head->type == FAR_REQ_MAN_APPOINT || head->type == FAR_REQ_OFFER)// || head->type == FAR_REQ_MAN_APPOINT_PENDING
		scc_kill(node_num, SIG_FAR_REQ);
	else if (head->type == IDAG_ADD_CORES_DDS)
		scc_kill(node_num, SIG_ADD_CORES_DDS);
	else if (head->type == IDAG_REM_CORES_DDS)
		scc_kill(node_num, SIG_REM_CORES_DDS);
	else if (head->type == INIT_WORK_NODE || head->type == APPOINT_WORK_NODE)
		scc_kill(node_num, SIG_APPOINT_WORK);
	else if (head->type == REMOVE_APP)
		scc_kill(node_num, SIG_FINISH);
	else if (head->type == INIT_APP)
		scc_kill(node_num, SIG_INIT_APP);
	else if (head->type == FAR_INIT_REQ)
		scc_kill(node_num, SIG_INIT_FAR_REQ);
	else if (head->type == REP_AGENT_REQ_CORES)
		scc_kill(node_num, SIG_REQ_CORES);
	else if (head->type == INIT_AGENT)
		scc_kill(node_num, SIG_INIT_AGENT);
	else if (head->type == ABORT_FAR_MAN)
		scc_kill(node_num, SIG_REMOVE_FAR_MAN);
	else if (head->type == REP_CHK_REM_TIME || head->type == APPOINT_WORK_NODE_PENDING || head->type == REMOVED_NODE_REM_TIME) {//|| head->type == INIT_WORK_NODE_PENDING 
		scc_kill(node_num, SIG_CHECK_REM_TIME);
		fprintf(log_file,"I sent in send next SIG_CHECK_REM_TIME to node %d\n",node_num);
		fflush(log_file);
	}	else if (head->type == NOTIFY_APP_TERMINATION) {
		scc_kill(0, SIG_APP_TERMINATED);
	
		tmp_inter_list = core_inter_head[0];
		core_inter_head[0] = core_inter_head[0]->next;
		if (core_inter_head[0] == NULL) core_inter_tail[0] = NULL;
		else send_next_signal(core_inter_head[0],0);
		free(tmp_inter_list);
	} else if (head->type != FAR_REQ_MAN) {
		printf("undefined state in send_next %d node_num=%d\n",head->type,node_num);
		fprintf(log_file,"undefined state in send_next %d node_num=%d\n",head->type,node_num);
		fflush(log_file);
	}
		
	my_stats.msg_count++;
	my_stats.distance += distance(node_id,node_num);
}

int RCCE_APP(int argc, char *argv[]){
	int Cl_x_max, Cl_y_max;//, num_idags_x, num_idags_y,i;
	int num_of_bytes,segment_id;	
	int i,j,k;
	int ans=0;
	int one_idag, one_core;
	DDS_list *tmp_DDS;
	pid_t p;
	core_list *tmp_cores_list;
	inter_list *tmp_inter_list;
	float avg_cluster_util;
	
	/*if (argc < 6) {
		printf("usage: ./my_rtrm scenario_number X_max Y_max num_idags_x num_idags_y\n");
		exit(1);
	}*/	
	RCCE_init(&argc, &argv);
	node_id = RCCE_ue();
	NUES = RCCE_num_ues();
	RCCE_flag_alloc(&flag_signals_enabled);
	RCCE_flag_alloc(&flag_data_written);
	RCCE_flag_write(&flag_signals_enabled, RCCE_FLAG_UNSET, node_id);
	RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);
	sig_array = (int *) RCCE_malloc(NUES * LINE_SIZE * sizeof(int));//NUES * NUES
	data_array = (int *) RCCE_malloc(3 * LINE_SIZE * sizeof(int));
	//sig_array_local = (int *) malloc(LINE_SIZE * sizeof(int));
	//data_array_local = (int *) malloc(LINE_SIZE * sizeof(int));
	//sig_read_ar = (int *) malloc(LINE_SIZE * sizeof(int));

	strcpy(scen_num,argv[1]);
	/*X_max = atoi(argv[2]);
	Y_max = atoi(argv[3]);
	num_idags_x = atoi(argv[4]);
	num_idags_y = atoi(argv[5]);*/
	num_idags = num_idags_x * num_idags_y;
	//printf("scen_num = %s\n",scen_num);
	//printf("num_idags = %d\n",num_idags);
	
	core_inter_head = (inter_list **) malloc(X_max*Y_max*sizeof(inter_list *));
	core_inter_tail = (inter_list **) malloc(X_max*Y_max*sizeof(inter_list *));
	for (i=0; i<X_max*Y_max; i++){
		core_inter_head[i] = NULL;
		core_inter_tail[i] = NULL;
	}

	my_stats.msg_count=0;
	my_stats.message_size=0;
	my_stats.distance=0;
	my_stats.app_turnaround=0;
	my_stats.comp_effort=0;
	my_stats.cores_utilized=0;
	my_stats.times_accessed=0;
	total_stats.msg_count=0;
	total_stats.message_size=0;
	total_stats.distance=0;
	total_stats.app_turnaround=0;
	total_stats.comp_effort=0;
	total_stats.cores_utilized=0;
	total_stats.times_accessed=0;
	my_app.A=-1;
	my_app.var=-1;
	my_app.num_of_cores=-1;
	init_app.A=-1;
	init_app.var=-1;
	init_app.num_of_cores=-1;

	if (node_id == 3) { 
		idle_agent_actions(1, scen_num);
	} else if (node_id == 18) {
		idle_agent_actions(2, scen_num);
	} else if (node_id == 21) {
		idle_agent_actions(3, scen_num);
	} else if (node_id != 0) {
		common_node_actions(scen_num);
	} else {
		i = get_cluster_info(0, &Cl_x_max, &Cl_y_max);
		if (i != node_id) printf("I am %d and i was %d\n",node_id,i);
		idag_id_arr = (int *) malloc(num_idags*sizeof(int));
		Cl_x_max_arr = (int *) malloc(num_idags*sizeof(int));
		Cl_y_max_arr = (int *) malloc(num_idags*sizeof(int));
		idag_mask = (int *) malloc(X_max*Y_max*sizeof(int));
	
		for (i=0; i<num_idags; i++){
			idag_id_arr[i] = get_cluster_info(i, &Cl_x_max_arr[i], &Cl_y_max_arr[i]);
			for (j=idag_id_arr[i]; j<idag_id_arr[i] + Cl_y_max_arr[i]*X_max; j+=X_max)
				for (k=0; k<Cl_x_max_arr[i]; k++) 
					idag_mask[j+k] = idag_id_arr[i];
		}

		//printf("node_id = %d\n",node_id);			
		log_file = create_log_file(node_id, scen_num);
		cur_time = time(NULL);	
		cur_t = localtime(&cur_time);
		fprintf(log_file, "[%d:%d:%d]: I start initialising node_id=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,node_id);
		fflush(log_file);
	
		install_signal_handlers();
		//sig_SEGV_enable();
		sev.sigev_notify = SIGEV_SIGNAL;
		sev.sigev_signo = SIG_TIMER;
		sev.sigev_value.sival_ptr = &timerid;
		if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1) printf("timer_create error\n");
		
		for (j=node_id; j<node_id+Cl_x_max*X_max; j+=X_max)
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
							
				if ((j+k) == node_id) {
					DDS = (DDS_list *) malloc(sizeof(DDS_list));
					DDS->agent_id = j+k;
					DDS->num_of_cores = Cl_x_max*Cl_y_max;
					DDS->next = NULL;
					DDS_tail = DDS;
					DDS_count++;
					//pid_num[j+k] = getpid();
				} 
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
					//kill(pid_num[one_core], SIG_INIT);
					scc_kill(one_core, SIG_INIT);
					//my_stats.msg_count++;
					//my_stats.distance += distance(node_id,one_core);				
					signals_enable();
				}

		int dummy=0;
		while (nodes_initialised != my_cores_count-1) {//pause(); my_cores_count
			for (i=0; i<1000; i++)
				for(j=0; j<1000; j++)
					dummy++;

			scc_signals_check();
		}
		//sleep(1);
		printf("End of initialisation\n");

		FILE *app_input;
		char app_input_file_name[64];
		int app_cnt=0,time_passed=-1,time_next,init_core;
		offer_list *tmp_offer_list;

		strcpy(app_input_file_name,"/shared/herc/scenaria/");
		strcat(app_input_file_name, argv[1]);
		strcat(app_input_file_name, "/app_input.txt");
		printf("file path = %s\n",app_input_file_name);

		if ((app_input = fopen(app_input_file_name, "r")) == NULL){
			printf("Cannot open input file with file path = %s ",app_input_file_name);		
			perror("open app_input");
		}
		fscanf(app_input,"%d",&time_next);

		state = IDLE_CHK_APP_FILE;
		//state = CHK_APP_FILE;	
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;
		its.it_value.tv_sec = 0;
		its.it_value.tv_nsec = 10 * MS;
		if (timer_settime(timerid, 0, &its, NULL) == -1) perror("timer_settime error9");

		while (state != IDAG_ENDING)
			if (state == IDLE_IDAG || state == IDLE_FAR_MAN || state == IDLE_CHK_APP_FILE) {
				//pause();
				dummy=0;
				for (i=0; i<1000; i++)
					for(j=0; j<1000; j++)
						dummy++;

				scc_signals_check();
			} else if (state == CHK_APP_FILE) {
				signals_disable();
			
				time_passed++;
				if (time_for_farman > 0) time_for_farman -= 10;
			
				if (time_next == time_passed) {
					fscanf(app_input,"%d",&init_core);
					num_apps++;

					for (tmp_inter_list = core_inter_head[init_core]; tmp_inter_list != NULL; tmp_inter_list = tmp_inter_list->next)
						if (tmp_inter_list->type == INIT_APP) break;
				
					if (tmp_inter_list == NULL) {
						if (core_inter_head[init_core] == NULL){
							core_inter_head[init_core] = (inter_list *) malloc(sizeof(inter_list));
							core_inter_tail[init_core] = core_inter_head[init_core];
						} else {
							core_inter_tail[init_core]->next = (inter_list *) malloc(sizeof(inter_list));
							core_inter_tail[init_core] = core_inter_tail[init_core]->next;
						}

						core_inter_tail[init_core]->type = INIT_APP;
						fscanf(app_input,"%f",&core_inter_tail[init_core]->data.new_app.workld);
						fscanf(app_input,"%f",&core_inter_tail[init_core]->data.new_app.A);
						fscanf(app_input,"%f",&core_inter_tail[init_core]->data.new_app.var);
			
						core_inter_tail[init_core]->data.new_app.id = app_cnt++;
						core_inter_tail[init_core]->data.new_app.num_of_cores = 0;
						core_inter_tail[init_core]->next = NULL;
				
						printf("time = %d, id = %d, workld = %0.2f \n",time_passed,core_inter_tail[init_core]->data.new_app.id,core_inter_tail[init_core]->data.new_app.workld);
						cur_time = time(NULL);	
						cur_t = localtime(&cur_time);
						fprintf(log_file, "[%d:%d:%d]: Initialising app_id=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,core_inter_tail[init_core]->data.new_app.id);
						fflush(log_file);
						//printf("A = %0.2f, var = %0.2f\n",core_inter_tail[init_core]->data.new_app.A,core_inter_tail[init_core]->data.new_app.var);
						if (core_inter_head[init_core]->next == NULL) {
							//kill(pid_num[init_core],SIG_INIT_APP);
							scc_kill(init_core,SIG_INIT_APP); 
							my_stats.msg_count++;
							my_stats.distance += distance(node_id,init_core);
						}
					} else {
						if (init_pending_head == NULL){
							init_pending_head = (inter_list *) malloc(sizeof(inter_list));
							init_pending_tail = init_pending_head;
						} else {
							init_pending_tail->next = (inter_list *) malloc(sizeof(inter_list));
							init_pending_tail = init_pending_tail->next;
						} 

						init_pending_tail->type = INIT_APP;
						fscanf(app_input,"%f",&init_pending_tail->data.new_app.workld);
						fscanf(app_input,"%f",&init_pending_tail->data.new_app.A);
						fscanf(app_input,"%f",&init_pending_tail->data.new_app.var);
			
						init_pending_tail->data.new_app.id = app_cnt++;
						//apparently i want num_of_cores to be 0. But i will temporarilly use it as an indicator of init_core so as not
						//change inter_list	type or introduse an a new data union structure
						init_pending_tail->data.new_app.num_of_cores = init_core;
						init_pending_tail->next = NULL;
					}

					if (fscanf(app_input,"%d",&time_next) == EOF) {
						if (time_for_farman == 0 || time_for_farman == -5) state = FAR_MAN_CHK_OFFERS;
						else if (time_for_farman > 0){
							its.it_value.tv_nsec = time_for_farman * 10 * MS;
							if (timer_settime(timerid, 0, &its, NULL) == -1) printf("timer_settime error10\n");	
							state = USER_INPUT;
						}	else state = USER_INPUT;
						time_passed = -1;
					} else {
						if (time_for_farman == 0 || time_for_farman == -5) state = FAR_MAN_CHK_OFFERS;
						else {
							its.it_value.tv_nsec = 10 * MS;
							if (timer_settime(timerid, 0, &its, NULL) == -1) printf("timer_settime error11\n");
							state = IDLE_CHK_APP_FILE;
						}
					}
				} else {
					state = IDLE_CHK_APP_FILE;
					its.it_value.tv_nsec = 10 * MS;
					if (timer_settime(timerid, 0, &its, NULL) == -1) printf("timer_settime error92\n");
				}

				signals_enable();
			/*} else if (state == FAR_MAN_CHK_OFFERS) {
				signals_disable();
				printf("far check alarm went off in idag %d far_req_or_sender = %d!\n",node_id,far_req_or_sender);
				cur_time = time(NULL);	
				cur_t = localtime(&cur_time);
				fprintf(log_file, "[%d:%d:%d]: far check alarm went off in idag %d! far_req_or_sender = %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,node_id,far_req_or_sender);
				fflush(log_file);		

				if (far_man_offers == NULL) fprintf(log_file,"far_man_offers is null far_list_count = %d\n",far_list_count);
				else fprintf(log_file,"far_man_offers is not null far_list_count = %d\n",far_list_count);
				fflush(log_file);
				
				tmp_offer_list = far_man_offers;
				while (tmp_offer_list != NULL){
					fprintf(log_file,"Offer by %d for %d cores\n",tmp_offer_list->sender,tmp_offer_list->off.num_of_cores);
					fflush(log_file);
					//tmp_offer_list->answer = &core_inter_head[sender_id]->data.offer_accepted; must be a serious bug
					//tmp_offer_list->answer = &core_inter_head[tmp_offer_list->sender]->data.offer_accepted;
					tmp_offer_list = tmp_offer_list->next;
				}

				if (core_inter_head[far_req_or_sender] == NULL){
					core_inter_head[far_req_or_sender] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[far_req_or_sender] = core_inter_head[far_req_or_sender];
				} else {
					core_inter_tail[far_req_or_sender]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[far_req_or_sender] = core_inter_tail[far_req_or_sender]->next;
				}

				core_inter_tail[far_req_or_sender]->type = FAR_REQ_OFFER;
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
			
				//if (selfopt_time_rem != -1) printf("selfopt timer in idag??\n");
				time_for_farman = -1;
				if (time_passed == -1) state = USER_INPUT;
				else {
					state = IDLE_CHK_APP_FILE;
					its.it_value.tv_nsec = 10 * MS;
					if (timer_settime(timerid, 0, &its, NULL) == -1) printf("timer_settime error112\n");
					state = IDLE_CHK_APP_FILE;
				}	
				signals_enable();*/
			} else if (state == USER_INPUT){
				while (num_apps_terminated != num_apps) {//pause(); my_cores_count
					dummy=0;
					for (i=0; i<1000; i++)
						for(j=0; j<1000; j++)
							dummy++;

					scc_signals_check();
				}

				for (j=0; j<num_idags; j++) {
              one_idag = idag_id_arr[j];

              if (one_idag != 0){
                if (core_inter_head[one_idag] == NULL){
                  core_inter_head[one_idag] = (inter_list *) malloc(sizeof(inter_list));
                  core_inter_tail[one_idag] = core_inter_head[one_idag];
                } else {
                  core_inter_tail[one_idag]->next = (inter_list *) malloc(sizeof(inter_list));
                  core_inter_tail[one_idag] = core_inter_tail[one_idag]->next;
                }

                core_inter_tail[one_idag]->type = DEBUG_IDAG_REQ_DDS;
                core_inter_tail[one_idag]->data.reg.C = -1;
                core_inter_tail[one_idag]->data.reg.r = 0;
                core_inter_tail[one_idag]->next = NULL;

                if (core_inter_head[one_idag]->next == NULL) scc_kill(one_idag, SIG_REQ_DDS);//kill(pid_num[one_idag], SIG_REQ_DDS);
                else printf("what the fuck? interaction is %d\n",core_inter_head[one_idag]->type);
              } else {
                printf("Number of agents in region = %d\n",DDS_count);
                tmp_DDS = DDS;
                i=0;
                while (tmp_DDS != NULL){
                  printf("Agent no %d is %d with %d cores\n",i,tmp_DDS->agent_id,tmp_DDS->num_of_cores);
                  tmp_DDS = tmp_DDS->next;
                  i++;
                }
              }
      }
	
				while (idags_replied < num_idags - 1) {
          dummy=0;
	        for (i=0; i<1000; i++)
        		for(j=0; j<1000; j++)
              dummy++;

      		scc_signals_check();
   			}		

	
				fprintf(log_file,"killing\n");
				fflush(log_file);
				for (i=1; i<num_idags; i++) {
					printf("i am killing %d\n",idag_id_arr[i]);
					one_core = idag_id_arr[i];
				
					if (core_inter_head[one_core] == NULL){
						core_inter_head[one_core] = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[one_core] = core_inter_head[one_core];
					} else {
						core_inter_tail[one_core]->next = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[one_core] = core_inter_tail[one_core]->next;
						fprintf(log_file,"I am still doing smth with idag %d interaction = %d\n",one_core,core_inter_head[one_core]->type);
						fflush(log_file);
					}

					core_inter_tail[one_core]->type = TERMINATION_STATS;
					core_inter_tail[one_core]->next = NULL;	

					scc_kill(one_core, SIG_TERMINATE);
					//kill(pid_num[idag_id_arr[i]], SIG_TERMINATE);
					//my_stats.msg_count++;
					//my_stats.distance += distance(node_id,idag_id_arr[i]);
				}

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
			
				state = IDAG_ENDING;

			} else {
				printf("Uknown state node_id = %d state = %d\n",node_id,state);	
				state = IDLE_IDAG;
			}

		while (state == IDAG_ENDING) {
			//pause();
			dummy=0;
			for (i=0; i<1000; i++)
				for(j=0; j<1000; j++)
					dummy++;

			scc_signals_check();

			if (stats_replied == my_cores_count+num_idags-2) state = TERMINATED;
		}

		//for (i=0; i<Cl_x_max*Cl_y_max-1; i++) wait(NULL); //wait for children
		//for (i=0; i<num_idags-1; i++) wait(NULL);//wait for the other idags
	
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

		printf("Total stats are:\n");
		printf("Total message count = %d\n",total_stats.msg_count);
		printf("Total message size = %d\n",total_stats.message_size);
		printf("Total distance = %d\n",total_stats.distance);
		printf("Total app turnaround time = %d\n",total_stats.app_turnaround);
		printf("Total computational effort = %d\n",total_stats.comp_effort);
		printf("Total cores_utilized = %d\n",total_stats.cores_utilized); 
		printf("Total times_accessed = %d\n",total_stats.times_accessed);
		
		free(idag_mask);

		free(idag_id_arr);
		free(Cl_x_max_arr);
		free(Cl_y_max_arr);
		for (i=0; i<X_max*Y_max; i++){
			free(core_inter_head[i]);
			free(core_inter_tail[i]);
		}
		free(core_inter_head);
		free(core_inter_tail);		

		RCCE_flag_free(&flag_signals_enabled);
         	RCCE_flag_free(&flag_data_written);
         	RCCE_free((t_vcharp) sig_array);
         	RCCE_free((t_vcharp) data_array);

		cur_time = time(NULL);	
		cur_t = localtime(&cur_time);
		fprintf(log_file, "[%d:%d:%d]: I ended well\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);	
		fclose(log_file);
	}

	return 0;
}

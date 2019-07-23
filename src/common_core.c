#include "include/common_core.h"
#include "include/my_rtrm.h"
#include "include/idag_defs.h" //PAXOS
#include "include/libfunctions.h"
#include "include/noc_functions.h"
#include "include/sig_aux.h"
#include "include/signal_handlers.h"
#include "include/scc_signals.h"
#include "include/structs.h"
#include "include/apps.h"

int worker_app_id = -1;
int init_areas_num;
int old_cores_cnt = 0;
int active_working_cores = 0;
int base_offset = -1;
int init_DDS_replies;
int init_DDS_idags;
int selfopt_DDS_replies;
int selfopt_DDS_idags;
int selfopt_interval=LEAST_SELF_OPT_INTERVAL_MS;
int max_cores_count;

int *alive;
int *suspected;

long int selfopt_time_rem = -1;//-1 means it is not set

float old_Speedup;
float my_Speedup;

app my_app;
app init_app;

application_states app_state;

core_states pending_state = NO_PENDING_STATE;

core_states paxos_state;

offer_list *init_man_offers = NULL;
offer_list *selfopt_man_offers = NULL;

target_list *init_targets_head = NULL;
target_list *init_targets_tail;
target_list *selfopt_targets_head = NULL;
target_list *selfopt_targets_tail;

coworkers_list *coworkers;

my_time_stamp init_app_times[2];
my_time_stamp my_app_times[2];

agent_info cur_agent;
agent_info pending_agent;

FILE *app_log_file;

app_exec executed_app;

void common_node_actions(char scen_directory[SCEN_DIR_SIZE], char scen_num[SCEN_NUM_SIZE], int Selfopt_Radius, int Max_SelfOpt_Interval_MS) {
  
	int i;
	int j;
	int tmp_int;
	int one_core;
	int new_agent;
	int min_dist;
	int row_reached;
	int per_core_rows;
	int rows_left;
		
	char app_log_file_name[128];
	offer_list *tmp_offer_list;
	core_list *tmp_cores_list;
	target_list *tmp_target_list;
	inter_list *tmp_inter_list, *tmp_inter_prev;
		
	if (paxos_state != NEW_AGENT) {
	  
		init_app_times[0].tm_sec = 0;
		init_app_times[0].tm_min = 0;
		init_app_times[0].tm_hour = 0;
		init_app_times[0].tm_usec = 0;
		init_app_times[1].tm_sec = 0;
		init_app_times[1].tm_min = 0;
		init_app_times[1].tm_hour = 0;
		init_app_times[1].tm_usec = 0;
		my_app_times[0].tm_sec = 0;
		my_app_times[0].tm_min = 0;
		my_app_times[0].tm_hour = 0;
		my_app_times[0].tm_usec = 0;
		my_app_times[1].tm_sec = 0;
		my_app_times[1].tm_min = 0;
		my_app_times[1].tm_hour = 0;
		my_app_times[1].tm_usec = 0;
	  
	  	my_cores_count=0;
		my_stats.msg_count=0;
		my_stats.fd_msg_count=0;
		my_stats.message_size=0;
		my_stats.distance=0;
		my_stats.app_turnaround=0;
		my_stats.comp_effort=0;
		my_stats.cores_utilized=0;
		my_stats.times_accessed=0;
		
		paxos_node_stats.msg_count=0;
		paxos_node_stats.fd_msg_count=0;
		paxos_node_stats.message_size=0;
		paxos_node_stats.distance=0;
		paxos_node_stats.app_turnaround=0;
		paxos_node_stats.comp_effort=0;
		paxos_node_stats.cores_utilized=0;
		paxos_node_stats.times_accessed=0;

#ifndef ARTIFICIAL_APPS_SIM
		my_app.array_size=-1;
#else
		my_app.var = 0.0;
		my_app.A = 0.0;
#endif
		my_app.workld=-1;
		my_app.num_of_cores=-1;
		
#ifndef ARTIFICIAL_APPS_SIM
		init_app.array_size=-1;
#else
		init_app.var = 0.0;
		init_app.A = 0.0;
#endif
		init_app.workld=-1;
		init_app.num_of_cores=-1;
		
		index_bottom[node_id] = 0;
		
		alive = (int *)malloc(X_max*Y_max*sizeof(int));
		suspected = (int *)malloc(X_max*Y_max*sizeof(int));
		for (i = 0; i < X_max*Y_max; i++){
			alive[i] = 1;
			suspected[i] = 0;
		}

		#ifdef PLAT_LINUX
		for (i=(node_id * MAX_SIGNAL_LIST_LEN * LINE_SIZE); i<((node_id + 1) * MAX_SIGNAL_LIST_LEN * LINE_SIZE); i++) {
				sig_array[i] = NO_SIG;
		}

		//semaphore inits
		if (sem_init(&scc_lock[node_id], 1, 1) == -1){
			printf("I am %d error\n",node_id);
			perror("sem_init");
		}

		if (sem_init(&flag_data_written[node_id], 1, 0) == -1){
			printf("I am %d error\n",node_id);
			perror("sem_init");
		}
		#endif

		cur_agent.my_agent = -1;
		cur_agent.array_size = -1;
		cur_agent.work_bounds[0]=0;
		cur_agent.work_bounds[1]=0;
		
		pending_agent.my_agent = -1;
		pending_agent.array_size = -1;
		pending_agent.work_bounds[0]=0;
		pending_agent.work_bounds[1]=0;
		
		my_cores=NULL;
		
		install_signal_handlers();
		
		sev.sigev_notify = SIGEV_SIGNAL;
		sev.sigev_signo = SIG_TIMER;
		sev.sigev_value.sival_ptr = &timerid;
		if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1) printf("timer_create error\n");
		its.it_value.tv_sec = 0;
		its.it_interval.tv_sec = 0;//its.it_value.tv_sec;
		its.it_interval.tv_nsec = 0;
		
		if (log_file == NULL){
			log_file = create_log_file(node_id, 0, scen_directory, scen_num);
			setbuf(log_file, NULL);
		}
		
		cur_time = time(NULL);
		cur_t = localtime(&cur_time);
		fprintf(log_file, "[%d:%d:%d]: Initialisation complete Selfopt R = %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,Selfopt_Radius);
		
		#ifdef PLAT_SCC
		RCCE_barrier(&RCCE_COMM_WORLD);
		#else
		sleep(1);
		#endif

		state = IDLE_CORE;
		app_init(scen_directory, scen_num);
		/* Something must return here */	
		
		fprintf(log_file, "Cache loading complete\n");

		my_idag = -1;
		while (my_idag == -1) {
			scc_pause();
			scc_signals_check();
		}

		#ifdef PLAT_SCC
		RCCE_barrier(&RCCE_COMM_WORLD);
		#else
		sleep(1);
		#endif
	}
	
	 while (state != TERMINATED)
		if (state == IDLE_CORE) {
			pending_state = NO_PENDING_STATE;
			scc_pause();
			scc_signals_check();
		} else if (state == IDLE_INIT_MAN) {
			if (app_state == APP_TERMINATED) {
				state = AGENT_ENDING;
				pending_state = IDLE_INIT_MAN;
				fprintf(log_file,"I enter hell\n");
			}  
		  
			scc_pause();
			scc_signals_check();
		} else if (state == IDLE_AGENT) {
			if (app_state == APP_TERMINATED) state = AGENT_ENDING;
			scc_pause();
			scc_signals_check();

			if (app_state != APP_TERMINATED) {
				scc_pause();
				scc_signals_check();
			} else {
				selfopt_time_rem = my_gettimer();
				
				if (selfopt_time_rem > 0) 
					my_settimer(0);
				
				selfopt_time_rem = -1;
				state = AGENT_ENDING; 
			}
		} else if (state == IDLE_AGENT_WAITING_OFF) {
			//pause();
			scc_pause();
			scc_signals_check();
			/*if (app_state == APP_TERMINATED) {
				signals_disable();
				
				selfopt_time_rem = my_gettimer();
				//den stamataw edw thn diadikasia tou selfopt gia na mhn meinoun oi mexri twra prosfores kai oxi mono anapanthtes
				if (selfopt_time_rem == 0) {//state = AGENT_SELF_CHK_OFFERS;
					fprintf(log_file,"Timer is zero\n");
					if (selfopt_man_offers == NULL)
						state = AGENT_ENDING;
					else state = AGENT_SELF_CHK_OFFERS;
				}
				signals_enable();
			}*/
		} else if (state == INIT_MANAGER) {
			signals_disable();
			//printf("Initialising app node_id = %d\n",node_id);
			cur_time = time(NULL);	
			cur_t = localtime(&cur_time);
			fprintf(log_file, "[%d:%d:%d]: Initialising app\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);

			state = INIT_MANAGER_SEND_OFFERS;
			
			signals_enable();
			scc_signals_check();
		} else if (state == INIT_MANAGER_SEND_OFFERS) {
			signals_disable();
			init_DDS_idags = 0;
			init_DDS_replies = 0;
			
			if (init_targets_head != NULL)
				while (init_targets_head != NULL){
					tmp_target_list = init_targets_head;
					init_targets_head = init_targets_head->next;
					free(tmp_target_list);
				}
	
			if (init_man_offers != NULL)
				while (init_man_offers != NULL) {
					tmp_offer_list = init_man_offers;
					init_man_offers = init_man_offers->next;
					free(tmp_offer_list);
				}
			
			fprintf(log_file, "near init area (%d,%d)\n",node_id,Selfopt_Radius);
			if (core_inter_head[my_idag] == NULL){
				core_inter_head[my_idag] = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[my_idag] = core_inter_head[my_idag];
			} else {
				core_inter_tail[my_idag]->next = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[my_idag] = core_inter_tail[my_idag]->next;
			}

			#ifndef ADAM_SIM
			core_inter_tail[my_idag]->type = IDAG_FIND_IDAGS_PENDING;
			core_inter_tail[my_idag]->data.reg.C = node_id-1; /* FIXME Why -1 ??? */
			core_inter_tail[my_idag]->data.reg.r = Selfopt_Radius;
			core_inter_tail[my_idag]->next = NULL;
			
			if (core_inter_head[my_idag]->next == NULL) {
				//kill(pid_num[my_idag],SIG_IDAG_FIND_IDAGS);
				scc_kill(my_idag, SIG_IDAG_FIND_IDAGS, core_inter_head[my_idag]);
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,my_idag);
			} else {
				fprintf(log_file, "common_core.c: Did not send idag_find_idags with interaction %s no2 %s\n",inter2string(core_inter_head[my_idag]->type),inter2string(core_inter_head[my_idag]->next->type));
			}
			#else
			core_inter_tail[my_idag]->type = IDAG_REQ_DDS_PENDING;
			core_inter_tail[my_idag]->data.reg.C = node_id-1; /* FIXME Why -1 ??? */
			core_inter_tail[my_idag]->data.reg.r = Selfopt_Radius;
			core_inter_tail[my_idag]->next = NULL;
			init_DDS_idags = 1; /* Necessary to override SIG_FIND_IDAGS */

			if (core_inter_head[my_idag]->next == NULL) {
				//kill(pid_num[my_idag],SIG_IDAG_FIND_IDAGS);
				scc_kill(my_idag, SIG_REQ_DDS, core_inter_head[my_idag]);
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,my_idag);
			} else {
				fprintf(log_file, "Adam common_core.c: Did not send idag_req_dds with interaction %s no2 %s\n",inter2string(core_inter_head[my_idag]->type),inter2string(core_inter_head[my_idag]->next->type));
			}
			#endif
			if (selfopt_time_rem != -1) { 
				selfopt_time_rem = my_gettimer();

				if (selfopt_time_rem > 0) 
					my_settimer(0);
			}

			if (pending_state == WORKING_NODE) {
				fprintf(log_file, "I change to working idle init\n");
				state = WORKING_NODE;
				pending_state = IDLE_INIT_MAN;
			} else	
				state = IDLE_INIT_MAN;
			signals_enable();
			scc_signals_check();
		} else if (state == INIT_MAN_CHK_OFFERS) {
		  
			signals_disable();
			cur_time = time(NULL);
			cur_t = localtime(&cur_time);
			fprintf(log_file, "[%d:%d:%d]: init check alarm went off init_DDS_idags = %d init_DDS_replies = %d state=%s pending=%s\n",
				cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,init_DDS_idags,init_DDS_replies,id2string(state),id2string(pending_state));
			//init_core_found = 0;

			if (init_man_offers == NULL) {
				//printf("I don't have offers\n");
				fprintf(log_file, "I don't have init man offers\n");

				if (app_state == APP_TERMINATED) {
					pending_state = INIT_MANAGER_SEND_OFFERS;
					state = AGENT_ENDING;
				} else state = INIT_MANAGER_SEND_OFFERS;
 				//state = INIT_MANAGER_SEND_OFFERS;
			} else {
				tmp_offer_list = init_man_offers;
				while (tmp_offer_list != NULL){
					fprintf(log_file,"Offer by %d for %d cores with spd_loss %0.4f :",tmp_offer_list->sender,tmp_offer_list->off.num_of_cores,tmp_offer_list->off.spd_loss);
					for (i=0; i<tmp_offer_list->off.num_of_cores; i++)
						fprintf(log_file," %d,",tmp_offer_list->off.offered_cores[i]);
					fprintf(log_file,"\n");	
					tmp_offer_list = tmp_offer_list->next;
				}

				*init_man_offers->answer = 1;
			
				if (low_voltage_core[init_man_offers->off.offered_cores[0]]) {
					new_agent = init_man_offers->off.offered_cores[0];
					printf("I am %d and explicitly assigned %d as the new agent!\n",node_id,new_agent);
					fprintf(log_file,"I explicitly assigned %d as the new agent!\n",new_agent);
				} else {  
					//decide which will be the agent. Will be the one tha minimizes distance between cores
					min_dist = -1;					
					for (i=0; i<init_man_offers->off.num_of_cores; i++){
						tmp_int = 0;
						for (j=0; j<init_man_offers->off.num_of_cores; j++){
							if (i == j) continue;
							tmp_int += distance(init_man_offers->off.offered_cores[i],init_man_offers->off.offered_cores[j]);
						}
						if (min_dist == -1 || tmp_int < min_dist) {
							min_dist = tmp_int;
							new_agent = init_man_offers->off.offered_cores[i];
						}
					} 
				}
				
				gettimeofday(&time_val, NULL);
				cur_t = localtime(&time_val.tv_sec);
				init_app_times[1].tm_sec = cur_t->tm_sec;
				init_app_times[1].tm_min = cur_t->tm_min;
				init_app_times[1].tm_hour = cur_t->tm_hour;
				init_app_times[1].tm_usec = time_val.tv_usec;

				//printf("New agent is %d\n",new_agent);
				cur_time = time(NULL);
				cur_t = localtime(&cur_time);
				fprintf(log_file, "[%d:%d:%d]: New agent is %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,new_agent);
				printf("[%d:%d:%d]: New agent is %d for app %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,new_agent,init_app.id);
				#ifdef MANAGER
				/*if (init_app.id == 5){
					scc_kill(new_agent, SIG_FAIL, NULL);
				}*/
				#endif
				
				if (new_agent != node_id) {
					if (core_inter_head[new_agent] == NULL){
						core_inter_head[new_agent] = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[new_agent] = core_inter_head[new_agent];
					} else {
						printf("Starting this app is not high priority. Should we change that?\n");
						core_inter_tail[new_agent]->next = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[new_agent] = core_inter_tail[new_agent]->next;
					}

					core_inter_tail[new_agent]->type = INIT_AGENT;
					core_inter_tail[new_agent]->data.one_app.new_app = init_app;
					core_inter_tail[new_agent]->data.one_app.new_app.num_of_cores = init_man_offers->off.num_of_cores;

					core_inter_tail[new_agent]->data.one_app.new_app_times[0].tm_sec = init_app_times[0].tm_sec;
					core_inter_tail[new_agent]->data.one_app.new_app_times[0].tm_min = init_app_times[0].tm_min;
					core_inter_tail[new_agent]->data.one_app.new_app_times[0].tm_hour = init_app_times[0].tm_hour;
					core_inter_tail[new_agent]->data.one_app.new_app_times[0].tm_usec = init_app_times[0].tm_usec;
					core_inter_tail[new_agent]->data.one_app.new_app_times[1].tm_sec = init_app_times[1].tm_sec;
					core_inter_tail[new_agent]->data.one_app.new_app_times[1].tm_min = init_app_times[1].tm_min;
					core_inter_tail[new_agent]->data.one_app.new_app_times[1].tm_hour = init_app_times[1].tm_hour;
					core_inter_tail[new_agent]->data.one_app.new_app_times[1].tm_usec = init_app_times[1].tm_usec;
					
					core_inter_tail[new_agent]->data.one_app.new_app_cores = (int *)malloc((init_man_offers->off.num_of_cores)*sizeof(int));
					for(i=0; i<init_man_offers->off.num_of_cores; i++)
						core_inter_tail[new_agent]->data.one_app.new_app_cores[i] = init_man_offers->off.offered_cores[i];

					core_inter_tail[new_agent]->next = NULL;
					init_app.num_of_cores = init_man_offers->off.num_of_cores;

					if (core_inter_head[new_agent]->next == NULL) {
						//kill(pid_num[new_agent], SIG_INIT_AGENT);
						scc_kill(new_agent, SIG_INIT_AGENT, core_inter_head[new_agent]);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,new_agent);
					} else {
						fprintf(log_file,"I am init manager and i am doing smth else with init agent interaction=%d\n",core_inter_head[new_agent]->type);
					}
				} else {
					my_app = init_app;
					my_cores_count = init_man_offers->off.num_of_cores;
					my_app.num_of_cores = my_cores_count;//+1;
					if (my_cores == NULL) {
						my_cores = (core_list *) malloc(sizeof(core_list));
						my_cores_tail = my_cores;
					} else {
						printf("My cores still not null!!\n");
						my_cores_tail->next = (core_list *) malloc(sizeof(core_list));
						my_cores_tail = my_cores_tail->next;
					}
					
					my_cores_tail->core_id = node_id;
					my_cores_tail->offered_to = -1;
					my_cores_tail->workload[0] = -1;
					my_cores_tail->workload[1] = -1;
					my_cores_tail->next = NULL;

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
					core_inter_tail[my_idag]->next = NULL;
					//I want myself to be first in my_cores list
					for(i=0; i<init_man_offers->off.num_of_cores; i++) {
						one_core = init_man_offers->off.offered_cores[i];
						core_inter_tail[my_idag]->data.app_cores[i+1] = one_core;

						if (one_core != node_id){
							my_cores_tail->next = (core_list *) malloc(sizeof(core_list));
							my_cores_tail = my_cores_tail->next;

							my_cores_tail->core_id = one_core;
							my_cores_tail->offered_to = -1;
							my_cores_tail->next = NULL;
						}
					}

					my_app_times[0].tm_sec = init_app_times[0].tm_sec;
					my_app_times[0].tm_min = init_app_times[0].tm_min;
					my_app_times[0].tm_hour = init_app_times[0].tm_hour;
					my_app_times[0].tm_usec = init_app_times[0].tm_usec;

					my_app_times[1].tm_sec = init_app_times[1].tm_sec;	
					my_app_times[1].tm_min = init_app_times[1].tm_min;
					my_app_times[1].tm_hour = init_app_times[1].tm_hour;
					my_app_times[1].tm_usec = init_app_times[1].tm_usec;

					init_app_times[0].tm_sec = 0;
					init_app_times[0].tm_min = 0;
					init_app_times[0].tm_hour = 0;
					init_app_times[0].tm_usec = 0;
					init_app_times[1].tm_sec = 0;
					init_app_times[1].tm_min = 0;
					init_app_times[1].tm_hour = 0;
					init_app_times[1].tm_usec = 0;
					
					scc_kill(idag_id_arr[0], SIG_INIT_APP, NULL);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,idag_id_arr[0]);

					//printf("I am new agent with id %d and app with A=%.2f, var=%.2f and %d my_cores_count = %d cores: \n"
					//	,node_id,my_app.A,my_app.var,my_app.num_of_cores,my_cores_count);
					my_Speedup = Speedup(my_app, my_app.num_of_cores);

					if (core_inter_head[my_idag]->next == NULL) {
						//kill(pid_num[my_idag], SIG_ADD_CORES_DDS);
						scc_kill(my_idag, SIG_ADD_CORES_DDS, core_inter_head[my_idag]);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,my_idag);
					} else {
						fprintf(log_file,"I didn't call add!! with interaction %d\n",core_inter_head[my_idag]->type);
					}

					cur_time = time(NULL);	
					cur_t = localtime(&cur_time);
					fprintf(log_file, "[%d:%d:%d]: Init ok!! my_cores_count = %d app_id=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,my_cores_count,my_app.id);
					fprintf(log_file, "my cores are:");
					for (tmp_cores_list=my_cores; tmp_cores_list!=NULL; tmp_cores_list=tmp_cores_list->next) fprintf(log_file, " %d", tmp_cores_list->core_id);
					fprintf(log_file, "\n");
				}

				tmp_offer_list = init_man_offers->next;
				while (tmp_offer_list != NULL){
					*tmp_offer_list->answer = 0;
					tmp_offer_list = tmp_offer_list->next;
				}

				while (init_man_offers != NULL) {
					if (core_inter_head[init_man_offers->sender] != NULL) {
						if (core_inter_head[init_man_offers->sender]->type == REP_AGENT_OFFER_PENDING){
							core_inter_head[init_man_offers->sender]->type = REP_AGENT_OFFER_SENT;
							scc_kill(init_man_offers->sender, SIG_REP_OFFERS, core_inter_head[init_man_offers->sender]);
							my_stats.msg_count++;
							my_stats.distance += distance(node_id,init_man_offers->sender);	
						} else if (core_inter_head[init_man_offers->sender]->type != REP_AGENT_OFFER_SENT) 
							printf("Problem A!\n");
					} else {
						fprintf(log_file,"Problem B = %d",init_man_offers->sender);
					}
					tmp_offer_list = init_man_offers;
					init_man_offers = init_man_offers->next;
					free(tmp_offer_list);
				}

				fprintf(log_file,"selfopt_time_rem = %ld app_state = %d state = %d pending_state = %d\n",selfopt_time_rem,app_state,state,pending_state);

				if (new_agent == node_id)
					state = AGENT_INIT_STATE;
				else if (app_state != APP_TERMINATED) {
					if (selfopt_time_rem != -1) { //a selfopt is pending !app_terminated
						selfopt_time_rem -= INIT_NODE_INTERVAL;
						if (selfopt_time_rem > 0){
							my_settimer(selfopt_time_rem);
							state = IDLE_AGENT;
						} else {
							selfopt_time_rem = -1;
							state = AGENT_SELF_OPT;
						}
					} else if (pending_state == IDLE_AGENT) {
						if (my_cores_count == 1) {
							fprintf(log_file,"I have to restart optimization process after init for app with id %d\n",init_app.id);
							fprintf(app_log_file,"I have to restart optimization process after init\n");
							selfopt_interval = LEAST_SELF_OPT_INTERVAL_MS;
							state = AGENT_SELF_OPT; 
						} else state = IDLE_AGENT;

						pending_state = NO_PENDING_STATE;
					} else if (pending_state == AGENT_SELF_OPT || pending_state == WORKING_NODE || pending_state == AGENT_ZOMBIE) {// || pending_state == AGENT_ENDING) {
						state = pending_state;//IDLE_AGENT;
						pending_state = NO_PENDING_STATE;
					} else state = IDLE_CORE;
				} else {  
					selfopt_time_rem = -1;
					state = AGENT_ENDING;
				}

				fprintf(log_file,"state = %d pending = %d\n",state,pending_state);
			}
			signals_enable();
			scc_signals_check(); 
		} else if (state == AGENT_INIT_STATE) {
			signals_disable();
			
			/* Manager failing Scenario */
			printf(KMAG "APP TO FAIL = %d\n" KNRM, manager_to_fail);
			if (paxos_state != NEW_AGENT && my_app.id == manager_to_fail){
			#if defined(BASIC_PAXOS) && defined(MANAGER)
			sev.sigev_notify = SIGEV_SIGNAL;
			sev.sigev_signo = SIG_CTIMER;
			sev.sigev_value.sival_ptr = &controller_timer;
			if (timer_create(CLOCK_REALTIME, &sev, &controller_timer) == -1)
				printf("timer_create error\n");
			else
				printf("Manager Timer created succesfully!\n");
			its.it_interval.tv_sec = 0;
			its.it_interval.tv_nsec = 0;
			its.it_value.tv_sec = 1;
			its.it_value.tv_nsec = 0;
			if (timer_settime(controller_timer, 0, &its, NULL) == -1)
				perror("controller_core.c : timer_settime error9");
			else
				printf(KMAG "%d : My timer will explode in %d seconds.\n" KNRM, node_id, 1);
			#endif
			}
			
			selfopt_interval = LEAST_SELF_OPT_INTERVAL_MS;
			nodes_ended_cnt = 0;
			app_state = RUNNING;
			max_cores_count = get_max_cores_count(my_app);
			active_working_cores = 0;
	
			/* Open Application Log File */
#ifdef PLAT_SCC
			strcpy(app_log_file_name, "/shared/herc/");
#else
			strcpy(app_log_file_name, "../");
#endif
			strcat(app_log_file_name,scen_directory);
			strcat(app_log_file_name, "/");
			strcat(app_log_file_name,scen_num);
			strcat(app_log_file_name,"/app_logs/");
			strcat(app_log_file_name, itoa(my_app.id));
			strcat(app_log_file_name, ".txt");
			
			printf("app_log_file_name %s\n",app_log_file_name);
			
			if (paxos_state == NEW_AGENT){
				if ((app_log_file = fopen(app_log_file_name, "a")) == NULL){
					printf("Cannot open input file with file path = %s ",app_log_file_name);
					perror("open app_log_file");
				}
			}else{
				if ((app_log_file = fopen(app_log_file_name, "w")) == NULL){
					printf("Cannot open input file with file path = %s ",app_log_file_name);
					perror("open app_log_file");
				}else{
					setbuf(app_log_file, NULL);
				}
			}
			fprintf(app_log_file, "Came into init_agent at [%d:%d:%d:%ld]\n",my_app_times[0].tm_hour,my_app_times[0].tm_min,my_app_times[0].tm_sec,my_app_times[0].tm_usec);
			fprintf(app_log_file, "New agent found at [%d:%d:%d:%ld]\n",my_app_times[1].tm_hour,my_app_times[1].tm_min,my_app_times[1].tm_sec,my_app_times[1].tm_usec);
			
			gettimeofday(&time_val, NULL);
			cur_t = localtime(&time_val.tv_sec);	
#ifndef ARTIFICIAL_APPS_SIM			
			fprintf(app_log_file, "[%d:%d:%d:%ld]: I am agent %d . Init ok!! my_cores_count = %d array_size = %d workld=%d\n",
				cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,time_val.tv_usec,node_id,my_cores_count,my_app.array_size,my_app.workld);
#else
			fprintf(app_log_file, "[%d:%d:%d:%ld]: I am agent %d . Init ok!! my_cores_count = %d var = %f workld=%d my_Speedup= %.2f\n",
                                cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,time_val.tv_usec,node_id,my_cores_count,my_app.var,my_app.workld,my_Speedup); 
			/* FIXME printf order is maintained for compatibility with python scrips */
#endif
			active_working_cores = my_cores_count - 1;
			if (my_cores_count > 1) {
#ifndef ARTIFICIAL_APPS_SIM
				row_reached = 0;
				per_core_rows = my_app.array_size / (my_cores_count-1);
				rows_left = my_app.array_size % (my_cores_count-1);
#else
				per_core_rows = (int) my_Speedup;
#endif
			}
						
			fprintf(app_log_file, "my cores are:");
			/*for (tmp_cores_list=my_cores; tmp_cores_list!=NULL; tmp_cores_list=tmp_cores_list->next) fprintf(app_log_file, " %d", tmp_cores_list->core_id);
			fprintf(app_log_file, "\n");
			fprintf(app_log_file,"speedup = %0.2f time_to_work = %d time_per_node = %d, time_left = %d, workld=%0.2f\n",my_Speedup,time_to_work,time_per_node,time_left,my_app.workld);*/

			tmp_cores_list = my_cores->next;
			while (tmp_cores_list != NULL) {
				one_core = tmp_cores_list->core_id;
				if (core_inter_head[one_core] == NULL){
					core_inter_head[one_core] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[one_core] = core_inter_head[one_core];
				} else {
					core_inter_tail[one_core]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[one_core] = core_inter_tail[one_core]->next;
				}

				core_inter_tail[one_core]->type = INIT_WORK_NODE;
#ifndef ARTIFICIAL_APPS_SIM
				core_inter_tail[one_core]->data.work_bounds[0] = row_reached; /* 27.6.2016 Added by dimos. If worker fails i have to know the workload given in order to reappoint */
				tmp_cores_list->workload[0] = row_reached;
				row_reached += per_core_rows;
				if (rows_left > 0) {
					row_reached++;
					rows_left--;
				}
				core_inter_tail[one_core]->data.work_bounds[1] = row_reached-1;
				tmp_cores_list->workload[1] = row_reached-1; /* 27.6.2016 Added by dimos. If worker fails i have to know the workload given in order to reappoint */
#else
				core_inter_tail[one_core]->data.work_bounds[0] = 0;
                                tmp_cores_list->workload[0] = 0;

				core_inter_tail[one_core]->data.work_bounds[1] = per_core_rows;
                                tmp_cores_list->workload[1] = per_core_rows;
#endif

				/* 28.6.2016 Scenario where worker fails */
				#ifdef WORKER
				if (worker_flag == 0){
					scc_kill(one_core, SIG_FAIL, NULL);
					worker_flag = 1;
				}
				#endif
				fprintf(app_log_file,"%d (%d, %d), ",one_core,core_inter_tail[one_core]->data.work_bounds[0],core_inter_tail[one_core]->data.work_bounds[1]);
				core_inter_tail[one_core]->next = NULL;

				if (core_inter_head[one_core]->next == NULL) {
					scc_kill(one_core, SIG_APPOINT_WORK, core_inter_head[one_core]);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,one_core);
				} else {
					fprintf(log_file,"I am doing smth else with my working node %d in init inter1 = %d inter2 = %d\n",
						one_core,core_inter_head[one_core]->type,core_inter_head[one_core]->next->type);
				}
				
				tmp_cores_list = tmp_cores_list->next;
			} 

			fprintf(app_log_file, "\n");

			if (my_cores_count == max_cores_count) {
				fprintf(app_log_file, "I have maximum cores count. I don't initiate selfopt process ini init_agent.\n");
				selfopt_time_rem = -1;
				if (pending_state == IDLE_INIT_MAN || pending_state == INIT_MANAGER || pending_state == INIT_MANAGER_SEND_OFFERS 
					|| pending_state == INIT_MAN_CHK_OFFERS || pending_state == WORKING_NODE) {
					state = pending_state;
					pending_state = IDLE_AGENT;
				} else state = IDLE_AGENT;
			} else if (my_cores_count > 1 && get_times(my_app, my_cores_count) <= INIT_NODE_INTERVAL) { 
				fprintf(app_log_file, "I have little working time left %d selfopt_interval=%d in init_agent.\n",
					get_times(my_app, my_cores_count),selfopt_interval);	
				if (pending_state == IDLE_INIT_MAN || pending_state == INIT_MANAGER || pending_state == INIT_MANAGER_SEND_OFFERS 
					|| pending_state == INIT_MAN_CHK_OFFERS || pending_state == WORKING_NODE) {
					state = pending_state;
					pending_state = IDLE_AGENT;
				} else state = IDLE_AGENT;
				selfopt_time_rem = -1;
			} else if (pending_state == INIT_MANAGER) {
				if (my_cores_count == 1)
					state = AGENT_SELF_OPT;
				else {
					state = INIT_MANAGER;
					pending_state = AGENT_SELF_OPT;
				}
			} else {
				if (pending_state == IDLE_INIT_MAN || pending_state == INIT_MANAGER_SEND_OFFERS || pending_state == INIT_MAN_CHK_OFFERS || pending_state == WORKING_NODE) {
					state = pending_state;
					pending_state = AGENT_SELF_OPT;
				} else state = AGENT_SELF_OPT;
			}

			gettimeofday(&time_val, NULL);
			cur_t = localtime(&time_val.tv_sec);	
			fprintf(app_log_file, "[%d:%d:%d:%ld] Agent init ok\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,time_val.tv_usec);

			signals_enable();
			scc_signals_check();
		} else if (state == AGENT_SELF_OPT) {
			signals_disable();
			
			cur_time = time(NULL);
			cur_t = localtime(&cur_time);
			fprintf(log_file, "[%d:%d:%d]: Initialising self opt %s!\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec, id2string(state));
			fprintf(app_log_file, "[%d:%d:%d]: Initialising self opt!\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);
			
			if (app_state != APP_TERMINATED) {
				selfopt_DDS_idags = 0;
				selfopt_DDS_replies = 0;
				
				if (selfopt_targets_head != NULL) {
					//printf("selfopt targets list is not NULL\n");
					while (selfopt_targets_head != NULL){
						tmp_target_list = selfopt_targets_head;
						selfopt_targets_head = selfopt_targets_head->next;
						free(tmp_target_list);
					}
				}
				
				if (core_inter_head[my_idag] == NULL){
					core_inter_head[my_idag] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[my_idag] = core_inter_head[my_idag];
				} else {
					core_inter_tail[my_idag]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[my_idag] = core_inter_tail[my_idag]->next;
				}

				#ifndef ADAM_SIM
				core_inter_tail[my_idag]->type = SELFOPT_IDAG_FIND_IDAGS_PENDING;
				core_inter_tail[my_idag]->data.reg.C = node_id;
				core_inter_tail[my_idag]->data.reg.r = Selfopt_Radius;
				core_inter_tail[my_idag]->next = NULL;

				if (core_inter_head[my_idag]->next == NULL)	{
					scc_kill(my_idag, SIG_IDAG_FIND_IDAGS, core_inter_head[my_idag]);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,my_idag);
				} else {
					fprintf(log_file,"common_core.c: Did not send sig_find_idags with inter1 = %s, inter2 = %s\n",inter2string(core_inter_head[my_idag]->type),inter2string(core_inter_head[my_idag]->next->type));
				}
				#else
				core_inter_tail[my_idag]->type = SELFOPT_IDAG_REQ_DDS_PENDING;
				core_inter_tail[my_idag]->data.reg.C = node_id;
				core_inter_tail[my_idag]->data.reg.r = Selfopt_Radius;
				core_inter_tail[my_idag]->next = NULL;
				selfopt_DDS_idags = 1; /* Necessary to override SIG_FIND_IDAGS */

				if (core_inter_head[my_idag]->next == NULL) {
					scc_kill(my_idag, SIG_REQ_DDS, core_inter_head[my_idag]);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,my_idag);
				} else {
					fprintf(log_file,"Adam common_core.c: Did not send sig_find_idags with inter1 = %s, inter2 = %s\n",inter2string(core_inter_head[my_idag]->type),inter2string(core_inter_head[my_idag]->next->type));
				}
				#endif				

	
				if (selfopt_interval <= Max_SelfOpt_Interval_MS) selfopt_interval = 2 * selfopt_interval;
				else {
					selfopt_interval = -1;
					/*fprintf(log_file,"I invalidate selfopt process here\n");
					fprintf(app_log_file,"I invalidate selfopt process here\n");*/
				}
				/*FIXME*/
				state = IDLE_AGENT_WAITING_OFF;
			} else state = AGENT_ENDING;
		
			signals_enable();
			scc_signals_check();
		} else if (state == AGENT_SELF_CHK_OFFERS) {
			signals_disable();
			
			cur_time = time(NULL);
			cur_t = localtime(&cur_time);
			fprintf(log_file, "[%d:%d:%d]: alarm went off for self opt selfopt_DDS_idags = %d selfopt_DDS_replies = %d app_state=%d pending_state = %d old_cores_cnt=%d\n",
				cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,selfopt_DDS_idags,selfopt_DDS_replies,app_state,pending_state,old_cores_cnt);
			fprintf(app_log_file, "[%d:%d:%d]: alarm went off for self opt selfopt_DDS_idags = %d selfopt_DDS_replies = %d app_state=%d pending_state = %d old_cores_cnt=%d\n",
				cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,selfopt_DDS_idags,selfopt_DDS_replies,app_state,pending_state,old_cores_cnt);

			//old_cores_cnt = my_cores_count;
			if (selfopt_man_offers == NULL) {
				fprintf(log_file,"I don't have self opt offers app_state=%d\n",app_state);
				fprintf(app_log_file,"I don't have self opt offers app_state=%d\n",app_state);
			} else {
				tmp_offer_list = selfopt_man_offers;
				while (tmp_offer_list != NULL){
					fprintf(log_file,"Offer by %d for %d cores\n",tmp_offer_list->sender,tmp_offer_list->off.num_of_cores);
					tmp_offer_list = tmp_offer_list->next;
				}
			
				if (app_state != APP_TERMINATED) {
					*selfopt_man_offers->answer = 1;
				
					if (core_inter_head[my_idag] == NULL) {
						core_inter_head[my_idag] = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[my_idag] = core_inter_head[my_idag];
					} else {
						core_inter_tail[my_idag]->next = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[my_idag] = core_inter_tail[my_idag]->next;
					}

					core_inter_tail[my_idag]->type = IDAG_ADD_CORES_DDS;
					core_inter_tail[my_idag]->data.app_cores = (int *)malloc((selfopt_man_offers->off.num_of_cores+1)*sizeof(int));
					core_inter_tail[my_idag]->data.app_cores[0] = selfopt_man_offers->off.num_of_cores;
					core_inter_tail[my_idag]->next = NULL;

					if (app_state != RESIZING) {
						old_Speedup = my_Speedup;
						old_cores_cnt = my_cores_count;
						app_state = RESIZING;
						my_cores_count += selfopt_man_offers->off.num_of_cores;
						my_app.num_of_cores = my_cores_count;
						my_Speedup = Speedup(my_app, my_cores_count);
	
					} else {
						fprintf(app_log_file,"Allready resizing\n");
						my_cores_count += selfopt_man_offers->off.num_of_cores;
						my_app.num_of_cores = my_cores_count;
						my_Speedup = Speedup(my_app, my_cores_count);
					}

					for (i=0; i<selfopt_man_offers->off.num_of_cores; i++){
						my_cores_tail->next = (core_list *) malloc(sizeof(core_list));
						my_cores_tail = my_cores_tail->next;

						my_cores_tail->core_id = selfopt_man_offers->off.offered_cores[i];
						my_cores_tail->offered_to = -1;
						my_cores_tail->next = NULL;
						core_inter_tail[my_idag]->data.app_cores[i+1] = selfopt_man_offers->off.offered_cores[i];

						one_core = my_cores_tail->core_id;
						if (cur_agent.my_agent == one_core) {
							fprintf(log_file,"I cancel my agent here\n");
							
							base_offset = -1;  
							cur_agent.my_agent = -1;
							cur_agent.array_size = -1;
							cur_agent.work_bounds[0] = 0;
							cur_agent.work_bounds[1] = 0;
						}

						if (core_inter_head[one_core] == NULL){
							core_inter_head[one_core] = (inter_list *) malloc(sizeof(inter_list));
							core_inter_tail[one_core] = core_inter_head[one_core];
						} else {
							fprintf(log_file,"I am doing sth with my new core %d interaction is %d\n",one_core,core_inter_head[one_core]->type);
							core_inter_tail[one_core]->next = (inter_list *) malloc(sizeof(inter_list));
							core_inter_tail[one_core] = core_inter_tail[one_core]->next;
						}

						core_inter_tail[one_core]->type = INIT_WORK_NODE_PENDING;
						core_inter_tail[one_core]->next = NULL;
					}
				} else *selfopt_man_offers->answer = 0;

				tmp_offer_list = selfopt_man_offers->next;
				while (tmp_offer_list != NULL){
					*tmp_offer_list->answer = 0;
					tmp_offer_list = tmp_offer_list->next;
				}

				if (app_state != APP_TERMINATED) {
					//my idag may have offered smth
					if (core_inter_head[my_idag]->next == NULL) {
						scc_kill(my_idag, SIG_ADD_CORES_DDS, core_inter_head[my_idag]);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,my_idag);
					} else {
						fprintf(log_file,"I didn't call add inside selfopt with interaction %d\n",core_inter_head[my_idag]->type);
					}

					if (old_cores_cnt == 1) {  
						row_reached = 0;
#ifndef ARTIFICIAL_APPS_SIM
						per_core_rows = my_app.array_size / (my_cores_count - 1);
						rows_left = my_app.array_size % (my_cores_count - 1);
#else
						per_core_rows = (int) my_Speedup; /* Cutting off floating points to have a more constrained Speedup */
#endif						
						active_working_cores = my_cores_count - 1;

						fprintf(app_log_file, "In one old core my cores are:");

						tmp_cores_list = my_cores->next;
						while (tmp_cores_list != NULL){
							//printf("I am inside core listing node=%d\n",node_id);
							one_core = tmp_cores_list->core_id;

							for (tmp_inter_list = core_inter_head[one_core]; tmp_inter_list != NULL; tmp_inter_list = tmp_inter_list->next)
								if (tmp_inter_list->type == INIT_WORK_NODE_PENDING) break;

							if (tmp_inter_list != NULL) {
								tmp_inter_list->type = INIT_WORK_NODE;
#ifndef ARTIFICIAL_APPS_SIM
								tmp_inter_list->data.work_bounds[0] = row_reached;
								tmp_cores_list->workload[0] = row_reached; /* 27.6.2016 dimos. If worker fails i have to know the workload given in order to reappoint */
								row_reached += per_core_rows;
								if (rows_left > 0) {
									row_reached++;
									rows_left--;
								}
								tmp_inter_list->data.work_bounds[1] = row_reached-1;
								tmp_cores_list->workload[1] = row_reached-1; /* 27.6.2016 dimos. If worker fails i have to know the workload given in order to reappoint */
#else
								tmp_inter_list->data.work_bounds[0] = 0;
								tmp_cores_list->workload[0] = 0;                                
                                
								tmp_inter_list->data.work_bounds[1] = per_core_rows;
								tmp_cores_list->workload[1] = per_core_rows;
#endif

								fprintf(app_log_file,"%d (%d, %d), ",one_core,tmp_inter_list->data.work_bounds[0],
									tmp_inter_list->data.work_bounds[1]);
							} else {
								printf("i am %d kai sto common_node ta hpiame one core = %d\n",node_id,one_core);
								fprintf(log_file,"i am %d kai sto common_node ta hpiame one core = %d\n",node_id,one_core);
							}
	
							if (core_inter_head[one_core]->next == NULL) {
								//kill(pid_num[one_core], SIG_APPOINT_WORK);
								scc_kill(one_core, SIG_APPOINT_WORK, core_inter_head[one_core]);
								my_stats.msg_count++;
								my_stats.distance += distance(node_id,one_core);
							} else {
								fprintf(log_file,"I am doing smth else with my working node? interaction=%d interaction2=%d\n",
									core_inter_head[one_core]->type,core_inter_head[one_core]->next->type);
							}

							tmp_cores_list = tmp_cores_list->next;
						}

						fprintf(app_log_file, "\n");
						app_state = RUNNING;
					}
				}

				while (selfopt_man_offers != NULL) {
					if (core_inter_head[selfopt_man_offers->sender]->type == REP_AGENT_OFFER_PENDING){				
						core_inter_head[selfopt_man_offers->sender]->type = REP_AGENT_OFFER_SENT;
						//kill(pid_num[selfopt_man_offers->sender], SIG_REP_OFFERS);
						scc_kill(selfopt_man_offers->sender, SIG_REP_OFFERS, core_inter_head[selfopt_man_offers->sender]);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,selfopt_man_offers->sender);	
					} else if (core_inter_head[selfopt_man_offers->sender]->type != REP_AGENT_OFFER_SENT) printf("We have zxcvzxc problem!\n");
					tmp_offer_list = selfopt_man_offers;
					selfopt_man_offers = selfopt_man_offers->next;
					free(tmp_offer_list);
				}

				cur_time = time(NULL);	
				cur_t = localtime(&cur_time);
				fprintf(log_file, "[%d:%d:%d]: Self opt ok. my_cores_count = %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,my_cores_count);
				fprintf(log_file, "my cores are:");
				fprintf(app_log_file, "[%d:%d:%d]: Self opt ok. my_cores_count = %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,my_cores_count);
				fprintf(app_log_file, "my cores are:");

				for (tmp_cores_list=my_cores; tmp_cores_list!=NULL; tmp_cores_list=tmp_cores_list->next) {
					fprintf(log_file, " %d", tmp_cores_list->core_id);
					fprintf(app_log_file, " %d", tmp_cores_list->core_id);					
				}

				fprintf(log_file, "\n");
				fprintf(app_log_file, "\n");
			}

			if (app_state == APP_TERMINATED) {
				state = AGENT_ENDING;
				if (pending_state == IDLE_AGENT)
					pending_state = NO_PENDING_STATE;
				else {
					fprintf(log_file,"In init app terminated and pending state == %d\n",pending_state);
				}	
			} else {
				fprintf(log_file,"in chk_offers and pending_state=%d selfopt_interval = %d\n",pending_state,selfopt_interval);
					
				if (selfopt_interval > 0) { // && !app_terminated
					selfopt_time_rem = selfopt_interval;

					if (my_cores_count == max_cores_count) {
						fprintf(app_log_file, "I have maximum cores count. I don't initiate selfopt process in selfopt.\n");
						selfopt_time_rem = -1;
					} else if (my_cores_count > 1 && get_times(my_app, my_cores_count) <= selfopt_interval) { 
						fprintf(app_log_file, "I have little working time left %d selfopt_interval=%d in selfopt.\n",
							get_times(my_app, my_cores_count),selfopt_interval);
						selfopt_time_rem = -1;
					} else if (my_cores_count == 1) {
						fprintf(app_log_file,"I have one core and i restart optimization process with id %d\n",my_app.id);

						selfopt_interval = LEAST_SELF_OPT_INTERVAL_MS;//200;
						selfopt_time_rem = selfopt_interval;
						//my_settimer(LEAST_SELF_OPT_INTERVAL_MS);
					} else my_settimer(selfopt_time_rem);
				} else if (my_cores_count == 1) {
					//printf("I am %d and i have to rewind my selfopt_areas_file for app with id %d\n",node_id,init_app.id);
					fprintf(log_file,"I restart optimization process for app with id %d\n",my_app.id);
					fprintf(app_log_file,"I restart optimization process for app with id %d\n",my_app.id);
					
					selfopt_interval = LEAST_SELF_OPT_INTERVAL_MS;//200;
					selfopt_time_rem = selfopt_interval;
					//my_settimer(LEAST_SELF_OPT_INTERVAL_MS);
				} else selfopt_time_rem = -1;
			  
			  	if (pending_state == INIT_MANAGER && my_cores_count > 1) {
					state = INIT_MANAGER;
					pending_state = IDLE_AGENT;
				} else {
					if (selfopt_time_rem != -1) {
					  my_settimer(selfopt_time_rem);
					}
					state = IDLE_AGENT;
				}
			}

			signals_enable();
			scc_signals_check();
		} else if (state == WORKING_NODE) {
			
			if (executed_app == MATRIX_MUL) {
				/* WORKLOAD EXECUTION */
				fprintf(log_file, "I enter working state -- executing MATRIX MUL\n");
				signals_disable();

				execute_workload(cur_agent.work_bounds[0], cur_agent.work_bounds[1]);
				
				cur_time = time(NULL);
				cur_t = localtime(&cur_time);
				fprintf(log_file, "[%d:%d:%d]: finished work agent=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,cur_agent.my_agent);
				
				scc_kill(cur_agent.my_agent, SIG_FINISH, NULL);
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,cur_agent.my_agent);

				fprintf(log_file, "finished work pending_agent=%d pending_state=%d\n",pending_agent.my_agent,pending_state);
			} else if (executed_app == SVM) {
				fprintf(log_file,"I enter working state -- executing SVM\n");
				execute_workload(cur_agent.work_bounds[0], cur_agent.work_bounds[1]);
				/*TODO I HAVE TO SEND BACK MY svm_local_sum TO BE TOTALLY CORRECT */
				scc_kill(cur_agent.my_agent, SIG_FINISH, NULL);
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,cur_agent.my_agent);

				fprintf(log_file, "finished work pending_agent=%d pending_state=%d\n",pending_agent.my_agent,pending_state);
			} else if (executed_app == FFT) {
				//printf("I enter working state -- executing FFT\n");
				fprintf(log_file,"I enter working state -- executing FFT\n");
				execute_workload(cur_agent.work_bounds[0], cur_agent.work_bounds[1]);
				
				/*TODO I HAVE TO SEND BACK result TO BE TOTALLY CORRECT */
				scc_kill(cur_agent.my_agent, SIG_FINISH, NULL);
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,cur_agent.my_agent);

				fprintf(log_file, "finished work pending_agent=%d pending_state=%d\n",pending_agent.my_agent,pending_state);
			} else if (executed_app == ARTIFICIAL) {
				fprintf(log_file,"I enter working state -- executing ARTIFICIAL\n");
				execute_workload(cur_agent.work_bounds[0], cur_agent.work_bounds[1]);

				/*TODO I HAVE TO SEND BACK result TO BE TOTALLY CORRECT */
				scc_kill(cur_agent.my_agent, SIG_FINISH, NULL);
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,cur_agent.my_agent);

				fprintf(log_file, "finished work pending_agent=%d pending_state=%d\n",pending_agent.my_agent,pending_state);
			}

			if (pending_agent.my_agent == -1) { //No agent change
				if (pending_state == WORKING_NODE) pending_state = NO_PENDING_STATE; //just finished on return
				else if (pending_state == IDLE_AGENT) {
					pending_state = NO_PENDING_STATE; 
					state = IDLE_AGENT;
					fprintf(log_file, "Finished working. Pending state is IDLE_AGENT\n");
				} else if (pending_state == AGENT_SELF_OPT) {
					pending_state = NO_PENDING_STATE; 
					state = AGENT_SELF_OPT;
					fprintf(log_file, "Finished working. Pending state is AGENT_SELF_OPT\n");
				} else if (pending_state == AGENT_INIT_STATE || pending_state == AGENT_INIT_APP_INIT || pending_state == AGENT_INIT_CHK_OFFERS || pending_state == AGENT_INIT_IDLE_INIT) {
					fprintf(log_file, "Finished working. Pending state is AGENT_INIT_smth = %d\n",pending_state);

					if (pending_state == AGENT_INIT_APP_INIT)
						pending_state = INIT_MANAGER;
					else if (pending_state == AGENT_INIT_CHK_OFFERS)
						pending_state = INIT_MAN_CHK_OFFERS;
					else if (pending_state == AGENT_INIT_IDLE_INIT)
						pending_state = IDLE_INIT_MAN;
					else		
						pending_state = NO_PENDING_STATE;

					state = AGENT_INIT_STATE;
				} else if (pending_state == IDLE_INIT_AGENT_SELFOPT) {
					state = IDLE_INIT_MAN;
					pending_state = AGENT_SELF_OPT;
				} else if (pending_state == IDLE_INIT_IDLE_AGENT) {
					state = IDLE_INIT_MAN;
					pending_state = IDLE_AGENT;
				} else if (pending_state == INIT_CHK_OFFERS_SELFOPT) {
					state = INIT_MAN_CHK_OFFERS;
					pending_state = AGENT_SELF_OPT;
				} else if (pending_state == INIT_CHK_OFFERS_IDLE_AGENT) {
					state = INIT_MAN_CHK_OFFERS;
					pending_state = IDLE_AGENT;
				} else if (pending_state == INIT_MAN_CHK_OFFERS || pending_state == IDLE_INIT_MAN) {
					state = pending_state;
					pending_state = NO_PENDING_STATE;
				} else if (state == WORKING_NODE) state = IDLE_CORE;
			} else {
				fprintf(log_file, "In pre_change pending_agent = %d\n",pending_agent.my_agent);
				
				base_offset = -1;  
				cur_agent = pending_agent;
				pending_agent.my_agent = -1;
				pending_agent.array_size = -1;
				pending_agent.work_bounds[0] = 0;
				pending_agent.work_bounds[1] = 0;
			}

			signals_enable();
			scc_signals_check();
		} else if (state == AGENT_ZOMBIE) {
			scc_pause();
			scc_signals_check();	
		} else if (state == AGENT_ENDING) {
			signals_disable();

			cur_time = time(NULL);	
			cur_t = localtime(&cur_time);
			fprintf(log_file, "[%d:%d:%d]: I entered agent_ending pending_state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,id2string(pending_state));
			fprintf(app_log_file, "[%d:%d:%d]: I entered agent_ending pending_state = %s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec, id2string(pending_state));
			fflush(app_log_file);
			
			for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next)
				fprintf(app_log_file,"Core %d is offered to %d\n",tmp_cores_list->core_id,tmp_cores_list->offered_to);
			
			fflush(app_log_file);


			for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next)
				if (tmp_cores_list->offered_to != -1) break;

			if (tmp_cores_list != NULL) {
				
				if (pending_state == INIT_MANAGER || pending_state == INIT_MAN_CHK_OFFERS || pending_state == INIT_MANAGER_SEND_OFFERS) {
					state = pending_state;
					pending_state = NO_PENDING_STATE;
				} else if (pending_state == NO_PENDING_STATE || pending_state == IDLE_INIT_MAN) {
					state = AGENT_ZOMBIE;
				} else if (pending_state == IDLE_AGENT || pending_state == IDLE_CORE) {
					pending_state = NO_PENDING_STATE;
					state = AGENT_ZOMBIE;
				} else {
					fprintf(log_file,"I am in zombie agent_ending and pending_state = %s\n",id2string(pending_state));
					state = AGENT_ZOMBIE;
				}

				for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next) {
					one_core = tmp_cores_list->core_id;
					
					tmp_inter_prev = NULL;
					tmp_inter_list = core_inter_head[one_core];
					//for (tmp_inter_list = core_inter_head[one_core]; tmp_inter_list != NULL; tmp_inter_list=tmp_inter_list->next)
					while (tmp_inter_list != NULL)
						if (tmp_inter_list->type == INIT_WORK_NODE || tmp_inter_list->type == APPOINT_WORK_NODE || tmp_inter_list->type == INIT_WORK_NODE_PENDING 
							|| tmp_inter_list->type == APPOINT_WORK_NODE_PENDING) {							
							
							fprintf(log_file, "Removing in zombie one node of %d with inter = %d\n",one_core,tmp_inter_list->type);

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
				}
			} else {
				if (core_inter_head[my_idag] == NULL){
					core_inter_head[my_idag] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[my_idag] = core_inter_head[my_idag];
				} else {
					core_inter_tail[my_idag]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[my_idag] = core_inter_tail[my_idag]->next;
				}
	
				core_inter_tail[my_idag]->type = REMOVE_APP;
				core_inter_tail[my_idag]->data.app_cores = (int *)malloc((my_cores_count+1)*sizeof(int));
				core_inter_tail[my_idag]->data.app_cores[0] = my_cores_count;
				core_inter_tail[my_idag]->next = NULL;

				i=1;
				core_inter_tail[my_idag]->data.app_cores[i++] = my_cores->core_id;
				tmp_cores_list = my_cores;
				my_cores=my_cores->next;
				free(tmp_cores_list);
				
				while (my_cores != NULL) {
					tmp_cores_list = my_cores;
					one_core = tmp_cores_list->core_id;
					core_inter_tail[my_idag]->data.app_cores[i++] = one_core;

					tmp_inter_prev = NULL;
					tmp_inter_list = core_inter_head[one_core];
					
					while (tmp_inter_list != NULL)
						if (tmp_inter_list->type == INIT_WORK_NODE || tmp_inter_list->type == APPOINT_WORK_NODE || tmp_inter_list->type == INIT_WORK_NODE_PENDING 
							|| tmp_inter_list->type == APPOINT_WORK_NODE_PENDING) {
							
							fprintf(log_file, "Removing one node of %d with inter = %d\n",one_core,tmp_inter_list->type);

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
	
					if (core_inter_head[one_core] == NULL) {
						scc_kill(one_core, SIG_FINISH, core_inter_head[one_core]);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,one_core);
					} else {
						fprintf(log_file,"I still still have smth to do with my work node %d before finish which is %d\n",one_core,core_inter_head[one_core]->type);
					}
	
					my_cores=my_cores->next;
					free(tmp_cores_list);
				}
					
				if (core_inter_head[my_idag]->next == NULL) {
					scc_kill(my_idag, SIG_FINISH, core_inter_head[my_idag]);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,my_idag);
				} else {
					fprintf(log_file,"I am doing smth else with my idag app_remove inter1=%d inter2=%d\n",core_inter_head[my_idag]->type,core_inter_head[my_idag]->next->type);
				}

				my_app.num_of_cores = -1;
				selfopt_time_rem = -1;
				app_state = NO_APP;
				
				if (cur_agent.my_agent != -1) {
					fprintf(log_file,"I abandon my old agent %d in agent_finish\n",cur_agent.my_agent);
		
					base_offset = -1;
					cur_agent.my_agent = -1;
					cur_agent.array_size = -1;
					cur_agent.work_bounds[0] = 0;
					cur_agent.work_bounds[1] = 0;
				}
				
				gettimeofday(&time_val, NULL);
				cur_t = localtime(&time_val.tv_sec);				
				fprintf(app_log_file, "[%d:%d:%d:%ld]: App ended pending_state=%s\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,time_val.tv_usec,id2string(pending_state));
				fclose(app_log_file);			
				app_log_file = NULL; /* Added on 5.5.2017 */

				if (pending_state == INIT_MANAGER || pending_state == IDLE_INIT_MAN || pending_state == INIT_MAN_CHK_OFFERS || pending_state == INIT_MANAGER_SEND_OFFERS) {
					state = pending_state;
					pending_state = NO_PENDING_STATE;
				} else if (pending_state == NO_PENDING_STATE) state = IDLE_CORE;
				else if (pending_state == IDLE_AGENT || pending_state == IDLE_CORE) {
					pending_state = NO_PENDING_STATE;
					state = IDLE_CORE;
				} else {
					fprintf(log_file,"I am in agent_ending and pending_state = %d\n",pending_state);
					state = IDLE_CORE;
				}
			}

		fprintf(log_file,"I arrive here my_idag = %d state = %d idag_id_arr[0]=%d\n",my_idag,state,idag_id_arr[0]);	
		
		/* masouros: changed idag_id_arr[0] with 0 because it was -1 and crashed*/
		if (my_idag != 0 && state != AGENT_ZOMBIE) {
			/*
			if (core_inter_head[0] == NULL) 
				scc_kill(0, SIG_APP_TERMINATED, core_inter_head[0]);
			*/
			if (core_inter_head[0] != NULL) {
				fprintf(log_file,"I sent SIG_APP_TERMINATED to 0 with interaction = %d\n",core_inter_head[0]->type);
			}
 
			scc_kill(0, SIG_APP_TERMINATED, NULL);
		} else {
			fprintf(log_file,"I come here in agent_ending but state = %d\n",state);
		}

		signals_enable();
		scc_signals_check();		
	} else {
		printf("common_core.c : Unknown state node_id = %d state = %d\n",node_id,state);	
		state = IDLE_CORE;
	}

	#ifdef PLAT_SCC	
	RCCE_flag_free(&flag_data_written);
	RCCE_free((t_vcharp) sig_array);
	RCCE_free((t_vcharp) data_array);
	#endif
	/*FIXME frees for LINUX */

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended well\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);
	fclose(log_file);
	exit(0);
}

void my_settimer(int msec) {
	int sec;

	sec = msec / 1000;
	msec = msec % 1000;
	
	its.it_value.tv_sec = sec;
	its.it_value.tv_nsec = msec * MS;
	if (timer_settime(timerid, 0, &its, NULL) == -1) 
		perror("timer_settime error\n");
}

int my_gettimer(void) { //return in ms
	struct itimerspec chk_timer;
	int msec=0;
	
	if (timer_gettime(timerid, &chk_timer) == -1) 
		perror("timer_gettime error\n");

	msec = (chk_timer.it_value.tv_sec * 1000) + (chk_timer.it_value.tv_nsec / MS);
	if ((chk_timer.it_value.tv_nsec % MS) >= 500000) //rounding
		msec++;
  
	return msec;
}

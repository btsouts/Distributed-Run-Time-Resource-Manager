#include "common_node.h"

//extern int *pid_num;
extern int num_idags, node_id ,my_idag;//, fd_log;
extern FILE *log_file;
extern core_states state;
core_states pending_state=NO_PENDING_STATE; 	
extern inter_list **core_inter_head,**core_inter_tail;
extern app my_app;
extern app init_app;
extern app far_req_app;
extern metrics my_stats;
extern int far_req_or_sender;
extern int *idag_mask, *idag_id_arr;
extern int *Cl_x_max_arr, *Cl_y_max_arr; 
extern int DDS_count, my_cores_count;
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
extern int nodes_ended_cnt;//, app_terminated;
int my_x, my_y, init_areas_num;
int init_DDS_replies, selfopt_DDS_replies, init_DDS_idags, selfopt_DDS_idags, selfopt_interval=200, init_idags_areas_replies, max_cores_count;
target_list *init_targets_head=NULL, *init_targets_tail;
target_list *selfopt_targets_head=NULL, *selfopt_targets_tail;
FILE *app_log_file;
application_states app_state;
my_time_stamp init_app_times[2], my_app_times[2];

extern int *sig_array, *data_array, NUES;
extern RCCE_FLAG flag_signals_enabled,flag_data_written;

void common_node_actions(char scen_num[4]){
	offer_list *tmp_offer_list;
	int one_core, i, old_cores_cnt;
	core_list *tmp_cores_list;
	target_list *tmp_target_list;
	inter_list *tmp_inter_list, *tmp_inter_prev;	
	FILE *init_areas_file;//, *selfopt_areas_file;
	char init_areas_file_name[64], app_log_file_name[64];//selfopt_areas_file_name[32], 
	int C,r,init_areas_cnt, init_areas_sent;//area_cnt,selfopt_areas=0, 
	int time_per_node, time_left, time_to_work, init_core_found=0;
	int new_agent, min_dist=-1, tmp_int, j;
	//int new_x, new_y;//, selfopt_areas_sent;
	int selfopt_r;

	DDS_count=0; 
	my_cores_count=0;
	my_stats.msg_count=0;
	my_stats.message_size=0;
	my_stats.distance=0;
	my_stats.app_turnaround=0;
	my_stats.comp_effort=0;
	my_stats.cores_utilized=0;
	my_stats.times_accessed=0;
	init_app_times[0].tm_sec = 0;
  init_app_times[0].tm_min = 0;
  init_app_times[0].tm_hour = 0;
	init_app_times[1].tm_sec = 0;
  init_app_times[1].tm_min = 0;
  init_app_times[1].tm_hour = 0;
	my_app_times[0].tm_sec = 0;
  my_app_times[0].tm_min = 0;
  my_app_times[0].tm_hour = 0;
	my_app_times[1].tm_sec = 0;
  my_app_times[1].tm_min = 0;
  my_app_times[1].tm_hour = 0;	

	//printf("I Am %d\n",node_id);
	DDS=NULL;
	my_cores=NULL;	
	selfopt_r = (int) (1.5 * (X_max / num_idags_x));
	
	install_signal_handlers();
	//sig_SEGV_enable();
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIG_TIMER;
	sev.sigev_value.sival_ptr = &timerid;
	if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1) printf("timer_create error\n");
	its.it_value.tv_sec = 0;
	its.it_interval.tv_sec = 0;//its.it_value.tv_sec;
	its.it_interval.tv_nsec = 0;
	log_file = create_log_file(node_id,scen_num);

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: Initialisation complete\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);
	fflush(log_file);
	
	RCCE_barrier(&RCCE_COMM_WORLD);

	my_x = node_id % X_max;
	my_y = node_id / X_max;
	state = IDLE_CORE;

	RCCE_flag_write(&flag_signals_enabled, RCCE_FLAG_SET, node_id);

	my_idag = -1;
	int dummy=0;
	while (my_idag==-1) {//pause();
		for (i=0; i<1000; i++)
			for(j=0; j<1000; j++)
				dummy++;

		scc_signals_check();
	}

	while (state != TERMINATED)
		if (state == IDLE_CORE) {	
			pending_state = NO_PENDING_STATE;		
			//pause();
			dummy=0;
			for (i=0; i<1000; i++)
				for(j=0; j<1000; j++)
					dummy++;

			scc_signals_check();
		} else if (state == IDLE_INIT_MAN) {// || state == IDLE_FAR_MAN || state == IDLE_INIT_MAN_SELFOPT_PENDING || state == IDLE_INIT_MAN_WORK_PENDING) {
			//pause();
			dummy=0;
			for (i=0; i<1000; i++)
				for(j=0; j<1000; j++)
					dummy++;

			scc_signals_check();
		} else if (state == IDLE_AGENT) {
			if (app_state != APP_TERMINATED) {
				//pause();//!app_terminated
				dummy=0;
				for (i=0; i<1000; i++)
					for(j=0; j<1000; j++)
						dummy++;

				scc_signals_check();
			} else {
				if (timer_gettime(timerid, &chk_timer) == -1) printf("timer_gettime error 2\n");
				else selfopt_time_rem = chk_timer.it_value.tv_nsec;
				
				if (selfopt_time_rem > 0) {
					its.it_value.tv_nsec = 0;
					if (timer_settime(timerid, 0, &its, NULL) == -1) printf("timer_settime error42\n");
				}
				
				state = AGENT_ENDING; 
			}
		} else if (state == AGENT_REWIND_FILE) {
			printf("I am %d and i have to rewind my selfopt_areas_file for app with id %d\n",node_id,init_app.id);
			fprintf(log_file,"I have to rewind my selfopt_areas_file for app with id %d\n",init_app.id);
			
			selfopt_interval = 200;
			its.it_value.tv_nsec = selfopt_interval * MS;
			selfopt_time_rem = selfopt_interval;

			if (timer_settime(timerid, 0, &its, NULL) == -1) perror("timer_settime error823\n");
			state = IDLE_AGENT;
		} else if (state == IDLE_AGENT_WAITING_OFF) {
			//pause();
			if (app_state != APP_TERMINATED) {
				//pause();//!app_terminated
				dummy=0;
				for (i=0; i<1000; i++)
					for(j=0; j<1000; j++)
						dummy++;

				scc_signals_check();
			} else {
				signals_disable();
				if (timer_gettime(timerid, &chk_timer) == -1) printf("timer_gettime error 2\n");
				else selfopt_time_rem = chk_timer.it_value.tv_nsec;
				
				//den stamataw edw thn diadikasia tou selfopt gia na mhn meinoun oi mexri twra prosfores kai oxi mono anapanthtes
				if (selfopt_time_rem == 0) state = AGENT_SELF_CHK_OFFERS;
				signals_enable();
			}
		} else if (state == INIT_MANAGER) {
			signals_disable();
			printf("Initialising app node_id = %d\n",node_id);
			cur_time = time(NULL);	
			cur_t = localtime(&cur_time);
			fprintf(log_file, "[%d:%d:%d]: Initialising app\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);
			fflush(log_file);

			strcpy(init_areas_file_name,"/shared/herc/scenaria/");
			strcat(init_areas_file_name,scen_num);
			strcat(init_areas_file_name,"/");			
			strcat(init_areas_file_name, itoa(init_app.id));
			strcat(init_areas_file_name, ".txt");
			//printf("area file path = %s\n",areas_file_name);

			if ((init_areas_file = fopen(init_areas_file_name, "r")) == NULL){
				printf("Cannot open input file with file path = %s ",init_areas_file_name);		
				perror("open init_areas_file_name");
			}
	
			init_areas_sent = 0;
			fscanf(init_areas_file,"%d",&init_areas_cnt);
			state = INIT_MANAGER_SEND_OFFERS;
			
			signals_enable();
			scc_signals_check();
		} else if (state == INIT_MANAGER_SEND_OFFERS) {// || state == INIT_MANAGER_SEND_OFFERS_SELFOPT_PENDING || state == INIT_MANAGER_SEND_OFFERS_WORK_PENDING) {
			signals_disable();
			init_idags_areas_replies=0;
			init_DDS_idags = 0;
			init_DDS_replies = 0;
			init_areas_num = 1;

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
			
			fprintf(log_file, "near init area (%d,%d)\n",node_id,selfopt_r);
			fflush(log_file);
			if (core_inter_head[my_idag] == NULL){
				core_inter_head[my_idag] = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[my_idag] = core_inter_head[my_idag];
			} else {
				core_inter_tail[my_idag]->next = (inter_list *) malloc(sizeof(inter_list));
				core_inter_tail[my_idag] = core_inter_tail[my_idag]->next;
			}

			core_inter_tail[my_idag]->type = IDAG_FIND_IDAGS_PENDING;
			core_inter_tail[my_idag]->data.reg.C = node_id;
			core_inter_tail[my_idag]->data.reg.r = selfopt_r;
			core_inter_tail[my_idag]->next = NULL;

			if (core_inter_head[my_idag]->next == NULL) {
				//kill(pid_num[my_idag],SIG_IDAG_FIND_IDAGS);
				scc_kill(my_idag,SIG_IDAG_FIND_IDAGS);
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,my_idag);
			}

			for (i=0; i<init_areas_cnt && i<INIT_FAR_AREAS_NUM; i++) {
				fscanf(init_areas_file,"%d",&C);
				fscanf(init_areas_file,"%d",&r);
		
				fprintf(log_file, "init area (%d,%d)\n",C,r);
				fflush(log_file);
				printf("I am %d and i am sending a far offer in region (%d,%d)\n",node_id,C,r);
				one_core = C;
				if (core_inter_head[one_core] == NULL){
					core_inter_head[one_core] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[one_core] = core_inter_head[one_core];
				} else {
					core_inter_tail[one_core]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[one_core] = core_inter_tail[one_core]->next;
				}

				core_inter_tail[one_core]->type = FAR_INIT_REQ;
				core_inter_tail[one_core]->data.reg.C = C;
				core_inter_tail[one_core]->data.reg.r = r;
				core_inter_tail[one_core]->next = NULL;

				if (core_inter_head[one_core]->next == NULL) {
					//kill(pid_num[one_core],SIG_INIT_FAR_REQ);
					scc_kill(one_core,SIG_INIT_FAR_REQ);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,one_core);
				}

				//init_areas_num++;
			}

			init_areas_sent += i;
			//fclose(areas);
			if (selfopt_time_rem != -1){ 
				if (timer_gettime(timerid, &chk_timer) == -1) printf("timer_gettime error\n");
				else selfopt_time_rem = chk_timer.it_value.tv_nsec;

				if (selfopt_time_rem > 0) {
					its.it_value.tv_nsec = 0;//750000000;// * MS;
					if (timer_settime(timerid, 0, &its, NULL) == -1) printf("timer_settime error4123\n");
				}
			}

			state = IDLE_INIT_MAN;			
			signals_enable();		
			scc_signals_check();
		} else if (state == INIT_MAN_CHK_OFFERS) {// || state == INIT_MAN_CHK_OFFERS_SELFOPT_PENDING || state == INIT_MAN_CHK_OFFERS_WORK_PENDING){ 
			signals_disable();			
			printf("init check alarm went off node_id=%d init_DDS_idags = %d init_DDS_replies = %d state=%d!\n",node_id,init_DDS_idags,init_DDS_replies,state);
			cur_time = time(NULL);	
			cur_t = localtime(&cur_time);
			fprintf(log_file, "[%d:%d:%d]: init check alarm went off init_DDS_idags = %d init_DDS_replies = %d state=%d\n",
				cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,init_DDS_idags,init_DDS_replies,state);
			fflush(log_file);
			init_core_found = 0;			

			if (init_man_offers == NULL) {
				printf("I don't have offers\n");
				fprintf(log_file, "I don't have init man offers\n");
				fflush(log_file);

				printf("I am %d and didn't manage to find my core in this round of search for app with id %d\n",node_id,init_app.id);
				//state = IDLE_CORE;
				if (init_areas_sent == init_areas_cnt){
					printf("I am %d and i have to rewind my init_areas_file for app with id %d\n",node_id,init_app.id);
					rewind(init_areas_file);
					init_areas_sent = 0;
					fscanf(init_areas_file,"%d",&init_areas_cnt);
				}
				state = INIT_MANAGER_SEND_OFFERS;	
			}	else {
				tmp_offer_list = init_man_offers;
				while (tmp_offer_list != NULL){
					fprintf(log_file,"Offer by %d for %d cores with spd_loss %0.4f :",tmp_offer_list->sender,tmp_offer_list->off.num_of_cores,tmp_offer_list->off.spd_loss);
					for (i=0; i<tmp_offer_list->off.num_of_cores; i++)
						fprintf(log_file," %d,",tmp_offer_list->off.offered_cores[i]);
					fprintf(log_file,"\n");	
					//printf("Offer by %d for %d cores\n",tmp_offer_list->sender,tmp_offer_list->off.num_of_cores);
					tmp_offer_list = tmp_offer_list->next;
				}
				fflush(log_file);

				*init_man_offers->answer = 1;
			
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
			
				cur_time = time(NULL);	
				cur_t = localtime(&cur_time);
				init_app_times[1].tm_sec = cur_t->tm_sec;
				init_app_times[1].tm_min = cur_t->tm_min;
  			init_app_times[1].tm_hour = cur_t->tm_hour;

				printf("New agent is %d\n",new_agent);
				cur_time = time(NULL);	
				cur_t = localtime(&cur_time);
				fprintf(log_file, "[%d:%d:%d]: New agent is %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,new_agent);
				fflush(log_file);

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
					core_inter_tail[new_agent]->data.app_cores = (int *)malloc((init_man_offers->off.num_of_cores+1)*sizeof(int));
					core_inter_tail[new_agent]->data.app_cores[0] = init_man_offers->off.num_of_cores;
					for(i=1; i<=init_man_offers->off.num_of_cores; i++)
						core_inter_tail[new_agent]->data.app_cores[i] = init_man_offers->off.offered_cores[i-1];
					core_inter_tail[new_agent]->next = NULL;
					init_app.num_of_cores = init_man_offers->off.num_of_cores;

					if (core_inter_head[new_agent]->next == NULL) {
						//kill(pid_num[new_agent], SIG_INIT_AGENT);
						scc_kill(new_agent, SIG_INIT_AGENT);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,new_agent);
					} else printf("I am init manager and i am doing smth else with init agent interaction=%d\n",core_inter_head[new_agent]->type);
				} else {
					my_app = init_app;
					my_cores_count = init_man_offers->off.num_of_cores;
					my_app.num_of_cores = my_cores_count;//+1;
					if (my_cores == NULL) {
						my_cores = (core_list *) malloc(sizeof(core_list));
						my_cores_tail = my_cores;
					} else {
						printf("My cores still not fucking null!!\n");
						my_cores_tail->next = (core_list *) malloc(sizeof(core_list));
						my_cores_tail = my_cores_tail->next;
					}

					my_cores_tail->core_id = node_id;
					my_cores_tail->offered_to = -1;
					my_cores_tail->next = NULL;

					/*if (core_inter_head[my_idag] == NULL){
						core_inter_head[my_idag] = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[my_idag] = core_inter_head[my_idag];
					} else {
						core_inter_tail[my_idag]->next = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[my_idag] = core_inter_tail[my_idag]->next;
					}

					core_inter_tail[my_idag]->type = IDAG_ADD_CORES_DDS;
					core_inter_tail[my_idag]->data.app_cores = (int *)malloc((my_cores_count+1)*sizeof(int));
					core_inter_tail[my_idag]->data.app_cores[0] = my_cores_count;
					core_inter_tail[my_idag]->next = NULL;*/
					//I want myself to be first in my_cores list
					for(i=0; i<init_man_offers->off.num_of_cores; i++) {
						one_core = init_man_offers->off.offered_cores[i];
						//core_inter_tail[my_idag]->data.app_cores[i+1] = one_core;

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
					my_app_times[1].tm_sec = init_app_times[1].tm_sec;	
					my_app_times[1].tm_min = init_app_times[1].tm_min;
					my_app_times[1].tm_hour = init_app_times[1].tm_hour;
					init_app_times[0].tm_sec = 0;
  				init_app_times[0].tm_min = 0;
					init_app_times[0].tm_hour = 0;
					init_app_times[1].tm_sec = 0;
					init_app_times[1].tm_min = 0;
					init_app_times[1].tm_hour = 0;	

					/*kill(pid_num[0], SIG_INIT_APP);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,0);*/

					printf("I am new agent with id %d and app with A=%.2f, var=%.2f and %d my_cores_count = %d cores: \n"
						,node_id,my_app.A,my_app.var,my_app.num_of_cores,my_cores_count);
					my_Speedup = Speedup(my_app, my_app.num_of_cores);

					/*if (core_inter_head[my_idag]->next == NULL) {
						kill(pid_num[my_idag], SIG_ADD_CORES_DDS);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,my_idag);
					} else printf("I am %d and i didn't call add!! with interaction %d\n",node_id,core_inter_head[my_idag]->type);*/

					if (my_agent != -1) {
						printf("I am %d and i do this agent switch in common node with pending_state=%d\n",node_id,pending_state);
						fprintf(log_file,"I do this agent switch in common node with pending_state=%d\n",pending_state);

						if (pending_state == WORKING_NODE) {
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
					state = AGENT_INIT_STATE;

					cur_time = time(NULL);	
					cur_t = localtime(&cur_time);
					fprintf(log_file, "[%d:%d:%d]: Init ok!! my_cores_count = %d app_id=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,my_cores_count,my_app.id);
					fprintf(log_file, "my cores are:");
					for (tmp_cores_list=my_cores; tmp_cores_list!=NULL; tmp_cores_list=tmp_cores_list->next) fprintf(log_file, " %d", tmp_cores_list->core_id);
					fprintf(log_file, "\n");
					fflush(log_file);
				}

				tmp_offer_list = init_man_offers->next;
				while (tmp_offer_list != NULL){
					fflush(log_file);
					*tmp_offer_list->answer = 0;
					tmp_offer_list = tmp_offer_list->next;
				}

				while (init_man_offers != NULL){
					if (core_inter_head[init_man_offers->sender] != NULL) {
						if (core_inter_head[init_man_offers->sender]->type == REP_AGENT_OFFER_PENDING){				
							core_inter_head[init_man_offers->sender]->type = REP_AGENT_OFFER_SENT;
							//kill(pid_num[init_man_offers->sender], SIG_REP_OFFERS);
							scc_kill(init_man_offers->sender, SIG_REP_OFFERS);
							my_stats.msg_count++;
							my_stats.distance += distance(node_id,init_man_offers->sender);	
						} else if (core_inter_head[init_man_offers->sender]->type == REP_FAR_REQ_OFFER_PENDING){
							core_inter_head[init_man_offers->sender]->type = REP_FAR_REQ_OFFER_SENT;				
							//kill(pid_num[init_man_offers->sender], SIG_REP_OFFERS);
							scc_kill(init_man_offers->sender, SIG_REP_OFFERS);
							my_stats.msg_count++;
							my_stats.distance += distance(node_id,init_man_offers->sender);
						} else if (core_inter_head[init_man_offers->sender]->type != REP_FAR_REQ_OFFER_SENT && core_inter_head[init_man_offers->sender]->type != REP_AGENT_OFFER_SENT) 
							printf("We have aasdfa problem!\n");
					} else {
						printf("gamietai b = %d",init_man_offers->sender);
						fprintf(log_file,"gamietai b = %d",init_man_offers->sender);
						fflush(log_file);
					}
					tmp_offer_list = init_man_offers;
					init_man_offers = init_man_offers->next;
					free(tmp_offer_list);
				}

				fclose(init_areas_file);
				fprintf(log_file,"selfopt_time_rem = %ld app_state = %d state = %d pending_state = %d\n",selfopt_time_rem,app_state,state,pending_state);
				fflush(log_file);

				if (selfopt_time_rem != -1 && app_state != APP_TERMINATED){ //a selfopt is pending !app_terminated
					selfopt_time_rem -= 500000000;
					if (selfopt_time_rem > 0){
						its.it_value.tv_nsec = selfopt_time_rem;
						if (timer_settime(timerid, 0, &its, NULL) == -1) printf("timer_settime error5\n");
						state = IDLE_AGENT;
					} else {
						selfopt_time_rem = -1;
						state = AGENT_SELF_OPT;
					}
				} else if (state != AGENT_INIT_STATE) {
					if (pending_state == IDLE_AGENT) {
						printf("I am %d and i entered this realm of whores\n",node_id);
						fprintf(log_file,"I finished my init selfopt_interval=%d, selfopt_time_rem=%ld my_cores_count=%d, old_cores_cnt=%d\n"
							,selfopt_interval,selfopt_time_rem,my_cores_count,old_cores_cnt);
						fflush(log_file);
						fprintf(app_log_file,"I finished my init selfopt_interval=%d, selfopt_time_rem=%ld my_cores_count=%d, old_cores_cnt=%d\n"
							,selfopt_interval,selfopt_time_rem,my_cores_count,old_cores_cnt);
						fflush(app_log_file);

						if (selfopt_interval > 0) { // && !app_terminated
							its.it_value.tv_nsec = selfopt_interval * MS;
							selfopt_time_rem = selfopt_interval;

							if (old_cores_cnt == 1 || old_cores_cnt == my_cores_count) {
								if (my_cores_count == max_cores_count) {
									fprintf(app_log_file, "I have maximum cores count. I don't initiate selfopt process in here anyway.\n");
									fflush(app_log_file);
									selfopt_time_rem = -1;
								} else if (time_per_node <= (selfopt_interval / 2) && my_cores_count > 1) { 
									fprintf(app_log_file, "I have little working time left time_per_node=%d selfopt_interval=%d in here anyway.\n",time_per_node,selfopt_interval);
									fflush(app_log_file);
									selfopt_time_rem = -1;
								} else if (timer_settime(timerid, 0, &its, NULL) == -1) printf("timer_settime error8\n");
							}
						} else if (my_cores_count == 1) {
							printf("I am %d and i have to rewind my selfopt_areas_file after init for app with id %d\n",node_id,init_app.id);
							fprintf(log_file,"I have to rewind my selfopt_areas_file after init for app with id %d\n",init_app.id);
							
							selfopt_interval = 200;
							its.it_value.tv_nsec = selfopt_interval * MS;
							selfopt_time_rem = selfopt_interval;

							if (timer_settime(timerid, 0, &its, NULL) == -1) perror("timer_settime error823\n");

						} else selfopt_time_rem = -1;

						state = IDLE_AGENT;
						pending_state = NO_PENDING_STATE;
					} else if (pending_state == AGENT_SELF_OPT || pending_state == WORKING_NODE || pending_state == AGENT_REWIND_FILE) {
						state = pending_state;//IDLE_AGENT;
						pending_state = NO_PENDING_STATE;
					} else state = IDLE_CORE;
				} else if (app_state != APP_TERMINATED && new_agent != node_id) state = IDLE_AGENT;//!app_terminated
			}
			signals_enable();
			scc_signals_check(); 
		} else if (state == AGENT_INIT_STATE) {//|| state == AGENT_INIT_STATE_INIT_INTERRUPTED
			signals_disable();
			selfopt_interval=200;
			nodes_ended_cnt = 0;
			app_state = RUNNING;
			max_cores_count = get_max_cores_count(my_app);
			//app_terminated = 0;
			//pending_state = NO_PENDING_STATE;

			/*if (my_cores_count > 1){
				time_to_work = (int) roundf(my_app.workld / my_Speedup);
				time_per_node = time_to_work / (my_cores_count-1);
				time_left = time_to_work % (my_cores_count-1);
				if (time_to_work == 0) time_left++;
			}*/
			printf("i am init agent %d. speedup = %0.2f time_to_work = %d time_per_node = %d, workld=%0.2f\n",node_id,my_Speedup,time_to_work,time_per_node,my_app.workld);
			
			strcpy(app_log_file_name,"/shared/herc/scenaria/");
			strcat(app_log_file_name,scen_num);
			strcat(app_log_file_name,"/app_logs/");			
			strcat(app_log_file_name, itoa(my_app.id));
			strcat(app_log_file_name, ".txt");
			//printf("area file path self opt = %s\n",areas_file_name);

			if ((app_log_file = fopen(app_log_file_name, "w")) == NULL){
				printf("Cannot open input file with file path = %s ",app_log_file_name);		
				perror("open selfopt_areas_file_name");
			}

			fprintf(app_log_file, "Came into init_agent at [%d:%d:%d]\n",my_app_times[0].tm_hour,my_app_times[0].tm_min,my_app_times[0].tm_sec);
			fprintf(app_log_file, "New agent found at [%d:%d:%d]\n",my_app_times[1].tm_hour,my_app_times[1].tm_min,my_app_times[1].tm_sec);
			
			cur_time = time(NULL);	
			cur_t = localtime(&cur_time);
			fprintf(app_log_file, "[%d:%d:%d]: I am agent %d . Init ok!! my_cores_count = %d app_id=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,node_id,my_cores_count,my_app.id);
			fprintf(app_log_file, "my cores are:");
			for (tmp_cores_list=my_cores; tmp_cores_list!=NULL; tmp_cores_list=tmp_cores_list->next) fprintf(app_log_file, " %d", tmp_cores_list->core_id);
			fprintf(app_log_file, "\n");
			fprintf(app_log_file,"speedup = %0.2f time_to_work = %d time_per_node = %d, time_left = %d, workld=%0.2f\n",my_Speedup,time_to_work,time_per_node,time_left,my_app.workld);
			fflush(app_log_file);

			/*tmp_cores_list = my_cores->next;
			while (tmp_cores_list != NULL){
				one_core = tmp_cores_list->core_id;
				if (core_inter_head[one_core] == NULL){
					core_inter_head[one_core] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[one_core] = core_inter_head[one_core];
				} else {
					core_inter_tail[one_core]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[one_core] = core_inter_tail[one_core]->next;
				}

				core_inter_tail[one_core]->type = INIT_WORK_NODE;
				core_inter_tail[one_core]->data.work_time = time_per_node;
				if (time_left > 0) {
					core_inter_tail[one_core]->data.work_time++;
					time_left--;
				}
				core_inter_tail[one_core]->next = NULL;
				my_stats.app_turnaround += core_inter_tail[one_core]->data.work_time;	

				if (core_inter_head[one_core]->next == NULL) {
					kill(pid_num[one_core], SIG_APPOINT_WORK);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,one_core);
				} else printf("I am %d and i am doing smth else with my working node %d in init inter1 = %d inter2 = %d\n",
					node_id,one_core,core_inter_head[one_core]->type,core_inter_head[one_core]->next->type);
				
				tmp_cores_list = tmp_cores_list->next;
			} //else printf("i have only one fucking core\n");

			\if (my_cores_count == max_cores_count) {
				fprintf(app_log_file, "I have maximum cores count. I don't initiate selfopt process ini init_agent.\n");
				fflush(app_log_file);
				selfopt_time_rem = -1;
				if (pending_state == IDLE_INIT_MAN || pending_state == INIT_MANAGER || pending_state == INIT_MANAGER_SEND_OFFERS 
					|| pending_state == INIT_MAN_CHK_OFFERS || pending_state == WORKING_NODE_IDLE_INIT) {
					state = pending_state;
					pending_state = IDLE_AGENT;
				} else state = IDLE_AGENT;
			} else if (time_per_node <= (selfopt_interval / 2) && my_cores_count > 1) { 
				fprintf(app_log_file, "I have little working time left time_per_node=%d selfopt_interval=%d in init_agent.\n",time_per_node,selfopt_interval);
				fflush(app_log_file);
				if (pending_state == IDLE_INIT_MAN || pending_state == INIT_MANAGER || pending_state == INIT_MANAGER_SEND_OFFERS 
					|| pending_state == INIT_MAN_CHK_OFFERS || pending_state == WORKING_NODE_IDLE_INIT) {
					state = pending_state;
					pending_state = IDLE_AGENT;
				} else state = IDLE_AGENT;
				selfopt_time_rem = -1;
			} else {
				if (pending_state == IDLE_INIT_MAN || pending_state == INIT_MANAGER || pending_state == INIT_MANAGER_SEND_OFFERS 
					|| pending_state == INIT_MAN_CHK_OFFERS || pending_state == WORKING_NODE_IDLE_INIT) {
					state = pending_state;//IDLE_INIT_MAN;//IDLE_INIT_MAN_SELFOPT_PENDING;
					pending_state = AGENT_SELF_OPT;
					//printf("I am %d o fountas einai poustara pou zwgrafizei k ton fuckaroun oi gkomenes tou!\n",node_id);
				} else state = AGENT_SELF_OPT;
			}*/
			
			scc_kill(0, SIG_APP_TERMINATED);
			state = IDLE_CORE;
			signals_enable();
			scc_signals_check();
		/*} else if (state == AGENT_SELF_OPT) {
			signals_disable();
			printf("Initialising self opt node_id = %d\n",node_id);
			cur_time = time(NULL);	
			cur_t = localtime(&cur_time);
			fprintf(log_file, "[%d:%d:%d]: Initialising self opt!\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);
			fflush(log_file);
			fprintf(app_log_file, "[%d:%d:%d]: Initialising self opt!\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);
			fflush(app_log_file);
			pending_state = NO_PENDING_STATE;
			
			if (app_state != APP_TERMINATED) {//!app_terminated
				selfopt_DDS_idags = 0;
				selfopt_DDS_replies = 0;
				//selfopt_areas_sent = 0;
				//if (selfopt_man_offers != NULL) printf("selfopt offers list is not NULL\n");
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

				core_inter_tail[my_idag]->type = SELFOPT_IDAG_FIND_IDAGS_PENDING;
				core_inter_tail[my_idag]->data.reg.C = node_id;
				core_inter_tail[my_idag]->data.reg.r = selfopt_r;
				core_inter_tail[my_idag]->next = NULL;

				if (core_inter_head[my_idag]->next == NULL)	{
					kill(pid_num[my_idag],SIG_IDAG_FIND_IDAGS);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,my_idag);
				}
					
				if (selfopt_interval != 800) selfopt_interval = 2 * selfopt_interval;
				else selfopt_interval = -1;
			
				state = IDLE_AGENT_WAITING_OFF;
			} else state = AGENT_ENDING;
		
			signals_enable();
			scc_signals_check();
		} else if (state == AGENT_SELF_CHK_OFFERS) {
			signals_disable();			
			printf("alarm went off for self opt node_id=%d selfopt_DDS_idags = %d selfopt_DDS_replies = %d! app_state=%d\n",node_id,selfopt_DDS_idags,selfopt_DDS_replies,app_state);
			cur_time = time(NULL);	
			cur_t = localtime(&cur_time);
			fprintf(log_file, "[%d:%d:%d]: alarm went off for self opt selfopt_DDS_idags = %d selfopt_DDS_replies = %d app_state=%d pending_state = %d\n",
				cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,selfopt_DDS_idags,selfopt_DDS_replies,app_state,pending_state);
			fflush(log_file);
			fprintf(app_log_file, "[%d:%d:%d]: alarm went off for self opt selfopt_DDS_idags = %d selfopt_DDS_replies = %d app_state=%d pending_state = %d\n",
				cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,selfopt_DDS_idags,selfopt_DDS_replies,app_state,pending_state);
			fflush(app_log_file);

			old_cores_cnt = my_cores_count;
			if (selfopt_man_offers == NULL) {
				printf("I don't have self opt offers app_state=%d\n",app_state);
				fprintf(log_file,"I don't have self opt offers app_state=%d\n",app_state);
				fflush(log_file); 
				fprintf(app_log_file,"I don't have self opt offers app_state=%d\n",app_state);
				fflush(app_log_file);
				//state = IDLE_AGENT;
			} else {
				tmp_offer_list = selfopt_man_offers;
				while (tmp_offer_list != NULL){
					//printf("Offer by %d for %d cores\n",tmp_offer_list->sender,tmp_offer_list->off.num_of_cores);
					fprintf(log_file,"Offer by %d for %d cores\n",tmp_offer_list->sender,tmp_offer_list->off.num_of_cores);
					fflush(log_file);					
					tmp_offer_list = tmp_offer_list->next;
				}
			
				if (app_state != APP_TERMINATED) {
					*selfopt_man_offers->answer = 1;
				
					if (core_inter_head[my_idag] == NULL){
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

					// 
					if (app_state != RESIZING) {
						old_Speedup = my_Speedup;
						chk_rem_num = my_cores_count-1;
						fprintf(app_log_file,"Initialising resize with check_rem = %d\n",chk_rem_num);
						fflush(app_log_file);						
						chk_rem_count = 0;
						sum_rem_time = 0;
						app_state = RESIZING;
						my_cores_count += selfopt_man_offers->off.num_of_cores;
						my_app.num_of_cores = my_cores_count;
						my_Speedup = Speedup(my_app, my_cores_count);

						if (old_cores_cnt > 1)
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
									kill(pid_num[one_core], SIG_CHECK_REM_TIME);
									my_stats.msg_count++;
									my_stats.distance += distance(node_id,one_core);
								} else printf("I am %d and i am doing smth else with my working node %d in send SIG_CHECK_REM_TIME interaction = %d\n",node_id,one_core,core_inter_head[one_core]->type);
							}	
					} else {
						fprintf(app_log_file,"Allready resizing with chk_rem_num = %d chk_rem_count=%d\n",chk_rem_num,chk_rem_count);
						fflush(app_log_file);		 
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
						//printf("I am %d and i put in appoint work %d\n",node_id,one_core);
						if (core_inter_head[one_core] == NULL){
							core_inter_head[one_core] = (inter_list *) malloc(sizeof(inter_list));
							core_inter_tail[one_core] = core_inter_head[one_core];
						} else {
							printf("I am %d and I am doing sth with my new core %d interaction is %d\n",node_id,one_core,core_inter_head[one_core]->type);
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
						kill(pid_num[my_idag], SIG_ADD_CORES_DDS);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,my_idag);
					} else printf("I am %d i didn't call add inside selfopt with interaction %d\n",node_id,core_inter_head[my_idag]->type);

					if (old_cores_cnt == 1) {  
						nodes_ended_cnt = 0;
						time_to_work = (int) roundf(my_app.workld / my_Speedup);
						time_per_node = time_to_work / (my_cores_count-1);
						time_left = time_to_work % (my_cores_count-1);
						if (time_to_work == 0) time_left++;
						
						fprintf(app_log_file,"in one core speedup = %0.2f time_to_work = %d time_per_node = %d, workld=%0.2f\n",my_Speedup,time_to_work,time_per_node,my_app.workld);
						fflush(app_log_file);

						tmp_cores_list = my_cores->next;
						while (tmp_cores_list != NULL){
							//printf("I am inside core listing node=%d\n",node_id);
							one_core = tmp_cores_list->core_id;

							for (tmp_inter_list = core_inter_head[one_core]; tmp_inter_list != NULL; tmp_inter_list = tmp_inter_list->next)
								if (tmp_inter_list->type == INIT_WORK_NODE_PENDING) break;

							if (tmp_inter_list != NULL) {
								tmp_inter_list->type = INIT_WORK_NODE;
								tmp_inter_list->data.work_time = time_per_node;
								if (time_left > 0) {
									tmp_inter_list->data.work_time++;
									time_left--;
								}
								my_stats.app_turnaround += tmp_inter_list->data.work_time;
							} else printf("i am %d kai sto common_node ta hpiame one core = %d\n",node_id,one_core);
	
							if (core_inter_head[one_core]->next == NULL) {
								kill(pid_num[one_core], SIG_APPOINT_WORK);
								my_stats.msg_count++;
								my_stats.distance += distance(node_id,one_core);
							} else printf("I am doing smth else with my working node? interaction=%d interaction2=%d\n",core_inter_head[one_core]->type,core_inter_head[one_core]->next->type);
					
							tmp_cores_list = tmp_cores_list->next;
						}

					app_state = RUNNING;
					}
				}

				while (selfopt_man_offers != NULL) {
					if (core_inter_head[selfopt_man_offers->sender]->type == REP_AGENT_OFFER_PENDING){				
						core_inter_head[selfopt_man_offers->sender]->type = REP_AGENT_OFFER_SENT;
						kill(pid_num[selfopt_man_offers->sender], SIG_REP_OFFERS);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,selfopt_man_offers->sender);	
					} else if (core_inter_head[selfopt_man_offers->sender]->type != REP_AGENT_OFFER_SENT) printf("We have zxcvzxc problem!\n");
					tmp_offer_list = selfopt_man_offers;
					selfopt_man_offers = selfopt_man_offers->next;
					free(tmp_offer_list);
				}

				printf("Self opt seems to be ok cores count = %d\n",my_cores_count);
				cur_time = time(NULL);	
				cur_t = localtime(&cur_time);
				fprintf(log_file, "[%d:%d:%d]: Self opt ok. my_cores_count = %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,my_cores_count);
				fprintf(log_file, "my cores are:");
				fprintf(app_log_file, "[%d:%d:%d]: Self opt ok. my_cores_count = %d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,my_cores_count);
				fprintf(app_log_file, "my cores are:");

				printf("I am %d and my cores are:",node_id);
				for (tmp_cores_list=my_cores; tmp_cores_list!=NULL; tmp_cores_list=tmp_cores_list->next) {
					fprintf(log_file, " %d", tmp_cores_list->core_id);
					fprintf(app_log_file, " %d", tmp_cores_list->core_id);					
					printf(" %d",tmp_cores_list->core_id);
				}

				printf("\n");
				fprintf(log_file, "\n");
				fflush(log_file);
				fprintf(app_log_file, "\n");
				fflush(app_log_file);
			}

			if (app_state == APP_TERMINATED) state = AGENT_ENDING;//app_terminated
			else {
				if (pending_state == INIT_MANAGER) {
					state = INIT_MANAGER;
					pending_state = IDLE_AGENT;
				} else {
					printf("I am %d in chk_offers and pending_state=%d\n",node_id,pending_state);
					if (selfopt_interval > 0) { // && !app_terminated
						its.it_value.tv_nsec = selfopt_interval * MS;
						selfopt_time_rem = selfopt_interval;

						if (old_cores_cnt == 1 || old_cores_cnt == my_cores_count) {
							if (my_cores_count == max_cores_count) {
								fprintf(app_log_file, "I have maximum cores count. I don't initiate selfopt process in selfopt.\n");
								fflush(app_log_file);
								selfopt_time_rem = -1;
							} else if (time_per_node <= (selfopt_interval / 2) && my_cores_count > 1) { 
								fprintf(app_log_file, "I have little working time left time_per_node=%d selfopt_interval=%d in selfopt.\n",time_per_node,selfopt_interval);
								fflush(app_log_file);
								selfopt_time_rem = -1;
							} else if (timer_settime(timerid, 0, &its, NULL) == -1) printf("timer_settime error8\n");
						}
					} else if (my_cores_count == 1) {
						printf("I am %d and i have to rewind my selfopt_areas_file for app with id %d\n",node_id,init_app.id);
						fprintf(log_file,"I have to rewind my selfopt_areas_file for app with id %d\n",my_app.id);
						fflush(log_file);
						fprintf(app_log_file,"I have to rewind my selfopt_areas_file for app with id %d\n",my_app.id);
						fflush(app_log_file);						
						
						selfopt_interval = 200;
						its.it_value.tv_nsec = selfopt_interval * MS;
						selfopt_time_rem = selfopt_interval;

						if (timer_settime(timerid, 0, &its, NULL) == -1) perror("timer_settime error82\n");

					} else selfopt_time_rem = -1;
			
					state = IDLE_AGENT;
				}
			}

			signals_enable();
		} else if (state == WORKING_NODE || state == WORKING_NODE_IDLE_INIT){
				int j,dummy=0;
				signals_disable();

				while (time_worked <= upper_work_bound){
					signals_enable();
					//the following code should be one ms long
					for (i=0; i<1000; i++)
						for(j=0; j<1000; j++)
							dummy++;
					signals_disable();
					time_worked++;
				}
				printf("I am node %d with agent = %d and I finished my work upper_work_bound = %ld time_passed=%d\n",node_id,my_agent,upper_work_bound,time_worked);
				if (upper_work_bound > 0) {
					kill(pid_num[my_agent],SIG_FINISH);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,my_agent);				
				}
				if (state == WORKING_NODE) state = IDLE_CORE;
				else if (state == WORKING_NODE_IDLE_INIT) state = IDLE_INIT_MAN;
				else if (pending_state == WORKING_NODE) pending_state = NO_PENDING_STATE; //just finished on return
				signals_enable();				
		} else if (state == AGENT_ZOMBIE) {		
			pause();	
		} else if (state == AGENT_ENDING) {
			signals_disable();

			cur_time = time(NULL);	
			cur_t = localtime(&cur_time);
			fprintf(log_file, "[%d:%d:%d]: I entered agent_ending\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);
			fflush(log_file);
			fprintf(app_log_file, "[%d:%d:%d]: I entered agent_ending\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);
			fflush(app_log_file);			

			for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next)
				fprintf(app_log_file,"Core %d is offered to %d\n",tmp_cores_list->core_id,tmp_cores_list->offered_to);

			fflush(app_log_file);

			for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next)
				if (tmp_cores_list->offered_to != -1) break;

			if (tmp_cores_list != NULL) {
				state = AGENT_ZOMBIE;

				for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next) {
					one_core = tmp_cores_list->core_id;
					
					tmp_inter_prev = NULL;
					tmp_inter_list = core_inter_head[one_core];
					//for (tmp_inter_list = core_inter_head[one_core]; tmp_inter_list != NULL; tmp_inter_list=tmp_inter_list->next)
					while (tmp_inter_list != NULL)
						if (tmp_inter_list->type == INIT_WORK_NODE || tmp_inter_list->type == APPOINT_WORK_NODE || tmp_inter_list->type == INIT_WORK_NODE_PENDING 
							|| tmp_inter_list->type == APPOINT_WORK_NODE_PENDING || tmp_inter_list->type == REMOVED_NODE_REM_TIME) {							
							//|| (tmp_inter_list->type == APPOINT_WORK_NODE_PENDING && tmp_inter_prev != NULL)) {
							
							fprintf(log_file, "Removing in zombie one node of %d with inter = %d\n",one_core,tmp_inter_list->type);
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
				//for(my_cores=my_cores->next; my_cores!=NULL; my_cores = my_cores->next) {
					//free(tmp_cores_list);
				while (my_cores != NULL) {
					tmp_cores_list = my_cores;
					one_core = tmp_cores_list->core_id;
					core_inter_tail[my_idag]->data.app_cores[i++] = one_core;

					tmp_inter_prev = NULL;
					tmp_inter_list = core_inter_head[one_core];
					//for (tmp_inter_list = core_inter_head[one_core]; tmp_inter_list != NULL; tmp_inter_list=tmp_inter_list->next)
					while (tmp_inter_list != NULL)
						if (tmp_inter_list->type == INIT_WORK_NODE || tmp_inter_list->type == APPOINT_WORK_NODE || tmp_inter_list->type == INIT_WORK_NODE_PENDING 
							|| tmp_inter_list->type == APPOINT_WORK_NODE_PENDING || tmp_inter_list->type == REMOVED_NODE_REM_TIME) {
							
							fprintf(log_file, "Removing one node of %d with inter = %d\n",one_core,tmp_inter_list->type);
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
	
					if (core_inter_head[one_core] == NULL) {
						kill(pid_num[one_core],SIG_FINISH);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,one_core);
					} else printf("I am %d and I still still have smth to do with my work node %d before finish which is %d\n",node_id,one_core,core_inter_head[one_core]->type);
					
					my_cores=my_cores->next;
					free(tmp_cores_list);
				}
					
				if (core_inter_head[my_idag]->next == NULL) {
					kill(pid_num[my_idag],SIG_FINISH);
					my_stats.msg_count++;
					my_stats.distance += distance(node_id,my_idag);
				} else printf("I am %d and i am doing smth else with my idag app_remove interaction=%d\n",node_id,core_inter_head[my_idag]->type);

				my_app.A = 0.0;
				my_app.var = 0.0;
				my_app.num_of_cores = -1;
				selfopt_time_rem = -1;
				//app_terminated = 0;
				app_state = NO_APP;
				printf("I am %d My app ended app_id = %d pending_state=%d\n",node_id,my_app.id,pending_state);
			
				cur_time = time(NULL);	
				cur_t = localtime(&cur_time);
				fprintf(app_log_file, "[%d:%d:%d]: App ended app_id = %d pending_state=%d",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,my_app.id,pending_state);
				//fprintf(app_log_file,"App ended app_id = %d pending_state=%d\n",my_app.id,pending_state);
				fclose(app_log_file);			

				if (pending_state == INIT_MANAGER) {
					state = INIT_MANAGER;
					pending_state = IDLE_CORE;
				} else if (pending_state == NO_PENDING_STATE)	state = IDLE_CORE;
				else {
					printf("I am %d in agent_ending and pending_state = %d\n",node_id,pending_state);
					state = IDLE_CORE;
				}
			}
			signals_enable();	*/		
		} else {
			printf("Unknown state node_id = %d state = %d\n",node_id,state);	
			state = IDLE_CORE;
		}

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	fprintf(log_file, "[%d:%d:%d]: I ended well\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);
	fclose(log_file);
	exit(0);
}

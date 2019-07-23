#include "include/my_rtrm.h"
#include "include/idag_defs.h"
#include "include/libfunctions.h"
#include "include/noc_functions.h"
#include "include/sig_aux.h"
#include "include/controller_core.h"
#include "include/common_core.h"
#include "include/signal_handlers.h"
#include "include/scc_signals.h"
#include "include/idag_defs.h"
#include "include/paxos_signal_handlers.h"
#include "include/apps.h"

#ifdef PLAT_SCC
int idag_mask[X_max*Y_max];
int low_voltage_core[X_max*Y_max];
int timer_schedule[X_max*Y_max];

volatile int *manager_result_out;
volatile int *index_bottom;

RCCE_FLAG flag_data_written;
RCCE_FLAG proposal_number_lock;

int num_idags_x = 1; //FIXME
#else
int X_max, Y_max;
int *pid_num = NULL;
int *idag_mask = NULL;
int *low_voltage_core = NULL;
int *timer_schedule = NULL;
int *manager_result_out = NULL;
int *index_bottom = NULL;
int num_idags_x;

sem_t *flag_data_written;
sem_t *scc_lock;
sem_t *proposal_number_lock;

#endif

int node_id = -1;
int my_idag = -1;
int num_idags = 0;
int DDS_count = 0;
int my_cores_count = 0;
int nodes_ended_cnt = 0;
int nodes_initialised = 0;
int stats_replied = 0;
int paxos_stats_replied = 0;
int num_apps_terminated = 0;
int num_apps = 0;
int idags_replied = 0;
int index_top = 0;
int last_index_bottom = 0;
int NUES;
int manager_to_fail = -1;

int *sig_array = NULL;
int *data_array = NULL;
int *idag_id_arr = NULL;
int *proposal_number_global;

char paxos_scen[MAX_STR_NAME_SIZE];
char app_input_file[MAX_STR_NAME_SIZE];

FILE *log_file = NULL;
FILE *init_ack_file = NULL;

core_states state;

inter_list **core_inter_head;
inter_list **core_inter_tail;
inter_list *init_pending_head;
inter_list *init_pending_tail;

app my_app;

metrics my_stats = {0,0,0,0,0,0,0,0};
metrics total_stats = {0,0,0,0,0,0,0,0};
metrics paxos_node_stats = {0,0,0,0,0,0,0,0}; 
metrics paxos_total_stats = {0,0,0,0,0,0,0,0};
  
DDS_list *DDS = NULL;
DDS_list *DDS_tail = NULL;

core_list *my_cores = NULL;
core_list *my_cores_tail = NULL;

time_t cur_time;
timer_t timerid;
timer_t inter_timer;
timer_t controller_timer;
timer_t epfd_timer;
timer_t pfd_timer;

struct tm *cur_t;
struct sigevent sev;
struct itimerspec its;
struct itimerspec chk_timer;
struct timeval time_val;

/* Dimos variables */
int PREPARE_ACCEPT_SENT = 0;
int controllers_replied = 0;

int delay = 500;
char *tabs;

#ifdef PLAT_SCC
int RCCE_APP(int argc, char *argv[]) {
  
	char error_str[64];
	int error, str_len, sig_array_local[LINE_SIZE];
	
	RCCE_init(&argc, &argv);
	node_id = RCCE_ue();
#else
int main(int argc, char *argv[]) {
  
	int segment_id;
#endif
	int num_of_bytes;
	int i;
	int j;
	int k;
	int c;
	int one_idag;
	int one_core;
	int app_cnt = 0;
	int time_passed = -1;
	int time_next;
	int init_core;
	int timer_init_null = 0;
	int executed_app_type_number = 0; /* Used for the initialization of init_app type */
	int executed_app_array_size = 0; /* Used for the initialization of init_app type */
	int Selfopt_Radius,Selfopt_Rounds,Max_SelfOpt_Interval_MS;

	float avg_cluster_util;
	
	char scen_directory[MAX_STR_NAME_SIZE];
	char scen_num[MAX_STR_NAME_SIZE];
	char idag_defs_file_name[MAX_STR_NAME_SIZE];
	char app_input_file_name[MAX_STR_NAME_SIZE];
	char time_log_file_name[MAX_STR_NAME_SIZE];
	char init_ack_file_name[MAX_STR_NAME_SIZE];
	
	inter_list *tmp_inter_list;
	inter_list *tmp_inter_prev;
	inter_list *tmp_pending_head;
	
	FILE *app_input;
	FILE *time_log;
	
	struct tm start_time_of_day, *end_time_of_day;
	
	pid_t p;
	DDS_list *tmp_DDS;
	core_list *tmp_cores_list;
	struct timeval time_val;

	setvbuf(stdout, NULL, _IONBF, 0);
	
	if (argc < 21) {
		printf("USAGE: ./my_rtrm <options>\n\n");
		printf("options:\n");
		printf("\t-d <scen_dir>    : <scen_dir> = Directory of all scenarios.\n");
		printf("\t-n <scen_num>    : <scen_num> = Scenario number to be executed.\n");
		printf("\t-i <idag_defs>   : <idag_defs> = File with the definitions of grid and idags.\n");
		printf("\t-a <app_input>   : <app_input> = File with application characteristics.\n");
		printf("\t-p <paxos_scen>  : <paxos_scen> = File with failure characteristics.\n");
		printf("\t-x X             : X = X * 6 will be the horizontal dimension of the grid.\n"); 
		printf("\t-y Y             : Y = Y * 8 will be the vertical dimension of the grid.\n");
		printf("\t-t <app_type>    : <app_type> M for MATRIX_MUL, S for SVM, F for FFT, A for Artificial \n");
		printf("\t-r <region_r>    : Selfopt radius \n");
		printf("\t-u <rounds>	   : Selfopt rounds \n");
		exit(0);
	}

	/* Put h last in getopt in order to work correctly */
	while ((c = getopt(argc, argv, "d:n:i:x:y:a:p:t:r:u:h")) != -1){
		switch(c){
			case 'd':
				strcpy(scen_directory, optarg);
				if (node_id == 0)
					printf("Scenario directory : %s...\n",scen_directory);
				break;
			case 'n':
				strcpy(scen_num, optarg);
				if (node_id == 0)
					printf("Scenario number : %s...\n",scen_num);
				break;
			case 'i':
				strcpy(idag_defs_file_name,optarg);
				if (node_id == 0)
					printf("Idag definitions file name : %s...\n",idag_defs_file_name);
				break;
			case 'x':
				#ifdef PLAT_LINUX
				X_max = atoi(optarg) * 6;
				#endif
				if (node_id == 0)
					printf("X = %d...\n",X_max);
				break;		
			case 'y':
				#ifdef PLAT_LINUX
				Y_max = atoi(optarg) * 8;
				#endif
				if (node_id == 0)
					printf("Y = %d...\n",Y_max);
				break;
			case 'r':
				Selfopt_Radius = atoi(optarg);
				if (node_id == 0)
								printf("Selfopt_Radius = %d...\n",Selfopt_Radius);
				break;
			case 'u':
				Selfopt_Rounds = atoi(optarg);
				if (node_id == 0) {
								printf("Selfopt_Rounds = %d...\n",Selfopt_Rounds);
				}
				
				Max_SelfOpt_Interval_MS = LEAST_SELF_OPT_INTERVAL_MS;
				for (i=1; i<Selfopt_Rounds; i++) {
					Max_SelfOpt_Interval_MS = 2 * Max_SelfOpt_Interval_MS;
				}
                                break;
			case 'a':
				strcpy(app_input_file, optarg);
				if (node_id == 0)
					printf("Application input file %s...\n",app_input_file);
				break;
			case 'p':
				strcpy(paxos_scen, optarg);
				if (node_id == 0)
					printf("Paxos scenario file name %s...\n", paxos_scen);
				break;
			case 't':
				if (!strcmp(optarg,"M")) { /* FIXME merge 10.7.2017 executed_app and executed_app_type_number */
					executed_app = MATRIX_MUL;
					executed_app_type_number = 0;
					executed_app_array_size = MATRIX_ARRAY_SIZE;
					//printf("Input application type is Matrix mul\n");
				} else if (!strcmp(optarg,"S")) {
					executed_app = SVM;
					executed_app_type_number = 1;
					executed_app_array_size = SVM_ARRAY_SIZE;
					//printf("Input application type is SVM\n");
				} else if (!strcmp(optarg,"F")) {
					executed_app = FFT;
					executed_app_type_number = 2;
					executed_app_array_size = FFT_ARRAY_SIZE;
					//printf("Input application type is FFT\n");
				} else if (!strcmp(optarg,"A")) {
					executed_app = ARTIFICIAL;
					executed_app_type_number = 3;
					//printf("Input application type is FFT\n");
				} else {
					printf("Input application type is wrong. Options are M,S,F\n");
				}
				break;
				
			case 'h':
				printf("USAGE: ./my_rtrm <options>\n\n");
				printf("options:\n");
				printf("\t-d <scen_dir>    : <scen_dir> = Directory of all scenarios.\n");
				printf("\t-n <scen_num>    : <scen_num> = Scenario number to be executed.\n");
				printf("\t-i <idag_defs>   : <idag_defs> = File with the definitions of grid and idags.\n");
				printf("\t-a <app_input>   : <app_input> = File with application characteristics.\n");
				printf("\t-p <paxos_scen>  : <paxos_scen> = File with failure characteristics.\n");
				printf("\t-x X             : X = X * 6 will be the horizontal dimension of the grid.\n"); 
				printf("\t-y Y             : Y = Y * 8 will be the vertical dimension of the grid.\n");
				printf("\t-t <app_type>    : <app_type> M for MATRIX_MUL, S for SVM, F for FFT \n");
				printf("\t-r <region_r>    : Selfopt radius \n");
		                printf("\t-u <rounds>      : Selfopt rounds \n");
				exit(0);
				break;
		}
	}
	
	#ifdef PLAT_SCC
		NUES = RCCE_num_ues();
		RCCE_flag_alloc(&flag_data_written);
		RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, node_id);
		RCCE_flag_alloc(&proposal_number_lock);
		RCCE_flag_write(&proposal_number_lock, RCCE_FLAG_UNSET, node_id);
		sig_array = (int *) RCCE_malloc(MAX_SIGNAL_LIST_LEN * LINE_SIZE * sizeof(int));//NUES * NUES
		data_array = (int *) RCCE_malloc(MAX_DATA_LIST_LEN * LINE_SIZE * sizeof(int));

		#ifndef ARTIFICIAL_APPS_SIM		
		if (executed_app == MATRIX_MUL) {
			num_of_bytes = NUES * MAX_ARRAY_SIZE * sizeof(int);
		} else if (executed_app == SVM) {
			num_of_bytes = NUES * SVM_ARRAY_SIZE * sizeof(float);
		} else if (executed_app == FFT) {
			num_of_bytes = NUES * FFT_ARRAY_SIZE * sizeof(float);
		}
		manager_result_out = (volatile int*) RCCE_shmalloc(num_of_bytes);
		#endif

		num_of_bytes = NUES * sizeof(int);
		index_bottom = (volatile int*) RCCE_shmalloc(num_of_bytes);
		
		proposal_number_global = (int *)RCCE_shmalloc(sizeof(int *));
		*proposal_number_global = 0;

		for (i=0; i<LINE_SIZE; i++) 
			sig_array_local[i] = NO_SIG;

		for (i=0; i<MAX_SIGNAL_LIST_LEN; i++) {
			error = RCCE_put((t_vcharp)(&sig_array[i*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), node_id);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				fprintf(log_file,"I got an error in put 2 with descr %s\n",error_str);
				fflush(log_file);
			}
		}

	#else
		idag_mask = (int *) malloc(X_max*Y_max*sizeof(int));
		low_voltage_core = (int *) malloc(X_max*Y_max*sizeof(int));
		timer_schedule = (int *) malloc(X_max*Y_max*sizeof(int));
	
		NUES = X_max * Y_max;
		node_id = 0;
		
		num_of_bytes = NUES * sizeof(pid_t);
		segment_id = shmget (IPC_PRIVATE, num_of_bytes, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
		
		pid_num = (int *) shmat (segment_id, NULL, 0);
		
		num_of_bytes = NUES * MAX_SIGNAL_LIST_LEN * LINE_SIZE * sizeof(int);
		segment_id = shmget (IPC_PRIVATE, num_of_bytes, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

		sig_array = (int *) shmat (segment_id, NULL, 0); //(int *) RCCE_malloc(MAX_SIGNAL_LIST_LEN * LINE_SIZE * sizeof(int));//NUES * NUES
		
		for (i = 0; i < MAX_SIGNAL_LIST_LEN*LINE_SIZE; i++) {
			sig_array[i] = NO_SIG;
		}
		
		num_of_bytes = NUES * MAX_DATA_LIST_LEN * LINE_SIZE * sizeof(int);
		segment_id = shmget (IPC_PRIVATE, num_of_bytes, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

		data_array = (int *) shmat (segment_id, NULL, 0); //(int *) RCCE_malloc(3 * LINE_SIZE * sizeof(int));

		if (executed_app == MATRIX_MUL) {
			num_of_bytes = NUES * MAX_ARRAY_SIZE * sizeof(int);
		} else if (executed_app == SVM) {
			num_of_bytes = NUES * SVM_ARRAY_SIZE * sizeof(int);
		} else if (executed_app == FFT) {
			num_of_bytes = NUES * FFT_ARRAY_SIZE * sizeof(float);
		}
		segment_id = shmget (IPC_PRIVATE, num_of_bytes, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

		manager_result_out = (int *) shmat (segment_id, NULL, 0); //(volatile int*) RCCE_shmalloc(num_of_bytes);
		
		num_of_bytes = NUES * sizeof(int);
		segment_id = shmget (IPC_PRIVATE, num_of_bytes, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

		index_bottom = (int *) shmat (segment_id, NULL, 0); //(volatile int*) RCCE_shmalloc(num_of_bytes);

		//for (i=0; i<LINE_SIZE; i++) 
		//	sig_array_local[i] = NO_SIG;
		
		//#if defined(BASIC_PAXOS)
		num_of_bytes = sizeof(sem_t *);
		if ((segment_id = shmget(IPC_PRIVATE, num_of_bytes, IPC_CREAT | S_IRUSR | S_IWUSR)) < 0){
		  printf("Unable to allocate shared memory!\n");
		  exit(1);
		}
		proposal_number_lock = (sem_t *) shmat (segment_id, NULL, 0);
		if (sem_init(proposal_number_lock, 1, 1) == -1){
			printf("I am %d error\n",node_id);
			perror("sem_init");
		}
		
		num_of_bytes = sizeof(sem_t *);
		if ((segment_id = shmget(IPC_PRIVATE, num_of_bytes, IPC_CREAT | S_IRUSR | S_IWUSR)) < 0){
		  printf("Unable to allocate shared memory!\n");
		  exit(1);
		}

		num_of_bytes = sizeof(int *);
		if ((segment_id = shmget(IPC_PRIVATE, num_of_bytes, IPC_CREAT | S_IRUSR | S_IWUSR)) < 0){
		  printf("Unable to allocate shared memory!\n");
		  exit(1);
		} 
		proposal_number_global = (int *) shmat (segment_id, NULL, 0);
		*proposal_number_global = 0;
		//#endif
		
		num_of_bytes = NUES * sizeof(sem_t);
		segment_id = shmget (IPC_PRIVATE, num_of_bytes, IPC_CREAT | S_IRUSR | S_IWUSR);

		scc_lock = (sem_t *) shmat (segment_id, NULL, 0);

		if (sem_init(&scc_lock[node_id], 1, 1) == -1){
			printf("I am %d error\n",node_id);
			perror("sem_init");
		}

		segment_id = shmget (IPC_PRIVATE, num_of_bytes,
			IPC_CREAT | S_IRUSR | S_IWUSR);

		flag_data_written = (sem_t *) shmat (segment_id, NULL, 0);

		if (sem_init(&flag_data_written[node_id], 1, 0) == -1){
			printf("I am %d error\n",node_id);
			perror("sem_init");
		}
	#endif
	init_speedup_structs();
	
	core_inter_head = (inter_list **) malloc(X_max*Y_max*sizeof(inter_list *));
	core_inter_tail = (inter_list **) malloc(X_max*Y_max*sizeof(inter_list *));
	for (i=0; i<X_max*Y_max; i++){
		core_inter_head[i] = NULL;
		core_inter_tail[i] = NULL;
		timer_schedule[i] = 0;
	}

	read_idag_defs(scen_directory, scen_num, idag_defs_file_name, paxos_scen);
	global_idag_defs();
	/* Initialise structs for mapping of scc coordinates to normal ones*/
	create_scc2grid_mapping(scen_directory, scen_num);
	create_grid2scc_mapping(scen_directory, scen_num);
	initialize_PAXOS_data(scen_directory,scen_num);
	
#ifdef MANAGER
	if (manager_to_fail == -1){
		time_t t;
		srand((unsigned) time(&t));
		manager_to_fail = rand() % 8;
		//printf("MANAGER TO FAIL IS OF APP %d\n",manager_to_fail);
	}
#endif

	/* Create rest of idags and assign only their node id */
	#ifdef PLAT_LINUX
	printf("I an idag with node_id = %d, pid = %d\n",0,getpid());
	for (i=1; i < num_idags; i++) {
		p = fork();
		if (p == 0) {
			node_id = idag_id_arr[i];
			idle_agent_actions(scen_directory,scen_num,Selfopt_Radius,Max_SelfOpt_Interval_MS);
			printf("I am newly created idag with i = %d and node_id = %d\n",i,node_id);
			fflush(stdout);
			break;
		}
	}
	#endif
	
	if ((node_id != idag_id_arr[0]) && (is_core_idag(node_id) == 1)) {
		#ifndef PLAT_LINUX
		idle_agent_actions(scen_directory,scen_num);
		#else
		idle_agent_actions(scen_directory,scen_num,Selfopt_Radius,Max_SelfOpt_Interval_MS);
		#endif
	} else if (node_id != idag_id_arr[0]) {
		common_node_actions(scen_directory,scen_num,Selfopt_Radius,Max_SelfOpt_Interval_MS);
	} else {

	 index_bottom[node_id] = 0;
	 install_signal_handlers();

	 #ifdef PLAT_LINUX	
	 sig_SEGV_enable();
	 #endif

	 sev.sigev_notify = SIGEV_SIGNAL;
	 sev.sigev_signo = SIG_TIMER;
	 sev.sigev_value.sival_ptr = &timerid;
	 if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1) printf("timer_create error\n");
	 
	 if (my_cores == NULL) {
		  my_cores = (core_list *) malloc(sizeof(core_list));
		  my_cores_tail = my_cores;
	 }
	 
	 my_cores_count++;
	 my_cores_tail->core_id = node_id;
	 my_cores_tail->offered_to = -1;
	 my_cores_tail->next = NULL;
	 
	 DDS = (DDS_list *) malloc(sizeof(DDS_list));
	 DDS->agent_id = node_id;
	 DDS->next = NULL;
	 DDS_tail = DDS;
	 DDS_count++;
	 for (i = 0; i < NUES; i++)
		  if (node_id != i && idag_mask[i] == node_id){
			 if (my_cores == NULL) {
				  my_cores = (core_list *) malloc(sizeof(core_list));
				  my_cores_tail = my_cores;
			 } else {
				  my_cores_tail->next = (core_list *) malloc(sizeof(core_list));
				  my_cores_tail = my_cores_tail->next;
			 }

			 my_cores_count++;
			 my_cores_tail->core_id = i;
			 my_cores_tail->offered_to = -1;
			 my_cores_tail->next = NULL;

			 #ifdef PLAT_LINUX
			 p = fork();
			 if (p==0) {
				node_id = i;
				common_node_actions(scen_directory,scen_num,Selfopt_Radius,Max_SelfOpt_Interval_MS);	
			 }
			 #endif
		}

	DDS->num_of_cores = my_cores_count;
	log_file = create_log_file(node_id, 0, scen_directory, scen_num);
	setbuf(log_file, NULL);
	        
	fprintf(log_file,"Selfopt_Radius = %d Selfopt_Rounds = %d Max_SelfOpt_Interval_MS = %d\n",Selfopt_Radius,Selfopt_Rounds,Max_SelfOpt_Interval_MS);
	if (node_id == 0) {
#ifdef RESOURCE_ALGO_ORIG
		printf("Resource algo is original\n");
		fprintf(log_file,"Resource algo is original\n");
#elif RESOURCE_ALGO_UPDATED
		printf("Resource algo is updated\n");
		fprintf(log_file,"Resource algo is updated\n");
#elif RESOURCE_ALGO_UPDATED_GENEROUS
		printf("Resource algo is updated generous\n");
                fprintf(log_file,"Resource algo is updated generous\n");
#else
		printf("Resource algo not chosen. Fallback to original\n");
		fprintf(log_file,"Resource algo not chosen. Fallback to original\n");
#endif
	}
 
	alive = (int *)malloc(X_max*Y_max*sizeof(int));
	suspected = (int *)malloc(X_max*Y_max*sizeof(int));
	for (i = 0; i < X_max*Y_max; i++){
		alive[i] = 0;
		suspected[i] = 0;
	}

	printf("DELAY = %d\n",delay);
	#if defined(EPFD) || defined(tEPFD)
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIG_EPFD_TIMER;
	sev.sigev_value.sival_ptr = &epfd_timer;
	if (timer_create(CLOCK_REALTIME, &sev, &epfd_timer) == -1)
		printf("timer_create error\n");
	else
		fprintf(log_file,"I succesfully created epfd_timer\n");
	#endif
		
	#if defined(PFD) || defined(tPFD)
		sev.sigev_notify = SIGEV_SIGNAL;
		sev.sigev_signo = SIG_PFD_TIMER;
		sev.sigev_value.sival_ptr = &pfd_timer;
		if (timer_create(CLOCK_REALTIME, &sev, &pfd_timer) == -1)
			printf("timer_create error\n");
		else
			fprintf(log_file,"I succesfully created pfd_timer\n");
	#endif
	 cur_time = time(NULL);
	 cur_t = localtime(&cur_time);
	 fprintf(log_file, "[%d:%d:%d]: Initialized node_id=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,node_id);
	 
	 #ifdef PLAT_SCC
	 RCCE_barrier(&RCCE_COMM_WORLD);
	 #else
	 sleep(1);
	 #endif
	 for (tmp_cores_list = my_cores->next; tmp_cores_list != NULL; tmp_cores_list = tmp_cores_list->next) {
		  one_core = tmp_cores_list->core_id;
		  if (core_inter_head[one_core] == NULL){
			 core_inter_head[one_core] = (inter_list *) malloc(sizeof(inter_list));
			 core_inter_tail[one_core] = core_inter_head[one_core];
		  } else {
			 core_inter_tail[one_core]->next = (inter_list *) malloc(sizeof(inter_list));
			 core_inter_tail[one_core] = core_inter_tail[one_core]->next;
		  }
		  core_inter_tail[one_core]->type = INIT_CORE;
		  core_inter_tail[one_core]->next = NULL;
		  scc_kill(one_core, SIG_INIT, core_inter_head[one_core]);
	 }
	 
	 while (nodes_initialised != my_cores_count-1) {
		 scc_pause();
		 scc_signals_check();
	 }
	 
	 #ifdef PLAT_SCC
	 RCCE_barrier(&RCCE_COMM_WORLD);
	 #else
	 sleep(1);
	 #endif

	 printf("End of initialisation\n");
	 //print_grid();
	 /* Open the Time Log File*/
#ifdef PLAT_SCC
	 strcpy(time_log_file_name, "/shared/herc/");
#else
	 strcpy(time_log_file_name, "../");
#endif
	 strcat(time_log_file_name,scen_directory);
	 strcat(time_log_file_name ,"/");
	 strcat(time_log_file_name ,scen_num);	 
	 strcat(time_log_file_name, "/times_log.txt");
	 printf("Time Log File Path = %s ... ",time_log_file_name);
	 fflush(stdout);
	 fprintf(log_file,"time log file path = %s\n",time_log_file_name);

	 if ((time_log = fopen(time_log_file_name, "w")) == NULL){
		  printf("Error!\n");
		  fflush(stdout);
		  perror("open time_log");
	 }else{
		  printf("Success!\n");
		  fflush(stdout);  
	 }
	 /* done */

	 /* Init app file is kept in linux for compatibility of result gathering scripts */
#ifdef PLAT_SCC
	strcpy(init_ack_file_name, "/shared/herc/");
#else
	strcpy(init_ack_file_name, "../");
#endif
	strcat(init_ack_file_name, scen_directory);
	strcat(init_ack_file_name, "/");
	strcat(init_ack_file_name, scen_num);
	strcat(init_ack_file_name, "/init_ack.txt");
	//printf("file path = %s\n",app_input_file_name);

	if ((init_ack_file = fopen(init_ack_file_name, "w")) == NULL){
		printf("Cannot open input file with file path = %s ",init_ack_file_name);
		perror("open app_input");
	}
	  
	 /* Open the Application Input File*/
#ifdef PLAT_SCC
	 strcpy(app_input_file_name, "/shared/herc/");
#else
	 strcpy(app_input_file_name, "../");
#endif
	 strcat(app_input_file_name, scen_directory);
	 strcat(app_input_file_name, "/");
	 strcat(app_input_file_name, scen_num);
	 strcat(app_input_file_name, "/inputs/");
	 strcat(app_input_file_name, app_input_file);
	 printf("App Input File Path = %s ...",app_input_file_name);
	 fprintf(log_file,"file path = %s\n",app_input_file_name);
	 
	 if ((app_input = fopen(app_input_file_name, "r")) == NULL){
		  printf("Error!\n");
		  perror("open app_input");
	 }else{
		  printf("Success!\n");
		  fflush(stdout);
	 }
	 /* done */
	 
	 fscanf(app_input,"%d",&time_next);
	 
	 state = IDLE_CHK_APP_FILE;
	 its.it_interval.tv_sec = 0;
	 its.it_interval.tv_nsec = 0;
	 its.it_value.tv_sec = 0;
	 its.it_value.tv_nsec = 10 * MS;
	 if (timer_settime(timerid, 0, &its, NULL) == -1) perror("timer_settime error9");
	  
	 gettimeofday(&time_val, NULL);
	 cur_t = localtime(&time_val.tv_sec);
	 start_time_of_day = *cur_t;
	 
	 while (state != IDAG_ENDING)
	   
		if (state == IDLE_IDAG) {
			scc_pause();
			scc_signals_check();
			if (num_apps_terminated == num_apps) {
				state = USER_INPUT;
				fprintf(log_file,"All apps terminated. Switching to USER_INPUT\n");
			} else if (!timer_init_null && init_pending_head != NULL) {
				its.it_value.tv_sec = 10;
				its.it_value.tv_nsec = 0;
				if (timer_settime(timerid, 0, &its, NULL) == -1) perror("timer_settime error9");
				timer_init_null = 1;
			}
		} else if (state == IDLE_IDAG_INIT_SEND) {
			signals_enable();
			tmp_inter_prev = NULL;
			tmp_pending_head = init_pending_head;
			while (tmp_pending_head != NULL) {
			      init_core = tmp_pending_head->data.new_app.num_of_cores;
				for (tmp_inter_list = core_inter_head[init_core]; tmp_inter_list != NULL; tmp_inter_list = tmp_inter_list->next)
					if (tmp_inter_list->type == INIT_APP) break;

				if (tmp_inter_list == NULL) {
					fprintf(log_file,"I am sending an aborted init_app to %d with id %d\n",init_core,tmp_pending_head->data.new_app.id);

					if (core_inter_head[init_core] == NULL){
						core_inter_head[init_core] = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[init_core] = core_inter_head[init_core];
					} else {
						core_inter_tail[init_core]->next = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[init_core] = core_inter_tail[init_core]->next;
					}
					core_inter_tail[init_core]->type = INIT_APP;
					core_inter_tail[init_core]->data.new_app = tmp_pending_head->data.new_app;
					core_inter_tail[init_core]->data.new_app.num_of_cores = 0;
					core_inter_tail[init_core]->next = NULL;

					if (core_inter_head[init_core]->next == NULL) {
						//kill(pid_num[sender_id],SIG_INIT_APP);
						scc_kill(init_core, SIG_INIT_APP, core_inter_head[init_core]);
						my_stats.msg_count++;
						my_stats.distance += distance(node_id,init_core);
					}

					if (tmp_inter_prev == NULL) {
						tmp_inter_list = init_pending_head;
						init_pending_head = init_pending_head->next;
						tmp_pending_head = init_pending_head;
						//free(tmp_inter_list);
					} else {
						tmp_inter_list = tmp_inter_prev->next;
						tmp_inter_prev->next = tmp_inter_list->next;
						tmp_pending_head = tmp_inter_prev->next;
						if (tmp_inter_prev->next == NULL) init_pending_tail = tmp_inter_prev;
					}

					free(tmp_inter_list);
				} else {
					tmp_inter_prev = tmp_pending_head;
					tmp_pending_head = tmp_pending_head->next;		
				}
			}
			timer_init_null = 0;
			state = IDLE_IDAG;
			fprintf(log_file,"I exit this\n");
		} else if (state == IDLE_CHK_APP_FILE) {
			scc_pause();
			scc_signals_check();
		} else if (state == CHK_APP_FILE) {
			signals_disable();
			
			time_passed++;
			//if (time_for_farman > 0) time_for_farman -= 10;a
			//printf("time passed=%d | time next=%d\n",time_passed,time_next);
			if (time_next == time_passed) {
				fscanf(app_input,"%d",&init_core);
				num_apps++;

				fprintf(log_file,"idag_mask[%d] = %d\n",init_core,idag_mask[init_core]);
				if (idag_mask[init_core] == init_core){
					fprintf(log_file,"init_core %d is a controller. New init_core = %d\n",init_core,init_core+1);
					printf("init_core %d is a controller. New init_core = %d\n",init_core,init_core+1);
					init_core++;
				}
				printf("time = %d, id = %d\n",time_passed,app_cnt);
				gettimeofday(&time_val, NULL);
				cur_t = localtime(&time_val.tv_sec);
					
				fprintf(log_file, "[%d:%d:%d]: Initialising app_id=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,app_cnt);
				fprintf(time_log, "[%d:%d:%d:%ld] app_id=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,time_val.tv_usec,app_cnt);
				fflush(time_log);

				for (tmp_inter_list = core_inter_head[init_core]; tmp_inter_list != NULL; tmp_inter_list = tmp_inter_list->next)
					if (tmp_inter_list->type == INIT_APP) break;
				signals_enable();
				if (tmp_inter_list == NULL) {
					if (core_inter_head[init_core] == NULL){
						core_inter_head[init_core] = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[init_core] = core_inter_head[init_core];
					} else {
						core_inter_tail[init_core]->next = (inter_list *) malloc(sizeof(inter_list));
						core_inter_tail[init_core] = core_inter_tail[init_core]->next;
					}

					core_inter_tail[init_core]->type = INIT_APP;
#ifndef ARTIFICIAL_APPS_SIM
					/* FIXME scanned for old app inputs compatibility reasons but it is discarded */
					fscanf(app_input,"%d",&core_inter_tail[init_core]->data.new_app.array_size);
					core_inter_tail[init_core]->data.new_app.array_size = executed_app_array_size;
#else
					fscanf(app_input,"%f",&core_inter_tail[init_core]->data.new_app.var);
					fscanf(app_input,"%f",&core_inter_tail[init_core]->data.new_app.A);
#endif					
					fscanf(app_input,"%d",&core_inter_tail[init_core]->data.new_app.workld);
					core_inter_tail[init_core]->data.new_app.app_type = executed_app_type_number;
					/* FIXME for the time being, do not scan for app type. All apps are of the same type, read from argv */
							
					core_inter_tail[init_core]->data.new_app.id = app_cnt++;
					core_inter_tail[init_core]->data.new_app.num_of_cores = 0;
					core_inter_tail[init_core]->next = NULL;
			
					/*printf("time = %d, id = %d, workld = %d\n",time_passed,core_inter_tail[init_core]->data.new_app.id,core_inter_tail[init_core]->data.new_app.workld);
					gettimeofday(&time_val, NULL);
					cur_t = localtime(&time_val.tv_sec);
					
					fprintf(log_file, "[%d:%d:%d]: Initialising app_id=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,core_inter_tail[init_core]->data.new_app.id);
					fflush(log_file);
					fprintf(time_log, "[%d:%d:%d:%ld] app_id=%d\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec,time_val.tv_usec,core_inter_tail[init_core]->data.new_app.id);
					fflush(time_log);*/
					
					if (core_inter_head[init_core]->next == NULL) {
						//kill(pid_num[init_core],SIG_INIT_APP);
						scc_kill(init_core, SIG_INIT_APP, core_inter_head[init_core]); 
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
					/* FIXME scanned for old app inputs compatibility reasons but it is discarded */
#ifndef ARTIFICIAL_APPS_SIM
					fscanf(app_input,"%d",&init_pending_tail->data.new_app.array_size);
					init_pending_tail->data.new_app.array_size = executed_app_array_size;
#else
					fscanf(app_input,"%f",&init_pending_tail->data.new_app.var);
					fscanf(app_input,"%f",&init_pending_tail->data.new_app.A);
#endif
					fscanf(app_input,"%d",&init_pending_tail->data.new_app.workld);
					core_inter_tail[init_core]->data.new_app.app_type = executed_app_type_number;
					/* FIXME for the time being, do not scan for app type. All apps are of the same type, read from argv */
					
					init_pending_tail->data.new_app.id = app_cnt++;
					init_pending_tail->data.new_app.num_of_cores = init_core;
					init_pending_tail->next = NULL;
				}


				if (fscanf(app_input,"%d",&time_next) == EOF) {
					//state = USER_INPUT;
					state = IDLE_IDAG;
					time_passed = -1;
				} else {
					its.it_value.tv_sec = 0;
					its.it_value.tv_nsec = 10 * MS;
					if (timer_settime(timerid, 0, &its, NULL) == -1) printf("timer_settime error11\n");
					state = IDLE_CHK_APP_FILE;
				}
			} else {
				state = IDLE_CHK_APP_FILE;
				its.it_value.tv_sec = 0;
				its.it_value.tv_nsec = 10 * MS;
				if (timer_settime(timerid, 0, &its, NULL) == -1) printf("timer_settime error92\n");
			}

			signals_enable();
		} else if (state == USER_INPUT) {
			//while (num_apps_terminated != num_apps) {//pause(); my_cores_count
			//	scc_pause();
			//	scc_signals_check();
			//}

#ifndef IDAG_SLEEP
			for (k=0; k<15000; k++) {
#else
			for (k=0; k<20; k++) {
#endif
				scc_pause();
				scc_signals_check();
			}

			for (j=0; j<num_idags; j++) {
				one_idag = idag_id_arr[j];
				
				fprintf(log_file, "Sending to controller core %d\n",one_idag);

				if (one_idag != idag_id_arr[0]) {
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
					if (core_inter_head[one_idag]->next == NULL) 
						scc_kill(one_idag, SIG_REQ_DDS, core_inter_head[one_idag]);
					else {
						fprintf(log_file,"interaction in debug req dds is %d\n",core_inter_head[one_idag]->type);
					}
				} else {
					printf("Number of agents in region = %d\n",DDS_count);
					tmp_DDS = DDS;
					i=0;
					while (tmp_DDS != NULL) {
						printf("Agent no %d is %d with %d cores\n",i,tmp_DDS->agent_id,tmp_DDS->num_of_cores);
						tmp_DDS = tmp_DDS->next;
						i++;
					}
				}
			}

			while (idags_replied < num_idags - 1) {
				scc_pause();
				scc_signals_check();
			}		

			fprintf(log_file,"killing\n");
			for (i=1; i<num_idags; i++){
				printf("i am killing %d\n",idag_id_arr[i]);
				one_core = idag_id_arr[i];
			
				if (core_inter_head[one_core] == NULL){
					core_inter_head[one_core] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[one_core] = core_inter_head[one_core];
				} else {
					core_inter_tail[one_core]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[one_core] = core_inter_tail[one_core]->next;
					fprintf(log_file,"I am still doing smth with idag %d interaction = %d\n",one_core,core_inter_head[one_core]->type);
				}

				core_inter_tail[one_core]->type = TERMINATION_STATS;
				core_inter_tail[one_core]->next = NULL;
				signals_disable();
				scc_kill(one_core, SIG_TERMINATE, core_inter_head[one_core]);
				signals_enable();
			}

			tmp_cores_list = my_cores;
			my_cores = my_cores->next;
			free(tmp_cores_list);
			inter_list tmp_inter_list;
			for (; my_cores != NULL; my_cores = my_cores->next) {
				tmp_cores_list = my_cores;

				one_core = my_cores->core_id;
				if (core_inter_head[one_core] == NULL){
					core_inter_head[one_core] = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[one_core] = core_inter_head[one_core];
				} else {
					core_inter_tail[one_core]->next = (inter_list *) malloc(sizeof(inter_list));
					core_inter_tail[one_core] = core_inter_tail[one_core]->next;
					fprintf(log_file,"I am still doing smth with my node %d interaction = %d\n",one_core,core_inter_head[one_core]->type);
				}

				core_inter_tail[one_core]->type = TERMINATION_STATS;
				core_inter_tail[one_core]->next = NULL;	
				//kill(pid_num[one_core], SIG_TERMINATE);
				signals_disable();
				scc_kill(one_core, SIG_TERMINATE, core_inter_head[one_core]);
				signals_enable();
				
				/* 8.7.2016 Paxos Stats */
				tmp_inter_list.next = NULL;
				tmp_inter_list.type = PAXOS_STATS_REQ;
				scc_kill(one_core, SIG_PAXOS_STATS_REQ, &tmp_inter_list);
				paxos_node_stats.msg_count++;
				paxos_node_stats.distance += distance(node_id,one_core);
				
				my_stats.msg_count++;
				my_stats.distance += distance(node_id,one_core);

				free(tmp_cores_list);
			}
	
		state = IDAG_ENDING;
	 } else {
		  printf("my_rtrm.c : Unknown state node_id = %d state = %d\n",node_id,state);
		  state = IDLE_IDAG;
	 }

	 while (state == IDAG_ENDING) {
		  scc_pause();
		  scc_signals_check();
		  if (stats_replied == my_cores_count+num_idags-2 && paxos_stats_replied == my_cores_count+num_idags-2) state = TERMINATED;
		  
	 }
	 
	 #ifdef PLAT_LINUX
	 for (i=0; i<my_cores_count-1; i++) 
		  wait(NULL); //wait for children
	 for (i=0; i<num_idags-1; i++) 
		  wait(NULL);//wait for the other idags
	 #endif	
		
			 
	 total_stats.msg_count += my_stats.msg_count;
	 total_stats.message_size += my_stats.message_size;
	 total_stats.distance += my_stats.distance; 
	 total_stats.app_turnaround += my_stats.app_turnaround;
	 total_stats.comp_effort += my_stats.comp_effort;
	 total_stats.cores_utilized += my_stats.cores_utilized;
	 total_stats.times_accessed += my_stats.times_accessed;
	 
	 paxos_total_stats.msg_count += paxos_node_stats.msg_count;
	 paxos_total_stats.fd_msg_count += paxos_node_stats.fd_msg_count;

	 avg_cluster_util = (float) my_stats.cores_utilized / (my_stats.times_accessed * (my_cores_count-1));
	 printf("I am %d with cores_utilized = %d times_accessed = %d my_cores_count = %d and avg_cluster_util = %0.2f\n",
		  node_id,my_stats.cores_utilized,my_stats.times_accessed,my_cores_count,avg_cluster_util);
	 fprintf(log_file,"cores_utilized = %d times_accessed = %d my_cores_count = %d and avg_cluster_util = %0.2f\n",
		  my_stats.cores_utilized,my_stats.times_accessed,my_cores_count,avg_cluster_util);
	 
	 gettimeofday(&time_val, NULL);
	 end_time_of_day = localtime(&time_val.tv_sec);

	 fprintf(log_file,"================= DRTRM STATS ==================\n");
	 fprintf(log_file,"Total message count = %lld\n",total_stats.msg_count);
	 fprintf(log_file,"Total message size = %d\n",total_stats.message_size);
	 fprintf(log_file,"Total distance = %d\n",total_stats.distance);
	 fprintf(log_file,"Total app turnaround time = %d\n",total_stats.app_turnaround);
	 fprintf(log_file,"Total computational effort = %d\n",total_stats.comp_effort);
	 fprintf(log_file,"Total cores_utilized = %d\n",total_stats.cores_utilized); 
	 fprintf(log_file,"Total times_accessed = %d\n",total_stats.times_accessed);
	 fprintf(log_file,"================= PAXOS STATS ==================\n");
	 fprintf(log_file,"Total message count = %lld\n",paxos_total_stats.msg_count);

	 printf("================= DRTRM STATS ==================\n");
	 printf("Total message count = %lld\n",total_stats.msg_count);
	 printf("Total message size = %d\n",total_stats.message_size);
	 printf("Total distance = %d\n",total_stats.distance);
	 printf("Total app turnaround time = %d\n",total_stats.app_turnaround);
	 printf("Total computational effort = %d\n",total_stats.comp_effort);
	 printf("Total cores_utilized = %d\n",total_stats.cores_utilized); 
	 printf("Total times_accessed = %d\n",total_stats.times_accessed);
	 printf("================= PAXOS STATS ==================\n");
	 printf("Start Time: [%d:%d:%d]\n",start_time_of_day.tm_hour,start_time_of_day.tm_min,start_time_of_day.tm_sec);
	 printf("End Time: [%d:%d:%d]\n",end_time_of_day->tm_hour,end_time_of_day->tm_min,end_time_of_day->tm_sec);
#ifdef PFD
	 printf("Perfect Failure Detector\n");
#elif tPFD
	 printf("tweaked Perfect Failure Detector\n");
#endif
	 printf("PAXOS Total message count = %lld\n",paxos_total_stats.msg_count);\
	 printf("Failure Detection Total message count = %d\n",paxos_total_stats.fd_msg_count);
	 
	 
	 
	 for (i=0; i<NUES; i++){
		  free(core_inter_head[i]);
		  free(core_inter_tail[i]);
	 }
	 
	 free(core_inter_head);
	 free(core_inter_tail);
	 
	 #ifdef PLAT_SCC
	 RCCE_flag_free(&flag_data_written);
	 RCCE_free((t_vcharp) sig_array);
	 RCCE_free((t_vcharp) data_array);
	 RCCE_free((t_vcharp) proposal_number_global);
	 
	 fclose(init_ack_file);
	 #else
	 shmdt(pid_num);
			 
	 for (i=0; i<NUES; i++) {
		  sem_destroy(&flag_data_written[i]);
		  sem_destroy(&scc_lock[i]);
	 }	
		  
	 shmdt(flag_data_written);
	 shmdt(scc_lock);
	 shmdt(manager_result_out);
	 shmdt(index_bottom);
	 #endif

	 cur_time = time(NULL);	
	 cur_t = localtime(&cur_time);
	 fprintf(log_file, "[%d:%d:%d]: I ended well\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);
	 fclose(log_file);
	 printf("[%d:%d:%d]: I ended well\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);
  //}
  }
  
  return 0;
}

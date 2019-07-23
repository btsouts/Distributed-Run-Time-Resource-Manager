#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#define OLD_INIT_AREAS_NUM 8

#include <time.h>
#include <sys/time.h>


typedef enum available_core_states {
	IDLE_CORE, 		//0
	WORKING_NODE, 		//1
	TERMINATED, 		//2
	/* Controller States */
	IDLE_IDAG, 		//3
	IDLE_IDAG_INIT_SEND, 	//4
	IDLE_CHK_APP_FILE, 	//5
	CHK_APP_FILE, 		//6
	USER_INPUT, 		//7 //FIXME change to a reasonable name
	/* Initial core States */
	INIT_MANAGER, 		//8
	INIT_MANAGER_SEND_OFFERS,//9
	IDLE_INIT_MAN, 		//10
	INIT_MAN_CHK_OFFERS, 	//11
	/* Manager States */
	IDLE_AGENT, 		//12
	IDLE_AGENT_WAITING_OFF, //13
	AGENT_INIT_STATE, 	//14
	AGENT_SELF_OPT, 	//15
	AGENT_SELF_CHK_OFFERS, 	//16
	AGENT_ENDING, 		//17
	IDAG_ENDING, 		//18
	NO_PENDING_STATE, 	//19
	AGENT_ZOMBIE, 		//20
	/* Multiple Pending States */
	AGENT_INIT_APP_INIT, 	//21
	AGENT_INIT_CHK_OFFERS, 	//22
	AGENT_INIT_IDLE_INIT, 	//23
	IDLE_INIT_IDLE_AGENT, 	//24
	IDLE_INIT_AGENT_SELFOPT,//25
	INIT_CHK_OFFERS_IDLE_AGENT, //26
	INIT_CHK_OFFERS_SELFOPT,//27
	/*PAXOS STATES*/
	PAXOS_ACTIVE, 		//28
	NEW_IDAG, 		//29
	NEW_AGENT, 		//30
	FAILED_CORE		//31
} core_states;//avail_states;

typedef enum available_app_states {
	NO_APP, 		//0
	APP_TERMINATED, 	//1
	RUNNING, 		//2
	RESIZING, 		//3
} application_states;//avail_states;

typedef enum available_exec_apps{
	MATRIX_MUL,
	SVM,
	FFT,
	ARTIFICIAL
} app_exec;

typedef enum interaction_types {
	/* General Interaction Types */
	INIT_CORE, 		//0
	REMOVE_APP, 		//1
	/* 0 <--> Initial */
	INIT_APP, 		//2
	DECLARE_INIT_AVAILABILITY,//3
	/* 0 --> Controller */
	DEBUG_IDAG_REQ_DDS, 	//4 FIXME Find better name
	/* Initial <--> Controller Related Interaction Types */
	IDAG_FIND_IDAGS_PENDING, //5
	IDAG_FIND_IDAGS,	//6
	IDAG_REQ_DDS_PENDING, 	//7
	IDAG_REQ_DDS, 		//8
	REP_IDAG_FIND_IDAGS, 	//9
	REP_IDAG_REQ_DDS, 	//10
	/* Initial --> Manager OR Controller */
	AGENT_REQ_CORES, 	//11
	AGENT_REQ_CORES_PENDING,//12
	/* Manager <--> Manager OR Controller */
	REP_AGENT_REQ_CORES, 	//13
	AGENT_OFFER_SENT, 	//14
	REP_AGENT_OFFER_SENT, 	//15
	REP_AGENT_OFFER_PENDING,//16
	/* Manager --> Controller */
	SELFOPT_IDAG_FIND_IDAGS_PENDING, //17
	SELFOPT_IDAG_FIND_IDAGS,//18
	SELFOPT_IDAG_REQ_DDS_PENDING,//19
	SELFOPT_IDAG_REQ_DDS, 	//20
	SELFOPT_REQ_CORES_PENDING,//21
	SELFOPT_REQ_CORES, 	//22
	IDAG_ADD_CORES_DDS, 	//23
	IDAG_REM_CORES_DDS, 	//24
	/* Initial --> Manager */
	INIT_AGENT, 		//25
	/* Manager --> Worker */
	INIT_WORK_NODE_PENDING, //26
	APPOINT_WORK_NODE_PENDING,//27
	INIT_WORK_NODE, 	//28
	APPOINT_WORK_NODE, 	//29
	/* Controller <--> Cores */
	TERMINATION_STATS, 	//30
	REP_STATISTICS,		//31
	/* PAXOS INTERACTIONS */
	PAXOS_INIT, 		//32
	PREPARE_REQUEST, 	//33
	PREPARE_ACCEPT_NO_PREVIOUS,//34
	PREPARE_ACCEPT, 	//35
	ACCEPT_REQUEST, 	//36
	ACCEPTED, 		//37
	LEARN, 			//38
	LEARN_ACK, 		//39
	LEARN_ACK_CONTR, 	//40
	REINIT_APP, 		//41
	CONTR_TO, 		//42
	REMOVE_FROM_DDS, 	//43
	ADD_TO_DDS, 		//44
	HEARTBEAT_REQ, 		//45
	HEARTBEAT_REP, 		//46
	PAXOS_STATS_REQ, 	//47
	PAXOS_STATS_REP, 	//48
	RECOVERED		//49
	/* END */
} inter_types;

typedef struct app_tag app;

struct app_tag {
	int id;
	int workld;
	int num_of_cores;
	int app_type;
#ifndef ARTIFICIAL_APPS_SIM
	int array_size; // 0 = 1024, 1= 2048, 3= 4096
#else
	float var;	
	float A;
#endif
};
typedef struct region_tag region;

struct region_tag {
	int C;
	int r;
};
typedef struct offer_tag offer;

struct offer_tag {
	int num_of_cores;
	int *offered_cores;
	float spd_loss;
	//float spd_gain;
};

typedef struct offer_list_tag offer_list;

struct offer_list_tag {
	offer off;
	int sender;
	int *answer;
	offer_list *next;
};

typedef struct target_list_tag target_list;

struct target_list_tag {
	int target;
	int num_of_regions;
	region region_arr[OLD_INIT_AREAS_NUM];
	target_list *next;
};


struct region_array_tag {
	int num_of_regions;
	region *region_arr;
};

typedef struct region_array_tag region_array;

struct offer_array_tag {
	int num_of_offers;
	offer *offer_arr;
};

typedef struct offer_array_tag offer_array;

struct metrics_tag {
	long long int msg_count;
	int fd_msg_count;
	int message_size;
	int distance;
	int app_turnaround;
	int comp_effort;
	int cores_utilized;
	int times_accessed;
};
typedef struct metrics_tag metrics;

/*
struct far_req_info_tag {
	int orig_sender;
	app far_app;
	region reg;
};
typedef struct far_req_info_tag far_req_info;
*/

struct my_time_stamp_tag {
	int tm_sec;  /* seconds */
	int tm_min;  /* minutes */
	int tm_hour; /* hours */
	suseconds_t tm_usec;
};
typedef struct my_time_stamp_tag my_time_stamp;

struct init_app_info_tag {
	app new_app;
	my_time_stamp new_app_times[2];
	int *new_app_cores;
};

typedef struct init_app_info_tag init_app_info;

typedef struct core_list_tag core_list;

struct core_list_tag {
	int core_id;
	int offered_to; //-1 if offered to nobody
	int workload[2]; /* 27.6.2016 Added by dimos. If worker fails i have to know the workload given in order to reappoint */
	core_list *next;
};

typedef struct DDS_tag DDS_list;

struct DDS_tag {
	int agent_id;
	int num_of_cores;
	DDS_list *next;
#ifdef ADAM_SIM
	DDS_list *prev;
#endif
};

typedef struct coworkers_list_tag coworkers_list;

struct coworkers_list_tag{
	int core_id;
	int agent_id;
	coworkers_list *next;
};

typedef struct agent_info_tag agent_info;

struct agent_info_tag {
	int my_agent;
	int array_size;
	int work_bounds[2];
	int segment_id;
	int app_type;
};

typedef struct interact_list_tag inter_list;

struct interact_list_tag {
	//int core_id;
	inter_types type;
	union {
		region reg;
		region_array reg_arr;
		app new_app;
		init_app_info one_app;
		offer my_offer;
		offer_array off_arr;
		metrics stats;
		int *idags_in_reg;
		int *agents_in_reg;
		int *app_cores;//first element is num of cores
		int offer_accepted;
		int *offer_acc_array;
		//int far_req_man;
		//far_req_info far_req; 
		int work_bounds[3];
		int agent_ended;
		/* PAXOS */
		int proposal_number;
		int accepted_values[3]; // 0 is proposal_number, 1 is value, 2 is state
		int learn_ack_info[2];
		app reappointed_app;
		int controller_index;
		int failed_node;
		int workers_info[9];
		int paxos_stats[2];
	} data;
	inter_list *next;
};

typedef struct{
	int highest_acc_n; //Highest accepted proposal number
	int highest_acc_value; //Value paired with highest accepted proposal number
	int highest_proposed_n; //Highest proposed proposal number
}acceptor_var;

typedef struct{
	int highest_replied_n;
	int highest_replied_value;
	int cores_promised;
	int cores_accepted;
	int *core_states;
}proposer_var;

typedef struct{
	int accepted_value;
	int cores_accepted;
}learner_var;

#endif

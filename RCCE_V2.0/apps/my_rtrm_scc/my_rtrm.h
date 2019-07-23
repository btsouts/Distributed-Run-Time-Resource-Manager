#ifndef __MY_RTRM_H__
#define __MY_RTRM_H__

#define _GNU_SOURCE
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
//#include <semaphore.h>
#include "RCCE.h"

/*#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>*/
//#include <sys/sem.h>
//#include <sys/time.h>

#define X_max 6
#define Y_max 6
#define num_idags_x 2
#define num_idags_y 2
#define MS 1000000
#define LOCALITY_THRESH 10//the greatest distance in two adjascent clusters
#define OLD_INIT_AREAS_NUM 8
#define INIT_FAR_AREAS_NUM 4
#define SELFOPT_AREAS_NUM 4
#define SELFOPT_ROUNDS 3
#define INIT_NODE_INTERVAL 500//750
#define LINE_SIZE 8
#define SIG_BASE_NUM 42

#define NO_SIG 0
#define SIG_ACK SIG_BASE_NUM
#define SIG_INIT SIG_BASE_NUM+1
#define SIG_TERMINATE SIG_BASE_NUM+2
#define SIG_INIT_APP SIG_BASE_NUM+3
#define SIG_IDAG_FIND_IDAGS SIG_BASE_NUM+4
#define SIG_REQ_DDS SIG_BASE_NUM+5
#define SIG_REQ_CORES SIG_BASE_NUM+6
#define SIG_REP_OFFERS SIG_BASE_NUM+7
#define SIG_INIT_AGENT SIG_BASE_NUM+8
#define SIG_ADD_CORES_DDS SIG_BASE_NUM+9
#define SIG_REM_CORES_DDS SIG_BASE_NUM+10
#define SIG_INIT_FAR_REQ SIG_BASE_NUM+11
#define SIG_FAR_REQ SIG_BASE_NUM+12
#define SIG_APPOINT_WORK SIG_BASE_NUM+13
#define SIG_CHECK_REM_TIME SIG_BASE_NUM+14
#define SIG_FINISH SIG_BASE_NUM+15
#define SIG_REJECT SIG_BASE_NUM+16
#define SIG_REMOVE_FAR_MAN SIG_BASE_NUM+17
#define SIG_APP_TERMINATED SIG_BASE_NUM+18

#define SIG_TIMER SIGRTMIN

typedef enum available_core_states { 
	IDLE_CORE, //0
	IDLE_IDAG, //1
	IDLE_AGENT, //2
	TERMINATED, //3
	INIT_MANAGER, //4
	INIT_MANAGER_SEND_OFFERS, //5
	IDLE_INIT_MAN, //6
	INIT_MAN_CHK_OFFERS, //7
	LC_REQ_MANAGER, //8
	FAR_REQ_MANAGER, //9
	EXTRA_STATE, //10
	TEST_STATE, //11
	AGENT_INIT_STATE, //12
	WORKING_NODE, //13
	AGENT_SELF_OPT, //14
	AGENT_SELF_CHK_OFFERS, //15
	IDLE_AGENT_WAITING_OFF, //16
	IDLE_FAR_MAN, //17
	FAR_MAN_CHK_OFFERS, //18
	IDLE_CHK_APP_FILE, //19
	CHK_APP_FILE, //20
	USER_INPUT, //21
	AGENT_ENDING, //22
	AGENT_INIT_STATE_INIT_INTERRUPTED, //23
	IDLE_INIT_MAN_SELFOPT_PENDING, //24
	INIT_MAN_CHK_OFFERS_SELFOPT_PENDING, //25
	INIT_MANAGER_SEND_OFFERS_SELFOPT_PENDING, //26
	IDLE_INIT_MAN_WORK_PENDING, //27
	INIT_MAN_CHK_OFFERS_WORK_PENDING, //28
	INIT_MANAGER_SEND_OFFERS_WORK_PENDING, //29
	IDAG_ENDING, //30
	NO_PENDING_STATE, //31
	WORKING_NODE_IDLE_INIT, //32
	AGENT_ZOMBIE,
	AGENT_REWIND_FILE			 
} core_states;//avail_states;

typedef enum available_app_states {
	NO_APP, //0
	APP_TERMINATED, //1
	RUNNING, //2
	RESIZING, //3
} application_states;//avail_states;

typedef enum interaction_types { 
	INIT_CORE, 
	INIT_APP, //1
	IDAG_FIND_IDAGS,//2 
	REP_IDAG_FIND_IDAGS, //3
	IDAG_REQ_DDS, //4
	REP_IDAG_REQ_DDS, //5
	AGENT_REQ_CORES, //6
	REP_AGENT_REQ_CORES, //7
	AGENT_OFFER_SENT, //8
	REP_AGENT_OFFER_SENT, //9 
	INIT_AGENT, //10
	IDAG_ADD_CORES_DDS, //11
	IDAG_REM_CORES_DDS, //12
	DEBUG_IDAG_REQ_DDS, //13
	FAR_INIT_REQ, //14
	FAR_REQ_MAN, //15
	FAR_REQ_OFFER_SENT, 
	FAR_REQ_OFFER, //17
	REP_FAR_REQ_OFFER_SENT, //18
	FAR_INIT_IDAG_FIND_IDAGS, //19
	REP_FAR_INIT_REQ, //20
	FAR_INIT_IDAG_REQ_DDS, //21
	FAR_REQ_IDAG_FIND_IDAGS, //22
	FAR_REQ_IDAG_REQ_DDS, //23
	FAR_REQ_CORES, //24
	FAR_REQ_MAN_APPOINT, //25
	REP_FAR_REQ_MAN, //26
	REP_AGENT_OFFER_PENDING, //27
	REP_FAR_REQ_OFFER_PENDING, //28
	INIT_WORK_NODE, //29
	APPOINT_WORK_NODE, //30
	CHK_REM_TIME, //31
	REP_CHK_REM_TIME, //32
	SELFOPT_IDAG_FIND_IDAGS, //33
	SELFOPT_IDAG_REQ_DDS, //34
	SELFOPT_REQ_CORES, //35
	REMOVE_APP, //36
	AGENT_REQ_CORES_PENDING, //37
	SELFOPT_REQ_CORES_PENDING, //38
	TERMINATION_STATS, //39
	REP_STATISTICS,	//40
	IDAG_REQ_DDS_PENDING, //41
	FAR_REQ_IDAG_REQ_DDS_PENDING, //42
	SELFOPT_IDAG_REQ_DDS_PENDING, //43
	IDAG_FIND_IDAGS_PENDING, //44
	SELFOPT_IDAG_FIND_IDAGS_PENDING, //45	
	ABORT_FAR_MAN, //46
	FAR_REQ_CORES_PENDING, //47
	APPOINT_WORK_NODE_PENDING, //48
	INIT_WORK_NODE_PENDING, //49
	REMOVED_NODE_REM_TIME,
	FAR_REQ_MAN_APPOINT_PENDING,
	NOTIFY_APP_TERMINATION 
} inter_types;

typedef struct app_tag app;

struct app_tag {
	int id;	
	float A;
	float var;
	float workld;	
	int num_of_cores;
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
	int	target;
	int num_of_regions;
	region region_arr[OLD_INIT_AREAS_NUM];
	target_list *next;
};

typedef struct interact_list_tag inter_list;

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
	int msg_count;
	int message_size;
	int distance;
	int app_turnaround;
	int comp_effort;
	int cores_utilized;
	int times_accessed;
};

typedef struct metrics_tag metrics;

struct far_req_info_tag {
	int orig_sender;
	app far_app;
	region reg;
};

typedef struct far_req_info_tag far_req_info;

struct interact_list_tag {
	//int core_id;
	inter_types type;
	union {
		region reg;
		region_array reg_arr;
		app new_app;
		offer my_offer;
		offer_array off_arr;
		metrics stats;
		int *idags_in_reg;
		int *agents_in_reg;
		int *app_cores;//first element is num of cores
		int offer_accepted;
		int *offer_acc_array;
		int far_req_man;
		far_req_info far_req; 
		int work_time;//either to execute or remaining
		int agent_ended;
	} data;
	inter_list *next;
};

typedef struct DDS_tag DDS_list;

struct DDS_tag {
	int agent_id;
	int num_of_cores;	
	DDS_list *next;
};

typedef struct core_list_tag core_list;

struct core_list_tag {
	int core_id;
	int offered_to; //-1 if offered to nobody	
	core_list *next;
};

typedef struct my_time_stamp_tag my_time_stamp;

struct my_time_stamp_tag {
	int tm_sec;  /* seconds */
  int tm_min;  /* minutes */
  int tm_hour; /* hours */
};

//int X_max, Y_max, num_idags_x, num_idags_y;
float Speedup(app cur_app, int num_of_cores);
int offer_cores(core_list *cores, app req_app, region req_reg, int *Offered_cores, int req_id);
void send_next_signal(inter_list *head, int node_num);
int get_max_cores_count(app cur_app);
#endif

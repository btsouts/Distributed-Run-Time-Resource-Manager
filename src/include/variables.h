#ifndef __VARIABLES_H__
#define __VARIABLES_H__

#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>

#ifdef PLAT_SCC
#include "RCCE.h"
#define X_max 6
#define Y_max 8
#define MAX_SIGNAL_LIST_LEN 128
#endif

#include "structs.h"

/* Printing Material */
#define KNRM  "\x1B[0m"
#define KBLK  "\x1B[30m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
/*********************/

/* Static Variables */
#define MS 1000000
#define INIT_NODE_INTERVAL 1000
#define LINE_SIZE 8
#define SIG_BASE_NUM 42
#define NUM_OF_MATRICES 3
#define MAX_WORKERS_COUNT 8 //FIXME
#define MAX_ARRAY_SIZE 4096
#define MAX_DATA_LIST_LEN 3
#define LEAST_SELF_OPT_INTERVAL_MS 500
#define MAX_SELF_OPT_INTERVAL_MS 12800
#define MAX_STR_NAME_SIZE 256
#define SCEN_NUM_SIZE MAX_STR_NAME_SIZE
#define SCEN_DIR_SIZE MAX_STR_NAME_SIZE
#define PAXOS_SCEN_SIZE 32
/**************************/

/* Signals */
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
#define SIG_APPOINT_WORK SIG_BASE_NUM+11
#define SIG_FINISH SIG_BASE_NUM+12
#define SIG_REJECT SIG_BASE_NUM+13
#define SIG_APP_TERMINATED SIG_BASE_NUM+14
//PAXOS_SIGNALS
#define SIG_PREPARE_REQUEST SIG_BASE_NUM+15
#define SIG_PREPARE_ACCEPT_NO_PREVIOUS SIG_BASE_NUM+16
#define SIG_PREPARE_ACCEPT SIG_BASE_NUM+17
#define SIG_ACCEPT_REQUEST SIG_BASE_NUM+18
#define SIG_ACCEPTED SIG_BASE_NUM+19
#define SIG_LEARN SIG_BASE_NUM+20
#define SIG_LEARN_ACK SIG_BASE_NUM+21
#define SIG_LEARN_ACK_CONTR SIG_BASE_NUM+22
#define SIG_REINIT_APP SIG_BASE_NUM+23
#define SIG_CONTR_TO SIG_BASE_NUM+24
#define SIG_REMOVE_FROM_DDS SIG_BASE_NUM+25
#define SIG_ADD_TO_DDS SIG_BASE_NUM+26
#define SIG_HEARTBEAT_REQ SIG_BASE_NUM+27
#define SIG_HEARTBEAT_REP SIG_BASE_NUM+28
#define SIG_FAIL SIG_BASE_NUM+29
#define SIG_PAXOS_STATS_REQ SIG_BASE_NUM+30
#define SIG_PAXOS_STATS_REP SIG_BASE_NUM+31

#define SIG_RECOVER SIG_BASE_NUM+32



#define SIG_TIMER SIGRTMIN
#define SIG_CTIMER SIGRTMIN+1
#define SIG_EPFD_TIMER SIGRTMIN+2
#define SIG_ITIMER SIGRTMIN+3
#define SIG_PFD_TIMER SIGRTMIN+4
/*****************************************/

#ifdef SCC
extern int idag_mask[X_max*Y_max];
extern int low_voltage_core[X_max*Y_max];
extern int timer_schedule[X_max*Y_max];

extern volatile int *manager_result_out;
extern volatile int *index_bottom;

extern RCCE_FLAG flag_data_written;
extern RCCE_FLAG proposal_number_lock;

extern int num_idags_x;

#else
extern int X_max, Y_max;
extern int *pid_num;
extern int *idag_mask;
extern int *low_voltage_core;
extern int *timer_schedule;
extern int *manager_result_out;
extern int *index_bottom;
extern int num_idags_x;
extern sem_t *flag_data_written, *scc_lock, *proposal_number_lock;
#endif

extern int node_id;
extern int my_idag;
extern int num_idags;
extern int DDS_count;
extern int my_cores_count;
extern int nodes_ended_cnt;
extern int nodes_initialised;
extern int stats_replied;
extern int paxos_stats_replied;
extern int num_apps_terminated;
extern int num_apps;
extern int idags_replied;
extern int index_top;
extern int last_index_bottom;
extern int NUES;
extern int selfopt_interval;
extern int init_DDS_replies;
extern int init_DDS_idags;
extern int selfopt_DDS_replies;
extern int selfopt_DDS_idags;
extern int base_offset;
extern int old_cores_cnt;
extern int active_working_cores;
extern int delay;
extern int R;

extern int *sig_array;
extern int *data_array;
extern int *idag_id_arr;
extern int *proposal_number_global;
extern int sig_read_ar[2 * LINE_SIZE];

extern long int selfopt_time_rem;

extern metrics my_stats;
extern metrics total_stats;
extern metrics paxos_node_stats; 
extern metrics paxos_total_stats;

extern FILE *log_file;
extern FILE *init_ack_file;
extern FILE *app_log_file;

extern float old_Speedup;
extern float my_Speedup;

/* SVM */

extern agent_info cur_agent;
extern agent_info pending_agent;

extern core_states state;
extern core_states pending_state;

extern application_states app_state;

extern inter_list **core_inter_head;
extern inter_list **core_inter_tail;
extern inter_list *init_pending_head;
extern inter_list *init_pending_tail;

extern DDS_list *DDS;
extern DDS_list *DDS_tail;

extern core_list *my_cores;
extern core_list *my_cores_tail;

extern offer_list *init_man_offers;
extern offer_list *selfopt_man_offers;

extern target_list *init_targets_head;
extern target_list *init_targets_tail;
extern target_list *selfopt_targets_head;
extern target_list *selfopt_targets_tail;

extern app my_app;
extern app init_app;

extern time_t cur_time;
extern timer_t timerid, inter_timer;
extern struct tm *cur_t;
extern struct sigevent sev;
extern struct itimerspec its, chk_timer;
extern struct timeval time_val;

extern char scen_num[SCEN_NUM_SIZE];
extern char scen_directory[SCEN_DIR_SIZE];

extern my_time_stamp init_app_times[2];
extern my_time_stamp my_app_times[2];

extern app_exec executed_app;

/* PAXOS */
extern int first_time;
extern int worker_app_id;
extern int worker_flag;
extern int manager_to_fail;
extern int faulty_core;
extern int fail_flag;

extern int pending_workload[2];

extern int *alive;
extern int *suspected;


extern core_states paxos_state;

extern acceptor_var acceptor_vars;
extern proposer_var proposer_vars;

extern coworkers_list *coworkers;

extern timer_t controller_timer;
extern timer_t epfd_timer;
extern timer_t pfd_timer;

extern struct timeval fail_time_val;
#endif

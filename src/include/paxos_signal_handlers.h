#ifndef __PAXOS_SIGNAL_HANDLERS_H__
#define __PAXOS_SIGNAL_HANDLERS_H__

#include "variables.h"

#define PROP_NW 0
#define PROP_NR 2
#define PREV_CW 0
#define PREV_CR 2
#define VALUE_W 1
#define VALUE_R 3

/* PAXOS DECLARATIONS */
int leader_preference();
void initialize_PAXOS_data (char scen_directory[SCEN_DIR_SIZE], char scen_num[SCEN_NUM_SIZE]);
void rollback();
void find_app_info();
void sig_CTIMER_handler(int signo, siginfo_t *info, void *context);
void sig_NTIMER_handler(int signo, siginfo_t *info, void *context);
void sig_ITIMER_handler(int signo, siginfo_t *info, void *context);
void sig_PAXOS_INIT(int sender_id);
void sig_PREPARE_REQUEST_handler(int sender_id);
void sig_PREPARE_ACCEPT_NO_PREVIOUS_handler(int sender_id);
void sig_PREPARE_ACCEPT_handler(int sender_id);
void sig_ACCEPT_REQUEST_handler(int sender_id);
void sig_ACCEPTED_handler(int sender_id);
void sig_LEARN_handler(int sender_id);
void sig_LEARN_ACK_handler(int sender_id);
void sig_LEARN_ACK_CONTR_handler(int sender_id);
void sig_REINIT_APP_handler(int sender_id);
void sig_CONTR_TO_handler(int sender_id);
void sig_REMOVE_FROM_DDS_handler(int sender_id);
void sig_ADD_TO_DDS_handler(int sender_id, int *inc_cnt, int cur_index_top);
void sig_HEARTBEAT_REQ_handler(int sender_id);
void sig_HEARTBEAT_REP_handler(int sender_id);
void sig_PFD_TIMER_handler(int signo, siginfo_t *info, void *context);
void sig_EPFD_TIMER_handler(int signo, siginfo_t *info, void *context);
void sig_FAIL_handler();
void sig_PAXOS_STATS_REQ_handler(int sender_id);
void sig_PAXOS_STATS_REP_handler(int sender_id);
#endif

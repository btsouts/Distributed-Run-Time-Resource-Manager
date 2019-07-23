#ifndef __SIGNAL_HANDLERS_H__
#define __SIGNAL_HANDLERS_H__

#include "my_rtrm.h"

#ifdef PLAT_LINUX
void new_RCCE_get(int target[], int *src, int index, int num_of_ints, int ID);
void my_RCCE_get(int *target, int *src, int num_of_ints, int ID);
#endif
void sig_TIMER_handler(int signo, siginfo_t *info, void *context);
void sig_INIT_handler(int sender_id);
void sig_ACK_handler(int sender_id);
void sig_TERMINATE_handler(int sender_id);
void sig_INIT_APP_handler(int sender_id);
void sig_IDAG_FIND_IDAGS_handler(int sender_id, int *inc_cnt, int cur_index_top);
void sig_REQ_DDS_handler(int sender_id);
void sig_REQ_CORES_handler(int sender_id, int *inc_cnt, int cur_index_top);
void sig_REJECT_handler(int sender_id);
void sig_REP_OFFERS_handler(int sender_id);
void sig_INIT_AGENT_handler(int sender_id);
void sig_ADD_CORES_DDS_handler(int sender_id, int *inc_cnt, int cur_index_top);
void sig_REM_CORES_DDS_handler(int sender_id, int *inc_cnt, int cur_index_top);
void sig_APPOINT_WORK_handler(int sender_id, int *inc_cnt, int cur_index_top);
//void sig_APPOINT_WORK_handler(int sender_id);
void sig_CHECK_REM_TIME_handler(int sender_id);
void sig_FINISH_handler(int sender_id, int *inc_cnt, int cur_index_top);
/*void sig_SEGV_handler(int signo, siginfo_t *info, void *context);
void sig_FAR_REQ_handler(int signo, siginfo_t *info, void *context);
void sig_REMOVE_FAR_MAN_handler(int signo, siginfo_t *info, void *context);*/
void trigger_shit(int failed_core);
#endif

#ifndef __SCC_SIGNALS_H__
#define __SCC_SIGNALS_H__

#include "structs.h"

int scc_kill(int target_ID, int sig, inter_list *ref_inter_list);
void scc_signals_check(void);
void scc_pause(void);
int find_sender_id(int SID);
//void scc_update_slowest(struct timeval time_val, int agent_id);
#endif

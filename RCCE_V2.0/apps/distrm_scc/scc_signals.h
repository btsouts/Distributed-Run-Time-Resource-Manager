#ifndef __SCC_SIGNALS_H__
#define __SCC_SIGNALS_H__

#include "distrm.h"
#include "signal_handlers.h"
#include "RCCE.h"

int scc_kill(int target_ID, int sig);
void scc_signals_check(void);
#endif

#ifndef __SIG_AUX_H__
#define __SIG_AUX_H__

#include "my_rtrm.h"
#include "signal_handlers.h"
#include "paxos_signal_handlers.h"

void signals_disable(void);
void sig_SEGV_enable(void);
void signals_enable(void);
void install_signal_handlers(void);

#endif

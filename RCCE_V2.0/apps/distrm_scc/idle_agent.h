#ifndef __IDLE_AGENT_H__
#define __IDLE_AGENT_H__

#include "distrm.h"
#include "libfunctions.h"
#include "noc_functions.h"
#include "sig_aux.h"
#include "common_node.h"
#include "signal_handlers.h"
#include "scc_signals.h"

void idle_agent_actions(int idag_num, char scen_num[4]);
#endif

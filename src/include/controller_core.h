#ifndef __IDLE_AGENT_H__
#define __IDLE_AGENT_H__

#include "variables.h"

#ifndef PLAT_LINUX
void idle_agent_actions(char scen_directory[SCEN_DIR_SIZE], char scen_num[SCEN_NUM_SIZE]);
#else
void idle_agent_actions(char scen_directory[SCEN_DIR_SIZE], char scen_num[SCEN_NUM_SIZE], int Selfopt_Radius, int Max_SelfOpt_Interval_MS);
#endif

#endif

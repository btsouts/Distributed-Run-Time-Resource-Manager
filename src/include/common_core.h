#ifndef __COMMON_NODE_H__
#define __COMMON_NODE_H__

#include "variables.h"
void common_node_actions(char scen_directory[SCEN_DIR_SIZE], char scen_num[SCEN_NUM_SIZE], int Selfopt_Radius, int Max_SelfOpt_Interval_MS);
void my_settimer(int msec);
int my_gettimer(void);
#endif

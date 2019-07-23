#ifndef __IDAG_DEFS_H__
#define __IDAG_DEFS_H__

#include "my_rtrm.h"
#include "variables.h"

void global_idag_defs(void);
int is_core_idag (int core_id);
void read_idag_defs(char scen_directory[SCEN_DIR_SIZE], char scen_num[SCEN_NUM_SIZE], char idag_defs_file_name[SCEN_NUM_SIZE], char paxos_scen[PAXOS_SCEN_SIZE]);
void print_grid();
int im_manager();
int im_worker();
int im_controller();
void my_settimer(int msec);
int my_gettimer(void);
#endif

#ifndef __LIBFUNCTIONS_H__
#define __LIBFUNCTIONS_H__

#include "variables.h"

FILE * create_log_file(int node_id, int log, char scen_directory[SCEN_DIR_SIZE], char scen_num[SCEN_NUM_SIZE]);

int majority(int cores);
int get_id_from_pid(pid_t spid);

char * itoa(int value);
char * id2string(core_states state_id);
char * sig2string(int sig_id);
char * inter2string(inter_types interaction);
char * app_state_2_string(application_states state_id);

void handler_Enter(int sender_id, char *handler);
void handler_Exit(int sender_id, char *handler);
#endif



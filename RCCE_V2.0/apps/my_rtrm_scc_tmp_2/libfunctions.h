#ifndef __LIBFUNCTIONS_H__
#define __LIBFUNCTIONS_H__

int get_id_from_pid(pid_t spid);
char * itoa(int value);
char * get_pipe_name(int node_id);
void create_named_pipe(int node_id);
FILE * create_log_file(int node_id, char scen_num[4]);
#endif



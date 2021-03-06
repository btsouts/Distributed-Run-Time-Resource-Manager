#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "distrm.h"
#include "libfunctions.h"
/*extern int *pid_num;

int get_id_from_pid(pid_t spid){
	int i=0;

	while (i < X_max * Y_max){
		if (pid_num[i] == spid) return i;
		i++;
	}

	return -1;
}*/

char * itoa(int value){
	char *tmp;
	int size,i;

	if (value<10)
		size = 1;
	else if (value<100)
		size = 2;
	else if (value<1000)
		size = 3;
	else
		size = 4;	
	
	tmp = (char *) malloc((size+1)*sizeof(char));
	
	for (i=size-1; i>=0; i--){
		tmp[i] = (char) 48 + (value % 10);
		value = value / 10;
	}

	tmp[size] = '\0';
	return tmp;
}

char * get_pipe_name(int node_id){	
	char *name, *fifo_name;
	
	fifo_name = (char *) malloc(32*sizeof(char));
	strcpy(fifo_name,"./pipes/npipe");
	
	name = itoa(node_id);
	strcat(fifo_name,name);
	free(name);	

	return fifo_name;
}

void create_named_pipe(int node_id){
	char *fifo_name;//, *fifo_dir;
	struct stat st;

	fifo_name = get_pipe_name(node_id);
	if(stat(fifo_name,&st) != 0)
	if (mkfifo(fifo_name, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) < 0) {
		printf("mkfifo my id is %d ",node_id);		
		perror("mkfifo");
	}
	
	free(fifo_name);
}

FILE * create_log_file(int node_id, char scen_num[4]){
	char *log_file_name, *name;//, *fifo_dir;
	//int fd;
	FILE *log_file;	

	log_file_name = (char *) malloc(64*sizeof(char));
	strcpy(log_file_name,"/shared/herc/distrm/scenaria/");
	strcat(log_file_name, scen_num);
	strcat(log_file_name,"/log_files/log_file_");
	
	name = itoa(node_id);
	strcat(log_file_name,name);
	free(name);	

	//if ((fd = open(log_file_name, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0) {
	if ((log_file = fopen(log_file_name, "w")) == NULL){
		printf("open log my id is %d ",node_id);		
		perror("open log");
	}
	free(log_file_name);
	return log_file;
}

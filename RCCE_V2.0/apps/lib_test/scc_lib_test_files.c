#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#define _GNU_SOURCE

int main(int argc, char *argv[]){
	FILE *app_input, *log_file;
	char app_input_file_name[64], log_file_name[64];
	int time_next;
	
	strcpy(app_input_file_name,"/shared/herc/app_input.txt");
	//strcat(app_input_file_name, argv[1]);
	//strcat(app_input_file_name, "/app_input.txt");
	//printf("file path = %s\n",app_input_file_name);

	if ((app_input = fopen(app_input_file_name, "r")) == NULL){
		printf("Cannot open input file with file path = %s ",app_input_file_name);		
		perror("open app_input");
	}
	fscanf(app_input,"%d",&time_next);
	printf("time next = %d\n",time_next);
	//strcpy(log_file_name,"./scenaria/");
	//strcat(log_file_name, scen_num);
	strcpy(log_file_name,"/shared/herc/log_file_test2.txt");
	
	if ((log_file = fopen(log_file_name, "w")) == NULL){
		//printf("open log my id is %d ",node_id);		
		perror("open log");
	}

	fprintf(log_file,"poutanes\n");
	fclose(app_input);
	fclose(log_file);

	return 0;
}

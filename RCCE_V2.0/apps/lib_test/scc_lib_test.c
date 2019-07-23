#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MS 1000000
#define SIG_TIMER SIGRTMIN

/*void sig_TIMER_handler(int signo, siginfo_t *info, void *context)
{
	printf("timer went off and i don't know what to do\n");
}
*/

int main(int argc, char *argv[]){
	int i;
	float f[2],x[2];

	f[0] = 0.2;
	f[1] = 0.3;
	//i = f;
	memcpy(&i, &f[0], sizeof(int));
	memcpy(&x[0], &i, sizeof(int));
	//printf("i = %d if = %0.2f\n",i,i);
	//x = i;
	memcpy(&i, &f[1], sizeof(int));
	memcpy(&x[1], &i, sizeof(int));

	printf("x[0] = %0.2f x[1] = %0.2f\n",x[0],x[1]);	
	/*int fd_r,dummy=0,i,j,res;	
	char filename[64], command='0';

	//strcpy(filename,"/shared/herc/command_file");
	strcpy(filename,"/home/billtsou/Desktop/command_file");  
	fd_r = open(filename, O_RDONLY);// | O_NONBLOCK

	if (fd_r < 0) perror("open: ");
	else {
		while (command != '1') {
			dummy=0;
			for (i=0; i<1000; i++)
				for(j=0; j<1000; j++)
					dummy++;

			res = read(fd_r, &command, sizeof(char));
			if (res < 0) perror("read: ");
			if (command != '0') printf("I read %c\n",command);
		}
	}*/
	
		
	/*time_t cur_time;
	struct tm *cur_t;
	struct sigevent sev;
	struct itimerspec its;//, chk_timer;
	timer_t timerid;
	sigset_t sigset;
	struct sigaction sa;

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIG_TIMER;
	sev.sigev_value.sival_ptr = &timerid;
	if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1) printf("timer_create error\n");

	sa.sa_flags = SA_RESTART | SA_SIGINFO;
	sigemptyset(&sigset);
	//sigaddset(&sigset, SIG_TIMER);
	sa.sa_mask = sigset;

	sa.sa_sigaction = sig_TIMER_handler;
	if (sigaction(SIG_TIMER, &sa, NULL) < 0) {
		perror("sigaction: SIG_TIMER");
		exit(1);
	}

	sigaddset(&sigset, SIG_TIMER);
	
	if (sigprocmask(SIG_UNBLOCK, &sigset, NULL) < 0) {
		perror("signals_enable: sigprocmask");
		exit(1);
	}

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 10 * MS;
	if (timer_settime(timerid, 0, &its, NULL) == -1) perror("timer_settime error9");
	pause();

	cur_time = time(NULL);	
	cur_t = localtime(&cur_time);
	printf("[%d:%d:%d]: Everything went well\n",cur_t->tm_hour,cur_t->tm_min,cur_t->tm_sec);
	*/




	/*FILE *app_input, *log_file;
	char app_input_file_name[64], log_file_name[64];
	int time_next;
	
	strcpy(app_input_file_name,"/home/herc/app_input.txt");
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
	strcpy(log_file_name,"/home/herc/log_file_test.txt");
	
	if ((log_file = fopen(log_file_name, "w")) == NULL){
		//printf("open log my id is %d ",node_id);		
		perror("open log");
	}

	fprintf(log_file,"poutanes\n");
	fclose(app_input);
	fclose(log_file);
	*/


	return 0;
}

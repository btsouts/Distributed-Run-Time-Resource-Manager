#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/apps.h"
#include "include/scc_signals.h"
#include "include/libfunctions.h"
#include "include/my_rtrm.h"

#define SWAP(a,b) {float tmp; tmp=a; a=b; b=tmp;}
#define FFT_MAX 136192
#define PAGE_SIZE 4096
#define ARTIFICIAL_ROUND_DURATION_SEC 0.5
#define ARTIFICIAL_ROUND_DURATION_NSEC 500000000 /* 500 ms */
 
static float **svm_vectors, *svm_coef;
static int *vector, **matrix;
static float input_vector[D_sv];
//static float matr_speedup[NUM_OF_MATRICES][MAX_WORKERS_COUNT];
//static int matr_times[NUM_OF_MATRICES][MAX_WORKERS_COUNT];

static float Exec_Speedup[MAX_WORKERS_COUNT];
static int Exec_Latencies[MAX_WORKERS_COUNT];
//static float **vectors, *coef;

//2*(N+rootN*pad_length)*sizeof(float)+PAGE_SIZE);
static int P = 1;						/* DEFAULT_P = 1			 */
static int M = 16;					/* DEFAULT_M = 10			 */
static int N = 65536; 			/* N = 2^M 				 */
static int rootN = 256;			/* rootN = sqrt(N)			 */
static int num_cache_lines = 65536;
#define PADLENGTH 2


static float *x_local;	/* x is the original time-domain data	 */
static float *trans;          /* trans is used as scratch space        */
static float *umain;          /* umain is roots of unity for 1D FFTs   */
static float *umain2;         /* umain2 is entire roots of unity matrix*/
static float *upriv;

void execute_workload_svm (int lower_bound, int upper_bound);
void execute_workload_matrix (int lower_bound, int upper_bound);

void matrix_transpose(int n1, float *src, float *dest, int node_id, int myFirst, int myLast, int pad_length);
void FFT1D(int direction, int M, int N, float *x, float *scratch, float *upriv, float *umain2, int node_id, int myFirst, int myLast, int pad_length, int P);
void copyColumn(int n1, float *src, float *dest);
void single_FFT1D(int direction, int M, int N, float *u, float *x);
void twiddle_Col(int direction, int n1, int N, int j, float *u, float *x, int pad_length);
void reverse(int N, int M, float *x);
int reverse_bit(int M, int k);

void execute_workload_svm (int lower_bound, int upper_bound) {
	int i = 0, j = 0;
	float diff = 0, norma = 0, local_sum[N_sv];
	/* int vector_id = 0; Removed 16.02. Only one test vector */

	if (base_offset == -1) {
		base_offset = cur_agent.my_agent * N_sv;
		//fprintf(log_file, "My agent is %d. Calculated base_offset is %d\n",cur_agent.my_agent,base_offset);
	}
	
	for (i = lower_bound; i <= upper_bound; i++) {
		local_sum[i] = 0;
		scc_signals_check();
		
		for (j = 0; j < D_sv; j++){
			diff = svm_vectors[i][j] - input_vector[j];
			norma += diff*diff;
		}
		local_sum[i] += (float) (exp((double) (-gamma*norma))*svm_coef[i]);
		norma = 0;
	}
		
	for (i=lower_bound; i<=upper_bound; i++) 
		manager_result_out[base_offset+i] = (int) local_sum[i];	
}

void execute_workload_matrix (int lower_bound, int upper_bound) {
	int i, j, local_sum[MAX_ARRAY_SIZE];
	
	if (base_offset == -1) {
		//matrix_out = (int*) shmat (cur_agent.segment_id, NULL, 0);
		base_offset = cur_agent.my_agent * MAX_ARRAY_SIZE;
	}
	
	for (i=lower_bound; i<=upper_bound; i++) {
		local_sum[i] = 0;
		scc_signals_check();
		//signals_enable();
		for (j=0; j<cur_agent.array_size; j++)
			local_sum[i] += matrix[i][j] * vector[j];
		//signals_disable();
	}

	for (i=lower_bound; i<=upper_bound; i++) 
		manager_result_out[base_offset+i] = local_sum[i];
}	

void execute_workload_fft (int lower_bound, int upper_bound) {
	int work_id = 0, pad_length = PADLENGTH;
	
	if ((lower_bound == 0) && (upper_bound == FFT_MAX)) {
		P = 1;
	}	else {
		P = 2;
	}	
	
	/* FIXME works only because fft is restricted to two workers */
	if (lower_bound > 0) {
		work_id = 1;
	}	
	
	FFT1D(1, M, N, x_local, trans, upriv, umain2, work_id, lower_bound, upper_bound, pad_length, P); //HACK node_id - 1 important!!
}

void execute_workload_artificial (int lower_bound, int upper_bound) {
	int AppSpeedup = upper_bound - lower_bound;
	struct timespec ts;

	//if (base_offset == -1) {
	//	base_offset = cur_agent.my_agent * MAX_ARRAY_SIZE; /* FIXME Why is it always MAX_ARRAY_SIZE */
	//}
	
	ts.tv_sec = 0;
	ts.tv_nsec = ARTIFICIAL_ROUND_DURATION_NSEC / AppSpeedup;
	nanosleep(&ts, NULL);
	/*
	for (i=lower_bound; i<=upper_bound; i++) {
		sleep(ARTIFICIAL_ROUND_DURATION_SEC);
	}
	*/
	/*
	for (i=lower_bound; i<=upper_bound; i++) 
		manager_result_out[base_offset+i] = -1;
	*/
}

void execute_workload (int lower_bound, int upper_bound) {
	
	if (executed_app == MATRIX_MUL) {
		execute_workload_matrix (lower_bound, upper_bound);
	} else if (executed_app == SVM) {
		execute_workload_svm (lower_bound, upper_bound);			
	} else if (executed_app == FFT) {
		execute_workload_fft (lower_bound, upper_bound);
	} else if (executed_app == ARTIFICIAL) {
		execute_workload_artificial (lower_bound, upper_bound);
	}
}

void init_speedup_structs (void) {
	
	if (executed_app == MATRIX_MUL) {
		if (MATRIX_ARRAY_SIZE == 1024) {
#ifdef PLAT_SCC	
			Exec_Speedup[0] = 1.0;
			Exec_Speedup[1] = 1.188;
			Exec_Speedup[2] = 2.264;
			Exec_Speedup[3] = 3.0;
			Exec_Speedup[4] = 3.429;
			Exec_Speedup[5] = 4.0;
			Exec_Speedup[6] = 8.0;
			Exec_Speedup[7] = 0.0;
			
			Exec_Latencies[0] = 120;//29352;
			Exec_Latencies[1] = 101;//15112;
			Exec_Latencies[2] = 53;//11194;
			Exec_Latencies[3] = 40;//10313;
			Exec_Latencies[4] = 35;//8645;
			Exec_Latencies[5] = 30;//7871;
			Exec_Latencies[6] = 15;//6715;
#else
			Exec_Speedup[0] = 1.0;
			Exec_Speedup[1] = 1.065;
			Exec_Speedup[2] = 1.270;
			Exec_Speedup[3] = 0.0;
			Exec_Speedup[4] = 0.0;
			Exec_Speedup[5] = 0.0;
			Exec_Speedup[6] = 0.0;
			Exec_Speedup[7] = 0.0;
			
			Exec_Latencies[0] = 100000000;//29352;
			Exec_Latencies[1] = 31;//15112;
			Exec_Latencies[2] = 29;//11194;
			Exec_Latencies[3] = 24;//10313;
			Exec_Latencies[4] = 0;//8645;
			Exec_Latencies[5] = 0;//7871;
			Exec_Latencies[6] = 0;//6715;
			Exec_Latencies[7] = 0;//7014;
#endif
		} else if (MATRIX_ARRAY_SIZE == 2048) {
#ifdef PLAT_SCC				
			Exec_Speedup[0] = 1.0;
			Exec_Speedup[1] = 1.091;
			Exec_Speedup[2] = 1.2;
			Exec_Speedup[3] = 1.491;
			Exec_Speedup[4] = 1.791;
			Exec_Speedup[5] = 2.824;
			Exec_Speedup[6] = 3.0;

			Exec_Latencies[0] = 240;//112276;
			Exec_Latencies[1] = 220;//58880;
			Exec_Latencies[2] = 200;//40305;
			Exec_Latencies[3] = 161;//31705;
			Exec_Latencies[4] = 134;//28309;
			Exec_Latencies[5] = 85;//24512;
			Exec_Latencies[6] = 80;//22239;
			//matr_times[1][7] = 23;//20332;

#else
			Exec_Speedup[0] = 1.0;
			Exec_Speedup[1] = 1.331;
			Exec_Speedup[2] = 2.009;
			Exec_Speedup[3] = 2.315;
			Exec_Speedup[4] = 2.572;
			Exec_Speedup[5] = 0.0;
			Exec_Speedup[6] = 0.0;
			Exec_Speedup[7] = 0.0;//5.522;

			Exec_Latencies[0] = 100000000;//112276;
			Exec_Latencies[1] = 116;//58880;
			Exec_Latencies[2] = 87;//40305;
			Exec_Latencies[3] = 58;//31705;
			Exec_Latencies[4] = 50;//28309;
			Exec_Latencies[5] = 45;//24512;
			Exec_Latencies[6] = 0;//22239;
			Exec_Latencies[7] = 0;//20332;
#endif
		} else if (MATRIX_ARRAY_SIZE == 4096) {
#ifdef PLAT_SCC		
			Exec_Speedup[0] = 1.0;
			Exec_Speedup[1] = 2.001;
			Exec_Speedup[2] = 2.976;
			Exec_Speedup[3] = 4.032;
			Exec_Speedup[4] = 5.034;
			Exec_Speedup[5] = 6.25;
			Exec_Speedup[6] = 6.678;
			Exec_Speedup[7] = 6.819;

			Exec_Latencies[0] = 750;//384005;
			Exec_Latencies[1] = 374;//231583;
			Exec_Latencies[2] = 252;//157966;
			Exec_Latencies[3] = 186;//121222;
			Exec_Latencies[4] = 149;//101208;
			Exec_Latencies[5] = 120;//87852;
			Exec_Latencies[6] = 110;//78093;
#else 
			Exec_Speedup[0] = 1.0;
			Exec_Speedup[1] = 1.517;
			Exec_Speedup[2] = 1.958;
			Exec_Speedup[3] = 2.112;
			Exec_Speedup[4] = 2.878;
			Exec_Speedup[5] = 3.338;
			Exec_Speedup[6] = 4.241;
			Exec_Speedup[7] = 0.0;//5.073;
						
			Exec_Latencies[0] = 100000000;//384005;
			Exec_Latencies[1] = 431;//231583;
			Exec_Latencies[2] = 284;//157966;
			Exec_Latencies[3] = 220;//121222;
			Exec_Latencies[4] = 204;//101208;
			Exec_Latencies[5] = 150;//87852;
			Exec_Latencies[6] = 129;//78093;
			Exec_Latencies[7] = 102;//75690;
#endif
		} else {
			printf("Unknown array size\n");
			exit(0);	
		}
	} else if (executed_app == SVM) {
		/* N_sv 4096 D_sv 4096 */
		Exec_Speedup[0] = 1.0; /* 1 worker */
		Exec_Speedup[1] = 1.959;
		Exec_Speedup[2] = 2.919;
		Exec_Speedup[3] = 3.853;
		Exec_Speedup[4] = 4.777;
		Exec_Speedup[5] = 5.723;
		Exec_Speedup[6] = 6.644;
		Exec_Speedup[7] = 0.0;
		
		Exec_Latencies[0] = 578;
		Exec_Latencies[1] = 295;
		Exec_Latencies[2] = 198;
		Exec_Latencies[3] = 150;
		Exec_Latencies[4] = 121;
		Exec_Latencies[5] = 101;
		Exec_Latencies[6] = 87;
		Exec_Latencies[7] = 6; /* Irrelevant */
	} else if (executed_app == FFT) {
		Exec_Speedup[0] = 1.0; /* 1 worker */
		Exec_Speedup[1] = 1.55;
		Exec_Speedup[2] = 0;
		Exec_Speedup[3] = 0;
		Exec_Speedup[4] = 0;
		Exec_Speedup[5] = 0;
		Exec_Speedup[6] = 0;
		Exec_Speedup[7] = 0;
		
		Exec_Latencies[0] = 772;
		Exec_Latencies[1] = 498;
		Exec_Latencies[2] = 0;
		Exec_Latencies[3] = 0;
		Exec_Latencies[4] = 0;
		Exec_Latencies[5] = 0;
		Exec_Latencies[6] = 0;
		Exec_Latencies[7] = 0;
	} if (executed_app == ARTIFICIAL) {

	}	
}

void app_init (char scen_directory[SCEN_DIR_SIZE], char scen_num[SCEN_NUM_SIZE]) {
	int i, j, pad_length = PADLENGTH;
	char buf[MAX_STR_NAME_SIZE], *buffer;
	FILE *matrix_input, *support_vectors_file, *coef_file, *test_vector_file, *umain_file, *umain2_file, *x_local_file;
	size_t bufsize = 32;
	
	if (executed_app == MATRIX_MUL) {
		cur_agent.array_size = MATRIX_ARRAY_SIZE;
		matrix = (int **) malloc(cur_agent.array_size * sizeof(int *));
		#ifdef PLAT_SCC
		strcpy(buf, "/shared/herc/");
		#else
		strcpy(buf, "../");
		#endif
		strcat(buf, scen_directory);
		strcat(buf, "/MATRIX-inputs/");
		strcat(buf, itoa(cur_agent.array_size));
		fprintf(log_file,"matrix file path = %s\n",buf);

		if ((matrix_input = fopen(buf, "r")) == NULL){
			printf("Cannot open input file with file path = %s ",buf);
			perror("open matrix_input");
		}

		for (i=0; i<cur_agent.array_size; i++) {
			matrix[i] = (int *) malloc(cur_agent.array_size * sizeof(int));
			for (j=0; j<cur_agent.array_size; j++) 
				fscanf(matrix_input,"%d",&matrix[i][j]);
		}

		vector = (int *) malloc(cur_agent.array_size * sizeof(int));
		for (j=0; j<cur_agent.array_size; j++) 
			fscanf(matrix_input,"%d",&vector[j]);
		
		fclose(matrix_input);
	} else if (executed_app == SVM) {
		#ifdef PLAT_SCC
		strcpy(buf, "/shared/herc/");
		#else
		strcpy(buf, "../");
		#endif
		strcat(buf,scen_directory);
		//strcat(buf,"/");
		//strcat(buf,scen_num);
		strcat(buf,"/SVM-inputs/support_vectors_N_sv_");
		strcat(buf,itoa(N_sv));
		strcat(buf,"_D_sv_");
		strcat(buf,itoa(D_sv));
		strcat(buf,".dat");
		fprintf(log_file,"svm file path = %s\n",buf);
		
		if ((support_vectors_file = fopen(buf,"r")) == NULL){
			printf("Cannot open input file with file path = %s ",buf);
			perror("open svm_input");
		}
		
		#ifdef PLAT_SCC
		strcpy(buf, "/shared/herc/");
		#else
		strcpy(buf, "../");
		#endif
		strcat(buf,scen_directory);
		//strcat(buf,"/");
		//strcat(buf,scen_num);
		strcat(buf,"/SVM-inputs/sv_coef_N_sv_");
		strcat(buf,itoa(N_sv));
		strcat(buf,"_D_sv_");
		strcat(buf,itoa(D_sv));
		strcat(buf,".dat");
		fprintf(log_file,"svm_coef file path = %s\n",buf);
		
		if ((coef_file = fopen(buf,"r")) == NULL){
			printf("Cannot open input file with file path = %s ",buf);
			perror("open svm_input");
		}
		
		#ifdef PLAT_SCC
		strcpy(buf, "/shared/herc/");
		#else
		strcpy(buf, "../");
		#endif
		strcat(buf,scen_directory);
		//strcat(buf,"/");
		//strcat(buf,scen_num);
		strcat(buf,"/SVM-inputs/test_vector_D_sv_");
		strcat(buf,itoa(D_sv));
		strcat(buf,".dat");
		fprintf(log_file,"test_vector file path = %s\n",buf);

		if ((test_vector_file = fopen(buf,"r")) == NULL){
						printf("Cannot open input file with file path = %s ",buf);
						perror("open svm_input");
		}

		svm_vectors = (float **)malloc(N_sv*sizeof(float *));
		if (svm_vectors == NULL){
			printf("--%d-- svm_vectors malloc fail!!\n", node_id);
			perror("malloc error");
		}
		svm_coef = (float *)malloc(N_sv*sizeof(float));
		if (svm_coef == NULL){
			printf("--%d-- svm_coef malloc fail!!\n", node_id);
			perror("malloc error");
		}
		
		buffer = (char *)malloc(bufsize * sizeof(char));
		for (i = 0; i < N_sv; i++) {
			svm_vectors[i] = (float *)malloc(D_sv*sizeof(float));
			if (svm_vectors[i] == NULL) {
				printf("--%d-- svm_vectors[%d] malloc fail!!\n", node_id, i);
				perror("malloc error");
			} else {
				for (j = 0; j < D_sv; j++) {
					/* Read support svm_vectors */
					if (j < D_sv){
						fscanf(support_vectors_file,"%f",&svm_vectors[i][j]);
						fgetc(support_vectors_file);
					}else{
						getline(&buffer,&bufsize,support_vectors_file);
					}
				}
			}
		}

		for (j = 0; j < N_sv; j++) {
			/* Read coefficients */
			fscanf(coef_file,"%f",&svm_coef[j]);
			fgetc(coef_file);
		}

		for (j = 0; j < D_sv; j++) {
			/* Read coefficients */
			fscanf(test_vector_file,"%f",&input_vector[j]);
		}

		cur_agent.array_size = -1;
		fclose(support_vectors_file);
		fclose(coef_file);
		fclose(test_vector_file);
		free(buffer);
	} else if (executed_app == FFT) {
		fprintf(log_file,"Initializing FFT application\n");
		
		x_local = (float *)malloc(2*(N+rootN*pad_length)*sizeof(float)+PAGE_SIZE);
		if (x_local == NULL){
			printf("Malloc error for x_local\n");
			perror("malloc error");
			exit(-1);
		}
		
		trans = (float *)malloc(2*(N+rootN*pad_length)*sizeof(float)+PAGE_SIZE);
		if (trans == NULL){
			printf("Malloc error for trans\n");
			perror("malloc error");
			exit(-1);
		}
		
		umain = (float *)malloc(2*rootN*sizeof(float));  
		if (umain == NULL){
			printf("Malloc error for umain\n");
			perror("malloc error");
			exit(-1);
		}
		
		umain2 = (float *)malloc(2*(N+rootN*pad_length)*sizeof(float)+PAGE_SIZE);
		if (umain2 == NULL){
			printf("Malloc error for umain2\n");
			perror("malloc error");
			exit(-1);
		}
		
		upriv = (float *)malloc(2*(rootN-1)*sizeof(float));
		if (upriv == NULL){
			printf("--%d-- Malloc error for upriv\n", node_id);
			perror("malloc error");
			exit(-1); 
		}
				
		#ifdef PLAT_SCC
		strcpy(buf, "/shared/herc/");
		#else
		strcpy(buf, "../");
		#endif
		strcat(buf,scen_directory);
		strcat(buf,"/FFT-inputs/umain_file");
		fprintf(log_file,"umain_file file path = %s\n",buf);
		
		if ((umain_file = fopen(buf,"r")) == NULL){
			printf("Cannot open input file with file path = %s ",buf);
			perror("open fft_input");
		}
		
		for (i=0; i<2*rootN; i++) {
			fscanf(umain_file,"%f",&umain[i]);
		}
		fclose(umain_file);
		
		#ifdef PLAT_SCC
		strcpy(buf, "/shared/herc/");
		#else
		strcpy(buf, "../");
		#endif
		strcat(buf,scen_directory);
		strcat(buf,"/FFT-inputs/umain2_file");
		fprintf(log_file,"umain2_file file path = %s\n",buf);
		
		if ((umain2_file = fopen(buf,"r")) == NULL){
			printf("Cannot open input file with file path = %s ",buf);
			perror("open umain_file");
		}
		
		for (i=0; i<2*(N+rootN*pad_length); i++) {
			fscanf(umain2_file,"%f",&umain2[i]);
		}
		fclose(umain2_file);
		
		#ifdef PLAT_SCC
		strcpy(buf, "/shared/herc/");
		#else
		strcpy(buf, "../");
		#endif
		strcat(buf,scen_directory);
		strcat(buf,"/FFT-inputs/x_local_file");
		fprintf(log_file,"x_local_file file path = %s\n",buf);
		
		if ((x_local_file = fopen(buf,"r")) == NULL){
			printf("Cannot open input file with file path = %s ",buf);
			perror("open x_local_file");
		}
		
		for (i=0; i<2*(N+rootN*pad_length); i++) {
			fscanf(x_local_file,"%f",&x_local[i]);
		}
		fclose(x_local_file);
		
		for (i = 0; i < 2*(rootN-1); i++){
			upriv[i] = umain[i];
		}
		
	} else if (executed_app == MATRIX_MUL) {

	}
}

int get_max_cores_count(app cur_app){
#ifdef SINGLE_WORKER
	return 2;
#elif ARTIFICIAL_APPS_SIM
	int tmp_max_cores=MAX_WORKERS_COUNT; /* FIXME 31.10.2017 unable to send more than 8 workers via MPB */
	
	if (cur_app.var < 1.0) {
		tmp_max_cores = (int) ceilf(2.0*cur_app.A - 1);
	} else {
		tmp_max_cores = (int) ceilf(cur_app.A + (cur_app.A*cur_app.var) - cur_app.var);
	}

	if (tmp_max_cores < MAX_WORKERS_COUNT) {
		return tmp_max_cores;
	} else {
		return MAX_WORKERS_COUNT;
	}
#else
	if (executed_app == FFT) {
		return 3;
	}	else {
		return MAX_WORKERS_COUNT;
	}
#endif	
}

#ifdef ARTIFICIAL_APPS_SIM
float Speedup_Artificial_App(app cur_app, int num_of_cores) {
	float res=0;
	
	if (num_of_cores > 0) {
		if (cur_app.var < 1.0) {
			if (num_of_cores == 1) {
				res = 1;
			} else if ((num_of_cores > 1) && (num_of_cores < cur_app.A)) {
				res = (num_of_cores*cur_app.A) / (cur_app.A + (cur_app.var / 2.0*(num_of_cores-1)));			
			} else if ((num_of_cores >= cur_app.A) && (num_of_cores < 2.0*cur_app.A - 1)) {
				res = (num_of_cores*cur_app.A) / (cur_app.var*(cur_app.A -0.5) + num_of_cores*(1.0 - 0.5*cur_app.var));
			} else {
				res = cur_app.A;
			}
		} else { /* For n=1, result is 1*/
			if ((num_of_cores >= 1) && (num_of_cores <= (cur_app.A + cur_app.A*cur_app.var - cur_app.var))) {
				res = (num_of_cores*cur_app.A*(cur_app.var + 1)) / (cur_app.A + cur_app.var*(num_of_cores-1 + cur_app.A));
			} else {
				res = cur_app.A;
			}
		}
	}

	return res;
}
#endif

float Speedup(app cur_app, int num_of_cores) {
	if ((num_of_cores < 2) || (num_of_cores > get_max_cores_count(cur_app))) {
		return 0;
	} else {
#ifndef ARTIFICIAL_APPS_SIM
		return Exec_Speedup[num_of_cores-2];
#else
		return Speedup_Artificial_App(cur_app, num_of_cores-1);	
#endif
	}
}

int get_times(app cur_app, int num_of_cores) {
#ifndef ARTIFICIAL_APPS_SIM
	return (cur_app.workld * Exec_Latencies[num_of_cores-2]);
#else
	return cur_app.workld * (ARTIFICIAL_ROUND_DURATION_SEC / ((int) Speedup_Artificial_App(cur_app, num_of_cores+1))); /* FIXME cutting off floating points -- +1 is because in Speedup calc it is -1*/
#endif
}

void matrix_transpose(int n1, float *src, float *dest, int node_id, int myFirst, int myLast, int pad_length){
	int i; 
	int j; 
	int k; 
	int l; 
	int m;
	int blksize;
	int numblks;
	int firstfirst;
	int h_off;
	int v_off;
	int v;
	int h;
	int n1p;
	int row_count;

	//fprintf(log_file,"I am inside matrix_transpose-0 node_id is %d n1 %d\n",node_id,n1);
	blksize = myLast-myFirst;
	numblks = (2*blksize)/num_cache_lines;
	if (numblks * num_cache_lines != 2 * blksize) {
		numblks ++;
	}
	blksize = blksize / numblks;
	firstfirst = myFirst;
	row_count = n1/P;
	n1p = n1+pad_length;
	for (l=node_id+1;l<P;l++) {
		v_off = l*row_count;
		for (k=0; k<numblks; k++) {
			h_off = firstfirst;
			for (m=0; m<numblks; m++) {
				for (i=0; i<blksize; i++) {
					v = v_off + i;
					for (j=0; j<blksize; j++) {
						h = h_off + j;
						//fprintf(log_file,"Index dest is %d\n",2*(h*n1p+v));
						//fprintf(log_file,"Index src is %d\n",2*(v*n1p+h));
						//fprintf(log_file,"src = %f\n",src[2*(v*n1p+h)]);
						//fprintf(log_file,"src + 1 = %f\n",src[2*(v*n1p+h)+1]);
						//fprintf(log_file,"dest = %f\n",dest[2*(h*n1p+v)]);
						//fprintf(log_file,"dest + 1 = %f\n",dest[2*(h*n1p+v)+1]);
						dest[2*(h*n1p+v)] = src[2*(v*n1p+h)];
						dest[2*(h*n1p+v)+1] = src[2*(v*n1p+h)+1];
						//fprintf(log_file,"yolo\n");
					}
				}
				h_off += blksize;
			}
			v_off+=blksize;
		}
	}
	//fprintf(log_file,"I am inside matrix_transpose-A\n");
	
	for (l=0;l<node_id;l++) {
		v_off = l*row_count;
		for (k=0; k<numblks; k++) {
			h_off = firstfirst;
			for (m=0; m<numblks; m++) {
				for (i=0; i<blksize; i++) {
					v = v_off + i;
					for (j=0; j<blksize; j++) {
						h = h_off + j;
						dest[2*(h*n1p+v)] = src[2*(v*n1p+h)];
						dest[2*(h*n1p+v)+1] = src[2*(v*n1p+h)+1];
					}
				}
				h_off += blksize;
			}
			v_off+=blksize;
		}
	}
	//fprintf(log_file,"I am inside matrix_transpose-B\n");
	
	v_off = node_id*row_count;
	for (k=0; k<numblks; k++) {
		h_off = firstfirst;
		for (m=0; m<numblks; m++) {
			for (i=0; i<blksize; i++) {
				v = v_off + i;
				for (j=0; j<blksize; j++) {
				  h = h_off + j;
				  dest[2*(h*n1p+v)] = src[2*(v*n1p+h)];
				  dest[2*(h*n1p+v)+1] = src[2*(v*n1p+h)+1];
				}
			}
			h_off += blksize;
		}
		v_off+=blksize;
	}
	//fprintf(log_file,"I am inside matrix_transpose-C\n");
}

//FFT1D(1, M, N, x_local, trans, upriv, umain2, work_id, lower_bound, upper_bound, pad_length, P);
void FFT1D(int direction, int M, int N, float *x, float *scratch, float *upriv, float *umain2, int node_id, int myFirst, int myLast, int pad_length, int P){
	int j, m1, n1;
	
	//fprintf(log_file,"I am inside FFT1D-A myFirst=%d myLast=%d\n",myFirst,myLast);
	
	m1 = M/2;
	n1  = 1 << m1;
	
	matrix_transpose(n1, x, scratch, node_id, myFirst, myLast, pad_length);
	//fprintf(log_file,"I am inside FFT1D-B\n");
	
	/* do n1 1D FFTs on columns */	
	for (j = myFirst; j < myLast; j++){
		single_FFT1D(direction, m1, n1, upriv, &scratch[2*j*(n1+pad_length)]);
		twiddle_Col(direction, n1, N, j, umain2, &scratch[2*j*(n1+pad_length)],pad_length);
	}
	//fprintf(log_file,"I am inside FFT1D-C\n");
	
	matrix_transpose(n1, scratch, x, node_id, myFirst, myLast, pad_length);
	//fprintf(log_file,"I am inside FFT1D-D\n");
	
	/* do n1 1D FFTs on columns again */
	for (j = myFirst; j < myLast; j++) {
		single_FFT1D(direction, m1, n1, upriv, &x[2*j*(n1+pad_length)]);
	}
	//fprintf(log_file,"I am inside FFT1D-E\n");
	
	matrix_transpose(n1, x, scratch, node_id, myFirst, myLast, pad_length);
	//fprintf(log_file,"I am inside FFT1D-F\n");
	/*for (j = myFirst; j < myLast; j++){
		copyColumn(n1, &scratch[2*j*(n1+pad_length)], &x_shared[2*j*(n1+pad_length)]);
	}*/
	
	return;

}

void copyColumn(int n1, float *src, float *dest){
      
	int i;

	for (i = 0; i < n1; i++) {
		dest[2*i] = src[2*i];
		dest[2*i+1] = src[2*i+1];
	}
}

void single_FFT1D(int direction, int M, int N, float *u, float *x){
	
	int j, k, q, L, r, Lstar;
	float *u1, *x1, *x2;
	float omega_r, omega_c, tau_r, tau_c, x_r, x_c;
	
	reverse(N, M, x);
	
	for (q=1; q<=M; q++) {
		L = 1<<q; r = N/L; Lstar = L/2;
		u1 = &u[2*(Lstar-1)];
		for (k=0; k<r; k++) {
			x1 = &x[2*(k*L)];
			x2 = &x[2*(k*L+Lstar)];
			for (j=0; j<Lstar; j++) {
				omega_r = u1[2*j]; 
				omega_c = direction*u1[2*j+1];
				x_r = x2[2*j]; 
				x_c = x2[2*j+1];
				tau_r = omega_r*x_r - omega_c*x_c;
				tau_c = omega_r*x_c + omega_c*x_r;
				x_r = x1[2*j]; 
				x_c = x1[2*j+1];
				x2[2*j] = x_r - tau_r;
				x2[2*j+1] = x_c - tau_c;
				x1[2*j] = x_r + tau_r;
				x1[2*j+1] = x_c + tau_c;
			}
		}
	}
	
	return;
}

void twiddle_Col(int direction, int n1, int N, int j, float *u, float *x, int pad_length){
 
	int i;
	float omega_c, omega_r, x_r, x_c;
	
	for (i = 0; i < n1; i++) {
		omega_r = u[2*(j*(n1+pad_length)+i)];
		omega_c = direction*u[2*(j*(n1+pad_length)+i)+1];  
		x_r = x[2*i]; 
		x_c = x[2*i+1];
		x[2*i] = omega_r*x_r - omega_c*x_c;
		x[2*i+1] = omega_r*x_c + omega_c*x_r;
	}
	
	return;
}

void reverse(int N, int M, float *x){
  
	int j, k;
	
	for (k = 0; k < N; k++){
		j = reverse_bit(M, k);
		if (j > k){
		      SWAP(x[2*j], x[2*k]);
		      SWAP(x[2*j+1], x[2*k+1]);  
		}
	}
	
	return;
}

int reverse_bit(int M, int k){
	
	int i, j = 0, tmp = k;
	
	for (i = 0; i < M; i++){
		j = 2*j + (tmp&0x1);
		tmp = tmp >> 1;
	}
	
	return j;
}

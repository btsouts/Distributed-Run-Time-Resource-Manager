#ifndef __APPS_H__
#define __APPS_H__

#include "variables.h"

#define MATRIX_ARRAY_SIZE 4096
#define FFT_ARRAY_SIZE 256 
#define N_sv 4096
#define D_sv 4096
#define gamma 0.12
#define SVM_ARRAY_SIZE N_sv

void init_speedup_structs (void);
void app_init (char scen_directory[SCEN_DIR_SIZE], char scen_num[SCEN_NUM_SIZE]);
void execute_workload (int lower_bound, int upper_bound);
int get_max_cores_count(app cur_app);
int get_times(app cur_app, int num_of_cores);
float Speedup(app cur_app, int num_of_cores);

#endif

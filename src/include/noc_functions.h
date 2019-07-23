#ifndef __NOC_FUNCTIONS_H__
#define __NOC_FUNCTIONS_H__

#include "variables.h"

int distance(int core_start, int core_fin);
void get_reg_idags(region cur_reg, int *region_idags);
int region_count(region cur_reg);
int get_cluster_info(int idag_num, int *Cl_x_max, int *Cl_y_max);
void create_scc2grid_mapping (char scen_directory[SCEN_DIR_SIZE], char scen_num[SCEN_NUM_SIZE]);
void create_grid2scc_mapping (char scen_directory[SCEN_DIR_SIZE], char scen_num[SCEN_NUM_SIZE]);

#endif

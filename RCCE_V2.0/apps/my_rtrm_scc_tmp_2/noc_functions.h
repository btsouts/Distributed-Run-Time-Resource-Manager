#ifndef __NOC_FUNCTIONS_H__
#define __NOC_FUNCTIONS_H__

int distance(int core_start, int core_fin);
void get_reg_idags(region cur_reg, int *region_idags);
int region_count(region cur_reg);
int get_cluster_info(int idag_num, int *Cl_x_max, int *Cl_y_max);
#endif

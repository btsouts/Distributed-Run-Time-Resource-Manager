#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "include/my_rtrm.h"
#include "include/idag_defs.h"
#include "include/libfunctions.h"
#include "include/variables.h"

/* FIXME num_idags_y to be removed from cluster info*/

/*
const int scc2grid_mapping[48] = {0, 6, 1, 7, 2, 8,
				  3, 9, 4, 10, 5, 11,
				  12, 18, 13, 19, 14, 20,
				  15, 21, 16, 22, 17, 23,
				  24, 30, 25, 31, 26, 32,
				  27, 33, 28, 34, 29, 35,
				  36, 42, 37, 43, 38, 44,
				  39, 45, 40, 46, 41, 47};

const int grid2scc_mapping[48] = {0, 2, 4, 6, 8, 10,
				  1, 3, 5, 7, 9, 11,
				  12, 14, 16, 18, 20, 22,
				  13, 15, 17, 19, 21, 23,
				  24, 26, 28, 30, 32, 34,
				  25, 27, 19, 31, 33, 35,
				  36, 38, 40, 42, 44, 46,
				  37, 39, 41, 43, 45, 47};
*/

static int *scc2grid_mapping, *grid2scc_mapping;

void create_scc2grid_mapping (char scen_directory[SCEN_DIR_SIZE], char scen_num[SCEN_NUM_SIZE]) {
    int i, j;
    char scc2grid_path[MAX_STR_NAME_SIZE]; 
    FILE *scc2grid_file;

    scc2grid_mapping = (int *) malloc(X_max * Y_max * sizeof(int));
#ifdef PLAT_SCC
    strcpy(scc2grid_path, "/shared/herc/");
#else
    strcpy(scc2grid_path, "../");
#endif
    strcat(scc2grid_path, scen_directory);
    strcat(scc2grid_path, "/");
    strcat(scc2grid_path, scen_num);
    strcat(scc2grid_path, "/grids/scc2grid_");
    strcat(scc2grid_path, itoa(X_max));
    strcat(scc2grid_path, "_");
    strcat(scc2grid_path, itoa(Y_max));
    
    if (node_id == 0){
	    printf("scc2grid_path = %s\n",scc2grid_path);
	    fflush(stdout);
    }
  
    if ((scc2grid_file = fopen(scc2grid_path, "r")) == NULL){
      printf("Cannot open input file with file path = %s ",scc2grid_path);
      perror("open scc2grid_path");
    }
    
    for (i=0; i<Y_max; i++)
      for (j=0; j<X_max; j++)
	fscanf(scc2grid_file, "%d", &scc2grid_mapping[i*X_max + j]);  
} 

void create_grid2scc_mapping (char scen_directory[SCEN_DIR_SIZE], char scen_num[SCEN_NUM_SIZE]) {
    int i, j;
    char grid2scc_path[MAX_STR_NAME_SIZE]; 
    FILE *grid2scc_file;
    
    grid2scc_mapping = (int *) malloc(X_max * Y_max * sizeof(int));
#ifdef PLAT_SCC
    strcpy(grid2scc_path, "/shared/herc/");
#else    
    strcpy(grid2scc_path, "../");
#endif
    strcat(grid2scc_path, scen_directory);
    strcat(grid2scc_path, "/");
    strcat(grid2scc_path, scen_num);
    strcat(grid2scc_path, "/grids/grid2scc_");
    strcat(grid2scc_path, itoa(X_max));
    strcat(grid2scc_path, "_");
    strcat(grid2scc_path, itoa(Y_max));
    
    if (node_id == 0){
	    printf("grid2scc_path = %s\n",grid2scc_path);
	    fflush(stdout);
    }
  
    if ((grid2scc_file = fopen(grid2scc_path, "r")) == NULL){
      printf("Cannot open input file with file path = %s ",grid2scc_path);
      perror("open grid2scc_path");
    }
    
    for (i=0; i<Y_max; i++)
      for (j=0; j<X_max; j++)
	fscanf(grid2scc_file, "%d", &grid2scc_mapping[i*X_max + j]);  
}

int distance(int core_start, int core_fin) {
	int x1,x2,y1,y2;

	x1 = scc2grid_mapping[core_start] % X_max;
	y1 = scc2grid_mapping[core_start] / X_max;

	//printf("x1 = %d, y1 = %d\n",x1,y1);
	x2 = scc2grid_mapping[core_fin] % X_max;
	y2 = scc2grid_mapping[core_fin] / X_max;
	//printf("x2 = %d, y2 = %d\n",x2,y2);

	return abs(x1 - x2) + abs(y1 - y2);
}
extern FILE *log_file;
void get_reg_idags(region cur_reg, int *region_idags){
	int y,l,count=0,center_node,cur_line,i,j;

	for (i=0; i<num_idags; i++) 
	  region_idags[i]=0;
	
	region_idags[i] = -1; //the one reserved for count is set to -1 for safety
	l = cur_reg.r;
	
	/* Lower half of area + center*/
	/* Transpose to regural grid coordinates*/
	center_node = scc2grid_mapping[cur_reg.C];
	cur_line = center_node / X_max;
	fprintf(log_file,"\t\tSearching for idags... num_idags = %d\n",num_idags);
	while (cur_line >= 0 && l>=0){
		cur_line = center_node / X_max;
		for (i=0; i<=l; i++){
			y = (center_node + i) / X_max;
			if (y == cur_line){
				for (j=0; j<num_idags; j++)
					if (idag_id_arr[j] == idag_mask[grid2scc_mapping[center_node + i]]) break;
			  
				if (region_idags[j] == 0) {
					region_idags[j] = 1;
					count++;
				}
			}
		
			y = (center_node - i) / X_max;
			if (y == cur_line && (center_node - i)>=0){
				for (j=0; j<num_idags; j++)
					if (idag_id_arr[j] == idag_mask[grid2scc_mapping[center_node - i]]) break;

				if (region_idags[j] == 0) {
					region_idags[j] = 1;
					count++;
				} 		
			}
		}

		center_node -= X_max;
		cur_line--;
		l--;
	}

	/* Upper half of area */
	if (cur_reg.C + X_max < X_max*Y_max) {
		center_node = scc2grid_mapping[cur_reg.C + X_max];
		l = cur_reg.r-1;	
		cur_line = center_node / X_max;	
		while (cur_line < Y_max && l>=0) {
			cur_line = center_node / X_max;
				for (i=0; i<=l; i++) {
					y = (center_node + i) / X_max;
				if (y == cur_line) {
					for (j=0; j<num_idags; j++)
						if (idag_id_arr[j] == idag_mask[grid2scc_mapping[center_node + i]]) break;

					if (region_idags[j] == 0) {
						region_idags[j] = 1;
						count++;
					}
				}

				y = (center_node - i) / X_max;
				if (y == cur_line && (center_node - i)>=0) {
					for (j=0; j<num_idags; j++)
						if (idag_id_arr[j] == idag_mask[grid2scc_mapping[center_node - i]]) break;

					if (region_idags[j] == 0) {
						region_idags[j] = 1;
						count++;
					} 		
				}
			}

			center_node += X_max;
			cur_line++;
			l--;	
		}
	}

	region_idags[num_idags] = count;
}

int region_count(region cur_reg){
	int x,y,l,count=0;

	x = scc2grid_mapping[cur_reg.C] % X_max;
	y = scc2grid_mapping[cur_reg.C] / X_max;
	l = cur_reg.r;
	//printf("x = %d, y = %d l=%d\n",x,y,l);
	
	while (y >= 0 && l>=0){
		count++;////1 for center
		if (x + l < X_max) count += l;
		else count += X_max - x -1;

		if (x - l >= 0) count += l;
		else count += x;

		y--;
		l--;
	}

	y = (scc2grid_mapping[cur_reg.C] / X_max) + 1;
	l = cur_reg.r - 1;

	while (y < Y_max && l>=0){
		count++;
		if (x + l < X_max) count += l;
		else count += X_max - x -1;

		if (x - l >= 0) count += l;
		else count += x;

		y++;
		l--;	
	}

	return count;
}

//returns on function name the idag_id and in call by name, cluster sizes
int get_cluster_info(int idag_num, int *Cl_x_max, int *Cl_y_max){
	int x_coord,y_coord, diff, tmp_cl_x, tmp_cl_y;
	int num_idags_y = num_idags / num_idags_x;
	
	tmp_cl_x = X_max / num_idags_x;
	tmp_cl_y = Y_max / num_idags_y;
	x_coord = (idag_num % num_idags_x) * tmp_cl_x;
	y_coord = (idag_num / num_idags_x) * tmp_cl_y;
	
	diff = X_max % num_idags_x;
	if ((diff > 0) && (idag_num % num_idags_x < diff)){
		x_coord += idag_num % num_idags_x;
		tmp_cl_x++;
	} else x_coord += diff;
		
	diff = Y_max % num_idags_y;
	if ((diff > 0) && (idag_num / num_idags_x < diff)){
		y_coord += idag_num / num_idags_x; //which line - height
		tmp_cl_y++;	
	} else y_coord += diff;

	*Cl_x_max = tmp_cl_x;
	*Cl_y_max = tmp_cl_y;
	return (y_coord * X_max) + x_coord;
}


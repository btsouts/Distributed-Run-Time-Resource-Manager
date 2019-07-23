#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "distrm.h"

extern int *idag_mask, *idag_id_arr;

int distance(int core_start, int core_fin){
	int x1,x2,y1,y2;

	x1 = core_start % X_max;
	y1 = core_start / X_max;

	//printf("x1 = %d, y1 = %d\n",x1,y1);
	x2 = core_fin % X_max;
	y2 = core_fin / X_max;
	//printf("x2 = %d, y2 = %d\n",x2,y2);

	return abs(x1 - x2) + abs(y1 - y2);
}

void get_reg_idags(region cur_reg, int *region_idags){
	int y,l,count=0,center_node,cur_line,i,j;

	//x = cur_reg.C % X_max;
	//y = cur_reg.C / X_max;
	for (i=0; i<num_idags_x*num_idags_y; i++) region_idags[i]=0;
	region_idags[i]=0;//the one reserved for count is set to 1 for safety
	l = cur_reg.r;
	//printf("x = %d, y = %d l=%d\n",x,y,l);

	center_node = cur_reg.C;
	cur_line = center_node / X_max;	
	while (cur_line >= 0 && l>=0){
		cur_line = center_node / X_max;
		for (i=0; i<=l; i++){
			y = (center_node + i) / X_max;
			if (y == cur_line){
				for (j=0; j<num_idags_x*num_idags_y; j++)
					if (idag_id_arr[j] == idag_mask[center_node + i]) break;
			  
				if (region_idags[j] == 0) {
					region_idags[j] = 1;
					count++;
				}
			}
		
			y = (center_node - i) / X_max;
			if (y == cur_line && (center_node - i)>=0){
				for (j=0; j<num_idags_x*num_idags_y; j++)
					if (idag_id_arr[j] == idag_mask[center_node - i]) break;

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

	center_node = cur_reg.C + X_max;
	l = cur_reg.r-1;	
	cur_line = center_node / X_max;	
	while (cur_line < Y_max && l>=0){
		cur_line = center_node / X_max;
		for (i=0; i<=l; i++){
			y = (center_node + i) / X_max;
			if (y == cur_line){
				for (j=0; j<num_idags_x*num_idags_y; j++)
					if (idag_id_arr[j] == idag_mask[center_node + i]) break;

				if (region_idags[j] == 0) {
					region_idags[j] = 1;
					count++;
				}
			}

			y = (center_node - i) / X_max;
			if (y == cur_line && (center_node - i)>=0){
				for (j=0; j<num_idags_x*num_idags_y; j++)
					if (idag_id_arr[j] == idag_mask[center_node - i]) break;

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

	region_idags[num_idags_x*num_idags_y] = count;
}

int region_count(region cur_reg){
	int x,y,l,count=0;

	x = cur_reg.C % X_max;
	y = cur_reg.C / X_max;
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

	y = cur_reg.C / X_max + 1;
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
		y_coord += idag_num / num_idags_x; //se poia grammh - ypsos tou y einai
		tmp_cl_y++;	
	} else y_coord += diff;

	//printf("idag_id = %d, x_coord = %d, y_coord = %d\n",idag_num,x_coord,y_coord);
	*Cl_x_max = tmp_cl_x;
	*Cl_y_max = tmp_cl_y;
	return (y_coord * X_max) + x_coord;
}


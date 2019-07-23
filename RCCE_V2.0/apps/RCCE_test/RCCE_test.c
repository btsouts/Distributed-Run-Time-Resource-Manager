//#include <string.h>
#include <stdio.h>
#include "RCCE.h"

#define max(x,y) ((x)>(y)?(x):(y))
#define LINE_SIZE 8 //number of bytes

int RCCE_APP(int argc, char **argv){
	int ME, NUES, i, dummy=0;
  	int *sig_array, *sig_array_local, sig_source, *sig_read_ar;
	RCCE_FLAG flag_rec_sig,flag_send_sig;
	RCCE_FLAG_STATUS receiver_status;
	int error, str_len;
	char error_str[64];
  	
	RCCE_init(&argc, &argv);

  	//  RCCE_debug_set(RCCE_DEBUG_ALL);
	ME = RCCE_ue();
	NUES = RCCE_num_ues();
	RCCE_flag_alloc(&flag_rec_sig);
	RCCE_flag_alloc(&flag_send_sig);
	RCCE_flag_write(&flag_rec_sig, RCCE_FLAG_SET, ME);
	RCCE_flag_write(&flag_send_sig, RCCE_FLAG_UNSET, ME);
	sig_array = (int *) RCCE_malloc(NUES * LINE_SIZE * sizeof(int));//NUES * NUES
	sig_array_local = (int *) malloc(LINE_SIZE * sizeof(int));
	sig_read_ar = (int *) malloc(LINE_SIZE * sizeof(int));

	for (i=0; i<LINE_SIZE; i++) 
		sig_array_local[i] = (ME+1) * 100;

	//for (i=0; i<NUES; i++) {
	//RCCE_get((t_vcharp)(&sig_read_ar), (t_vcharp)(&sig_array), NUES * sizeof(int), ME);
	 //printf("I am %d and init signal of %d is %d\n",ME,i,sig_array_local[i]);
	//}
	/*
		if (ME == 0) sig_source = 0;
		else if (ME == 1) sig_source = 100;
		else if (ME == 2) sig_source = 200;
		else sig_source = 300;
	*/

	//for (i=0; i<NUES; i++)
	//if (ME == 0) {//ME*NUES

	error = RCCE_put((t_vcharp)(&sig_array[ME*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), ME);
	if (error != RCCE_SUCCESS) {
		RCCE_error_string(error, error_str, &str_len);
		printf("I am %d and i got an error in put with descr %s\n",ME,error_str);
	}
		//RCCE_put((t_vcharp)(sig_array), (t_vcharp)(&sig_source), NUES*sizeof(int), ME);
	RCCE_get((t_vcharp)(&sig_read_ar[0]), (t_vcharp)(&sig_array[ME*LINE_SIZE]), LINE_SIZE * sizeof(int), ME);

	for (i=0; i<LINE_SIZE; i++) {
		//RCCE_get((t_vcharp)(&sig_read_ar), (t_vcharp)(&sig_array), NUES * sizeof(int), ME);
	  printf("I am %d and signal of %d is %d\n",ME,i,sig_read_ar[i]);
	}
	//}
	//}

	RCCE_barrier(&RCCE_COMM_WORLD);

	/*if (ME == 0) {
		for (i=0; i<100000; i++)
			dummy++;

		printf("I am 0 and i finished my first round my source is %d\n",sig_source);
		//RCCE_wait_until(flag_rec_sig, RCCE_FLAG_UNSET);
		//RCCE_wait_until(flag_send_sig, RCCE_FLAG_SET);
		RCCE_flag_write(&flag_rec_sig, RCCE_FLAG_UNSET, ME);	
		for (i=0; i<NUES; i++) {
			RCCE_get((t_vcharp)(&sig_read), (t_vcharp)(&sig_array[i]), sizeof(int), ME);
			printf("I am %d and signal of %d is %d\n",ME,i,sig_read);
		}
		RCCE_flag_write(&flag_rec_sig, RCCE_FLAG_SET, ME);	
	} else {
		RCCE_flag_read(flag_rec_sig, &receiver_status, 0);
		if (receiver_status == RCCE_FLAG_UNSET) printf("I am %d kai gamietai to sumpan\n",ME);	
		while (receiver_status == RCCE_FLAG_UNSET) {//unable to send sig
			//printf("I am %d and flag_rec_sig is taken");
			RCCE_flag_read(flag_rec_sig, &receiver_status, 0);
		}
		//RCCE_flag_write(&flag_rec_sig, RCCE_FLAG_SET, 0);

		RCCE_put((t_vcharp)(&sig_array[ME]),(t_vcharp)(&sig_source), sizeof(int), 0);
		
		//RCCE_flag_write(&flag_rec_sig, RCCE_FLAG_UNSET, 0);
		//RCCE_flag_write(&flag_send_sig, RCCE_FLAG_SET, 0);
	
		printf("I am %d and i have sent my signal %d\n",ME,sig_source);
	}*/

  RCCE_finalize();

  return(0);
}

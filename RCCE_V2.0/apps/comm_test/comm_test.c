//#include <string.h>
#include <stdio.h>
#include "RCCE.h"

#define max(x,y) ((x)>(y)?(x):(y))
#define LINE_SIZE 8 //number of bytes
#define SIG_IDAG_FIND_IDAGS 123
#define SIG_ACK 257

int RCCE_APP(int argc, char **argv){
	int ME, NUES, i, dummy=0;
  int *sig_array, *sig_array_local, sig_source, *sig_read_ar, *data_array, *data_array_local;
	RCCE_FLAG flag_signals_enabled,flag_data_written;
	RCCE_FLAG_STATUS receiver_status;
	int error, str_len;
	char error_str[64];
  	
	RCCE_init(&argc, &argv);

  	//  RCCE_debug_set(RCCE_DEBUG_ALL);
	ME = RCCE_ue();
	NUES = RCCE_num_ues();
	RCCE_flag_alloc(&flag_signals_enabled);
	RCCE_flag_alloc(&flag_data_written);
	RCCE_flag_write(&flag_signals_enabled, RCCE_FLAG_UNSET, ME);
	RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, ME);
	sig_array = (int *) RCCE_malloc(NUES * LINE_SIZE * sizeof(int));//NUES * NUES
	data_array = (int *) RCCE_malloc(LINE_SIZE * sizeof(int));
	sig_array_local = (int *) malloc(LINE_SIZE * sizeof(int));
	data_array_local = (int *) malloc(LINE_SIZE * sizeof(int));
	sig_read_ar = (int *) malloc(LINE_SIZE * sizeof(int));

	//for (i=0; i<LINE_SIZE; i++) 
	//	sig_array_local[i] = (ME+1) * 100;
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
	if (ME == 0) {//ME*NUES
		for (i=0; i<LINE_SIZE; i++) 
			sig_array_local[i] = SIG_ACK;

		//initial delay
		//for (i=0; i<10000; i++) dummy++;

		RCCE_flag_write(&flag_signals_enabled, RCCE_FLAG_SET, ME);

		for (i=0; i<500000; i++) dummy++;

		//RCCE_flag_write(&flag_signals_enabled, RCCE_FLAG_UNSET, ME);

		for (i=1; i<NUES; i++) {
			error = RCCE_get((t_vcharp)(&sig_read_ar[0]), (t_vcharp)(&sig_array[i*LINE_SIZE]), LINE_SIZE * sizeof(int), ME);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				printf("I am 0 and i got an error in get from %d with descr %s\n",i,error_str);
			} else {

				while (sig_read_ar[0] != SIG_IDAG_FIND_IDAGS)
					error = RCCE_get((t_vcharp)(&sig_read_ar[0]), (t_vcharp)(&sig_array[i*LINE_SIZE]), LINE_SIZE * sizeof(int), ME);
					if (error != RCCE_SUCCESS) {
						RCCE_error_string(error, error_str, &str_len);
						printf("I am 0 and i got an error in get from %d with descr %s\n",i,error_str);
					}

				//if (sig_read_ar[0] == SIG_IDAG_FIND_IDAGS) {
				//} else printf("I am 0 and i did not get SIG_IDAG_FIND_IDAGS from %d\n",i);
				RCCE_flag_read(flag_signals_enabled, &receiver_status, i);
				if (receiver_status == RCCE_FLAG_UNSET) printf("I am %d kai signals of %d are disabled\n",ME,i);	
				while (receiver_status == RCCE_FLAG_UNSET) {//unable to send sig
					//printf("I am %d and flag_rec_sig is taken");
					RCCE_flag_read(flag_signals_enabled, &receiver_status, i);
				}

				error = RCCE_put((t_vcharp)(&sig_array[ME*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), i);
				if (error != RCCE_SUCCESS) {
					RCCE_error_string(error, error_str, &str_len);
					printf("I am %d and i got an error in put with descr %s\n",ME,error_str);
				}					
	
				RCCE_flag_write(&flag_data_written, RCCE_FLAG_UNSET, ME);
				RCCE_wait_until(flag_data_written, RCCE_FLAG_SET);

				error = RCCE_get((t_vcharp)(&data_array_local[0]), (t_vcharp)(&data_array[0]), LINE_SIZE * sizeof(int), ME);
				if (error != RCCE_SUCCESS) {
					RCCE_error_string(error, error_str, &str_len);
					printf("I am 0 and i got an error in get data from %d with descr %s\n",i,error_str);
				} else printf("Data from %d are %d\n",i,data_array_local[0]);
			}
		}

		//RCCE_flag_write(&flag_signals_enabled, RCCE_FLAG_SET, ME);

		//for (i=0; i<500000; i++) dummy++;
	} else {
		for (i=0; i<LINE_SIZE; i++) {
			sig_array_local[i] = SIG_IDAG_FIND_IDAGS;	
			data_array_local[i] = ME * 100;
		}

		RCCE_flag_read(flag_signals_enabled, &receiver_status, 0);
		if (receiver_status == RCCE_FLAG_UNSET) printf("I am %d kai signals of 0 are disabled\n",ME);	
		while (receiver_status == RCCE_FLAG_UNSET) {//unable to send sig
			//printf("I am %d and flag_rec_sig is taken");
			RCCE_flag_read(flag_signals_enabled, &receiver_status, 0);
		}
		
		printf("I am %d and i am going to write my signal\n",ME);

		error = RCCE_put((t_vcharp)(&sig_array[ME*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), 0);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			printf("I am %d and i got an error in put with descr %s\n",ME,error_str);
		}

		printf("I am %d and i come here a\n",ME);

		RCCE_flag_write(&flag_signals_enabled, RCCE_FLAG_SET, ME);

		for (i=0; i<50000; i++) dummy++;

		printf("I am %d and i come here b\n",ME);

		//RCCE_flag_write(&flag_signals_enabled, RCCE_FLAG_UNSET, ME);

		error = RCCE_get((t_vcharp)(&sig_read_ar[0]), (t_vcharp)(&sig_array[0]), LINE_SIZE * sizeof(int), ME);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			printf("I am 0 and i got an error in get from %d with descr %s\n",i,error_str);
		}  	
		
		while (sig_read_ar[0] != SIG_ACK) {
			error = RCCE_get((t_vcharp)(&sig_read_ar[0]), (t_vcharp)(&sig_array[0]), LINE_SIZE * sizeof(int), ME);
			if (error != RCCE_SUCCESS) {
				RCCE_error_string(error, error_str, &str_len);
				printf("I am 0 and i got an error in get from %d with descr %s\n",i,error_str);
			}
		}

		error = RCCE_put((t_vcharp)(&data_array[0]), (t_vcharp)(&data_array_local[0]), LINE_SIZE * sizeof(int), 0);
		if (error != RCCE_SUCCESS) {
			RCCE_error_string(error, error_str, &str_len);
			printf("I am %d and i got an error in put data with descr %s\n",ME,error_str);
		}	else {
			printf("I am %d and data successfully written in 0\n",ME);
			RCCE_flag_write(&flag_data_written, RCCE_FLAG_SET, 0);
		}
	}

	/*error = RCCE_put((t_vcharp)(&sig_array[ME*LINE_SIZE]), (t_vcharp)(&sig_array_local[0]), LINE_SIZE * sizeof(int), ME);
	if (error != RCCE_SUCCESS) {
		RCCE_error_string(error, error_str, &str_len);
		printf("I am %d and i got an error in put with descr %s\n",ME,error_str);
	}
		//RCCE_put((t_vcharp)(sig_array), (t_vcharp)(&sig_source), NUES*sizeof(int), ME);
	RCCE_get((t_vcharp)(&sig_read_ar[0]), (t_vcharp)(&sig_array[ME*LINE_SIZE]), LINE_SIZE * sizeof(int), ME);

	for (i=0; i<LINE_SIZE; i++) {
		//RCCE_get((t_vcharp)(&sig_read_ar), (t_vcharp)(&sig_array), NUES * sizeof(int), ME);
	  printf("I am %d and signal of %d is %d\n",ME,i,sig_read_ar[i]);
	}*/
	
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

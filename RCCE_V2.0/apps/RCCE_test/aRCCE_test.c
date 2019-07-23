#include <string.h>
#include <stdio.h>
#include "RCCE.h"

#define max(x,y) ((x)>(y)?(x):(y))

int RCCE_APP(int argc, char **argv){
  int YOU, NUES;
  char *sig_array, sig_source;
	RCCE_FLAG flag_rec_sig,flag_send_sig);
	RCCE_FLAG_STATUS receiver_status;

  RCCE_init(&argc, &argv);

  //  RCCE_debug_set(RCCE_DEBUG_ALL);
  ME = RCCE_ue();
	NUES = RCCE_num_ues();
	RCCE_flag_alloc(&flag_rec_sig);
	RCCE_flag_alloc(&flag_send_sig);
	RCCE_flag_write(&flag_rec_sig, RCCE_FLAG_UNSET, ME);
	RCCE_flag_write(&flag_send_sig, RCCE_FLAG_UNSET, ME);
	sig_array = (char *) RCCE_malloc(NUES * sizeof(char));

	if (ME == 0) sig_source = '\0';
	else if (ME == 1) sig_source = 'a';
	else if (ME == 2) sig_source = 'b';
	else sig_source = 'c';

	for (i=0; i<NUES; i++)
		RCCE_put(&sig_array[i], &sig_source, sizeof(char), ME);

	RCCE_barrier(&RCCE_COMM_WORLD);

	if (ME == 0) {
		for (i=0; i<1000; i++)
			dummy++;

		printf("I am 0 and i finished my first round\n");
		RCCE_wait_until(flag_rec_sig, RCCE_FLAG_UNSET);

		for (i=0; i<NUES; i++) {
			RCCE_get(&sig_read, &sig_array[i], sizeof(char), ME);
			printf("I am %d and signal of %d is %d",ME,i,sig_read);
		}
			
	} else {
		RCCE_flag_read(flag_rec_sig, &receiver_status, 0);
		while (receiver_status == RCCE_FLAG_SET) {
			//printf("I am %d and flag_rec_sig is taken");
			RCCE_flag_read(flag_rec_sig, &receiver_status, 0);
		}
		RCCE_flag_write(&flag_rec_sig, RCCE_FLAG_SET, 0);

		RCCE_put(&sig_array[ME], &sig_source, sizeof(char), 0);
		
		RCCE_flag_write(&flag_rec_sig, RCCE_FLAG_UNSET, 0);

		printf("I am %d and i sent my signal");
	}

  RCCE_finalize();

  return(0);
}

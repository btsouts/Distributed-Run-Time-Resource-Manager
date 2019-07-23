// 
// Copyright 2010 Intel Corporation
// 
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
// 
//        http://www.apache.org/licenses/LICENSE-2.0
// 
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
// 
#include "RCCE.h"
void print_dividers(void);

#include <stdio.h>

/* hardwired predefined constants */
#define NX      16
#define NY      25
#define NXNY    ((NX)*(NY))
#define NXNY1   ((NX)*(NY-1))
#define NXNY2   ((NX)*(NY-2))

#define O1      0
#define O2      NX-1
#define O3      NX
#define O4      NX+1
#define O5      2*(NX)
#define W1      0.25
#define W2      0.25
#define W4      0.25
#define W5      0.25
#define W3      -1.0

/* initialization;
   resulting 2D data set represented by a[] is as follows, where
   first and last row of each strip are fixed boundary values (1's
   and 2's) or fringe data copied from strips on neighboring tiles.


           1 1 1 1 1 1 1 1 1 1
           0 0 0 0 0 0 0 0 0 0
           ...................       CORE 0
           0 0 0 0 0 0 0 0 0 0
           0 0 0 0 0 0 0 0 0 0
           

           0 0 0 0 0 0 0 0 0 0
           0 0 0 0 0 0 0 0 0 0
           ...................       CORE 1
           0 0 0 0 0 0 0 0 0 0
           0 0 0 0 0 0 0 0 0 0


           0 0 0 0 0 0 0 0 0 0
           0 0 0 0 0 0 0 0 0 0
           ...................       CORE 2
           0 0 0 0 0 0 0 0 0 0
           0 0 0 0 0 0 0 0 0 0


           0 0 0 0 0 0 0 0 0 0
           0 0 0 0 0 0 0 0 0 0
           ...................       CORE NTILES-1
           0 0 0 0 0 0 0 0 0 0
           2 2 2 2 2 2 2 2 2 2

*/

int RCCE_APP(int argc, char **argv){

  float     a[NXNY];
  int       i, offset, iter=3;
  int       fdiv, vlevel;
  int       ID, ID_right, ID_left;
  int       NTILES1;
  double    time;
  RCCE_REQUEST req;

  RCCE_init(&argc, &argv);
 
  //  RCCE_debug_set(RCCE_DEBUG_ALL);

  NTILES1 = RCCE_num_ues()-1;
  ID = RCCE_ue();


  ID_right = (ID+1)%RCCE_num_ues();
  ID_left = (ID-1+RCCE_num_ues())%RCCE_num_ues();

// set the relevant areas of the board to the default frequency and voltage
  RCCE_set_frequency_divider(8, &fdiv);
  if (ID==0)print_dividers();

  //  return(0);
  //    RCCE_iset_power(3, &req, &fdiv, &vlevel);
  //  if (ID==RCCE_power_domain_master()) printf("UE %d computed vlevel %d\n", ID,vlevel);
  //  RCCE_wait_power(&req);
  //  RCCE_set_frequency_divider(3, &fdiv);

  if (NX%8) {
    printf("Grid width should be multiple of 8: %d\n", NX);
    exit(1);
  }
  if (argc>1) iter=atoi(*++argv);
  if (!ID) printf("Core %d Executing %d iterations\n", ID, iter);

  /* initialize array a on all tiles; this stuffs a into private caches  */

  for (offset=0,       i=0; i<NXNY; i++) a[i+offset] = 0.0;
  if (ID == 0) 
     for (offset=0,    i=0; i<NX;   i++) a[i+offset] = 1.0;
  if (ID == NTILES1) 
     for (offset=NXNY1,i=0; i<NX;   i++) a[i+offset] = 2.0;

  /* main loop */

  if (ID==0) time = RCCE_wtime();

  while ((iter--)>0){

    RCCE_iset_power(3, &req, &fdiv, &vlevel);
    if (ID==RCCE_power_domain_master()) 
      printf("asked for divider 3, received %d, voltage level %d\n", fdiv, vlevel); 
    fflush(NULL);
      if (!(iter%100)) printf("Iteration %d\n", iter);
    /* start with copying fringe data to neighboring tiles; we need to
       group semantic send/recv pairs together to avoid deadlock         */
    if (ID_right!=0) RCCE_send((char*)(&a[NXNY2]), NX*sizeof(float), ID_right);
    if (ID != 0)     RCCE_recv((char*)(&a[0]),     NX*sizeof(float), ID_left);

    RCCE_wait_power(&req);
    if (ID!=0)       RCCE_send((char *)(&a[NX]),    NX*sizeof(float), ID_left);
    if (ID_right!=0) RCCE_recv((char *)(&a[NXNY1]), NX*sizeof(float), ID_right);

    RCCE_iset_power(3, &req, &fdiv, &vlevel);
    RCCE_set_frequency_divider(3, &fdiv);

    if (ID==RCCE_power_domain_master())    
      printf("asked for divider 3, received %d, voltage level %d\n", fdiv, vlevel);
    fflush(NULL);

    /* apply the stencil operation                                       */
    for (i=0; i<NXNY2; i++) {
      a[i+O3] +=
         W1*a[i+O1] + W2*a[i+O2] + W3*a[i+O3] + W4*a[i+O4] + W5*a[i+O5];
    }
    RCCE_wait_power(&req);
  }


//  /* print result strip by strip; this would not be done on RC */
//  for (int id=0; id<=NTILES1; id++) {
//    RCCE_barrier(&RCCE_COMM_WORLD);
//    if (ID==id) {
//      int start = NX; int end = NXNY1;
//      if (ID==0) start = 0;
//      if (ID == NTILES1) end = NXNY;
//      for (offset=0, i=start; i<end; i++) {
//        if (!(i%NX)) printf("\n");
////        comment out next line and uncomment subsequent three to print error
//        printf("%1.5f ",a[i+offset]); fflush(stdout);
////        int jj=i/NX+(ID*(NY-1));
////        double aexact=1.0+(double)jj/((NTILES1+1)*(NY-1));
////        printf("%f ",a[i+offset]-aexact);
//      }
//    }
//  }
//  RCCE_barrier(&RCCE_COMM_WORLD);
//  if (ID==0) { 
//    printf("\n");
//    time = RCCE_wtime()-time;
//    printf("Total time: %lf\n", time);
//  }

//reset the relevant areas of the board to the default frequency and voltage
//  RCCE_set_frequency_divider(8, &fdiv);
//  RCCE_iset_power(2, &req, &fdiv, &vlevel);
//  if (ID==RCCE_power_domain_master()) printf("UE %d computed vlevel %d\n", ID,vlevel);
//  RCCE_wait_power(&req);

//  RCCE_set_frequency_divider(3, &fdiv);
  RCCE_barrier(&RCCE_COMM_WORLD);
  if (ID==0)print_dividers();

  RCCE_finalize();

  return(0);
}

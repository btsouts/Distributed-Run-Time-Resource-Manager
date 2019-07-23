#ifndef __MY_RTRM_H__
#define __MY_RTRM_H__

#include "variables.h"
#define CLUSTER_SIZE 6
//#define MAX_STR_NAME_SIZE 256

#ifdef PLAT_SCC
#include "RCCE.h"
#else
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#define MAX_SIGNAL_LIST_LEN 256
#endif

int PREPARE_ACCEPT_SENT;
int CORE_HAS_ACCEPTED_VALUE;
int SIG_LEARN_SENT;
int proposal_number_personal;

#ifdef PLAT_SCC
  RCCE_FLAG proposal_number_lock;
#else
  sem_t *cores_detected;
#endif
int **core_detected;
#endif

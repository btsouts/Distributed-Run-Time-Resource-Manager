#
# Makefile
#
# my_rtrm
#

ifeq ($(PLATFORM),SCC)
	CFLAGS = -Wall -g
	SHELL=sh
	RCCEROOT=./RCCE_V2.0
	include $(RCCEROOT)/common/symbols
	PLATFORM_INCLUDES = $(RCCEINCLUDE)/RCCE.h
	MY_FLAGS += -DPLAT_SCC
else
	CCOMPILE = gcc
	CFLAGS = -O0 -Wall -g #-O0 -Wextra
	MY_FLAGS += -DPLAT_LINUX
	#PLATFORM_INCLUDES = variables.h structs.h macros.h
endif

C_FILES_DIR = ./src
H_FILES_DIR = ./src/include
PLATFORM_INCLUDES += $(H_FILES_DIR)/variables.h $(H_FILES_DIR)/structs.h $(H_FILES_DIR)/macros.h
OBJS=my_rtrm.o libfun.o nocfun.o sigaux.o controller.o common_node.o signal_handlers.o scc_signals.o idag_defs.o paxos_signal_handlers.o apps.o resource_negotiation.o $(ARCHIVE)
exec_name=my_rtrm
#
#PAXOS OBJECTS
#
ifeq ($(SCEN),BASIC_PAXOS)
	MY_FLAGS += -DBASIC_PAXOS
else
	MY_FLAGS += -DPLAIN
endif

ifeq ($(FAILURE),CONTROLLER)
	MY_FLAGS += -DCONTROLLER
else ifeq ($(FAILURE),MANAGER)
	MY_FLAGS += -DMANAGER
else ifeq ($(FAILURE),WORKER)
	MY_FLAGS += -DWORKER
endif

ifeq ($(DETECTOR), PFD)
	MY_FLAGS += -DPFD
else ifeq ($(DETECTOR), EPFD)
	MY_FLAGS += -DEPFD
else ifeq ($(DETECTOR), tPFD)
	MY_FLAGS += -DtPFD
else ifeq ($(DETECTOR), tEPFD)
	MY_FLAGS += -DtEPFD
endif

#
#
#

#
#PAXOS SCENARIA
#
ifeq ($(IDAG_CONF),PAXOS_1)
  MY_FLAGS += -DPAXOS_CONF_1 -DIDAGS_4
endif
#
#
#
ifeq ($(RESOURCE_ALGO), ORIGINAL)
	MY_FLAGS += -DRESOURCE_ALGO_ORIG
else ifeq ($(RESOURCE_ALGO), UPDATED)
	MY_FLAGS += -DRESOURCE_ALGO_UPDATED
else ifeq ($(RESOURCE_ALGO), UPDATED_GENEROUS)
        MY_FLAGS += -DRESOURCE_ALGO_UPDATED_GENEROUS
else
	MY_FLAGS += -DRESOURCE_ALGO_ORIG
endif

ifeq ($(SINGLE_WORKER),1)
  MY_FLAGS += -DSINGLE_WORKER
endif

ifeq ($(ADAM_SIM),1)
	MY_FLAGS += -DADAM_SIM

	ifneq ($(SINGLE_IDAG),1)	 
		MY_FLAGS += -DSINGLE_IDAG
	endif

	exec_name=my_rtrm_adam
endif

ifeq ($(APPS_SIM),1)
	MY_FLAGS += -DARTIFICIAL_APPS_SIM
endif

ifeq ($(SINGLE_IDAG),1)
  MY_FLAGS += -DSINGLE_IDAG
endif

ifeq ($(VERBOSE_WORKER),1)
  MY_FLAGS += -DVERBOSE_WORKER
endif

ifeq ($(EXTRA_DELAY),1)
  MY_FLAGS += -DEXTRA_DELAY
endif

ifeq ($(IDAG_SLEEP),1)
  MY_FLAGS += -DIDAG_SLEEP

  exec_name=my_rtrm.idag_sleep	
endif

ifeq ($(LOW_VOLTAGE),0)
  MY_FLAGS += -DLOW_VOLTAGE_0
endif

ifeq ($(LOW_VOLTAGE),1)
  MY_FLAGS += -DLOW_VOLTAGE_1
endif

ifeq ($(LOW_VOLTAGE),2)
  MY_FLAGS += -DLOW_VOLTAGE_2
endif

ifeq ($(LOW_VOLTAGE),3)
  MY_FLAGS += -DLOW_VOLTAGE_3
endif

ifeq ($(LOW_VOLTAGE),4)
  MY_FLAGS += -DLOW_VOLTAGE_4
endif

ifeq ($(LOW_VOLTAGE),5)
  MY_FLAGS += -DLOW_VOLTAGE_5
endif

ifeq ($(GREEDY_MANAGER),1)
  MY_FLAGS += -DGREEDY_MANAGER
endif

$(exec_name):$(OBJS) 
	cd ./bin && $(CCOMPILE) $(CFLAGS) $(MY_FLAGS) -o $(exec_name) $(OBJS) -pthread -lrt -lm

nocfun.o: $(C_FILES_DIR)/noc_functions.c $(H_FILES_DIR)/noc_functions.h $(H_FILES_DIR)/my_rtrm.h $(H_FILES_DIR)/idag_defs.h $(PLATFORM_INCLUDES)
	$(CCOMPILE) $(CFLAGS) $(MY_FLAGS) -o ./bin/nocfun.o -c $(C_FILES_DIR)/noc_functions.c

libfun.o: $(C_FILES_DIR)/libfunctions.c $(H_FILES_DIR)/libfunctions.h $(H_FILES_DIR)/my_rtrm.h $(PLATFORM_INCLUDES)
	$(CCOMPILE) $(CFLAGS) $(MY_FLAGS) -o ./bin/libfun.o -c $(C_FILES_DIR)/libfunctions.c

sigaux.o: $(C_FILES_DIR)/sig_aux.c $(H_FILES_DIR)/sig_aux.h $(H_FILES_DIR)/my_rtrm.h $(H_FILES_DIR)/signal_handlers.h $(H_FILES_DIR)/paxos_signal_handlers.h $(PLATFORM_INCLUDES)
	$(CCOMPILE) $(CFLAGS) $(MY_FLAGS) -o ./bin/sigaux.o -c $(C_FILES_DIR)/sig_aux.c

scc_signals.o: $(C_FILES_DIR)/scc_signals.c $(H_FILES_DIR)/scc_signals.h $(H_FILES_DIR)/my_rtrm.h $(H_FILES_DIR)/signal_handlers.h $(H_FILES_DIR)/paxos_signal_handlers.h $(PLATFORM_INCLUDES)
	$(CCOMPILE) $(CFLAGS) $(MY_FLAGS) -o ./bin/scc_signals.o -c $(C_FILES_DIR)/scc_signals.c

controller.o: $(C_FILES_DIR)/controller_core.c $(H_FILES_DIR)/controller_core.h $(H_FILES_DIR)/libfunctions.h $(H_FILES_DIR)/my_rtrm.h $(H_FILES_DIR)/noc_functions.h $(H_FILES_DIR)/sig_aux.h $(H_FILES_DIR)/signal_handlers.h $(H_FILES_DIR)/paxos_signal_handlers.h $(H_FILES_DIR)/scc_signals.h $(H_FILES_DIR)/idag_defs.h $(PLATFORM_INCLUDES)
	$(CCOMPILE) $(CFLAGS) $(MY_FLAGS) -o ./bin/controller.o -c $(C_FILES_DIR)/controller_core.c

common_node.o: $(C_FILES_DIR)/common_core.c $(H_FILES_DIR)/common_core.h $(H_FILES_DIR)/libfunctions.h $(H_FILES_DIR)/my_rtrm.h $(H_FILES_DIR)/noc_functions.h $(H_FILES_DIR)/sig_aux.h $(H_FILES_DIR)/signal_handlers.h $(H_FILES_DIR)/paxos_signal_handlers.h $(H_FILES_DIR)/apps.h $(PLATFORM_INCLUDES)
	$(CCOMPILE) $(CFLAGS) $(MY_FLAGS) -o ./bin/common_node.o -c $(C_FILES_DIR)/common_core.c

my_rtrm.o: $(C_FILES_DIR)/my_rtrm.c $(H_FILES_DIR)/libfunctions.h $(H_FILES_DIR)/my_rtrm.h $(H_FILES_DIR)/noc_functions.h $(H_FILES_DIR)/sig_aux.h $(H_FILES_DIR)/controller_core.h $(H_FILES_DIR)/common_core.h $(H_FILES_DIR)/signal_handlers.h $(H_FILES_DIR)/paxos_signal_handlers.h $(H_FILES_DIR)/scc_signals.h $(H_FILES_DIR)/idag_defs.h $(PLATFORM_INCLUDES)
	$(CCOMPILE) $(CFLAGS) $(MY_FLAGS) -o ./bin/my_rtrm.o -c $(C_FILES_DIR)/my_rtrm.c

signal_handlers.o: $(C_FILES_DIR)/signal_handlers.c $(H_FILES_DIR)/libfunctions.h $(H_FILES_DIR)/my_rtrm.h $(H_FILES_DIR)/noc_functions.h $(H_FILES_DIR)/sig_aux.h $(H_FILES_DIR)/scc_signals.h $(H_FILES_DIR)/resource_negotiation.h $(PLATFORM_INCLUDES)
	$(CCOMPILE) $(CFLAGS) $(MY_FLAGS) -o ./bin/signal_handlers.o -c $(C_FILES_DIR)/signal_handlers.c

paxos_signal_handlers.o: $(C_FILES_DIR)/paxos_signal_handlers.c $(H_FILES_DIR)/libfunctions.h $(H_FILES_DIR)/my_rtrm.h $(H_FILES_DIR)/noc_functions.h $(H_FILES_DIR)/sig_aux.h $(H_FILES_DIR)/scc_signals.h $(H_FILES_DIR)/signal_handlers.h $(H_FILES_DIR)/sig_aux.h $(PLATFORM_INCLUDES)
	$(CCOMPILE) $(CFLAGS) $(MY_FLAGS) -o ./bin/paxos_signal_handlers.o -c $(C_FILES_DIR)/paxos_signal_handlers.c
	
idag_defs.o: $(C_FILES_DIR)/idag_defs.c $(H_FILES_DIR)/idag_defs.h $(H_FILES_DIR)/my_rtrm.h $(PLATFORM_INCLUDES)
	$(CCOMPILE) $(CFLAGS) $(MY_FLAGS) -o ./bin/idag_defs.o -c $(C_FILES_DIR)/idag_defs.c
	
apps.o: $(C_FILES_DIR)/apps.c $(H_FILES_DIR)/apps.h $(H_FILES_DIR)/scc_signals.h $(H_FILES_DIR)/libfunctions.h $(PLATFORM_INCLUDES)
	$(CCOMPILE) $(CFLAGS) $(MY_FLAGS) -o ./bin/apps.o -c $(C_FILES_DIR)/apps.c	

resource_negotiation.o: $(C_FILES_DIR)/resource_negotiation.c $(H_FILES_DIR)/resource_negotiation.h $(H_FILES_DIR)/apps.h $(H_FILES_DIR)/noc_functions.h $(PLATFORM_INCLUDES)
	$(CCOMPILE) $(CFLAGS) $(MY_FLAGS) -o ./bin/resource_negotiation.o -c $(C_FILES_DIR)/resource_negotiation.c

clean:
	rm -f ./bin/my_rtrm ./bin/my_rtrm_adam ./bin/*.o 
	rm -rf ./../scenaria/6x6/paxos_log_files/*
	rm -rf ./../scenaria/6x6/log_files/*
	rm -rf ./../scenaria/6x6/app_logs/*

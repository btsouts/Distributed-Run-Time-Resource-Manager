#
# Makefile
#
# my_rtrm
#

#CC = gcc
#CFLAGS = -O0 -Wall -g #-Wextra

SHELL=sh
RCCEROOT=../..
include $(RCCEROOT)/common/symbols

OBJS=my_rtrm.o libfun.o nocfun.o sigaux.o idle_agent.o common_node.o signal_handlers.o scc_signals.o  $(ARCHIVE)

#default:
#	@echo "Usage: make pingpong "

#comm_test: $(OBJS)
#	$(CCOMPILE) -o comm_test $(OBJS) $(CFLAGS)

#comm_test.o: comm_test.c $(RCCEINCLUDE)/RCCE.h
#	$(CCOMPILE) -c $(CFLAGS) comm_test.c  

#clean:
#	@ rm -f *.o comm_test 

# my_rtrm.o libfun.o nocfun.o sigaux.o idle_agent.o common_node.o signal_handlers.o scc_signals.o
my_rtrm: $(OBJS) 
	$(CCOMPILE) $(CFLAGS) -o my_rtrm $(OBJS) -lrt -lm

nocfun.o: noc_functions.c noc_functions.h my_rtrm.h	$(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) $(CFLAGS) -o nocfun.o -c noc_functions.c

libfun.o: libfunctions.c libfunctions.h my_rtrm.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) $(CFLAGS) -o libfun.o -c libfunctions.c

sigaux.o: sig_aux.c sig_aux.h my_rtrm.h signal_handlers.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) $(CFLAGS) -o sigaux.o -c sig_aux.c

scc_signals.o: scc_signals.c scc_signals.h my_rtrm.h signal_handlers.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) $(CFLAGS) -o scc_signals.o -c scc_signals.c

idle_agent.o: idle_agent.c idle_agent.h libfunctions.h my_rtrm.h noc_functions.h sig_aux.h signal_handlers.h scc_signals.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) $(CFLAGS) -o idle_agent.o -c idle_agent.c

common_node.o: common_node.c common_node.h libfunctions.h my_rtrm.h noc_functions.h sig_aux.h signal_handlers.h scc_signals.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) $(CFLAGS) -o common_node.o -c common_node.c

my_rtrm.o: my_rtrm.c libfunctions.h my_rtrm.h noc_functions.h sig_aux.h idle_agent.h common_node.h signal_handlers.h scc_signals.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) $(CFLAGS) -o my_rtrm.o -c my_rtrm.c

signal_handlers.o: signal_handlers.c libfunctions.h my_rtrm.h noc_functions.h sig_aux.h scc_signals.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) $(CFLAGS) -o signal_handlers.o -c signal_handlers.c

clean:
	rm -f my_rtrm my_rtrm.o libfun.o nocfun.o sigaux.o common_node.o idle_agent.o signal_handlers.o scc_signals.o

clean_pipes:
	rm -f ./pipes/*

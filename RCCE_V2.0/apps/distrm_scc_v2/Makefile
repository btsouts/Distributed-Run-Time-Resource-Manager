#
# Makefile
#
# DistRM
#

SHELL=sh
RCCEROOT=../..
include $(RCCEROOT)/common/symbols

OBJS=distrm.o libfun.o nocfun.o sigaux.o idle_agent.o common_node.o signal_handlers.o scc_signals.o  $(ARCHIVE)

distrm: $(OBJS) 
	$(CCOMPILE) $(CFLAGS) -o distrm $(OBJS) -lrt -lm

nocfun.o: noc_functions.c noc_functions.h distrm.h	$(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) $(CFLAGS) -o nocfun.o -c noc_functions.c

libfun.o: libfunctions.c libfunctions.h distrm.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) $(CFLAGS) -o libfun.o -c libfunctions.c

sigaux.o: sig_aux.c sig_aux.h distrm.h signal_handlers.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) $(CFLAGS) -o sigaux.o -c sig_aux.c

scc_signals.o: scc_signals.c scc_signals.h distrm.h signal_handlers.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) $(CFLAGS) -o scc_signals.o -c scc_signals.c

idle_agent.o: idle_agent.c idle_agent.h libfunctions.h distrm.h noc_functions.h sig_aux.h signal_handlers.h scc_signals.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) $(CFLAGS) -o idle_agent.o -c idle_agent.c

common_node.o: common_node.c common_node.h libfunctions.h distrm.h noc_functions.h sig_aux.h signal_handlers.h scc_signals.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) $(CFLAGS) -o common_node.o -c common_node.c

distrm.o: distrm.c libfunctions.h distrm.h noc_functions.h sig_aux.h idle_agent.h common_node.h signal_handlers.h scc_signals.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) $(CFLAGS) -o distrm.o -c distrm.c

signal_handlers.o: signal_handlers.c libfunctions.h distrm.h noc_functions.h sig_aux.h scc_signals.h $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) $(CFLAGS) -o signal_handlers.o -c signal_handlers.c

clean:
	rm -f distrm distrm.o libfun.o nocfun.o sigaux.o common_node.o idle_agent.o signal_handlers.o scc_signals.o

clean_pipes:
	rm -f ./pipes/*

SHELL=sh
RCCEROOT=../..
include $(RCCEROOT)/common/symbols

OBJS=scc_lib_test.o  $(ARCHIVE)

default:
	@echo $(CCOMPILE)
scc_lib_test: $(OBJS)
	$(CCOMPILE) -o scc_lib_test $(OBJS) $(CFLAGS) -I/lib/ -lrt

scc_lib_test.o: scc_lib_test.c $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) -c $(CFLAGS) -I/lib/ scc_lib_test.c  

clean:
	@ rm -f *.o comm_test 

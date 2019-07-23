SHELL=sh
RCCEROOT=../..
include $(RCCEROOT)/common/symbols

OBJS=comm_test.o  $(ARCHIVE)

default:
	@echo "Usage: make pingpong "
	@echo "       make pingpong-xt "
	@echo "       make clean"

comm_test: $(OBJS)
	$(CCOMPILE) -o comm_test $(OBJS) $(CFLAGS)

comm_test.o: comm_test.c $(RCCEINCLUDE)/RCCE.h
	$(CCOMPILE) -c $(CFLAGS) comm_test.c  

clean:
	@ rm -f *.o comm_test 

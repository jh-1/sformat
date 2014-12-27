ifeq ($(strip($BCCOPTS)),)
BCCOPTS = -0 -ansi -c
endif

all: sformat.o

sformat.o: sformat.c
	bcc -0 -ansi -c sformat.c

tester: test_sformat.c sformat.o
	bcc -0 -ansi test_sformat.c sformat.o -o tester


ifeq ($(strip($BCCOPTS)),)
BCCOPTS = -0 -ansi -c
endif

OBJ := obj/
SRC := src/

all: $(OBJ)sformat.o

$(OBJ)sformat.o: $(SRC)sformat.c
	mkdir -p $(OBJ)
	bcc -0 -ansi -c -o $@ $<

tester: test_sformat.c sformat.o
	bcc -0 -ansi test_sformat.c sformat.o -o tester


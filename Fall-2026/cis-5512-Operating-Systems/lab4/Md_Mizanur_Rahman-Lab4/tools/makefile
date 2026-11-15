CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
CPPFLAGS = -I../include
LIB = ../obj/libsng.a

all: tshctl launch demo_worker

tshctl: tshctl.c ../include/tshlib.h ../include/tsh_shell.h $(LIB)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tshctl.c -L../obj -lsng

launch: launch.c ../include/tshlib.h ../include/tsh_shell.h $(LIB)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ launch.c -L../obj -lsng

demo_worker: demo_worker.c
	$(CC) $(CFLAGS) -o $@ demo_worker.c

.PHONY: all

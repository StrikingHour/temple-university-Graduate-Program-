CC = gcc
CPPFLAGS = -I../include
CFLAGS = -O3 -Wall -Wextra -std=c11
LDLIBS = ../obj/libsng.a -lm
all: client worker
client: client.c common.h ../include/tuplelib.h ../obj/libsng.a
	$(CC) $(CPPFLAGS) $(CFLAGS) client.c $(LDLIBS) -o $@
worker: worker.c common.h ../include/tuplelib.h ../obj/libsng.a
	$(CC) $(CPPFLAGS) $(CFLAGS) worker.c $(LDLIBS) -o $@
.PHONY: all

all: seq_kij seq_ikj
seq_kij: seq_kij.c
	$(CC) $(CFLAGS) seq_kij.c -lm -o $@
seq_ikj: seq_kij.c
	$(CC) $(CFLAGS) -DBASELINE_IKJ seq_kij.c -lm -o $@

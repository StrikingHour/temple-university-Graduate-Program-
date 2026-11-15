CC = gcc
AR = ar
CPPFLAGS = -I../include
CFLAGS = -O3 -Wall -Wextra
LIB = ../obj/libsng.a
OBJECTS = netlib.o wall_clock.o tshlib.o tuplelib.o

all: $(LIB)

$(LIB): $(OBJECTS)
	mkdir -p ../obj
	$(AR) rcs $@ $(OBJECTS)

netlib.o: netlib.c ../include/synergy.h ../include/portab.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

wall_clock.o: wall_clock.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

tshlib.o: tshlib.c ../include/tshlib.h ../include/tsh_shell.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -std=c11 -c $< -o $@

.PHONY: all

tuplelib.o: tuplelib.c ../include/tuplelib.h ../include/synergy.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

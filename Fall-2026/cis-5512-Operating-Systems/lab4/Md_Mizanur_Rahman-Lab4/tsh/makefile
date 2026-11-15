CC = gcc
CFLAGS = -g -Wall -Wextra
CPPFLAGS = -I../include

all: tsh tshtest myshell

tsh: tsh.c tsh.h shell_service.c ../include/tsh_shell.h ../include/synergy.h ../obj/libsng.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tsh.c shell_service.c -L../obj -lsng

tshtest: tshtest.c tshtest.h ../include/tsh_shell.h ../include/synergy.h ../obj/libsng.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tshtest.c -L../obj -lsng

myshell: myshell.c
	$(CC) -std=c11 $(CFLAGS) -o $@ myshell.c

.PHONY: all

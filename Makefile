CC=gcc
CFLAGS=-Wall
DEBUG=-ggdb
APPNAME=bftp
FILES=srvr.c clnt.c main.c utils.c

release:
	$(CC) $(CFLAGS) -o $(APPNAME) $(FILES)

debug:
	$(CC) $(CFLAGS) $(DEBUG) -o $(APPNAME) $(FILES)
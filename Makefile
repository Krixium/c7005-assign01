CC=gcc
CFLAGS=-Wall
DEBUG=-ggdb
APPNAME=bftp
FILES=srvr.c clnt.c main.c utils.c

release:
	$(CC) $(CFLAGS) -o $(APPNAME) $(FILES)

debug:
	$(CC) $(CFLAGS) $(DEBUG) -o $(APPNAME) $(FILES)

test:
	make debug
	mkdir test
	mkdir test/clnt
	mkdir test/srvr
	cp $(APPNAME) test/clnt/$(APPNAME)
	cp $(APPNAME) test/srvr/$(APPNAME)

clean:
	rm -rf $(APPNAME)
	rm -rf test
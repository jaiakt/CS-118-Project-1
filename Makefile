CC=gcc
CPPFLAGS=-g -Wall
USERID=604605757

all: server

server: server.c
	$(CC) -o server $(CPPFLAGS) server.c

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM server *.tar.gz

dist: tarball

tarball: clean
	tar -cvzf /tmp/$(USERID).tar.gz --exclude=./.vagrant . && mv /tmp/$(USERID).tar.gz .

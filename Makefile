CC=gcc
CPPFLAGS=-g -Wall
USERID=123456789

all: server client

server: server.c
	$(CC) -o server $(CPPFLAGS) server.c

client: client.c
	$(CC) -o client $(CPPFLAGS) client.c

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM server client *.tar.gz

dist: tarball

tarball: clean
	tar -cvzf /tmp/$(USERID).tar.gz --exclude=./.vagrant . && mv /tmp/$(USERID).tar.gz .

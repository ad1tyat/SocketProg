CC=gcc
CFLAGS=-I.
DEPS = message.h

all: server client

server:
	$(CC) server.c -o server

client:
	$(CC) client.c -o client

clean:
	rm -f server client
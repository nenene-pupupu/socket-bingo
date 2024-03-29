CC = gcc
CFLAGS = -Wall -pthread

all: server client

server: server.o
	$(CC) $(CFLAGS) -o server server.o

client: client.o board.o
	$(CC) $(CFLAGS) -o client client.o board.o

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

client.o: client.c board.h
	$(CC) $(CFLAGS) -c client.c

board.o: board.c board.h
	$(CC) $(CFLAGS) -c board.c

clean:
	rm -f server client *.o

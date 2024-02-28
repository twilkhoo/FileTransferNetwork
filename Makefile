CC = g++
CFLAGS = -Wall -std=c++17

all: server client

server: server.o util.o
	$(CC) $(CFLAGS) -o server server.o util.o

server.o: server.cpp util.h
	$(CC) $(CFLAGS) -c server.cpp

client: client.o util.o
	$(CC) $(CFLAGS) -o client client.o util.o

client.o: client.cpp util.h
	$(CC) $(CFLAGS) -c client.cpp

util.o: util.cpp util.h
	$(CC) $(CFLAGS) -c util.cpp

clean:
	rm -f server server.o client client.o util.o

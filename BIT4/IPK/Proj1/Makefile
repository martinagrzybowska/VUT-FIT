CC = g++
CFLAGS = -Wall -Wextra -pedantic -std=c++11

all: ftrestd ftrest

ftrestd: server.o server2.o
	$(CC) $(CFLAGS) server.o server2.o -o $@

ftrest: client.o client2.o 
	$(CC) $(CFLAGS) client.o client2.o -o $@

client.o: shared.h ftrest.h ftrest.cpp
	$(CC) $(CFLAGS) -c ftrest.cpp -o $@

server.o: shared.h ftrestd.h ftrestd.cpp
	$(CC) $(CFLAGS) -c ftrestd.cpp -o $@

client2.o: shared.h ftrest.h client.cpp
	$(CC) $(CFLAGS) -c client.cpp -o $@

server2.o: shared.h ftrestd.h server.cpp
	$(CC) $(CFLAGS) -c server.cpp -o $@
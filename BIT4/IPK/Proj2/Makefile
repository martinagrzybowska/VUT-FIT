CC = g++
CFLAGS = -Wall -Wextra -pedantic -std=c++11 -pthread

all: chat_client

chat_client: client.o client2.o 
	$(CC) $(CFLAGS) client.o client2.o -o $@

client.o: shared.h chat_client.h chat_client.cpp
	$(CC) $(CFLAGS) -c chat_client.cpp -o $@

client2.o: shared.h chat_client.h client.cpp
	$(CC) $(CFLAGS) -c client.cpp -o $@
	
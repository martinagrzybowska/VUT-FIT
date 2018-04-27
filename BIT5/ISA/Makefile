CC = g++
CFLAGS = -Wall -Wextra -pedantic -std=c++11

all: isabot

isabot: bot.o bot2.o bot3.o bot4.o
	$(CC) $(CFLAGS) bot.o bot2.o bot3.o bot4.o -o $@

bot.o: isabot.h isabot.cpp
	$(CC) $(CFLAGS) -c isabot.cpp -o $@

bot2.o: isabot.h arguments.cpp
	$(CC) $(CFLAGS) -c arguments.cpp -o $@

bot3.o: isabot.h commands.cpp
	$(CC) $(CFLAGS) -c commands.cpp -o $@

bot4.o: isabot.h networking.cpp
	$(CC) $(CFLAGS) -c networking.cpp -o $@
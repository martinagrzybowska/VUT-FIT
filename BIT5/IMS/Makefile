CLAGS=-g -O2 
PFLAGS=-lsimlib -lm
 
CC = g++
FILES = liahen.cpp

all: liahen

liahen: liahen.cpp
	$(CC) $(CFLAGS) -o $@ liahen.cpp $(PFLAGS)

run:
	./liahen

test:
	./liahen
	cat stats.out

rebuild: clean all
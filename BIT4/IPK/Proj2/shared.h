#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <thread>
#include <csignal>
#include <fcntl.h>


using namespace std;
#define BUFFSIZE 1024

extern int my_socket;
extern string user_name;

/**
 * enum of error codes
 */
enum errorCodes {
	ALL_OK = 0,
	CONNECTION_ERR, //2
	OPTS_ERR, //3
	ULTIMATE_ANSWER = 42
};

#endif
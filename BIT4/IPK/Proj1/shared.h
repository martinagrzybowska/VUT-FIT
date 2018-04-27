#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
//#include <arpa/inet.h>
#include <unistd.h>
#include <syslog.h>
#include <netdb.h>
#include <algorithm>
#include <vector>
#include <fstream>
#include <dirent.h>
#include <errno.h>

using namespace std;
#define BUFFSIZE 1024
/**
 * enum of error codes
 */
enum errorCodes {
	ALL_OK = 0,
	CONNECTION_ERR, //2
	OPTS_ERR, //3
	HTTP_ERR, //4, HTTP 406
	COMM_ERR, //5, HTTP 500
	N_EXIST_USR, //6, HTTP 404
	N_EXIST_DIR, //7, HTTP 404
	N_EXIST_FILE, //8, HTTP 404
	FORBIDDEN, //9, HTTP 403
	WRONG_TYPE_DIR, //10, HTTP 400
	WRONG_TYPE_FILE, //11, HTTP 400 
	EXIST, //12, HTTP 418
	SOMETHING_WRONG, //13
	ULTIMATE_ANSWER = 42
};
/**
 * enum of command types
 */
enum command_vars {
	DEL = 0, // delete file by remote path
	GET, // copy file from remote path to cwd or local path
	PUT, // copy file from local path to remote path
	LST, // print contents of a dir
	MKD, // create dir
	RMD // delete dir
};
//- - - - - - - - - - server struct
/**
 * received_info is a structure used for storing information drew from
 * the received header string
 */
typedef struct received_info {
	std::string path, type, data;
	command_vars command;
	int error, content_length;
} TS_args;
/**
 * post_info is a structure used for storing information to be used
 * for creating a response header to the client
 */
typedef struct post_info {
	string code_phrase, text_resp;
	command_vars which_comm;
	bool was_error;
} T_post;

//- - - - - - - - - - client struct
typedef struct input_arguments {
	std::string host_name, remote_path, local_path;
	command_vars command;
	int port_num, error, content_length;
} TC_args;

typedef struct received_answer {
	string data;
	int content_length;
} T_answer;

#endif
#ifndef FREST_H
#define FREST_H

#include "shared.h"

int parse_args(int argc, char const *argv[], string *s_address);
int set_connection(string host_name);
void receive_from_server();
void send_to_server();
int send_minion(string message, bool flag);
void sigint_handler(int signal);

#endif

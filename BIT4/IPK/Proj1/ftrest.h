#ifndef FREST_H
#define FREST_H

#include "shared.h"

/**
 * P_args gets and parses input arguments
 */
int P_args(int argc, char const *argv[], TC_args *args);
/**
 * set_connection creates a socket and sets up a connection with the server
 */
int set_connection(int *sock, TC_args arguments);
/**
 * create_mesasge creates a http header which is to be sent to the server
 */
int create_message(TC_args *var_struct, string *rollingDung);
/**
 * parse_response parses string received from the server
 */
int parse_response(string received, TC_args orig_struct);

#endif

#ifndef FRESTD_H
#define FRESTD_H
#include "shared.h"

/**
 * P_args parses command line arguments
 */
int P_args(int argc, char * argv[], int *port, string *directory);
/**
 * Ch_remote_path performs a check whether the remote path is correct
 * in combination with the given command
 */
int Ch_remote_path(string check_path, string root_dir, TS_args *Uni_struct);
/**
 * P_header parses incoming header string and the contents
 */
int P_header(string parse_header, string root_dir, TS_args *header_struct);
/**
 * command_DEL represents DEL
 */
int command_DEL(string path_to_dir);
/**
 * command_MKD represents MKD
 */
int command_MKD(string path_to_dir);
/**
 * command_RMD represents RMD
 */
int command_RMD(string path_to_dir);
/**
 * command_LST represents LST
 */
string command_LST(string path_to_dir);
/**
 * command_PUT represents PUT
 */
int command_PUT(TS_args *put_here);
/**
 * command_GET represents GET
 */
TS_args command_GET(TS_args *get_this);
/**
 * create_response combines gathered information from both structures and
 * creates a response string which is to be sent to the client
 */
string create_response(T_post build_struct, TS_args add_struct);
/**
 * evaulate_codes evaulates either number of the error code a which command was used,
 * it also calls command functions if everything was correct
 */
void evaulate_codes(int controller, T_post *fill_struct, TS_args *check_struct);

#endif
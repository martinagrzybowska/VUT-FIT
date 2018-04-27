#include "shared.h"
#include "ftrest.h"

/**
 * main
 */
int main(int argc, char const *argv[])
{
	int main_socket, bytestx;
	string send_header;
	TC_args create_header; 
	
	//get arguments
	if (P_args(argc, argv, &create_header) != ALL_OK) {
		fprintf(stderr, "OPT-ERR - Incorrect arguments\n");
		return OPTS_ERR;
	}
	// set up connection
	if (set_connection(&main_socket, create_header) != ALL_OK) {
		fprintf(stderr, "CONNECT-ERR - Unable to set up connection\n");
		return CONNECTION_ERR;
	}
	//generate header which is to be sent
	if (create_message(&create_header, &send_header) != ALL_OK) {
		fprintf(stderr, "CONNECT-ERR - Incorrect message form\n");
		return CONNECTION_ERR;
	}
	
	//sent header with data
	bytestx = send(main_socket, send_header.c_str(), send_header.length(), 0);
	if (bytestx < 0) {
		fprintf(stderr, "CONNECT-ERR - Unable to send the header\n");
		return CONNECTION_ERR;
	}
	
	shutdown(main_socket, SHUT_WR);
	
	char buff[BUFFSIZE + 1];
	int res = 0;
	string receive_string = "";

	while (ULTIMATE_ANSWER) {	
		bzero(buff, BUFFSIZE + 1);
		res = recv(main_socket, buff, BUFFSIZE, 0);
		if (res <= 0)                
			break;

		receive_string += string(buff, res); 
	}
	
	close(main_socket);
	//parse received response
	if (parse_response(receive_string, create_header) != ALL_OK) {
		return SOMETHING_WRONG;
	}
	else {
		return ALL_OK;
	}
}
#include "shared.h"
#include "chat_client.h"

int my_socket;
string user_name;

/**
 * main
 */
int main(int argc, char const *argv[])
{
	string server_address;
	signal(SIGINT, sigint_handler);
	signal(SIGPIPE, sigint_handler);
	
	//get arguments
	if (parse_args(argc, argv, &server_address) != ALL_OK) {
		fprintf(stderr, "OPT-ERR - Incorrect arguments\n");
		return OPTS_ERR;
	}
	//set up connection
	if (set_connection(server_address) != ALL_OK) {
		fprintf(stderr, "CONNECT-ERR - Unable to set up connection\n");
		return CONNECTION_ERR;
	}
	//send logged in message
	send_minion(user_name + " logged in\r\n", false);
	
	//create a child thread that will send messages
	thread t1(send_to_server);
	//the parent thread receives messages
	receive_from_server();
	
	t1.detach();
	
	fprintf(stderr, "CONNECT-ERR - Connection to server lost\n");
	return CONNECTION_ERR;
}
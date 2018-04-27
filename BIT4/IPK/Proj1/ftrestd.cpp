#include "shared.h"
#include "ftrestd.h"
/**
 * main
 */
int main (int argc, char * argv[]) {
	
	int main_socket;
	struct sockaddr_in6 sserver_address;
	struct sockaddr_in6 cserver_address;
	TS_args parsed_header;
	T_post response_header;
   
	int main_port = 0; //parameter
	string root_dir = ""; //parameter

	if (P_args(argc, argv, &main_port, &root_dir) != ALL_OK) {
		fprintf(stderr, "OPTS-ERR - Bad arguments\n");
		return OPTS_ERR;
	}
	//if port number was not set, set it do default
	if (main_port == 0) {
		main_port = 6677;
	}
	//if root dir was not received, set it to cwd
	if (root_dir == "") {
		char buffer[128];
		getcwd(buffer, 128);
		root_dir = string(buffer);
	}
	
	socklen_t cserver_len = sizeof(cserver_address);
	
	if ((main_socket = socket(PF_INET6, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "SOC-ERR - Something wrong with sockets\n");
		return CONNECTION_ERR;
	}
	
	memset(&sserver_address,0,sizeof(sserver_address));
	sserver_address.sin6_family = AF_INET6;
	sserver_address.sin6_addr = in6addr_any;
	sserver_address.sin6_port = htons(main_port);   
		
	if ((bind(main_socket, (struct sockaddr*)&sserver_address, sizeof(sserver_address))) < 0) {
		fprintf(stderr, "BIND-ERR - Something wrong with bondage");
		return CONNECTION_ERR;
	}
	
	if ((listen(main_socket, 1)) < 0) { 
		fprintf(stderr, "LISTEN-ERR - Something wrong with listening");
		return CONNECTION_ERR;
	}
	
	while (ULTIMATE_ANSWER) {
		
		int c_socket = accept(main_socket, (struct sockaddr*)&cserver_address, &cserver_len);        
		
		if (c_socket > 0) {
		
			char buff[BUFFSIZE + 1];
			int res = 0;
			string message_string = "";
			string http_header = "";
			
			while (ULTIMATE_ANSWER) {  
				
				bzero(buff, BUFFSIZE + 1);
				res = recv(c_socket, buff, BUFFSIZE, 0);
				
				if (res <= 0) {               
					break;
				}

				http_header += string(buff, res); 
			}
			//if some message from the client was received
			if (http_header != "") {    
				//parse the header and fill its structure
				int error_code = P_header(http_header, root_dir, &parsed_header);
				//evaulate paths and commands
				evaulate_codes(error_code, &response_header, &parsed_header);
				//create a response and send it
				message_string = create_response(response_header, parsed_header);
				send(c_socket, message_string.c_str(), message_string.length(), 0);
			}
		}
		close(c_socket);
	}
}

#include "shared.h"
#include "chat_client.h"

/**
 * parse_args gets and parses input arguments
 */
int parse_args(int argc, char const *argv[], string *s_address) {

    if (argc == 5) {
        if (!strcmp(argv[1], "-i")) {
            if (!strcmp(argv[3], "-u")) {
                user_name = string(argv[4]);
                *s_address = string(argv[2]);
                return ALL_OK;
            }
        }
        else if (!strcmp(argv[1], "-u")) {
            if (!strcmp(argv[3], "-i")) {
                user_name = string(argv[2]);
                *s_address = string(argv[4]);
                return ALL_OK;
            }
        }
    }
    return OPTS_ERR;
}
/**
 * set_connection creates a socket and sets up a connection with the server
 */
int set_connection(string host_name) {
    
    struct sockaddr_in tmp_server;
    struct hostent *tmp_host;
    int port_num = 21011;
    
    // get host name
    if ((tmp_host = gethostbyname(host_name.c_str())) == NULL) {
        return CONNECTION_ERR;
    }
   
    bzero((char *) &tmp_server, sizeof(tmp_server));
    tmp_server.sin_family = AF_INET;
    bcopy((char *)tmp_host->h_addr, (char *)&tmp_server.sin_addr.s_addr, tmp_host->h_length);
    tmp_server.sin_port = htons(port_num);

    //create socket
    if ((my_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
        return CONNECTION_ERR;
    }
    // connect
    if (connect(my_socket, (const struct sockaddr *)&tmp_server, sizeof(tmp_server)) != 0) {
        return CONNECTION_ERR;       
    }
    
    return ALL_OK;
}
/**
 * receive_from_server is used for obtaining messages from other clients on the server
 */
void receive_from_server() {
    char buff[BUFFSIZE + 1];
    int res;
    string receive_string;

    while (ULTIMATE_ANSWER) {   
        
        // clean it before using
        res = 0;
        bzero(buff, BUFFSIZE + 1);
        
        if (my_socket != 0) {
            res = recv(my_socket, buff, BUFFSIZE, 0);
            
            if (res <= 0) {
                break;
            }             
            //print message to stdout
            printf("%s", buff); 
        }
    }
    return;
}
/**
 * send_to_server is used for reading stdin input
 */
void send_to_server(){
    
    string message;
    //read from stdin
    while (getline(cin, message)) {
        if (!strcmp(message.c_str(), "")) {
            continue;
        }
        send_minion(message, true);
    }
    return;
}
/**
 * send_minion sends the message to the server
 */
int send_minion(string message, bool flag) {

    // in case of user-defined message flag was set to 1
    if (flag == 1) {
        message = user_name + ": " + message + "\r\n";
    }
    
    int bytestx = send(my_socket, message.c_str(), message.length(), MSG_NOSIGNAL);
    if (bytestx < 0) {
        fprintf(stderr, "CONNECT-ERR - Unable to send the message, PIPE\n");
        return CONNECTION_ERR;
    }
    
    return ALL_OK;
}
/**
 * sigint_handler is used for catching singint signal and sending log out message
 */
void sigint_handler(int signal) {
   
    // send user logged out message
    send_minion(user_name + " logged out\r\n", false);
    //close the connection
    close(my_socket);
    
    (void) signal; // ¯\_(ツ)_/¯
    
    exit(ALL_OK); 
}
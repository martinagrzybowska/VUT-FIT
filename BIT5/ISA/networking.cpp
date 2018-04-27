/*
 * \file networking.cpp
 * \brief File containing functions managing connections to the servers 
 * \author: Martina Grzybowska, xgrzyb00
 * 
 * Project for ISA 2017/2018
 * 
 */

#include "isabot.h"

/**
 * @brief      Receives messages from tcp irc server
 *
 * @param[in]  irc     The irc structure
 * @param      syslog  Pointer to the syslog structure
 *
 * @return     Various error codes or ALL_OK (0).
 */
int receive_from_tcp_server(irc_struct irc, syslog_struct *syslog) {
	
	char buff[BUFFSIZE + 1];
	std::string receive_string = "";
	int result = 0, received = 0;
	
	while(INFINITE_LOOP) {
		
		// clean our message buffer
		bzero(buff, BUFFSIZE + 1);
		// receive from server
		received = recv(tcp_socket, buff, 1, 0);

		if (received <= 0) {
			return RECEIVE_ERR;
		}
		else {
			// roll the received string until we get end of line
			receive_string += buff;
			if (receive_string.find("\n") != std::string::npos) {

				if ((result = determine_command(receive_string, syslog, &irc)) != ALL_OK) {
					return result;
				} 

				//clean the received string
				receive_string = "";
			}	
		}
	}
	return ALL_OK;
}

/**
 * @brief      Gets the current ip for the syslog message.
 * 			   Credits to Jinghao Shi (http://jhshi.me/home/) and his article on "How to Get Local Host's Real IP Address", 
 * 			   which has provided basis for a part of this function's implementation.
 *
 * @param[in]  irc     The irc structure
 * @param      syslog  Pointer to the syslog structure
 *
 * @return     Various error codes or ALL_OK (0).
 */
int get_current_ip(irc_struct irc, syslog_struct *syslog) {

	socklen_t addr_len = sizeof(irc.soc_addr_tcp);
	
	if (getsockname(tcp_socket, (struct sockaddr*)&irc.soc_addr_tcp, &addr_len) < 0) {
		return CURRENT_IP_ERR;
	}

	char tmp_ip[INET_ADDRSTRLEN];
	if (inet_ntop(irc.soc_addr_tcp.sin_family, &(irc.soc_addr_tcp.sin_addr), tmp_ip, sizeof(tmp_ip)) == NULL) {
		return CURRENT_IP_ERR;
	}

	syslog->current_ip = std::string(tmp_ip);

	return ALL_OK;
}

/**
 * send_syslog_message sends a string message to UDP SYSLOG server
 */

/**
 * @brief      Sends a syslog message.
 *
 * @param[in]  message  The message to be sent
 * @param      syslog   Pointer to the syslog structure
 *
 * @return     Various error codes or ALL_OK (0).
 */
int send_syslog_message(std::string message, syslog_struct *syslog) {

	int result = sendto(udp_socket, message.c_str(), message.length(), 0, (struct sockaddr*)&(syslog->soc_addr_udp), sizeof(syslog->soc_addr_udp));
		
	if (result < 0) {
		return UDP_MESSAGE_SEND_ERR;
	}

	return ALL_OK;
}

/**
 * @brief      Creates a syslog message.
 *
 * @param[in]  message  The text of the original message
 * @param      syslog   Pointer to the syslog structure
 *
 * @return     A message in syslog format.
 */
std::string create_syslog_message(std::string username, std::string message_content, syslog_struct *syslog) {

	std::string rolling_dung;

	// create TIMESTAMP in format Mmm dd hh:mm:ss
	char tmp_time[1024];
	time_t timestamp = time(0);
	struct tm tm_struct = *localtime(&timestamp);
	strftime(tmp_time, sizeof(tmp_time), "%b %e %H:%M:%S", &tm_struct);

	// <PRI>TIMESTAMP IP_ADDRESS TAG
	rolling_dung += "<" + syslog->priority + ">" + std::string(tmp_time) + " " + syslog->current_ip + " " + syslog->tag + " ";

	// adding CONTENT (<nickname> : <message>)
	rolling_dung += username + ":" + message_content;

	return rolling_dung;
}

/**
 * @brief      Connects to tcp irc server.
 *
 * @param      irc   Pointer to the irc structure
 *
 * @return     Various error codes or ALL_OK (0).
 */
int connect_to_tcp(irc_struct *irc) {
	
	//struct sockaddr_in socket_addr;
	struct hostent *tmp_host;

	// get host's IP address from name
	if ((tmp_host = gethostbyname(irc->host_name.c_str())) == NULL) {
		return TCP_CONNECTION_ERR;
	}

	bzero((char *) &(irc->soc_addr_tcp), sizeof(irc->soc_addr_tcp));
	(irc->soc_addr_tcp).sin_family = AF_INET;
	bcopy((char *)tmp_host->h_addr, (char *) &(irc->soc_addr_tcp).sin_addr.s_addr, tmp_host->h_length);
	(irc->soc_addr_tcp).sin_port = htons(irc->port_num);

	if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
		return TCP_CONNECTION_ERR;
	}

	if (connect((tcp_socket), (const struct sockaddr *) &(irc->soc_addr_tcp), sizeof(irc->soc_addr_tcp)) != 0) {
		return TCP_CONNECTION_ERR;       
	}

	return ALL_OK;
}

/**
 * @brief      Connects to udp syslog server.
 *
 * @param      syslog  Pointer to the syslog structure
 *
 * @return     Various error codes or ALL_OK (0).
 */
int connect_to_udp(syslog_struct *syslog) {

	bzero((char *) &syslog->soc_addr_udp, sizeof(syslog->soc_addr_udp));
	syslog->soc_addr_udp.sin_family = AF_INET;
	syslog->soc_addr_udp.sin_addr.s_addr = inet_addr(syslog->syslog_ip.c_str());
	syslog->soc_addr_udp.sin_port = htons(514);

	if ((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) <= 0) {
		return UDP_CONNECTION_ERR;
	}
	
	return ALL_OK;
}

/**
 * @brief      Sends an irc message.
 *
 * @param[in]  message  The message to be sent
 *
 * @return     Error code of message failure or ALL_OK (0).
 */
int send_irc_message(std::string message) {

	int bytestx = send(tcp_socket, message.c_str(), message.length(), 0);
	
	if (bytestx < 0) {
		return TCP_MESSAGE_SEND_ERR;
	}
	
	return ALL_OK;
}

/**
 * @brief      Closes the sockets.
 */
void kill_it_with_fire() {
	close(tcp_socket);
	close(udp_socket);
}

/**
 * @brief      Handles SIGINT.
 *
 * @param[in]  signal  The signal
 */
void sigint_handler(int signal) {
   
   	// close the sockets and send quit message
	send_irc_message("QUIT\r\n");
	kill_it_with_fire();

	(void) signal; // ¯\_(ツ)_/¯
	
	fprintf(stderr, "SIGINT - Program terminated by user (Ctrl+C)\n");
	exit(ALL_OK);

}
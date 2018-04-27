/*
 * \file isabot.cpp
 * \brief File containing main function
 * \author: Martina Grzybowska, xgrzyb00
 * 
 * Project for ISA 2017/2018
 * 
 */

#include "isabot.h"

int tcp_socket, udp_socket;
std::string login = "xgrzyb00";

int main(int argc, char const *argv[]) {
	
	signal(SIGINT, sigint_handler);
	
	irc_struct irc;
	syslog_struct syslog;

	int result;

	// parse arguments and fill all neccessary structures
	if ((result = parse_args(argc, argv, &irc, &syslog)) != ALL_OK) {
		if (result == HELP) {
			return ALL_OK;
		}
		else {
			fprintf(stderr, "ARGUMENTS ERR - something went wrong with arguments\n");
			return ARGUMENTS_ERR;
		}
	}
	// connect to TCP IRC server
	if (connect_to_tcp(&irc) != ALL_OK) {
		fprintf(stderr, "CONNECTION ERR - Unable to connect to TCP IRC server\n");
		return TCP_CONNECTION_ERR;
	}
	// get current IP if the key word list was specified
	if (irc.key_word_list.size() > 0) {
		if (get_current_ip(irc, &syslog) != ALL_OK) {
			kill_it_with_fire();
			fprintf(stderr, "IP ERR - Unable to retrieve current IP address\n");
			return CURRENT_IP_ERR;
		}
	}

	//create an UDP socket for Syslog
	if (connect_to_udp(&syslog) != ALL_OK) {
		fprintf(stderr, "UDP_SOCKET_ERR - Unable to create UDP socket\n");
		return UDP_CONNECTION_ERR;
	}

	// send NICK, USER and JOIN messages
	if (send_irc_message("NICK " + login + "\r\n") != ALL_OK) {
		kill_it_with_fire();
		fprintf(stderr, "SEND ERR - Unable to send TCP message to IRC server\n");
		return TCP_MESSAGE_SEND_ERR;
	}
	if (send_irc_message("USER " + login + " " + login + " " + login + " " + ":" + login + "\r\n") != ALL_OK) {
		kill_it_with_fire();
		fprintf(stderr, "SEND ERR - Unable to send TCP message to IRC server\n");
		return TCP_MESSAGE_SEND_ERR;
	}
	
	if (send_irc_message("JOIN " + irc.channel_print + "\r\n") != ALL_OK) {
		kill_it_with_fire();
		fprintf(stderr, "SEND ERR - Unable to send TCP message to IRC server\n");
		return TCP_MESSAGE_SEND_ERR;
	}

	try {
		// listen to what the IRC server has to say
		if ((result = receive_from_tcp_server(irc, &syslog)) != ALL_OK) {
			
			if (result == TCP_MESSAGE_SEND_ERR) {
				kill_it_with_fire();
				fprintf(stderr, "SEND ERR - Unable to send TCP message to IRC server\n");
				return TCP_MESSAGE_SEND_ERR;
			}
			else if (result == UDP_MESSAGE_SEND_ERR) {
				send_irc_message("QUIT\r\n");
				kill_it_with_fire();
				fprintf(stderr, "SEND ERR - Unable to send UDP message to SYSLOG server\n");
				return UDP_MESSAGE_SEND_ERR;
			}
			else if (result == BOT_KICK) {
				send_irc_message("QUIT\r\n");
				kill_it_with_fire();
				fprintf(stderr, "KICK - Kicked from a channel, unable to function fully\n");
				return BOT_KICK;
			}
			else if (result == SERVER_ERR) {
				send_irc_message("QUIT\r\n");
				kill_it_with_fire();
				fprintf(stderr, "ERROR in range 400-500 - something went wrong\n");
				return SERVER_ERR;
			}
			else if (result == RECEIVE_ERR) {
				send_irc_message("QUIT\r\n");
				kill_it_with_fire();
				fprintf(stderr, "RECEIVE ERR - Unable to receive messages from TCP IRC server\n");
				return RECEIVE_ERR;
			}
		}
	} catch (const std::exception &exc) {
		send_irc_message("QUIT\r\n");
		kill_it_with_fire();
		fprintf(stderr, "EXCEPTION - Something went wrong\n");
	}
	
	return ALL_OK;
	
}
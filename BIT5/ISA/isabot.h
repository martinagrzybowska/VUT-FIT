/*
 * \file isabot.h
 * \brief Header file
 * \author: Martina Grzybowska, xgrzyb00
 * 
 * Project for ISA 2017/2018
 * 
 */

#ifndef ISABOT_H
#define ISABOT_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <string>
#include <regex>

#include <sstream> 
#include <iterator> 
#include <vector>   
#include <list>
#include <numeric>
#include <signal.h>
#include <map>

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <ctype.h>

/**
 * @brief     Error codes.
 */
enum ErrorCodes {
	ALL_OK = 0,
	ARGUMENTS_ERR,
	UDP_CONNECTION_ERR,
	TCP_CONNECTION_ERR,
	CURRENT_IP_ERR,
	UDP_MESSAGE_SEND_ERR,
	TCP_MESSAGE_SEND_ERR,
	RECEIVE_ERR,
	SIGNAL_ERR,
	LOGIN_ERR,
	SERVER_ERR,
	INFINITE_LOOP
};
/**
 * @brief     Commands.
 */
enum Commands {
	PING,
	PRIVMSG,
	NOTICE,
	NAMES,
	JOIN,
	QUIT,
	PART,
	KICK,
	BOT_KICK,
	NICK,
	HELP
};
/**
 * @brief      Structure for storing important irc data.
 */
struct irc_struct {
	std::string host_name, channel_print;
	std::vector<std::string> key_word_list, channels_list;
	std::map<std::string, std::vector<std::string>> online_users;
	std::map<std::string, std::map<std::string, std::vector<std::string>>> offline_users;
	struct sockaddr_in soc_addr_tcp;
	int port_num;
};
/**
 * @brief      Structure for storing importat syslog data
 */
struct syslog_struct {
	std::string tag, syslog_ip, current_ip, priority = "134";
	struct sockaddr_in soc_addr_udp;
};

extern int tcp_socket, udp_socket;
extern std::string login;

#define BUFFSIZE 1
#define DEFAULT_PORT 6667

// Commands
void get_command_data(int command, std::string message, std::string *username, std::string *channel, size_t position);
int determine_command(std::string message, syslog_struct *syslog, irc_struct *irc);
int command_names(std::string prefix, std::string message_content, irc_struct *irc);
int command_nick(std::string username, std::string newname, irc_struct *irc);
void command_part(std::string username, std::string channel, irc_struct *irc);
int command_join(std::string username, std::string channel, irc_struct *irc);
void command_quit(std::string username, irc_struct *irc);
int command_kick(std::string username, std::string channel, irc_struct *irc);
bool find_argument_keywords(std::string message_content, irc_struct *irc);
int find_special_keywords(std::string channel, std::string message_content, irc_struct *irc);
int find_pending_messages(std::string username, std::string channel, irc_struct *irc);
void save_pending_message(std::string username, std::string channel, std::string message_content, irc_struct *irc);
bool check_if_online(std::string username, std::string channel, irc_struct *irc);
std::string get_message_content(std::string message);
std::string get_username(std::string message);
std::string skip_spaces(std::string message, size_t *position);
std::string to_uppercase(std::string word);

// Networking
int receive_from_tcp_server(irc_struct irc, syslog_struct *syslog);
int get_current_ip(irc_struct irc, syslog_struct *syslog);
int send_irc_message(std::string message);
int send_syslog_message(std::string message, syslog_struct *syslog);
std::string create_syslog_message(std::string username, std::string message_content, syslog_struct *syslog);
int connect_to_udp(syslog_struct *syslog);
int connect_to_tcp(irc_struct *irc);
void sigint_handler(int signal);
void kill_it_with_fire();

// Arguments
int parse_args(int argc, char const *argv[], irc_struct *irc, syslog_struct *syslog);
int split_by_delimiter(char const *argv, std::vector<std::string> *tmp_vector, bool toggle);
void print_help();

#endif
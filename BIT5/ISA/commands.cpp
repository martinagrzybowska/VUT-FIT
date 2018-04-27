/*
 * \file commands.cpp
 * \brief File contaning functions implementing the main logic
 * \author: Martina Grzybowska, xgrzyb00
 * 
 * Project for ISA 2017/2018
 * 
 */

#include "isabot.h"

/**
 * @brief      Skip spaces in a string.
 *
 * @param[in]  message   The message
 * @param[in]  position  The position
 *
 * @return     The string after spaces.
 */
std::string skip_spaces(std::string message, size_t *position) {
	std::string rolling_dung = "";
	// skip spaces
	while (isspace(message[*position])) {
			(*position)++;
	}
	// roll till there is a space
	while (!isspace(message[*position])) {
			rolling_dung += message[*position];
			(*position)++;
	}

	return rolling_dung;
}

/**
 * @brief      Gets the message content.
 *
 * @param[in]  message  The message
 *
 * @return     The message content.
 */
std::string get_message_content(std::string message) {
	// look for the first : after the one in the beginning
	message = message.substr(message.find(":", 1) + 1);
	
	return message;
}

/**
 * @brief      Gets the username.
 *
 * @param[in]  message  The message to cut from
 *
 * @return     The username.
 */
std::string get_username(std::string message) {
	message = message.substr(1, message.find("!") - 1);
	// cut the @ and + which may sometimes be at the beginning of the real username
	if (!strcmp(message.substr(0,1).c_str(), "@") || !strcmp(message.substr(0,1).c_str(), "+")) {
		message = message.substr(1);
	}

	return message;
}

/**
 * @brief      Converts string to uppercase.
 *
 * @param[in]  word  The word
 *
 * @return     The uppercase string.
 */
std::string to_uppercase(std::string word) {
	std::string tmp_string;
	std::transform(word.begin(), word.end(), std::back_inserter(tmp_string), ::toupper);
	
	return tmp_string;
}

/**
 * @brief      Checks whether the user is online.
 *
 * @param[in]  username  The username of the user
 * @param[in]  channel   The channel the message came to
 * @param      irc       Pointer to the irc structure
 *
 * @return     True if is online, false otherwise.
 */
bool check_if_online(std::string username, std::string channel, irc_struct *irc) {
	
	std::map<std::string,std::vector<std::string>>::iterator iter;
	// if user is in our list
	if ((iter = irc->online_users.find(username)) != irc->online_users.end()) {
		// if they are online on the channel the message was send to
		if (std::find(iter->second.begin(), iter->second.end(), channel) != iter->second.end()) {
			return true;
		}
	}
	return false;
}

/**
 * @brief      Stores messages of users which are not yet online on the given channel.
 *
 * @param[in]  username         The username of the user
 * @param[in]  channel          The channel the message came to
 * @param[in]  message_content  The content part of the message received
 * @param      irc              Pointer to the irc structure
 */
void save_pending_message(std::string username, std::string channel, std::string message_content, irc_struct *irc) {

	std::map<std::string, std::map<std::string, std::vector<std::string>>>::iterator iter_user;
	std::map<std::string, std::vector<std::string>>::iterator iter_channel;
	std::map<std::string, std::vector<std::string>> tmp_map;
	std::vector<std::string> tmp_vector;
	
	// check whether this user already has some pending messages
	if ((iter_user = irc->offline_users.find(username)) != irc->offline_users.end()) {
		// if he does, check whether they come from an already existing channel
		if ((iter_channel = iter_user->second.find(channel)) != iter_user->second.end()) {
			iter_channel->second.push_back(message_content);
		}
		// if they do not, create a new map for channel and message vector
		else {
			tmp_vector.push_back(message_content);
			iter_user->second.insert(std::pair<std::string, std::vector<std::string>>(channel, tmp_vector));
		}
	}
	// if such a user does not yet exist
	else {
		tmp_vector.push_back(message_content);
		tmp_map.insert(std::pair<std::string, std::vector<std::string>>(channel, tmp_vector));
		// create a map of channel and message vector
		irc->offline_users.insert(std::pair<std::string, std::map<std::string, std::vector<std::string>>>(username, tmp_map));
	}
}

/**
 * @brief      Sends pending messages to new online users and deletes the from the offline list.
 *
 * @param[in]  username  The username of the user
 * @param[in]  channel   The channel the message came to
 * @param      irc       Pointer to the irc structure
 */
int find_pending_messages(std::string username, std::string channel, irc_struct *irc) {

	std::map<std::string, std::map<std::string, std::vector<std::string>>>::iterator iter_user;
	std::map<std::string, std::vector<std::string>>::iterator iter_channel;

	//check whether there are any pending messages
	if ((iter_user = irc->offline_users.find(username)) != irc->offline_users.end()) {
		// if yes, check whether they are of this channel
		if ((iter_channel = iter_user->second.find(channel)) != iter_user->second.end()) {
			
			for (auto &tmp_message : iter_channel->second) {
				if (send_irc_message("PRIVMSG " + channel + " :" + tmp_message + "\r\n") != ALL_OK) {
					return TCP_MESSAGE_SEND_ERR;
				}
			}
			
			if (iter_user->second.size() == 1) {
				// if they have no more pending messages on other channels, delete them from the offline list
				irc->offline_users.erase(iter_user);
			}
			else {
				iter_user->second.erase(iter_channel);
			}
		}
	}

	return ALL_OK;
}

/**
 * @brief      Handles the irc NAMES command (353).
 *
 * @param[in]  prefix           The prefix of the message containing channel
 * @param[in]  message_content  Contains the string of users delimited by spaces
 * @param      irc              Pointer to the irc structure
 */
int command_names(std::string prefix, std::string message_content, irc_struct *irc) {
	
	size_t pos;
	std::string tmp_username, channel_name;
	std::istringstream tmp_buf(message_content);
	//prefix = to_uppercase(prefix);

	prefix = prefix.substr(1);
	prefix = prefix.substr(0, prefix.find(":") - 1);
	
	if (((pos = prefix.find("#")) != std::string::npos) || ((pos = prefix.find("&")) != std::string::npos)) {
		channel_name = to_uppercase(prefix.substr(pos));
	}

	for (auto &channel : irc->channels_list) {
		if (!strcmp(channel.c_str(), channel_name.c_str())) {
			// parse evey username in the list delimited by a space
			while (std::getline(tmp_buf, tmp_username, ' ')) {
				// get rid of the possible [@|+] at the beginning of username
				if (!strcmp(tmp_username.substr(0,1).c_str(), "@") || !strcmp(tmp_username.substr(0,1).c_str(), "+")) {
					tmp_username = tmp_username.substr(1);
				}

				if (command_join(tmp_username, channel, irc) != ALL_OK) {
					return TCP_MESSAGE_SEND_ERR;
				}
				
			}
			// we break after a match was found
			break;
		}
	}

	return ALL_OK;
}

/**
 * @brief      Handles the irc JOIN command.
 *
 * @param[in]  username  The username of the user
 * @param      channel   The channel the message was received from
 * @param      irc       The irc Pointer to the irc structure
 */
int command_join(std::string username, std::string channel, irc_struct *irc) {
	
	std::map<std::string,std::vector<std::string>>::iterator iter = irc->online_users.find(username);

	// if such a user is not yet online on other channels, create a new one
	if (iter == irc->online_users.end()) { 
		std::vector<std::string> tmp_vector;
		tmp_vector.push_back(channel);
		irc->online_users.insert(std::pair<std::string, std::vector<std::string>>(username, tmp_vector));
	}
	// if they are already online, simply expand their channels list
	else {
		if (std::find(iter->second.begin(), iter->second.end(), channel) == iter->second.end()) {
			iter->second.push_back(channel);
		}
	}

	// checking for any unfinished business with messages
	if (find_pending_messages(username, channel, irc) != ALL_OK) {
		return TCP_MESSAGE_SEND_ERR;
	}
	else {
		return ALL_OK;
	}
}

/**
 * @brief      Handles the irc PART command.
 *
 * @param[in]  username  The username of the user
 * @param      channel   The channel the message was received from
 * @param      irc       The irc Pointer to the irc structure
 */
void command_part(std::string username, std::string channel, irc_struct *irc) {
	
	std::map<std::string,std::vector<std::string>>::iterator iter;
	iter = irc->online_users.find(username);

	// if they only had one channel, delete them completely
	if (iter->second.size() == 1) {
		irc->online_users.erase(iter);
	}
	// else only delete that single channel
	else {
		iter->second.erase(std::remove(iter->second.begin(), iter->second.end(), channel), iter->second.end());
	}

	return;
}

/**
 * @brief      Handles the irc QUIT command.
 *
 * @param[in]  username  The username of the user
 * @param      irc       The irc Pointer to the irc structure
 */
void command_quit(std::string username, irc_struct *irc) {
	
	std::map<std::string,std::vector<std::string>>::iterator iter;
	std::string tmp_string;
	
	// look the user up
	iter = irc->online_users.find(username);
	// delete them from the online list
	irc->online_users.erase(iter);

	return;
}

/**
 * @brief      Changes nickname of a user in the user map.
 *
 * @param[in]  username  The username of the user
 * @param[in]  newname   The new name of the user
 * @param      irc       Pointer to the irc structure
 */
int command_nick(std::string username, std::string newname, irc_struct *irc) {
	
	std::map<std::string,std::vector<std::string>>::iterator iter;
	
	iter = irc->online_users.find(username);
	// create a new user with the nickname and old data
	irc->online_users.insert(std::pair<std::string, std::vector<std::string>>(newname, iter->second));
	irc->online_users.erase(iter);
	
	// check if there are any pending messages for the new nick
	iter = irc->online_users.find(newname);
	for (auto &channel : iter->second) {
		if (find_pending_messages(newname, channel, irc) != ALL_OK) {
			return TCP_MESSAGE_SEND_ERR;
		}
	}

	return ALL_OK;
}
/**
 * @brief      Deletes a kicked username from the online_list
 *
 * @param[in]  username  The username of the user
 * @param[in]  newname   The new name of the user
 * @param      irc       Pointer to the irc structure
 */
int command_kick(std::string username, std::string channel, irc_struct *irc) {
	
	if (!strcmp(username.c_str(), login.c_str())) {
		return BOT_KICK;
	}
	else {
		command_part(username, channel, irc);
		return ALL_OK;
	}
}

/**
 * @brief      Looks for ?msg and ?today in messages received from the server
 *  		   and invokes the respective function for their handling.
 *
 * @param[in]  channel          The channel the message was received from
 * @param[in]  message_content  The content part of the message received
 * @param      irc              Pointer to the irc structure
 */

int find_special_keywords(std::string channel, std::string message_content, irc_struct *irc) {
	
	if (regex_match(message_content, std::regex("^\\?today"))) {
		
		// create a timestamp
		char tmp_time[1024];
		time_t timestamp = time(0);
		struct tm tm_struct = *localtime(&timestamp);
		strftime(tmp_time, sizeof(tmp_time), "%d.%m.%Y", &tm_struct);
		// send the message
		if (send_irc_message("PRIVMSG " + channel + " :" + std::string(tmp_time) + "\r\n") != ALL_OK) {
			return TCP_MESSAGE_SEND_ERR;
		}
	}
	else if(regex_match(message_content, std::regex("^\\?msg \\S+:.+"))) {
		
		// get the username to which the message is to be sent
		message_content = message_content.substr(5);
		std::string username = message_content.substr(0, message_content.find(":"));
	
		// if they are online on that channel
		if (check_if_online(username, channel, irc)) {
			if (send_irc_message("PRIVMSG " + channel + " :" + message_content + "\r\n") != ALL_OK) {
				return TCP_MESSAGE_SEND_ERR;
			}
		}
		// if not, add the username and message to pending list
		else {
			save_pending_message(username, channel, message_content, irc);
		}
	}

	return ALL_OK;
}

/**
 * @brief      Looks for the keyword (specified in command line argument) in
 * 			   messages received from the server.
 *
 * @param[in]  message_content  The content part of the message received
 * @param      irc      		Pointer to the irc structure
 *
 * @return     True when a match was found, false otherwise
 */
bool find_argument_keywords(std::string message_content, irc_struct *irc) {
	
	size_t tmp_pos;
	
	for (auto &key_word : irc->key_word_list) {
		// if the key_word is somewhere in the string
		if ((tmp_pos = message_content.find(key_word)) != std::string::npos) {
			
			// the word is not at the end nor at the beginning
			if ((tmp_pos != 0) && ((tmp_pos + key_word.length()) < message_content.length())) {
				if (regex_match(message_content.substr(tmp_pos - 1, 1), std::regex("[[:space:]]")) &&
					regex_match(message_content.substr(tmp_pos + key_word.length(), 1), std::regex("[[:space:]]"))) {
					return true;
				}
			}
			// it is at the end
			else if (tmp_pos != 0) {
				if (regex_match(message_content.substr(tmp_pos - 1, 1), std::regex("[[:space:]]"))) {
					return true;
				}
			}
			// it is at the beginning
			else if ((tmp_pos + key_word.length()) < message_content.length()) {
				if (regex_match(message_content.substr(tmp_pos + key_word.length(), 1), std::regex("[[:space:]]"))) {
					return true;
				}
			}
			else if (!key_word.compare(message_content)) {
				return true;
			}
		}
	}

	return false;
}

/**
 * @brief      Gets the command data.
 *
 * @param[in]  command   The command toggle
 * @param[in]  message   The message
 * @param      username  The username
 * @param      channel   The channel
 * @param[in]  position  The position in string
 */
void get_command_data(int command, std::string message, std::string *username, std::string *channel, size_t position) {
	
		if (command == PRIVMSG || command == JOIN || command == PART) {
			*channel = to_uppercase(skip_spaces(message + "\r\n", &position));
			*username = get_username(message);
		}
		else if (command == NOTICE) {
			*username = skip_spaces(message + "\r\n", &position);
		}
		else if (command == QUIT || command == NICK) {
			*username = get_username(message);
		}
		else if (command == KICK) {
			*channel = to_uppercase(skip_spaces(message, &position));
			*username = skip_spaces(message, &position);
		}

		return;
}

/**
 * @brief      Determines which command function is to be invoked.
 *
 * @param[in]  message  The message received from a server
 * @param      irc      Pointer to the irc structure
 */
int determine_command(std::string message, syslog_struct *syslog, irc_struct *irc) {

	std::string command, channel, username;
	size_t position = 0, tmp_res;

	// get rid of the trailing CRLF
	message = message.substr(0, message.length() - 2);

	// ============ PING COMMAND ============================== //
	if (!strcmp(message.substr(0,4).c_str(), "PING")) {

		if (send_irc_message("PONG" + message.substr(4, message.length()) + "\r\n") != ALL_OK) {
			return TCP_MESSAGE_SEND_ERR;
		}
		else {
			return ALL_OK;
		}
	}

	if ((position = message.find(' ')) != std::string::npos) {
		// parse the command
		command = skip_spaces(message, &position);

		/* ============ PRIVMSG COMMAND =========================== */
		if (!strcmp(command.c_str(), "PRIVMSG")) {
			
			get_command_data(PRIVMSG, message, &username, &channel, position);
			
			// if it was not a prive message /msg, look for special keywords
			if (strcmp(channel.c_str(), to_uppercase(login).c_str())) {
				 if (find_special_keywords(channel, get_message_content(message), irc) != ALL_OK) {
				 	return TCP_MESSAGE_SEND_ERR;
				 }
			}
			//look for keywords from highlights
			if ((irc->key_word_list.size() > 0) && find_argument_keywords(get_message_content(message), irc)) {
				tmp_res = send_syslog_message(create_syslog_message(username, get_message_content(message), syslog), syslog);
				if (tmp_res == UDP_MESSAGE_SEND_ERR) {
					return UDP_MESSAGE_SEND_ERR;
				}
			}
		}
		/* ============ NOTICE COMMAND ============================ */
		else if (!strcmp(command.c_str(), "NOTICE")) {
			
			get_command_data(NOTICE, message, &username, &channel, position);
			
			//look for keywords from highlights
			if (strcmp(username.c_str(), "*")) {
				if ((irc->key_word_list.size() > 0) && find_argument_keywords(get_message_content(message), irc)) {
					tmp_res = send_syslog_message(create_syslog_message(username, get_message_content(message), syslog), syslog);
					if (tmp_res == UDP_MESSAGE_SEND_ERR) {
						return UDP_MESSAGE_SEND_ERR;
					}
				}
			}
		}
		/* ============ NAMES COMMAND ============================= */
		if (!strcmp(command.c_str(), "353")) { 
			if (command_names(message, get_message_content(message), irc) != ALL_OK) {
				return TCP_MESSAGE_SEND_ERR;
			}
		}
		/* ============ JOIN COMMAND ============================== */
		else if (!strcmp(command.c_str(), "JOIN")) { 
			
			get_command_data(JOIN, message, &username, &channel, position);
			
			// ignore JOIN message with our bots name
			if (!strcmp(username.c_str(), login.c_str())) {
				return ALL_OK;
			}

			if (command_join(username, channel, irc) != ALL_OK) {
				return TCP_MESSAGE_SEND_ERR;
			}
		}
		/* ============ QUIT COMMAND ============================== */
		else if (!strcmp(command.c_str(), "QUIT")) { 	
			get_command_data(QUIT, message, &username, &channel, position);
			command_quit(username, irc);
		}
		/* ============ PART COMMAND ============================== */
		else if (!strcmp(command.c_str(), "PART")) { 
			get_command_data(PART, message, &username, &channel, position);
			command_part(username, channel, irc);
		}
		/* ============ NICK COMMAND ============================== */
		else if (!strcmp(command.c_str(), "NICK")) { 			
			get_command_data(NICK, message, &username, &channel, position);
			if (command_nick(username, get_message_content(message), irc) != ALL_OK) {
				return TCP_MESSAGE_SEND_ERR;
			}
		}
		/* ============ KICK COMMAND ============================== */
		else if (!strcmp(command.c_str(), "KICK")) {
			get_command_data(KICK, message, &username, &channel, position);
			if (command_kick(username, channel, irc) != ALL_OK) {
				return BOT_KICK;
			}
		}
		/* ============ ERRORS ==================================== */
		else if (!strcmp(command.substr(0,1).c_str(), "4") || !strcmp(command.substr(0,1).c_str(), "5")) {
			return SERVER_ERR;
		}
	}

	return ALL_OK;

}
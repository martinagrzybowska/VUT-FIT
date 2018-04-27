/*
 * \file arguments.cpp
 * \brief File contaning functions parsing command line arguments
 * \author: Martina Grzybowska, xgrzyb00
 * 
 * Project for ISA 2017/2018
 * 
 */

#include "isabot.h"

/**
 * @brief      Parses arguments from command line and stores the values in respective structures.
 *
 * @param[in]  argc    Number of command line arguments
 * @param      argv    Command line arguments array
 * @param      irc     Pointer to the irc structure
 * @param      syslog  Pointer to the syslog structure
 *
 * @return     Various error codes or ALL_OK (0)
 */
int parse_args(int argc, char const *argv[], irc_struct *irc, syslog_struct *syslog) {
	
	size_t tmp_size;
	std::string tmp_string;
	char *str_err;
	
	// there are two compulsory arguments
	if (argc >= 3) {
		
		// syslog message TAG
		syslog->tag = "isabot";
		
		// irc connection hostname and port
		tmp_string = argv[1];
		if ((tmp_size = tmp_string.find(":")) != std::string::npos) {
			
			// checking hostname: missing port
			if (tmp_size + 1 == tmp_string.length()) {
				return ARGUMENTS_ERR;
			}

			irc->host_name = tmp_string.substr(0, tmp_size);
			irc->port_num = strtol(tmp_string.substr(++tmp_size).c_str(), &str_err, 10);

			if (*str_err != '\0' || irc->port_num <= 0) {
				return ARGUMENTS_ERR;
			}
		}
		else {
			// if port was not specified, we use the default one
			irc->host_name = tmp_string;
			irc->port_num = DEFAULT_PORT;
		}

		//checking whether channels names are all valid
		if (split_by_delimiter(argv[2], &irc->channels_list, true) != ALL_OK) {
			return ARGUMENTS_ERR;
		}
		
		irc->channel_print = argv[2];
		irc->channel_print = to_uppercase(irc->channel_print);
	
		// if there are any optional arguments, we loop through them
		for (int index = 3; index < argc; index++) {
			tmp_string = argv[index];
			if (!strcmp(tmp_string.substr(0,1).c_str(), "-")) {
				// -s argument stating syslog server IP address
				if ((!strcmp(tmp_string.substr(1,1).c_str(), "s")) && syslog->syslog_ip.empty()) {
					syslog->syslog_ip = argv[++index];
				}
				// -l argument stating key words to be looked for in irc messages
				else if ((!strcmp(tmp_string.substr(1,1).c_str(), "l")) && irc->key_word_list.empty()) {
					// split the string delimited by comma into a vector of strings
					split_by_delimiter(argv[++index], &irc->key_word_list, false);

				}
				// -h or --help, prints help message
				else if ((!strcmp(tmp_string.substr(1,1).c_str(), "h") && tmp_string.length() == 2) || !strcmp(tmp_string.c_str(), "--help")) {
					print_help();
					return HELP;
				}
				else {
					return ARGUMENTS_ERR;
				}
			}
			else {
				return ARGUMENTS_ERR;
			}
		}
	}
	else if (argc == 2) {
		tmp_string = argv[1];
		if (!strcmp(tmp_string.c_str(), "--help") || !strcmp(tmp_string.c_str(), "-h")) {
			print_help();
			return HELP;
		}
		else {
			return ARGUMENTS_ERR;
		}
	}
	else {
		return ARGUMENTS_ERR;
	}

	// if syslog server address remains empty after parsing all arguments, we use the default one
	if (syslog->syslog_ip.empty()) {
		syslog->syslog_ip = "127.0.0.1";
	}

	return ALL_OK;
}

/**
 * @brief      Splits string into a vector of strings by delimiter.
 *
 * @param      argv        The argv
 * @param      tmp_vector  The temporary vector
 * @param[in]  toggle      The toggle
 *
 * @return     Various error codes or ALL_OK (0)
 */
int split_by_delimiter(char const *argv, std::vector<std::string> *tmp_vector, bool toggle) {
	
	std::istringstream tmp_buf(argv);
	std::string tmp_string;
	
	while (std::getline(tmp_buf, tmp_string, ',')) {
		// channels must start either with # or &
		if (toggle && (strcmp(tmp_string.substr(0,1).c_str(), "#") && strcmp(tmp_string.substr(0,1).c_str(), "&"))) {
			return ARGUMENTS_ERR;
		}
		else {
			if (toggle) {
				tmp_string = to_uppercase(tmp_string);
			}

			(*tmp_vector).push_back(tmp_string);
		}
	}
	return ALL_OK;
}
/**
 * @brief      Prints help.
 */
void print_help() {
	printf("IRC bot with SYSLOG logging\n");
	printf("isabot HOST[:PORT] CHANNELS [-s SYSLOG_SERVER] [-l HIGHLIGHT] [-h|--help]\n");
	printf("HOST - name of a server\n");
	printf("PORT - port number of the server (default 6667)\n");
	printf("CHANNELS - name of the channel/s to connect to\n");
	printf("-s SYSLOG_SERVER - IP address of the SYSLOG server\n");
	printf("-l HIGHLIGHT - a list of keywords which are to be logged\n");
}
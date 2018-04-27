#include "shared.h"
#include "ftrestd.h"

/**
 * P_args parses command line arguments
 */
int P_args(int argc, char * argv[], int *port, string *directory) {
	
	int out;
	char *str_err;
	bool flag_r = false, flag_p = false;
   
	while ((out = getopt(argc, argv, "r:p:")) != EOF) {
		switch (out) {
			case 'r':
				if (flag_r == false) {
					struct stat buf;
					if (stat(optarg, &buf) != -1) {
						if (S_ISDIR(buf.st_mode)) {
							*directory = optarg;
							flag_r = true;
							break;
						}
					}
				}
				return OPTS_ERR;
			case 'p':
				if (flag_p == false) {
					*port = strtol(optarg, &str_err, 10);
					//check range of available ports
					if (*str_err != '\0' || *port > 65535 || *port < 1024) {
						return OPTS_ERR;
					}
					flag_p = true;
					break;
				}
				else {
					return OPTS_ERR;
				}
			default:
				return OPTS_ERR;
		 }
	 }
	 argc -= optind;
	 argv += optind;

	 if (argc != 0) {
		return OPTS_ERR;
	 }

	 return ALL_OK;
}
/**
 * Ch_remote_path performs a check whether the remote path is correct
 * in combination with the given command
 */
int Ch_remote_path(string check_path, string root_dir, TS_args *Uni_struct) {
	
	string user_account;

	
	// - - - - - - - - - - - - - - Checking slash
	//ditch first slash
	check_path = check_path.substr(1);
	
	// if there was no slash after the initial one, only LST is valid
	if (check_path.find('/') == string::npos) {
		if (Uni_struct->command == LST) {
			user_account = '/' + check_path;
		}
		else {
			return FORBIDDEN; //Unknown error
		}
	}
	else {
		//if /foo/ and nothing else
		if (check_path.substr(check_path.find('/') + 1) == "") {
			if (Uni_struct->command == LST) {
				user_account = '/' + check_path.substr(0, check_path.length() - 1);
			}
			else {
				return FORBIDDEN; //Unknown error
			}
		}
		//if /foo/boo, take the foo for further checking
		else {
			user_account = '/' + check_path.substr(0, check_path.find('/'));
		}
	}
	
	check_path = '/' + check_path;
	// if foo is a dir, boo is user, if the last char of root is slash
	if (root_dir.substr(root_dir.length() - 1) == "/") {
		Uni_struct->path = root_dir.substr(0, root_dir.length() - 1) + check_path;
		user_account = root_dir.substr(0, root_dir.length() - 1) + user_account;
	}
	else {
		Uni_struct->path = root_dir + check_path;
		user_account = root_dir + user_account;
	}

	// - - - - - - - - - - - - - - Deleting "%20"
	size_t iter;
	while ((iter = Uni_struct->path.find("%20")) != string::npos) {
		Uni_struct->path = Uni_struct->path.replace(iter, 3, " ");
	}
	while ((iter = user_account.find("%20")) != string::npos) {
		user_account = user_account.replace(iter, 3, " ");
	}
	
	// - - - - - - - - - - - - - - Checking user existence
	struct stat buf;
	
	if (stat(user_account.c_str(), &buf) != -1) {
		if (S_ISDIR(buf.st_mode)) {
			;
		}
		else {
			return N_EXIST_USR; //User account not found
		}
	}
	else {
		return N_EXIST_USR; //User account not found
	}
	// - - - - - - - - - - - - - - Forbidding backtracking
	if (Uni_struct->path.find("/../") != string::npos) {
		return FORBIDDEN;
	}
	
	// - - - - - - - - - - - - - - Checking dir/file existence and combinations
	if (stat(Uni_struct->path.c_str(), &buf) != -1) {
		if (S_ISDIR(buf.st_mode)) {
			//  "Not a file." when REMOTE-PATH points to a dir, but either DEL or GET was used
			//  PUT wouldnt know what to name the file, if only dir was specified
			if (Uni_struct->command == DEL || Uni_struct->command == GET || Uni_struct->command == PUT) {
				return WRONG_TYPE_FILE;
			}
			// "Already exists." when REMOTE-PATH points to a dir/file that already exists and either MKD or PUT was used
			else if (Uni_struct->command == MKD) {
				return EXIST;
			}
		}
		else if (S_ISREG(buf.st_mode)) {
			//  "Not a dir." when REMOTE-PATH points to a file, but either RMD or LST was used
			//  MKD would not be able to create a dir with a name of an existing file
			if (Uni_struct->command == LST || Uni_struct->command == RMD || Uni_struct->command == MKD) {
				return WRONG_TYPE_DIR;
			}
			// "Already exists." when REMOTE-PATH points to a dir/file that already exists and either MKD or PUT was used
			else if (Uni_struct->command == PUT) {
				return EXIST;
			}
		}
		//something not a file or not a dir
		else {
			return HTTP_ERR;
		}
	}
	//if it does not exist, it is ok for MKD or PUT so far, will check later
	else if (Uni_struct->command == MKD || Uni_struct->command == PUT) {
		;
	}
	//  "File not found." file does not exists
	else if (Uni_struct->command == DEL || Uni_struct->command == GET) {
		return N_EXIST_FILE;
	}
	// "Directory not found." directory does not exist
	else if (Uni_struct->command == LST || Uni_struct->command == RMD) {
		return N_EXIST_DIR;
	}
	// Shouldn't really be able to get here
	else {
		return HTTP_ERR;
	}

	return ALL_OK;
}
/**
 * P_header parses incoming header string and the contents
 */
int P_header(string parse_header, string root_dir, TS_args *header_struct) {
	
	string tmp_aid, tmp_aid2;
	
	//get content length in order to know number of data bytes
	char*str_err;
	tmp_aid = parse_header.substr(parse_header.find("Content-Length: ") + 16);
	header_struct->content_length = (int) strtol(tmp_aid.substr(0, tmp_aid.find("\r\n")).c_str(), &str_err, 10);
	
	//get the first line
	tmp_aid = parse_header.substr(0, parse_header.find("\r\n"));
	if (tmp_aid.substr(tmp_aid.length() - 9) != " HTTP/1.1") {
		return HTTP_ERR;
	}
	//ditch " HTTP/1.1"
	tmp_aid = tmp_aid.substr(0, tmp_aid.length() - 9); 

	if (tmp_aid.substr(tmp_aid.length() - 4) == "file") {
		header_struct->type = "file";
		tmp_aid = tmp_aid.substr(0, (tmp_aid.length() - 10));
	}
	else if (tmp_aid.substr(tmp_aid.length() - 6) == "folder") {
		header_struct->type = "folder";
		tmp_aid = tmp_aid.substr(0, tmp_aid.length() - 12);
	}
	else {
		return HTTP_ERR;
	}
	
	//load commands
	tmp_aid2 = tmp_aid.substr(0, 4);
	if (tmp_aid2 == "GET ") {
		if (header_struct->type == "file") {
			header_struct->command = GET;
		}
		else {
			header_struct->command = LST;
		}
	}
	else if (tmp_aid2 == "PUT ") {
		if (header_struct->type == "file") {
			header_struct->command = PUT;
		}
		else {
			header_struct->command = MKD;
		}
	}
	else if (tmp_aid2 == "DELE") {
		if (header_struct->type == "file") {
			header_struct->command = DEL;
		}
		else {
			header_struct->command = RMD;
		}
	}
	else {
		return HTTP_ERR;
	}
	//ditch command 
	if (header_struct->command == DEL || header_struct->command == RMD) {
		tmp_aid = tmp_aid.substr(7);
	}
	else {
		tmp_aid = tmp_aid.substr(4);
	}
	

	//get data
	header_struct->data = parse_header.substr(parse_header.find("\r\n\r\n") + 4, header_struct->content_length);
	//check whether the path is correct and its combination with the current command
	int check_err = Ch_remote_path(tmp_aid, root_dir, header_struct);
	
	return check_err;
}
/**
 * command_DEL represents DEL
 */
int command_DEL(string path_to_dir) {
	return remove(path_to_dir.c_str());
}
/**
 * command_MKD represents MKD
 */
int command_MKD(string path_to_dir) {
	errno = 0;
	if (mkdir(path_to_dir.c_str(), 0755) == -1) {
		if (errno == ENOTDIR || errno == ENOENT) {
			return N_EXIST_DIR;
		}
		else {
			return COMM_ERR;
		}
	}
	return ALL_OK;
}
/**
 * command_RMD represents RMD
 */
int command_RMD(string path_to_dir) {
	if (!rmdir(path_to_dir.c_str())) {
		return ALL_OK;
	}
	//"Directory not empty." if remote path points to a dir which is not empty
	else {
		return COMM_ERR;
	}
}
/**
 * command_LST represents LST
 */
string command_LST(string path_to_dir) {
	DIR *d;
	string rollingDung;
	struct dirent *dir;
	d = opendir(path_to_dir.c_str());
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			string tmp_aid = string(dir->d_name);
			if (tmp_aid.substr(0, 1) != ".") {
				rollingDung += tmp_aid + '\n';
			}
		}
		closedir(d);
		return rollingDung;
	}
	//if something went wrong, return ".", because it can not be there
	rollingDung = ".";
	return rollingDung;
}
/**
 * command_PUT represents PUT
 */
int command_PUT(TS_args *put_here) {
	errno = 0;
	FILE *received_file = fopen(put_here->path.c_str(), "w");
	if (received_file == NULL) {
		if (errno == ENOENT || errno == ENOTDIR) {
			return N_EXIST_DIR;
		}
		else {
			return COMM_ERR;
		}
	}
	fwrite(put_here->data.c_str(), sizeof(char), put_here->content_length, received_file);
	fclose(received_file);
	return ALL_OK;
}
/**
 * command_GET represents GET
 */
TS_args command_GET(TS_args *get_this) {
	
	ifstream tmp_aid(get_this->path.c_str(), ios::binary|ios::ate);
	ifstream::pos_type tmp_aid2 = tmp_aid.tellg();

	std::vector<char> byte_array(tmp_aid2);

	tmp_aid.seekg(0, ios::beg);
	tmp_aid.read(&byte_array[0], tmp_aid2);

	get_this->content_length = byte_array.size();
	get_this->data = string(byte_array.begin(), byte_array.end());
	
	return *get_this;
}
/**
 * create_response combines gathered information from both structures and
 * creates a response string which is to be sent to the client
 */
string create_response(T_post build_struct, TS_args add_struct) {
	
	string rollingDung = "HTTP/1.1 " + build_struct.code_phrase + "\r\n";
	
	char buffer[1024];
	time_t timestamp = time(0);
	struct tm tm_struct = *localtime(&timestamp);
	strftime(buffer, sizeof buffer, "%a, %d %b %Y %H:%M:%S %Z", &tm_struct);
	rollingDung += "Date: " + string(buffer) + "\r\n";

	// - - - - - - - - - - - - - - - - - other commands
	if (build_struct.which_comm != GET) {
		rollingDung += "Content-Type: text/plain\r\n";
		
		if (build_struct.which_comm == LST) {
			rollingDung += "Content-Length: " + to_string(build_struct.text_resp.length()) + "\r\nContent-Encoding: identity\r\n\r\n";
			rollingDung += build_struct.text_resp;
		}
		else if (build_struct.was_error == false) {
			rollingDung += "Content-Length: 15\r\nContent-Encoding: identity\r\n\r\nMessage: All ok";
		}
		else {
			rollingDung += "Content-Length: " + to_string(build_struct.text_resp.length()) + "\r\nContent-Encoding: identity\r\n\r\n";
			rollingDung += build_struct.text_resp;
		}
	}
	// - - - - - - - - - - - - - - - - - GET command
	else {
		// if everything was ok, stick data onto the header
		if (build_struct.was_error == false) {
			rollingDung += "Content-Type: octet/stream\r\n";
			rollingDung += "Content-Length: " + to_string(add_struct.content_length) + "\r\nContent-Encoding: identity\r\n\r\n";
			rollingDung += add_struct.data;
		}
		// else add error message
		else {
			rollingDung += "Content-Type: text/plain\r\n";
			rollingDung += "Content-Length: " + to_string(build_struct.text_resp.length()) + "\r\nContent-Encoding: identity\r\n\r\n";
			rollingDung += build_struct.text_resp;
		}
	}

	return rollingDung;
}
/**
 * evaulate_codes evaulates either number of the error code a which command was used,
 * it also calls command functions if everything was correct
 */
void evaulate_codes(int controller, T_post *fill_struct, TS_args *check_struct) {
	fill_struct->text_resp = "";
	//- - - - - - - - - - - - - - errors
	if (controller == HTTP_ERR) {
		fill_struct->code_phrase = "406 Not acceptable";
		fill_struct->text_resp = "Unknown error."; 
		fill_struct->was_error = true;
	}
	else if (controller == WRONG_TYPE_DIR) {
		fill_struct->code_phrase = "400 Bad Request";
		fill_struct->text_resp = "Not a directory."; //lst rmd
		fill_struct->was_error = true;
	}
	else if (controller == FORBIDDEN) {
		fill_struct->code_phrase = "403 Forbidden";
		fill_struct->text_resp = "Unknown error."; //lst rmd
		fill_struct->was_error = true;
	}
	else if (controller == N_EXIST_USR) {
		fill_struct->code_phrase = "404 Not found";
		fill_struct->text_resp = "User account not found."; //lst rmd
		fill_struct->was_error = true;
	}
	else if (controller == WRONG_TYPE_FILE) {
		fill_struct->code_phrase = "400 Bad Request";
		fill_struct->text_resp = "Not a file."; //del get
		fill_struct->was_error = true;
	}
	else if (controller == EXIST) {
		fill_struct->code_phrase = "418 I'm a teapot ¯\\_(ツ)_/¯";
		fill_struct->text_resp = "Already exists."; //mkd put
		fill_struct->was_error = true;
	}
	else if (controller == N_EXIST_FILE) {
		fill_struct->code_phrase = "404 Not found";
		fill_struct->text_resp = "File not found."; //del get
		fill_struct->was_error = true;
	}
	else if (controller == N_EXIST_DIR) {
		fill_struct->code_phrase = "404 Not found";
		fill_struct->text_resp = "Directory not found."; //lst rmd
		fill_struct->was_error = true;
	}
	//- - - - - - - - - - - - - - command functions
	else {
		//- - - - - - - DEL
		if (check_struct->command == DEL) {
			if (command_DEL(check_struct->path)) {
				fill_struct->code_phrase = "500 Internal Server Error";
				fill_struct->text_resp = "Unknown error.";
				fill_struct->was_error = true;
			}
			else {
				fill_struct->code_phrase = "200 OK";
				fill_struct->was_error = false;
			}
		}
		//- - - - - - - RMD
		else if (check_struct->command == RMD) {
			if (command_RMD(check_struct->path) == COMM_ERR) {
				fill_struct->code_phrase = "406 Not acceptable";
				fill_struct->text_resp = "Directory not empty."; //rmd
				fill_struct->was_error = true;
			}
			else {
				fill_struct->code_phrase = "200 OK";
				fill_struct->was_error = false;
			}
		}
		//- - - - - - - LST
		else if (check_struct->command == LST) {
			fill_struct->text_resp = command_LST(check_struct->path);
			if (fill_struct->text_resp == ".") {
				fill_struct->was_error = true;
				fill_struct->code_phrase = "500 Internal Server Error";
				fill_struct->text_resp = "Unknown error.";
			}
			else {
				fill_struct->which_comm = LST;
				fill_struct->code_phrase = "200 OK";
				fill_struct->was_error = false;
			}
		}
		//- - - - - - - MKD
		else if (check_struct->command == MKD) {
			int temp = command_MKD((check_struct->path));
			if (temp == COMM_ERR) {
				fill_struct->code_phrase = "500 Internal Server Error";
				fill_struct->text_resp = "Unknown error.";
				fill_struct->was_error = true;
			}
			else if (temp == N_EXIST_DIR) {
				fill_struct->code_phrase = "404 Not found";
				fill_struct->text_resp = "Directory not found.";
				fill_struct->was_error = true;
			}
			else {
				fill_struct->code_phrase = "200 OK";
				fill_struct->was_error = false;
			}
		}
		//- - - - - - - PUT
		else if (check_struct->command == PUT) {
			int temp = command_PUT(check_struct);
			if (temp == COMM_ERR) {
				fill_struct->code_phrase = "500 Internal Server Error";
				fill_struct->text_resp = "Unknown error.";
				fill_struct->was_error = true;
			}
			else if (temp == N_EXIST_DIR) {
				fill_struct->code_phrase = "404 Not found";
				fill_struct->text_resp = "Directory not found.";
				fill_struct->was_error = true;
			}
			else {
				fill_struct->code_phrase = "200 OK";
				fill_struct->was_error = false;
			}
		}
		//- - - - - - - GET
		else if (check_struct->command == GET) {
			*check_struct = command_GET(check_struct);
			fill_struct->which_comm = GET;
			fill_struct->code_phrase = "200 OK";
			fill_struct->was_error = false;
		}
	}
}
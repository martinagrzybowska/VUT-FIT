#include <fitkitlib.h>
#include <keyboard/keyboard.h>
#include <lcd/display.h>
#include <string.h>

char previous_pressed; 
char current_pressed;
char char_counter;
char output_storage[256];
char output_visible[32];

#define TRUE 1
#define FALSE 0
#define DELAY 25000
#define ULTIMATE_ANSWER 42


/**
 * @brief      Gets the value of the current pressed key.
 *
 * @return     TRUE if was pressed, FALSE otherwise.
 */
int get_current() {
	
	current_pressed = key_decode(read_word_keyboard_4x4());
	if (current_pressed != previous_pressed) {
		previous_pressed = current_pressed;
		return TRUE;
	}
	else {
		return FALSE;
	}
}

void print_user_help() {
}

unsigned char decode_user_cmd(char *cmd_ucase, char *cmd){
	return CMD_UNKNOWN;
}

void fpga_initialized(){
	LCD_init();
	LCD_clear();
} 

/**
 * @brief      Clears strings used for storage and display
 *
 * @param[in]  visible  Flag for clearing the visible string
 * @param[in]  storage  Flag for clearing the storage string
 */
void clear_strings(int visible, int storage) {

	LCD_clear();

	int iter;
	if (visible) {
		for (iter = 0; iter < 32; iter++) {
			output_visible[iter] = '\0';
		}
	}

	if (storage) {
		for (iter = 0; iter < 256; iter++) {
			output_storage[iter] = '\0';
		}
	}

	char_counter = 0;

}

/**
 * @brief      Delays the outputing of string and listens for key press
 *
 * @param[in]  time  Time to delay
 *
 * @return     TRUE if key * was pressed while looping, FALSE otherwise
 */
int delay(int time) {
	long long i;
  	
  	for(i = 0; i < time; i++) {
  		if (get_current()) {
  			if (current_pressed == 42) {
  				return TRUE;
  			}
  		}
  	}
  	return FALSE;
}

/**
 * @brief      Creates an effect of upwards rotation of a text on display
 */
void rotate_upwards() {

	char display_window[32];
	int position = 0;
	int iter;

	int current_length = strlen(output_storage);
	// if storage string does not fill the whole 2x16, fill it with blanks
	if (current_length < 32) {
		for (iter = current_length; iter < 32; iter++) {
			output_storage[iter] = 32;
		}
	}
	else if (current_length % 32 != 0) {
		for (iter = current_length; iter < current_length + (31 - (current_length % 32)); iter++) {
			output_storage[iter] = 32;
		}
	}

	current_length = strlen(output_storage);

	while (ULTIMATE_ANSWER) {
		
		for (iter = 0; iter < 32; iter++) {
			display_window[iter] = output_storage[(position * 32) + iter];
		}

		LCD_clear();
		LCD_write_string(display_window);
		position++;

		if ((position * 32) > current_length) {
			for (iter = 0; iter < 32; iter++) {
				display_window[iter] = output_storage[iter];
			}
			position = 0;
		}

		if (!delay(15000)) {
			continue;
		}
		else {
			break;
		}
	}

	clear_strings(TRUE, TRUE);
	return;

}

/**
 * @brief      Creates an effect of downwards rotation of a text on display
 */
void rotate_downwards() {

	char display_window[32];
	int position = 1;
	int iter;

	int current_length = strlen(output_storage);
	// if storage string does not fill the whole 2x16, fill it with blanks
	if (current_length < 32) {
		for (iter = current_length; iter < 32; iter++) {
			output_storage[iter] = 32;
		}
	}
	else if (current_length % 32 != 0) {
		for (iter = current_length; iter < current_length + (32 - (current_length % 32)); iter++) {
			output_storage[iter] = 32;
		}
	}

	current_length = strlen(output_storage);

	while (ULTIMATE_ANSWER) {
		
		for (iter = 0; iter < 32; iter++) {
			display_window[iter] = output_storage[current_length - (position * 32) + iter];
		}

		LCD_clear();
		LCD_write_string(display_window);
		position++;

		if (current_length - (position * 32) < 0) {
			for (iter = 0; iter < 32; iter++) {
				display_window[iter] = output_storage[current_length - 32 + iter];
			}
			position = 1;
		}

		if (!delay(15000)) {
			continue;
		}
		else {
			break;
		}
	}

	clear_strings(TRUE, TRUE);
	return;

}

/**
 * @brief      Creates an effect of sideways slide by one of a text on display
 */
void slide_right() {

	char display_window[32];
	int position = 0;
	int iter;
	int current_length = strlen(output_storage);

	while (ULTIMATE_ANSWER) {

		for (iter = 0; iter < 16; iter++) {
			if (iter + position > current_length - 1) {
				display_window[iter] = 32;
				display_window[iter + 16] = 45;
			}
			else {
				display_window[iter] = output_storage[iter + position];
				display_window[iter + 16] = 45;
			}
		}

		LCD_clear();
		LCD_write_string(display_window);
		position++;

		if (current_length <= position) {
			position = 0;
		}

		if (!delay(8000)) {
			continue;
		}
		else {
			break;
		}
	}

	clear_strings(TRUE, TRUE);
	return;

}

/**
 * @brief      Creates an effect of sideways skip by 16 of a text on display
 */
void skip_right() {

	char display_window[32];
	int position = 0;
	int iter;
	int current_length = strlen(output_storage);

	for (iter = 16; iter < 32; iter++) {
		display_window[iter] = 42;
	}

	while (ULTIMATE_ANSWER) {

		for (iter = 0; iter < 16; iter++) {
			if (iter + position > current_length - 1) {
				display_window[iter] = 32;
				display_window[iter + 16] = 42;
			}
			else {
				display_window[iter] = output_storage[iter + position];
				display_window[iter + 16] = 42;
			}
		}

		LCD_clear();
		LCD_write_string(display_window);
		position += 16;

		if (current_length <= position) {
			position = 0;
		}

		if (!delay(25000)) {
			continue;
		}
		else {
			break;
		}
	}

	clear_strings(TRUE, TRUE);
	return;

}

/**
 * @brief      Determines which effect is to be used
 */
void determine_effect() {

	while(ULTIMATE_ANSWER){
		
		if (!get_current() || current_pressed == 0) {
			continue;
		}

		if (current_pressed == 65) {
			rotate_upwards();
			return;
		}
		else if (current_pressed == 66) {
			rotate_downwards();
			return;
		}
		else if (current_pressed == 67) {
			slide_right();
			return;
		}
		else if (current_pressed == 68) {
			skip_right();
			return;
		}
	}

	return;
}

/**
 * @brief      Adds or replaces letters in storage and visible strings
 *
 * @param[in]  current        The current letter
 * @param[in]  to_replace     Whether the letter is to be replaced or a new one is to be typed
 * @param      press_counter  The press counter
 */
void display_new(char current, int to_replace, int* press_counter) {
	
	int length = strlen(output_storage);
	int rot_length = strlen(output_visible);
	
	if (length < 256 && rot_length < 32) {
		
		if (to_replace) {
			//repace the last letter with the new one
			output_storage[length - 1] = current;
			output_visible[rot_length - 1] = current;
		}
		else {
			//append a new letter
			output_storage[length] = current;
			output_visible[rot_length] = current;
			(*press_counter) = 1;
		}

		LCD_clear();
		LCD_write_string(output_visible);
	}
}

/**
 * @brief      Determines which one of the key's letters is to be displayed
 *
 * @param[in]  pressed        The key pressed
 * @param[in]  vals           How many values the key represents
 * @param      press_counter  The press counter
 * @param      current        The current value
 * @param[in]  first          The first value of the key
 */
void determine_output(char pressed, int vals, int* press_counter, char* current, char first) {

	if ((*press_counter % vals) == 0) {
		*current = first;
	}
	else if ((*press_counter % vals) == 1 || (*press_counter % vals) == 2) {
		if (pressed == '0' || pressed == '1') {
			*current = pressed;
		}
		else {
			(*current)++;	
		}
	}
	else if ((*press_counter % vals) == 3) {
		if (pressed == '7' || pressed == '9') {
			(*current)++;
		}
		else {
			*current = pressed;
		}
	}
	else if ((*press_counter % vals) == 4) {
		*current = pressed;
	}

	(*press_counter)++;

}

/**
 * @brief      Determines parameters of a key pressed for further use
 *
 * @param      vals       The number of values of the key
 * @param      char_code  The character code of the first value
 */
void determine_parameters(int* vals, int* char_code) { 	
	
	if (current_pressed == '0') {
		*vals = 2;
		*char_code = 32;
	}
	else if (current_pressed == '1') {
		*vals = 2;
		*char_code = 46;
	}
	else if (current_pressed == '2') {
		*vals = 4;
		*char_code = 65;
	}
	else if (current_pressed == '3') {
		*vals = 4;
		*char_code = 68;
	}
	else if (current_pressed == '4') {
		*vals = 4;
		*char_code = 71;
	}
	else if (current_pressed == '5') {
		*vals = 4;
		*char_code = 74;
	}
	else if (current_pressed == '6') {
		*vals = 4;
		*char_code = 77;
	}
	else if (current_pressed == '7') {
		*vals = 5;
		*char_code = 80;
	}
	else if (current_pressed == '8') {
		*vals = 4;
		*char_code = 84;
	}
	else if (current_pressed == '9') {
		*vals = 5;
		*char_code = 87;
	}
	else {
		*vals = 0;
		*char_code = 0;
	}
}


void read_input() {
	
	static char last = 0;
	static int press_counter = 0;
	static char current = '\0';
	
	unsigned long int time = 0;
	int values;
	int char_code;

	while (time <= DELAY) {
	
		time++;
		//time for pressing a single key more than one time
		if (time == DELAY - 100) {
			last = 0;
		}

		if (!get_current()) {
			continue;
		}

		if (current_pressed != 0) {

			if (current_pressed == 35 && strlen(output_storage) != 0) {
				determine_effect();
				return;
			}

			if (current_pressed == last) {
				determine_parameters(&values, &char_code);
				if (values && char_code) {
					determine_output(current_pressed, values, &press_counter, &current, char_code);
					display_new(current, TRUE, &press_counter);
					time = DELAY;
				}
			}
			else {
				//if storage is 256, only accept effects
				if (strlen(output_storage) > 255) {
					if (current_pressed != 35) {
						LCD_clear();
						strcpy(output_visible, "Out of bounds   Choose effect");
						LCD_write_string(output_visible);
						return;
					}
				}
				//if the current number of letters is greater than 31, clear the screen
				if (char_counter % 31 == 0) {
					clear_strings(TRUE, FALSE);
				}

				determine_parameters(&values, &char_code);
				if (values && char_code) {
					current = char_code;
					display_new(current, FALSE, &press_counter);
					char_counter++;
					time = DELAY;
				}
			}

			last = current_pressed;

		}
		else if (previous_pressed != 0) {
			last = previous_pressed;
		}
	}
	
	return;

}

/**
 * @brief      Main
 *
 * @return     Always TRUE
 */
int main(void){
	
	initialize_hardware();
	keyboard_init();

	while (ULTIMATE_ANSWER) {
		read_input();
		terminal_idle();                   
	}
	
	return 0;      

}

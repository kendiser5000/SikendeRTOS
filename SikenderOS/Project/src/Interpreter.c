/** @file Interpreter.c
 * @brief Runs on TM4C123
 * UART Command Line Interface
 * @author Sikender Ashraf and Sijin Woo
 */

#include "Interpreter.h"
#include "UART0.h"
#include "OS.h"
#include "cpu.h"

// Command Types
// If more commands are added, add on to the list
#define CMD_MENU				0
#define CMD_HELP				1
#define CMD_ECHO				2
#define CMD_MEASURE				3
#define CMD_DISPLAY				4
#define CMD_LED					5
#define CMD_OS					6

// Type of modules to measure. Add more to the list
#define MEASURE_ADC		1

static char GraphicRTOS[] = "      ___       ___          ___          ___\n\r     /\\  \\     /\\  \\        /\\  \\        /\\  \\\n\r    /::\\  \\    \\:\\  \\      /::\\  \\      /::\\  \\\n\r   /:/\\:\\  \\    \\:\\  \\    /:/\\:\\  \\    /:/\\ \\  \\\n\r  /::\\~\\:\\  \\   /::\\  \\  /:/  \\:\\  \\  _\\:\\~\\ \\  \\\n\r /:/\\:\\ \\:\\__\\ /:/\\:\\__\\/:/__/ \\:\\__\\/\\ \\:\\ \\ \\__\\\n\r \\/_|::\\/:/  //:/  \\/__/\\:\\  \\ /:/  /\\:\\ \\:\\ \\/__/\n\r    |:|::/  //:/  /      \\:\\  /:/  /  \\:\\ \\:\\__\\\n\r    |:|\\/__/ \\/__/        \\:\\/:/  /    \\:\\/:/  /\n\r    |:|  |                 \\::/  /      \\::/  /\n\r     \\|__|                  \\/__/        \\/__/\n\r";

char *readline(char *str);
void parseString(char *str, char cmd[MAX_WORDS][MAX_CHARS_PER_WORD]);
int32_t findCommandType(char cmd[MAX_WORDS][MAX_CHARS_PER_WORD]);
void executeCommand(int32_t cmdType, char cmd[MAX_WORDS][MAX_CHARS_PER_WORD]);
void printWelcomeMenu(void);
void printGenericMenu(void);
void printHelpMenu(void);
void commandMeasure(char cmd[MAX_WORDS][MAX_CHARS_PER_WORD]);
void commandDisplay(char cmd[MAX_WORDS][MAX_CHARS_PER_WORD]);
void commandOS(char cmd[MAX_WORDS][MAX_CHARS_PER_WORD]);
void commandLED(char cmd[MAX_WORDS][MAX_CHARS_PER_WORD]);


/** newLine
 *	Create new line on UART interface
*/
void newLine(void){
	OutCRLF();
}


/** Interpretert
 * The following function starts Interpreter with UART
 */
void Interpreter(void){
	UART_Init();
	newLine();
	printWelcomeMenu();
	
	char *inString;
	static char cmdString[MAX_WORDS][MAX_CHARS_PER_WORD];
	int32_t cmdType = -1;
	while((inString = readline(">>")) != NULL){
		parseString(inString, cmdString);
		cmdType = findCommandType(cmdString);
		executeCommand(cmdType, cmdString);
	}
}

/** readline
 * Acts exactly the same as the readline function from readline.h library
 * @param str to be printed
 * @return string that has been typed by user
 */
char *readline(char *str){
	// must be static since dynamic memory is not available
	// length 80 was chosen because that's how many characters can fit in the default terminal
	static char inString[80];
	
	char letter = 0;
	int32_t idx = 0;
	printf("%s", str);		// print start command notifier (usually >>)
	while(letter != '\n' && letter != '\r'){
		letter = UART_InChar();
		
		// If backspace was used, decrease index
		if(letter == 0x7F){
			idx--;
		}
		
		// Check may seem redundant but this is to exclude the new line character in the string
		if(letter != '\n' && letter != '\r' && letter != 0x7F && letter != 0x1B){
			inString[idx++] = letter;
		}
		
		printf("%c", letter);
	}
	inString[idx] = 0;
	newLine();
	return inString;
}

/** parseString
 * Tokenizes each word
 * @param string to tokenize
 * @return double pointer to each word in the whole string
 */
void parseString(char *str, char cmd[MAX_WORDS][MAX_CHARS_PER_WORD]){
	// strtok is not thread safe. Strtok_r must be used to make is thread safe.
	// Both strtoks modifies the string that it tokenizes. Strtok_r reqruies the
	// savePtr to know the location that it left off at.
	char *savePtr, *quotePtr;
	
	char *token, *quoteToken;
	int32_t idx = 0;
	
	// Check if there are any quotation marks. If there are, then everything encompassed in it will
	// go into a row of cmd.
	quoteToken = strtok_r(str, "\"", &quotePtr);
	
	token = strtok_r(quoteToken, " ", &savePtr);
	while(token != NULL && idx < MAX_WORDS){
		if(strlen(token) >= MAX_CHARS_PER_WORD){
			printf("OVERFLOW");
		}else{
			memset(cmd[idx], 0, MAX_CHARS_PER_WORD);
			strncpy(cmd[idx], token, strlen(token));
			cmd[idx++][strlen(token)] = '\0';
		}
		token = strtok_r(NULL, " ", &savePtr);
	}
	
	// Copy string in quotation marks
	quoteToken = strtok_r(NULL, "\"", &quotePtr);
	memset(cmd[idx], 0, MAX_CHARS_PER_WORD);
	if(quoteToken != NULL){
		strncpy(cmd[idx], quoteToken, strlen(quoteToken));
		cmd[idx++][strlen(quoteToken)] = '\0';
	}
	
	cmd[idx][0] = 0;	// End with end character
}

/** findCommandType
 * Determines which type of command the user has typed. The first word should specify what type.
 * @return CMD id. Listed as macros at top of this file.
 */
int32_t findCommandType(char cmd[MAX_WORDS][MAX_CHARS_PER_WORD]){
	int32_t type = -1;		// Undefined type
	
	// Determine type of command
	// Menu
	if(strcmp(cmd[0], "menu") == 0 || strcmp(cmd[0], "m") == 0){
		type = CMD_MENU;
		
	// help
	}else if(strcmp(cmd[0], "help") == 0 || strcmp(cmd[0], "h") == 0){
		type = CMD_HELP;
	
	// echo
	}else if(strcmp(cmd[0], "echo") == 0){
		type = CMD_ECHO;
		
	// measure
	}else if(strcmp(cmd[0], "measure") == 0){
		type = CMD_MEASURE;
		
	// display
	}else if(strcmp(cmd[0], "display") == 0){
		type = CMD_DISPLAY;
	
	// LED
	}else if(strcmp(cmd[0], "led") == 0){
		type = CMD_LED;
	
	// OS
	}else if(strcmp(cmd[0], "os") == 0){
		type = CMD_OS;
	}
	
	return type;
}

/** executeCommand
 * NOTE!!!! Modify this function when adding more commands
 *
 */
void executeCommand(int32_t cmdType, char cmd[MAX_WORDS][MAX_CHARS_PER_WORD]){
	switch(cmdType){
		// generic menu
		case CMD_MENU:
			printGenericMenu();
			break;
		
		// Help menu
		case CMD_HELP:
			printHelpMenu();
			break;
		
		// Echoes string
		case CMD_ECHO:
			printf("%s\n\r", cmd[1]);
			break;
		
		// Get measurements
		case CMD_MEASURE:
			commandMeasure(cmd);
			break;
		
		// Display onto ST7735
		case CMD_DISPLAY:
			commandDisplay(cmd);
			break;
		
		// Change LED
		case CMD_LED:
			commandLED(cmd);
			break;
		
		// read OS time
		case CMD_OS:
			commandOS(cmd);
			break;
		
		// Undefined command
		default:
			printf("Undefined command.\n\r");
			break;
	}
}

/** printWelcomeMenu
 * Add graphics here
 */
void printWelcomeMenu(void){
	newLine(); newLine();;
	for(int32_t i = 0; i < 55; i++){
		UART_OutChar('=');
	}
	printf("\n\r\n\r%s\n\r", GraphicRTOS);
	printf("\n\rWelcome to the Sikender Ashraf's and Sijin Woo's OS.\n\r");
	for(int32_t i = 0; i < 55; i++){
		UART_OutChar('=');
	}
	newLine(); newLine();;
	printf("Type \"menu\" or \"m\" to see menu...\n\r");
}

/** printGenericMenu
 * This is a condensed list of all the possible commands
 */
void printGenericMenu(void){
	printf("Type \"help\" or \"h\" to see more details...\n\r");
	printf("echo \"<string>\"\n\r");
	printf("measure -module -channel -samples\n\r");
	printf("display -device -line \"<string>\"\n\r");
	printf("led -color -status\n\r");
	printf("os -set\n\r");
	
	newLine();
}

/** printHelpMenu
 * Prints all the details for each command that has been implemented
 */
void printHelpMenu(void){
	newLine(); newLine();;
	for(int32_t i = 0; i < 80; i++){
		UART_OutChar('=');
	}
	printf("\n\rWelcome to the manual.\n\r");
	for(int32_t i = 0; i < 80; i++){
		UART_OutChar('=');
	}
	newLine(); newLine();;
	
	// Echo command
	printf("Name:\n\r");
	printf("\techo \"<string>\"\n\r");
	printf("Description:\n\r");
	printf("This is the same as the echo command in linux. This is mainly to test if the \n\r");
	printf("microcontroller is receiving and sending correctly.\n\r\n\r");
	printf("  \"<string>\"\tString to be echoed. Make sure the string is in quotations\n\r");
	printf("\t\tor the whole message might not be reciprocated.\n\n\r\r");
	
	// Measure command
//	printf("Name:\n\r");
//	printf("\tmeasure -module -channel -samples\n\r");
//	printf("Description:\n\r");
//	printf("Gets the measurement value from the specific module. This can be reused to get\n\r");
//	printf("measurments from any module in the future such as the lidar, ultrasonic, etc.\n\r\n\r");
//	printf("  module\tModule to get the measurement from.\n\r");
//	printf("\t\tCurrently available:\n\r");
//	printf("\t\t  -ADC\n\r");
//	printf("  channel\tSpecifies which channel of the module.\n\r");
//	printf("\t\tCurrently available:\n\r");
//	printf("\t\t  -ADC : channel 0 to 11 are available\n\r");
//	printf("\t\t\t 0 : PE3\n\r");
//	printf("\t\t\t 1 : PE2\n\r");
//	printf("\t\t\t 2 : PE1\n\r");
//	printf("\t\t\t 3 : PE0\n\r");
//	printf("\t\t\t 4 : PD3\n\r");
//	printf("\t\t\t 5 : PD2\n\r");
//	printf("\t\t\t 6 : PD1\n\r");
//	printf("\t\t\t 7 : PD0\n\r");
//	printf("\t\t\t 8 : PE5\n\r");
//	printf("\t\t\t 9 : PE4\n\r");
//	printf("\t\t\t10 : PB4\n\r");
//	printf("\t\t\t11 : PB5\n\r\n\r");
//	printf("  samples\tSpecifies how many samples to retrieve.\n\r\n\r");
//	printf("  rate\tSpecifies rate(Hz) to retrieve samples. 100 <= rate <= 10000\n\r");
	
	
	// Display command
	printf("Name:\n\r");
	printf("\tdisplay -device -line \"<string>\"\n\r");
	printf("Description:\n\r");
	printf("Displays message onto the ST7735 depending on the device and row.\n\r\n\r");
	printf("  device\tSpecifies which half of the screen to display the message.\n\r");
	printf("\t\t\"0\" or \"top\" for top half, \"1\" or \"bottom\" for bottom half\n\r\n\r");
	printf("  line\t\tSpecifies which row of device section to display the message\n\r");
	printf("\t\tline must be <=7 or else nothing will be displayed\n\r\n\r");
	printf("  \"<string\"\tString to be displayed. Make sure the string is in quotations.\n\r");
	printf("\t\tor the whole message might not be reciprocated.\n\n\r\r");
	
	// LED command
	printf("Name:\n\r");
	printf("\tled -color\n\r");
	printf("Description:\n\r");
	printf("The chosen colored LED will be toggled.\n\r\n\r");
	printf("  color\t\tSpecifies which colored LED will be changed. The LED colors can\n\r");
	printf("\t\the combined if two or more LEDs are on. If red and blue LEDs\n\r");
	printf("\t\tare both on then a magenta color will appear.\n\r");
	printf("\t\t  -red\n\r");
	printf("\t\t  -green\n\r");
	printf("\t\t  -blue\n\r\n\r");
	//printf("  status\tNew state of the LED. The options are \"on\", \"off\", and \"toggle\".\n\r\n\r");
	
	// OS
	printf("Name:\n\r");
	printf("\tos -set\n\r");
	printf("Description:\n\r");
	printf("This interfaces with the OS. Currently not much support is available.\n\r");
	printf("Read and clear are only available\n\r\n\r");
	printf("  set\t\tReads/clears the periodic time counter of the OS.\n\r\n\r");
}

/** commandMeasure
 * Gathers data from the module depending on what module the user indicated
 * Format: measure -module -channel -samples -rate
 * @params cmd input by user that has been tokenized
 */
void commandMeasure(char cmd[MAX_WORDS][MAX_CHARS_PER_WORD]){
	int32_t type = -1;
	
	int32_t channel = 0;
	int32_t samples = 1;
	int32_t rate = 100;
	
	// Find which module to get data from
	if(strcmp(cmd[1], "ADC") == 0 || strcmp(cmd[1], "-ADC") == 0 || strcmp(cmd[1], "adc") == 0 || strcmp(cmd[1], "-adc") == 0){
		type = MEASURE_ADC;
	}
	
	// Find what channel
	sscanf(cmd[2], "%d", &channel);
	// In case user typed a dash in front of the number
	if(channel < 0){
		channel = -channel;
	}
	
	// Find how many samples
	if(cmd[3][0] != 0){
		sscanf(cmd[3], "%d", &samples);
		// In case user typed a dash in front of the number
		if(samples < 0){
			samples = -samples;
		}
	}
	
	// Find rate to gather samples
	if(cmd[4][0] != 0){
		sscanf(cmd[4], "%d", &rate);
		// In case user typed a dash in front of the number
		if(rate < 0){
			rate = -rate;
		}
	}
	
	switch(type){
		case MEASURE_ADC:
			printf("ADC data:\n\r");
			
			break;
		
		default:
			break;
	}
	
	newLine();
}

/** commandDisplay
 * Displays message onto the ST7735
 * Format: display -device -line "<string>"
 * @params cmd input by user that has been tokenized
 */
void commandDisplay(char cmd[MAX_WORDS][MAX_CHARS_PER_WORD]){
	
}

/** commandOS
 * Runs specified OS command
 * Format: display -device -line "<string>"
 * @params cmd input by user that has been tokenized
 */
void commandOS(char cmd[MAX_WORDS][MAX_CHARS_PER_WORD]){
	if(strcmp(cmd[1], "read") == 0 || strcmp(cmd[1], "-read") == 0){
		printf("Periodic Time: %d\n\r", OS_ReadMsTime());
	}else if(strcmp(cmd[1], "clear") == 0 || strcmp(cmd[1], "-clear") == 0){
		OS_ClearMsTime();
		printf("Periodic Cleared.\n\r");
	}
}

/** commandLED
 * Toggles specified LED
 * Format: display -device -line "<string>"
 * @params cmd input by user that has been tokenized
 */
void commandLED(char cmd[MAX_WORDS][MAX_CHARS_PER_WORD]){
	if(strcmp(cmd[1], "red") == 0 || strcmp(cmd[1], "-red") == 0){
		printf("Togglin RED LED");
		RED_LED ^= RED_BLINK;
	}else if(strcmp(cmd[1], "blue") == 0 || strcmp(cmd[1], "-blue") == 0){
		printf("Togglin BLUE LED");
		BLUE_LED ^= BLUE_BLINK;
	}else if(strcmp(cmd[1], "clear") == 0 || strcmp(cmd[1], "-clear") == 0){
		printf("Togglin GREEN LED");
		GREEN_LED ^= GREEN_BLINK;
	}
	
	
}




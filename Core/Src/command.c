// includes
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "queue.h"
#include "command.h"
#include "rtc.h"
#include "temperature.h"
#include "battery.h"
#include "tsl237.h"
#include "sample.h"
#include "flash.h"


extern queue_t rx_queue;

// function prototypes
void help_command(char *);
void lof_command(char *);
void lon_command(char *);
//void test_command(char *);
void ds_command(char *);
void ts_command(char *);
void dr_command(char *);
void tr_command(char *);
void temp_read_command(char *);
void tsl237_command(char *);
void battery_read_command(char *);
void sample_command(char *);
void log_command(char *);
void data_command(char *);
void flash_erase_command(char *);

command_t commands[] = {
		{"help", help_command},
//		{"test", test_command},
		{"ds", ds_command},
		{"ts", ts_command},
		{"dr", dr_command},
		{"tr", tr_command},
		{"temp", temp_read_command},
		{"tsl237", tsl237_command},
		{"battery", battery_read_command},
		{"sample", sample_command},
		{"log", log_command},
		{"data", data_command},
		{"ef", flash_erase_command},
		{0,0}
};

// Creating an enum to represent the modes
enum {COLLECT_CHARS, COMPLETED};
// command file from video
int get_command(uint8_t *command_buf) {
	static uint32_t counter=0;
	static uint32_t mode = COLLECT_CHARS;

	uint8_t ch=0;;
	// uint32_t mask;

	ch=dequeue(&rx_queue);

	while(ch!=0) {
		// check end of line
		if ((ch!='\n') && (ch!='\r')) {
//			command_buf[counter] = '\0'; // terminate
//			counter = 0;
//			return 1;
		// check backspace
			if (ch == 0x7f) {
				if (counter > 0) {
					printf("\b \b");
					counter--;
				}
			}
			else {
				putchar(ch);
				command_buf[counter++] = ch;

				if (counter >= (QUEUE_SIZE - 2)) {
					command_buf[counter] = '\0';
					counter = 0;
					mode = COMPLETED;
					return 1;
				}
			}
		}
		else {
			mode = COMPLETED;
			break;
		}
		ch = dequeue(&rx_queue);
	}
	if (mode == COMPLETED) {
		command_buf[counter] = 0;
		printf("\n\r");
		counter = 0;
		mode = COLLECT_CHARS;
		return(1);
	}
	else {
		return(0);
	}
}

// Splits commands and arguments
int parse_command (uint8_t *line, uint8_t **command, uint8_t **args) {
	//looks for first comma, places NULL and captures remainder
	uint8_t *p;

	if((!line) || (!command) || (!args)) {
		return (-1); // passed bad pointer
	}
	*command=line;
	p=line;
	while(*p!=',') {
		if (!*p) {
			*args = NULL;
			return(0);
		}
		p++;
	}
	*p++ = '\0'; // replace first comma with null
	*args = p; // arguments right after comma
	return (0);
}

int execute_command(uint8_t * line) {
	uint8_t *cmd;
	uint8_t *arg;
	command_t *p = commands;
	int success = 0;

	if (!line) {
		return (-1); // Passed a bad pointer
	}
	if (parse_command(line,&cmd,&arg) == -1) {
		printf("Error with parse command\n\r");
		return (-1);
	}
	while (p->cmd_string) {
		if (!strcmp(p->cmd_string, (char *) cmd )){
			if (!p->cmd_function){
				return (-1);
			}
			// running cmd with passed args
			(*p->cmd_function)((char *)arg);
			success = 1;
			break;
		}
		p++;
	}
	if (success) {
		return 0;
	}
	else {
		return -1;
	}
}


void __attribute__((weak)) help_command(char *arguments) {
	command_t * p = commands;
	printf("Available Commands:\n\r");
	while (p->cmd_string) {
		printf("%s\n\r",p->cmd_string);
		p++;
	}
	printf("OK\n\r");
	}


void __attribute__((weak)) ds_command(char *arguments){
	printf("DS Command Processing\n\r");
	if (arguments){
		printf("Arguments = %s\n\r", arguments);
	}
}

void __attribute__((weak)) ts_command(char *arguments){
	printf("TS Command Processing\n\r");
	if (arguments){
		printf("Arguments = %s\n\r", arguments);
	}
}

void __attribute__((weak)) dr_command(char *arguments){
	printf("DR Command Processing\n\r");
	if (arguments){
		printf("Arguments = %s\n\r", arguments);
	}
}

void __attribute__((weak)) tr_command(char *arguments){
	printf("TR Command Processing\n\r");
	if (arguments){
		printf("Arguments = %s\n\r", arguments);
	}
}

void __attribute__((weak)) temp_read_command(char *arguments){
	printf("Temp Command Processing\n\r");
	if (arguments){
		printf("Arguments = %s\n\r", arguments);
	}
}


void __attribute__((weak)) tsl237_command(char *arguments){
	printf("TSL237 Command Processing\n\r");
	if (arguments) {
		printf("Arguments = %s\n\r", arguments);
	}
}

void __attribute__((weak)) battery_read_command(char *arguments){
	printf("Battery Command Processing\n\r");
	if (arguments){
		printf("Arguments = %s\n\r", arguments);
	}
}

void __attribute__((weak)) sample_command(char *arguments){
	printf("Sample Command Processing\n\r");
	if (arguments){
		printf("Arguments = %s\n\r", arguments);
	}
}

void __attribute__((weak)) log_command(char *arguments) {
	printf("Log Command Processing");
	if (arguments) {
		printf("Arguments = %s\n\r", arguments);
	}
}

void __attribute__((weak)) data_command(char *arguments){
	printf("Data Command Processing\n\r");
	if (arguments){
		printf("Arguments = %s\n\r", arguments);
	}
}

void __attribute__((weak)) flash_erase_command(char *arguments){
	printf("Flash Erase Command Processing\n\r");
	if (arguments){
		printf("Arguments = %s\n\r", arguments);
	}
}

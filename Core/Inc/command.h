#ifndef COMMAND_H
#define COMMAND_H

#include <stdint.h>

// return values for get_command()
#define INCOMPLETE 0
#define COMPLETE   1
#define ERROR     -1

typedef struct command {
	char * cmd_string;
	void(*cmd_function)(char * arg);
} command_t;

int get_command(uint8_t *command_buf);
int parse_command(uint8_t *line, uint8_t **command, uint8_t **args);
int execute_command(uint8_t *line);

#endif

#ifndef INC_TSL237_H_
#define INC_TSL237_H_

#include "main.h"
#include <stdint.h>

float read_tsl237_frequency(void);
uint32_t read_tsl237_period(void);
void tsl237_command(char *arguments);

#endif /* INC_TSL237_H_ */
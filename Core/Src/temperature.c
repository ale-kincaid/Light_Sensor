/*
 * temperature.c
 *
 *  Created on: Feb 22, 2026
 *      Author: Ale K. & Ben G.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "temperature.h"
#include "adc.h"

void temp_read_command(char *arguments){
	sensorval_t mysensor;
	uint32_t read_status;
	read_status = readsensor(&mysensor);
	if(read_status != 0){
		printf("NOK\n\r");
		return;
	}
	printf("%d C\n\r", (int)mysensor.temperature);
	printf("OK\n\r");
}


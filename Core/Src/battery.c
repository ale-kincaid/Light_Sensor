/*
 * battery.c
 *
 *  Created on: Feb 24, 2026
 *      Author: Ale K. & Ben G.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "battery.h"
#include "adc.h"

void battery_read_command(char *arguments){
	sensorval_t mysensor;
	uint32_t read_status;
	read_status = readsensor(&mysensor);
	if(read_status != 0){
		printf("NOK\n\r");
		return;
	}
	// convert vdda value by /1000
	printf("%.2f V\n\r", (float)mysensor.vdda_value / 1000);
	printf("OK\n\r");
}

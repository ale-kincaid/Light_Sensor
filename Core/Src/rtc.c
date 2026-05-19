#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rtc.h"

extern RTC_HandleTypeDef hrtc;

void ds_command(char *arguments) {
	RTC_DateTypeDef current_date = {0};
	char * argument;
	int argument_count = 0;

	if (!arguments) {
		printf("NOK\n\r");
		return;
	}

	argument = strtok(arguments, ",");
	if (!argument) {
		printf("NOK\n\r");
		return;
	}
	while(argument) {
		switch (argument_count) {
			case 0:
				current_date.Month = (int) strtol(argument, NULL, 10);
				if ((current_date.Month<1) || (current_date.Month>12)) {
					printf("NOK\n\r");
					return;
				}
				break;
			case 1:
				current_date.Date = (int) strtol(argument, NULL, 10);
				if ((current_date.Date<1) || (current_date.Date>31)) {
					printf("NOK\n\r");
					return;
				}
				break;
			case 2:
				current_date.Year = (int) strtol(argument, NULL, 10);
				if ((current_date.Year<0) || (current_date.Year>99)) {
					printf("NOK\n\r");
					return;
				}
				break;
			default:
				printf("NOK\n\r");
				return;
		}
		argument_count++;
		argument=strtok(NULL, ",");
	}

	if (argument_count != 3) {
		printf("NOK\n\r");
		return;
	}

	if (HAL_RTC_SetDate(&hrtc, &current_date, RTC_FORMAT_BIN) != HAL_OK) {
		printf("NOK\n\r");
		return;
	}
	printf("OK\n\r");
}

void dr_command(char *arguments) {
	RTC_TimeTypeDef current_time;
	RTC_DateTypeDef current_date;
	if (arguments) {
		printf("NOK\n\r");
	}
	else {
		HAL_RTC_GetTime(&hrtc, &current_time, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &current_date, RTC_FORMAT_BIN);
		printf("dr,%02d,%02d,20%02d\n\r", current_date.Month, current_date.Date, current_date.Year);
		printf("OK\n\r");
	}
}

void ts_command(char *arguments) {
	RTC_TimeTypeDef current_time;
	char * argument;
	int argument_count = 0;

	if (!arguments) {
		printf("NOK\n\r");
		return;
	}
	argument = strtok(arguments, ",");
	if (!argument) {
		printf("NOK\n\r");
		return;
	}

	while (argument) {
		switch (argument_count) {
		case 0:
			current_time.Hours = (int) strtol(argument, NULL, 10);
			if ((current_time.Hours > 23) || (current_time.Hours < 0)) {
				printf("NOK\n\r");
				return;
			}
			break;
		case 1:
			current_time.Minutes = (int) strtol(argument, NULL, 10);
			if ((current_time.Minutes > 59) || (current_time.Minutes < 0)) {
				printf("NOK\n\r");
				return;
			}
			break;
		case 2:
			current_time.Seconds = (int) strtol(argument, NULL, 10);
			if ((current_time.Seconds > 59) || (current_time.Seconds <0)) {
				printf("NOK\n\r");
				return;
			}
			break;
		default:
			printf("NOK\n\r");
			return;
		}
		argument_count++;
		argument=strtok(NULL, ",");
	}

	if (argument_count != 3) {
		printf("NOK\n\r");
		return;
	}

	if (HAL_RTC_SetTime(&hrtc, &current_time, RTC_FORMAT_BIN) != HAL_OK) {
		printf("NOK\n\r");
		return;
	}

	printf("OK\n\r");
}

void tr_command(char *arguments) {
	RTC_TimeTypeDef current_time;
	RTC_DateTypeDef current_date;
	if (arguments) {
		printf("NOK\n\r");
	}
	else {
		HAL_RTC_GetTime(&hrtc, &current_time, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &current_date, RTC_FORMAT_BIN);
		printf("tr,%02d,%02d,%02d\n\r", current_time.Hours, current_time.Minutes, current_time.Seconds);
		printf("OK\n\r");
	}
}

void prompt(void) {
	RTC_TimeTypeDef current_time;
	RTC_DateTypeDef current_date;
	HAL_RTC_GetTime(&hrtc,&current_time,RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc,&current_date,RTC_FORMAT_BIN);
	printf("%02d/%02d/20%02d ",current_date.Month,current_date.Date,current_date.Year);
	printf("%02d:%02d:%02d ",current_time.Hours,current_time.Minutes,current_time.Seconds);
	printf("IULS> ");
}

